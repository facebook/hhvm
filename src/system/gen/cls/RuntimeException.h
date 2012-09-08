
#ifndef __GENERATED_cls_RuntimeException_h10646350__
#define __GENERATED_cls_RuntimeException_h10646350__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/Exception.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 262 */
FORWARD_DECLARE_CLASS(RuntimeException);
extern const ObjectStaticCallbacks cw_RuntimeException;
class c_RuntimeException : public c_Exception {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(RuntimeException, RuntimeException)
  c_RuntimeException(const ObjectStaticCallbacks *cb = &cw_RuntimeException) : c_Exception(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_RuntimeException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_RuntimeException_h10646350__
