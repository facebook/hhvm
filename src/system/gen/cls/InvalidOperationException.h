
#ifndef __GENERATED_cls_InvalidOperationException_h3db7dd88__
#define __GENERATED_cls_InvalidOperationException_h3db7dd88__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/RuntimeException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 306 */
FORWARD_DECLARE_CLASS(InvalidOperationException);
extern const ObjectStaticCallbacks cw_InvalidOperationException;
class c_InvalidOperationException : public c_RuntimeException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(InvalidOperationException, InvalidOperationException)
  c_InvalidOperationException(const ObjectStaticCallbacks *cb = &cw_InvalidOperationException) : c_RuntimeException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_InvalidOperationException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_InvalidOperationException_h3db7dd88__
