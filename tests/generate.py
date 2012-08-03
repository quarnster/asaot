
class DataType:
    def __init__(self, asname, cname, bitsize):
        self.asname = asname
        self.cname = cname
        self.bitsize = bitsize
        self.bytesize = bitsize/8

datatypes = {
    "char":    DataType("int8",   "char",     8),
    "short":   DataType("int16",  "short",   16),
    "int":     DataType("int",    "int",     32),
    "int64_t": DataType("int64",  "int64_t",   64),
    "float":   DataType("float",  "float",   32),
    "double":  DataType("double", "double",  64),
    "bool":    DataType("bool", "bool", 8),
    "char&":    DataType("int8&in",   "char&",     8),
    "short&":   DataType("int16&in",  "short&",   16),
    "int&":     DataType("int&in",    "int&",     32),
    "int64_t&": DataType("int64&in",  "int64_t&",   64),
    "float&":   DataType("float&in",  "float&",   32),
    "double&":  DataType("double&in", "double&",  64),
    "bool&":    DataType("bool&in", "bool&", 8)
}

class Test:
    def __init__(self):
        pass
tests = [
    ["int"],
    ["float"],
    ["double"],
    ["int64_t"],
    ("int "    * 4).split(),
    ("float "  * 4).split(),
    ("double " * 4).split(),
    ("int "    *32).split(),
    ("float "  *32).split(),
    ("double " *32).split(),
    ("int float " * 16).split(),
    ("int64_t double char " * 10).split(),
    ["int&"],
    ["float&"],
    ["double&"],
    ["int64_t&"],
    ("int& "    * 4).split(),
    ("float& "  * 4).split(),
    ("double& " * 4).split(),
    ("int& "    *32).split(),
    ("float& "  *32).split(),
    ("double& " *32).split(),
    ("int& float& " * 16).split(),
    ("int64_t& double& char& " * 10).split()
]

_valcount = 0
def getval(type):
    global _valcount
    type = type.replace("&", "")
    _valcount += 1
    add = _valcount
    val = "0x%s" % ("".join(["%02d" % (i+1) for i in range (datatypes[type].bytesize)]))

    if type == "double" or type == "float":
        val =  "%f" % (1337.0+add)
    elif type == "bool":
        val = "true"
    else:
        val = "0x%x" % (int(val,16)+add)
    return val

count = 0
cimplementation = ""
binding = ""
ascall = ""

def register(calltype="cdecl"):
    global binding
    global cimplementation
    global ascall
    global count
    register = "RegisterGlobalFunction("
    macro = "asFUNCTION("
    decl = "asCALL_CDECL"
    member = False

    if calltype == "thiscall":
        register = "RegisterObjectMethod(\"Test\", "
        macro = "asMETHOD(Test, "
        decl = "asCALL_THISCALL"
        member = True
    elif calltype == "stdcall":
        decl = "asCALL_STDCALL"

    calltype = "STDCALL" if calltype == "stdcall" else ""

    for test in tests:
        val = [getval(t) for t in test]
        binding += "    r = engine->%s\"void func%d(%s)\", %sfunc%d), %s); assert(r>=0);\n" % (register, count, ", ".join([datatypes[a].asname for a in test]), macro, count, decl)
        cimplementation += """void %s func%d(%s)
    {
    %s
    %s
        printf("%%s succeeded\\n", __FUNCTION__);
    }
    """ % (calltype, count, ", ".join(["%s a%d" % (a,i) for i,a in enumerate(test)]),
        "assert(magic == 0x40302010);\n" if member else "",
        "\n".join(["\tassert(a%d == (%s)%s);" % (i,a.replace("&", ""),val[i]) for i,a in enumerate(test)]))
        if "double" in test or "int64_t" in test:
            ascall += "#ifndef ANDROID // unaligned 64bit arguments are broken in AngelScript ARM at the moment\n"
        ascall += "        \"    %sfunc%d(%s);\\n\"\n" % ("test." if member else "", count, ", ".join([val[i] for i in range(len(test))]))
        if "double" in test or "int64_t" in test:
            ascall += "#endif\n"
        count += 1

    for type in datatypes:
        if "&" in type:
            break
        val = getval(type)
        asname = datatypes[type].asname
        binding += "    r = engine->%s\"%s func%d()\", %sfunc%d), %s); assert(r>=0);\n" % (register, asname, count, macro, count, decl)
        cimplementation += """%s %s func%d()
    {
        %s
        return (%s) %s;
    }\n""" % (type, calltype, count, "assert(magic == 0x40302010);\n" if member else "", type, val)
        if type == "double" or type == "int64_t":
            ascall += "#ifndef ANDROID // unaligned 64bit arguments are broken in AngelScript ARM at the moment\n"
        ascall += "        \"    assert(%sfunc%d() == %s(%s));\\n\"\n" % ("test." if member else "", count, asname, val)
        if type == "double" or type == "int64_t":
            ascall += "#endif\n"
        count += 1

