/*****************************************************************************
 * lookahead.c: Lookahead slicetype decisions for x264
 *****************************************************************************
 * Lookahead.c and associated modifications:
 *     Copyright (C) 2008 Avail Media
 *
 * Authors: Michael Kazmier <mkazmier@availmedia.com>
 *          Alex Giladi <agiladi@availmedia.com>
 *          Steven Walters <kemuri9@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/* LOOKAHEAD (threaded and non-threaded mode)
 *
 * Lookahead types:
 *     [1] Slice type / scene cut;
 *
 * In non-threaded mode, we run the existing slicetype decision code as it was.
 * In threaded mode, we run in a separate thread, that lives between the calls
 * to x264_encoder_open() and x264_encoder_close(), and performs lookahead for
 * the number of frames specified in rc_lookahead.  Recommended setting is
 * # of bframes + # of threads.
 */
#include "common/common.h"
#include "analyse.h"

#ifdef _DEBUG_LOOKAHEAD_
int xavs_lookahead_output_count = 0;
int xavs_lookahead_input_count = 0;
#endif

static void xavs_lookahead_shift( xavs_synch_frame_list_t *dst, xavs_synch_frame_list_t *src, int count )
{
    int i = count;
    while (i--)
    {
        assert(dst->i_size < dst->i_max_size);
        assert(src->i_size);
        dst->list[dst->i_size++] = xavs_frame_get( src->list );
        src->i_size--;
    }
    if (count)
    {
        xavs_pthread_cond_broadcast( &dst->cv_fill );
        xavs_pthread_cond_broadcast( &src->cv_empty );
    }
}

static void xavs_lookahead_update_last_nonb( xavs_t *h, xavs_frame_t *new_nonb )
{
    if (h->lookahead->last_nonb)
        xavs_frame_put_unused(h, h->lookahead->last_nonb);
    h->lookahead->last_nonb = new_nonb;
    new_nonb->i_reference_count++;
}

#ifdef HAVE_PTHREAD
static void xavs_lookahead_slicetype_decide( xavs_t *h )
{
    xavs_slicetype_decide( h );

    xavs_lookahead_update_last_nonb( h, h->lookahead->next.list[0] );

    xavs_pthread_mutex_lock( &h->lookahead->ofbuf.mutex );
    while( h->lookahead->ofbuf.i_size == h->lookahead->ofbuf.i_max_size )
        xavs_pthread_cond_wait( &h->lookahead->ofbuf.cv_empty, &h->lookahead->ofbuf.mutex );

    xavs_pthread_mutex_lock( &h->lookahead->next.mutex );
    xavs_lookahead_shift( &h->lookahead->ofbuf, &h->lookahead->next, 
                          h->lookahead->next.list[0]->i_bframes + 1 );
    xavs_pthread_mutex_unlock( &h->lookahead->next.mutex );

    /* For MB-tree and VBV lookahead, we have to perform propagation analysis on I-frames too. */
    if( h->lookahead->b_analyse_keyframe && IS_XAVS_TYPE_I( h->lookahead->last_nonb->i_type ) )
        xavs_slicetype_analyse( h );

    xavs_pthread_mutex_unlock( &h->lookahead->ofbuf.mutex );
}

