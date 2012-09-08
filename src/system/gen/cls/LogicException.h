
#ifndef __GENERATED_cls_LogicException_h4260dc47__
#define __GENERATED_cls_LogicException_h4260dc47__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/Exception.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 200 */
FORWARD_DECLARE_CLASS(LogicException);
extern const ObjectStaticCallbacks cw_LogicException;
class c_LogicException : public c_Exception {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(LogicException, LogicException)
  c_LogicException(const ObjectStaticCallbacks *cb = &cw_LogicException) : c_Exception(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_LogicException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_LogicException_h4260dc47__