register("cdecl")
register("stdcall")
cimplementation += """class Test
{
public:
    unsigned int magic;
    Test()
    {
        magic = 0x40302010;
    }
"""
register("thiscall")
cimplementation += "};\n"


print """
#include <angelscript.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../tests/common.h"
%s

void Assert(asIScriptGeneric *gen)
{
    bool expr;
    if( sizeof(bool) == 1 )
        expr = gen->GetArgByte(0) ? true : false;
    else
        expr = gen->GetArgDWord(0) ? true : false;
    if( !expr )
    {
        printf("--- Assert failed ---\\n");
        asIScriptContext *ctx = asGetActiveContext();
        if( ctx )
        {
            const asIScriptFunction *function = ctx->GetFunction();
            if( function != 0 )
            {
                printf("func: %%s\\n", function->GetDeclaration());
                printf("mdle: %%s\\n", function->GetModuleName());
                printf("sect: %%s\\n", function->GetScriptSectionName());
            }
            printf("line: %%d\\n", ctx->GetLineNumber());
            ctx->SetException("Assert failed");
            printf("---------------------\\n");
        }
        assert(0);
    }
}

int main(int argc, char **argv)
{
    asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, 1);
    engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, 1);
    engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
#define AOT_GENERATE_CODE 1
#if !AOT_GENERATE_CODE
    extern unsigned int AOTLinkerTableSize;
    extern AOTLinkerEntry AOTLinkerTable[];
    SimpleAOTLinker *linker = new SimpleAOTLinker(AOTLinkerTable, AOTLinkerTableSize);
#else
    SimpleAOTLinker *linker = new SimpleAOTLinker(NULL, 0);
#endif
    asIJITCompiler *jit = new AOTCompiler(linker);
    engine->SetJITCompiler(jit);
    Test test;

    int r;
    r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r>=0);
    r = engine->RegisterObjectType("Test", 0, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
    %s
    r = engine->RegisterGlobalProperty("Test test", &test); assert(r >= 0);

    const char *script =
        "void main()\\n"
        "{\\n"
%s        "}\\n";
    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    mod->AddScriptSection("Test", script, strlen(script), 0);
    mod->Build();

    asIScriptContext *ctx = engine->CreateContext();
    ctx->Prepare(mod->GetFunctionByDecl("void main()"));
    r = ctx->Execute();
    assert(r == asEXECUTION_FINISHED);

    printf("all is good %%d\\n", AOT_GENERATE_CODE);

#if AOT_GENERATE_CODE
#ifdef ANDROID
    #define EXTRA "/data/"
#else
    #define EXTRA
#endif
    AOTCompiler *c = (AOTCompiler*) jit;
    CCodeStream cs(EXTRA "aot_generated_code2.cpp");
    c->SaveCode(&cs);
#endif

    ctx->Release();
    engine->Release();
    engine = NULL;
    return 0;
}
""" % (cimplementation, binding, ascall)
