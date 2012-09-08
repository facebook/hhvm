
#ifndef __GENERATED_cls_BadMethodCallException_h0a9d48e5__
#define __GENERATED_cls_BadMethodCallException_h0a9d48e5__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/BadFunctionCallException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 220 */
FORWARD_DECLARE_CLASS(BadMethodCallException);
extern const ObjectStaticCallbacks cw_BadMethodCallException;
class c_BadMethodCallException : public c_BadFunctionCallException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(BadMethodCallException, BadMethodCallException)
  c_BadMethodCallException(const ObjectStaticCallbacks *cb = &cw_BadMethodCallException) : c_BadFunctionCallException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_BadMethodCallException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_BadMethodCallException_h0a9d48e5__
