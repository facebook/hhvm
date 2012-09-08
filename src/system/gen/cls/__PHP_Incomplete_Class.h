
#ifndef __GENERATED_cls___PHP_Incomplete_Class_h114d7894__
#define __GENERATED_cls___PHP_Incomplete_Class_h114d7894__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/stdclass.php line 8 */
FORWARD_DECLARE_CLASS(__PHP_Incomplete_Class);
extern const ObjectStaticCallbacks cw___PHP_Incomplete_Class;
class c___PHP_Incomplete_Class : public ExtObjectData {
  public:

  // Properties
  Variant m___PHP_Incomplete_Class_Name;

  // Destructor
  ~c___PHP_Incomplete_Class() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(__PHP_Incomplete_Class, __PHP_Incomplete_Class)
  static const ClassPropTable os_prop_table;
  c___PHP_Incomplete_Class(const ObjectStaticCallbacks *cb = &cw___PHP_Incomplete_Class) : ExtObjectData(cb), m___PHP_Incomplete_Class_Name(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo___PHP_Incomplete_Class() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls___PHP_Incomplete_Class_h114d7894__
