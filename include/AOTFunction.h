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
#ifndef __INCLUDED_AOTFUNCTION_H
#define __INCLUDED_AOTFUNCTION_H

#include <string>
#include <angelscript.h>


class AOTFunction
{
public:
    AOTFunction(asIScriptFunction *func);

    const std::string&       GetName() const;
    const asIScriptFunction* GetScriptFunction() const;
private:
    std::string              m_output;
    std::string              m_name;
    unsigned int             m_labelCount;

    asIScriptFunction*       m_scriptFunction;
    friend class AOTCompiler;
};

#endif
