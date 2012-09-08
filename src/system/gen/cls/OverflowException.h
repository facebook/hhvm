
#ifndef __GENERATED_cls_OverflowException_h2fddf195__
#define __GENERATED_cls_OverflowException_h2fddf195__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 278 */
FORWARD_DECLARE_CLASS(OverflowException);
extern const ObjectStaticCallbacks cw_OverflowException;
class c_OverflowException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(OverflowException, OverflowException)
  c_OverflowException(const ObjectStaticCallbacks *cb = &cw_OverflowException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_OverflowException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_OverflowException_h2fddf195__