static void xavs_lookahead_thread( xavs_t *h )
{
    int shift;
#ifdef HAVE_MMX
    if( h->param.cpu&XAVS_CPU_SSE_MISALIGN )
        xavs_cpu_mask_misalign_sse();
#endif
    while( !h->lookahead->b_exit_thread )
    {
        xavs_pthread_mutex_lock( &h->lookahead->ifbuf.mutex );
        xavs_pthread_mutex_lock( &h->lookahead->next.mutex );
        shift = XAVS_MIN(h->lookahead->next.i_max_size - h->lookahead->next.i_size, 
                         h->lookahead->ifbuf.i_size );
        xavs_lookahead_shift( &h->lookahead->next, &h->lookahead->ifbuf, shift );
        xavs_pthread_mutex_unlock( &h->lookahead->next.mutex );
        if( h->lookahead->next.i_size <= h->lookahead->i_slicetype_length )
        {
            while( !h->lookahead->ifbuf.i_size && !h->lookahead->b_exit_thread )
            {
                h->lookahead->b_input_pending = 1;
                xavs_pthread_cond_wait( &h->lookahead->ifbuf.cv_fill, &h->lookahead->ifbuf.mutex );
            }
            h->lookahead->b_input_pending = 0;
            xavs_pthread_mutex_unlock( &h->lookahead->ifbuf.mutex );
        }
        else
        {
            xavs_pthread_mutex_unlock( &h->lookahead->ifbuf.mutex );
            xavs_lookahead_slicetype_decide( h );
        }
    }   /* end of input frames */
    xavs_pthread_mutex_lock( &h->lookahead->ifbuf.mutex );
    xavs_pthread_mutex_lock( &h->lookahead->next.mutex );
    xavs_lookahead_shift( &h->lookahead->next, &h->lookahead->ifbuf, h->lookahead->ifbuf.i_size );
    xavs_pthread_mutex_unlock( &h->lookahead->next.mutex );
    xavs_pthread_mutex_unlock( &h->lookahead->ifbuf.mutex );
#ifdef _DEBUG_LOOKAHEAD_
    printf("\n lookahead quiting, ifbuf: %d, next:%d, ofbuf: %d, input: %d, output: %d\n", 
            h->lookahead->ifbuf.i_size, h->lookahead->next.i_size, h->lookahead->ofbuf.i_size,
            xavs_lookahead_input_count, xavs_lookahead_output_count);
#endif
    while( h->lookahead->next.i_size )
        xavs_lookahead_slicetype_decide( h );
    xavs_pthread_mutex_lock( &h->lookahead->ofbuf.mutex );
    h->lookahead->b_thread_active = 0;
    xavs_pthread_cond_broadcast( &h->lookahead->ofbuf.cv_fill );
    xavs_pthread_mutex_unlock( &h->lookahead->ofbuf.mutex );
}
#endif

int xavs_lookahead_init( xavs_t *h, int i_slicetype_length )
{
    int i;
    xavs_lookahead_t *look;
    xavs_t *look_h;

    CHECKED_MALLOCZERO( look, sizeof(xavs_lookahead_t) );
    h->lookahead = look;
    for( i = 0; i < h->param.i_threads; i++ )
        h->thread[i]->lookahead = look;

    look->i_last_idr = - h->param.i_keyint_max;
    look->b_analyse_keyframe = (h->param.rc.b_mb_tree 
                                || (h->param.rc.i_vbv_buffer_size && h->param.rc.i_lookahead))
                               && !h->param.rc.b_stat_read;
    look->i_slicetype_length = i_slicetype_length;

    /* init frame lists */
    if( xavs_synch_frame_list_init( &look->ifbuf, h->param.i_sync_lookahead+3 ) ||
        xavs_synch_frame_list_init( &look->next, h->frames.i_delay+3 ) ||
        xavs_synch_frame_list_init( &look->ofbuf, h->frames.i_delay+3 ) )
        goto fail;

    if( !h->param.i_sync_lookahead ) {
        return 0;
    }

    /*xavs_t **/
	look_h = h->thread[h->param.i_threads];
    *look_h = *h;
    xavs_macroblock_cache_init( look_h );

    // if( xavs_macroblock_thread_allocate( look_h, 1 ) < 0 )
    //     goto fail;

    if( xavs_pthread_create( &look_h->thread_handle, NULL, (void *)xavs_lookahead_thread, look_h ) )
        goto fail;
    look->b_thread_active = 1;

    return 0;
fail:
    xavs_free( look );
    return -1;
}

