
#ifndef __GENERATED_cls_ErrorException_h0c3c27ba__
#define __GENERATED_cls_ErrorException_h0c3c27ba__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/Exception.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern StaticStringProxy s_sys_ssp00000000;
#ifndef s_sys_ss00000000
#define s_sys_ss00000000 (*(StaticString *)(&s_sys_ssp00000000))
#endif

extern const VarNR &s_sys_svif01bca90;

extern VariantProxy s_sys_svsp00000000;
#ifndef s_sys_svs00000000
#define s_sys_svs00000000 (*(Variant *)&s_sys_svsp00000000)
#endif

/* SRC: classes/exception.php line 315 */
FORWARD_DECLARE_CLASS(ErrorException);
extern const ObjectStaticCallbacks cw_ErrorException;
class c_ErrorException : public c_Exception {
  public:

  // Properties
  Variant m_severity;

  // Destructor
  ~c_ErrorException() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(ErrorException, ErrorException, Exception)
  static const ClassPropTable os_prop_table;
  c_ErrorException(const ObjectStaticCallbacks *cb = &cw_ErrorException) : c_Exception(cb), m_severity(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_message = NAMSTR(s_sys_ss00000000, ""), Variant v_code = 0LL, Variant v_severity = 0LL, Variant v_filename = null, Variant v_lineno = null);
  public: c_ErrorException *create(CVarRef v_message = NAMVAR(s_sys_svs00000000, ""), CVarRef v_code = NAMVAR(s_sys_svif01bca90, 0LL), CVarRef v_severity = NAMVAR(s_sys_svif01bca90, 0LL), CVarRef v_filename = null_variant, CVarRef v_lineno = null_variant);
  public: Variant t_getseverity();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(getseverity);
};
ObjectData *coo_ErrorException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ErrorException_h0c3c27ba__
