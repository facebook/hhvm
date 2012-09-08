
#ifndef __GENERATED_cls_UnexpectedValueException_h4e6f5a94__
#define __GENERATED_cls_UnexpectedValueException_h4e6f5a94__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 304 */
FORWARD_DECLARE_CLASS(UnexpectedValueException);
extern const ObjectStaticCallbacks cw_UnexpectedValueException;
class c_UnexpectedValueException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(UnexpectedValueException, UnexpectedValueException)
  c_UnexpectedValueException(const ObjectStaticCallbacks *cb = &cw_UnexpectedValueException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_UnexpectedValueException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_UnexpectedValueException_h4e6f5a94__
