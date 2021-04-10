/*****************************************************************************
 * subtitle.c: Demux for subtitle text files.
 *****************************************************************************
 * Copyright (C) 1999-2007 VLC authors and VideoLAN
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Derk-Jan Hartman <hartman at videolan dot org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "subtitle.h"

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int  TextLoad( text_t *, stream_t *s );
static void TextUnload( text_t * );

static int  ParseMicroDvd   ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseSubRip     ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseSubViewer  ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseSSA        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseVplayer    ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseSami       ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseDVDSubtitle( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseMPL2       ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseAQT        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParsePJS        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseMPSub      ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseJSS        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParsePSB        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseRealText   ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseDKS        ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseSubViewer1 ( subs_properties_t *, text_t *, subtitle_t *, size_t );
static int  ParseCommonSBV  ( subs_properties_t *, text_t *, subtitle_t *, size_t );

const sub_info_t sub_read_subtitle_function [] =
{
    { "microdvd",   SUB_TYPE_MICRODVD,    "MicroDVD",    ParseMicroDvd },
    { "subrip",     SUB_TYPE_SUBRIP,      "SubRIP",      ParseSubRip },
    { "subviewer",  SUB_TYPE_SUBVIEWER,   "SubViewer",   ParseSubViewer },
    { "ssa1",       SUB_TYPE_SSA1,        "SSA-1",       ParseSSA },
    { "ssa2-4",     SUB_TYPE_SSA2_4,      "SSA-2/3/4",   ParseSSA },
    { "ass",        SUB_TYPE_ASS,         "SSA/ASS",     ParseSSA },
    { "vplayer",    SUB_TYPE_VPLAYER,     "VPlayer",     ParseVplayer },
    { "sami",       SUB_TYPE_SAMI,        "SAMI",        ParseSami },
    { "dvdsubtitle",SUB_TYPE_DVDSUBTITLE, "DVDSubtitle", ParseDVDSubtitle },
    { "mpl2",       SUB_TYPE_MPL2,        "MPL2",        ParseMPL2 },
    { "aqt",        SUB_TYPE_AQT,         "AQTitle",     ParseAQT },
    { "pjs",        SUB_TYPE_PJS,         "PhoenixSub",  ParsePJS },
    { "mpsub",      SUB_TYPE_MPSUB,       "MPSub",       ParseMPSub },
    { "jacosub",    SUB_TYPE_JACOSUB,     "JacoSub",     ParseJSS },
    { "psb",        SUB_TYPE_PSB,         "PowerDivx",   ParsePSB },
    { "realtext",   SUB_TYPE_RT,          "RealText",    ParseRealText },
    { "dks",        SUB_TYPE_DKS,         "DKS",         ParseDKS },
    { "subviewer1", SUB_TYPE_SUBVIEW1,    "Subviewer 1", ParseSubViewer1 },
    { "sbv",        SUB_TYPE_SBV,         "SBV",         ParseCommonSBV },
    { NULL,         SUB_TYPE_UNKNOWN,     "Unknown",     NULL }
};

//static char * ReadLine(stream_t * stream) {
//    if (stream->current >= stream->size)
//        return NULL;
//
//    uint64_t current = stream->current;
//
//    uint64_t size = 0;
//    for (uint64_t i = stream->current; i < stream->size; ++i) {
//        if (stream->content[i] == '\n') {
//            current = i + 1;
//
//            if (size > 0 && stream->content[i - 1] == '\r')
//                --size;
//            break;
//        }
//        ++size;
//    }
//
//    char *line = malloc((size + 1) * sizeof(char));
//    memcpy(line, stream->content + stream->current, size);
//    line[size] = 0;
//
//    stream->current = current;
//    return line;
//}

static char * ReadLine(stream_t * stream) {
    if (stream->current >= stream->size)
        return NULL;

    uint64_t size = 0;
    uint64_t newCurrent = stream->current;
    const uint64_t oldCurrent = stream->current;

    char *offset = memchr(&stream->content[oldCurrent], '\n', stream->size - oldCurrent);
    if (offset == NULL) {
        // End of file
        size = stream->size - oldCurrent;
        newCurrent = stream->size;
    } else {
        size = (size_t) offset;
        size -= (size_t) (&stream->content[oldCurrent]);
        newCurrent += size + 1; // skip '\n'
        if (size > 0 && *(offset - 1) == '\r')
            --size; // skip '\r'
    }

    char *line = malloc((size + 1) * sizeof(char));
    memcpy(line, stream->content + oldCurrent, size);
    line[size] = 0;

    stream->current = newCurrent;
    return line;
}

/*****************************************************************************
 * Module initializer
 *****************************************************************************/
enum subtitle_type_e DetectType(const char * content, uint64_t size) {
    enum subtitle_type_e type = SUB_TYPE_UNKNOWN;

