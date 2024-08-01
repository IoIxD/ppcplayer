/*****************************************************************************
 * osdep.h: h264 encoder
 *****************************************************************************
 * Copyright (C) 2007-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
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

#ifndef XAVS_OSDEP_H
#define XAVS_OSDEP_H

#include <stdio.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#ifdef _WIN32
#include <io.h>                 // _setmode()
#include <fcntl.h>              // _O_BINARY
#endif

#ifdef _MSC_VER
#define inline __inline
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define snprintf _snprintf
#define fseek _fseeki64
#define ftell _ftelli64
#define isfinite _finite
#define strtok_r strtok_s
#define XAVS_VERSION ""         // no configure script for msvc
#endif

#if (defined(SYS_OPENBSD) && !defined(isfinite)) || defined(SYS_SunOS)
#define isfinite finite
#endif
#if defined(_MSC_VER) || defined(SYS_SunOS) || defined(SYS_MACOSX)
#define sqrtf sqrt
#endif
#ifdef _WIN32
#define rename(src,dst) (unlink(dst), rename(src,dst))  // POSIX says that rename() removes the destination, but win32 doesn't.
#ifndef strtok_r
#define strtok_r(str,delim,save) strtok(str,delim)
#endif
#endif

#ifdef _MSC_VER
//#define DECLARE_ALIGNED( var, n ) __declspec(align(n)) var
#define DECLARE_ALIGNED( type, var, n ) __declspec(align(n)) type var
#else
//#define DECLARE_ALIGNED( var, n ) var __attribute__((aligned(n)))
#define DECLARE_ALIGNED( type, var, n ) type var __attribute__((aligned(n)))
#endif

#define DECLARE_ALIGNED_16( var ) DECLARE_ALIGNED( var, 16 )
#define DECLARE_ALIGNED_8( var )  DECLARE_ALIGNED( var, 8 )
#define DECLARE_ALIGNED_4( var )  DECLARE_ALIGNED( var, 4 )

#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#define UNUSED __attribute__((unused))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))
#define xavs_constant_p(x) __builtin_constant_p(x)
#else
#define UNUSED
#define ALWAYS_INLINE inline
#define NOINLINE
#define xavs_constant_p(x) 0
#endif

/* threads */
#if defined(SYS_BEOS)
#include <kernel/OS.h>
#define xavs_pthread_t               thread_id
static inline int
xavs_pthread_create (xavs_pthread_t * t, void *a, void *(*f) (void *), void *d)
{
  *t = spawn_thread (f, "", 10, d);
  if (*t < B_NO_ERROR)
    return -1;
  resume_thread (*t);
  return 0;
}

#define xavs_pthread_join(t,s)       { long tmp; \
                                       wait_for_thread(t,(s)?(long*)(*(s)):&tmp); }
#ifndef usleep
#define usleep(t)                    snooze(t)
#endif
#define HAVE_PTHREAD 1

#elif defined(__WIN32__)
#include <windows.h>
#define xavs_pthread_t   HANDLE
static inline int
xavs_pthread_create (xavs_pthread_t * t, void *a, LPVOID f, void *d)
{
	 *t = CreateThread(NULL,0,f,d,0,NULL);
	  if (*t == NULL)
		    return -1;
	   ResumeThread (*t);
	    return 0;
}
#define xavs_pthread_join(t,s)       {  WaitForSingleObject(t,INFINITE); \
	                                CloseHandle(t); }

#ifndef usleep
#define usleep(t)                    Sleep(t)
#endif
#define HAVE_PTHREAD 1

#elif defined(HAVE_PTHREAD)
#include <pthread.h>
#define USE_REAL_PTHREAD

#else
#define xavs_pthread_t               int
#define xavs_pthread_create(t,u,f,d) 0
#define xavs_pthread_join(t,s)
#endif //SYS_*

