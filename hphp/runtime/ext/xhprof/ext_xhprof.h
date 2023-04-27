#pragma once

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(xhprof_enable, int64_t flags = 0,
                                  const Array& args = null_array);
Variant HHVM_FUNCTION(xhprof_disable);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
