#ifndef __INCLUDED_COMMON_H
#define __INCLUDED_COMMON_H

#include <angelscript.h>
#include <AOTCompiler.h>
#include <stdio.h>
#include <string.h>


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

class CCodeStream : public asIBinaryStream
{
public:
    CCodeStream(const char *filename)
    {
        m_fp = fopen(filename, "w");
    }
    ~CCodeStream()
    {
        fclose(m_fp);
    }
    virtual void Read(void *ptr, asUINT size)
    {
        // Not really needed.
    }
    virtual void Write(const void *ptr, asUINT size)
    {
        fwrite(ptr, size, 1, m_fp);
    }
private:
    FILE *m_fp;
};

typedef struct
{
    const char name[256];
    asJITFunction entry;
} AOTLinkerEntry;

class SimpleAOTLinker : public AOTLinker
{
public:
    SimpleAOTLinker(AOTLinkerEntry *linkerTable=NULL, unsigned int linkerTableSize=-1)
    : m_linkerTable(linkerTable), m_linkerTableSize(linkerTableSize)
    {

    }

    virtual LinkerResult LookupFunction(AOTFunction *function, asJITFunction *jitFunction)
    {
        if (m_linkerTable && m_linkerTableSize)
        {
            for (unsigned int i = 0; i < m_linkerTableSize; i++)
            {
                if (function->GetName() == m_linkerTable[i].name)
                {
                    *jitFunction = m_linkerTable[i].entry;
                    return LinkSuccessful;
                }
            }
        }
        return GenerateCode;
    }

    virtual void LinkTimeCodeGeneration(std::string &code, std::vector<AOTFunction> &compiledFunctions)
    {
        code += "typedef struct\n";
        code += "{\n";
        code += "    const char name[256];\n";
        code += "    asJITFunction entry;\n";
        code += "} AOTLinkerEntry;\n";
        code += "";
        char buf[512];
        snprintf(buf, 512, "\nunsigned int AOTLinkerTableSize = %d;\n", (int) compiledFunctions.size());

        code += buf;
        code += "AOTLinkerEntry AOTLinkerTable[] =\n{\n";
        for (std::vector<AOTFunction>::iterator i = compiledFunctions.begin(); i < compiledFunctions.end(); i++)
        {
            snprintf(buf, 512, "{\"%s\", %s},\n", (*i).GetName().c_str(), (*i).GetName().c_str());
            code += buf;
        }
        code += "};\n";
    }
private:
    AOTLinkerEntry *m_linkerTable;
    unsigned int m_linkerTableSize;
};

#endif

