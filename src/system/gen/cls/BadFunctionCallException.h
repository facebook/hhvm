
#ifndef __GENERATED_cls_BadFunctionCallException_h1998da7d__
#define __GENERATED_cls_BadFunctionCallException_h1998da7d__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/LogicException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 210 */
FORWARD_DECLARE_CLASS(BadFunctionCallException);
extern const ObjectStaticCallbacks cw_BadFunctionCallException;
class c_BadFunctionCallException : public c_LogicException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(BadFunctionCallException, BadFunctionCallException)
  c_BadFunctionCallException(const ObjectStaticCallbacks *cb = &cw_BadFunctionCallException) : c_LogicException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_BadFunctionCallException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_BadFunctionCallException_h1998da7d__
