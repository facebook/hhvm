#ifndef incl_HPHP_JIT_DEBUG_GUARDS_H
#define incl_HPHP_JIT_DEBUG_GUARDS_H

#include "hphp/runtime/vm/srckey.h"

namespace HPHP {
namespace Transl { class SrcRec; }
namespace JIT {

void addDbgGuardImpl(SrcKey sk, Transl::SrcRec* srcRec);

}}

#endif
