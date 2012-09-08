
#ifndef __GENERATED_cls_OutOfBoundsException_h3d378326__
#define __GENERATED_cls_OutOfBoundsException_h3d378326__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 270 */
FORWARD_DECLARE_CLASS(OutOfBoundsException);
extern const ObjectStaticCallbacks cw_OutOfBoundsException;
class c_OutOfBoundsException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(OutOfBoundsException, OutOfBoundsException)
  c_OutOfBoundsException(const ObjectStaticCallbacks *cb = &cw_OutOfBoundsException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_OutOfBoundsException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_OutOfBoundsException_h3d378326__
