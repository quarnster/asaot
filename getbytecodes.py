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
angelscript_h = "%s/include/angelscript.h" % sys.argv[1]

f = open(as_context_file)
data = f.read()
f.close()

data          = re.search(r"(asCContext::ExecuteNext.*?\n}\n)", data, re.DOTALL).group(1)
commentre     = re.compile(r"^\s*//")
jumpre        = re.compile(r"\s*l_bc\s*\+=[\s\d+]+asBC")
argre         = re.compile(r"(\w+ARG\w*)\(l_bc([^)]*)\)")
callscriptre  = re.compile(r"(m_regs\.stackFramePointer\s*=\s*l_fp;\s+?)"\
                            "(.*?)([ \t]*)(Call(InterfaceMethod|ScriptFunction))\((.*?)\);(.+?)"\
                            "(l_\w+\s+=\s*m_regs\.\w+Pointer;)[\s\n\r]+"\
                            "(l_\w+\s+=\s*m_regs\.\w+Pointer;)[\s\n\r]+"\
                            "(l_\w+\s+=\s*m_regs\.\w+Pointer;)[\s\n\r]+"\
                            "((.*?if.*?)(return;))", re.DOTALL)
retre = re.compile(r"([\t ]+)(PopCallState\(\);).*?l_sp\s*\+=\s*(.*?);", re.DOTALL)


f = open(angelscript_h)
data2 = f.read()
f.close()

lut = dict(re.findall(r"#define\s*(asBC_\w+ARG[^\(]*)\(.*?\)\s+.*?(\w+)", data2, re.DOTALL))
def gettypename(macro):
    return lut[macro]

def gettypeprintf(macro):
    if lut[macro] == "float":
        return "%f"
    elif lut[macro] == "asQWORD":
        return "%ld"
    return "%d"


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
    print "            func.m_output += \"\t\t// %s\\n\";" % bytecode[0]

    data = bytecode[1]
    data = callscriptre.sub(r"""\1\3
\2
\3asCScriptFunction *__func = \6;
\3\4(__func);
#if %d                                                                                                                             __RAW__
            int i = asBC_INTARG(byteCode);                                                                                         __RAW__
            asCScriptFunction *asfunc = ((asCScriptEngine*)m_engine)->GetScriptFunction(i);                                        __RAW__
            if (asfunc)                                                                                                            __RAW__
            {                                                                                                                      __RAW__
                func.m_output += "\3asDWORD * expected = l_bc;\\n";                                                                __RAW__
\12
                func.m_output += "\3    __PLACEHOLDER2__\\n";                                                                      __RAW__
                func.m_output += "\3if (__func->jitFunction == " + GetAOTName(asfunc) + ")\\n";                                    __RAW__
                func.m_output += "\3{\\n";                                                                                         __RAW__
                func.m_output += "\3    " + GetAOTName(asfunc) + "(registers, 0);\\n";                                             __RAW__
                func.m_output += "\3}\\n";                                                                                         __RAW__
                func.m_output += "\3\8\\n";                                                                                        __RAW__
                func.m_output += "\3\9\\n";                                                                                        __RAW__
                func.m_output += "\3\10\\n";                                                                                       __RAW__
                func.m_output += "\3if (l_bc != expected)\\n";                                                                     __RAW__
                func.m_output += "\3    __PLACEHOLDER2__\\n";                                                                      __RAW__
            }                                                                                                                      __RAW__
            else                                                                                                                   __RAW__
#endif                                                                                                                             __RAW__
            {                                                                                                                      __RAW__
                func.m_output += "\3__PLACEHOLDER2__\\n";                                                                          __RAW__
            }                                                                                                                      __RAW__
\3""" % (bytecode[0] == "asBC_CALL" or bytecode[0] == "asBC_ALLOC"), data)

    if bytecode[0] == "asBC_RET":
        data = retre.sub(r"""\1\2
\1registers->stackPointer += \3;
            func.m_output += "\1__PLACEHOLDER2__\\n"; __RAW__""", data)

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
        line = line.replace("m_regs.", "registers->")

        if "__PLACEHOLDER2__" in line:
            line = line.replace("__PLACEHOLDER2__", "goto \" + func.m_name + \"_end2;")

        if "__RAW__" in line:
            line = line.replace("__RAW__", "")
            print line
            continue

        if "return" in line:
            line =  """            func.m_output += "%sgoto " + func.m_name + "_end;\\n";""" % line.replace("return;", "")
            print line
            continue

        line = line.replace("m_", "context->m_")


        if commentre.match(line) or len(line.strip()) == 0:
            continue

        jump = jumpre.match(line)

        count = 0
        for m in argre.finditer(line):
            if "PTR" in m.group(1):
                continue
            if count == 0:
                print "            {"
            print "                %s aottmp%d = %s(byteCode%s);" % (gettypename(m.group(1)), count, m.group(1), m.group(2))
            line = line.replace(m.string[m.start(1):m.end(2)+1], gettypeprintf(m.group(1)))
            count += 1

        if jump:
            print "            {"
            print "                asDWORD target = asBC_INTARG(byteCode)+2 + offset;"
            print "                func.m_output += \"                {\\n\";"
        if count == 0:
            print "            func.m_output += \"%s\\n\";" % line
        else:
            print "                char aotbuf[BUFSIZE];"
            print "                snprintf(aotbuf, BUFSIZE, \"%s\\n\", %s);" % (line, ",".join(["aottmp%d" % d for d in range(count)]))
            print "                func.m_output += aotbuf;"
        if jump:
            print "                char aotbuf2[BUFSIZE];"
            print "                snprintf(aotbuf2, BUFSIZE, \"                    goto bytecodeoffset_%d;\\n\", target);"
            print "                func.m_output += aotbuf2;"
            print "                func.m_output += \"                }\\n\";"
            print "            }"
        if count != 0:
            print "            }"

    print "            break;"
    print "        }"

print "        default:"
print "            ASSERT(\"can't handle that opcode...\");"
print "            break;"
print "    }"
print "}"
