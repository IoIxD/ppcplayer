/*****************************************************************************
 * me.c: h264 encoder library (Motion Estimation)
 *****************************************************************************
 * Copyright (C) 2003-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Jason Garrett-Glaser <darkshikari@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * me.c: xavs encoder library (Motion Estimation)
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "common/common.h"
#include "me.h"

/* presets selected from good points on the speed-vs-quality curve of several test videos
 * subpel_iters[i_subpel_refine] = { refine_hpel, refine_qpel, me_hpel, me_qpel }
 * where me_* are the number of EPZS iterations run on all candidate block types,
 * and refine_* are run only on the winner. */
static const int subpel_iterations[][4] = { {1, 0, 0, 0},
{1, 1, 0, 0},
{0, 1, 1, 0},
{0, 2, 1, 0},
{0, 2, 1, 1},
{0, 2, 1, 2},
{0, 0, 2, 2},
{0, 0, 2, 2}
};

static const int mod6m1[8] = { 5, 0, 1, 2, 3, 4, 5, 0 };
static const int hex2[8][2] = { {-1, -2}, {-2, 0}, {-1, 2}, {1, 2}, {2, 0}, {1, -2}, {-1, -2}, {-2, 0} };
static const int square1[9][2] = { {0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

static void refine_subpel (xavs_t * h, xavs_me_t * m, int hpel_iters, int qpel_iters, int *p_halfpel_thresh, int b_refine_qpel);

#define BITS_MVD( mx, my )\
    (p_cost_mvx[(mx)<<2] + p_cost_mvy[(my)<<2])

#define COST_MV( mx, my )\
{\
    int cost = h->pixf.sad[i_pixel]( m->p_fenc[0], FENC_STRIDE,\
                   &p_fref[(my)*m->i_stride[0]+(mx)], m->i_stride[0] )\
             + BITS_MVD(mx,my);\
    COPY3_IF_LT( bcost, cost, bmx, mx, bmy, my );\
}

#define COST_MV_X3_DIR( m0x, m0y, m1x, m1y, m2x, m2y, costs )\
{\
    uint8_t *pix_base = p_fref + bmx + bmy*m->i_stride[0];\
    h->pixf.sad_x3[i_pixel]( m->p_fenc[0],\
        pix_base + (m0x) + (m0y)*m->i_stride[0],\
        pix_base + (m1x) + (m1y)*m->i_stride[0],\
        pix_base + (m2x) + (m2y)*m->i_stride[0],\
        m->i_stride[0], costs );\
    (costs)[0] += BITS_MVD( bmx+(m0x), bmy+(m0y) );\
    (costs)[1] += BITS_MVD( bmx+(m1x), bmy+(m1y) );\
    (costs)[2] += BITS_MVD( bmx+(m2x), bmy+(m2y) );\
}

#define COST_MV_X4( m0x, m0y, m1x, m1y, m2x, m2y, m3x, m3y )\
{\
    uint8_t *pix_base = p_fref + omx + omy*m->i_stride[0];\
    h->pixf.sad_x4[i_pixel]( m->p_fenc[0],\
        pix_base + (m0x) + (m0y)*m->i_stride[0],\
        pix_base + (m1x) + (m1y)*m->i_stride[0],\
        pix_base + (m2x) + (m2y)*m->i_stride[0],\
        pix_base + (m3x) + (m3y)*m->i_stride[0],\
        m->i_stride[0], costs );\
    costs[0] += BITS_MVD( omx+(m0x), omy+(m0y) );\
    costs[1] += BITS_MVD( omx+(m1x), omy+(m1y) );\
    costs[2] += BITS_MVD( omx+(m2x), omy+(m2y) );\
    costs[3] += BITS_MVD( omx+(m3x), omy+(m3y) );\
    COPY3_IF_LT( bcost, costs[0], bmx, omx+(m0x), bmy, omy+(m0y) );\
    COPY3_IF_LT( bcost, costs[1], bmx, omx+(m1x), bmy, omy+(m1y) );\
    COPY3_IF_LT( bcost, costs[2], bmx, omx+(m2x), bmy, omy+(m2y) );\
    COPY3_IF_LT( bcost, costs[3], bmx, omx+(m3x), bmy, omy+(m3y) );\
}

#define COST_MV_X4_ABS( m0x, m0y, m1x, m1y, m2x, m2y, m3x, m3y )\
{\
    h->pixf.sad_x4[i_pixel]( m->p_fenc[0],\
        p_fref + (m0x) + (m0y)*m->i_stride[0],\
        p_fref + (m1x) + (m1y)*m->i_stride[0],\
        p_fref + (m2x) + (m2y)*m->i_stride[0],\
        p_fref + (m3x) + (m3y)*m->i_stride[0],\
        m->i_stride[0], costs );\
    costs[0] += BITS_MVD( m0x, m0y );\
    costs[1] += BITS_MVD( m1x, m1y );\
    costs[2] += BITS_MVD( m2x, m2y );\
    costs[3] += BITS_MVD( m3x, m3y );\
    COPY3_IF_LT( bcost, costs[0], bmx, m0x, bmy, m0y );\
    COPY3_IF_LT( bcost, costs[1], bmx, m1x, bmy, m1y );\
    COPY3_IF_LT( bcost, costs[2], bmx, m2x, bmy, m2y );\
    COPY3_IF_LT( bcost, costs[3], bmx, m3x, bmy, m3y );\
}

/*  1  */
/* 101 */
/*  1  */
#define DIA1_ITER( mx, my )\
{\
    omx = mx; omy = my;\
    COST_MV_X4( 0,-1, 0,1, -1,0, 1,0 );\
}

#define CROSS( start, x_max, y_max )\
{\
    i = start;\
    if( x_max <= XAVS_MIN(mv_x_max-omx, omx-mv_x_min) )\
        for( ; i < x_max-2; i+=4 )\
            COST_MV_X4( i,0, -i,0, i+2,0, -i-2,0 );\
    for( ; i < x_max; i+=2 )\
    {\
        if( omx+i <= mv_x_max )\
            COST_MV( omx+i, omy );\
        if( omx-i >= mv_x_min )\
            COST_MV( omx-i, omy );\
    }\
    i = start;\
    if( y_max <= XAVS_MIN(mv_y_max-omy, omy-mv_y_min) )\
        for( ; i < y_max-2; i+=4 )\
            COST_MV_X4( 0,i, 0,-i, 0,i+2, 0,-i-2 );\
    for( ; i < y_max; i+=2 )\
    {\
        if( omy+i <= mv_y_max )\
            COST_MV( omx, omy+i );\
        if( omy-i >= mv_y_min )\
            COST_MV( omx, omy-i );\
    }\
}


void
xavs_me_search_ref (xavs_t * h, xavs_me_t * m, int (*mvc)[2], int i_mvc, int *p_halfpel_thresh)
{
  const int i_pixel = m->i_pixel;
  int i_me_range = h->param.analyse.i_me_range;
  int bmx, bmy, bcost;
  int omx, omy, pmx, pmy;
  uint8_t *p_fref = m->p_fref[0];
  int i, j;
  int dir;
  int costs[6];

  int mv_x_min = h->mb.mv_min_fpel[0];
  int mv_y_min = h->mb.mv_min_fpel[1];
  int mv_x_max = h->mb.mv_max_fpel[0];
  int mv_y_max = h->mb.mv_max_fpel[1];

  const int16_t *p_cost_mvx = m->p_cost_mv - m->mvp[0];
  const int16_t *p_cost_mvy = m->p_cost_mv - m->mvp[1];

  if (h->mb.i_me_method == XAVS_ME_UMH)
  {
    // clamp mvp to inside frame+padding, so that we don't have to check it each iteration 
    p_cost_mvx = m->p_cost_mv - xavs_clip3 (m->mvp[0], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
    p_cost_mvy = m->p_cost_mv - xavs_clip3 (m->mvp[1], h->mb.mv_min_spel[1], h->mb.mv_max_spel[1]);
  }

  bmx = pmx = xavs_clip3 ((m->mvp[0] + 2) >> 2, mv_x_min, mv_x_max);
  bmy = pmy = xavs_clip3 ((m->mvp[1] + 2) >> 2, mv_y_min, mv_y_max);
  bcost = COST_MAX;
  COST_MV (pmx, pmy);
  // I don't know why this helps 
  bcost -= BITS_MVD (bmx, bmy);

  // try extra predictors if provided 
  for (i = 0; i < i_mvc; i++)
  {
    const int mx = xavs_clip3 ((mvc[i][0] + 2) >> 2, mv_x_min, mv_x_max);
    const int my = xavs_clip3 ((mvc[i][1] + 2) >> 2, mv_y_min, mv_y_max);
    if (mx != bmx || my != bmy)
      COST_MV (mx, my);
  }

  COST_MV (0, 0);

  mv_x_max += 8;
  mv_y_max += 8;
  mv_x_min -= 8;
  mv_y_min -= 8;

  switch (h->mb.i_me_method)
  {
  case XAVS_ME_DIA:
    // diamond search, radius 1 
    for (i = 0; i < i_me_range; i++)
    {
      DIA1_ITER (bmx, bmy);
      if (bmx == omx && bmy == omy)
        break;
    }
    break;

  case XAVS_ME_HEX:
  me_hex2:
    // hexagon search, radius 2 
#if 0
    for (i = 0; i < i_me_range / 2; i++)
    {
      omx = bmx;
      omy = bmy;
      COST_MV (omx - 2, omy);
      COST_MV (omx - 1, omy + 2);
      COST_MV (omx + 1, omy + 2);
      COST_MV (omx + 2, omy);
      COST_MV (omx + 1, omy - 2);
      COST_MV (omx - 1, omy - 2);
      if (bmx == omx && bmy == omy)
        break;
    }
#else
    // equivalent to the above, but eliminates duplicate candidates 
    dir = -2;

    // hexagon 
    COST_MV_X3_DIR (-2, 0, -1, 2, 1, 2, costs);
    COST_MV_X3_DIR (2, 0, 1, -2, -1, -2, costs + 3);
    COPY2_IF_LT (bcost, costs[0], dir, 0);
    COPY2_IF_LT (bcost, costs[1], dir, 1);
    COPY2_IF_LT (bcost, costs[2], dir, 2);
    COPY2_IF_LT (bcost, costs[3], dir, 3);
    COPY2_IF_LT (bcost, costs[4], dir, 4);
    COPY2_IF_LT (bcost, costs[5], dir, 5);

    if (dir != -2)
    {
      static const int hex2[8][2] = { {-1, -2}, {-2, 0}, {-1, 2}, {1, 2}, {2, 0}, {1, -2}, {-1, -2}, {-2, 0} };
      bmx += hex2[dir + 1][0];
      bmy += hex2[dir + 1][1];
      // half hexagon, not overlapping the previous iteration 
      for (i = 1; i < i_me_range / 2; i++)
      {
        static const int mod6[8] = { 5, 0, 1, 2, 3, 4, 5, 0 };
        const int odir = mod6[dir + 1];
        COST_MV_X3_DIR (hex2[odir + 0][0], hex2[odir + 0][1], hex2[odir + 1][0], hex2[odir + 1][1], hex2[odir + 2][0], hex2[odir + 2][1], costs);
        dir = -2;
        COPY2_IF_LT (bcost, costs[0], dir, odir - 1);
        COPY2_IF_LT (bcost, costs[1], dir, odir);
        COPY2_IF_LT (bcost, costs[2], dir, odir + 1);
        if (dir == -2)
          break;
        bmx += hex2[dir + 1][0];
        bmy += hex2[dir + 1][1];
      }
    }
#endif
    // square refine 
    omx = bmx;
    omy = bmy;
    COST_MV_X4 (0, -1, 0, 1, -1, 0, 1, 0);
    COST_MV_X4 (-1, -1, -1, 1, 1, -1, 1, 1);
    break;

  case XAVS_ME_UMH:
    {
      // Uneven-cross Multi-Hexagon-grid Search
      // as in JM, except with different early termination 

      static const int xavs_pixel_size_shift[7] = { 0, 1, 1, 2, 3, 3, 4 };

      int ucost1, ucost2;
      int cross_start = 1;

      // refine predictors 
      ucost1 = bcost;
      DIA1_ITER (pmx, pmy);
      if (pmx || pmy)
        DIA1_ITER (0, 0);

      if (i_pixel == PIXEL_4x4)
        goto me_hex2;

      ucost2 = bcost;
      if ((bmx || bmy) && (bmx != pmx || bmy != pmy))
        DIA1_ITER (bmx, bmy);
      if (bcost == ucost2)
        cross_start = 3;
      omx = bmx;
      omy = bmy;

      // early termination 
#define SAD_THRESH(v) ( bcost < ( v >> xavs_pixel_size_shift[i_pixel] ) )
      if (bcost == ucost2 && SAD_THRESH (2000))
      {
        COST_MV_X4 (0, -2, -1, -1, 1, -1, -2, 0);
        COST_MV_X4 (2, 0, -1, 1, 1, 1, 0, 2);
        if (bcost == ucost1 && SAD_THRESH (500))
          break;
        if (bcost == ucost2)
        {
          int range = (i_me_range >> 1) | 1;
          CROSS (3, range, range);
          COST_MV_X4 (-1, -2, 1, -2, -2, -1, 2, -1);
          COST_MV_X4 (-2, 1, 2, 1, -1, 2, 1, 2);
          if (bcost == ucost2)
            break;
          cross_start = range + 2;
        }
      }

      // adaptive search range 
      if (i_mvc)
      {
        // range multipliers based on casual inspection of some statistics of
        // average distance between current predictor and final mv found by ESA.
        // these have not been tuned much by actual encoding. 
        static const int range_mul[4][4] = {
          {3, 3, 4, 4},
          {3, 4, 4, 4},
          {4, 4, 4, 5},
          {4, 4, 5, 6},
        };
        int mvd;
        int sad_ctx, mvd_ctx;

        if (i_mvc == 1)
        {
          if (i_pixel == PIXEL_16x16)
            // mvc is probably the same as mvp, so the difference isn't meaningful.
            // but prediction usually isn't too bad, so just use medium range 
            mvd = 25;
          else
            mvd = abs (m->mvp[0] - mvc[0][0]) + abs (m->mvp[1] - mvc[0][1]);
        }
        else
        {
          // calculate the degree of agreement between predictors. 
          //in 16x16, mvc includes all the neighbors used to make mvp,
          //so don't count mvp separately. 
          int i_denom = i_mvc - 1;
          mvd = 0;
          if (i_pixel != PIXEL_16x16)
          {
            mvd = abs (m->mvp[0] - mvc[0][0]) + abs (m->mvp[1] - mvc[0][1]);
            i_denom++;
          }
          for (i = 0; i < i_mvc - 1; i++)
            mvd += abs (mvc[i][0] - mvc[i + 1][0]) + abs (mvc[i][1] - mvc[i + 1][1]);
          mvd /= i_denom;       //FIXME idiv
        }

        sad_ctx = SAD_THRESH (1000) ? 0 : SAD_THRESH (2000) ? 1 : SAD_THRESH (4000) ? 2 : 3;
        mvd_ctx = mvd < 10 ? 0 : mvd < 20 ? 1 : mvd < 40 ? 2 : 3;

        i_me_range = i_me_range * range_mul[mvd_ctx][sad_ctx] / 4;
      }

      // FIXME if the above DIA2/OCT2/CROSS found a new mv, it has not updated omx/omy.
      // we are still centered on the same place as the DIA2. is this desirable? 
      CROSS (cross_start, i_me_range, i_me_range / 2);

      // 5x5 ESA 
      omx = bmx;
      omy = bmy;
      if (bcost != ucost2)
        COST_MV_X4 (1, 0, 0, 1, -1, 0, 0, -1);
      COST_MV_X4 (1, 1, -1, 1, -1, -1, 1, -1);
      COST_MV_X4 (2, -1, 2, 0, 2, 1, 2, 2);
      COST_MV_X4 (1, 2, 0, 2, -1, 2, -2, 2);
      COST_MV_X4 (-2, 1, -2, 0, -2, -1, -2, -2);
      COST_MV_X4 (-1, -2, 0, -2, 1, -2, 2, -2);

      // hexagon grid 
      omx = bmx;
      omy = bmy;
      for (i = 1; i <= i_me_range / 4; i++)
      {
        static const int hex4[16][2] = {
          {-4, 2}, {-4, 1}, {-4, 0}, {-4, -1}, {-4, -2},
          {4, -2}, {4, -1}, {4, 0}, {4, 1}, {4, 2},
          {2, 3}, {0, 4}, {-2, 3},
          {-2, -3}, {0, -4}, {2, -3},
        };

        if (4 * i > XAVS_MIN4 (mv_x_max - omx, omx - mv_x_min, mv_y_max - omy, omy - mv_y_min))
        {
          for (j = 0; j < 16; j++)
          {
            int mx = omx + hex4[j][0] * i;
            int my = omy + hex4[j][1] * i;
            if (mx >= mv_x_min && mx <= mv_x_max && my >= mv_y_min && my <= mv_y_max)
              COST_MV (mx, my);
          }
        }
        else
        {
          COST_MV_X4 (-4 * i, 2 * i, -4 * i, 1 * i, -4 * i, 0 * i, -4 * i, -1 * i);
          COST_MV_X4 (-4 * i, -2 * i, 4 * i, -2 * i, 4 * i, -1 * i, 4 * i, 0 * i);
          COST_MV_X4 (4 * i, 1 * i, 4 * i, 2 * i, 2 * i, 3 * i, 0 * i, 4 * i);
          COST_MV_X4 (-2 * i, 3 * i, -2 * i, -3 * i, 0 * i, -4 * i, 2 * i, -3 * i);
        }
      }
      goto me_hex2;
    }

  case XAVS_ME_ESA:
    {
      const int min_x = XAVS_MAX (bmx - i_me_range, mv_x_min);
      const int min_y = XAVS_MAX (bmy - i_me_range, mv_y_min);
      const int max_x = XAVS_MIN (bmx + i_me_range, mv_x_max);
      const int max_y = XAVS_MIN (bmy + i_me_range, mv_y_max);
      int mx, my;
#if 0
      // plain old exhaustive search 
      for (my = min_y; my <= max_y; my++)
        for (mx = min_x; mx <= max_x; mx++)
          COST_MV (mx, my);
#else
      // successive elimination by comparing DC before a full SAD,
      // because sum(abs(diff)) >= abs(diff(sum)). 
      const int stride = m->i_stride[0];
      const int dw = xavs_pixel_size[i_pixel].w;
      const int dh = xavs_pixel_size[i_pixel].h * stride;
      static uint8_t zero[16 * 16] = { 0, };
      const int enc_dc = h->pixf.sad[i_pixel] (m->p_fenc[0], FENC_STRIDE, zero, 16);
      const uint16_t *integral_base = &m->integral[-1 - 1 * stride];

      for (my = min_y; my <= max_y; my++)
      {
        int mvs[3], i_mvs = 0;
        for (mx = min_x; mx <= max_x; mx++)
        {
          const uint16_t *integral = &integral_base[mx + my * stride];
          const uint16_t ref_dc = integral[0] + integral[dh + dw] - integral[dw] - integral[dh];
          const int bsad = bcost - BITS_MVD (mx, my);
          if (abs (ref_dc - enc_dc) < bsad)
          {
            if (i_mvs == 3)
            {
              COST_MV_X4_ABS (mvs[0], my, mvs[1], my, mvs[2], my, mx, my);
              i_mvs = 0;
            }
            else
              mvs[i_mvs++] = mx;
          }
        }
        for (i = 0; i < i_mvs; i++)
          COST_MV (mvs[i], my);
      }
#endif
    }
    break;
  }

  // -> qpel mv 
  m->mv[0] = bmx << 2;
  m->mv[1] = bmy << 2;

  // compute the real cost 
  m->cost_mv = p_cost_mvx[m->mv[0]] + p_cost_mvy[m->mv[1]];
  m->cost = bcost;
  if (bmx == pmx && bmy == pmy)
    m->cost += m->cost_mv;

  // subpel refine 
  if (h->mb.i_subpel_refine >= 2)
  {
    int hpel = subpel_iterations[h->mb.i_subpel_refine][2];
    int qpel = subpel_iterations[h->mb.i_subpel_refine][3];
    refine_subpel (h, m, hpel, qpel, p_halfpel_thresh, 0);
  }
}

#undef COST_MV




void
xavs_me_refine_qpel (xavs_t * h, xavs_me_t * m)
{
  int hpel = subpel_iterations[h->mb.i_subpel_refine][0];
  int qpel = subpel_iterations[h->mb.i_subpel_refine][1];

  if (m->i_pixel <= PIXEL_8x8 && h->sh.i_type == SLICE_TYPE_P)
    m->cost -= m->i_ref_cost;

  refine_subpel (h, m, hpel, qpel, NULL, 1);

  m->mv[0] = xavs_clip3 (m->mv[0], h->mb.mv_min[0], h->mb.mv_max[0]);
  m->mv[1] = xavs_clip3 (m->mv[1], h->mb.mv_min[1], h->mb.mv_max[1]);
}

#define COST_MV_SAD( mx, my ) \
{ \
    int stride = 16; \
    uint8_t *src = h->mc.get_ref( m->p_fref, m->i_stride[0], pix[0], &stride, mx, my, bw, bh ); \
    int cost = h->pixf.sad[i_pixel]( m->p_fenc[0], FENC_STRIDE, src, stride ) \
             + p_cost_mvx[ mx ] + p_cost_mvy[ my ]; \
    COPY3_IF_LT( bcost, cost, bmx, mx, bmy, my ); \
}

#define COST_MV_SATD( mx, my, dir ) \
if( b_refine_qpel || (dir^1) != odir ) \
{ \
    int stride = 16; \
    uint8_t *src = h->mc.get_ref( m->p_fref, m->i_stride[0], pix[0], &stride, mx, my, bw, bh ); \
    int cost = h->pixf.mbcmp[i_pixel]( m->p_fenc[0], FENC_STRIDE, src, stride ) \
             + p_cost_mvx[ mx ] + p_cost_mvy[ my ]; \
    if( b_chroma_me && cost < bcost ) \
    { \
        h->mc.mc_chroma( m->p_fref[4], m->i_stride[1], pix[0], 8, mx, my, bw/2, bh/2 ); \
        cost += h->pixf.mbcmp[i_pixel+3]( m->p_fenc[1], FENC_STRIDE, pix[0], 8 ); \
        if( cost < bcost ) \
        { \
            h->mc.mc_chroma( m->p_fref[5], m->i_stride[1], pix[0], 8, mx, my, bw/2, bh/2 ); \
            cost += h->pixf.mbcmp[i_pixel+3]( m->p_fenc[2], FENC_STRIDE, pix[0], 8 ); \
        } \
    } \
    if( cost < bcost ) \
    {                  \
        bcost = cost;  \
        bmx = mx;      \
        bmy = my;      \
        bdir = dir;    \
    } \
}

static void
refine_subpel (xavs_t * h, xavs_me_t * m, int hpel_iters, int qpel_iters, int *p_halfpel_thresh, int b_refine_qpel)
{
  const int bw = xavs_pixel_size[m->i_pixel].w;
  const int bh = xavs_pixel_size[m->i_pixel].h;
  const int16_t *p_cost_mvx = m->p_cost_mv - m->mvp[0];
  const int16_t *p_cost_mvy = m->p_cost_mv - m->mvp[1];
  const int i_pixel = m->i_pixel;
  const int b_chroma_me = h->mb.b_chroma_me && i_pixel <= PIXEL_8x8;

  DECLARE_ALIGNED (uint8_t, pix[4][16 * 16], 16);
  int omx, omy;
  int i;

  int bmx = m->mv[0];
  int bmy = m->mv[1];
  int bcost = m->cost;
  int odir = -1, bdir;


  /* try the subpel component of the predicted mv */
  if (hpel_iters)
  {
    int mx = xavs_clip3 (m->mvp[0], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
    int my = xavs_clip3 (m->mvp[1], h->mb.mv_min_spel[1], h->mb.mv_max_spel[1]);
    if (mx != bmx || my != bmy)
      COST_MV_SAD (mx, my);
  }

  /* halfpel diamond search */
  for (i = hpel_iters; i > 0; i--)
  {
    int omx = bmx, omy = bmy;
    int costs[4];
    int stride = 16;            // candidates are either all hpel or all qpel, so one stride is enough
    uint8_t *src0, *src1, *src2, *src3;
    src0 = h->mc.get_ref (m->p_fref, m->i_stride[0], pix[0], &stride, omx, omy - 2, bw, bh);
    src2 = h->mc.get_ref (m->p_fref, m->i_stride[0], pix[2], &stride, omx - 2, omy, bw, bh);
    if ((omx | omy) & 1)
    {
      src1 = h->mc.get_ref (m->p_fref, m->i_stride[0], pix[1], &stride, omx, omy + 2, bw, bh);
      src3 = h->mc.get_ref (m->p_fref, m->i_stride[0], pix[3], &stride, omx + 2, omy, bw, bh);
    }
    else
    {
      src1 = src0 + stride;
      src3 = src2 + 1;
    }
    h->pixf.sad_x4[i_pixel] (m->p_fenc[0], src0, src1, src2, src3, stride, costs);
    COPY2_IF_LT (bcost, costs[0] + p_cost_mvx[omx] + p_cost_mvy[omy - 2], bmy, omy - 2);
    COPY2_IF_LT (bcost, costs[1] + p_cost_mvx[omx] + p_cost_mvy[omy + 2], bmy, omy + 2);
    COPY3_IF_LT (bcost, costs[2] + p_cost_mvx[omx - 2] + p_cost_mvy[omy], bmx, omx - 2, bmy, omy);
    COPY3_IF_LT (bcost, costs[3] + p_cost_mvx[omx + 2] + p_cost_mvy[omy], bmx, omx + 2, bmy, omy);
    if (bmx == omx && bmy == omy)
      break;
  }

  if (!b_refine_qpel)
  {
    bcost = COST_MAX;
    COST_MV_SATD (bmx, bmy, -1);
  }

  /* early termination when examining multiple reference frames */
  if (p_halfpel_thresh)
  {
    if ((bcost * 7) >> 3 > *p_halfpel_thresh)
    {
      m->cost = bcost;
      m->mv[0] = bmx;
      m->mv[1] = bmy;
      // don't need cost_mv
      return;
    }
    else if (bcost < *p_halfpel_thresh)
      *p_halfpel_thresh = bcost;
  }

  /* quarterpel diamond search */
  bdir = -1;
  for (i = qpel_iters; i > 0; i--)
  {
    odir = bdir;
    omx = bmx;
    omy = bmy;
    COST_MV_SATD (omx, omy - 1, 0);
    COST_MV_SATD (omx, omy + 1, 1);
    COST_MV_SATD (omx - 1, omy, 2);
    COST_MV_SATD (omx + 1, omy, 3);
    if (bmx == omx && bmy == omy)
      break;
  }

  m->cost = bcost;
  m->mv[0] = bmx;
  m->mv[1] = bmy;
  m->cost_mv = p_cost_mvx[bmx] + p_cost_mvy[bmy];
}

#define BIME_CACHE( dx, dy ) \
{ \
    int i = 4 + 3*dx + dy; \
    h->mc.mc_luma( m0->p_fref, m0->i_stride[0], pix0[i], bw, om0x+dx, om0y+dy, bw, bh ); \
    h->mc.mc_luma( m1->p_fref, m1->i_stride[0], pix1[i], bw, om1x+dx, om1y+dy, bw, bh ); \
}

#define BIME_CACHE2(a,b) \
    BIME_CACHE(a,b) \
    BIME_CACHE(-(a),-(b))

#define COST_BIMV_SATD( m0x, m0y, m1x, m1y ) \
if( pass == 0 || !visited[(m0x)&7][(m0y)&7][(m1x)&7][(m1y)&7] ) \
{ \
    int cost; \
    int i0 = 4 + 3*(m0x-om0x) + (m0y-om0y); \
    int i1 = 4 + 3*(m1x-om1x) + (m1y-om1y); \
    visited[(m0x)&7][(m0y)&7][(m1x)&7][(m1y)&7] = 1; \
    memcpy( pix, pix0[i0], bs ); \
    if( i_weight == 32 ) \
        h->mc.avg[i_pixel]( pix, bw, pix1[i1], bw ); \
    else \
        h->mc.avg_weight[i_pixel]( pix, bw, pix1[i1], bw, i_weight ); \
    cost = h->pixf.mbcmp[i_pixel]( m0->p_fenc[0], FENC_STRIDE, pix, bw ) \
         + p_cost_m0x[ m0x ] + p_cost_m0y[ m0y ] \
         + p_cost_m1x[ m1x ] + p_cost_m1y[ m1y ]; \
    if( cost < bcost ) \
    {                  \
        bcost = cost;  \
        bm0x = m0x;    \
        bm0y = m0y;    \
        bm1x = m1x;    \
        bm1y = m1y;    \
    } \
}

#define CHECK_BIDIR(a,b,c,d) \
    COST_BIMV_SATD(om0x+a, om0y+b, om1x+c, om1y+d)

#define CHECK_BIDIR2(a,b,c,d) \
    CHECK_BIDIR(a,b,c,d) \
    CHECK_BIDIR(-(a),-(b),-(c),-(d))

#define CHECK_BIDIR8(a,b,c,d) \
    CHECK_BIDIR2(a,b,c,d) \
    CHECK_BIDIR2(b,c,d,a) \
    CHECK_BIDIR2(c,d,a,b) \
    CHECK_BIDIR2(d,a,b,c)

int
xavs_me_refine_bidir (xavs_t * h, xavs_me_t * m0, xavs_me_t * m1, int i_weight)
{
  const int i_pixel = m0->i_pixel;
  const int bw = xavs_pixel_size[i_pixel].w;
  const int bh = xavs_pixel_size[i_pixel].h;
  const int bs = bw * bh;
  const int16_t *p_cost_m0x = m0->p_cost_mv - xavs_clip3 (m0->mvp[0], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
  const int16_t *p_cost_m0y = m0->p_cost_mv - xavs_clip3 (m0->mvp[1], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
  const int16_t *p_cost_m1x = m1->p_cost_mv - xavs_clip3 (m1->mvp[0], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
  const int16_t *p_cost_m1y = m1->p_cost_mv - xavs_clip3 (m1->mvp[1], h->mb.mv_min_spel[0], h->mb.mv_max_spel[0]);
  DECLARE_ALIGNED (uint8_t, pix0[9][16 * 16], 16);
  DECLARE_ALIGNED (uint8_t, pix1[9][16 * 16], 16);
  DECLARE_ALIGNED (uint8_t, pix[16 * 16], 16);
  int bm0x = m0->mv[0], om0x = bm0x;
  int bm0y = m0->mv[1], om0y = bm0y;
  int bm1x = m1->mv[0], om1x = bm1x;
  int bm1y = m1->mv[1], om1y = bm1y;
  int bcost = COST_MAX;
  int pass = 0;
  uint8_t visited[8][8][8][8];
  memset (visited, 0, sizeof (visited));

  BIME_CACHE (0, 0);
  CHECK_BIDIR (0, 0, 0, 0);

  for (pass = 0; pass < 8; pass++)
  {
    /* check all mv pairs that differ in at most 2 components from the current mvs. */
    /* doesn't do chroma ME. this probably doesn't matter, as the gains
     * from bidir ME are the same with and without chroma ME. */

    BIME_CACHE2 (1, 0);
    BIME_CACHE2 (0, 1);
    BIME_CACHE2 (1, 1);
    BIME_CACHE2 (1, -1);

    CHECK_BIDIR8 (0, 0, 0, 1);
    CHECK_BIDIR8 (0, 0, 1, 1);
    CHECK_BIDIR2 (0, 1, 0, 1);
    CHECK_BIDIR2 (1, 0, 1, 0);
    CHECK_BIDIR8 (0, 0, -1, 1);
    CHECK_BIDIR2 (0, -1, 0, 1);
    CHECK_BIDIR2 (-1, 0, 1, 0);

    if (om0x == bm0x && om0y == bm0y && om1x == bm1x && om1y == bm1y)
      break;

    om0x = bm0x;
    om0y = bm0y;
    om1x = bm1x;
    om1y = bm1y;
    BIME_CACHE (0, 0);
  }

  m0->mv[0] = bm0x;
  m0->mv[1] = bm0y;
  m1->mv[0] = bm1x;
  m1->mv[1] = bm1y;
  return bcost;
}

#undef  COST_MV_SATD
#define SATD_THRESH 17/16
#define COST_MV_SATD( mx, my, dst, avoid_mvp ) \
{ \
    if( !avoid_mvp || !(mx == pmx && my == pmy) ) \
    { \
        int stride = 16; \
        uint8_t *src = h->mc.get_ref( m->p_fref, m->i_stride[0], pix, &stride, mx, my, bw*4, bh*4 ); \
        dst = h->pixf.mbcmp[i_pixel]( m->p_fenc[0], FENC_STRIDE, src, stride ) \
            + p_cost_mvx[mx] + p_cost_mvy[my]; \
        COPY1_IF_LT( bsatd, dst ); \
    } \
    else \
        dst = COST_MAX; \
}
#define COST_MV_RD( mx, my, satd, do_dir, mdir ) \
{ \
    if( satd <= bsatd * SATD_THRESH ) \
    { \
        int cost; \
        cache_mv[0] = cache_mv2[0] = mx; \
        cache_mv[1] = cache_mv2[1] = my; \
        cost = xavs_rd_cost_part( h, i_lambda2, i4/4, m->i_pixel ); \
        COPY4_IF_LT( bcost, cost, bmx, mx, bmy, my, dir, do_dir?mdir:dir ); \
    } \
}

void
xavs_me_refine_qpel_rd (xavs_t * h, xavs_me_t * m, int i_lambda2, int i4, int i_list)
{
  // don't have to fill the whole mv cache rectangle
  static const int pixel_mv_offs[] = { 0, 4, 4 * 8, 0, 2, 2 * 8, 0 };
  int16_t *cache_mv = h->mb.cache.mv[i_list][xavs_scan8[i4]];
  int16_t *cache_mv2 = cache_mv + pixel_mv_offs[m->i_pixel];
  const int16_t *p_cost_mvx, *p_cost_mvy;
  const int bw = xavs_pixel_size[m->i_pixel].w >> 2;
  const int bh = xavs_pixel_size[m->i_pixel].h >> 2;

  const int i_pixel = m->i_pixel;

  //DECLARE_ALIGNED_16( uint8_t pix[16*16] );
  DECLARE_ALIGNED (uint8_t, pix[16 * 16], 16);
  int bcost = m->i_pixel == PIXEL_16x16 ? m->cost : COST_MAX;
  int bmx = m->mv[0];
  int bmy = m->mv[1];
  int omx, omy, pmx, pmy, i, j;
  unsigned bsatd;
  unsigned int satd = 0;
  int dir = -2;
  unsigned int satds[8];

  if (m->i_pixel != PIXEL_16x16 && i4 != 0)
    xavs_mb_predict_mv (h, i_list, i4, bw, m->mvp);
  pmx = m->mvp[0];
  pmy = m->mvp[1];
  p_cost_mvx = m->p_cost_mv - pmx;
  p_cost_mvy = m->p_cost_mv - pmy;
  COST_MV_SATD (bmx, bmy, bsatd, 0);
  COST_MV_RD (bmx, bmy, 0, 0, 0);

  // check the predicted mv 
  if ((bmx != pmx || bmy != pmy) && pmx >= h->mb.mv_min_spel[0] && pmx <= h->mb.mv_max_spel[0] && pmy >= h->mb.mv_min_spel[1] && pmy <= h->mb.mv_max_spel[1])
  {
    COST_MV_SATD (pmx, pmy, satd, 0);
    COST_MV_RD (pmx, pmy, satd, 0, 0);
    // The hex motion search is guaranteed to not repeat the center candidate,
    //so if pmv is chosen, set the "MV to avoid checking" to bmv instead. 
    if (bmx == pmx && bmy == pmy)
    {
      pmx = m->mv[0];
      pmy = m->mv[1];
    }
  }

  if (bmy < h->mb.mv_min_spel[1] + 3 || bmy > h->mb.mv_max_spel[1] - 3)
    return;

  // subpel hex search, same pattern as ME HEX.
  dir = -2;
  omx = bmx;
  omy = bmy;
  for (j = 0; j < 6; j++)
    COST_MV_SATD (omx + hex2[j + 1][0], omy + hex2[j + 1][1], satds[j], 1);
  for (j = 0; j < 6; j++)
    COST_MV_RD (omx + hex2[j + 1][0], omy + hex2[j + 1][1], satds[j], 1, j);

  if (dir != -2)
  {
    // half hexagon, not overlapping the previous iteration 
    for (i = 1; i < 10; i++)
    {
      const int odir = mod6m1[dir + 1];
      if (bmy < h->mb.mv_min_spel[1] + 3 || bmy > h->mb.mv_max_spel[1] - 3)
        break;
      dir = -2;
      omx = bmx;
      omy = bmy;
      for (j = 0; j < 3; j++)
        COST_MV_SATD (omx + hex2[odir + j][0], omy + hex2[odir + j][1], satds[j], 1);
      for (j = 0; j < 3; j++)
        COST_MV_RD (omx + hex2[odir + j][0], omy + hex2[odir + j][1], satds[j], 1, odir - 1 + j);
      if (dir == -2)
        break;
    }
  }

  //square refine, same as pattern as ME HEX. 
  omx = bmx;
  omy = bmy;
  for (i = 0; i < 8; i++)
    COST_MV_SATD (omx + square1[i + 1][0], omy + square1[i + 1][1], satds[i], 1);
  for (i = 0; i < 8; i++)
    COST_MV_RD (omx + square1[i + 1][0], omy + square1[i + 1][1], satds[i], 0, 0);

  m->cost = bcost;
  m->mv[0] = bmx;
  m->mv[1] = bmy;

  xavs_macroblock_cache_mv (h, block_idx_x[i4], block_idx_y[i4], bw, bh, i_list, bmx, bmy);
  xavs_macroblock_cache_mvd (h, block_idx_x[i4], block_idx_y[i4], bw, bh, i_list, bmx - m->mvp[0], bmy - m->mvp[1]);
}
