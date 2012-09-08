
#ifndef __GENERATED_cls_Directory_h6dfdf420__
#define __GENERATED_cls_Directory_h6dfdf420__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/directory.php line 3 */
FORWARD_DECLARE_CLASS(Directory);
extern const ObjectStaticCallbacks cw_Directory;
class c_Directory : public ExtObjectData {
  public:

  // Properties
  Variant m_path;
  Variant m_handle;

  // Destructor
  ~c_Directory() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(Directory, Directory, ObjectData)
  static const ClassPropTable os_prop_table;
  c_Directory(const ObjectStaticCallbacks *cb = &cw_Directory) : ExtObjectData(cb), m_path(Variant::nullInit), m_handle(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_path);
  public: c_Directory *create(CVarRef v_path);
  public: Variant t_read();
  public: void t_rewind();
  public: void t_close();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(read);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(close);
};
ObjectData *coo_Directory() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_Directory_h6dfdf420__
