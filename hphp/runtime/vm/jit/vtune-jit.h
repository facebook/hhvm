#ifndef VTUNE_JIT_H
#define VTUNE_JIT_H

#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP {
namespace Transl {

void reportTraceletToVtune(const Unit* unit, const Func* func, const TransRec& transRec);
void reportTrampolineToVtune(void* begin, size_t size);

}
}

#endif
