import platform

class DataType:
    def __init__(self, asname, cname, bitsize):
        self.asname = asname
        self.cname = cname
        self.bitsize = bitsize
        if bitsize != 0:
            self.bytesize = bitsize/8
        else:
            self.bytesize = 4


datatypes = {
    "char":     DataType("int8",   "char",     8),
    "short":    DataType("int16",  "short",   16),
    "int":      DataType("int",    "int",     32),
    "int64_t":  DataType("int64",  "int64_t",   64),
    "float":    DataType("float",  "float",   32),
    "double":   DataType("double", "double",  64),
    "bool":     DataType("bool", "bool", 8),
    "MyClass":  DataType("MyClass", "MyClass", 0),
    "MyClass2":  DataType("MyClass2", "MyClass2", 0),
    "MyClass3":  DataType("MyClass3", "MyClass3", 0),
    "MyClass&":  DataType("MyClass&in", "MyClass&", 0),
    "MyClass2&":  DataType("MyClass2&in", "MyClass2&", 0),
    "MyClass3&":  DataType("MyClass3&in", "MyClass3&", 0),
    "MyClassr*":  DataType("MyClassr@", "MyClassr*", 0),
    "char&":    DataType("int8&in",   "char&",     8),
    "short&":   DataType("int16&in",  "short&",   16),
    "int&":     DataType("int&in",    "int&",     32),
    "int64_t&": DataType("int64&in",  "int64_t&",   64),
    "float&":   DataType("float&in",  "float&",   32),
    "double&":  DataType("double&in", "double&",  64),
    "bool&":    DataType("bool&in", "bool&", 8),
}

class Test:
    def __init__(self):
        pass
