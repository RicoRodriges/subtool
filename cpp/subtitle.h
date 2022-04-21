#pragma once

// Makes "subtitle.c" VLC source file shared

#include <vlc_common.h>

int Open ( vlc_object_t *p_this );
void Close( vlc_object_t *p_this );

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
//    SUB_TYPE_SCC,      /* Scenarist Closed Caption */
};

typedef struct
{
    vlc_tick_t i_start;
    vlc_tick_t i_stop;

    char    *psz_text;
} subtitle_t;

typedef struct
{
    enum subtitle_type_e i_type;
    vlc_tick_t  i_microsecperframe;

    char        *psz_header; /* SSA */
    char        *psz_lang;

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
//    es_out_id_t *es;
//    bool        b_slave;
//    bool        b_first_time;

//    double      f_rate;
//    vlc_tick_t  i_next_demux_date;

    struct
    {
        subtitle_t *p_array;
        size_t      i_count;
        size_t      i_current;
    } subtitles;

//    vlc_tick_t  i_length;

    /* */
    subs_properties_t props;

//    block_t * (*pf_convert)( const subtitle_t * );
} demux_sys_t;