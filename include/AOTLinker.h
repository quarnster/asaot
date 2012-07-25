/*
 * Copyright (c) 2012 Fredrik Ehnbom
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */
#ifndef __INCLUDED_AOTLINKER_H
#define __INCLUDED_AOTLINKER_H

#include <angelscript.h>
#include "AOTFunction.h"
#include <string>
#include <vector>

class AOTLinker
{
public:
    virtual ~AOTLinker() {}
    enum LinkerResult
    {
        /// Tell the compiler to generate code
        GenerateCode,
        /// Tell the compiler that the linking was successful, so don't generate code,
        /// but do modify the jitEntry bytecode arguments to the appropriate values.
        LinkSuccessful,
        /// Just make the Compiler return immediately without further action with a successful error code
        AllDone,
        /// Just make the Compiler return immediately without further action with an unsuccessful error code
        ErrorOccurred
    };
    /// Used to look up AOT compiled script functions and provide the asJITFunction
    /// to AngelScript
    virtual LinkerResult LookupFunction(AOTFunction *function, asJITFunction *jitFunction) = 0;

    /// Any additional code additions or transformations the Linker wants to do
    virtual void LinkTimeCodeGeneration(std::string &code, std::vector<AOTFunction> &compiledFunctions) = 0;
};

#endif
