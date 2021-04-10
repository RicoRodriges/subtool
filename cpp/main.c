#include <stdio.h>
#include "subtitle.h"

#include <emscripten.h>

EM_JS(void, write_type, (const char* str), {
    window.parsed = {type: UTF8ToString(str)};
});

EM_JS(void, write_sub_entity, (const char* str, int32_t start, int32_t stop), {
    if (!window.parsed.data) {
        window.parsed.data = [];
    }
    window.parsed.data.push({start: start, stop: stop, text: UTF8ToString(str)});
});

int libAssTest(demux_sys_t *t);

//int main() {
//    printf("Hello, World!\n");
//
//    enum subtitle_type_e type = DetectType(TEXT, sizeof(TEXT) - 1);
//    for( int i = 0; ; i++ )
//    {
//        if( sub_read_subtitle_function[i].i_type == type )
//        {
//            printf("Detected %s format\n", sub_read_subtitle_function[i].psz_name);
//            write_type(sub_read_subtitle_function[i].psz_name);
//            break;
//        }
//    }
//
//
//    demux_sys_t *t = NULL;
//    Open(TEXT, sizeof(TEXT) - 1, 23, type, &t);
//    printf("\n\n");
//    if (t != NULL) {
//        for (size_t i = 0; i < t->subtitles.i_count; ++i) {
//            vlc_tick_t start = t->subtitles.p_array[i].i_start;
//            vlc_tick_t stop = t->subtitles.p_array[i].i_stop;
//            char *text = t->subtitles.p_array[i].psz_text;
//            printf("%02d:%02d:%02d.%03d, %02d:%02d:%02d.%03d -> %s\n",
//                   (int) start / 1000 / 60 / 60, ((int) start / 1000 / 60) % 60, ((int) start / 1000) % 60, (int) start % 60,
//                   (int) stop / 1000 / 60 / 60, ((int) stop / 1000 / 60) % 60, ((int) stop / 1000) % 60, (int) stop % 60,
//                   text);
//        }
//        Close(t);
//    } else {
//        printf("NULL!!!");
//    }
//    printf("END\n");
//    return 0;
//}

int parse(const char * content, uint32_t size, float fps) {
    printf("Content size: %d\n", (int) size);
    printf("FPS: %f\n", fps);
    enum subtitle_type_e type = DetectType(content, size);
    for( int i = 0; ; i++ )
    {
        if( sub_read_subtitle_function[i].i_type == type )
        {
            write_type(sub_read_subtitle_function[i].psz_name);
            break;
        }
    }

    demux_sys_t *t = NULL;
    int code = Open(content, size, fps <= 0 ? 0 : fps , type, &t);
    if (t != NULL) {
        if( type == SUB_TYPE_SSA1 ||
            type == SUB_TYPE_SSA2_4 ||
            type == SUB_TYPE_ASS )
        {
            libAssTest(t);
        }

        for (size_t i = 0; i < t->subtitles.i_count; ++i) {
            vlc_tick_t start = t->subtitles.p_array[i].i_start;
            vlc_tick_t stop = t->subtitles.p_array[i].i_stop;
            const char *text = t->subtitles.p_array[i].psz_text;
//            printf("%02d:%02d:%02d.%03d, %02d:%02d:%02d.%03d -> %s\n",
//                   (int) start / 1000 / 60 / 60, ((int) start / 1000 / 60) % 60, ((int) start / 1000) % 60,
//                   (int) start % 60,
//                   (int) stop / 1000 / 60 / 60, ((int) stop / 1000 / 60) % 60, ((int) stop / 1000) % 60,
//                   (int) stop % 60,
//                   text);
            write_sub_entity(text, start, stop);
        }
        if (t->props.psz_header) {
            printf("%s\n", t->props.psz_header);
        }
        Close(t);
        printf("Success\n");
        return SUCCESS;
    } else {
        printf("Fail\n");
        return code;
    }
}

#include "libass/ass.h"
#include <string.h>
#include <malloc.h>

int libAssTest(demux_sys_t *t) {
    ASS_Library *p_library = ass_library_init();
    if( !p_library )
    {
        printf("Libass library creation failed\n");
        return FAIL;
    }

    ASS_Track *p_track = ass_new_track( p_library );
    if( !p_track )
    {
        ass_library_done(p_library);
        printf("Libass track creation failed\n");
        return FAIL;
    }
    ass_process_codec_private( p_track, t->props.psz_header, strlen(t->props.psz_header) );

    for (int i = 0; i < t->subtitles.i_count; ++i) {
        ass_process_chunk(p_track,
                t->subtitles.p_array[i].psz_text, strlen(t->subtitles.p_array[i].psz_text),
                t->subtitles.p_array[i].i_start, t->subtitles.p_array[i].i_stop - t->subtitles.p_array[i].i_start);
        free(t->subtitles.p_array[i].psz_text);
        t->subtitles.p_array[i].psz_text = NULL;
    }

    if (t->subtitles.i_count < p_track->n_events) {
        t->subtitles.i_count = p_track->n_events;
        t->subtitles.p_array = realloc(t->subtitles.p_array, sizeof(subtitle_t) * t->subtitles.i_count);
    }
    for (int j = 0; j < p_track->n_events; ++j) {
        t->subtitles.p_array[j].i_start = p_track->events[j].Start;
        t->subtitles.p_array[j].i_stop = p_track->events[j].Start + p_track->events[j].Duration;
        t->subtitles.p_array[j].psz_text = strdup(p_track->events[j].Text);

        // "\\N" -> "\n"
        char *sub = t->subtitles.p_array[j].psz_text;
        while ((sub = strcasestr(sub, "\\n")) != NULL) {
            *sub = '\n';
            sub++;
            strcpy(sub, sub + 1);
        }
    }

    // TODO: call parse_events(renderer, event)


    ass_free_track(p_track);
    ass_library_done(p_library);
    return SUCCESS;
}
