#if defined (__cplusplus)
extern "C" {
#endif

#ifndef SUBTITLES_TYPES_H
#define SUBTITLES_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define SUCCESS 0
#define FAIL -1
#define FAIL_MEM -2
#define ZERO_FPS -3
#define UNKNOWN_FORMAT -4

enum subtitle_type_e
{
    SUB_TYPE_UNKNOWN = -1,
    SUB_TYPE_MICRODVD,
    SUB_TYPE_SUBRIP,
    SUB_TYPE_SSA1,
    SUB_TYPE_SSA2_4,
    SUB_TYPE_ASS,
    SUB_TYPE_VPLAYER,
    SUB_TYPE_SAMI,
    SUB_TYPE_SUBVIEWER, /* SUBVIEWER 2 */
    SUB_TYPE_DVDSUBTITLE, /* Mplayer calls it subviewer2 */
    SUB_TYPE_MPL2,
    SUB_TYPE_AQT,
    SUB_TYPE_PJS,
    SUB_TYPE_MPSUB,
    SUB_TYPE_JACOSUB,
    SUB_TYPE_PSB,
    SUB_TYPE_RT,
    SUB_TYPE_DKS,
    SUB_TYPE_SUBVIEW1, /* SUBVIEWER 1 - mplayer calls it subrip09,
                         and Gnome subtitles SubViewer 1.0 */
    SUB_TYPE_SBV,
};

typedef struct
{
    const char * content;
    const uint64_t size;

    uint64_t current;
} stream_t;

typedef struct
{
    size_t  i_line_count;
    size_t  i_line;
    char    **line;
} text_t;

typedef int64_t vlc_tick_t;

typedef struct
{
    vlc_tick_t i_start;
    vlc_tick_t i_stop;

    char    *psz_text;
} subtitle_t;

typedef struct
{
    enum subtitle_type_e i_type;
    float i_microsecperframe;

    char        *psz_header; /* SSA */

    struct
    {
        bool b_inited;

        int i_comment;
        int i_time_resolution;
        int i_time_shift;
    } jss;

    struct
    {
        bool  b_inited;

        float f_total;
        int i_factor;
    } mpsub;

    struct
    {
        const char *psz_start;
    } sami;

} subs_properties_t;

typedef struct
{
    struct
    {
        subtitle_t *p_array;
        size_t      i_count;
        size_t      i_current;
    } subtitles;
    subs_properties_t props;
} demux_sys_t;

#endif

#if defined (__cplusplus)
}
#endif