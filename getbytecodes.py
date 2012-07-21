import os
import re
import sys

as_context_file = "%s/source/as_context.cpp" % sys.argv[1]

f = open(as_context_file)
data = f.read()
f.close()

data = re.search(r"(asCContext::ExecuteNext.*?\n}\n)", data, re.DOTALL).group(1)
commentre = re.compile(r"^\s*//")

bytecodes = re.findall(r"case\s+(a\w+):(.*?)(?=case\s+\w+:)", data, re.DOTALL)
print "#include <angelscript.h>"
print "#include <stdio.h>"
print "#include <assert.h>"
print "#include <as_config.h>"
print "#include <as_context.h>"
print "#include <as_scriptengine.h>"
print "#include <as_callfunc.h>"
print "#include <as_scriptobject.h>"
print "#include <as_texts.h>"
print "#include <math.h>"
print "#include \"AOTCompiler.h\""
print ""
print "#define ASSERT assert"
print "#define asASSERT(x) "
print "extern const int CALLSTACK_FRAME_SIZE;"
print ""
print "void AOTCompiler::ProcessByteCode(asEBCInstr op, AOTFunction &func)"
print "{"
print "    asSVMRegisters *registers = NULL;"
print "    asDWORD *l_bc = registers->programPointer;"
print "    asDWORD *l_sp = registers->stackPointer;"
print "    asDWORD *l_fp = registers->stackFramePointer;"
print "    asCContext* context = NULL;"
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

    lines = bytecode[1].split("\n")
    for line in lines:
        dobreak = False
        if "break;" in line:
            dobreak = True
            line = line.replace("break;", "")

        #line = line.replace("%", "%%")
        line = line.replace("this", "context")
        line = line.replace("m_", "context->m_")
        line = line.replace("CallSc", "context->CallSc")
        line = line.replace("CallInt", "context->CallInt")
        line = line.replace("CallLine", "context->CallLine")
        line = line.replace("SetInternal", "context->SetInternal")
        line = line.replace("PopCallState", "context->PopCallState")

        if commentre.match(line) or len(line.strip()) == 0:
            continue

        print "            func.m_output += \"%s\\n\";" % line
        if dobreak:
            break
    print "            break;"
    print "        }"

print "        default:"
print "            ASSERT(\"can't handle that opcode...\");"
print "            break;"
print "    }"
print "}"
