#ifndef __INCLUDED_AOT_CONFIG_H
#define __INCLUDED_AOT_CONFIG_H

#if _MSC_VER
    #define snprintf _snprintf_s
    #define THISCALL __thiscall
#else
    #define THISCALL
#endif

#ifndef STDCALL
    #define STDCALL __stdcall
#endif


#endif