    stream_t stream = {content, size, 0};
    /* Probe if unknown type */
    {
        int     i_try;
        char    *s = NULL;

        for( i_try = 0; i_try < 256; i_try++ )
        {
            int i_dummy;
            char p_dummy;

            if( (s = ReadLine( &stream ) ) == NULL )
                break;

            if( strcasestr( s, "<SAMI>" ) )
            {
                type = SUB_TYPE_SAMI;
                break;
            }
            else if( sscanf( s, "{%d}{%d}", &i_dummy, &i_dummy ) == 2 ||
                     sscanf( s, "{%d}{}", &i_dummy ) == 1)
            {
                type = SUB_TYPE_MICRODVD;
                break;
            }
            else if( sscanf( s, "%d:%d:%d,%d --> %d:%d:%d,%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy ) == 8 ||
                     sscanf( s, "%d:%d:%d --> %d:%d:%d,%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy ) == 7 ||
                     sscanf( s, "%d:%d:%d,%d --> %d:%d:%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy ) == 7 ||
                     sscanf( s, "%d:%d:%d.%d --> %d:%d:%d.%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy ) == 8 ||
                     sscanf( s, "%d:%d:%d --> %d:%d:%d.%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy ) == 7 ||
                     sscanf( s, "%d:%d:%d.%d --> %d:%d:%d",
                             &i_dummy,&i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy ) == 7 ||
                     sscanf( s, "%d:%d:%d --> %d:%d:%d",
                             &i_dummy,&i_dummy,&i_dummy,
                             &i_dummy,&i_dummy,&i_dummy ) == 6 )
            {
                type = SUB_TYPE_SUBRIP;
                break;
            }
            else if( !strncasecmp( s, "!: This is a Sub Station Alpha v1", 33 ) )
            {
                type = SUB_TYPE_SSA1;
                break;
            }
            else if( !strncasecmp( s, "ScriptType: v4.00+", 18 ) )
            {
                type = SUB_TYPE_ASS;
                break;
            }
            else if( !strncasecmp( s, "ScriptType: v4.00", 17 ) )
            {
                type = SUB_TYPE_SSA2_4;
                break;
            }
            else if( !strncasecmp( s, "Dialogue: Marked", 16  ) )
            {
                type = SUB_TYPE_SSA2_4;
                break;
            }
            else if( !strncasecmp( s, "Dialogue:", 9  ) )
            {
                type = SUB_TYPE_ASS;
                break;
            }
            else if( strcasestr( s, "[INFORMATION]" ) )
            {
                type = SUB_TYPE_SUBVIEWER; /* I hope this will work */
                break;
            }
            else if( sscanf( s, "%d:%d:%d.%d %d:%d:%d",
                             &i_dummy, &i_dummy, &i_dummy, &i_dummy,
                             &i_dummy, &i_dummy, &i_dummy ) == 7 ||
                     sscanf( s, "@%d @%d", &i_dummy, &i_dummy) == 2)
            {
                type = SUB_TYPE_JACOSUB;
                break;
            }
            else if( sscanf( s, "%d:%d:%d.%d,%d:%d:%d.%d",
                             &i_dummy, &i_dummy, &i_dummy, &i_dummy,
                             &i_dummy, &i_dummy, &i_dummy, &i_dummy ) == 8 )
            {
                type = SUB_TYPE_SBV;
                break;
            }
            else if( sscanf( s, "%d:%d:%d:", &i_dummy, &i_dummy, &i_dummy ) == 3 ||
                     sscanf( s, "%d:%d:%d ", &i_dummy, &i_dummy, &i_dummy ) == 3 )
            {
                type = SUB_TYPE_VPLAYER;
                break;
            }
            else if( sscanf( s, "{T %d:%d:%d:%d", &i_dummy, &i_dummy,
                             &i_dummy, &i_dummy ) == 4 )
            {
                type = SUB_TYPE_DVDSUBTITLE;
                break;
            }
            else if( sscanf( s, "[%d:%d:%d]%c",
                             &i_dummy, &i_dummy, &i_dummy, &p_dummy ) == 4 )
            {
                type = SUB_TYPE_DKS;
                break;
            }
            else if( strstr( s, "*** START SCRIPT" ) )
            {
                type = SUB_TYPE_SUBVIEW1;
                break;
            }
            else if( sscanf( s, "[%d][%d]", &i_dummy, &i_dummy ) == 2 ||
                     sscanf( s, "[%d][]", &i_dummy ) == 1)
            {
                type = SUB_TYPE_MPL2;
                break;
            }
            else if( sscanf (s, "FORMAT=%d", &i_dummy) == 1 ||
                     ( sscanf (s, "FORMAT=TIM%c", &p_dummy) == 1
                       && p_dummy =='E' ) )
            {
                type = SUB_TYPE_MPSUB;
                break;
            }
            else if( sscanf( s, "-->> %d", &i_dummy) == 1 )
            {
                type = SUB_TYPE_AQT;
                break;
            }
            else if( sscanf( s, "%d,%d,", &i_dummy, &i_dummy ) == 2 )
            {
                type = SUB_TYPE_PJS;
                break;
            }
            else if( sscanf( s, "{%d:%d:%d}",
                             &i_dummy, &i_dummy, &i_dummy ) == 3 )
            {
                type = SUB_TYPE_PSB;
                break;
            }
            else if( strcasestr( s, "<time" ) )
            {
                type = SUB_TYPE_RT;
                break;
            }

            free( s );
            s = NULL;
        }

        free( s );
    }

    return type;
}

int Open (const char * content, const uint64_t size, const float fps, enum subtitle_type_e type, demux_sys_t **res)
{
    if (size < 16) {
        printf("File is too small\n");
        return FAIL;
    }

    demux_sys_t *p_sys = malloc( sizeof( demux_sys_t ) );
    if( p_sys == NULL )
        return FAIL_MEM;

    p_sys->subtitles.i_current= 0;
    p_sys->subtitles.i_count  = 0;
    p_sys->subtitles.p_array  = NULL;

    p_sys->props.psz_header = NULL;
    p_sys->props.i_microsecperframe = (fps == 0) ? 0 : (1000.f / fps);
    p_sys->props.i_type = type;

    /* Quit on unknown subtitles */
    if( p_sys->props.i_type == SUB_TYPE_UNKNOWN )
    {
        printf("Unknown subtitle format\n");
        free( p_sys );
        return UNKNOWN_FORMAT;
    }

    int  (*pf_read)( subs_properties_t *, text_t *, subtitle_t*, size_t ) = NULL;
    for( int i = 0; ; i++ )
    {
        if( sub_read_subtitle_function[i].i_type == p_sys->props.i_type )
        {
            //printf("Detected %s format\n", sub_read_subtitle_function[i].psz_name);
            pf_read = sub_read_subtitle_function[i].pf_read;
            break;
        }
    }

    /* Load the whole file */
    text_t txtlines;
    stream_t stream = {content, size, 0};
    TextLoad( &txtlines, &stream );

    /* Parse it */
    for( size_t i_max = 0; i_max < SIZE_MAX - 500 * sizeof(subtitle_t); )
    {
        if( p_sys->subtitles.i_count >= i_max )
        {
            i_max += 500;
            subtitle_t *p_realloc = realloc( p_sys->subtitles.p_array, sizeof(subtitle_t) * i_max );
            if( p_realloc == NULL )
            {
                TextUnload( &txtlines );
                Close( p_sys );
                return FAIL_MEM;
            }
            p_sys->subtitles.p_array = p_realloc;
        }

        int code = pf_read( &p_sys->props, &txtlines,
                            &p_sys->subtitles.p_array[p_sys->subtitles.i_count],
                            p_sys->subtitles.i_count );
        if (code == ZERO_FPS) {
            TextUnload( &txtlines );
            Close( p_sys );
            printf("FPS must be set\n");
            return ZERO_FPS;
        } else if( code )
            break;

        p_sys->subtitles.i_count++;
    }
    /* Unload */
    TextUnload( &txtlines );

    printf("Loaded %zu subtitles\n", p_sys->subtitles.i_count);

    p_sys->subtitles.i_current = 0;

    *res = p_sys;
    return SUCCESS;
}

/*****************************************************************************
 * Close: Close subtitle demux
 *****************************************************************************/
void Close( demux_sys_t *p_sys )
{
    for( size_t i = 0; i < p_sys->subtitles.i_count; i++ )
        free( p_sys->subtitles.p_array[i].psz_text );
    free( p_sys->subtitles.p_array );
    free( p_sys->props.psz_header );

    free( p_sys );
}

static int TextLoad( text_t *txt, stream_t *s )
{
    size_t i_line_max;

    /* init txt */
    i_line_max          = 500;
    txt->i_line_count   = 0;
    txt->i_line         = 0;
    txt->line           = calloc( i_line_max, sizeof( char * ) );
    if( !txt->line )
        return FAIL_MEM;

    /* load the complete file */
    for( ;; )
    {
        char *psz = ReadLine( s );

        if( psz == NULL )
            break;

        txt->line[txt->i_line_count] = psz;
        if( txt->i_line_count + 1 >= i_line_max )
        {
            i_line_max += 100;
            char **p_realloc = realloc( txt->line, i_line_max * sizeof( char * ) );
            if( p_realloc == NULL )
                return FAIL_MEM;
            txt->line = p_realloc;
        }
        txt->i_line_count++;
    }

    if( txt->i_line_count == 0 )
    {
        free( txt->line );
        return FAIL;
    }

    return SUCCESS;
}
static void TextUnload( text_t *txt )
{
    if( txt->i_line_count )
    {
        for( size_t i = 0; i < txt->i_line_count; i++ )
            free( txt->line[i] );
        free( txt->line );
    }
    txt->i_line       = 0;
    txt->i_line_count = 0;
}

static char *TextGetLine( text_t *txt )
{
    if( txt->i_line >= txt->i_line_count )
        return( NULL );

    return txt->line[txt->i_line++];
}
static void TextPreviousLine( text_t *txt )
{
    if( txt->i_line > 0 )
        txt->i_line--;
}

/*****************************************************************************
 * Specific Subtitle function
 *****************************************************************************/
/* ParseMicroDvd:
 *  Format:
 *      {n1}{n2}Line1|Line2|Line3....
 *  where n1 and n2 are the video frame number (n2 can be empty)
 */
static int ParseMicroDvd( subs_properties_t *p_props,
                          text_t *txt, subtitle_t *p_subtitle,
                          size_t i_idx )
{
    char *psz_text;
    int  i_start;
    int  i_stop;
    int  i;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        if( !s )
            return FAIL;

        psz_text = malloc( strlen(s) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        i_start = 0;
        i_stop  = -1;
        if( sscanf( s, "{%d}{}%[^\r\n]", &i_start, psz_text ) == 2 ||
            sscanf( s, "{%d}{%d}%[^\r\n]", &i_start, &i_stop, psz_text ) == 3)
        {
            if( i_start != 1 || i_stop != 1 )
                break;

            /* We found a possible setting of the framerate "{1}{1}23.976" */
            /* Check if it's usable, and if the sub-original-fps is not set */
            float f_fps = strtof( psz_text, NULL );
            if( f_fps > 0.f)
                p_props->i_microsecperframe = (1000.f / f_fps);
        }
        free( psz_text );
    }

    /* replace | by \n */
    for( i = 0; psz_text[i] != '\0'; i++ )
    {
        if( psz_text[i] == '|' )
            psz_text[i] = '\n';
    }

    if (p_props->i_microsecperframe == 0) {
        free( psz_text );
        return ZERO_FPS;
    }

    /* */
    p_subtitle->i_start  = i_start * p_props->i_microsecperframe;
    p_subtitle->i_stop   = i_stop >= 0 ? (i_stop  * p_props->i_microsecperframe) : -1;
    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

/* ParseSubRipSubViewer
 *  Format SubRip
 *      n
 *      h1:m1:s1,d1 --> h2:m2:s2,d2
 *      Line1
 *      Line2
 *      ....
 *      [Empty line]
 *  Format SubViewer v1/v2
 *      h1:m1:s1.d1,h2:m2:s2.d2
 *      Line1[br]Line2
 *      Line3
 *      ...
 *      [empty line]
 *  We ignore line number for SubRip
 */
static int ParseSubRipSubViewer( subs_properties_t *p_props,
                                 text_t *txt, subtitle_t *p_subtitle,
                                 int (* pf_parse_timing)(subtitle_t *, const char *),
                                 bool b_replace_br )
{
    char    *psz_text;

    for( ;; )
    {
        const char *s = TextGetLine( txt );

        if( !s )
            return FAIL;

        if( pf_parse_timing( p_subtitle, s) == SUCCESS &&
            p_subtitle->i_start < p_subtitle->i_stop )
        {
            break;
        }
    }

    /* Now read text until an empty line */
    psz_text = strdup("");
    if( !psz_text )
        return FAIL;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        size_t i_len;
        size_t i_old;

        i_len = s ? strlen( s ) : 0;
        if( i_len <= 0 )
        {
            p_subtitle->psz_text = psz_text;
            return SUCCESS;
        }

        i_old = strlen( psz_text );
        psz_text = realloc( psz_text, i_old + i_len + 1 + 1 );
        if( !psz_text )
        {
            return FAIL_MEM;
        }
        strcat( psz_text, s );
        strcat( psz_text, "\n" );

        /* replace [br] by \n */
        if( b_replace_br )
        {
            char *p;

            while( ( p = strstr( psz_text, "[br]" ) ) )
            {
                *p++ = '\n';
                memmove( p, &p[3], strlen(&p[3])+1 );
            }
        }
    }
}

/* subtitle_ParseSubRipTimingValue
 * Parses SubRip timing value.
 */
static int subtitle_ParseSubRipTimingValue(vlc_tick_t *timing_value,
                                           const char *s)
{
    int h1, m1, s1, d1 = 0;

    if ( sscanf( s, "%d:%d:%d,%d",
                 &h1, &m1, &s1, &d1 ) == 4 ||
         sscanf( s, "%d:%d:%d.%d",
                 &h1, &m1, &s1, &d1 ) == 4 ||
         sscanf( s, "%d:%d:%d",
                 &h1, &m1, &s1) == 3 )
    {
        (*timing_value) = ( h1 * 3600 + m1 * 60 + s1) * 1000 + d1;

        return SUCCESS;
    }

    return FAIL;
}

/* subtitle_ParseSubRipTiming
 * Parses SubRip timing.
 */
static int subtitle_ParseSubRipTiming( subtitle_t *p_subtitle,
                                       const char *s )
{
    int i_result = FAIL;
    char *psz_start, *psz_stop;
    psz_start = malloc( strlen(s) + 1 );
    psz_stop = malloc( strlen(s) + 1 );

    if( sscanf( s, "%s --> %s", psz_start, psz_stop) == 2 &&
        subtitle_ParseSubRipTimingValue( &p_subtitle->i_start, psz_start ) == SUCCESS &&
        subtitle_ParseSubRipTimingValue( &p_subtitle->i_stop,  psz_stop ) == SUCCESS )
    {
        i_result = SUCCESS;
    }

    free(psz_start);
    free(psz_stop);

    return i_result;
}
/* ParseSubRip
 */
static int  ParseSubRip( subs_properties_t *p_props,
                         text_t *txt, subtitle_t *p_subtitle,
                         size_t i_idx )
{
    return ParseSubRipSubViewer( p_props, txt, p_subtitle,
                                 &subtitle_ParseSubRipTiming,
                                 false );
}

/* subtitle_ParseSubViewerTiming
 * Parses SubViewer timing.
 */
static int subtitle_ParseSubViewerTiming( subtitle_t *p_subtitle,
                                   const char *s )
{
    int h1, m1, s1, d1, h2, m2, s2, d2;

    if( sscanf( s, "%d:%d:%d.%d,%d:%d:%d.%d",
                &h1, &m1, &s1, &d1, &h2, &m2, &s2, &d2) == 8 )
    {
        p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000 + d1;
        p_subtitle->i_stop  = ( h2 * 3600 + m2 * 60 + s2 ) * 1000 + d2;
        return SUCCESS;
    }
    return FAIL;
}

/* ParseSubViewer
 */
static int  ParseSubViewer( subs_properties_t *p_props,
                            text_t *txt, subtitle_t *p_subtitle,
                            size_t i_idx )
{
    return ParseSubRipSubViewer( p_props, txt, p_subtitle,
                                 &subtitle_ParseSubViewerTiming,
                                 true );
}

/* ParseSSA
 */
static int  ParseSSA( subs_properties_t *p_props,
                      text_t *txt, subtitle_t *p_subtitle,
                      size_t i_idx )
{
    size_t header_len = 0;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int h1, m1, s1, c1, h2, m2, s2, c2;
        char *psz_text, *psz_temp;
        char temp[16];

        if( !s )
            return FAIL;

        /* We expect (SSA2-4):
         * Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
         * Dialogue: Marked=0,0:02:40.65,0:02:41.79,Wolf main,Cher,0000,0000,0000,,Et les enregistrements de ses ondes delta ?
         *
         * SSA-1 is similar but only has 8 commas up untill the subtitle text. Probably the Effect field is no present, but not 100 % sure.
         */

        /* For ASS:
         * Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
         * Dialogue: Layer#,0:02:40.65,0:02:41.79,Wolf main,Cher,0000,0000,0000,,Et les enregistrements de ses ondes delta ?
         */

        /* The output text is - at least, not removing numbers - 18 chars shorter than the input text. */
        psz_text = malloc( strlen(s) );
        if( !psz_text )
            return FAIL_MEM;

        if( sscanf( s,
                    "Dialogue: %15[^,],%d:%d:%d.%d,%d:%d:%d.%d,%[^\r\n]",
                    temp,
                    &h1, &m1, &s1, &c1,
                    &h2, &m2, &s2, &c2,
                    psz_text ) == 10 )
        {
            /* The dec expects: ReadOrder, Layer, Style, Name, MarginL, MarginR, MarginV, Effect, Text */
            /* (Layer comes from ASS specs ... it's empty for SSA.) */
            if( p_props->i_type == SUB_TYPE_SSA1 )
            {
                /* SSA1 has only 8 commas before the text starts, not 9 */
                memmove( &psz_text[1], psz_text, strlen(psz_text)+1 );
                psz_text[0] = ',';
            }
            else
            {
                int i_layer = ( p_props->i_type == SUB_TYPE_ASS ) ? atoi( temp ) : 0;

                /* ReadOrder, Layer, %s(rest of fields) */
                if( asprintf( &psz_temp, "%zu,%d,%s", i_idx, i_layer, psz_text ) == -1 )
                {
                    free( psz_text );
                    return FAIL;
                }

                free( psz_text );
                psz_text = psz_temp;
            }

            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000 + ( c1 * 10 );
            p_subtitle->i_stop  = ( h2 * 3600 + m2 * 60 + s2 ) * 1000 + ( c2 * 10 );
            p_subtitle->psz_text = psz_text;
            return SUCCESS;
        }
        free( psz_text );

        /* All the other stuff we add to the header field */
        if( header_len == 0 && p_props->psz_header )
            header_len = strlen( p_props->psz_header );

        size_t s_len = strlen( s );
        p_props->psz_header = realloc( p_props->psz_header, header_len + s_len + 2 );
        if( !p_props->psz_header )
            return FAIL_MEM;
        snprintf( p_props->psz_header + header_len, s_len + 2, "%s\n", s );
        header_len += s_len + 1;
    }
}

/* ParseVplayer
 *  Format
 *      h:m:s:Line1|Line2|Line3....
 *  or
 *      h:m:s Line1|Line2|Line3....
 */
static int ParseVplayer( subs_properties_t *p_props,
                         text_t *txt, subtitle_t *p_subtitle,
                         size_t i_idx )
{
    char *psz_text;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int h1, m1, s1;

        if( !s )
            return FAIL;

        psz_text = malloc( strlen( s ) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        if( sscanf( s, "%d:%d:%d%*c%[^\r\n]",
                    &h1, &m1, &s1, psz_text ) == 4 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000;
            p_subtitle->i_stop  = -1;
            break;
        }
        free( psz_text );
    }

    /* replace | by \n */
    for( size_t i = 0; psz_text[i] != '\0'; i++ )
    {
        if( psz_text[i] == '|' )
            psz_text[i] = '\n';
    }
    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

/* ParseSami
 */
static const char *ParseSamiSearch( text_t *txt,
                                    const char *psz_start, const char *psz_str )
{
    if( psz_start && strcasestr( psz_start, psz_str ) )
    {
        const char *s = strcasestr( psz_start, psz_str );
        return &s[strlen( psz_str )];
    }

    for( ;; )
    {
        const char *p = TextGetLine( txt );
        if( !p )
            return NULL;

        const char *s = strcasestr( p, psz_str );
        if( s != NULL )
            return &s[strlen( psz_str )];
    }
}
static int ParseSami( subs_properties_t *p_props,
                      text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    const char *s;
    int64_t i_start;

    unsigned int i_text;
    char text[8192]; /* Arbitrary but should be long enough */

    /* search "Start=" */
    s = ParseSamiSearch( txt, p_props->sami.psz_start, "Start=" );
    p_props->sami.psz_start = NULL;
    if( !s )
        return FAIL;

    /* get start value */
    char *psz_end;
    i_start = strtol( s, &psz_end, 0 );
    s = psz_end;

    /* search <P */
    if( !( s = ParseSamiSearch( txt, s, "<P" ) ) )
        return FAIL;

    /* search > */
    if( !( s = ParseSamiSearch( txt, s, ">" ) ) )
        return FAIL;

    i_text = 0;
    text[0] = '\0';
    /* now get all txt until  a "Start=" line */
    for( ;; )
    {
        char c = '\0';
        /* Search non empty line */
        while( s && *s == '\0' )
            s = TextGetLine( txt );
        if( !s )
            break;

        if( *s == '<' )
        {
            if( !strncasecmp( s, "<br", 3 ) )
            {
                c = '\n';
            }
            else if( strcasestr( s, "Start=" ) )
            {
                p_props->sami.psz_start = s;
                break;
            }
            s = ParseSamiSearch( txt, s, ">" );
        }
        else if( !strncmp( s, "&nbsp;", 6 ) )
        {
            c = ' ';
            s += 6;
        }
        else if( *s == '\t' )
        {
            c = ' ';
            s++;
        }
        else
        {
            c = *s;
            s++;
        }
        if( c != '\0' && i_text+1 < sizeof(text) )
        {
            text[i_text++] = c;
            text[i_text] = '\0';
        }
    }

    p_subtitle->i_start = i_start;
    p_subtitle->i_stop  = -1;
    p_subtitle->psz_text = strdup( text );

    return SUCCESS;
}

/* ParseDVDSubtitle
 *  Format
 *      {T h1:m1:s1:c1
 *      Line1
 *      Line2
 *      ...
 *      }
 * TODO it can have a header
 *      { HEAD
 *          ...
 *          CODEPAGE=...
 *          FORMAT=...
 *          LANG=English
 *      }
 *      LANG support would be cool
 *      CODEPAGE is probably mandatory FIXME
 */
static int ParseDVDSubtitle(subs_properties_t *p_props,
                            text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char *psz_text;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int h1, m1, s1, c1;

        if( !s )
            return FAIL;

        if( sscanf( s,
                    "{T %d:%d:%d:%d",
                    &h1, &m1, &s1, &c1 ) == 4 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000 + ( c1 * 10 );
            p_subtitle->i_stop = -1;
            break;
        }
    }

    /* Now read text until a line containing "}" */
    psz_text = strdup("");
    if( !psz_text )
        return FAIL;
    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int i_len;
        int i_old;

        if( !s )
        {
            free( psz_text );
            return FAIL;
        }

        i_len = strlen( s );
        if( i_len == 1 && s[0] == '}')
        {
            p_subtitle->psz_text = psz_text;
            return SUCCESS;
        }

        i_old = strlen( psz_text );
        psz_text = realloc( psz_text, i_old + i_len + 1 + 1 );
        if( !psz_text )
            return FAIL_MEM;
        strcat( psz_text, s );
        strcat( psz_text, "\n" );
    }
}

/* ParseMPL2
 *  Format
 *     [n1][n2]Line1|Line2|Line3...
 *  where n1 and n2 are the video frame number (n2 can be empty)
 */
static int ParseMPL2(subs_properties_t *p_props,
                     text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char *psz_text;
    int i;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int i_start;
        int i_stop;

        if( !s )
            return FAIL;

        psz_text = malloc( strlen(s) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        i_start = 0;
        i_stop  = -1;
        if( sscanf( s, "[%d][] %[^\r\n]", &i_start, psz_text ) == 2 ||
            sscanf( s, "[%d][%d] %[^\r\n]", &i_start, &i_stop, psz_text ) == 3)
        {
            p_subtitle->i_start = (i_start * 100);
            p_subtitle->i_stop  = i_stop >= 0 ? (i_stop  * 100) : -1;
            break;
        }
        free( psz_text );
    }

    for( i = 0; psz_text[i] != '\0'; )
    {
        /* replace | by \n */
        if( psz_text[i] == '|' )
            psz_text[i] = '\n';

        /* Remove italic */
        if( psz_text[i] == '/' && ( i == 0 || psz_text[i-1] == '\n' ) )
            memmove( &psz_text[i], &psz_text[i+1], strlen(&psz_text[i+1])+1 );
        else
            i++;
    }
    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int ParseAQT(subs_properties_t *p_props, text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{

    char *psz_text = strdup( "" );
    int i_old = 0;
    int i_firstline = 1;

    for( ;; )
    {
        int t; /* Time */

        const char *s = TextGetLine( txt );

        if( !s )
        {
            free( psz_text );
            return FAIL;
        }

        /* Data Lines */
        if( sscanf (s, "-->> %d", &t) == 1)
        {
            p_subtitle->i_start = (int64_t)t; /* * FPS*/
            p_subtitle->i_stop  = -1;

            /* Starting of a subtitle */
            if( i_firstline )
            {
                i_firstline = 0;
            }
            /* We have been too far: end of the subtitle, begin of next */
            else
            {
                TextPreviousLine( txt );
                break;
            }
        }
        /* Text Lines */
        else
        {
            i_old = strlen( psz_text ) + 1;
            psz_text = realloc( psz_text, i_old + strlen( s ) + 1 );
            if( !psz_text )
                 return FAIL_MEM;
            strcat( psz_text, s );
            strcat( psz_text, "\n" );
            if( txt->i_line == txt->i_line_count )
                break;
        }
    }
    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int ParsePJS(subs_properties_t *p_props,
                    text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{

    char *psz_text;
    int i;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int t1, t2;

        if( !s )
            return FAIL;

        psz_text = malloc( strlen(s) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        /* Data Lines */
        if( sscanf (s, "%d,%d,\"%[^\n\r]", &t1, &t2, psz_text ) == 3 )
        {
            /* 1/10th of second ? Frame based ? FIXME */
            p_subtitle->i_start = 10 * t1;
            p_subtitle->i_stop = 10 * t2;
            /* Remove latest " */
            psz_text[ strlen(psz_text) - 1 ] = '\0';

            break;
        }
        free( psz_text );
    }

    /* replace | by \n */
    for( i = 0; psz_text[i] != '\0'; i++ )
    {
        if( psz_text[i] == '|' )
            psz_text[i] = '\n';
    }

    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int ParseMPSub( subs_properties_t *p_props,
                       text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{

    char *psz_text = strdup( "" );

    if( !p_props->mpsub.b_inited )
    {
        p_props->mpsub.f_total = 0.0;
        p_props->mpsub.i_factor = 0;

        p_props->mpsub.b_inited = true;
    }

    for( ;; )
    {
        char p_dummy;
        char *psz_temp;

        const char *s = TextGetLine( txt );
        if( !s )
        {
            free( psz_text );
            return FAIL;
        }

        if( strstr( s, "FORMAT" ) )
        {
            if( sscanf (s, "FORMAT=TIM%c", &p_dummy ) == 1 && p_dummy == 'E')
            {
                p_props->mpsub.i_factor = 100;
                break;
            }

            psz_temp = malloc( strlen(s) );
            if( !psz_temp )
            {
                free( psz_text );
                return FAIL_MEM;
            }

            if( sscanf( s, "FORMAT=%[^\r\n]", psz_temp ) )
            {
                float f_fps = strtof( psz_temp, NULL );
                p_props->i_microsecperframe = f_fps;

                p_props->mpsub.i_factor = 1;
                free( psz_temp );
                break;
            }
            free( psz_temp );
        }

        /* Data Lines */
        float f1 = strtof( s, &psz_temp );
        if( *psz_temp )
        {
            float f2 = strtof( psz_temp, NULL );
            p_props->mpsub.f_total += f1 * p_props->mpsub.i_factor;
            p_subtitle->i_start = llroundf(10000.f * p_props->mpsub.f_total);
            p_props->mpsub.f_total += f2 * p_props->mpsub.i_factor;
            p_subtitle->i_stop = llroundf(10000.f * p_props->mpsub.f_total);
            break;
        }
    }

    for( ;; )
    {
        const char *s = TextGetLine( txt );

        if( !s )
        {
            free( psz_text );
            return FAIL;
        }

        size_t i_len = strlen( s );
        if( i_len == 0 )
            break;

        size_t i_old = strlen( psz_text );

        psz_text = realloc( psz_text, i_old + i_len + 1 + 1 );
        if( !psz_text )
             return FAIL_MEM;

        strcat( psz_text, s );
        strcat( psz_text, "\n" );
    }

    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int ParseJSS( subs_properties_t *p_props,
                     text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char         *psz_text, *psz_orig;
    char         *psz_text2, *psz_orig2;

    if( !p_props->jss.b_inited )
    {
        p_props->jss.i_comment = 0;
        p_props->jss.i_time_resolution = 30;
        p_props->jss.i_time_shift = 0;

        p_props->jss.b_inited = true;
    }

    /* Parse the main lines */
    for( ;; )
    {
        const char *s = TextGetLine( txt );
        if( !s )
            return FAIL;

        size_t line_length = strlen( s );
        psz_orig = malloc( line_length + 1 );
        if( !psz_orig )
            return FAIL_MEM;
        psz_text = psz_orig;

        /* Complete time lines */
        int h1, h2, m1, m2, s1, s2, f1, f2;
        if( sscanf( s, "%d:%d:%d.%d %d:%d:%d.%d %[^\n\r]",
                    &h1, &m1, &s1, &f1, &h2, &m2, &s2, &f2, psz_text ) == 9 )
        {
            p_subtitle->i_start = ( ( h1 *3600 + m1 * 60 + s1 ) +
                (int64_t)( ( f1 +  p_props->jss.i_time_shift ) / p_props->jss.i_time_resolution ) ) * 1000;
            p_subtitle->i_stop = ( ( h2 *3600 + m2 * 60 + s2 ) +
                (int64_t)( ( f2 +  p_props->jss.i_time_shift ) / p_props->jss.i_time_resolution ) ) * 1000;
            break;
        }
        /* Short time lines */
        else if( sscanf( s, "@%d @%d %[^\n\r]", &f1, &f2, psz_text ) == 3 )
        {
            p_subtitle->i_start =
                    ( (f1 + p_props->jss.i_time_shift ) / p_props->jss.i_time_resolution ) * 1000;
            p_subtitle->i_stop =
                    ( (f2 + p_props->jss.i_time_shift ) / p_props->jss.i_time_resolution ) * 1000;
            break;
        }
        /* General Directive lines */
        /* Only TIME and SHIFT are supported so far */
        else if( s[0] == '#' )
        {
            int h = 0, m =0, sec = 1, f = 1;
            unsigned shift = 1;
            int inv = 1;

            strcpy( psz_text, s );

            switch( toupper( (unsigned char)psz_text[1] ) )
            {
            case 'S':
                 shift = isalpha( (unsigned char)psz_text[2] ) ? 6 : 2 ;
                 if ( shift > line_length )
                     break;

                 if( sscanf( &psz_text[shift], "%d", &h ) )
                 {
                     /* Negative shifting */
                     if( h < 0 )
                     {
                         h *= -1;
                         inv = -1;
                     }

                     if( sscanf( &psz_text[shift], "%*d:%d", &m ) )
                     {
                         if( sscanf( &psz_text[shift], "%*d:%*d:%d", &sec ) )
                         {
                             sscanf( &psz_text[shift], "%*d:%*d:%*d.%d", &f );
                         }
                         else
                         {
                             h = 0;
                             sscanf( &psz_text[shift], "%d:%d.%d",
                                     &m, &sec, &f );
                             m *= inv;
                         }
                     }
                     else
                     {
                         h = m = 0;
                         sscanf( &psz_text[shift], "%d.%d", &sec, &f);
                         sec *= inv;
                     }
                     p_props->jss.i_time_shift = ( ( h * 3600 + m * 60 + sec )
                         * p_props->jss.i_time_resolution + f ) * inv;
                 }
                 break;

            case 'T':
                shift = isalpha( (unsigned char)psz_text[2] ) ? 8 : 2 ;
                if ( shift > line_length )
                    break;

                sscanf( &psz_text[shift], "%d", &p_props->jss.i_time_resolution );
                if( !p_props->jss.i_time_resolution )
                    p_props->jss.i_time_resolution = 30;
                break;
            }
            free( psz_orig );
            continue;
        }
        else
            /* Unkown type line, probably a comment */
        {
            free( psz_orig );
            continue;
        }
    }

    while( psz_text[ strlen( psz_text ) - 1 ] == '\\' )
    {
        const char *s2 = TextGetLine( txt );

        if( !s2 )
        {
            free( psz_orig );
            return FAIL;
        }

        size_t i_len = strlen( s2 );
        if( i_len == 0 )
            break;

        size_t i_old = strlen( psz_text );

        psz_text = realloc( psz_text, i_old + i_len + 1 );
        if( !psz_text )
             return FAIL_MEM;

        psz_orig = psz_text;
        strcat( psz_text, s2 );
    }

    /* Skip the blanks */
    while( *psz_text == ' ' || *psz_text == '\t' ) psz_text++;

    /* Parse the directives */
    if( isalpha( (unsigned char)*psz_text ) || *psz_text == '[' )
    {
        while( *psz_text && *psz_text != ' ' )
            ++psz_text;

        /* Directives are NOT parsed yet */
        /* This has probably a better place in a decoder ? */
        /* directive = malloc( strlen( psz_text ) + 1 );
           if( sscanf( psz_text, "%s %[^\n\r]", directive, psz_text2 ) == 2 )*/
    }

    /* Skip the blanks after directives */
    while( *psz_text == ' ' || *psz_text == '\t' ) psz_text++;

    /* Clean all the lines from inline comments and other stuffs */
    psz_orig2 = calloc( strlen( psz_text) + 1, 1 );
    psz_text2 = psz_orig2;

    for( ; *psz_text != '\0' && *psz_text != '\n' && *psz_text != '\r'; )
    {
        switch( *psz_text )
        {
        case '{':
            p_props->jss.i_comment++;
            break;
        case '}':
            if( p_props->jss.i_comment )
            {
                p_props->jss.i_comment = 0;
                if( (*(psz_text + 1 ) ) == ' ' ) psz_text++;
            }
            break;
        case '~':
            if( !p_props->jss.i_comment )
            {
                *psz_text2 = ' ';
                psz_text2++;
            }
            break;
        case ' ':
        case '\t':
            if( (*(psz_text + 1 ) ) == ' ' || (*(psz_text + 1 ) ) == '\t' )
                break;
            if( !p_props->jss.i_comment )
            {
                *psz_text2 = ' ';
                psz_text2++;
            }
            break;
        case '\\':
            if( (*(psz_text + 1 ) ) == 'n' )
            {
                *psz_text2 = '\n';
                psz_text++;
                psz_text2++;
                break;
            }
            if( ( toupper((unsigned char)*(psz_text + 1 ) ) == 'C' ) ||
                    ( toupper((unsigned char)*(psz_text + 1 ) ) == 'F' ) )
            {
                psz_text++;
                break;
            }
            if( (*(psz_text + 1 ) ) == 'B' || (*(psz_text + 1 ) ) == 'b' ||
                (*(psz_text + 1 ) ) == 'I' || (*(psz_text + 1 ) ) == 'i' ||
                (*(psz_text + 1 ) ) == 'U' || (*(psz_text + 1 ) ) == 'u' ||
                (*(psz_text + 1 ) ) == 'D' || (*(psz_text + 1 ) ) == 'N' )
            {
                psz_text++;
                break;
            }
            if( (*(psz_text + 1 ) ) == '~' || (*(psz_text + 1 ) ) == '{' ||
                (*(psz_text + 1 ) ) == '\\' )
                psz_text++;
            else if( ( *(psz_text + 1 ) == '\r' ||  *(psz_text + 1 ) == '\n' ) &&
                     *(psz_text + 1 ) != '\0' )
            {
                psz_text++;
            }
            break;
        default:
            if( !p_props->jss.i_comment )
            {
                *psz_text2 = *psz_text;
                psz_text2++;
            }
        }
        psz_text++;
    }

    p_subtitle->psz_text = psz_orig2;
    free( psz_orig );
    return SUCCESS;
}

static int ParsePSB( subs_properties_t *p_props,
                     text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char *psz_text;
    int i;

    for( ;; )
    {
        int h1, m1, s1;
        int h2, m2, s2;
        const char *s = TextGetLine( txt );

        if( !s )
            return FAIL;

        psz_text = malloc( strlen( s ) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        if( sscanf( s, "{%d:%d:%d}{%d:%d:%d}%[^\r\n]",
                    &h1, &m1, &s1, &h2, &m2, &s2, psz_text ) == 7 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000;
            p_subtitle->i_stop  = ( h2 * 3600 + m2 * 60 + s2 ) * 1000;
            break;
        }
        free( psz_text );
    }

    /* replace | by \n */
    for( i = 0; psz_text[i] != '\0'; i++ )
    {
        if( psz_text[i] == '|' )
            psz_text[i] = '\n';
    }
    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int64_t ParseRealTime( char *psz, int *h, int *m, int *s, int *f )
{
    if( *psz == '\0' ) return 0;
    if( sscanf( psz, "%d:%d:%d.%d", h, m, s, f ) == 4 ||
            sscanf( psz, "%d:%d.%d", m, s, f ) == 3 ||
            sscanf( psz, "%d.%d", s, f ) == 2 ||
            sscanf( psz, "%d:%d", m, s ) == 2 ||
            sscanf( psz, "%d", s ) == 1 )
    {
        return ((( *h * 60 + *m ) * 60 ) + *s ) * 1000 + (*f * 10);
    }
    else return FAIL;
}

static int ParseRealText( subs_properties_t *p_props,
                          text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char *psz_text = NULL;

    for( ;; )
    {
        int h1 = 0, m1 = 0, s1 = 0, f1 = 0;
        int h2 = 0, m2 = 0, s2 = 0, f2 = 0;
        const char *s = TextGetLine( txt );
        free( psz_text );

        if( !s )
            return FAIL;

        psz_text = malloc( strlen( s ) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        /* Find the good begining. This removes extra spaces at the beginning
           of the line.*/
        char *psz_temp = strcasestr( s, "<time");
        if( psz_temp != NULL )
        {
            char psz_end[12], psz_begin[12];
            /* Line has begin and end */
            if( ( sscanf( psz_temp,
                  "<%*[t|T]ime %*[b|B]egin=\"%11[^\"]\" %*[e|E]nd=\"%11[^\"]%*[^>]%[^\n\r]",
                            psz_begin, psz_end, psz_text) != 3 ) &&
                    /* Line has begin and no end */
                    ( sscanf( psz_temp,
                              "<%*[t|T]ime %*[b|B]egin=\"%11[^\"]\"%*[^>]%[^\n\r]",
                              psz_begin, psz_text ) != 2) )
                /* Line is not recognized */
            {
                continue;
            }

            /* Get the times */
            int64_t i_time = ParseRealTime( psz_begin, &h1, &m1, &s1, &f1 );
            p_subtitle->i_start = i_time >= 0 ? i_time : 0;

            i_time = ParseRealTime( psz_end, &h2, &m2, &s2, &f2 );
            p_subtitle->i_stop = i_time >= 0 ? i_time : -1;
            break;
        }
    }

    /* Get the following Lines */
    for( ;; )
    {
        const char *s = TextGetLine( txt );

        if( !s )
        {
            free( psz_text );
            return FAIL;
        }

        size_t i_len = strlen( s );
        if( i_len == 0 ) break;

        if( strcasestr( s, "<time" ) ||
            strcasestr( s, "<clear/") )
        {
            TextPreviousLine( txt );
            break;
        }

        size_t i_old = strlen( psz_text );

        psz_text = realloc( psz_text, i_old + i_len + 1 + 1 );
        if( !psz_text )
            return FAIL_MEM;

        strcat( psz_text, s );
        strcat( psz_text, "\n" );
    }

    /* Remove the starting ">" that remained after the sscanf */
    memmove( &psz_text[0], &psz_text[1], strlen( psz_text ) );

    p_subtitle->psz_text = psz_text;

    return SUCCESS;
}

static int ParseDKS( subs_properties_t *p_props,
                     text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{

    char *psz_text;

    for( ;; )
    {
        int h1, m1, s1;
        int h2, m2, s2;
        char *s = TextGetLine( txt );

        if( !s )
            return FAIL;

        psz_text = malloc( strlen( s ) + 1 );
        if( !psz_text )
            return FAIL_MEM;

        if( sscanf( s, "[%d:%d:%d]%[^\r\n]",
                    &h1, &m1, &s1, psz_text ) == 4 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000;

            s = TextGetLine( txt );
            if( !s )
            {
                free( psz_text );
                return FAIL;
            }

            if( sscanf( s, "[%d:%d:%d]", &h2, &m2, &s2 ) == 3 )
                p_subtitle->i_stop  = (h2 * 3600 + m2 * 60 + s2 ) * 1000;
            else
                p_subtitle->i_stop  = -1;
            break;
        }
        free( psz_text );
    }

    /* replace [br] by \n */
    char *p;
    while( ( p = strstr( psz_text, "[br]" ) ) )
    {
        *p++ = '\n';
        memmove( p, &p[3], strlen(&p[3])+1 );
    }

    p_subtitle->psz_text = psz_text;
    return SUCCESS;
}

static int ParseSubViewer1( subs_properties_t *p_props,
                            text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char *psz_text;

    for( ;; )
    {
        int h1, m1, s1;
        int h2, m2, s2;
        char *s = TextGetLine( txt );

        if( !s )
            return FAIL;

        if( sscanf( s, "[%d:%d:%d]", &h1, &m1, &s1 ) == 3 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000;

            s = TextGetLine( txt );
            if( !s )
                return FAIL;

            psz_text = strdup( s );
            if( !psz_text )
                return FAIL;

            s = TextGetLine( txt );
            if( !s )
            {
                free( psz_text );
                return FAIL;
            }

            if( sscanf( s, "[%d:%d:%d]", &h2, &m2, &s2 ) == 3 )
                p_subtitle->i_stop  = ( h2 * 3600 + m2 * 60 + s2 ) * 1000;
            else
                p_subtitle->i_stop  = -1;

            break;
        }
    }

    p_subtitle->psz_text = psz_text;

    return SUCCESS;
}

static int ParseCommonSBV( subs_properties_t *p_props,
                           text_t *txt, subtitle_t *p_subtitle, size_t i_idx )
{
    char        *psz_text;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        int h1 = 0, m1 = 0, s1 = 0, d1 = 0;
        int h2 = 0, m2 = 0, s2 = 0, d2 = 0;

        if( !s )
            return FAIL;

        if( sscanf( s,"%d:%d:%d.%d,%d:%d:%d.%d",
                    &h1, &m1, &s1, &d1,
                    &h2, &m2, &s2, &d2 ) == 8 )
        {
            p_subtitle->i_start = ( h1 * 3600 + m1 * 60 + s1 ) * 1000 + d1;

            p_subtitle->i_stop  = ( h2 * 3600 + m2 * 60 + s2 ) * 1000 + d2;
            if( p_subtitle->i_start < p_subtitle->i_stop )
                break;
        }
    }

    /* Now read text until an empty line */
    psz_text = strdup("");
    if( !psz_text )
        return FAIL;

    for( ;; )
    {
        const char *s = TextGetLine( txt );
        size_t i_len;
        size_t i_old;

        i_len = s ? strlen( s ) : 0;
        if( i_len <= 0 )
        {
            p_subtitle->psz_text = psz_text;
            return SUCCESS;
        }

        i_old = strlen( psz_text );
        psz_text = realloc( psz_text, i_old + i_len + 1 + 1 );
        if( !psz_text )
            return FAIL_MEM;

        strcat( psz_text, s );
        strcat( psz_text, "\n" );
    }
}