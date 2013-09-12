/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_OBJECT_DATA_H_
#define incl_HPHP_OBJECT_DATA_H_

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/system/systemlib.h"
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/int.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayIter;
class MutableArrayIter;

class HphpArray;
struct TypedValue;
class PreClass;
class Class;

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps);

/**
 * Base class of all PHP objects and PHP resources.
 */
class ObjectData {
 public:
  enum Attribute {
    NoDestructor  = 0x0001, // __destruct()
    HasSleep      = 0x0002, // __sleep()
    UseSet        = 0x0004, // __set()
    UseGet        = 0x0008, // __get()
    UseIsset      = 0x0010, // __isset()
    UseUnset      = 0x0020, // __unset()
    HasCall       = 0x0080, // defines __call
    HasCallStatic = 0x0100, // defines __callStatic
    CallToImpl    = 0x0200, // call o_to{Boolean,Int64,Double}Impl
    HasClone      = 0x0400, // has custom clone logic
    // The top 3 bits of o_attributes are reserved to indicate the
    // type of collection
    CollectionTypeAttrMask = (7 << 13),
    VectorAttrInit = (Collection::VectorType << 13),
    MapAttrInit = (Collection::MapType << 13),
    StableMapAttrInit = (Collection::StableMapType << 13),
    SetAttrInit = (Collection::SetType << 13),
    PairAttrInit = (Collection::PairType << 13),
  };

  enum {
    RealPropCreate = 1,    // Property should be created if it doesn't exist
    RealPropUnchecked = 8, // Don't check property accessibility
    RealPropExist = 16,    // For property_exists
  };

 private:
  static DECLARE_THREAD_LOCAL_NO_CHECK(int, os_max_id);

 public:
  explicit ObjectData(Class* cls)
    : o_attribute(0)
    , m_count(0)
    , m_cls(cls) {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    o_id = ++(*os_max_id);
    instanceInit(cls);
  }

 private:
  enum class NoInit { noinit };
  explicit ObjectData(Class* cls, NoInit)
    : o_attribute(0)
    , m_count(0)
    , m_cls(cls) {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    o_id = ++(*os_max_id);
  }

  // Disallow copy construction
  ObjectData(const ObjectData&) = delete;

 public:
  void setStatic() const { assert(false); }
  bool isStatic() const { return false; }
  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

  virtual ~ObjectData();

  // Call newInstance() to instantiate a PHP object
  static ObjectData* newInstance(Class* cls) {
    if (auto const ctor = cls->instanceCtor()) {
      return ctor(cls);
    }
    Attr attrs = cls->attrs();
    if (UNLIKELY(attrs & (AttrAbstract | AttrInterface | AttrTrait))) {
      raiseAbstractClassError(cls);
    }
    size_t nProps = cls->numDeclProperties();
    size_t size = sizeForNProps(nProps);
    auto const obj = new (MM().objMalloc(size)) ObjectData(cls);
    if (UNLIKELY(cls->callsCustomInstanceInit())) {
      /*
        This must happen after the constructor finishes,
        because it can leak references to obj AND it can
        throw exceptions. If we have this in the ObjectData
        constructor, and it throws, obj will be partially
        destroyed (ie ~ObjectData will be called, resetting
        the vtable pointer) leaving dangling references
        to the object (eg in backtraces).
      */
      obj->callCustomInstanceInit();
    }
    return obj;
  }

  /*
   * Given a Class that is assumed to be a concrete, regular (not a
   * trait or interface), pure PHP class, and an allocation size,
   * return a new, uninitialized object of that class.
   *
   * newInstanceRaw should be called only when size <=
   * MemoryManager::kMaxSmartSize, otherwise use newInstanceRawBig.
   */
  static ObjectData* newInstanceRaw(Class* cls, uint32_t size);
  static ObjectData* newInstanceRawBig(Class* cls, size_t size);

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

 public:
  void operator delete(void* p);

  void release() {
    assert(getCount() == 0);
    destruct();
    if (UNLIKELY(getCount() != 0)) {
      // Object was resurrected.
      return;
    }
    delete this;
  }

  Class* getVMClass() const {
    return m_cls;
  }
  static size_t getVMClassOffset() {
    // For assembly linkage.
    return offsetof(ObjectData, m_cls);
  }
  static size_t attributeOff() { return offsetof(ObjectData, o_attribute); }
  bool instanceof(const Class* c) const;

