
#ifndef __GENERATED_cls_ArrayIterator_h7630121e__
#define __GENERATED_cls_ArrayIterator_h7630121e__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/ArrayAccess.h>
#include <cls/SeekableIterator.h>
#include <cls/Countable.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const VarNR &s_sys_svif01bca90;

/* SRC: classes/iterator.php line 345 */
FORWARD_DECLARE_CLASS(ArrayIterator);
extern const ObjectStaticCallbacks cw_ArrayIterator;
class c_ArrayIterator : public ExtObjectData {
  public:

  // Properties
  Variant m_arr;
  Variant m_flags;

  // Destructor
  ~c_ArrayIterator() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(ArrayIterator, ArrayIterator, ObjectData)
  static const ClassPropTable os_prop_table;
  c_ArrayIterator(const ObjectStaticCallbacks *cb = &cw_ArrayIterator) : ExtObjectData(cb), m_arr(Variant::nullInit), m_flags(Variant::nullInit) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_array, Variant v_flags = 0LL);
  public: c_ArrayIterator *create(CVarRef v_array, CVarRef v_flags = NAMVAR(s_sys_svif01bca90, 0LL));
  public: void t_append(CVarRef v_value);
  public: bool t_asort();
  public: int64 t_count();
  public: Variant t_current();
  public: Variant t_getarraycopy();
  public: Variant t_getflags();
  public: Variant t_key();
  public: bool t_ksort();
  public: Variant t_natcasesort();
  public: Variant t_natsort();
  public: void t_next();
  public: bool t_offsetexists(CVarRef v_index);
  public: Variant t_offsetget(Variant v_index);
  public: virtual Variant &___offsetget_lval(Variant v_index);
  public: Variant t_offsetset(CVarRef v_index, CVarRef v_newval);
  public: Variant t_offsetunset(CVarRef v_index);
  public: void t_rewind();
  public: void t_seek(CVarRef v_position);
  public: void t_setflags(CVarRef v_flags);
  public: bool t_uasort(CVarRef v_cmp_function);
  public: bool t_uksort(CVarRef v_cmp_function);
  public: bool t_valid();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(append);
  DECLARE_METHOD_INVOKE_HELPERS(asort);
  DECLARE_METHOD_INVOKE_HELPERS(count);
  DECLARE_METHOD_INVOKE_HELPERS(current);
  DECLARE_METHOD_INVOKE_HELPERS(getarraycopy);
  DECLARE_METHOD_INVOKE_HELPERS(getflags);
  DECLARE_METHOD_INVOKE_HELPERS(key);
  DECLARE_METHOD_INVOKE_HELPERS(ksort);
  DECLARE_METHOD_INVOKE_HELPERS(natcasesort);
  DECLARE_METHOD_INVOKE_HELPERS(natsort);
  DECLARE_METHOD_INVOKE_HELPERS(next);
  DECLARE_METHOD_INVOKE_HELPERS(offsetexists);
  DECLARE_METHOD_INVOKE_HELPERS(offsetget);
  DECLARE_METHOD_INVOKE_HELPERS(offsetset);
  DECLARE_METHOD_INVOKE_HELPERS(offsetunset);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(seek);
  DECLARE_METHOD_INVOKE_HELPERS(setflags);
  DECLARE_METHOD_INVOKE_HELPERS(uasort);
  DECLARE_METHOD_INVOKE_HELPERS(uksort);
  DECLARE_METHOD_INVOKE_HELPERS(valid);
};
ObjectData *coo_ArrayIterator() NEVER_INLINE;
extern const int64 q_ArrayIterator$$STD_PROP_LIST;
extern const int64 q_ArrayIterator$$ARRAY_AS_PROPS;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_ArrayIterator_h7630121e__
