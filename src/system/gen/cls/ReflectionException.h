
#ifndef __GENERATED_cls_ReflectionException_h54ca6983__
#define __GENERATED_cls_ReflectionException_h54ca6983__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/Exception.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 18 */
FORWARD_DECLARE_CLASS(ReflectionException);
extern const ObjectStaticCallbacks cw_ReflectionException;
class c_ReflectionException : public c_Exception {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(ReflectionException, ReflectionException)
  c_ReflectionException(const ObjectStaticCallbacks *cb = &cw_ReflectionException) : c_Exception(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_ReflectionException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ReflectionException_h54ca6983__
