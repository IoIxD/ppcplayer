/*****************************************************************************
 * x264: h264 encoder testing program.
 * ***************************************************************************
 * Copyright (C) 2003-2008 x264 project
 * 
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Steven Walters <kemuri9@gmail.com>
 *          Kieran Kunhya <kieran@kunhya.com>
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
 * ***************************************************************************/

/*****************************************************************************
 * xavs: xavs encoder/decoder testing program.
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <signal.h>
#define _GNU_SOURCE
#include <getopt.h>

#ifdef _MSC_VER
#include <io.h>                 /* _setmode() */
#include <fcntl.h>              /* _O_BINARY */
#endif

#ifndef _MSC_VER
#include "config.h"
#endif

#include "common/common.h"
#include "common/osdep.h"
#include "xavs.h"
#include "muxers.h"

#ifdef _WIN32
#include <windows.h>
#else
#define SetConsoleTitle(t)
#endif
#define DATA_MAX 3000000
uint8_t data[DATA_MAX];

/* Ctrl-C handler */
static int b_ctrl_c = 0;
static int b_exit_on_ctrl_c = 0;
static void
SigIntHandler (int a)
{
  if (b_exit_on_ctrl_c)
    exit (0);
  b_ctrl_c = 1;
}

typedef struct
{
  int b_decompress;
  int b_progress;
  int i_seek;
  hnd_t hin;
  hnd_t hout;
  FILE *qpfile;
} cli_opt_t;

/* input file operation function pointers */
int (*p_open_infile) (char *psz_filename, hnd_t * p_handle, xavs_param_t * p_param);
int (*p_get_frame_total) (hnd_t handle);
int (*p_read_frame) (xavs_picture_t * p_pic, hnd_t handle, int i_frame);
int (*p_close_infile) (hnd_t handle);

/* output file operation function pointers */
static int (*p_open_outfile) (char *psz_filename, hnd_t * p_handle);
static int (*p_set_outfile_param) (hnd_t handle, xavs_param_t * p_param);
static int (*p_write_nalu) (hnd_t handle, uint8_t * p_nal, int i_size);
static int (*p_set_eop) (hnd_t handle, xavs_picture_t * p_picture);
static int (*p_close_outfile) (hnd_t handle);

static void Help (xavs_param_t * defaults, int longhelp);
static int Parse (int argc, char **argv, xavs_param_t * param, cli_opt_t * opt);
static int Encode (xavs_param_t * param, cli_opt_t * opt);


/****************************************************************************
 * main:
 ****************************************************************************/
int
main (int argc, char **argv)
{
  xavs_param_t param;
  cli_opt_t opt;
  int ret;

#ifdef PTW32_STATIC_LIB
  pthread_win32_process_attach_np ();
  pthread_win32_thread_attach_np ();
#endif

#ifdef _WIN32
  _setmode (_fileno (stdin), _O_BINARY);
  _setmode (_fileno (stdout), _O_BINARY);
#endif

  xavs_param_default (&param);

  /* Parse command line */
  if (Parse (argc, argv, &param, &opt) < 0)
    return -1;

  /* Control-C handler */
  signal (SIGINT, SigIntHandler);

  ret = Encode (&param, &opt);
#ifdef PTW32_STATIC_LIB
  pthread_win32_thread_detach_np ();
  pthread_win32_process_detach_np ();
#endif
  return ret;
}


static char const *
strtable_lookup (const char *const table[], int index)
{
  int i = 0;
  while (table[i])
    i++;
  return ((index >= 0 && index < i) ? table[index] : "???");
}

/*****************************************************************************
 * Help:
 *****************************************************************************/
