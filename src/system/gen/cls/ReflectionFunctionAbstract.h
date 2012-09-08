
#ifndef __GENERATED_cls_ReflectionFunctionAbstract_h0c29f291__
#define __GENERATED_cls_ReflectionFunctionAbstract_h0c29f291__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 277 */
FORWARD_DECLARE_CLASS(ReflectionFunctionAbstract);
extern const ObjectStaticCallbacks cw_ReflectionFunctionAbstract;
class c_ReflectionFunctionAbstract : public ExtObjectData {
  public:

  // Properties
  Variant m_info;

  // Destructor
  ~c_ReflectionFunctionAbstract() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(ReflectionFunctionAbstract, ReflectionFunctionAbstract, ObjectData)
  static const ClassPropTable os_prop_table;
  c_ReflectionFunctionAbstract(const ObjectStaticCallbacks *cb = &cw_ReflectionFunctionAbstract) : ExtObjectData(cb), m_info(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: Variant t_getname();
  public: bool t_isinternal();
  public: Variant t_getclosure();
  public: bool t_isuserdefined();
  public: Variant t_getfilename();
  public: Variant t_getstartline();
  public: Variant t_getendline();
  public: Variant t_getdoccomment();
  public: Variant t_getstaticvariables();
  public: bool t_returnsreference();
  public: Array t_getparameters();
  public: int64 t_getnumberofparameters();
  public: int64 t_getnumberofrequiredparameters();
  DECLARE_METHOD_INVOKE_HELPERS(getname);
  DECLARE_METHOD_INVOKE_HELPERS(isinternal);
  DECLARE_METHOD_INVOKE_HELPERS(getclosure);
  DECLARE_METHOD_INVOKE_HELPERS(isuserdefined);
  DECLARE_METHOD_INVOKE_HELPERS(getfilename);
  DECLARE_METHOD_INVOKE_HELPERS(getstartline);
  DECLARE_METHOD_INVOKE_HELPERS(getendline);
  DECLARE_METHOD_INVOKE_HELPERS(getdoccomment);
  DECLARE_METHOD_INVOKE_HELPERS(getstaticvariables);
  DECLARE_METHOD_INVOKE_HELPERS(returnsreference);
  DECLARE_METHOD_INVOKE_HELPERS(getparameters);
  DECLARE_METHOD_INVOKE_HELPERS(getnumberofparameters);
  DECLARE_METHOD_INVOKE_HELPERS(getnumberofrequiredparameters);
};
ObjectData *coo_ReflectionFunctionAbstract() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ReflectionFunctionAbstract_h0c29f291__
