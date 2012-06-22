/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_VM_INSTANCE_H_
#define incl_VM_INSTANCE_H_

#include "runtime/base/object_data.h"
#include "runtime/base/memory/smart_allocator.h"
#include "runtime/base/array/array_init.h"
#include "runtime/base/runtime_option.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/vm/class.h"
#include "runtime/vm/unit.h"

namespace HPHP {
namespace VM {

class Instance : public ObjectData {
  // Do not declare any fields directly in Instance; instead embed them in
  // ObjectData, so that a property vector can always reside immediately past
  // the end of an object.
public:
  Instance() {}
  // Used by getVTablePtr().
  Instance(const ObjectStaticCallbacks *cb, bool isResource)
    : ObjectData(cb, isResource) {}

public:
  static int ObjAllocatorSizeClassCount;
  //============================================================================
  // Construction/destruction.

  // Call newInstance() instead of directly calling new.
  static Instance* newInstance(Class* cls) {
    const_assert(hhvm);
    if (cls->m_InstanceCtor) {
      return cls->m_InstanceCtor(cls);
    }
    Attr attrs = cls->attrs();
    if (UNLIKELY(attrs & (AttrAbstract | AttrInterface | AttrTrait))) {
      raise_error("Cannot instantiate %s %s",
                  (attrs & AttrInterface) ? "interface" :
                  (attrs & AttrTrait)     ? "trait" : "abstract class",
                  cls->preClass()->name()->data());
    }
    size_t nProps = cls->numDeclProperties();
    size_t size = sizeForNProps(nProps);
    Instance* obj = (Instance*)ALLOCOBJSZ(size);
    new (obj) Instance(cls);
    return obj;
  }

private:
  explicit Instance(Class* cls) : ObjectData(NULL, false, cls) {
    /*
     * During the construction of an instance, the instance has a ref
     * count of zero, and no pointer to it yet exists anywhere the
     * tracing collector can find it.  (I.e., newInstance() hasn't
     * returned, so it isn't on the execution stack or in an Object
     * smart pointer yet.)
     *
     * However, instance creation can sometimes lead to execution of
     * arbitrary code (in the form of an autoload handler).  Moreover
     * it can also lead to memory allocations (which may be a point at
     * which we want to do GC), so we need to register the root for
     * the duration of construction.
     */
    DECLARE_STACK_GC_ROOT(ObjectData, this);

    size_t nProps = cls->numDeclProperties();
    setAttributes(cls->getODAttrs() | (m_cls->clsInfo() ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this
                + sizeof(ObjectData));
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        // initialize() is not inlined because trying to use g_context here
        // trips on a tangle of header dependencies.
        initialize(nProps);
      } else {
        ASSERT(nProps == cls->declPropInit().size());
        memcpy(m_propVec, &cls->declPropInit()[0],
               nProps * sizeof(TypedValue));
      }
    }
    if (UNLIKELY(cls->needInstanceInit())) {
      instanceInit();
    }
  }
protected:
  void initialize(Slot nProps);
  void instanceInit();
public:
  virtual ~Instance() {
  }
public:
  void operator delete(void* p) {
    Instance* this_ = (Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    for (Slot i = 0; i < nProps; ++i) {
      TypedValue* prop = &this_->m_propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps))(this_);
  }
  virtual void destruct() {
    if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
      forgetSweepable();
    }

    if (!noDestruct()) {
      setNoDestruct();
      CountableHelper h(this);
      static StringData* sd__destruct
        = StringData::GetStaticString("__destruct");
      const Func* meth = m_cls->lookupMethod(sd__destruct);
      if (meth != NULL) {
        // We raise the refcount around the call to __destruct(). This is to
        // prevent the refcount from going to zero when the destructor returns.
        destructHard(meth);
      }
    }
  }

private:
  void destructHard(const Func* meth);
  void forgetSweepable();
public:

  //============================================================================
  // Miscellaneous.