static void
Help (xavs_param_t * defaults, int longhelp)
{
#define H0 printf
#define H1 if(longhelp>=1) printf
#define H2 if(longhelp==2) printf
  H0 ("xavs core:%d%s\n"
      "Syntax: xavs [options] -o outfile infile [widthxheight]\n"
      "\n"
      "Infile can be raw YUV 4:2:0 (in which case resolution is required),\n"
      "  or YUV4MPEG 4:2:0 (*.y4m),\n"
      "  or AVI or Avisynth if compiled with AVIS support (%s).\n"
      "Outfile type is selected by filename:\n"
      " .264 -> Raw bytestream\n"
      " .mkv -> Matroska\n"
      " .mp4 -> MP4 if compiled with GPAC support (%s)\n" "\n" "Options:\n" "\n" "  -h, --help                  List basic options\n" "      --longhelp              List more options\n" "      --fullhelp              List all options\n" "\n", XAVS_BUILD, XAVS_VERSION,
#ifdef AVIS_INPUT
      "yes",
#else
      "no",
#endif
#ifdef MP4_OUTPUT
      "yes"
#else
      "no"
#endif
    );
  H0 ("Example usage:\n");
  H0 ("\n");
  H0 ("      Constant quality mode:\n");
  H0 ("            xavs --crf 24 -o output input\n");
  H0 ("\n");
  H0 ("      Two-pass with a bitrate of 1000kbps:\n");
  H0 ("            xavs --pass 1 --bitrate 1000 -o output input\n");
  H0 ("            xavs --pass 2 --bitrate 1000 -o output input\n");
  H0 ("\n");
  H0 ("      Lossless:\n");
  H0 ("            xavs --crf 0 -o output input\n");
  H0 ("\n");
  H0 ("      Maximum PSNR at the cost of speed and visual quality:\n");
  H0 ("            xavs --preset placebo --tune psnr -o output input\n");
  H0 ("\n");
  H0 ("      Constant bitrate at 1000kbps with a 2 second-buffer:\n");
  H0 ("            xavs --vbv-bufsize 2000 --bitrate 1000 -o output input\n");
  H0 ("\n");
  H0 ("Presets:\n");
  H0 ("\n");
  H0 ("      --preset                Use a preset to select encoding settings [medium]\n");
  H0 ("                                  Overridden by user settings\n");
  H0 ("                                  - ultrafast,veryfast,faster,fast,medium\n" "                                  - slow,slower,veryslow,placebo\n");
  H0 ("      --tune                  Tune the settings for a particular type of source\n");
  H0 ("                                  Overridden by user settings\n");
  H2 ("                                  - film,animation,grain,psnr,ssim\n" "                                  - fastdecode,touhou\n");
  else
  H0 ("                                  - film,animation,grain,psnr,ssim,fastdecode\n");
  H1 ("      --slow-firstpass        Don't use faster settings with --pass 1\n");
  H0 ("\n");
  H0 ("Frame-type options:\n");
  H0 ("\n");
  H0 ("  -I, --keyint <integer>      Maximum GOP size [%d]\n", defaults->i_keyint_max);
  H2 ("  -i, --min-keyint <integer>  Minimum GOP size [%d]\n", defaults->i_keyint_min);
  H2 ("      --no-scenecut           Disable adaptive I-frame decision\n");
  H2 ("      --scenecut <integer>    How aggressively to insert extra I-frames [%d]\n", defaults->i_scenecut_threshold);
  H1 ("  -b, --bframes <integer>     Number of B-frames between I and P [%d]\n", defaults->i_bframe);
  H1 ("      --b-adapt               Adaptive B-frame decision method [%d]\n"
      "                                  Higher values may lower threading efficiency.\n"
      "                                  - 0: Disabled\n" "                                  - 1: Fast\n" "                                  - 2: Optimal (slow with high --bframes)\n", defaults->i_bframe_adaptive);
  H2 ("      --b-bias <integer>      Influences how often B-frames are used [%d]\n", defaults->i_bframe_bias);
  H1 ("      --b-pyramid             Keep some B-frames as references\n");
  H1 ("      --no-cabac              Disable CABAC\n");
  H1 ("  -r, --ref <integer>         Number of reference frames [%d]\n", defaults->i_frame_reference);
  H1 ("      --no-deblock            Disable loop filter\n");
  H1 ("  -f, --deblock <alpha:beta>  Loop filter parameters [%d:%d]\n", defaults->i_deblocking_filter_alphac0, defaults->i_deblocking_filter_beta);
  H2 ("      --slices <integer>      Number of slices per frame; forces rectangular\n" "                              slices and is overridden by other slicing options\n");
  else
  H1 ("      --slices <integer>      Number of slices per frame\n");
  H2 ("      --slice-max-size <integer> Limit the size of each slice in bytes\n");
  H2 ("      --slice-max-mbs <integer> Limit the size of each slice in macroblocks\n");
  H0 ("      --interlaced            Enable pure-interlaced mode\n");
  H2 ("      --constrained-intra     Enable constrained intra prediction.\n");
  H0 ("\n");
  H0 ("Ratecontrol:\n");
  H0 ("\n");
  H1 ("  -q, --qp <integer>          Force constant QP (0-63, 0=lossless)\n");
  H0 ("  -B, --bitrate <integer>     Set bitrate (kbit/s)\n");
  H0 ("      --crf <float>           Quality-based VBR (0-63, 0=lossless) [%.1f]\n", defaults->rc.f_rf_constant);
  H1 ("      --rc-lookahead <integer> Number of frames for frametype lookahead [%d]\n", defaults->rc.i_lookahead);
  H0 ("      --vbv-maxrate <integer> Max local bitrate (kbit/s) [%d]\n", defaults->rc.i_vbv_max_bitrate);
  H0 ("      --vbv-bufsize <integer> Set size of the VBV buffer (kbit) [%d]\n", defaults->rc.i_vbv_buffer_size);
  H2 ("      --vbv-init <float>      Initial VBV buffer occupancy [%.1f]\n", defaults->rc.f_vbv_buffer_init);
  H2 ("      --qpmin <integer>       Set min QP [%d]\n", defaults->rc.i_qp_min);
  H2 ("      --qpmax <integer>       Set max QP [%d]\n", defaults->rc.i_qp_max);
  H2 ("      --qpstep <integer>      Set max QP step [%d]\n", defaults->rc.i_qp_step);
  H2 ("      --ratetol <float>       Tolerance of ABR ratecontrol and VBV [%.1f]\n", defaults->rc.f_rate_tolerance);
  H2 ("      --ipratio <float>       QP factor between I and P [%.2f]\n", defaults->rc.f_ip_factor);
  H2 ("      --pbratio <float>       QP factor between P and B [%.2f]\n", defaults->rc.f_pb_factor);
  H2 ("      --chroma-qp-offset <integer>  QP difference between chroma and luma [%d]\n", defaults->analyse.i_chroma_qp_offset);
  H2 ("      --aq-mode <integer>     AQ method [%d]\n" "                                  - 0: Disabled\n" "                                  - 1: Variance AQ (complexity mask)\n" "                                  - 2: Auto-variance AQ (experimental)\n", defaults->rc.i_aq_mode);
  H1 ("      --aq-strength <float>   Reduces blocking and blurring in flat and\n" "                              textured areas. [%.1f]\n", defaults->rc.f_aq_strength);
  H1 ("\n");
  H2 ("  -p, --pass <1|2|3>          Enable multipass ratecontrol\n");
  else
  H0 ("  -p, --pass <1|2>            Enable multipass ratecontrol\n");
  H0 ("                                  - 1: First pass, creates stats file\n" "                                  - 2: Last pass, does not overwrite stats file\n");
  H2 ("                                  - 3: Nth pass, overwrites stats file\n");
  H1 ("      --stats <string>        Filename for 2 pass stats [\"%s\"]\n", defaults->rc.psz_stat_out);
  H2 ("      --no-mbtree             Disable mb-tree ratecontrol.\n");
  H2 ("      --qcomp <float>         QP curve compression [%.2f]\n", defaults->rc.f_qcompress);
  H2 ("      --cplxblur <float>      Reduce fluctuations in QP (before curve compression) [%.1f]\n", defaults->rc.f_complexity_blur);
  H2 ("      --qblur <float>         Reduce fluctuations in QP (after curve compression) [%.1f]\n", defaults->rc.f_qblur);
  H2 ("      --zones <zone0>/<zone1>/...  Tweak the bitrate of regions of the video\n");
  H2 ("                              Each zone is of the form\n"
      "                                  <start frame>,<end frame>,<option>\n"
      "                                  where <option> is either\n" "                                      q=<integer> (force QP)\n" "                                  or  b=<float> (bitrate multiplier)\n");
  H2 ("      --qpfile <string>       Force frametypes and QPs for some or all frames\n"
      "                              Format of each line: framenumber frametype QP\n" "                              QP of -1 lets xavs choose. Frametypes: I,i,P,B,b.\n" "                              QPs are restricted by qpmin/qpmax.\n");
  H1 ("\n");
  H1 ("Analysis:\n");
  H1 ("\n");
  H1 ("  -A, --partitions <string>   Partitions to consider [\"p8x8,b8x8,i8x8,i4x4\"]\n"
      "                                  - p8x8, p4x4, b8x8, i8x8, i4x4\n" "                                  - none, all\n" "                                  (p4x4 requires p8x8. i8x8 requires --8x8dct.)\n");
  H1 ("      --direct <string>       Direct MV prediction mode [\"%s\"]\n" "                                  - none, spatial, temporal, auto\n", strtable_lookup (xavs_direct_pred_names, defaults->analyse.i_direct_mv_pred));
  H2 ("      --no-weightb            Disable weighted prediction for B-frames\n");
  H1 ("      --me <string>           Integer pixel motion estimation method [\"%s\"]\n", strtable_lookup (xavs_motion_est_names, defaults->analyse.i_me_method));
  H2 ("                                  - dia: diamond search, radius 1 (fast)\n"
      "                                  - hex: hexagonal search, radius 2\n"
      "                                  - umh: uneven multi-hexagon search\n" "                                  - esa: exhaustive search\n" "                                  - tesa: hadamard exhaustive search (slow)\n");
  else
  H1 ("                                  - dia, hex, umh\n");
  H2 ("      --merange <integer>     Maximum motion vector search range [%d]\n", defaults->analyse.i_me_range);
  H2 ("      --mvrange <integer>     Maximum motion vector length [-1 (auto)]\n");
  H2 ("      --mvrange-thread <int>  Minimum buffer between threads [-1 (auto)]\n");
  H1 ("  -m, --subme <integer>       Subpixel motion estimation and mode decision [%d]\n", defaults->analyse.i_subpel_refine);
  H2 ("                                  - 0: fullpel only (not recommended)\n"
      "                                  - 1: SAD mode decision, one qpel iteration\n"
      "                                  - 2: SATD mode decision\n"
      "                                  - 3-5: Progressively more qpel\n"
      "                                  - 6: RD mode decision for I/P-frames\n"
      "                                  - 7: RD mode decision for all frames\n"
      "                                  - 8: RD refinement for I/P-frames\n" "                                  - 9: RD refinement for all frames\n" "                                  - 10: QP-RD - requires trellis=2, aq-mode>0\n");
  else
  H1 ("                                  decision quality: 1=fast, 10=best.\n");
  H1 ("      --psy-rd                Strength of psychovisual optimization [\"%.1f:%.1f\"]\n"
      "                                  #1: RD (requires subme>=6)\n" "                                  #2: Trellis (requires trellis, experimental)\n", defaults->analyse.f_psy_rd, defaults->analyse.f_psy_trellis);
  H2 ("      --no-psy                Disable all visual optimizations that worsen\n" "                              both PSNR and SSIM.\n");
  H2 ("      --no-mixed-refs         Don't decide references on a per partition basis\n");
  H2 ("      --no-chroma-me          Ignore chroma in motion estimation\n");
  H1 ("      --no-8x8dct             Disable adaptive spatial transform size\n");
  H1 ("  -t, --trellis <integer>     Trellis RD quantization. Requires CABAC. [%d]\n"
      "                                  - 0: disabled\n" "                                  - 1: enabled only on the final encode of a MB\n" "                                  - 2: enabled on all mode decisions\n", defaults->analyse.i_trellis);
  H2 ("      --no-fast-pskip         Disables early SKIP detection on P-frames\n");
  H2 ("      --no-dct-decimate       Disables coefficient thresholding on P-frames\n");
  H1 ("      --nr <integer>          Noise reduction [%d]\n", defaults->analyse.i_noise_reduction);
  H2 ("\n");
  H2 ("      --deadzone-inter <int>  Set the size of the inter luma quantization deadzone [%d]\n", defaults->analyse.i_luma_deadzone[0]);
  H2 ("      --deadzone-intra <int>  Set the size of the intra luma quantization deadzone [%d]\n", defaults->analyse.i_luma_deadzone[1]);
  H2 ("                                  Deadzones should be in the range 0 - 32.\n");
  H2 ("      --cqm <string>          Preset quant matrices [\"flat\"]\n" "                                  - jvt, flat\n");
  H1 ("      --cqmfile <string>      Read custom quant matrices from a JM-compatible file\n");
  H2 ("                                  Overrides any other --cqm* options.\n");
  H2 ("      --cqm4 <list>           Set all 4x4 quant matrices\n" "                                  Takes a comma-separated list of 16 integers.\n");
  H2 ("      --cqm8 <list>           Set all 8x8 quant matrices\n" "                                  Takes a comma-separated list of 64 integers.\n");
  H2 ("      --cqm4i, --cqm4p, --cqm8i, --cqm8p\n" "                              Set both luma and chroma quant matrices\n");
  H2 ("      --cqm4iy, --cqm4ic, --cqm4py, --cqm4pc\n" "                              Set individual quant matrices\n");
  H2 ("\n");
  H2 ("Video Usability Info (Annex E):\n");
  H2 ("The VUI settings are not used by the encoder but are merely suggestions to\n");
  H2 ("the playback equipment. See doc/vui.txt for details. Use at your own risk.\n");
  H2 ("\n");
  H2 ("      --overscan <string>     Specify crop overscan setting [\"%s\"]\n" "                                  - undef, show, crop\n", strtable_lookup (xavs_overscan_names, defaults->vui.i_overscan));
  H2 ("      --videoformat <string>  Specify video format [\"%s\"]\n" "                                  - component, pal, ntsc, secam, mac, undef\n", strtable_lookup (xavs_vidformat_names, defaults->vui.i_vidformat));
  H2 ("      --fullrange <string>    Specify full range samples setting [\"%s\"]\n" "                                  - off, on\n", strtable_lookup (xavs_fullrange_names, defaults->vui.b_fullrange));
  H2 ("      --colorprim <string>    Specify color primaries [\"%s\"]\n" "                                  - undef, bt709, bt470m, bt470bg\n" "                                    smpte170m, smpte240m, film\n", strtable_lookup (xavs_colorprim_names, defaults->vui.i_colorprim));
  H2 ("      --transfer <string>     Specify transfer characteristics [\"%s\"]\n"
      "                                  - undef, bt709, bt470m, bt470bg, linear,\n" "                                    log100, log316, smpte170m, smpte240m\n", strtable_lookup (xavs_transfer_names, defaults->vui.i_transfer));
  H2 ("      --colormatrix <string>  Specify color matrix setting [\"%s\"]\n"
      "                                  - undef, bt709, fcc, bt470bg\n" "                                    smpte170m, smpte240m, GBR, YCgCo\n", strtable_lookup (xavs_colmatrix_names, defaults->vui.i_colmatrix));
  H2 ("      --chromaloc <integer>   Specify chroma sample location (0 to 5) [%d]\n", defaults->vui.i_chroma_loc);
  H0 ("\n");
  H0 ("Input/Output:\n");
  H0 ("\n");
  H0 ("  -o, --output                Specify output file\n");
  H0 ("      --sar width:height      Specify Sample Aspect Ratio\n");
  H0 ("      --fps <float|rational>  Specify framerate\n");
  H0 ("      --seek <integer>        First frame to encode\n");
  H0 ("      --frames <integer>      Maximum number of frames to encode\n");
  H0 ("      --level <string>        Specify level (as defined by Annex A)\n");
  H1 ("\n");
  H1 ("  -v, --verbose               Print stats for each frame\n");
  H1 ("      --no-progress           Don't show the progress indicator while encoding\n");
  H0 ("      --quiet                 Quiet Mode\n");
  H1 ("      --psnr                  Enable PSNR computation\n");
  H1 ("      --ssim                  Enable SSIM computation\n");
  H1 ("      --threads <integer>     Force a specific number of threads\n");
  H1 ("      --sliced-thread <integer>     Force a specific number of threads for slices\n");
  H2 ("      --thread-input          Run Avisynth in its own thread\n");
  H2 ("      --sync-lookahead <integer> Number of buffer frames for threaded lookahead\n");
  H2 ("      --non-deterministic     Slightly improve quality of SMP, at the cost of repeatability\n");
  H2 ("      --asm <integer>         Override CPU detection\n");
  H2 ("      --no-asm                Disable all CPU optimizations\n");
  H2 ("      --visualize             Show MB types overlayed on the encoded video\n");
  H2 ("      --dump-yuv <string>     Save reconstructed frames\n");
  H2 ("      --sps-id <integer>      Set SPS and PPS id numbers [%d]\n", defaults->i_sps_id);
  H2 ("      --aud                   Use access unit delimiters\n");
  H0 ("\n");
}

