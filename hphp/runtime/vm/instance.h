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

#ifndef incl_HPHP_VM_INSTANCE_H_
#define incl_HPHP_VM_INSTANCE_H_

#include "runtime/base/complex_types.h"
#include "runtime/base/memory/smart_allocator.h"
#include "runtime/base/array/array_init.h"
#include "runtime/base/runtime_option.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/vm/class.h"
#include "runtime/vm/unit.h"

namespace HPHP {
namespace VM {

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps);

class Instance : public ObjectData {
  // Do not declare any fields directly in Instance; instead embed them in
  // ObjectData, so that a property vector can always reside immediately past
  // the end of an object.

 private:
  // This constructor is used for all pure classes that are not
  // descendents of cppext classes
  explicit Instance(Class* cls) : ObjectData(false, cls) {
    instanceInit(cls);
  }

  enum NoInit { noinit };
  explicit Instance(Class* cls, NoInit) : ObjectData(false, cls) {}

 public:
  // This constructor is used for all cppext classes (including resources)
  // and their descendents.
  Instance(Class* cls, bool isResource)
      : ObjectData(isResource, cls) {
    instanceInit(cls);
  }

  virtual ~Instance() {}

  static int ObjAllocatorSizeClassCount;

  // Call newInstance() to instantiate an Instance
  static Instance* newInstance(Class* cls) {
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
    if (UNLIKELY(cls->callsCustomInstanceInit())) {
      /*
        This must happen after the constructor finishes,
        because it can leak references to obj AND it can
        throw exceptions. If we have this in the Instance
        constructor, and it throws, obj will be partially
        destroyed (ie ~ObjectData will be called, resetting
        the vtable pointer) leaving dangling references
        to the object (eg in backtraces).
      */
      obj->callCustomInstanceInit();
    }
    return obj;
  }

  // Given a Class that is assumed to be a concrete, regular (not a
  // trait or interface), pure PHP class, and an allocator index,
  // return a new, uninitialized object of that class.
  static Instance* newInstanceRaw(Class* cls, int idx);

 private:
  void instanceInit(Class* cls) {
    setAttributes(cls->getODAttrs());
    size_t nProps = cls->numDeclProperties();
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        const Class::PropInitVec* propInitVec = m_cls->getPropData();
        assert(propInitVec != nullptr);
        assert(nProps == propInitVec->size());
        if (!cls->hasDeepInitProps()) {
          memcpy(propVec(), &(*propInitVec)[0], nProps * sizeof(TypedValue));
        } else {
          deepInitHelper(propVec(), &(*propInitVec)[0], nProps);
        }
      } else {
        assert(nProps == cls->declPropInit().size());
        memcpy(propVec(), &cls->declPropInit()[0], nProps * sizeof(TypedValue));
      }
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
    assert(this_->builtinPropSize() == 0);
    TypedValue* propVec = (TypedValue *)((uintptr_t)this_ + sizeof(ObjectData));
    for (unsigned i = 0; i < nProps; ++i) {
      TypedValue* prop = &propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps))(this_);
  }

  //============================================================================
  // Virtual ObjectData methods that we need to override

 public:
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
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
    assert((sz & (sizeof(TypedValue) - 1)) == 0);
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

  friend class ObjectData;
};

inline Instance* instanceFromTv(TypedValue* tv) {
  assert(dynamic_cast<Instance*>(tv->m_data.pobj));
  return static_cast<Instance*>(tv->m_data.pobj);
}

} } // HPHP::VM

namespace HPHP {

class ExtObjectData : public HPHP::VM::Instance {
 public:
  explicit ExtObjectData(HPHP::VM::Class* cls)
    : HPHP::VM::Instance(cls, false) {
    assert(!m_cls->callsCustomInstanceInit());
  }
};

template <int flags> class ExtObjectDataFlags : public ExtObjectData {
 public:
  explicit ExtObjectDataFlags(HPHP::VM::Class* cb) : ExtObjectData(cb) {
    ObjectData::setAttributes(flags);
  }
};

} // HPHP

#endif
