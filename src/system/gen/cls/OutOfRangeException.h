
#ifndef __GENERATED_cls_OutOfRangeException_h54da4fc9__
#define __GENERATED_cls_OutOfRangeException_h54da4fc9__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/LogicException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 254 */
FORWARD_DECLARE_CLASS(OutOfRangeException);
extern const ObjectStaticCallbacks cw_OutOfRangeException;
class c_OutOfRangeException : public c_LogicException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(OutOfRangeException, OutOfRangeException)
  c_OutOfRangeException(const ObjectStaticCallbacks *cb = &cw_OutOfRangeException) : c_LogicException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_OutOfRangeException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_OutOfRangeException_h54da4fc9__
