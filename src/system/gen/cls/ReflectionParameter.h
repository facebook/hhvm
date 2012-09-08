
#ifndef __GENERATED_cls_ReflectionParameter_h178d2c14__
#define __GENERATED_cls_ReflectionParameter_h178d2c14__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/Reflector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/reflection.php line 37 */
FORWARD_DECLARE_CLASS(ReflectionParameter);
extern const ObjectStaticCallbacks cw_ReflectionParameter;
class c_ReflectionParameter : public ExtObjectData {
  public:

  // Properties
  Variant m_info;

  // Destructor
  ~c_ReflectionParameter() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(ReflectionParameter, ReflectionParameter, ObjectData)
  static const ClassPropTable os_prop_table;
  c_ReflectionParameter(const ObjectStaticCallbacks *cb = &cw_ReflectionParameter) : ExtObjectData(cb), m_info(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_func, Variant v_param);
  public: c_ReflectionParameter *create(CVarRef v_func, CVarRef v_param);
  public: String t___tostring();
  public: static Variant t_export(CVarRef v_func, CVarRef v_param, CVarRef v_ret = false_varNR);
  public: Variant t_getname();
  public: bool t_ispassedbyreference();
  public: Variant t_getdeclaringclass();
  public: Variant t_getclass();
  public: Variant t_gettypehinttext();
  public: bool t_isarray();
  public: bool t_allowsnull();
  public: bool t_isoptional();
  public: bool t_isdefaultvalueavailable();
  public: Variant t_getdefaultvalue();
  public: Variant t_getdefaultvaluetext();
  public: Variant t_getposition();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(__tostring);
  DECLARE_METHOD_INVOKE_HELPERS(export);
  DECLARE_METHOD_INVOKE_HELPERS(getname);
  DECLARE_METHOD_INVOKE_HELPERS(ispassedbyreference);
  DECLARE_METHOD_INVOKE_HELPERS(getdeclaringclass);
  DECLARE_METHOD_INVOKE_HELPERS(getclass);
  DECLARE_METHOD_INVOKE_HELPERS(gettypehinttext);
  DECLARE_METHOD_INVOKE_HELPERS(isarray);
  DECLARE_METHOD_INVOKE_HELPERS(allowsnull);
  DECLARE_METHOD_INVOKE_HELPERS(isoptional);
  DECLARE_METHOD_INVOKE_HELPERS(isdefaultvalueavailable);
  DECLARE_METHOD_INVOKE_HELPERS(getdefaultvalue);
  DECLARE_METHOD_INVOKE_HELPERS(getdefaultvaluetext);
  DECLARE_METHOD_INVOKE_HELPERS(getposition);
};
ObjectData *coo_ReflectionParameter() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ReflectionParameter_h178d2c14__
