#ifndef __INCLUDED_AOTCOMPILER_H
#define __INCLUDED_AOTCOMPILER_H

#include <angelscript.h>
#include <vector>
#include "AOTFunction.h"
#include "AOTLinker.h"

class AOTCompiler : public asIJITCompiler
{
public:
    AOTCompiler(AOTLinkerEntry *linkerTable=NULL, unsigned int linkerTableSize=-1);
    virtual ~AOTCompiler();
    virtual int CompileFunction(asIScriptFunction *function, asJITFunction *output);
    virtual void ReleaseJITFunction(asJITFunction func);

    void DumpCode();
private:
    AOTLinkerEntry *m_linkerTable;
    unsigned int m_linkerTableSize;
    void ProcessByteCode(asDWORD *byteCode, asUINT offset, asEBCInstr op, AOTFunction &f);
    std::vector<AOTFunction> m_functions;
};

#endif