tests = [
    ["int"],
    ["float"],
    ["double"],
    ["int64_t"],
    ["MyClass"],
    ["MyClass2"],
    ["MyClass3"],
    ["MyClassr*"],
    ("int "    * 4).split(),
    ("float "  * 4).split(),
    ("double " * 4).split(),
    ("int "    *32).split(),
    ("float "  *32).split(),
    ("double " *32).split(),
    ("int float " * 16).split(),
    ("int64_t double char " * 10).split(),
    ("MyClass " * 4).split(),
    ("MyClass2 " * 4).split(),
    ("MyClass3 " * 4).split(),
    ("MyClassr* " * 4).split(),
    ["int&"],
    ["float&"],
    ["double&"],
    ["int64_t&"],
    ["MyClass&"],
    ["MyClass2&"],
    ["MyClass3&"],
    ("int& "    * 4).split(),
    ("float& "  * 4).split(),
    ("double& " * 4).split(),
    ("int& "    *32).split(),
    ("float& "  *32).split(),
    ("double& " *32).split(),
    ("int& float& " * 16).split(),
    ("int64_t& double& char& " * 10).split(),
    ("MyClass& "*4).split(),
    ("MyClass2& "*4).split(),
    ("MyClass3& "*4).split(),
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
        mc = 0
        for i, a in enumerate(test):
            if datatypes[a].bitsize == 0:
                val[i] = "0x1337BEEF-%d" % mc
                mc += 1
        binding += "    r = engine->%s\"void func%d(%s)\", %sfunc%d), %s); assert(r>=0);\n" % (register, count, ", ".join([datatypes[a].asname for a in test]), macro, count, decl)

        magicassert = "assert(magic == 0x40302010);\n" if member else ""

        validate = []
        for i, a in enumerate(test):
            extra = ""
            type = a.replace("&", "")
            if datatypes[a].bitsize == 0:
                if "r" in a:
                    extra = "->magic"
                else:
                    extra = ".magic"
                type = "uint64_t"
            validate.append("\tassert(a%d%s == (%s) (%s));" % (i, extra, type, val[i]))

        impl = "\n".join(validate)
        cimplementation += """void %s func%d(%s)
    {
    %s
    %s
        printf("%%s succeeded\\n", __FUNCTION__);
    }
    """ % \
        (calltype, count, ", ".join(["%s a%d" % (a,i) for i,a in enumerate(test)]),
            magicassert,
            impl
        )

        mc = 1
        args = []
        for i, a in enumerate(test):
            if datatypes[a].bitsize == 0:
                extra = ""
                if "r" in a: extra = "r"
                if "2" in a: extra = "2"
                if "3" in a: extra = "3"
                args.append("my%s%d" % (extra, mc))
                mc += 1
            else:
                args.append(val[i])

        args = ", ".join(args)
        ascall += "        \"    %sfunc%d(%s);\\n\"\n" % ("test." if member else "", count, args)
        count += 1

    for type in datatypes:
        val = getval(type)
        asname = datatypes[type].asname
        extra = ""
        oldval = val
        conv = "(%s)" % type
        cextra = ""
        if "&" in type:
            if not "MyClass" in type:
                t2 = type.replace("&", "")
                cextra = "static %s ret_%d = %s;\n" % (t2, count, oldval)
                oldval =val
                val = "ret_%d" % count
            asname = asname.replace("&in", "&")
            conv = ""
        conv2 = asname.replace("&", "")


        if datatypes[type].bitsize == 0:
            conv = ""
            conv2 = ""
            extra = ".magic"
            nextra = ""
            if "r" in type: nextra = "r"
            if "2" in type: nextra = "2"
            if "3" in type: nextra = "3"
            val = "my%s1" % (nextra)
            oldval = val + extra
            if "r" in type:
                val = "&" + val

        binding += "    r = engine->%s\"%s func%d()\", %sfunc%d), %s); assert(r>=0);\n" % (register, asname, count, macro, count, decl)
        cimplementation += """%s %s func%d()
    {
        %s
        %s
        return %s %s;
    }\n""" % (type, calltype, count, "assert(magic == 0x40302010);\n" if member else "", cextra, conv, val)

        if datatypes[type].bitsize == 0:
            val = val.replace("&", "")
            val += ".magic"

        ascall += "        \"    assert(%sfunc%d()%s == %s(%s));\\n\"\n" % ("test." if member else "", count, extra, conv2, oldval)
        count += 1

register("cdecl")
if platform.system() != "Linux":
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

class MyClass
{
public:
    uint64_t magic;
};

class MyClass2
{
public:
    uint64_t magic;
    MyClass2(int val)
    : magic((uint64_t) val)
    {
    }
};

class MyClass3
{
public:
    uint64_t magic;
    MyClass3(int val)
    : magic((uint64_t) val)
    {
    }
    MyClass3(const MyClass3 &other)
    : magic(other.magic)
    {
    }
    ~MyClass3() {}

    MyClass3& operator=(const MyClass3& other)
    {
        magic = other.magic;
        return *this;
    }
};


class MyClassr
{
public:
    uint64_t magic;
    MyClassr(int val)
    : magic((uint64_t) val)
    {
    }
    ~MyClassr() {}
};

MyClass my1;
MyClass my2;
MyClass my3;
MyClass my4;

MyClass2 my21(0x1337BEEF);
MyClass2 my22(0x1337BEEF-1);
MyClass2 my23(0x1337BEEF-2);
MyClass2 my24(0x1337BEEF-3);

MyClass3 my31(0x1337BEEF);
MyClass3 my32(0x1337BEEF-1);
MyClass3 my33(0x1337BEEF-2);
MyClass3 my34(0x1337BEEF-3);


MyClassr myr1(0x1337BEEF);
MyClassr myr2(0x1337BEEF-1);
MyClassr myr3(0x1337BEEF-2);
MyClassr myr4(0x1337BEEF-3);

void MessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    printf("%%s (%%d, %%d) : %%s : %%s\\n", msg->section, msg->row, msg->col, type, msg->message);
}


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

static void MyClass3_constr(MyClass3* ptr, int val)
{
    new(ptr) MyClass3(val);
}

static void MyClass3_constr_def(MyClass3* ptr)
{
    new(ptr) MyClass3(0x7331);
}

static void MyClass3_constr_copy(MyClass3* ptr, const MyClass3 &other)
{
    new(ptr) MyClass3(other);
}

