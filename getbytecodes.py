#
# Copyright (c) 2012 Fredrik Ehnbom
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
#    1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
#
#    2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
#
#    3. This notice may not be removed or altered from any source
#    distribution.
#/
import os
import re
import sys

as_context_file = "%s/source/as_context.cpp" % sys.argv[1]

f = open(as_context_file)
data = f.read()
f.close()

data          = re.search(r"(asCContext::ExecuteNext.*?\n}\n)", data, re.DOTALL).group(1)
commentre     = re.compile(r"^\s*//")
jumpre        = re.compile(r"\s*l_bc\s*\+=[\s\d+]+asBC")
argre         = re.compile(r"(\w+ARG\w*)\(l_bc([^)]*)\)")
callscriptre  = re.compile(r"(m_regs\.stackFramePointer\s*=\s*l_fp;\s+?)(.*?)([ \t]*)(Call(InterfaceMethod|ScriptFunction)\((.*?)\);)(\s+.*?l_fp\s+=.*?m_regs\.stackFramePointer;)", re.DOTALL)

bytecodes = re.findall(r"case\s+(a\w+):(.*?)(?=case\s+\w+:)", data, re.DOTALL)
print "#include <angelscript.h>"
print "#include <as_scriptengine.h>"
print "#include <stdio.h>"
print "#include <assert.h>"
print "#include <math.h>"
print "#include \"AOTCompiler.h\""
print ""
print "#define ASSERT assert"
print "#define asASSERT(x) "
print "#define BUFSIZE 512 "
print ""
print "void AOTCompiler::ProcessByteCode(asDWORD *byteCode, asUINT offset, asEBCInstr op, AOTFunction &func)"
print "{"
print "    switch (op)"
print "    {"

for bytecode in bytecodes:
    if bytecode[0] == "asBC_JitEntry":
        continue

    if bytecode[1].count("break;") != 1:
        print "====================================="
        print bytecode[0]
        print bytecode[1]
        raise Exception("Code generation failed as there are more or less breaks than expected")


    print "        case %s:" % bytecode[0]
    print "        {"
    print "            func.m_output += \"// %s\\n\";" % bytecode[0]
    print "            func.m_output += \"asm(\\\"# %s\\\");\\n\";" % bytecode[0]

    data = bytecode[1]
    data = callscriptre.sub(r"""\1\3
\2
\3\4
#if %d __RAW__
            asDWORD * expected = l_bc;
            int i = asBC_INTARG(byteCode); __RAW__
            func.m_output += "\3if (\6->jitFunction == " + GetAOTName(\6) + ")\\n"; __RAW__
\3{
            func.m_output += "\3    " + GetAOTName(\6) + "(registers, 1);\\n"; __RAW__
\3}
\7
\3if (l_bc != expected)
\3    __PLACEHOLDER2__
#else __RAW__
\7
\3    __PLACEHOLDER2__
#endif __RAW__
\3""" % (bytecode[0] == "asBC_CALL" or bytecode[0] == "asBC_CALLINTF"), data)

    lines = data.split("\n")
    for line in lines:
        dobreak = False
        if "break;" in line:
            dobreak = True
            line = line.replace("break;", "")

        line = line.replace("%", "%%")
        line = line.replace("this", "context")
        line = line.replace("CallSc", "context->CallSc")
        line = line.replace("CallInt", "context->CallInt")
        line = line.replace("CallLine", "context->CallLine")
        line = line.replace("SetInternal", "context->SetInternal")
        line = line.replace("PopCallState", "context->PopCallState")


        if "__RAW__" in line:
            line = line.replace("__RAW__", "")
            idx = line.rfind("m_engine")
            if idx != -1:
                line = "%s%s" % (line[:idx].replace("m_engine", "context->m_engine"), line[idx:].replace("m_engine", "((asCScriptEngine*)m_engine)"))
            print line
            continue

        line = line.replace("m_", "context->m_")

        if "return" in line:
            line =  """            func.m_output += "%sgoto " + func.m_name + "_end;\\n";""" % line.replace("return;", "")
            print line
            continue

        if "__PLACEHOLDER2__" in line:
            line = "            func.m_output += \"%sgoto \" + func.m_name + \"_end;\\n\";" % line.replace("__PLACEHOLDER2__", "")
            print line
            continue



        if commentre.match(line) or len(line.strip()) == 0:
            continue

        jump = jumpre.match(line)

        count = 0
        for m in argre.finditer(line):
            if count == 0:
                print "{"
            print "            int aottmp%d = %s(byteCode%s);" % (count, m.group(1), m.group(2))
            line = line.replace(m.string[m.start(1):m.end(2)+1], "%d")
            count += 1

        if jump:
            print "            {"
            print "                asDWORD target = asBC_INTARG(byteCode)+2 + offset;"
            print "            func.m_output += \"                {\\n\";"
        if count == 0:
            print "            func.m_output += \"%s\\n\";" % line
        else:
            print "            char aotbuf[BUFSIZE];"
            print "            snprintf(aotbuf, BUFSIZE, \"%s\\n\", %s);" % (line, ",".join(["aottmp%d" % d for d in range(count)]))
            print "            func.m_output += aotbuf;"
        if jump:
            print "                char aotbuf2[BUFSIZE];"
            print "                snprintf(aotbuf2, BUFSIZE, \"                    goto bytecodeoffset_%d;\\n\", target);"
            print "            func.m_output += aotbuf2;"
            print "            func.m_output += \"                }\\n\";"
            print "            }"
        if count != 0:
            print "}"

    if bytecode[0] == "asBC_RET":
            print "            func.m_output += \"      goto \" + func.m_name + \"_end;\\n\";"
    print "            break;"
    print "        }"

print "        default:"
print "            ASSERT(\"can't handle that opcode...\");"
print "            break;"
print "    }"
print "}"
