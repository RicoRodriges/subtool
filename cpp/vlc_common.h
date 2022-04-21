#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef void vlc_object_t;

static inline void * realloc_or_free( void *p, size_t sz )
{
    void *n = realloc(p,sz);
    if( !n )
        free(p);
    return n;
}

#define VLC_OBJECT(x) (x)
#define VLC_UNUSED(x) ;

#define NDEBUG

#define VLC_SUCCESS 0
#define VLC_EGENERIC -1
#define VLC_ENOMEM -2


typedef int64_t vlc_tick_t;

// Lets 1 tick = 1 ms. CLOCK_FREQ = 1s
#define CLOCK_FREQ 1000
#define VLC_TICK_FROM_MS(ms)  (ms)
#define MS_FROM_VLC_TICK(vtk) (vtk)
#define vlc_tick_from_sec(x) ((x)*1000)

#define msg_Dbg(...) ;
static inline void msg_Warn(void *o, const char * msg)
{
    fprintf(stderr, "%s\n", msg);
}
//VLC_API void vlc_object_vaLog(vlc_object_t *obj, int prio, const char *module,
//                              const char *file, unsigned line, const char *func,
//                              const char *format, va_list ap);
//
//#define msg_GenericVa(o, p, fmt, ap) \
//    vlc_object_vaLog(VLC_OBJECT(o), p, vlc_module_name, __FILE__, __LINE__, \
//                     __func__, fmt, ap)
//
//#define msg_Generic(o, p, ...) \
//    vlc_object_Log(VLC_OBJECT(o), p, vlc_module_name, __FILE__, __LINE__, \
//                   __func__, __VA_ARGS__)
//#define msg_Info(p_this, ...) \
//    msg_Generic(p_this, VLC_MSG_INFO, __VA_ARGS__)
//#define msg_Err(p_this, ...) \
//    msg_Generic(p_this, VLC_MSG_ERR, __VA_ARGS__)
//#define msg_Warn(p_this, ...) \
//    msg_Generic(p_this, VLC_MSG_WARN, __VA_ARGS__)
//#define msg_Dbg(p_this, ...) \
//    msg_Generic(p_this, VLC_MSG_DBG, __VA_ARGS__)