#ifdef USE_REAL_PTHREAD
#if defined(__WIN32__)
#pragma comment(lib, "pthreadVC2.lib")
#endif
#define xavs_pthread_t               pthread_t
#define xavs_pthread_create          pthread_create
#define xavs_pthread_join            pthread_join
#define xavs_pthread_mutex_t         pthread_mutex_t
#define xavs_pthread_mutex_init      pthread_mutex_init
#define xavs_pthread_mutex_destroy   pthread_mutex_destroy
#define xavs_pthread_mutex_lock      pthread_mutex_lock
#define xavs_pthread_mutex_unlock    pthread_mutex_unlock
#define xavs_pthread_cond_t          pthread_cond_t
#define xavs_pthread_cond_init       pthread_cond_init
#define xavs_pthread_cond_destroy    pthread_cond_destroy
#define xavs_pthread_cond_broadcast  pthread_cond_broadcast
#define xavs_pthread_cond_wait       pthread_cond_wait
#define x264_pthread_attr_t          pthread_attr_t
#define x264_pthread_attr_init       pthread_attr_init
#define x264_pthread_attr_destroy    pthread_attr_destroy
#define XAVS_PTHREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#else
#define xavs_pthread_mutex_t         int
#define xavs_pthread_mutex_init(m,f) 0
#define xavs_pthread_mutex_destroy(m)
#define xavs_pthread_mutex_lock(m)
#define xavs_pthread_mutex_unlock(m)
#define xavs_pthread_cond_t          int
#define xavs_pthread_cond_init(c,f)  0
#define xavs_pthread_cond_destroy(c)
#define xavs_pthread_cond_broadcast(c)
#define xavs_pthread_cond_wait(c,m)
#define xavs_pthread_attr_t          int
#define xavs_pthread_attr_init(a)    0
#define xavs_pthread_attr_destroy(a)
#define XAVS_PTHREAD_MUTEX_INITIALIZER 0
#endif

#define WORD_SIZE sizeof(void*)

#define asm __asm__

#if !defined(_WIN64) && !defined(__LP64__)
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define BROKEN_STACK_ALIGNMENT  /* define it if stack is not mod16 */
#endif
#endif

#if WORDS_BIGENDIAN
#define endian_fix(x) (x)
#define endian_fix64(x) (x)
#define endian_fix32(x) (x)
#define endian_fix16(x) (x)
#else
#if defined(__GNUC__) && defined(HAVE_MMX)
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("bswap %0":"+r"(x));
    return x;
}
#elif defined(__GNUC__) && HAVE_ARMV6
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("rev %0, %0":"+r"(x));
    return x;
}
#else
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    return (x<<24) + ((x<<8)&0xff0000) + ((x>>8)&0xff00) + (x>>24);
}
#endif
#if defined(__GNUC__) && defined(ARCH_X86_64)
static ALWAYS_INLINE uint64_t endian_fix64( uint64_t x )
{
    asm("bswap %0":"+r"(x));
    return x;
}
#else
static ALWAYS_INLINE uint64_t endian_fix64( uint64_t x )
{
    return endian_fix32(x>>32) + ((uint64_t)endian_fix32(x)<<32);
}
#endif
static ALWAYS_INLINE intptr_t endian_fix( intptr_t x )
{
    return WORD_SIZE == 8 ? endian_fix64(x) : endian_fix32(x);
}
static ALWAYS_INLINE uint16_t endian_fix16( uint16_t x )
{
    return (x<<8)|(x>>8);
}
#endif

#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 3)
#define xavs_clz(x) __builtin_clz(x)
#else
static int ALWAYS_INLINE
xavs_clz (uint32_t x)
{
  static uint8_t lut[16] = { 4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
  int y, z = ((x - 0x10000) >> 27) & 16;
  x >>= z ^ 16;
  z += y = ((x - 0x100) >> 28) & 8;
  x >>= y ^ 8;
  z += y = ((x - 0x10) >> 29) & 4;
  x >>= y ^ 4;
  return z + lut[x];
}
#endif

#endif /* XAVS_OSDEP_H */
