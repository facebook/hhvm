#ifndef incl_HPHP_XHPROF_H
#define incl_HPHP_XHPROF_H

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(xhprof_enable, int64_t flags = 0,
                                  const Array& args = null_array);
Variant HHVM_FUNCTION(xhprof_disable);
void HHVM_FUNCTION(xhprof_network_enable);
Variant HHVM_FUNCTION(xhprof_network_disable);
void HHVM_FUNCTION(xhprof_frame_begin, const String& name);
void HHVM_FUNCTION(xhprof_frame_end);
Variant HHVM_FUNCTION(xhprof_run_trace, const String& packedTrace,
                                        int64_t flags);
void HHVM_FUNCTION(xhprof_sample_enable);
Variant HHVM_FUNCTION(xhprof_sample_disable);
void HHVM_FUNCTION(fb_setprofile, const Variant& callback, int64_t flags);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_XHPROF_H
