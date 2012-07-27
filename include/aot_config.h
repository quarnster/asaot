#ifndef __INCLUDED_AOT_CONFIG_H
#define __INCLUDED_AOT_CONFIG_H

#if _MSC_VER
    #define snprintf _snprintf_s
    #define THISCALL __thiscall
#else
    #define THISCALL
#endif

#ifndef STDCALL
    #ifdef __linux__
        #define STDCALL __attribute__((stdcall))
    #else
        #define STDCALL __stdcall
    #endif
#endif


#endif
