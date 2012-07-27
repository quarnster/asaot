/*
 * Copyright (c) 2012 Fredrik Ehnbom
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#if defined(__LP64__) || defined(__amd64__) || defined(__x86_64__) || defined(_M_X64)
        #ifndef AS_64BIT_PTR
                #define AS_64BIT_PTR
        #endif
#endif

#include "AOTCompiler.h"
#include <stdio.h>

AOTCompiler::AOTCompiler(AOTLinker *linker)
: m_linker(linker)
{
}

AOTCompiler::~AOTCompiler()
{
}

std::string add(const char *toadd)
{
    std::string ret;
    if (toadd)
    {
        std::string tmp = toadd;
        int off;
        while ((off = tmp.find(':')) != -1)
        {
            tmp = tmp.replace(off, 1, 1, '_');
        }
        while ((off = tmp.find('.')) != -1)
        {
            tmp = tmp.replace(off, 1, 1, '_');
        }
        while ((off = tmp.find('/')) != -1)
        {
            tmp = tmp.replace(off, 1, 1, '_');
        }
        ret += tmp;
    }
    return ret;
}

std::string AOTCompiler::GetAOTName(asIScriptFunction *function)
{
    std::string name;
    name += "___";
    name += add(function->GetModuleName());
    name += "_";
    name += add(function->GetNamespace());
    name += "_";
    name += add(function->GetScriptSectionName());
    name += "_";
    name += add(function->GetObjectName());
    name += "_";
    if (function->GetName())
    {
        std::string fname = function->GetName();
        if (fname[0] == '~')
            fname = "__destructor__" + fname.substr(1);
        name += fname;
    }
    name += "_";
    return name;
}

int AOTCompiler::CompileFunction(asIScriptFunction *function, asJITFunction *output)
{
    m_engine = function->GetEngine();
    asUINT   length;
    asDWORD *byteCode = function->GetByteCode(&length);
    asUINT   offset = 0;
    asDWORD *end = byteCode + length;

    if (length == 0)
        return asERROR;

    AOTFunction f(function);

    f.m_name = GetAOTName(function);

    AOTLinker::LinkerResult ret = m_linker->LookupFunction(&f, output);
    if (ret == AOTLinker::AllDone)
        return asSUCCESS;
    if (ret == AOTLinker::ErrorOccurred)
        return asERROR;

    f.m_output += "        case 0:\n";

    while ( byteCode < end )
    {
        asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
        char buf[128];
        if (ret == AOTLinker::GenerateCode)
        {
            snprintf(buf, 128, "bytecodeoffset_%d:\n", offset);
            f.m_output += buf;
        }
        if (op == asBC_JitEntry)
        {
            snprintf(buf, 128, "        case %d:\n", ++f.m_labelCount);
            f.m_output += buf;
            snprintf(buf, 128, "            l_bc += %d;\n", asBCTypeSize[asBCInfo[asBC_JitEntry].type]);
            f.m_output += buf;
            asBC_PTRARG(byteCode) = f.m_labelCount;
        }
        else if (ret == AOTLinker::GenerateCode)
        {
            ProcessByteCode(byteCode, offset, op, f);
        }

        byteCode += asBCTypeSize[asBCInfo[op].type];
        offset   += asBCTypeSize[asBCInfo[op].type];
    }
    m_functions.push_back(f);

    if (ret != AOTLinker::GenerateCode)
    {
        return asSUCCESS;
    }

    return asERROR;
}

void AOTCompiler::SaveCode(asIBinaryStream *stream)
{
    std::string output;
    output += "#include <as_config.h>\n";
    output += "#include <as_context.h>\n";
    output += "#include <as_scriptengine.h>\n";
    output += "#include <as_callfunc.h>\n";
    output += "#include <as_scriptobject.h>\n";
    output += "#include <as_texts.h>\n";
    output += "#include <aot_config.h>\n";
    // TODO: is there a better way to handle this? What if it changes?
    output += "\nstatic const int CALLSTACK_FRAME_SIZE = 5;\n\n";

    for (std::vector<AOTFunction>::iterator i = m_functions.begin(); i < m_functions.end(); i++)
    {
        output += "void ";
        output += (*i).m_name;
        output += "(asSVMRegisters * registers, asPWORD jitArg);\n";
    }
    for (std::vector<AOTFunction>::iterator i = m_functions.begin(); i < m_functions.end(); i++)
    {
        output += "void ";
        output += (*i).m_name;
        output += "(asSVMRegisters * registers, asPWORD jitArg)\n";
        output += "{\n";
//        output += "    printf(\"In aot compiled function %%s, %%lx!\\n\", __FUNCTION__, jitArg);\n";
        output += "    asDWORD * l_bc = registers->programPointer;\n";
        output += "    asDWORD * l_sp = registers->stackPointer;\n";
        output += "    asDWORD * l_fp = registers->stackFramePointer;\n";
        output += "    asCContext * context = (asCContext*) registers->ctx;\n";
        // char buf[32];
        // snprintf(buf, 32, "    l_bc += %d;\n", asBCTypeSize[asBCInfo[asBC_JitEntry].type]);

        // output += buf;
        // output += "    goto ";
        // output += (*i).m_name;
        // output += "_end;";
        output += "    switch (jitArg)\n";
        output += "    {\n";
        output += (*i).m_output;
        output += "    }\n";
        output += (*i).m_name;
        output += "_end:\n";
        output += "    registers->programPointer    = l_bc;\n";
        output += "    registers->stackPointer      = l_sp;\n";
        output += "    registers->stackFramePointer = l_fp;\n";
        output += (*i).m_name;
        output += "_end2:\n";
        output += "    return;\n";
        output += "}\n";
    }

    m_linker->LinkTimeCodeGeneration(output, m_functions);

    stream->Write(output.c_str(), output.length());
}

void AOTCompiler::ReleaseJITFunction(asJITFunction func)
{

}
