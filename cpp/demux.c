#include "vlc_demux.h"

char * vlc_stream_ReadLine( stream_t *s )
{
    const char * const begin = s->content + s->current;
    const char * const end = s->content + s->size;

    if (begin >= end) return NULL;

    const char *line_end = begin;
    while (line_end != end) {
        // handles \r, \n and \r\n
        const char next_char = (line_end + 1 != end) ? *(line_end + 1) : '\0';
        if (*line_end == '\n' || (*line_end == '\r' && next_char != '\n')) break;
        ++line_end;
    }

    s->current = line_end - s->content + 1;

    const size_t size = line_end - begin;
    char * const buf = malloc(size + 1);
    if (size > 0)
        memcpy(buf, begin, size);
    buf[size] = 0;

    // remove \r and \n
    char *buf_end = buf + size - 1;
    while (buf_end >= buf && (*buf_end == '\r' || *buf_end == '\n')) {
        *buf_end = 0;
        --buf_end;
    }
    return buf;
}

void vlc_stream_Delete( stream_t *s )
{
    free(s);
}

stream_t * vlc_stream_MemoryNew( demux_t *d ) {
    const stream_t tempstream = {d->s->content, d->s->size, 0};

    stream_t *r = malloc(sizeof(stream_t));
    if( !r )
        return NULL;
    memcpy(r, &tempstream, sizeof(stream_t));
    return r;
}