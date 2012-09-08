
#ifndef __GENERATED_cls_ReflectionObject_h0fef8256__
#define __GENERATED_cls_ReflectionObject_h0fef8256__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/ReflectionClass.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 1403 */
FORWARD_DECLARE_CLASS(ReflectionObject);
extern const ObjectStaticCallbacks cw_ReflectionObject;
class c_ReflectionObject : public c_ReflectionClass {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_NO_SWEEP(ReflectionObject, ReflectionObject, ReflectionClass)
  c_ReflectionObject(const ObjectStaticCallbacks *cb = &cw_ReflectionObject) : c_ReflectionClass(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: static Variant t_export(Variant v_obj, CVarRef v_ret = false_varNR);
  DECLARE_METHOD_INVOKE_HELPERS(export);
};
ObjectData *coo_ReflectionObject() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ReflectionObject_h0fef8256__
