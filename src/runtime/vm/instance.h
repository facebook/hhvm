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

 private:
  // This constructor is used for all pure classes that are not
  // descendents of cppext classes
  explicit Instance(Class* cls) : ObjectData(NULL, false, cls) {
    instanceInit(cls);
  }

  enum NoInit { noinit };
  explicit Instance(Class* cls, NoInit) : ObjectData(NULL, false, cls) {}

 public:
  // This constructor is used for all cppext classes (including resources)
  // and their descendents.
  Instance(const ObjectStaticCallbacks *cb, bool isResource)
    : ObjectData(NULL, isResource) {
    if (ObjectStaticCallbacks::isEncodedVMClass(cb)) {
      m_cls = ObjectStaticCallbacks::decodeVMClass(cb);
    } else {
      m_cls = *cb->os_cls_ptr;
    }
    instanceInit(m_cls);
  }

  virtual ~Instance() {}

  static int ObjAllocatorSizeClassCount;

  // Call newInstance() to instantiate an Instance
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

  // Given a Class that is assumed to be a concrete, regular (not a
  // trait or interface), pure PHP class, and an allocator index,
  // return a new, uninitialized object of that class.
  static Instance* newInstanceRaw(Class* cls, int idx);

 private:
  void instanceInit(Class* cls) {
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
    setAttributes(cls->getODAttrs());
    size_t nProps = cls->numDeclProperties();
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        const Class::PropInitVec* propInitVec = m_cls->getPropData();
        ASSERT(propInitVec != NULL);
        ASSERT(nProps == propInitVec->size());
        memcpy(propVec(), &(*propInitVec)[0], nProps * sizeof(TypedValue));
      } else {
        ASSERT(nProps == cls->declPropInit().size());
        memcpy(propVec(), &cls->declPropInit()[0], nProps * sizeof(TypedValue));
      }
    }
    if (UNLIKELY(cls->callsCustomInstanceInit())) {
      callCustomInstanceInit();
    }
  }

 protected:
  TypedValue* propVec();
  const TypedValue* propVec() const;

 public:
  Instance* callCustomInstanceInit();

  void operator delete(void* p) {
    Instance* this_ = (Instance*)p;
    Class* cls = this_->getVMClass();
    size_t nProps = cls->numDeclProperties();
    // cppext classes have their own implementation of delete
    ASSERT(this_->builtinPropSize() == 0);
    TypedValue* propVec = (TypedValue *)((uintptr_t)this_ + sizeof(ObjectData));
    for (unsigned i = 0; i < nProps; ++i) {
      TypedValue* prop = &propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps))(this_);
  }

 private:
  void destructHard(const Func* meth);
  void forgetSweepable();

  //============================================================================
  // Virtual ObjectData methods that we need to override

 public:
  virtual void destruct() {
    if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
      forgetSweepable();
    }
    if (!noDestruct()) {
      setNoDestruct();
      if (const Func* meth = m_cls->getDtor()) {
        // We raise the refcount around the call to __destruct(). This is to
        // prevent the refcount from going to zero when the destructor returns.
        CountableHelper h(this);
        destructHard(meth);
      }
    }
  }

  virtual Array o_toIterArray(CStrRef context, bool getRef=false);

  virtual void o_setArray(CArrRef properties);
  virtual void o_getArray(Array& props, bool pubOnly=false) const;

  virtual bool o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp,
                                    strhash_t hash = -1);

  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual Variant& ___offsetget_lval(Variant key);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();

  //============================================================================
  // Miscellaneous.

  void cloneSet(ObjectData* clone);
  ObjectData* cloneImpl();

  void invokeUserMethod(TypedValue* retval, const Func* method,
                        CArrRef params);

  const Func* methodNamed(const StringData* sd) const {
    return getVMClass()->lookupMethod(sd);
  }

  static size_t sizeForNProps(Slot nProps) {
    size_t sz = sizeof(Instance) + (sizeof(TypedValue) * nProps);
    ASSERT((sz & (sizeof(TypedValue) - 1)) == 0);
    return sz;
  }

  static Object FromArray(ArrayData *properties);

  //============================================================================
  // Properties.
 public:
  int builtinPropSize() const {
    return m_cls->builtinPropSize();
  }

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
 private:
  template <bool setResult>
  void incDecPropImpl(TypedValue& tvRef, Class* ctx, unsigned char op,
                      const StringData* key, TypedValue& dest);
 public:
  template <bool setResult>
  void incDecProp(TypedValue& tvRef, Class* ctx, unsigned char op,
                  const StringData* key, TypedValue& dest);
  void unsetProp(Class* ctx, const StringData* key);

  void raiseUndefProp(const StringData* name);
};

inline Instance* instanceFromTv(TypedValue* tv) {
  ASSERT(dynamic_cast<Instance*>(tv->m_data.pobj));
  return static_cast<Instance*>(tv->m_data.pobj);
}

} } // HPHP::VM

namespace HPHP {

#ifdef HHVM
class ExtObjectData : public HPHP::VM::Instance {
 public:
  ExtObjectData(const ObjectStaticCallbacks *cb)
    : HPHP::VM::Instance(cb, false) {}
  virtual void setRoot(ObjectData *r) {}
  virtual ObjectData *getRoot() { return this; }
  ObjectData *getBuiltinRoot() { return this; }
};
#else
class ExtObjectData : public ObjectData {
 public:
  ExtObjectData(const ObjectStaticCallbacks *cb)
    : ObjectData(cb, false), root(this) {}
  virtual void setRoot(ObjectData *r) { root = r; }
  virtual ObjectData *getRoot() { return root; }
  ObjectData *getBuiltinRoot() { return root; }
 protected:
  ObjectData *root;
};
#endif

template <int flags> class ExtObjectDataFlags : public ExtObjectData {
 public:
  ExtObjectDataFlags(const ObjectStaticCallbacks *cb) : ExtObjectData(cb) {
    ObjectData::setAttributes(flags);
  }
};

} // HPHP

#endif
