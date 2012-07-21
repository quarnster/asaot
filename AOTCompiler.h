#ifndef __INCLUDED_AOTCOMPILER_H
#define __INCLUDED_AOTCOMPILER_H

#include <angelscript.h>
#include <string>
#include "AOTFunction.h"

class AOTCompiler : public asIJITCompiler
{
public:
    virtual ~AOTCompiler();
    virtual int CompileFunction(asIScriptFunction *function, asJITFunction *output);
    virtual void ReleaseJITFunction(asJITFunction func);

private:
    void ProcessByteCode(asEBCInstr op, AOTFunction &f);
};

#endif
