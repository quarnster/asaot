#include "AOTCompiler.h"


AOTCompiler::AOTCompiler()
{
}

AOTCompiler::~AOTCompiler()
{

}

int AOTCompiler::CompileFunction(asIScriptFunction *function, asJITFunction *output)
{
    asUINT   length;
    asDWORD *byteCode = function->GetByteCode(&length);
    asUINT   offset = 0;
    asDWORD *end = byteCode + length;
    AOTFunction f;

    if (function->GetModuleName())
        f.m_name += function->GetModuleName();
    if (function->GetNamespace())
        f.m_name += function->GetNamespace();
    if (function->GetScriptSectionName())
        f.m_name += function->GetScriptSectionName();
    if (function->GetObjectName())
        f.m_name += function->GetObjectName();
    if (function->GetName())
        f.m_name += function->GetName();

    while ( byteCode < end )
    {
        asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
        char buf[128];
        snprintf(buf, 128, "bytecodeoffset_%d:\n", offset);
        f.m_output += buf;
        if (op == asBC_JitEntry)
        {
            snprintf(buf, 128, "        case %d:\n", ++f.m_labelCount);
            f.m_output += buf;
            asBC_PTRARG(byteCode) = f.m_labelCount;
        }
        else
        {
            ProcessByteCode(byteCode, offset, op, f);
        }

        byteCode += asBCTypeSize[asBCInfo[op].type];
        offset   += asBCTypeSize[asBCInfo[op].type];
    }
    m_functions.push_back(f);

    return asERROR;
}

void AOTCompiler::DumpCode()
{
    std::string output;
    output += "#include <as_config.h>\n";
    output += "#include <as_context.h>\n";
    output += "#include <as_scriptengine.h>\n";
    output += "#include <as_callfunc.h>\n";
    output += "#include <as_scriptobject.h>\n";
    output += "#include <as_texts.h>\n";
    // TODO: is there a better way to handle this? What if it changes?
    output += "\nstatic const int CALLSTACK_FRAME_SIZE = 5;\n\n";


    for (std::vector<AOTFunction>::iterator i = m_functions.begin(); i < m_functions.end(); i++)
    {
        output += "void ";
        output += (*i).m_name;
        output += "(asSVMRegisters * __restrict registers, asPWORD jitArg)\n";
        output += "{\n";
        output += "    printf(\"In aot compiled function!\\n\");\n";
        output += "    asDWORD * __restrict l_bc = registers->programPointer;\n";
        output += "    asDWORD * __restrict l_sp = registers->stackPointer;\n";
        output += "    asDWORD * __restrict l_fp = registers->stackFramePointer;\n";
        output += "    asCContext __restrict &context = *(asCContext*) registers->ctx;\n";
        output += "    switch (jitArg)\n";
        output += "    {\n";
        output += (*i).m_output;
        output += "     }\n";
        output += "}\n";
    }
    FILE *fp = fopen("bice.cpp", "w");
    fprintf(fp, output.c_str());
    fclose(fp);
}

void AOTCompiler::ReleaseJITFunction(asJITFunction func)
{

}
