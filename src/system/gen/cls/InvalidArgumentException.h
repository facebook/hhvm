
#ifndef __GENERATED_cls_InvalidArgumentException_h1eaa4f7f__
#define __GENERATED_cls_InvalidArgumentException_h1eaa4f7f__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/LogicException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 238 */
FORWARD_DECLARE_CLASS(InvalidArgumentException);
extern const ObjectStaticCallbacks cw_InvalidArgumentException;
class c_InvalidArgumentException : public c_LogicException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(InvalidArgumentException, InvalidArgumentException)
  c_InvalidArgumentException(const ObjectStaticCallbacks *cb = &cw_InvalidArgumentException) : c_LogicException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_InvalidArgumentException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_InvalidArgumentException_h1eaa4f7f__