static int
parse_enum (const char *arg, const char *const *names, int *dst)
{
  int i;
  for (i = 0; names[i]; i++)
    if (!strcmp (arg, names[i]))
    {
      *dst = i;
      return 0;
    }
  return -1;
}

static int
parse_cqm (const char *str, uint8_t * cqm, int length)
{
  int i = 0;
  do
  {
    int coef;
    if (!sscanf (str, "%d", &coef) || coef < 1 || coef > 255)
      return -1;
    cqm[i++] = coef;
  }
  while (i < length && (str = strchr (str, ',')) && str++);
  return (i == length) ? 0 : -1;
}

static int
xavs_atobool (const char *str, int *b_error)
{
  if (!strcmp (str, "1") || !strcmp (str, "true") || !strcmp (str, "yes"))
    return 1;
  if (!strcmp (str, "0") || !strcmp (str, "false") || !strcmp (str, "no"))
    return 0;
  *b_error = 1;
  return 0;
}

static int
xavs_atoi (const char *str, int *b_error)
{
  char *end;
  int v = strtol (str, &end, 0);
  if (end == str || *end != '\0')
    *b_error = 1;
  return v;
}

static double
xavs_atof (const char *str, int *b_error)
{
  char *end;
  double v = strtod (str, &end);
  if (end == str || *end != '\0')
    *b_error = 1;
  return v;
}

#define atobool(str) ( name_was_bool = 1, xavs_atobool( str, &b_error ) )
#define atoi(str) xavs_atoi( str, &b_error )
#define atof(str) xavs_atof( str, &b_error )

int
xavs_param_parse (xavs_param_t * p, const char *name, const char *value)
{
  char *name_buf = NULL;
  int b_error = 0;
  int name_was_bool;
  int value_was_null = !value;
  int i;

  if (!name)
    return XAVS_PARAM_BAD_NAME;
  if (!value)
    value = "true";

  if (value[0] == '=')
    value++;

  if (strchr (name, '_'))       // s/_/-/g
  {
    char *p;
    name_buf = strdup (name);
    while ((p = strchr (name_buf, '_')))
      *p = '-';
    name = name_buf;
  }

  if ((!strncmp (name, "no-", 3) && (i = 3)) || (!strncmp (name, "no", 2) && (i = 2)))
  {
    name += i;
    value = atobool (value) ? "false" : "true";
  }
  name_was_bool = 0;

#define OPT(STR) else if( !strcmp( name, STR ) )
#define OPT2(STR0, STR1) else if( !strcmp( name, STR0 ) || !strcmp( name, STR1 ) )
  if (0);
  OPT ("sar")
  {
    b_error = (2 != sscanf (value, "%d:%d", &p->vui.i_sar_width, &p->vui.i_sar_height) && 2 != sscanf (value, "%d/%d", &p->vui.i_sar_width, &p->vui.i_sar_height));
  }
  OPT ("chromaloc")
  {
    p->vui.i_chroma_loc = atoi (value);
    b_error = (p->vui.i_chroma_loc < 0 || p->vui.i_chroma_loc > 5);
  }
  OPT ("fps")
  {
    if (sscanf (value, "%d/%d", &p->i_fps_num, &p->i_fps_den) == 2)
      ;
    else
    {
      float fps = atof (value);
      p->i_fps_num = (int) (fps * 1000 + .5);
      p->i_fps_den = 1000;
    }
  }
  OPT2 ("ref", "frameref") p->i_frame_reference = atoi (value);
  OPT ("keyint")
  {
    p->i_keyint_max = atoi (value);
    if (p->i_keyint_min > p->i_keyint_max)
      p->i_keyint_min = p->i_keyint_max;
  }
  OPT2 ("min-keyint", "keyint-min")
  {
    p->i_keyint_min = atoi (value);
    if (p->i_keyint_max < p->i_keyint_min)
      p->i_keyint_max = p->i_keyint_min;
  }
  OPT ("scenecut")
  {
    p->i_scenecut_threshold = atobool (value);
    if (b_error || p->i_scenecut_threshold)
    {
      b_error = 0;
      p->i_scenecut_threshold = atoi (value);
    }
  }
  OPT ("bframes") p->i_bframe = atoi (value);
  OPT ("b-adapt")
  {
    p->i_bframe_adaptive = atobool (value);
    if (b_error)
    {
      b_error = 0;
      p->i_bframe_adaptive = atoi (value);
    }
  }
  OPT ("b-bias") p->i_bframe_bias = atoi (value);
  OPT ("nf") p->b_deblocking_filter = !atobool (value);
  OPT2 ("filter", "deblock")
  {
    if (2 == sscanf (value, "%d:%d", &p->i_deblocking_filter_alphac0, &p->i_deblocking_filter_beta) || 2 == sscanf (value, "%d,%d", &p->i_deblocking_filter_alphac0, &p->i_deblocking_filter_beta))
    {
      p->b_deblocking_filter = 1;
    }
    else if (sscanf (value, "%d", &p->i_deblocking_filter_alphac0))
    {
      p->b_deblocking_filter = 1;
      p->i_deblocking_filter_beta = p->i_deblocking_filter_alphac0;
    }
    else
      p->b_deblocking_filter = atobool (value);
  }
  OPT ("log") p->i_log_level = atoi (value);
#ifdef VISUALIZE
  OPT ("visualize") p->b_visualize = atobool (value);
#endif
  OPT ("dump-yuv") p->psz_dump_yuv = strdup (value);
  OPT2 ("analyse", "partitions")
  {
    p->analyse.inter = 0;
    if (strstr (value, "none"))
      p->analyse.inter = 0;
    if (strstr (value, "all"))
      p->analyse.inter = ~0;

    if (strstr (value, "i8x8"))
      p->analyse.inter |= XAVS_ANALYSE_I8x8;
    if (strstr (value, "p8x8"))
      p->analyse.inter |= XAVS_ANALYSE_PSUB16x16;
    if (strstr (value, "b8x8"))
      p->analyse.inter |= XAVS_ANALYSE_BSUB16x16;
  }
  OPT ("8x8dct") p->analyse.b_transform_8x8 = atobool (value);
  OPT2 ("weightb", "weight-b") p->analyse.b_weighted_bipred = atobool (value);
  OPT2 ("direct", "direct-pred") b_error |= parse_enum (value, xavs_direct_pred_names, &p->analyse.i_direct_mv_pred);
  OPT ("chroma-qp-offset") p->analyse.i_chroma_qp_offset = atoi (value);
  OPT ("me") b_error |= parse_enum (value, xavs_motion_est_names, &p->analyse.i_me_method);
  OPT2 ("merange", "me-range") p->analyse.i_me_range = atoi (value);
  OPT2 ("mvrange", "mv-range") p->analyse.i_mv_range = atoi (value);
  OPT2 ("subme", "subq") p->analyse.i_subpel_refine = atoi (value);
  OPT ("psy-rd")
  {
    if (2 == sscanf (value, "%f:%f", &p->analyse.f_psy_rd, &p->analyse.f_psy_trellis) || 2 == sscanf (value, "%f,%f", &p->analyse.f_psy_rd, &p->analyse.f_psy_trellis))
    {
    }
    else if (sscanf (value, "%f", &p->analyse.f_psy_rd))
    {
      p->analyse.f_psy_trellis = 0;
    }
    else
    {
      p->analyse.f_psy_rd = 0;
      p->analyse.f_psy_trellis = 0;
    }
  }
  OPT ("chroma-me") p->analyse.b_chroma_me = atobool (value);
  OPT ("mixed-refs") p->analyse.b_mixed_references = atobool (value);
  OPT ("trellis") p->analyse.i_trellis = atoi (value);
  OPT ("fast-pskip") p->analyse.b_fast_pskip = atobool (value);
  OPT ("dct-decimate") p->analyse.b_dct_decimate = atobool (value);
  OPT ("deadzone-inter") p->analyse.i_luma_deadzone[0] = atoi (value);
  OPT ("deadzone-intra") p->analyse.i_luma_deadzone[1] = atoi (value);
  OPT ("nr") p->analyse.i_noise_reduction = atoi (value);
  OPT ("bitrate")
  {
    p->rc.i_bitrate = atoi (value);
    p->rc.i_rc_method = XAVS_RC_ABR;
  }
  OPT2 ("qp", "qp_constant")
  {
    p->rc.i_qp_constant = atoi (value);
    p->rc.i_rc_method = XAVS_RC_CQP;
  }
  OPT ("crf")
  {
    p->rc.f_rf_constant = atof (value);
    p->rc.i_rc_method = XAVS_RC_CRF;
  }
  OPT ("rc-lookahead") p->rc.i_lookahead = atoi (value);
  OPT2 ("qpmin", "qp-min") p->rc.i_qp_min = atoi (value);
  OPT2 ("qpmax", "qp-max") p->rc.i_qp_max = atoi (value);
  OPT2 ("qpstep", "qp-step") p->rc.i_qp_step = atoi (value);
  OPT ("ratetol") p->rc.f_rate_tolerance = !strncmp ("inf", value, 3) ? 1e9 : atof (value);
  OPT ("vbv-maxrate") p->rc.i_vbv_max_bitrate = atoi (value);
  OPT ("vbv-bufsize") p->rc.i_vbv_buffer_size = atoi (value);
  OPT ("vbv-init") p->rc.f_vbv_buffer_init = atof (value);
  OPT2 ("ipratio", "ip-factor") p->rc.f_ip_factor = atof (value);
  OPT2 ("pbratio", "pb-factor") p->rc.f_pb_factor = atof (value);
  OPT ("aq-mode") p->rc.i_aq_mode = atoi (value);
  OPT ("aq-strength") p->rc.f_aq_strength = atof (value);
  OPT ("pass")
  {
    int i = xavs_clip3 (atoi (value), 0, 3);
    p->rc.b_stat_write = i & 1;
    p->rc.b_stat_read = i & 2;
  }
  OPT ("stats")
  {
    p->rc.psz_stat_in = strdup (value);
    p->rc.psz_stat_out = strdup (value);
  }
  OPT ("qcomp") p->rc.f_qcompress = atof (value);
  OPT ("mbtree") p->rc.b_mb_tree = atobool (value);
  OPT ("qblur") p->rc.f_qblur = atof (value);
  OPT2 ("cplxblur", "cplx-blur") p->rc.f_complexity_blur = atof (value);
  OPT ("zones") p->rc.psz_zones = strdup (value);
  OPT ("psnr") p->analyse.b_psnr = atobool (value);
  OPT ("aud") p->b_aud = atobool (value);
  OPT ("global-header") p->b_repeat_headers = !atobool (value);
  OPT ("repeat-headers") p->b_repeat_headers = atobool (value);
  else
  return XAVS_PARAM_BAD_NAME;
#undef OPT
#undef OPT2
#undef atobool
#undef atoi
#undef atof

  if (name_buf)
    free (name_buf);

  b_error |= value_was_null && !name_was_bool;
  return b_error ? XAVS_PARAM_BAD_VALUE : 0;
}