  bool isCollection() const {
    return getCollectionType() != Collection::InvalidType;
  }
  Collection::Type getCollectionType() const {
    // Return the upper 3 bits of o_attribute
    return (Collection::Type)((uint16_t)(o_attribute >> 13) & 7);
  }

  bool implementsIterator() {
    return (instanceof(SystemLib::s_IteratorClass));
  }

  void setAttributes(int attrs) { o_attribute |= attrs; }
  void setAttributes(const ObjectData* o) { o_attribute |= o->o_attribute; }
  bool getAttribute(Attribute attr) const { return o_attribute & attr; }
  void setAttribute(Attribute attr) const { o_attribute |= attr;}
  void clearAttribute(Attribute attr) const { o_attribute &= ~attr;}
  bool noDestruct() const { return getAttribute(NoDestructor); }
  void setNoDestruct() { setAttribute(NoDestructor); }
  ObjectData* clearNoDestruct() { clearAttribute(NoDestructor); return this; }

  Object iterableObject(bool& isIterable, bool mayImplementIterator = true);
  ArrayIter begin(CStrRef context = null_string);
  MutableArrayIter begin(Variant* key, Variant& val,
                         CStrRef context = null_string);

  /**
   * o_instanceof() can be used for both classes and interfaces.
   */
  bool o_instanceof(CStrRef s) const;

  // class info
  CStrRef o_getClassName() const;
  int o_getId() const { return o_id;}

  bool o_toBoolean() const {
    if (UNLIKELY(getAttribute(CallToImpl))) {
      return o_toBooleanImpl();
    }
    return true;
  }

  int64_t o_toInt64() const {
    if (UNLIKELY(getAttribute(CallToImpl))) {
      return o_toInt64Impl();
    }
    raiseObjToIntNotice(o_getClassName().data());
    return 1;
  }

  double o_toDouble() const {
    if (UNLIKELY(getAttribute(CallToImpl))) {
      return o_toDoubleImpl();
    }
    return o_toInt64();
  }

  // overridable casting
  bool o_toBooleanImpl() const noexcept;
  int64_t o_toInt64Impl() const noexcept;
  double o_toDoubleImpl() const noexcept;
  Array o_toArray() const;

  void destruct();

  Array o_toIterArray(CStrRef context, bool getRef = false);

  Variant* o_realProp(CStrRef s, int flags,
                      CStrRef context = null_string) const;

  Variant o_get(CStrRef s, bool error = true,
                CStrRef context = null_string);

  Variant o_set(CStrRef s, CVarRef v);
  Variant o_set(CStrRef s, RefResult v);
  Variant o_setRef(CStrRef s, CVarRef v);

  Variant o_set(CStrRef s, CVarRef v, CStrRef context);
  Variant o_set(CStrRef s, RefResult v, CStrRef context);
  Variant o_setRef(CStrRef s, CVarRef v, CStrRef context);

  void o_setArray(CArrRef properties);
  void o_getArray(Array& props, bool pubOnly = false) const;

  static Object FromArray(ArrayData* properties);

  // TODO Task #2584896: o_invoke and o_invoke_few_args are deprecated. These
  // APIs don't properly take class context into account when looking up the
  // method, and they duplicate some of the functionality from invokeFunc(),
  // invokeFuncFew(), and vm_decode_function(). We should remove these APIs
  // and migrate all callers to use invokeFunc(), invokeFuncFew(), and
  // vm_decode_function() instead.
  Variant o_invoke(CStrRef s, CArrRef params, bool fatal = true);
  Variant o_invoke_few_args(CStrRef s, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);

  void serialize(VariableSerializer* serializer) const;
  void serializeImpl(VariableSerializer* serializer) const;
  bool hasInternalReference(PointerSet& vars, bool ds = false) const;
  void dump() const;
  ObjectData* clone();

  Variant offsetGet(Variant key);
  String invokeToString();
  bool hasToString();

  Variant invokeSleep();
  Variant invokeToDebugDisplay();
  Variant invokeWakeup();

  static int GetMaxId() ATTRIBUTE_COLD;

 public:
  CArrRef getDynProps() const { return o_properties; }
  void initProperties(int nProp);

  // heap profiling helpers
  void getChildren(std::vector<TypedValue*> &out) {
    // statically declared properties
    Slot nProps = m_cls->numDeclProperties();
    for (Slot i = 0; i < nProps; ++i) {
      out.push_back(&propVec()[i]);
    }

    // dynamic properties
    ArrayData* props = o_properties.get();
    if (props) {
      props->getChildren(out);
    }
  }

