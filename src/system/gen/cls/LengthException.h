
#ifndef __GENERATED_cls_LengthException_h78d45e3c__
#define __GENERATED_cls_LengthException_h78d45e3c__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/LogicException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 246 */
FORWARD_DECLARE_CLASS(LengthException);
extern const ObjectStaticCallbacks cw_LengthException;
class c_LengthException : public c_LogicException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(LengthException, LengthException)
  c_LengthException(const ObjectStaticCallbacks *cb = &cw_LengthException) : c_LogicException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_LengthException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_LengthException_h78d45e3c__
