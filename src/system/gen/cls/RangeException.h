
#ifndef __GENERATED_cls_RangeException_h509e241f__
#define __GENERATED_cls_RangeException_h509e241f__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 286 */
FORWARD_DECLARE_CLASS(RangeException);
extern const ObjectStaticCallbacks cw_RangeException;
class c_RangeException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(RangeException, RangeException)
  c_RangeException(const ObjectStaticCallbacks *cb = &cw_RangeException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_RangeException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_RangeException_h509e241f__
