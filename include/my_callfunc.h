

#ifdef __INCLUDED_AOTFUNCTION_H
#define __HACK
#endif

#ifndef __HACK
#include <angelscript.h>
#include <as_scriptengine.h>
#include <as_texts.h>
#include "AOTFunction.h"

asIScriptEngine            *m_engine;
int __id;
void *objectPointer;
asDWORD *l_sp;
AOTFunction func(NULL);
#define UNIQUE_CALLSYS_END_LABEL std::string("something")

asSVMRegisters *registers;
#define goto_label hack_label

class AOTCompiler
{
//int CallSystemFunction(int id, asCContext *context, void *objectPointer)
void hack()
#endif

{
    asCScriptEngine            *engine  = (asCScriptEngine*) m_engine;
    asCScriptFunction          *descr   = engine->scriptFunctions[__id];
    asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;

    int callConv = sysFunc->callConv;
    int      popSize           = 0;
    func.m_output += "{\n";
    if( callConv == ICC_GENERIC_FUNC || callConv == ICC_GENERIC_METHOD )
    {
        char buf[128];
        snprintf(buf, 128, "l_sp += context->CallGeneric(%d, objectPointer);\n", __id);
        func.m_output += buf;
        goto goto_label;
    }
    popSize = sysFunc->paramSize;

    char buf[128];
    snprintf(buf, 128, "asCScriptFunction          *descr   = context->m_engine->scriptFunctions[%d];\n", __id);
    func.m_output += buf;
    func.m_output += "asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;\n";
    func.m_output += "asQWORD  retQW             = 0;\n";
    func.m_output += "asQWORD  retQW2            = 0;\n";
    func.m_output += "asDWORD *args              = context->m_regs.stackPointer;\n";
    func.m_output += "void    *retPointer        = 0;\n";
    func.m_output += "void    *obj               = 0;\n";
    func.m_output += "int      popSize           = 0;\n";
    func.m_output += "int      spos              = 0;\n";
    func.m_output += "int      n                 = 0;\n";
    func.m_output += "asSTypeBehaviour *beh      = 0;\n";


    if( callConv >= ICC_THISCALL )
    {
        func.m_output += "        if( objectPointer )\n";
        func.m_output += "        {\n";
        func.m_output += "            obj = objectPointer;\n";
        func.m_output += "        }\n";
        func.m_output += "        else\n";
        func.m_output += "        {\n";
        func.m_output += "            // The object pointer should be popped from the context stack\n";
        func.m_output += "            popSize += AS_PTR_SIZE;\n";
        func.m_output += "\n";
        func.m_output += "            // Check for null pointer\n";
        func.m_output += "            obj = (void*)*(asPWORD*)(args);\n";
        func.m_output += "            if( obj == 0 )\n";
        func.m_output += "            {\n";
        func.m_output += "                context->SetInternalException(TXT_NULL_POINTER_ACCESS);\n";
        func.m_output += "                if( retPointer )\n";
        func.m_output += "                    context->m_engine->CallFree(retPointer);\n";
        func.m_output += "                goto " + UNIQUE_CALLSYS_END_LABEL + ";\n";
        func.m_output += "            }\n";
        func.m_output += "\n";
        func.m_output += "            // Add the base offset for multiple inheritance\n";
        func.m_output += "#if defined(__GNUC__) && defined(AS_ARM)\n";
        func.m_output += "            // On GNUC + ARM the lsb of the offset is used to indicate a virtual function\n";
        func.m_output += "            // and the whole offset is thus shifted one bit left to keep the original\n";
        func.m_output += "            // offset resolution\n";
        func.m_output += "            obj = (void*)(asPWORD(obj) + (sysFunc->baseOffset>>1));\n";
        func.m_output += "#else\n";
        func.m_output += "            obj = (void*)(asPWORD(obj) + sysFunc->baseOffset);\n";
        func.m_output += "#endif\n";
        func.m_output += "\n";
        func.m_output += "            // Skip the object pointer\n";
        func.m_output += "            args += AS_PTR_SIZE;\n";
        func.m_output += "        }\n";
    }

    if( descr->DoesReturnOnStack() )
    {
        // Get the address of the location for the return value from the stack
        func.m_output += "retPointer = (void*)*(asPWORD*)(args);\n";
        popSize += AS_PTR_SIZE;
        func.m_output += "args += AS_PTR_SIZE;\n";

        // When returning the value on the location allocated by the called
        // we shouldn't set the object type in the register
        func.m_output += "context->m_regs.objectType = 0;\n";
    }
    else
    {
        // Set the object type of the reference held in the register
        func.m_output += "context->m_regs.objectType = descr->returnType.GetObjectType();\n";
    }

    func.m_output += "context->m_callingSystemFunction = descr;\n";

    if (sysFunc->hostReturnInMemory)
        callConv++;

    switch (callConv)
    {
#if AOT_ENABLE_QUICKCALL
        case ICC_CDECL:
        case ICC_CDECL_OBJFIRST:
        case ICC_CDECL_OBJLAST:
        case ICC_THISCALL:
        case ICC_STDCALL:
        {
            snprintf(buf, 128, "// %s, %d\n", descr->GetName(), sysFunc->callConv);
            asCDataType &retType = descr->returnType;
            func.m_output += buf;
            snprintf(buf, 128, "// ret: %d, %d, %d, %d, %d, %d, %d\n", sysFunc->hostReturnFloat, retType.IsObject(), retType.IsReference(), retType.IsPrimitive(), retType.GetSizeInMemoryDWords(), retType.GetSizeOnStackDWords(), sysFunc->hostReturnSize);
            func.m_output += buf;
            std::string funcptr("typedef ");
            std::string argsstr;
            switch (retType.GetSizeInMemoryDWords())
            {
                case 0: funcptr += "void ";    break;
                case 1:
                    if (retType.IsFloatType())
                        funcptr += "float ";
                    else
                        funcptr += "asDWORD ";
                    break;
                case 2:
                    if (retType.IsDoubleType())
                        funcptr += "double ";
                    else if (sysFunc->hostReturnFloat)
                    {
                        func.m_output += "typedef struct {float a; float b;} retstruct;\n";
                        funcptr += "retstruct ";
                    }
                    else
                        funcptr += "asQWORD ";
                    break;
                case 3:
                case 4:
                    if (sysFunc->hostReturnFloat)
                        func.m_output += "typedef struct {struct {float a; float b;} a; struct {float a; float b;} b;} retstruct;\n";
                    else
                        func.m_output += "typedef retstruct {asQWORD a; asDWORD b;};";
                    funcptr += "retstruct ";
                    break;
                default: assert(0);
            }
            if (retType.IsReference())
                funcptr += "&";
            func.m_output += "{\n";
            if (callConv == ICC_THISCALL)
                funcptr += "(THISCALL *funcptr)(";
            else if (callConv == ICC_STDCALL)
                funcptr += "(STDCALL *funcptr)(";
            else
                funcptr += "(*funcptr)(";
            int off = 0;
            if (callConv == ICC_CDECL_OBJFIRST || callConv == ICC_THISCALL)
            {
                funcptr += "void*";
                argsstr += "obj";
                if (descr->parameterTypes.GetLength())
                {
                    funcptr += ", ";
                    argsstr += ", ";
                }
            }
            for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
            {
                asCDataType &dt = descr->parameterTypes[n];
                int flags = 0;
                if (dt.IsObject())
                    flags = dt.GetObjectType()->GetFlags();
                snprintf(buf, 128, "// arg%d: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", n, dt.IsObject(), flags,dt.IsObjectHandle(), dt.IsScriptObject(), dt.IsHandleToConst(), dt.IsReference(), dt.IsPrimitive(), dt.CanBeCopied(), dt.CanBeInstanciated(),  dt.GetSizeInMemoryDWords(), dt.GetSizeOnStackDWords());
                func.m_output += buf;

                std::string type = "asDWORD";
                switch (dt.GetSizeOnStackDWords())
                {
                    case 1:
                        if (dt.IsFloatType())
                            type = "float";
                        else
                            type = "asDWORD";
                        break;
                    case 2:
                        if (dt.IsDoubleType())
                            type = "double";
                        else
                            type = "asQWORD";
                        break;
                    default: assert(0);
                }
                funcptr += type;
                if (dt.IsReference())
                {
                    funcptr += "&";
                    if (!dt.IsObjectHandle())
                    {
                        type += "*";
                        argsstr += "*";
                    }
                }
                if (dt.IsObject() && !dt.IsObjectHandle() && !dt.IsReference() && (dt.GetObjectType()->flags & COMPLEX_MASK) == 0)
                {
                    type += "*";
                    argsstr += "*";
                }
                snprintf(buf, 128, "*(%s*)(&args[%d])", type.c_str(), off);
                argsstr += buf;

                if (n < descr->parameterTypes.GetLength()-1)
                {
                    funcptr += ", ";
                    argsstr += ", ";
                }
                off += dt.GetSizeOnStackDWords();
            }
            if (sysFunc->callConv == ICC_CDECL_OBJLAST)
            {
                if (descr->parameterTypes.GetLength())
                {
                    funcptr += ", ";
                    argsstr += ", ";
                }

                funcptr += "void*";
                argsstr += "obj";
            }
            funcptr += ");\n";


            std::string call;
            func.m_output += funcptr;
            func.m_output += "funcptr a = (funcptr)sysFunc->func;\n";
            call = "a(" + argsstr + ");\n";
            if (retType.IsFloatType() || (retType.IsDoubleType() && retType.GetSizeInMemoryDWords() == 1))
                func.m_output += "float ret = " + call + "retQW = *(asDWORD*) &ret;\n";
            else if (retType.IsDoubleType())
                func.m_output += "double ret = " + call + "retQW = *(asQWORD*) &ret;\n";
            else if (retType.GetSizeInMemoryDWords() == 1)
                func.m_output += "retQW = (asQWORD) " + call;
            else if (retType.GetSizeInMemoryDWords() == 2)
            {
                if (sysFunc->hostReturnFloat)
                {
                    func.m_output += "retstruct ret = " + call;
                    func.m_output += "retQW = *(asQWORD*) &ret;\n";
                }
                else
                    func.m_output += "retQW = (asQWORD) " + call;
            }
            else if (retType.GetSizeInMemoryDWords() > 2)
            {
                func.m_output += "retstruct ret = " + call;
                func.m_output += "retQW  = *(asQWORD*) &ret.a;\n";
                func.m_output += "retQW2 = *(asQWORD*) &ret.b;\n";
            }
            else if (retType.GetSizeInMemoryDWords())
                assert(0);
            else
                func.m_output += call;

            func.m_output += "}\n";
            break;
        }
#endif // AOT_ENABLE_QUICKCALL

        default:
        {
            func.m_output += "extern asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &retQW2);\n";
            func.m_output += "retQW = CallSystemFunctionNative(context, descr, obj, args, sysFunc->hostReturnInMemory ? retPointer : 0, retQW2);\n";
        }
    }
    func.m_output += "context->m_callingSystemFunction = 0;\n";

#if defined(COMPLEX_OBJS_PASSED_BY_REF) || defined(AS_LARGE_OBJS_PASSED_BY_REF)
    if( sysFunc->takesObjByVal )
    {
        // Need to free the complex objects passed by value, but that the
        // calling convention implicitly passes by reference behind the scene as the
        // calling function is the owner of that memory.

        // args is pointing to the first real argument as used in CallSystemFunctionNative,
        // i.e. hidden arguments such as the object pointer and return address have already
        // been skipped.

        int spos = 0;
        for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
        {
            bool needFree = false;
            asCDataType &dt = descr->parameterTypes[n];
#ifdef COMPLEX_OBJS_PASSED_BY_REF
            if( dt.GetObjectType() && dt.GetObjectType()->flags & COMPLEX_MASK ) needFree = true;
#endif
#ifdef AS_LARGE_OBJS_PASSED_BY_REF
            if( dt.GetSizeInMemoryDWords() >= AS_LARGE_OBJ_MIN_SIZE ) needFree = true;
#endif
            if( needFree &&
                dt.IsObject() &&
                !dt.IsObjectHandle() &&
                !dt.IsReference() )
            {
                func.m_output += "obj = (void*)*(asPWORD*)&args[spos];\n";
                func.m_output += "spos += AS_PTR_SIZE;\n";
                spos += AS_PTR_SIZE;

#ifndef AS_CALLEE_DESTROY_OBJ_BY_VAL
                // If the called function doesn't destroy objects passed by value we must do so here
                func.m_output += "beh = &descr->parameterTypes[n].GetObjectType()->beh;\n";
                func.m_output += "if( beh->destruct )\n";
                func.m_output += "    context->m_engine->CallObjectMethod(obj, beh->destruct);\n";
#endif

                func.m_output += "context->m_engine->CallFree(obj);\n";
            }
            else
            {
                spos += dt.GetSizeOnStackDWords();
                char buf[32];
                snprintf(buf, 32, "spos += %d;\n", dt.GetSizeOnStackDWords());
                func.m_output += buf;
            }

            func.m_output += "n++;\n";
        }
    }
#endif

    // Store the returned value in our stack
    if( descr->returnType.IsObject() && !descr->returnType.IsReference() )
    {
        if( descr->returnType.IsObjectHandle() )
        {
#if defined(AS_BIG_ENDIAN) && AS_PTR_SIZE == 1
            // Since we're treating the system function as if it is returning a QWORD we are
            // actually receiving the value in the high DWORD of retQW.
            func.m_output += "retQW >>= 32;\n";
#endif

            func.m_output += "registers->objectRegister = (void*)(asPWORD)retQW;\n";

            if( sysFunc->returnAutoHandle)
            {
                func.m_output += "if (context->m_regs.objectRegister )\n";
                func.m_output += "{\n";
                func.m_output += "    asASSERT( !(descr->returnType.GetObjectType()->flags & asOBJ_NOCOUNT) );\n";
                func.m_output += "    context->m_engine->CallObjectMethod(context->m_regs.objectRegister, descr->returnType.GetObjectType()->beh.addref);\n";
                func.m_output += "}\n";
            }
        }
        else
        {
            if( !sysFunc->hostReturnInMemory )
            {
                // Copy the returned value to the pointer sent by the script engine
                if( sysFunc->hostReturnSize == 1 )
                {
#if defined(AS_BIG_ENDIAN) && AS_PTR_SIZE == 1
                    // Since we're treating the system function as if it is returning a QWORD we are
                    // actually receiving the value in the high DWORD of retQW.
                    func.m_output += "retQW >>= 32;\n";
#endif

                    func.m_output += "*(asDWORD*)retPointer = (asDWORD)retQW;\n";
                }
                else if( sysFunc->hostReturnSize == 2 )
                    func.m_output += "*(asQWORD*)retPointer = retQW;\n";
                else if( sysFunc->hostReturnSize == 3 )
                {
                    func.m_output += "*(asQWORD*)retPointer         = retQW;\n";
                    func.m_output += "*(((asDWORD*)retPointer) + 2) = (asDWORD)retQW2;\n";
                }
                else // if( sysFunc->hostReturnSize == 4 )
                {
                    func.m_output += "*(asQWORD*)retPointer         = retQW;\n";
                    func.m_output += "*(((asQWORD*)retPointer) + 1) = retQW2;\n";
                }
            }

            // Store the object in the register
            func.m_output += "context->m_regs.objectRegister = retPointer;\n";

            // If the value is returned on the stack we shouldn't update the object register
            if( descr->DoesReturnOnStack() )
            {
                func.m_output += "context->m_regs.objectRegister = 0;\n";

                func.m_output += "if( context->m_status == asEXECUTION_EXCEPTION )\n";
                func.m_output += "{\n";
                    // If the function raised a script exception it really shouldn't have
                    // initialized the object. However, as it is a soft exception there is
                    // no way for the application to not return a value, so instead we simply
                    // destroy it here, to pretend it was never created.
                func.m_output += "    if( descr->returnType.GetObjectType()->beh.destruct )\n";
                func.m_output += "        context->m_engine->CallObjectMethod(retPointer, descr->returnType.GetObjectType()->beh.destruct);\n";
                func.m_output += "}\n";
            }
        }
    }
    else
    {
        // Store value in value register
        if( sysFunc->hostReturnSize == 1 )
        {
#if defined(AS_BIG_ENDIAN)
            // Since we're treating the system function as if it is returning a QWORD we are
            // actually receiving the value in the high DWORD of retQW.
            retQW >>= 32;

            // Due to endian issues we need to handle return values that are
            // less than a DWORD (32 bits) in size specially
            int numBytes = descr->returnType.GetSizeInMemoryBytes();
            if( descr->returnType.IsReference() ) numBytes = 4;
            switch( numBytes )
            {
            case 1:
                {
                    // 8 bits
                    asBYTE *val = (asBYTE*)&context->m_regs.valueRegister;
                    val[0] = (asBYTE)retQW;
                    val[1] = 0;
                    val[2] = 0;
                    val[3] = 0;
                    val[4] = 0;
                    val[5] = 0;
                    val[6] = 0;
                    val[7] = 0;
                }
                break;
            case 2:
                {
                    // 16 bits
                    asWORD *val = (asWORD*)&context->m_regs.valueRegister;
                    val[0] = (asWORD)retQW;
                    val[1] = 0;
                    val[2] = 0;
                    val[3] = 0;
                }
                break;
            default:
                {
                    // 32 bits
                    asDWORD *val = (asDWORD*)&context->m_regs.valueRegister;
                    val[0] = (asDWORD)retQW;
                    val[1] = 0;
                }
                break;
            }
#else
            func.m_output += "*(asDWORD*)&registers->valueRegister = (asDWORD)retQW;\n";
#endif
        }
        else
            func.m_output += "registers->valueRegister = retQW;\n";
    }

    // Release autohandles in the arguments
    if( sysFunc->hasAutoHandles )
    {
        func.m_output += "args = context->m_regs.stackPointer;\n";
        if( callConv >= ICC_THISCALL)
        {
            func.m_output += "if (!objectPointer )\n";
            func.m_output += "    args += AS_PTR_SIZE;\n";
        }

        int spos = 0;
        func.m_output += "spos = 0;\n";
        for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
        {
            if( sysFunc->paramAutoHandles[n])
            {
                func.m_output += "if (*(asPWORD*)&args[spos] != 0 )\n";
                func.m_output += "{\n";
                func.m_output += "    // Call the release method on the type\n";
                func.m_output += "    context->m_engine->CallObjectMethod((void*)*(asPWORD*)&args[spos], descr->parameterTypes[n].GetObjectType()->beh.release);\n";
                func.m_output += "    *(asPWORD*)&args[spos] = 0;\n";
                func.m_output += "}\n";
            }

            if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() )
            {
                spos += AS_PTR_SIZE;
                func.m_output += "spos += AS_PTR_SIZE;\n";
            }
            else
            {
                spos += descr->parameterTypes[n].GetSizeOnStackDWords();
                char buf[32];
                snprintf(buf, 32, "spos += %d;\n", descr->parameterTypes[n].GetSizeOnStackDWords());
                func.m_output += buf;
            }
        }
    }

    {
        char buf[32];
        snprintf(buf, 32, "l_sp += %d;\n", popSize);
        func.m_output += buf;
    }
    func.m_output += UNIQUE_CALLSYS_END_LABEL + ":\n";

    func.m_output += "l_sp += popSize;\n";
goto_label:
    func.m_output += "}\n";
}
#ifndef __HACK
};
#endif

