
#ifndef __GENERATED_cls_UnderflowException_h2fad4bbb__
#define __GENERATED_cls_UnderflowException_h2fad4bbb__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 295 */
FORWARD_DECLARE_CLASS(UnderflowException);
extern const ObjectStaticCallbacks cw_UnderflowException;
class c_UnderflowException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(UnderflowException, UnderflowException)
  c_UnderflowException(const ObjectStaticCallbacks *cb = &cw_UnderflowException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_UnderflowException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_UnderflowException_h2fad4bbb__
