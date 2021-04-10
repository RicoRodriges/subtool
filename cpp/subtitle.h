#if defined (__cplusplus)
extern "C" {
#endif

#ifndef SUBTITLES_SUBTITLE_H
#define SUBTITLES_SUBTITLE_H


#include "types.h"

typedef struct {
    const char *psz_type_name;
    int  i_type;
    const char *psz_name;
    int  (*pf_read)( subs_properties_t *, text_t *, subtitle_t*, size_t );
} sub_info_t;

extern const sub_info_t sub_read_subtitle_function [];

enum subtitle_type_e DetectType(const char * content, uint64_t size);
int  Open (const char * content, uint64_t size, float fps, enum subtitle_type_e type, demux_sys_t **res);
void Close( demux_sys_t *p_sys );

#endif

#if defined (__cplusplus)
}
#endif