#define OPT_QPMIN 256
#define OPT_QPMAX 257
#define OPT_QPSTEP 258
#define OPT_IPRATIO 260
#define OPT_PBRATIO 261
#define OPT_RATETOL 262
#define OPT_RCSTATS 264
#define OPT_QCOMP 266
#define OPT_PSNR 267
#define OPT_QUIET 268
#define OPT_SCENECUT 270
#define OPT_QBLUR 271
#define OPT_CPLXBLUR 272
#define OPT_FRAMES 273
#define OPT_FPS 274
#define OPT_DIRECT 275
#define OPT_LEVEL 276
#define OPT_NOBADAPT 277
#define OPT_BBIAS 278
#define OPT_BPYRAMID 279
#define OPT_CHROMA_QP 280
#define OPT_NO_CHROMA_ME 281
#define OPT_NO_CABAC 282
#define OPT_AUD 283
#define OPT_NOPROGRESS 284
#define OPT_ME 285
#define OPT_MERANGE 286
#define OPT_VBVMAXRATE 287
#define OPT_VBVBUFSIZE 288
#define OPT_VBVINIT 289
#define OPT_VISUALIZE 290
#define OPT_SEEK 291
#define OPT_ZONES 292
#define OPT_THREADS 293
#define OPT_CQM 294
#define OPT_CQM4 295
#define OPT_CQM4I 296
#define OPT_CQM4IY 297
#define OPT_CQM4IC 298
#define OPT_CQM4P 299
#define OPT_CQM4PY 300
#define OPT_CQM4PC 301
#define OPT_CQM8 302
#define OPT_CQM8I 303
#define OPT_CQM8P 304
#define OPT_CQMFILE 305
#define OPT_SAR 306
#define OPT_OVERSCAN 307
#define OPT_VIDFORMAT 308
#define OPT_FULLRANGE 309
#define OPT_COLOURPRIM 310
#define OPT_TRANSFER 311
#define OPT_COLOURMATRIX 312
#define OPT_CHROMALOC 313
#define OPT_MIXED_REFS 314
#define OPT_CRF 315
#define OPT_B_RDO 316
#define OPT_NO_FAST_PSKIP 317
#define OPT_BIME 318
#define OPT_NR 319
#define OPT_THREAD_INPUT 320
#define OPT_LONGHELP 321
#define OPT_FULLHELP 322
#define OPT_PROFILE avs
#define OPT_PRESET 323
#define OPT_TUNE 324
#define OPT_SLOWFIRSTPASS 325
#define OPT_QPFILE 326
#define OPT_DUMP_YUV 327
#define OPT_SYNC_LOOKAHEAD 328
#define OPT_RC_LOOKAHEAD 329
#define OPT_SLICED_THREAD 330
#define OPT_SSIM 331


