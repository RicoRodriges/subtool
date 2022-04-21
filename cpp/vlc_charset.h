#pragma once

static inline float us_strtof(const char * str, char ** end)
{
    return strtof(str, end);
}