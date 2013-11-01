#include <vector>

#include "vtune-jit.h"
#include "jitapi/jitprofiling.h"

namespace HPHP {
namespace Transl {

void reportTraceletToVtune(const Unit* unit, const Func* func, const TransRec& transRec)
{
    iJIT_Method_Load methodInfo;
    memset(&methodInfo, 0, sizeof(methodInfo));

    if (!unit) return;

    methodInfo.method_id = transRec.src.getFuncId() + 1000;

    if (func && func->fullName())
    {
        methodInfo.method_name = const_cast<char *>(func->fullName()->data());
    }
    else
    {
        methodInfo.method_name = const_cast<char *>("unknown");
    }

    methodInfo.source_file_name = const_cast<char *>(unit->filepath()->data());

    // Report main body

    methodInfo.method_load_address = transRec.aStart;
    methodInfo.method_size = transRec.aLen;

    std::vector<LineNumberInfo> lineMapping;
    
    unsigned bcSize = transRec.bcMapping.size();
    LineNumberInfo lineInfo;

    // Attribute prologue to first bytecode; note that offset marks the
    // *end* of corresponding line
    if (bcSize > 0)
    {
        lineInfo.Offset = transRec.bcMapping[0].aStart - transRec.aStart;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[0].bcStart);
        lineMapping.push_back(lineInfo);
    }
    
    for(unsigned i = 1; i < bcSize; i++)
    {
        lineInfo.Offset = transRec.bcMapping[i].aStart - transRec.aStart;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[i - 1].bcStart);
        lineMapping.push_back(lineInfo);
    }

    // Report the code until tracelet end to the line corresponding to last bytecode
    if (bcSize > 0)
    {
        lineInfo.Offset = transRec.aLen;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[bcSize - 1].bcStart);
        lineMapping.push_back(lineInfo);
    }

    methodInfo.line_number_size = bcSize == 0 ? bcSize : bcSize + 1;
    methodInfo.line_number_table = &lineMapping[0];

    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void *)&methodInfo);

    // Report stubs

    methodInfo.method_load_address = transRec.astubsStart;
    methodInfo.method_size = transRec.astubsLen;
    lineMapping.clear();

    if (bcSize > 0)
    {
        lineInfo.Offset = transRec.bcMapping[0].astubsStart - transRec.astubsStart;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[0].bcStart);
        lineMapping.push_back(lineInfo);
    }
    
    for(unsigned i = 1; i < bcSize; i++)
    {
        lineInfo.Offset = transRec.bcMapping[i].astubsStart - transRec.astubsStart;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[i - 1].bcStart);
        lineMapping.push_back(lineInfo);
    }

    if (bcSize > 0)
    {
        lineInfo.Offset = transRec.astubsLen;
        lineInfo.LineNumber = unit->getLineNumber(transRec.bcMapping[bcSize - 1].bcStart);
        lineMapping.push_back(lineInfo);
    }

    methodInfo.line_number_table = &lineMapping[0];

    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void *)&methodInfo);
}

void reportTrampolineToVtune(void* begin, size_t size)
{
    iJIT_Method_Load methodInfo;
    memset(&methodInfo, 0, sizeof(methodInfo));

    methodInfo.method_id = 1000;

    methodInfo.method_name = const_cast<char *>("Trampoline");

    methodInfo.source_file_name = const_cast<char *>("Undefined");

    // Report main body

    methodInfo.method_load_address = begin;
    methodInfo.method_size = size;

    methodInfo.line_number_size = 0;
    methodInfo.line_number_table = 0;

    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void *)&methodInfo);
}

}
}