static char short_options[] = "8A:B:b:f:hI:i:m:o:p:q:r:t:Vvw";
static struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"longhelp", no_argument, NULL, OPT_LONGHELP},
  {"version", no_argument, NULL, 'V'},
  {"preset", required_argument, NULL, OPT_PRESET},
  {"tune", required_argument, NULL, OPT_TUNE},
  {"slow-firstpass", no_argument, NULL, OPT_SLOWFIRSTPASS},
  {"bitrate", required_argument, NULL, 'B'},
  {"bframes", required_argument, NULL, 'b'},
  {"b-adapt", required_argument, NULL, 0},
  {"no-b-adapt", no_argument, NULL, 0},
  {"b-bias", required_argument, NULL, 0},
  {"min-keyint", required_argument, NULL, 'i'},
  {"keyint", required_argument, NULL, 'I'},
  {"scenecut", required_argument, NULL, 0},
  {"no-scenecut", no_argument, NULL, 0},
  {"nf", no_argument, NULL, 0},
  {"no-deblock", no_argument, NULL, 0},
  {"filter", required_argument, NULL, 0},
  {"deblock", required_argument, NULL, 'f'},
  {"interlaced", no_argument, NULL, 0},
  {"qp", required_argument, NULL, 'q'},
  {"qpmin", required_argument, NULL, 0},
  {"qpmax", required_argument, NULL, 0},
  {"qpstep", required_argument, NULL, 0},
  {"crf", required_argument, NULL, 0},
  {"rc-lookahead", required_argument, NULL, OPT_RC_LOOKAHEAD},
  {"ref", required_argument, NULL, 'r'},
  {"asm", required_argument, NULL, 0},
  {"no-asm", no_argument, NULL, 0},
  {"sar", required_argument, NULL, 0},
  {"fps", required_argument, NULL, OPT_FPS},
  {"frames", required_argument, NULL, OPT_FRAMES},
  {"seek", required_argument, NULL, OPT_SEEK},
  {"output", required_argument, NULL, 'o'},
  {"analyse", required_argument, NULL, 0},
  {"partitions", required_argument, NULL, 'A'},
  {"direct", required_argument, NULL, 0},
  {"weightb", no_argument, NULL, 'w'},
  {"no-weightb", no_argument, NULL, 0},
  {"me", required_argument, NULL, 0},
  {"merange", required_argument, NULL, 0},
  {"mvrange", required_argument, NULL, 0},
  {"mvrange-thread", required_argument, NULL, 0},
  {"subme", required_argument, NULL, 'm'},
  {"psy-rd", required_argument, NULL, 0},
  {"no-psy", no_argument, NULL, 0},
  {"psy", no_argument, NULL, 0},
  {"mixed-refs", no_argument, NULL, 0},
  {"no-mixed-refs", no_argument, NULL, 0},
  {"no-chroma-me", no_argument, NULL, 0},
  {"8x8dct", no_argument, NULL, 0},
  {"no-8x8dct", no_argument, NULL, 0},
  {"trellis", required_argument, NULL, 't'},
  {"fast-pskip", no_argument, NULL, 0},
  {"no-fast-pskip", no_argument, NULL, 0},
  {"no-dct-decimate", no_argument, NULL, 0},
  {"aq-strength", required_argument, NULL, 0},
  {"aq-mode", required_argument, NULL, 0},
  {"deadzone-inter", required_argument, NULL, '0'},
  {"deadzone-intra", required_argument, NULL, '0'},
  {"level", required_argument, NULL, 0},
  {"ratetol", required_argument, NULL, OPT_RATETOL},
  {"vbv-maxrate", required_argument, NULL, OPT_VBVMAXRATE},
  {"vbv-bufsize", required_argument, NULL, OPT_VBVBUFSIZE},
  {"vbv-init", required_argument, NULL, OPT_VBVINIT},
  {"ipratio", required_argument, NULL, OPT_IPRATIO},
  {"pbratio", required_argument, NULL, OPT_PBRATIO},
  {"chroma-qp-offset", required_argument, NULL, 0},
  {"pass", required_argument, NULL, 'p'},
  {"stats", required_argument, NULL, 0},
  {"qcomp", required_argument, NULL, 0},
  {"mbtree", no_argument, NULL, 0},
  {"no-mbtree", no_argument, NULL, 0},
  {"qblur", required_argument, NULL, 0},
  {"cplxblur", required_argument, NULL, 0},
  {"zones", required_argument, NULL, 0},
  {"qpfile", required_argument, NULL, OPT_QPFILE},
  {"threads", required_argument, NULL, OPT_THREADS},
  {"sliced-thread", required_argument, NULL, OPT_SLICED_THREAD},
  {"no-sliced-threads", no_argument, NULL, 0 },
  {"thread-input", no_argument, NULL, OPT_THREAD_INPUT},
  {"sync-lookahead", required_argument, NULL, OPT_SYNC_LOOKAHEAD},
  {"non-deterministic", no_argument, NULL, 0},
  {"psnr", no_argument, NULL, OPT_PSNR},
  {"ssim", no_argument, NULL, OPT_SSIM},
  {"quiet", no_argument, NULL, OPT_QUIET},
  {"verbose", no_argument, NULL, 'v'},
  {"no-progress", no_argument, NULL, OPT_NOPROGRESS},
  {"visualize", no_argument, NULL, OPT_VISUALIZE},
  {"dump-yuv", required_argument, NULL, OPT_DUMP_YUV},
  {"dump-yuv", required_argument, NULL, 0},
  {"sps-id", required_argument, NULL, 0},
  {"aud", no_argument, NULL, 0},
  {"nr", required_argument, NULL, 0},
  {"cqm", required_argument, NULL, 0},
  {"cqmfile", required_argument, NULL, 0},
  {"cqm4", required_argument, NULL, 0},
  {"cqm4i", required_argument, NULL, 0},
  {"cqm4iy", required_argument, NULL, 0},
  {"cqm4ic", required_argument, NULL, 0},
  {"cqm4p", required_argument, NULL, 0},
  {"cqm4py", required_argument, NULL, 0},
  {"cqm4pc", required_argument, NULL, 0},
  {"cqm8", required_argument, NULL, 0},
  {"cqm8i", required_argument, NULL, 0},
  {"cqm8p", required_argument, NULL, 0},
  {"overscan", required_argument, NULL, 0},
  {"videoformat", required_argument, NULL, 0},
  {"fullrange", required_argument, NULL, 0},
  {"colorprim", required_argument, NULL, 0},
  {"transfer", required_argument, NULL, 0},
  {"colormatrix", required_argument, NULL, 0},
  {"chromaloc", required_argument, NULL, 0},
  {0, 0, 0, 0}
};

/*****************************************************************************
 * Parse:
 *****************************************************************************/
