#pragma once

#include "vlc_common.h"

typedef struct
{
    const char * content;
    const uint64_t size;

    uint64_t current;
} stream_t;

typedef struct {
    void      *p_sys; // demux_sys_t
    float     fps;
    stream_t  *s;
} demux_t;

#define var_GetFloat(x, y) (((demux_t *)x)->fps)
#define var_SetFloat(x, y, v) (((demux_t *)x)->fps = v)
#define var_CreateGetFloat var_GetFloat

char * vlc_stream_ReadLine( stream_t *s );
void vlc_stream_Delete( stream_t *s );
stream_t * vlc_stream_MemoryNew( demux_t *d );
