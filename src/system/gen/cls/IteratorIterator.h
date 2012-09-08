
#ifndef __GENERATED_cls_IteratorIterator_h0fa235b1__
#define __GENERATED_cls_IteratorIterator_h0fa235b1__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/OuterIterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/iterator.php line 661 */
FORWARD_DECLARE_CLASS(IteratorIterator);
extern const ObjectStaticCallbacks cw_IteratorIterator;
class c_IteratorIterator : public ExtObjectData {
  public:

  // Properties
  Variant m_iterator;

  // Destructor
  ~c_IteratorIterator() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(IteratorIterator, IteratorIterator, ObjectData)
  static const ClassPropTable os_prop_table;
  c_IteratorIterator(const ObjectStaticCallbacks *cb = &cw_IteratorIterator) : ExtObjectData(cb), m_iterator(Variant::nullInit) {
    setAttribute(HasCall);
    if (!hhvm) setAttribute(NoDestructor);
  }
  Variant doCall(Variant v_name, Variant v_arguments, bool fatal);
  public: void t___construct(Variant v_iterator);
  public: c_IteratorIterator *create(CVarRef v_iterator);
  public: Variant t_getinneriterator();
  public: Variant t_valid();
  public: Variant t_key();
  public: Variant t_current();
  public: Variant t_next();
  public: Variant t_rewind();
  public: Variant t___call(Variant v_func, Variant v_params);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(getinneriterator);
  DECLARE_METHOD_INVOKE_HELPERS(valid);
  DECLARE_METHOD_INVOKE_HELPERS(key);
  DECLARE_METHOD_INVOKE_HELPERS(current);
  DECLARE_METHOD_INVOKE_HELPERS(next);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(__call);
};
ObjectData *coo_IteratorIterator() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_IteratorIterator_h0fa235b1__