static void MyClass3_destr(MyClass3 *ptr)
{
    ptr->~MyClass3();
}


int main(int argc, char **argv)
{
    asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
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
    my1.magic = (uint64_t) (0x1337BEEF);
    my2.magic = (uint64_t) (0x1337BEEF-1);
    my3.magic = (uint64_t) (0x1337BEEF-2);
    my4.magic = (uint64_t) (0x1337BEEF-3);

    int r;
    r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r>=0);
    r = engine->RegisterObjectType("Test", 0, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
    r = engine->RegisterObjectType("MyClass", sizeof(MyClass), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALIGN8); assert( r >= 0 );
    r = engine->RegisterObjectType("MyClass2", sizeof(MyClass2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_CONSTRUCTOR  | asOBJ_APP_CLASS_ALIGN8); assert( r >= 0 );
    r = engine->RegisterObjectType("MyClass3", sizeof(MyClass3), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_CLASS_ASSIGNMENT  | asOBJ_APP_CLASS_ALIGN8); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("MyClass3", asBEHAVE_CONSTRUCT, "void f(int m)", asFUNCTION(MyClass3_constr), asCALL_CDECL_OBJFIRST); assert(r >= asSUCCESS);
    r = engine->RegisterObjectBehaviour("MyClass3", asBEHAVE_CONSTRUCT, "void f(const MyClass3&in)", asFUNCTION(MyClass3_constr_copy), asCALL_CDECL_OBJFIRST); assert(r >= asSUCCESS);
    r = engine->RegisterObjectBehaviour("MyClass3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(MyClass3_constr_def), asCALL_CDECL_OBJFIRST); assert(r >= asSUCCESS);
    r = engine->RegisterObjectBehaviour("MyClass3", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(MyClass3_destr), asCALL_CDECL_OBJFIRST); assert(r >= asSUCCESS);
    r = engine->RegisterObjectMethod("MyClass3", "MyClass3 &opAssign(const MyClass3 &in)", asMETHODPR(MyClass3, operator =, (const MyClass3&), MyClass3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectProperty("MyClass", "uint64 magic", asOFFSET(MyClass,magic)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("MyClass2", "uint64 magic", asOFFSET(MyClass2,magic)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("MyClass3", "uint64 magic", asOFFSET(MyClass3,magic)); assert( r >= 0 );
    r = engine->RegisterObjectType("MyClassr", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
    r = engine->RegisterObjectProperty("MyClassr", "uint64 magic", asOFFSET(MyClassr,magic)); assert( r >= 0 );
    %s
    r = engine->RegisterGlobalProperty("Test test", &test); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass my1", &my1); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass my2", &my2); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass my3", &my3); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass my4", &my4); assert(r >= 0);

    r = engine->RegisterGlobalProperty("MyClass2 my21", &my21); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass2 my22", &my22); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass2 my23", &my23); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass2 my24", &my24); assert(r >= 0);

    r = engine->RegisterGlobalProperty("MyClass3 my31", &my31); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass3 my32", &my32); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass3 my33", &my33); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClass3 my34", &my34); assert(r >= 0);

    r = engine->RegisterGlobalProperty("MyClassr myr1", &myr1); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClassr myr2", &myr2); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClassr myr3", &myr3); assert(r >= 0);
    r = engine->RegisterGlobalProperty("MyClassr myr4", &myr4); assert(r >= 0);

    const char *script =
        "void main()\\n"
        "{\\n"
%s      "}\\n";
    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    mod->AddScriptSection("Test", script, strlen(script), 0);
    mod->Build();

    asIScriptContext *ctx = engine->CreateContext();
    ctx->Prepare(mod->GetFunctionByDecl("void main()"));
    r = ctx->Execute();
    assert(r == asEXECUTION_FINISHED);

    printf("all is good %%d, %%d\\n", AOT_GENERATE_CODE, sizeof(void*));

#if AOT_GENERATE_CODE
#ifdef ANDROID
    #define EXTRA "/data/local/"
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