static int
Parse (int argc, char **argv, xavs_param_t * param, cli_opt_t * opt)
{
  char *psz_filename = NULL;
  xavs_param_t defaults = *param;
  char *psz;
  int b_avis = 0;
  int b_y4m = 0;
  int b_thread_input = 0;
  int b_turbo = 1;
  int b_pass1 = 0;
  int b_user_ref = 0;
  int b_user_fps = 0;

  memset (opt, 0, sizeof (cli_opt_t));
  opt->b_progress = 1;

  /* Default input file driver */
  p_open_infile = open_file_yuv;
  p_get_frame_total = get_frame_total_yuv;
  p_read_frame = read_frame_yuv;
  p_close_infile = close_file_yuv;

  /* Default output file driver */
  p_open_outfile = open_file_bsf;
  p_set_outfile_param = set_param_bsf;
  p_write_nalu = write_nalu_bsf;
  p_set_eop = set_eop_bsf;
  p_close_outfile = close_file_bsf;

  /* Presets are applied before all other options. */
  for (optind = 0;;)
  {
    int c = getopt_long (argc, argv, short_options, long_options, NULL);
    if (c == -1)
      break;
    if (c == OPT_PRESET)
    {
      if (!strcasecmp (optarg, "ultrafast"))
      {
        param->i_frame_reference = 1;
        param->i_scenecut_threshold = 0;
        param->b_deblocking_filter = 0;
        param->i_bframe = 0;
        param->analyse.intra = 0;
        param->analyse.inter = 0;
        param->analyse.b_transform_8x8 = 0;
        param->analyse.i_me_method = XAVS_ME_DIA;
        param->analyse.i_subpel_refine = 0;
        param->rc.i_aq_mode = 0;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->i_bframe_adaptive = XAVS_B_ADAPT_NONE;
        param->rc.b_mb_tree = 0;
      }
      else if (!strcasecmp (optarg, "veryfast"))
      {
        param->analyse.inter = XAVS_ANALYSE_I8x8;
        param->analyse.i_me_method = XAVS_ME_DIA;
        param->analyse.i_subpel_refine = 1;
        param->i_frame_reference = 1;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->rc.b_mb_tree = 0;
      }
      else if (!strcasecmp (optarg, "faster"))
      {
        param->analyse.b_mixed_references = 0;
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 4;
        param->rc.b_mb_tree = 0;
      }
      else if (!strcasecmp (optarg, "fast"))
      {
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 6;
        param->rc.i_lookahead = 30;
      }
      else if (!strcasecmp (optarg, "medium"))
      {
        /* Default is medium */
      }
      else if (!strcasecmp (optarg, "slow"))
      {
        param->analyse.i_me_method = XAVS_ME_UMH;
        param->analyse.i_subpel_refine = 8;
        param->i_frame_reference = 5;
        param->i_bframe_adaptive = XAVS_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = XAVS_DIRECT_PRED_AUTO;
        param->rc.i_lookahead = 50;
      }
      else if (!strcasecmp (optarg, "slower"))
      {
        param->analyse.i_me_method = XAVS_ME_UMH;
        param->analyse.i_subpel_refine = 9;
        param->i_frame_reference = 8;
        param->i_bframe_adaptive = XAVS_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = XAVS_DIRECT_PRED_AUTO;
        param->analyse.inter |= XAVS_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->rc.i_lookahead = 60;
      }
      else if (!strcasecmp (optarg, "veryslow"))
      {
        param->analyse.i_me_method = XAVS_ME_UMH;
        param->analyse.i_subpel_refine = 10;
        param->analyse.i_me_range = 24;
        param->i_frame_reference = 16;
        param->i_bframe_adaptive = XAVS_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = XAVS_DIRECT_PRED_AUTO;
        param->analyse.inter |= XAVS_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->i_bframe = 8;
        param->rc.i_lookahead = 60;
      }
      else if (!strcasecmp (optarg, "placebo"))
      {
        param->analyse.i_me_method = XAVS_ME_TESA;
        param->analyse.i_subpel_refine = 10;
        param->analyse.i_me_range = 24;
        param->i_frame_reference = 16;
        param->i_bframe_adaptive = XAVS_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = XAVS_DIRECT_PRED_AUTO;
        param->analyse.inter |= XAVS_ANALYSE_PSUB8x8;
        param->analyse.b_fast_pskip = 0;
        param->analyse.i_trellis = 2;
        param->i_bframe = 16;
        param->rc.i_lookahead = 60;
      }
      else
      {
        fprintf (stderr, "xavs [error]: invalid preset: %s\n", optarg);
        return -1;
      }
    }
    else if (c == '?')
      return -1;
  }

  /* Tunings are applied next. */
  for (optind = 0;;)
  {
    int c = getopt_long (argc, argv, short_options, long_options, NULL);
    if (c == -1)
      break;

    if (c == OPT_TUNE)
    {
      if (!strcasecmp (optarg, "film"))
      {
        param->i_deblocking_filter_alphac0 = -1;
        param->i_deblocking_filter_beta = -1;
        param->analyse.f_psy_trellis = 0.15;
      }
      else if (!strcasecmp (optarg, "animation"))
      {
        param->i_frame_reference = param->i_frame_reference > 1 ? param->i_frame_reference * 2 : 1;
        param->i_deblocking_filter_alphac0 = 1;
        param->i_deblocking_filter_beta = 1;
        param->analyse.f_psy_rd = 0.4;
        param->rc.f_aq_strength = 0.6;
        param->i_bframe += 2;
      }
      else if (!strcasecmp (optarg, "grain"))
      {
        param->i_deblocking_filter_alphac0 = -2;
        param->i_deblocking_filter_beta = -2;
        param->analyse.f_psy_trellis = 0.25;
        param->analyse.b_dct_decimate = 0;
        param->rc.f_pb_factor = 1.1;
        param->rc.f_ip_factor = 1.1;
        param->rc.f_aq_strength = 0.5;
        param->analyse.i_luma_deadzone[0] = 6;
        param->analyse.i_luma_deadzone[1] = 6;
        param->rc.f_qcompress = 0.8;
      }
      else if (!strcasecmp (optarg, "psnr"))
      {
        param->rc.i_aq_mode = XAVS_AQ_NONE;
        param->analyse.b_psy = 0;
      }
      else if (!strcasecmp (optarg, "ssim"))
      {
        param->rc.i_aq_mode = XAVS_AQ_AUTOVARIANCE;
        param->analyse.b_psy = 0;
      }
      else if (!strcasecmp (optarg, "fastdecode"))
      {
        param->b_deblocking_filter = 0;
        param->analyse.b_weighted_bipred = 0;
      }
      else if (!strcasecmp (optarg, "touhou"))
      {
        param->i_frame_reference = param->i_frame_reference > 1 ? param->i_frame_reference * 2 : 1;
        param->i_deblocking_filter_alphac0 = -1;
        param->i_deblocking_filter_beta = -1;
        param->analyse.f_psy_trellis = 0.2;
        param->rc.f_aq_strength = 1.3;
        if (param->analyse.inter & XAVS_ANALYSE_PSUB16x16)
          param->analyse.inter |= XAVS_ANALYSE_PSUB8x8;
      }
      else
      {
        fprintf (stderr, "xavs [error]: invalid tune: %s\n", optarg);
        return -1;
      }
    }
    else if (c == '?')
      return -1;
  }

  /* Parse command line options */
  for (optind = 0;;)
  {
    int b_error = 0;
    int long_options_index = -1;

    int c = getopt_long (argc, argv, short_options, long_options, &long_options_index);

    if (c == -1)
    {
      break;
    }

    switch (c)
    {
    case 'h':
      Help (&defaults, 0);
      exit (0);
    case OPT_LONGHELP:
      Help (&defaults, 1);
      exit (0);
    case OPT_FULLHELP:
      Help (&defaults, 2);
      exit (0);
    case 'V':
#ifdef XAVS_POINTVER
      printf ("xavs " XAVS_POINTVER "\n");
#else
      printf ("xavs 0.%d.X\n", XAVS_BUILD);
#endif
      printf ("built on " __DATE__ ", ");
#ifdef __GNUC__
      printf ("gcc: " __VERSION__ "\n");
#else
      printf ("using a non-gcc compiler\n");
#endif
      exit (0);

    case 'B':
      param->rc.i_bitrate = atol (optarg);
      param->rc.i_rc_method = XAVS_RC_ABR;
      break;
    case 'b':
      param->i_bframe = atol (optarg);
      break;
    case OPT_NOBADAPT:
      param->i_bframe_adaptive = 0;
      break;
    case OPT_BBIAS:
      param->i_bframe_bias = atol (optarg);
      break;
    case OPT_BPYRAMID:
      param->b_bframe_pyramid = 1;
      break;
    case 'i':
      param->i_keyint_min = atol (optarg);
      if (param->i_keyint_max < param->i_keyint_min)
        param->i_keyint_max = param->i_keyint_min;
      break;
    case 'I':
      param->i_keyint_max = atol (optarg);
      if (param->i_keyint_min > param->i_keyint_max)
        param->i_keyint_min = param->i_keyint_max;
      break;
    case OPT_SCENECUT:
      param->i_scenecut_threshold = atol (optarg);
      break;
    case 'n':
      param->b_deblocking_filter = 0;
      break;
    case 'f':
      {
        char *p = strchr (optarg, ':');
        if (!p)
          p = strchr (optarg, ',');
        param->i_deblocking_filter_alphac0 = atoi (optarg);
        param->i_deblocking_filter_beta = p ? atoi (p + 1) : param->i_deblocking_filter_alphac0;
        break;
      }
    case 'q':
      param->rc.i_qp_constant = atoi (optarg);
      param->rc.i_rc_method = XAVS_RC_CQP;
      break;
    case OPT_QPMIN:
      param->rc.i_qp_min = atoi (optarg);
      break;
    case OPT_QPMAX:
      param->rc.i_qp_max = atoi (optarg);
      break;
    case OPT_QPSTEP:
      param->rc.i_qp_step = atoi (optarg);
      break;
    case OPT_NO_CABAC:
      param->b_cabac = 0;
      break;
    case 'x':
      opt->b_decompress = 1;
      break;
    case 'C':
      param->cpu = 0;
      break;
    case OPT_FRAMES:
      param->i_frame_total = atoi (optarg);
      break;
    case OPT_SEEK:
      opt->i_seek = atoi (optarg);
      break;
    case 'o':
      if (!strncasecmp (optarg + strlen (optarg) - 4, ".mp4", 4))
      {
#ifdef MP4_OUTPUT
        p_open_outfile = open_file_mp4;
        p_write_nalu = write_nalu_mp4;
        p_set_outfile_param = set_param_mp4;
        p_set_eop = set_eop_mp4;
        p_close_outfile = close_file_mp4;
#else
        fprintf (stderr, "xavs [error]: not compiled with MP4 output support\n");
        return -1;
#endif
      }
      else if (!strncasecmp (optarg + strlen (optarg) - 4, ".mkv", 4))
      {
        p_open_outfile = open_file_mkv;
        p_write_nalu = write_nalu_mkv;
        p_set_outfile_param = set_param_mkv;
        p_set_eop = set_eop_mkv;
        p_close_outfile = close_file_mkv;
      }
      if (!strcmp (optarg, "-"))
        opt->hout = stdout;
      else if (p_open_outfile (optarg, &opt->hout))
      {
        fprintf (stderr, "xavs [error]: can't open output file `%s'\n", optarg);
        return -1;
      }
      break;
    case OPT_QPFILE:
      opt->qpfile = fopen (optarg, "rb");
      if (!opt->qpfile)
      {
        fprintf (stderr, "xavs [error]: can't open `%s'\n", optarg);
        return -1;
      }
      break;
    case OPT_SAR:
      {
        char *p = strchr (optarg, ':');
        if (!p)
          p = strchr (optarg, '/');
        if (p)
        {
          param->vui.i_sar_width = atoi (optarg);
          param->vui.i_sar_height = atoi (p + 1);
        }
        break;
      }
    case OPT_FPS:
      {
        float fps;
        if (sscanf (optarg, "%d/%d", &param->i_fps_num, &param->i_fps_den) == 2)
          ;
        else if (sscanf (optarg, "%f", &fps))
        {
          param->i_fps_num = (int) (fps * 1000 + .5);
          param->i_fps_den = 1000;
        }
        else
        {
          fprintf (stderr, "bad fps `%s'\n", optarg);
          return -1;
        }
        break;
      }
    case 'A':
      param->analyse.inter = 0;
      if (strstr (optarg, "none"))
        param->analyse.inter = 0;
      if (strstr (optarg, "all"))
        param->analyse.inter = ~0;

      if (strstr (optarg, "i4x4"))
        param->analyse.inter |= XAVS_ANALYSE_I4x4;
      if (strstr (optarg, "i8x8"))
        param->analyse.inter |= XAVS_ANALYSE_I8x8;
      if (strstr (optarg, "p8x8"))
        param->analyse.inter |= XAVS_ANALYSE_PSUB16x16;
      if (strstr (optarg, "p4x4"))
        param->analyse.inter |= XAVS_ANALYSE_PSUB8x8;
      if (strstr (optarg, "b8x8"))
        param->analyse.inter |= XAVS_ANALYSE_BSUB16x16;
      break;
    case OPT_DIRECT:
      b_error |= parse_enum (optarg, xavs_direct_pred_names, &param->analyse.i_direct_mv_pred);
      break;
    case 'w':
      param->analyse.b_weighted_bipred = 1;
      break;
    case OPT_ME:
      b_error |= parse_enum (optarg, xavs_motion_est_names, &param->analyse.i_me_method);
      break;
    case OPT_MERANGE:
      param->analyse.i_me_range = atoi (optarg);
      break;
    case 'm':
      param->analyse.i_subpel_refine = atoi (optarg);
      break;
    case OPT_B_RDO:
      param->analyse.b_bframe_rdo = 1;
      break;
    case OPT_MIXED_REFS:
      param->analyse.b_mixed_references = 1;
      break;
    case OPT_NO_CHROMA_ME:
      param->analyse.b_chroma_me = 0;
      break;
    case OPT_BIME:
      param->analyse.b_bidir_me = 1;
      break;
    case '8':
      param->analyse.b_transform_8x8 = 1;
      break;
    case 't':
      param->analyse.i_trellis = atoi (optarg);
      break;
    case OPT_NO_FAST_PSKIP:
      param->analyse.b_fast_pskip = 0;
      break;
    case OPT_LEVEL:
      if (atof (optarg) < 6)
        param->i_level_idc = (int) (10 * atof (optarg) + .5);
      else
        param->i_level_idc = atoi (optarg);
      break;
    case OPT_RATETOL:
      param->rc.f_rate_tolerance = (float) (!strncmp ("inf", optarg, 3) ? 1e9 : atof (optarg));
      break;
    case OPT_VBVMAXRATE:
      param->rc.i_vbv_max_bitrate = atoi (optarg);
      break;
    case OPT_VBVBUFSIZE:
      param->rc.i_vbv_buffer_size = atoi (optarg);
      break;
    case OPT_VBVINIT:
      param->rc.f_vbv_buffer_init = (float) atof (optarg);
      break;
    case OPT_IPRATIO:
      param->rc.f_ip_factor = (float) atof (optarg);
      break;
    case OPT_PBRATIO:
      param->rc.f_pb_factor = (float) atof (optarg);
      break;
    case OPT_CHROMA_QP:
      param->analyse.i_chroma_qp_offset = atoi (optarg);
      break;
    case OPT_RCSTATS:
      param->rc.psz_stat_in = optarg;
      param->rc.psz_stat_out = optarg;
      break;
    case OPT_QCOMP:
      param->rc.f_qcompress = (float) atof (optarg);
      break;
    case OPT_QBLUR:
      param->rc.f_qblur = (float) atof (optarg);
      break;
    case OPT_CPLXBLUR:
      param->rc.f_complexity_blur = (float) atof (optarg);
      break;
    case OPT_ZONES:
      param->rc.psz_zones = optarg;
      break;
    case OPT_THREADS:
      param->i_threads = XAVS_MIN(atoi(optarg), XAVS_THREAD_MAX);
      break;
    case OPT_SLICED_THREAD:
      param->b_sliced_threads = atoi (optarg);
      break;
    case OPT_THREAD_INPUT:
      b_thread_input = 1;
      break;
    case OPT_SYNC_LOOKAHEAD:
      param->i_sync_lookahead = XAVS_MIN(atoi(optarg), XAVS_LOOKAHEAD_MAX);
      break;
    case OPT_RC_LOOKAHEAD:
      param->rc.i_lookahead = atoi(optarg); 
      break;
    case OPT_PSNR:
      param->analyse.b_psnr = 1;
      break;
    case OPT_SSIM:
      param->analyse.b_ssim = 1;
      break;
    case OPT_QUIET:
      param->i_log_level = XAVS_LOG_NONE;
      break;
    case 'v':
      param->i_log_level = XAVS_LOG_DEBUG;
      break;
    case OPT_AUD:
      param->b_aud = 1;
      break;
    case OPT_NOPROGRESS:
      opt->b_progress = 0;
      break;
    case OPT_VISUALIZE:
#ifdef VISUALIZE
      param->b_visualize = 1;
      b_exit_on_ctrl_c = 1;
#else
      fprintf (stderr, "xavs [warning]: not compiled with visualization support\n");
#endif
      break;
    case OPT_TUNE:
    case OPT_PRESET:
      break;
    case OPT_SLOWFIRSTPASS:
      b_turbo = 0;
      break;
    case 'r':
      b_user_ref = 1;
      goto generic_option;
    case 'p':
      b_pass1 = atoi (optarg) == 1;
      goto generic_option;
    generic_option:
      {
        int i;
        if (long_options_index < 0)
        {
          for (i = 0; long_options[i].name; i++)
            if (long_options[i].val == c)
            {
              long_options_index = i;
              break;
            }
          if (long_options_index < 0)
          {
            /* getopt_long already printed an error message */
            return -1;
          }
        }

        b_error |= xavs_param_parse (param, long_options[long_options_index].name, optarg);
      }
    case OPT_DUMP_YUV:
      param->psz_dump_yuv = strdup (optarg);
      break;
    case OPT_NR:
      param->analyse.i_noise_reduction = atoi (optarg);
      break;
    case OPT_CQM:
      if (strstr (optarg, "flat"))
        param->i_cqm_preset = XAVS_CQM_FLAT;
      else if (strstr (optarg, "jvt"))
        param->i_cqm_preset = XAVS_CQM_JVT;
      else
      {
        fprintf (stderr, "bad CQM preset `%s'\n", optarg);
        return -1;
      }
      break;
    case OPT_CQMFILE:
      param->psz_cqm_file = optarg;
      break;
    case OPT_CQM4:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4iy, 16);
      b_error |= parse_cqm (optarg, param->cqm_4ic, 16);
      b_error |= parse_cqm (optarg, param->cqm_4py, 16);
      b_error |= parse_cqm (optarg, param->cqm_4pc, 16);
      break;
    case OPT_CQM8:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_8iy, 64);
      b_error |= parse_cqm (optarg, param->cqm_8py, 64);
      break;
    case OPT_CQM4I:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4iy, 16);
      b_error |= parse_cqm (optarg, param->cqm_4ic, 16);
      break;
    case OPT_CQM4P:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4py, 16);
      b_error |= parse_cqm (optarg, param->cqm_4pc, 16);
      break;
    case OPT_CQM4IY:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4iy, 16);
      break;
    case OPT_CQM4IC:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4ic, 16);
      break;
    case OPT_CQM4PY:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4py, 16);
      break;
    case OPT_CQM4PC:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_4pc, 16);
      break;
    case OPT_CQM8I:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_8iy, 64);
      break;
    case OPT_CQM8P:
      param->i_cqm_preset = XAVS_CQM_CUSTOM;
      b_error |= parse_cqm (optarg, param->cqm_8py, 64);
      break;
    case OPT_CHROMALOC:
      param->vui.i_chroma_loc = atoi (optarg);
      b_error = (param->vui.i_chroma_loc < 0 || param->vui.i_chroma_loc > 5);
      break;
    default:
      fprintf (stderr, "unknown option (%c)\n", optopt);
      return -1;
    }

    if (b_error)
    {
      const char *name = long_options_index > 0 ? long_options[long_options_index].name : argv[optind - 2];
      fprintf (stderr, "xavs [error]: invalid argument: %s = %s\n", name, optarg);
      return -1;
    }
  }
  /* Set faster options in case of turbo firstpass. */
  if (b_turbo && b_pass1)
  {
    param->i_frame_reference = 1;
    param->analyse.b_transform_8x8 = 0;
    param->analyse.inter = 0;
    param->analyse.i_me_method = XAVS_ME_DIA;
    param->analyse.i_subpel_refine = XAVS_MIN (2, param->analyse.i_subpel_refine);
    param->analyse.i_trellis = 0;
  }

  /* Get the file name */
  if (optind > argc - 1 || !opt->hout)
  {
    fprintf (stderr, "xavs [error]: No %s file. Run xavs --help for a list of options.\n", optind > argc - 1 ? "input" : "output");
    return -1;
  }
  psz_filename = argv[optind++];

  /* check demuxer type */
  psz = psz_filename + strlen (psz_filename) - 1;
  while (psz > psz_filename && *psz != '.')
    psz--;
  if (!strncasecmp (psz, ".avi", 4) || !strncasecmp (psz, ".avs", 4))
    b_avis = 1;
  if (!strncasecmp (psz, ".y4m", 4))
    b_y4m = 1;
  if (!(b_avis || b_y4m))       // raw yuv
  {
    if (optind > argc - 1)
    {
      /* try to parse the file name */
      for (psz = psz_filename; *psz; psz++)
      {
        if (*psz >= '0' && *psz <= '9' && sscanf (psz, "%ux%u", &param->i_width, &param->i_height) == 2)
        {
          if (param->i_log_level >= XAVS_LOG_INFO)
            fprintf (stderr, "xavs [info]: %dx%d (given by file name) @ %.2f fps\n", param->i_width, param->i_height, (double) param->i_fps_num / (double) param->i_fps_den);
          break;
        }
      }
    }
    else
    {
      sscanf (argv[optind++], "%ux%u", &param->i_width, &param->i_height);
      if (param->i_log_level >= XAVS_LOG_INFO)
        fprintf (stderr, "xavs [info]: %dx%d @ %.2f fps\n", param->i_width, param->i_height, (double) param->i_fps_num / (double) param->i_fps_den);
    }
  }

  if (!(b_avis || b_y4m) && (!param->i_width || !param->i_height))
  {
    fprintf (stderr, "xavs [error]: Rawyuv input requires a resolution.\n");
    return -1;
  }

  /* open the input */
  {
    int i_fps_num = param->i_fps_num;
    int i_fps_den = param->i_fps_den;
    if (b_avis)
    {
#ifdef AVIS_INPUT
      p_open_infile = open_file_avis;
      p_get_frame_total = get_frame_total_avis;
      p_read_frame = read_frame_avis;
      p_close_infile = close_file_avis;
#else
      fprintf (stderr, "xavs [error]: not compiled with AVIS input support\n");
      return -1;
#endif
    }
    if (b_y4m)
    {
      p_open_infile = open_file_y4m;
      p_get_frame_total = get_frame_total_y4m;
      p_read_frame = read_frame_y4m;
      p_close_infile = close_file_y4m;
    }

    if (p_open_infile (psz_filename, &opt->hin, param))
    {
      fprintf (stderr, "xavs [error]: could not open input file '%s'\n", psz_filename);
      return -1;
    }
    /* Restore the user's frame rate if fps has been explicitly set on the commandline. */
    if (b_user_fps)
    {
      param->i_fps_num = i_fps_num;
      param->i_fps_den = i_fps_den;
    }
  }

