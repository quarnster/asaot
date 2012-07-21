#include "AOTCompiler.h"


AOTCompiler::~AOTCompiler()
{

}

int AOTCompiler::CompileFunction(asIScriptFunction *function, asJITFunction *output)
{
    asUINT   length;
    asDWORD *byteCode = function->GetByteCode(&length);
    asDWORD *end = byteCode + length;
    AOTFunction f;

    while ( byteCode < end )
    {
        asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
        if (op == asBC_JitEntry)
        {
            f.m_output += "case ";
            f.m_output += ++f.labelCount;
            asBC_PTRARG() = f.labelCount;
        }
        else
        {
            ProcessByteCode(op, f);
        }

        byteCode += asBCTypeSize[asBCInfo[op].type];
    }

    return asERROR;
}

void AOTCompiler::ReleaseJITFunction(asJITFunction func)
{

}
