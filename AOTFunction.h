#ifndef __INCLUDED_AOTFUNCTION_H
#define __INCLUDED_AOTFUNCTION_H

#include <string>
#include <angelscript.h>


class AOTFunction
{
public:
    AOTFunction();

    std::string m_output;
    std::string m_name;
    unsigned int m_labelCount;

    asJITFunction m_entry;
};

#endif
