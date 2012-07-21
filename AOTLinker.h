#ifndef __INCLUDED_AOTLINKER_H
#define __INCLUDED_AOTLINKER_H

#include <angelscript.h>

typedef struct
{
    const char name[256];
    asJITFunction entry;
} AOTLinkerEntry;

#endif