#ifdef HAVE_PTHREAD
  if (b_thread_input || param->i_threads > 1 || (param->i_threads == XAVS_THREADS_AUTO && xavs_cpu_num_processors () > 1))
  {
    if (open_file_thread (NULL, &opt->hin, param))
    {
      fprintf (stderr, "xavs [error]: threaded input failed\n");
      return -1;
    }
    else
    {
      p_open_infile = open_file_thread;
      p_get_frame_total = get_frame_total_thread;
      p_read_frame = read_frame_thread;
      p_close_infile = close_file_thread;
    }
  }
#endif

  return 0;
}

static void
parse_qpfile (cli_opt_t * opt, xavs_picture_t * pic, int i_frame)
{
  int num = -1, qp, ret;
  char type;
  uint64_t file_pos;
  while (num < i_frame)
  {
    file_pos = ftell (opt->qpfile);
    ret = fscanf (opt->qpfile, "%d %c %d\n", &num, &type, &qp);
    if (num > i_frame || ret == EOF)
    {
      pic->i_type = XAVS_TYPE_AUTO;
      pic->i_qpplus1 = 0;
      fseek (opt->qpfile, file_pos, SEEK_SET);
      break;
    }
    if (num < i_frame && ret == 3)
      continue;
    pic->i_qpplus1 = qp + 1;
    if (type == 'I')
      pic->i_type = XAVS_TYPE_IDR;
    else if (type == 'i')
      pic->i_type = XAVS_TYPE_I;
    else if (type == 'P')
      pic->i_type = XAVS_TYPE_P;
    else if (type == 'B')
      pic->i_type = XAVS_TYPE_BREF;
    else if (type == 'b')
      pic->i_type = XAVS_TYPE_B;
    else
      ret = 0;
    if (ret != 3 || qp < -1 || qp > 63)
    {
      fprintf (stderr, "xavs [error]: can't parse qpfile for frame %d\n", i_frame);
      fclose (opt->qpfile);
      opt->qpfile = NULL;
      pic->i_type = XAVS_TYPE_AUTO;
      pic->i_qpplus1 = 0;
      break;
    }
  }
}

