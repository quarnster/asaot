#include "AOTCompiler.h"
#include <sys/time.h>

#define TESTNAME "asJITTest"
static const char *script =
"int TestInt(int a, int b, int c)          \n"
"{                                         \n"
"    int ret = 0;                          \n"
"    for (int i = 0; i < 25000; i++)       \n"
"        for (int j = 0; j < 1000; j++)    \n"
"        {                                 \n"
"           ret += a*b+i*j;                \n"
"           ret += c * 2;                  \n"
"        }                                 \n"
"    return ret;                           \n"
"}                                         \n";

extern "C"
{
int __aeabi_idiv(int a, int b)
{
    return a/b;
}
}

double GetTime()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + (t.tv_usec / (1000.0 * 1000.0));
}


class COutStream
{
public:
    void Callback(asSMessageInfo *msg)
    {
        const char *msgType = 0;
        if( msg->type == 0 ) msgType = "Error  ";
        if( msg->type == 1 ) msgType = "Warning";
        if( msg->type == 2 ) msgType = "Info   ";

        printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
    }
};


extern unsigned int AOTLinkerTableSize;
extern AOTLinkerEntry AOTLinkerTable[];
int main(int argc, char ** argv)
{
    asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    COutStream out;
    engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
    engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, 1);
    engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, 1);
    engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);

#define AOT_GENERATE_CODE 1
#if !AOT_GENERATE_CODE
    asIJITCompiler *jit = new AOTCompiler(AOTLinkerTable, AOTLinkerTableSize);
#else
    asIJITCompiler *jit = new AOTCompiler();
#endif
    if (argc != 2)
    {
        engine->SetJITCompiler(jit);
    }
    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
    mod->Build();

    asIScriptContext *ctx = engine->CreateContext();
    ctx->Prepare(mod->GetFunctionByDecl("int TestInt(int a, int b, int c)"));
    ctx->SetArgDWord(0, 3);
    ctx->SetArgDWord(1, 5);
    ctx->SetArgDWord(2, 2);

    printf("Executing AngelScript version...\n");

    double time = GetTime();

    int r = ctx->Execute();

    time = GetTime() - time;

    if( r != 0 )
    {
        printf("Execution didn't terminate with asEXECUTION_FINISHED\n");
        if( r == asEXECUTION_EXCEPTION )
        {
            printf("Script exception\n");
            asIScriptFunction *func = ctx->GetExceptionFunction();
            printf("Func: %s\n", func->GetName());
            printf("Line: %d\n", ctx->GetExceptionLineNumber());
            printf("Desc: %s\n", ctx->GetExceptionString());
        }
    }
    else
    {
        printf("Time = %f secs\n", time);
        printf("returned: %d\n", (int) ctx->GetReturnDWord());
    }
#if AOT_GENERATE_CODE
    AOTCompiler *c = (AOTCompiler*) jit;
    c->DumpCode();
#endif


    ctx->Release();
    engine->Release();
    delete jit;
    return r;
}
