#ifndef __INCLUDED_AOTCOMPILER_H
#define __INCLUDED_AOTCOMPILER_H

#include <angelscript.h>
#include <vector>
#include "AOTFunction.h"


class AOTCompiler : public asIJITCompiler
{
public:
    AOTCompiler();
    virtual ~AOTCompiler();
    virtual int CompileFunction(asIScriptFunction *function, asJITFunction *output);
    virtual void ReleaseJITFunction(asJITFunction func);

    void DumpCode();
private:
    void ProcessByteCode(asDWORD *byteCode, asUINT offset, asEBCInstr op, AOTFunction &f);
    std::vector<AOTFunction> m_functions;
};

#endif