 protected:
  TypedValue* propVec();
  const TypedValue* propVec() const;

 public:
  ObjectData* callCustomInstanceInit();

  //============================================================================
  // Miscellaneous.

  void cloneSet(ObjectData* clone);
  ObjectData* cloneImpl();

  const Func* methodNamed(const StringData* sd) const {
    return getVMClass()->lookupMethod(sd);
  }

  static size_t sizeForNProps(Slot nProps) {
    size_t sz = sizeof(ObjectData) + (sizeof(TypedValue) * nProps);
    assert((sz & (sizeof(TypedValue) - 1)) == 0);
    return sz;
  }

  //============================================================================
  // Properties.
 public:
  int builtinPropSize() const { return m_cls->builtinPropSize(); }

 private:
  void initDynProps(int numDynamic = 0);
  Slot declPropInd(TypedValue* prop) const;

  inline Variant o_getImpl(CStrRef propName, int flags,
                           bool error = true, CStrRef context = null_string);
  template <typename T>
  inline Variant o_setImpl(CStrRef propName, T v, CStrRef context);
 public:
  TypedValue* getProp(Class* ctx, const StringData* key, bool& visible,
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

  void setProp(Class* ctx, const StringData* key, TypedValue* val,
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

  static void raiseObjToIntNotice(const char*);
  static void raiseAbstractClassError(Class* cls);
  void raiseUndefProp(const StringData* name);

 private:
  static void compileTimeAssertions() {
    static_assert(offsetof(ObjectData, m_count) == FAST_REFCOUNT_OFFSET, "");
  }

  friend struct MemoryProfile;

  //============================================================================
  // ObjectData fields

 private:
  // Various per-instance flags
  mutable int16_t o_attribute;
 protected:
  // 16 bits of memory that can be reused by subclasses
  union {
    uint16_t u16;
    uint8_t u8[2];
  } o_subclassData;
  // Counter to keep track of the number of references to this object
  // (i.e. the object's "refcount")
  mutable RefCount m_count;
  // Pointer to this object's class
  Class* m_cls;
  // Storage for dynamic properties
  ArrNR o_properties;
  // Numeric identifier of this object (used for var_dump())
  int o_id;

} __attribute__((aligned(16)));

template<> inline SmartPtr<ObjectData>::~SmartPtr() {}

typedef GlobalNameValueTableWrapper GlobalVariables;

inline
CountableHelper::CountableHelper(ObjectData* object) : m_object(object) {
  object->incRefCount();
}

inline
CountableHelper::~CountableHelper() {
  m_object->decRefCount();
}

///////////////////////////////////////////////////////////////////////////////
// Attribute helpers
class AttributeSetter {
public:
  AttributeSetter(ObjectData::Attribute a, ObjectData* o) : m_a(a), m_o(o) {
    o->setAttribute(a);
  }
  ~AttributeSetter() {
    m_o->clearAttribute(m_a);
  }
private:
  ObjectData::Attribute m_a;
  ObjectData* m_o;
};

class AttributeClearer {
public:
  AttributeClearer(ObjectData::Attribute a, ObjectData* o) : m_a(a), m_o(o) {
    o->clearAttribute(a);
  }
  ~AttributeClearer() {
    m_o->setAttribute(m_a);
  }
private:
  ObjectData::Attribute m_a;
  ObjectData* m_o;
};

ALWAYS_INLINE void decRefObj(ObjectData* obj) {
  if (obj->decRefCount() == 0) obj->release();
}

///////////////////////////////////////////////////////////////////////////////

inline ObjectData* instanceFromTv(TypedValue* tv) {
  assert(tv->m_type == KindOfObject);
  assert(dynamic_cast<ObjectData*>(tv->m_data.pobj));
  return tv->m_data.pobj;
}

class ExtObjectData : public ObjectData {
 public:
  explicit ExtObjectData(HPHP::Class* cls) : ObjectData(cls) {
    assert(!m_cls->callsCustomInstanceInit());
  }
};

template <int flags> class ExtObjectDataFlags : public ExtObjectData {
 public:
  explicit ExtObjectDataFlags(HPHP::Class* cb) : ExtObjectData(cb) {
    ObjectData::setAttributes(flags);
  }
};

} // HPHP

#endif
