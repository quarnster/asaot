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



int main(int argc, char const *argv[])
{
    asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    int r;

    float returnValue_f = 0.0f;
    double returnValue_d = 0.0f;
    bool returned = false;
    bool returned2 = true;
    asPoint p={0,0};
    asRect rc={{0,0},{0,0}};
    r = engine->RegisterObjectType(    "point",                sizeof(asPoint), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);
    r = engine->RegisterObjectType(    "rect",                 sizeof(asRect), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS|asOBJ_APP_CLASS_ALLFLOATS);  assert(r >= 0);
    r = engine->RegisterGlobalFunction("void assert(bool)",    asFUNCTION(Assert),        asCALL_GENERIC);                                        assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool cfunction_b()",   asFUNCTION(cfunction_b),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool retfalse()",      asFUNCTION(retfalse),      asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool retfalse2()",     asFUNCTION(retfalse_fake), asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("int64 reti64()",       asFUNCTION(reti64),        asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("float cfunction_f()",  asFUNCTION(cfunction_f),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("double cfunction_d()", asFUNCTION(cfunction_d),   asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("point Point()",        asFUNCTION(TestPoint),     asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalFunction("rect Rect()",          asFUNCTION(TestRect),      asCALL_CDECL);                                          assert(r >= 0);
    r = engine->RegisterGlobalProperty("double returnValue_d", &returnValue_d);                                                                   assert(r >= 0);
    r = engine->RegisterGlobalProperty("float returnValue_f",  &returnValue_f);                                                                   assert(r >= 0);
    r = engine->RegisterGlobalProperty("bool returned",        &returned);                                                                        assert(r >= 0);
    r = engine->RegisterGlobalProperty("bool returned2",       &returned2);                                                                       assert(r >= 0);
    r = engine->RegisterGlobalProperty("point p",              &p);                                                                               assert(r >= 0);
    r = engine->RegisterGlobalProperty("rect r",               &rc);                                                                              assert(r >= 0);

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

    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    mod->AddScriptSection("TestReturn", script, strlen(script), 0);
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

    printf("Tests were successful\n");

    ctx->Release();
    engine->Release();
    engine = NULL;
    return 0;
}