/*****************************************************************************
 * Decode:
 *****************************************************************************/

static int
Encode_frame (xavs_t * h, hnd_t hout, xavs_picture_t * pic)
{
  xavs_picture_t pic_out;
  xavs_nal_t *nal;
  int i_nal = 0, i;
  int i_file = 0;

  /* Do not force any parameters */
  if (pic)
  {
    pic->i_type = XAVS_TYPE_AUTO;
    pic->i_qpplus1 = 0;
  }
  if (xavs_encoder_encode (h, &nal, &i_nal, pic, &pic_out) < 0)
  {
    fprintf (stderr, "xavs_encoder_encode failed, h->frames.i_input: %d\n", h->frames.i_input);
  }

  for (i = 0; i < i_nal; i++)
  {
    int i_size;
    int i_data;

    i_data = DATA_MAX;
    if ((i_size = xavs_nal_encode (data, &i_data, 1, &nal[i])) > 0)
    {
      i_file += p_write_nalu (hout, data, i_size);
    }
    else if (i_size < 0)
    {
      fprintf (stderr, "need to increase buffer size (size=%d)\n", -i_size);
    }
  }
  if (i_nal)
    p_set_eop (hout, &pic_out);

  return i_file;

}

static void
Print_status (int64_t i_start, int i_frame, int i_frame_total, int64_t i_file, xavs_param_t * param)
{
  char buf[200];
  int64_t i_elapsed = xavs_mdate () - i_start;
  double fps = i_elapsed > 0 ? i_frame * 1000000. / i_elapsed : 0;
  double bitrate = (double) i_file * 8 * param->i_fps_num / ((double) param->i_fps_den * i_frame * 1000);
  if (i_frame_total)
  {
    int eta = i_elapsed * (i_frame_total - i_frame) / ((int64_t) i_frame * 1000000);
    sprintf (buf, "xavs [%.1f%%] %d/%d frames, %.2f fps, %.2f kb/s, eta %d:%02d:%02d", 100. * i_frame / i_frame_total, i_frame, i_frame_total, fps, bitrate, eta / 3600, (eta / 60) % 60, eta % 60);
  }
  else
  {
    sprintf (buf, "xavs %d frames: %.2f fps, %.2f kb/s", i_frame, fps, bitrate);
  }
  fprintf (stderr, "%s  \r", buf + 5);
  SetConsoleTitle (buf);
  fflush (stderr);              // needed in windows
}

/*****************************************************************************
 * Encode:
 *****************************************************************************/
static int
Encode (xavs_param_t * param, cli_opt_t * opt)
{
  xavs_t *h;
  xavs_picture_t pic;

  int i_frame, i_frame_total, i_frame_output;
  int64_t i_start, i_end;
  int64_t i_file;
  int i_frame_size;
  int i_update_interval;
  int endcode = 0xb1010000;
  double  f_timestamp = 0;
  int     i_timestamp = 0;
  FILE    *fp;

  opt->b_progress &= param->i_log_level < XAVS_LOG_DEBUG;
  i_frame_total = p_get_frame_total (opt->hin);
  i_frame_total -= opt->i_seek;
  if ((i_frame_total == 0 || param->i_frame_total < i_frame_total) && param->i_frame_total > 0)
    i_frame_total = param->i_frame_total;
  param->i_frame_total = i_frame_total;
  i_update_interval = i_frame_total ? xavs_clip3 (i_frame_total / 1000, 1, 10) : 10;

  if ((h = xavs_encoder_open (param)) == NULL)
  {
    fprintf (stderr, "xavs [error]: xavs_encoder_open failed\n");
    p_close_infile (opt->hin);
    p_close_outfile (opt->hout);
    return -1;
  }

  if (p_set_outfile_param (opt->hout, param))
  {
    fprintf (stderr, "xavs [error]: can't set outfile param\n");
    p_close_infile (opt->hin);
    p_close_outfile (opt->hout);
    return -1;
  }

  /* Create a new pic */
  if (xavs_picture_alloc (&pic, XAVS_CSP_I420, param->i_width, param->i_height) < 0)
  {
    fprintf (stderr, "xavs [error]: malloc failed\n");
    return -1;
  }

  i_start = xavs_mdate ();
  remove("timestamp.dat"); //delete timestamp file

  /* Encode frames */
  for (i_frame = 0, i_file = 0, i_frame_output = 0; b_ctrl_c == 0 && (i_frame < i_frame_total || i_frame_total == 0);)
  {
    if (p_read_frame (&pic, opt->hin, i_frame + opt->i_seek))
      break;

    pic.i_pts = (int64_t) i_frame *param->i_fps_den;

    if (opt->qpfile)
      parse_qpfile (opt, &pic, i_frame + opt->i_seek);
    else
    {
      /* Do not force any parameters */
      pic.i_type = XAVS_TYPE_AUTO;
      pic.i_qpplus1 = 0;
    }

    i_frame_size = Encode_frame (h, opt->hout, &pic);
    if (i_frame_size < 0)
      return -1;
    i_file += i_frame_size;
    if (i_frame_size)
      i_frame_output++;

    i_frame++;

    //timestamp output
    fp = fopen("timestamp.dat", "ab+");
    f_timestamp += ((double)param->i_fps_den / (double) param->i_fps_num * 1000);
    i_timestamp = (int)f_timestamp;
    fwrite(&i_timestamp,sizeof(int),1,fp);		
    fclose(fp);	

    /* update status line (up to 1000 times per input file) */
    if (opt->b_progress && i_frame_output % i_update_interval == 0 && i_frame_output)
      Print_status (i_start, i_frame_output, i_frame_total, i_file, param);
  }

  /* Flush delayed frames */
  do
  {
    i_frame_size = Encode_frame (h, opt->hout, NULL);
    i_file += i_frame_size;
    if (i_frame_size)
      i_frame_output++;
    if (opt->b_progress && i_frame_output % i_update_interval == 0 && i_frame_output)
      Print_status (i_start, i_frame_output, i_frame_total, i_file, param);
  }
  while (!b_ctrl_c && i_frame_size);

  //  write sequence end code
  if (fwrite (&endcode, 4, 1, opt->hout) < 0)
    fprintf (stderr, "can't set outfile param\n");      //   printf("cannot write avs file.\n");

  i_end = xavs_mdate ();
  xavs_picture_clean (&pic);
  if (opt->b_progress)
    fprintf (stderr, "                                                                               \r");
  xavs_encoder_close (h);
  fprintf (stderr, "\n");

  if (b_ctrl_c)
    fprintf (stderr, "aborted at input frame %d, output frame %d\n", opt->i_seek + i_frame, i_frame_output);

  p_close_infile (opt->hin);
  p_close_outfile (opt->hout);

  if (i_frame_output > 0)
  {
    double fps = (double) i_frame_output * (double) 1000000 / (double) (i_end - i_start);

    fprintf (stderr, "encoded %d frames, %.2f fps, %.2f kb/s\n", i_frame_output, fps, (double) i_file * 8 * param->i_fps_num / ((double) param->i_fps_den * i_frame_output * 1000));
  }

  return 0;
}