void xavs_lookahead_delete( xavs_t *h )
{
    if( h->param.i_sync_lookahead )
    {
        xavs_pthread_mutex_lock( &h->lookahead->ifbuf.mutex );
        h->lookahead->b_exit_thread = 1;
        xavs_pthread_cond_broadcast( &h->lookahead->ifbuf.cv_fill );
        xavs_pthread_mutex_unlock( &h->lookahead->ifbuf.mutex );
        xavs_pthread_join( h->thread[h->param.i_threads]->thread_handle, NULL );
        xavs_macroblock_cache_end( h->thread[h->param.i_threads] );
        // xavs_macroblock_thread_free( h->thread[h->param.i_threads], 1 );
        xavs_free( h->thread[h->param.i_threads] );
    }
    xavs_synch_frame_list_delete( &h->lookahead->ifbuf );
    xavs_synch_frame_list_delete( &h->lookahead->next );
    if (h->lookahead->last_nonb)
        xavs_frame_put_unused( h, h->lookahead->last_nonb );
    xavs_synch_frame_list_delete( &h->lookahead->ofbuf );
    xavs_free( h->lookahead );
}

void xavs_lookahead_put_frame( xavs_t *h, xavs_frame_t *frame )
{
    if( h->param.i_sync_lookahead )
        xavs_synch_frame_list_push( &h->lookahead->ifbuf, frame );
    else
        xavs_synch_frame_list_push( &h->lookahead->next, frame );

#ifdef _DEBUG_LOOKAHEAD_
    xavs_lookahead_input_count ++;
#endif
}

int xavs_lookahead_is_empty( xavs_t *h )
{
	int b_empty;
    xavs_pthread_mutex_lock( &h->lookahead->ofbuf.mutex );
    xavs_pthread_mutex_lock( &h->lookahead->next.mutex );
    /*int*/
	b_empty = !h->lookahead->next.i_size && !h->lookahead->ofbuf.i_size;
    xavs_pthread_mutex_unlock( &h->lookahead->next.mutex );
    xavs_pthread_mutex_unlock( &h->lookahead->ofbuf.mutex );
    return b_empty;
}

static void xavs_lookahead_encoder_shift( xavs_t *h )
{
	int i_frames;
    if( !h->lookahead->ofbuf.i_size )
        return;
    /*int*/
	i_frames = h->lookahead->ofbuf.list[0]->i_bframes + 1;
#ifdef _DEBUG_LOOKAHEAD_
    xavs_lookahead_output_count += i_frames;
#endif
    while( i_frames-- )
    {
        xavs_frame_put(h->frames.current, xavs_frame_get(h->lookahead->ofbuf.list));
        h->lookahead->ofbuf.i_size--;
    }
    xavs_pthread_cond_broadcast( &h->lookahead->ofbuf.cv_empty );
#ifdef _DEBUG_LOOKAHEAD_
    printf("input: %d, output: %d", 
           xavs_lookahead_input_count, xavs_lookahead_output_count);
#endif
}

void xavs_lookahead_get_frames( xavs_t *h )
{
    if( h->param.i_sync_lookahead )
    {   /* We have a lookahead thread, so get frames from there */
        xavs_pthread_mutex_lock( &h->lookahead->ofbuf.mutex );
        while( !h->lookahead->ofbuf.i_size && !h->lookahead->b_input_pending 
               && h->lookahead->b_thread_active )
        {
            xavs_pthread_cond_wait( &h->lookahead->ofbuf.cv_fill, &h->lookahead->ofbuf.mutex );
        }
        xavs_lookahead_encoder_shift( h );
        xavs_pthread_mutex_unlock( &h->lookahead->ofbuf.mutex );
    }
    else
    {   /* We are not running a lookahead thread, so perform all the slicetype decide on the fly */
        if( h->frames.current[0] || !h->lookahead->next.i_size )
            return;

        xavs_slicetype_decide( h );
        h->lookahead->last_nonb = h->lookahead->next.list[0];
        xavs_lookahead_shift( &h->lookahead->ofbuf, &h->lookahead->next, 
                              h->lookahead->next.list[0]->i_bframes + 1 );

        /* For MB-tree and VBV lookahead, we have to perform propagation analysis on I-frames too. */
        if( h->lookahead->b_analyse_keyframe && IS_XAVS_TYPE_I( h->lookahead->last_nonb->i_type ) )
            xavs_slicetype_analyse( h );

        xavs_lookahead_encoder_shift( h );
        h->lookahead->last_nonb = NULL;
    }
}

