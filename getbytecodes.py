import os
import re
import sys

as_context_file = "%s/source/as_context.cpp" % sys.argv[1]

f = open(as_context_file)
data = f.read()
f.close()

data = re.search(r"(asCContext::ExecuteNext.*?\n}\n)", data, re.DOTALL).group(1)
commentre = re.compile(r"^\s*//")
jumpre = re.compile(r"\s*l_bc\s*\+=[\s\d+]+asBC")
argre = re.compile(r"(\w+ARG\w*)\(l_bc([^)]*)\)")

bytecodes = re.findall(r"case\s+(a\w+):(.*?)(?=case\s+\w+:)", data, re.DOTALL)
print "#include <angelscript.h>"
print "#include <stdio.h>"
print "#include <assert.h>"
print "#include <math.h>"
print "#include \"AOTCompiler.h\""
print ""
print "#define ASSERT assert"
print "#define asASSERT(x) "
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

    lines = bytecode[1].split("\n")
    for line in lines:
        dobreak = False
        if "break;" in line:
            dobreak = True
            line = line.replace("break;", "")

        line = line.replace("%", "%%")
        line = line.replace("this", "context")
        line = line.replace("m_", "context->m_")
        line = line.replace("CallSc", "context->CallSc")
        line = line.replace("CallInt", "context->CallInt")
        line = line.replace("CallLine", "context->CallLine")
        line = line.replace("SetInternal", "context->SetInternal")
        line = line.replace("PopCallState", "context->PopCallState")

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
            print "            char aotbuf[128];"
            print "            snprintf(aotbuf, 128, \"%s\\n\", %s);" % (line, ",".join(["aottmp%d" % d for d in range(count)]))
            print "            func.m_output += aotbuf;"
        if jump:
            print "                char aotbuf2[128];"
            print "                snprintf(aotbuf2, 128, \"                    goto bytecodeoffset_%d;\\n\", target);"
            print "            func.m_output += aotbuf2;"
            print "            func.m_output += \"                }\\n\";"
            print "            }"
        if count != 0:
            print "}"
        if dobreak:
            break
    print "            break;"
    print "        }"

print "        default:"
print "            ASSERT(\"can't handle that opcode...\");"
print "            break;"
print "    }"
print "}"
