// this file is pretty much a copy an paste + refactoring of various tests
// coming from the AngelScript SDK
/*
   AngelCode Scripting Library
   Copyright (c) 2003-2012 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/

#include <angelscript.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

void Assert(asIScriptGeneric *gen)
{
    bool expr;
    if( sizeof(bool) == 1 )
        expr = gen->GetArgByte(0) ? true : false;
    else
        expr = gen->GetArgDWord(0) ? true : false;
    if( !expr )
    {
        printf("--- Assert failed ---\n");
        asIScriptContext *ctx = asGetActiveContext();
        if( ctx )
        {
            const asIScriptFunction *function = ctx->GetFunction();
            if( function != 0 )
            {
                printf("func: %s\n", function->GetDeclaration());
                printf("mdle: %s\n", function->GetModuleName());
                printf("sect: %s\n", function->GetScriptSectionName());
            }
            printf("line: %d\n", ctx->GetLineNumber());
            ctx->SetException("Assert failed");
            printf("---------------------\n");
        }
        assert(0);
    }
}


static bool    cfunction_b()    { return true;                      }
static bool    retfalse()       { return false;                     }
static int     retfalse_fake()
{
    if( sizeof(bool) == 1 )
        // This function is designed to test AS ability to handle bools that may not be returned in full 32 bit values
        return 0x00FFFF00;
    else
        return 0;
}
#if defined(_MSC_VER) && _MSC_VER <= 1200 // MSVC++ 6
    #define I64(x) x##l
#else // MSVC++ 7, GNUC, etc
    #define I64(x) x##ll
#endif

static asINT64 reti64()         { return I64(0x102030405);          }
static float   cfunction_f()    { return 18.87f;                    }
static double  cfunction_d()    { return 88.32;                     }
struct asPoint                  { float x,y;                        };
struct asRect                   { asPoint tl,br;                    };
asPoint        TestPoint()      { asPoint p={1,2}; return p;        }
asRect         TestRect()       { asRect r={{3,4},{5,6}}; return r; }

static int testVal1 = 0;
static bool called1 = false;
static void cfunction1(int f1) { called1 = true; testVal1 = f1; }
static int testVal2 = 0;
static bool called2 = false;
static void cfunction2(int f1, int f2) { called2 = true; testVal2 = f1+f2; }

static bool testVal4 = false;
static bool called4  = false;
static void cfunction4(int f1, short f2, char f3, int f4)
{
    called4 = true;
    testVal4 = (f1 == 5) && (f2 == 9) && (f3 == 1) && (f4 == 3);
}

static bool testVal4f = false;
static bool called4f  = false;

static void cfunction4f(float f1, float f2, double f3, float f4)
{
    called4f = true;
    testVal4f = (f1 == 9.2f) && (f2 == 13.3f) && (f3 == 18.8) && (f4 == 3.1415f);
}
static bool testValmix = false;
static bool calledmix = false;

static void cfunctionmix(int f1, float f2, double f3, int f4)
{
    calledmix = true;
    testValmix = (f1 == 10) && (f2 == 1.92f) && (f3 == 3.88) && (f4 == 97);
}
static bool testValmix2 = false;
static bool calledmix2  = false;
static void cfunctionmix2(asINT64 i1, float f2, char i3, int i4)
{
    calledmix2 = true;
    testValmix2 = ((i1 == I64(0x102030405)) && (f2 == 3) && (i3 == 24) && (i4 == 128));
}

static bool testVal32 = false;
static bool called32  = false;
static void cfunction32(int f1 , int f2 , int f3 , int f4 ,
                      int f5 , int f6 , int f7 , int f8 ,
                      int f9 , int f10, int f11, int f12,
                      int f13, int f14, int f15, int f16,
                      int f17, int f18, int f19, int f20,
                      int f21, int f22, int f23, int f24,
                      int f25, int f26, int f27, int f28,
                      int f29, int f30, int f31, int f32)
{
    called32 = true;
    testVal32 = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
              (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
              (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
              (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
              (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
              (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
              (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
              (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}
static bool testVal32f = false;
static bool called32f  = false;
static void cfunction32f(float f1 , float f2 , float f3 , float f4 ,
                      float f5 , float f6 , float f7 , float f8 ,
                      float f9 , float f10, float f11, float f12,
                      float f13, float f14, float f15, float f16,
                      float f17, float f18, float f19, float f20,
                      float f21, float f22, float f23, float f24,
                      float f25, float f26, float f27, float f28,
                      float f29, float f30, float f31, float f32)
{
    called32f = true;
    testVal32f = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
              (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
              (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
              (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
              (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
              (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
              (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
              (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}
static bool testVal32d = false;
static bool called32d  = false;
static void cfunction32d(double f1 , double f2 , double f3 , double f4 ,
                      double f5 , double f6 , double f7 , double f8 ,
                      double f9 , double f10, double f11, double f12,
                      double f13, double f14, double f15, double f16,
                      double f17, double f18, double f19, double f20,
                      double f21, double f22, double f23, double f24,
                      double f25, double f26, double f27, double f28,
                      double f29, double f30, double f31, double f32)
{
    called32d = true;
    testVal32d = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
              (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
              (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
              (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
              (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
              (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
              (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
              (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}
static bool testVal32mix = false;
static bool called32mix  = false;
static void cfunction32mix(int f1 , int f2 , int f3 , int f4 ,
        float f5 , float f6 , float f7 , float f8 ,
        int f9 , int f10, int f11, int f12,
        float f13, float f14, float f15, float f16,
        int f17, int f18, int f19, int f20,
        float f21, float f22, float f23, float f24,
        int f25, int f26, int f27, int f28,
        float f29, float f30, float f31, float f32)
{
    called32mix = true;
    testVal32mix =   (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
            (f5  ==  5.0f) && (f6  ==  6.0f) && (f7  ==  7.0f) && (f8  ==  8.0f) &&
            (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
            (f13 == 13.0f) && (f14 == 14.0f) && (f15 == 15.0f) && (f16 == 16.0f) &&
            (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
            (f21 == 21.0f) && (f22 == 22.0f) && (f23 == 23.0f) && (f24 == 24.0f) &&
            (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
            (f29 == 29.0f) && (f30 == 30.0f) && (f31 == 31.0f) && (f32 == 32.0f);
}


int main(int argc, char const *argv[])
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
    int r;

    float returnValue_f = 0.0f;
    double returnValue_d = 0.0f;
    bool returned = false;
    bool returned2 = true;
    asPoint p={0,0};
    asRect rc={{0,0},{0,0}};
    r = engine->RegisterObjectType(    "point",                                       sizeof(asPoint), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
    r = engine->RegisterObjectType(    "rect",                                        sizeof(asRect), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS);  assert(r >= 0);
    r = engine->RegisterGlobalFunction("void assert(bool)",                           asFUNCTION(Assert),        asCALL_GENERIC);                                        assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool cfunction_b()",                          asFUNCTION(cfunction_b),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool retfalse()",                             asFUNCTION(retfalse),      asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool retfalse2()",                            asFUNCTION(retfalse_fake), asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("int64 reti64()",                              asFUNCTION(reti64),        asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("float cfunction_f()",                         asFUNCTION(cfunction_f),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("double cfunction_d()",                        asFUNCTION(cfunction_d),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("point Point()",                               asFUNCTION(TestPoint),     asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("rect Rect()",                                 asFUNCTION(TestRect),      asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalProperty("double returnValue_d",                        &returnValue_d);                                                                   assert(r >= 0);
    r = engine->RegisterGlobalProperty("float returnValue_f",                         &returnValue_f);                                                                   assert(r >= 0);
    r = engine->RegisterGlobalProperty("bool returned",                               &returned);                                                                        assert(r >= 0);
    r = engine->RegisterGlobalProperty("bool returned2",                              &returned2);                                                                       assert(r >= 0);
    r = engine->RegisterGlobalProperty("point p",                                     &p);                                                                               assert(r >= 0);
    r = engine->RegisterGlobalProperty("rect r",                                      &rc);                                                                              assert(r >= 0);
    r = engine->RegisterGlobalFunction("void cfunction1(int)",                        asFUNCTION(cfunction1),    asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("void cfunction2(int,int)",                    asFUNCTION(cfunction2),    asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("void cfunction4(int,int16,int8,int)",         asFUNCTION(cfunction4),    asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("void cfunction4f(float,float,double,float)",  asFUNCTION(cfunction4f),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("void cfunctionmix(int, float, double, int)",  asFUNCTION(cfunctionmix),  asCALL_CDECL);                                          assert(r>= 0);
    r = engine->RegisterGlobalFunction("void cfunctionmix2(int64, float, int8, int)", asFUNCTION(cfunctionmix2), asCALL_CDECL);                                          assert(r>= 0);
    r = engine->RegisterGlobalFunction("void cfunction32(int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int,"
                                                        "int, int, int, int)",
                                       asFUNCTION(cfunction32), asCALL_CDECL); assert(r>=0);
    r = engine->RegisterGlobalFunction("void cfunction32f(float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float,"
                                                        "float, float, float, float)",
                                       asFUNCTION(cfunction32f), asCALL_CDECL); assert(r>=0);
    r = engine->RegisterGlobalFunction("void cfunction32d(double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double,"
                                                        "double, double, double, double)",
                                       asFUNCTION(cfunction32d), asCALL_CDECL); assert(r>=0);
    r = engine->RegisterGlobalFunction("void cfunction32mix(int, int, int, int,"
                                                        "float, float, float, float,"
                                                        "int, int, int, int,"
                                                        "float, float, float, float,"
                                                        "int, int, int, int,"
                                                        "float, float, float, float,"
                                                        "int, int, int, int,"
                                                        "float, float, float, float)",
                                       asFUNCTION(cfunction32mix), asCALL_CDECL); assert(r>=0);
    const char *script =
    "void test_cdecl_return()                     \n"
    "{                                            \n"
    "    assert(reti64() == 0x102030405);         \n"
    "    returned = cfunction_b();                \n"
    "    assert(!retfalse() == cfunction_b());    \n"
    "    assert(retfalse() == false);             \n"
    "    returned = retfalse();                   \n"
    "    assert(!retfalse2() == cfunction_b());   \n"
    "    assert(retfalse2() == false);            \n"
    "    returned2 = retfalse2();                 \n"
    "    returnValue_f = cfunction_f();           \n"
    "    returnValue_d = cfunction_d();           \n"
    "    p=Point();                               \n"
    "    r=Rect();                                \n"
    "}                                            \n"
    ;

    const char *script2 =
    "void test_call_args()                         \n"
    "{                                             \n"
    "    cfunction1(5);                            \n"
    "    cfunction2(5,9);                          \n"
    "    cfunction4(5, 9, 1, 3);                   \n"
    "    cfunction4f(9.2f, 13.3f, 18.8, 3.1415f);  \n"
    "    cfunctionmix(10, 1.92f, 3.88, 97);        \n"
    "    cfunctionmix2(0x102030405, 3, 24, 128);   \n"
    "    cfunction32( 1,  2,  3,  4,"
                    " 5,  6,  7,  8,"
                    " 9, 10, 11, 12,"
                    "13, 14, 15, 16,"
                    "17, 18, 19, 20,"
                    "21, 22, 23, 24,"
                    "25, 26, 27, 28,"
                    "29, 30, 31, 32);\n"
    "    cfunction32f( 1,  2,  3,  4,"
                     " 5,  6,  7,  8,"
                     " 9, 10, 11, 12,"
                     "13, 14, 15, 16,"
                     "17, 18, 19, 20,"
                     "21, 22, 23, 24,"
                     "25, 26, 27, 28,"
                     "29, 30, 31, 32);\n"
    "    cfunction32d( 1,  2,  3,  4,"
                     " 5,  6,  7,  8,"
                     " 9, 10, 11, 12,"
                     "13, 14, 15, 16,"
                     "17, 18, 19, 20,"
                     "21, 22, 23, 24,"
                     "25, 26, 27, 28,"
                     "29, 30, 31, 32);\n"
    "    cfunction32mix("
            " 1,  2,  3,  4,"
            " 5.0f,  6.0f,  7.0f,  8.0f,"
            " 9, 10, 11, 12,"
            "13.0f, 14.0f, 15.0f, 16.0f,"
            "17, 18, 19, 20,"
            "21.0f, 22.0f, 23.0f, 24.0f,"
            "25, 26, 27, 28,"
            "29.0f, 30.0f, 31.0f, 32.0f);\n"
    "}                                             \n"
    ;
    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    mod->AddScriptSection("TestReturn", script, strlen(script), 0);
    mod->AddScriptSection("Testargs", script2, strlen(script2), 0);
    mod->Build();

    asIScriptContext *ctx = engine->CreateContext();
    ctx->Prepare(mod->GetFunctionByDecl("void test_cdecl_return()"));
    r = ctx->Execute();

    assert(r == asEXECUTION_FINISHED);
    assert(!returned);
    assert(!returned2);
    assert(returnValue_f == 18.87f);
    assert(returnValue_d == 88.32);
    assert(p.x == 1 && p.y == 2);
    assert(rc.tl.x == 3 && rc.tl.y == 4 && rc.br.x == 5 && rc.br.y == 6);

    ctx->Prepare(mod->GetFunctionByDecl("void test_call_args()"));
    r = ctx->Execute();

    assert(r == asEXECUTION_FINISHED);
    assert(called1);
    assert(testVal1 == 5);
    assert(called2);
    assert(testVal2 == 5+9);
    assert(called4);
    assert(testVal4);
    assert(called4f);
    assert(testVal4f);
    assert(calledmix);
    assert(testValmix);
    assert(calledmix2);
    assert(testValmix2);
    assert(called32);
    assert(testVal32);
    assert(called32f);
    assert(testVal32f);
    assert(called32d);
    assert(testVal32d);
    assert(called32mix);
    assert(testVal32mix);
    printf("Tests were successful\n");

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
