#include "vlc_demux.h"
#include "subtitle.h"

/****************************************************************************
 * WebAssembly and JS bridges
 ****************************************************************************/
#include <emscripten.h>

EM_JS(void, write_type, (const char* str), {
    globalThis.wasm_parsed = {type: UTF8ToString(str), data: []};
});

EM_JS(void, write_sub_entity, (const char* str, int start, int stop), {
    globalThis.wasm_parsed.data.push({start: start, stop: stop, text: UTF8ToString(str)});
});


/****************************************************************************
 * Entry point
 ****************************************************************************/
static int parseLibAss(demux_sys_t *t);

int parse(const char * content, uint32_t size, float fps) {
    printf("Content size: %d bytes\n", (int) size);
    printf("FPS: %f\n", fps);

    demux_t d;
    d.fps = fps;
    d.p_sys = NULL;
    d.s = malloc(sizeof(stream_t));
    {
        const stream_t temp = {content, size, 0};
        memcpy(d.s, &temp, sizeof(stream_t));
    }

    const int code = Open((vlc_object_t*) &d);
    if (code != VLC_SUCCESS)
        return code;

    demux_sys_t *t = (demux_sys_t*) d.p_sys;
    if (t != NULL) {
        const enum subtitle_type_e type = t->props.i_type;

        const char * type_name = NULL;
        switch (type) {
            case SUB_TYPE_MICRODVD:
                type_name = "MicroDVD";
                break;
            case SUB_TYPE_SUBRIP:
                type_name = "SubRIP";
                break;
            case SUB_TYPE_SSA1:
                type_name = "SSA-1";
                break;
            case SUB_TYPE_SSA2_4:
                type_name = "SSA-2/3/4";
                break;
            case SUB_TYPE_ASS:
                type_name = "SSA/ASS";
                break;
            case SUB_TYPE_VPLAYER:
                type_name = "VPlayer";
                break;
            case SUB_TYPE_SAMI:
                type_name = "SAMI";
                break;
            case SUB_TYPE_SUBVIEWER:
                type_name = "SubViewer";
                break;
            case SUB_TYPE_DVDSUBTITLE:
                type_name = "DVDSubtitle";
                break;
            case SUB_TYPE_MPL2:
                type_name = "MPL2";
                break;
            case SUB_TYPE_AQT:
                type_name = "AQTitle";
                break;
            case SUB_TYPE_PJS:
                type_name = "PhoenixSub";
                break;
            case SUB_TYPE_MPSUB:
                type_name = "MPSub";
                break;
            case SUB_TYPE_JACOSUB:
                type_name = "JacoSub";
                break;
            case SUB_TYPE_PSB:
                type_name = "PowerDivx";
                break;
            case SUB_TYPE_RT:
                type_name = "RealText";
                break;
            case SUB_TYPE_DKS:
                type_name = "DKS";
                break;
            case SUB_TYPE_SUBVIEW1:
                type_name = "Subviewer 1";
                break;
            case SUB_TYPE_SBV:
                type_name = "SBV";
                break;
            default:
                type_name = "unknown type";
                break;
        }
        write_type(type_name);

        if( type == SUB_TYPE_SSA1 ||
            type == SUB_TYPE_SSA2_4 ||
            type == SUB_TYPE_ASS )
        {
            parseLibAss(t);
        }

        for (size_t i = 0; i < t->subtitles.i_count; ++i) {
            vlc_tick_t start = t->subtitles.p_array[i].i_start;
            vlc_tick_t stop = t->subtitles.p_array[i].i_stop;
            const char *text = t->subtitles.p_array[i].psz_text;
//            printf("%02d:%02d:%02d.%03d, %02d:%02d:%02d.%03d -> %s\n",
//                   (int) start / 1000 / 60 / 60, ((int) start / 1000 / 60) % 60, ((int) start / 1000) % 60,
//                   (int) start % 1000,
//                   (int) stop / 1000 / 60 / 60, ((int) stop / 1000 / 60) % 60, ((int) stop / 1000) % 60,
//                   (int) stop % 1000,
//                   text);
            write_sub_entity(text, start, stop);
        }
//        if (t->props.psz_header) {
//            printf("%s\n", t->props.psz_header);
//        }
        Close((vlc_object_t*) &d);
        printf("Success\n");
        return VLC_SUCCESS;
    } else {
        printf("Fail\n");
        return code;
    }
}

#include "libass/ass.h"

static int parseLibAss(demux_sys_t *t) {
    ASS_Library *p_library = ass_library_init();
    if( !p_library )
    {
        msg_Warn(NULL, "Libass library creation failed");
        return VLC_EGENERIC;
    }

    ASS_Track *p_track = ass_new_track( p_library );
    if( !p_track )
    {
        ass_library_done(p_library);
        msg_Warn(NULL, "Libass track creation failed");
        return VLC_EGENERIC;
    }
    ass_process_codec_private( p_track, t->props.psz_header, strlen(t->props.psz_header) );

    for (int i = 0; i < t->subtitles.i_count; ++i) {
        subtitle_t *it = &t->subtitles.p_array[i];
        ass_process_chunk(p_track,
                it->psz_text, strlen(it->psz_text),
                it->i_start, it->i_stop - it->i_start);
        // copy libass parsed strings later
        free(it->psz_text);
        it->psz_text = NULL;
    }

    if (t->subtitles.i_count < p_track->n_events) {
        t->subtitles.i_count = p_track->n_events;
        t->subtitles.p_array = realloc(t->subtitles.p_array, sizeof(subtitle_t) * t->subtitles.i_count);
    }
    for (int j = 0; j < p_track->n_events; ++j) {
        subtitle_t *it = &t->subtitles.p_array[j];
        const ASS_Event *e = &p_track->events[j];
        it->i_start = e->Start;
        it->i_stop = e->Start + e->Duration;
        it->psz_text = strdup(e->Text);

        // "\\N" -> "\n"
        char *sub = it->psz_text;
        while ((sub = strcasestr(sub, "\\n")) != NULL) {
            *sub = '\n';
            sub++;
            strcpy(sub, sub + 1);
        }
    }

    // TODO: call parse_events(renderer, event)
    // It generates subtitle styles (bold, italic, color, ...)
    // Skip styles for now

    ass_free_track(p_track);
    ass_library_done(p_library);
    return VLC_SUCCESS;
}