  void invokeUserMethod(TypedValue* retval, const Func* method,
                        CArrRef params);

  const Func* methodNamed(const StringData* sd) const {
    return getVMClass()->lookupMethod(sd);
  }

  static size_t sizeForNProps(Slot nProps) {
    return sizeof(Instance) + (sizeof(TypedValue) * nProps);
  }

  static Object FromArray(ArrayData *properties);

  //============================================================================
  // ObjectData glue.

  virtual const String& o_getClassName() const;
  virtual const String& o_getParentName() const;

  virtual Array o_toIterArray(CStrRef context, bool getRef=false);

  virtual void o_setArray(CArrRef properties);
  virtual void o_getArray(Array& props, bool pubOnly=false) const;

  virtual bool o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp, int64 hash = -1);

  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual Variant& ___offsetget_lval(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();

  void cloneSet(ObjectData* clone);
  ObjectData* cloneImpl();

  virtual bool hasCall();
  virtual bool hasCallStatic();

  //============================================================================
  // Properties.
public:
  // public for ObjectData access
  void initDynProps(int numDynamic = 0);
  Slot declPropInd(TypedValue* prop) const;
 private:
  template <bool declOnly>
  TypedValue* getPropImpl(Class* ctx, const StringData* key, bool& visible,
                          bool& accessible, bool& unset);
 public:
  TypedValue* getProp(Class* ctx, const StringData* key, bool& visible,
                      bool& accessible, bool& unset);
  TypedValue* getDeclProp(Class* ctx, const StringData* key, bool& visible,
                          bool& accessible, bool& unset);
private:
  template <bool warn, bool define>
  void propImpl(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
                const StringData* key);
  void invokeSet(TypedValue* retval, const StringData* key, TypedValue* val);
  void invokeGet(TypedValue* retval, const StringData* key);
  void invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                     const StringData* key);
  void invokeIsset(TypedValue* retval, const StringData* key);
  void invokeUnset(TypedValue* retval, const StringData* key);
  void getProp(const Class* klass, bool pubOnly, const PreClass::Prop* prop,
               Array& props, std::vector<bool>& inserted) const;
  void getProps(const Class* klass, bool pubOnly, const PreClass* pc,
                Array& props, std::vector<bool>& inserted) const;
public:
  void prop(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
            const StringData* key);
  void propD(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
             const StringData* key);
  void propW(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
             const StringData* key);
  void propWD(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
              const StringData* key);
  bool propIsset(Class* ctx, const StringData* key);
  bool propEmpty(Class* ctx, const StringData* key);

  TypedValue* setProp(Class* ctx, const StringData* key, TypedValue* val,
                      bool bindingAssignment = false);
  TypedValue* setOpProp(TypedValue& tvRef, Class* ctx, unsigned char op,
                        const StringData* key, Cell* val);
  void incDecProp(TypedValue& tvRef, Class* ctx, unsigned char op,
                  const StringData* key, TypedValue& dest);
  void unsetProp(Class* ctx, const StringData* key);

  void raiseUndefProp(const StringData* name);
};

} } // HPHP::VM

namespace HPHP {

#ifdef HHVM
#define EOD_PARENT HPHP::VM::Instance
#else
#define EOD_PARENT ObjectData
#endif
class ExtObjectData : public EOD_PARENT {
public:
  ExtObjectData(const ObjectStaticCallbacks *cb) :
      EOD_PARENT(cb, false), root(this) {}
  virtual void setRoot(ObjectData *r) { root = r; }
  virtual ObjectData *getRoot() { return root; }
  ObjectData *getBuiltinRoot() { return root; }
protected: ObjectData *root;

};
#undef EOD_PARENT

template <int flags> class ExtObjectDataFlags : public ExtObjectData {
public:
  ExtObjectDataFlags(const ObjectStaticCallbacks *cb) : ExtObjectData(cb) {
    ObjectData::setAttributes(flags);
  }
};
} // HPHP

#endif
