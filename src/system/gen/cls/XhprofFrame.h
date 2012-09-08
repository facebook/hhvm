
#ifndef __GENERATED_cls_XhprofFrame_h6d1dc80a__
#define __GENERATED_cls_XhprofFrame_h6d1dc80a__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/xhprof.php line 6 */
FORWARD_DECLARE_CLASS(XhprofFrame);
extern const ObjectStaticCallbacks cw_XhprofFrame;
class c_XhprofFrame : public ExtObjectData {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_NO_SWEEP(XhprofFrame, XhprofFrame, ObjectData)
  c_XhprofFrame(const ObjectStaticCallbacks *cb = &cw_XhprofFrame) : ExtObjectData(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_name);
  public: c_XhprofFrame *create(CVarRef v_name);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);
};
ObjectData *coo_XhprofFrame() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_XhprofFrame_h6d1dc80a__
