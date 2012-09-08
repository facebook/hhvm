
#ifndef __GENERATED_cls_stdClass_h52e0228e__
#define __GENERATED_cls_stdClass_h52e0228e__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/stdclass.php line 4 */
FORWARD_DECLARE_CLASS(stdClass);
extern const ObjectStaticCallbacks cw_stdClass;
class c_stdClass : public ExtObjectData {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(stdClass, stdClass)
  c_stdClass(const ObjectStaticCallbacks *cb = &cw_stdClass) : ExtObjectData(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_stdClass() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_stdClass_h52e0228e__
