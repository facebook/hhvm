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
#include "hphp/runtime/vm/hhbc.h"
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

class ObjectData {
 public:
  enum Attribute {
    NoDestructor  = 0x0001, // __destruct()
    HasSleep      = 0x0002, // __sleep()
    UseSet        = 0x0004, // __set()
    UseGet        = 0x0008, // __get()
    UseIsset      = 0x0010, // __isset()
    UseUnset      = 0x0020, // __unset()
    IsWaitHandle  = 0x0040, // This is a c_WaitHandle or derived
    HasCall       = 0x0080, // defines __call
    HasClone      = 0x0100, // if IsCppBuiltin, has custom clone logic
                            // if not IsCppBuiltin, defines __clone PHP method
    CallToImpl    = 0x0200, // call o_to{Boolean,Int64,Double}Impl
    // FreeFlag      = 0x0400, // ** a free flag **
    HasDynPropArr = 0x0800, // has a dynamic properties array
    IsCppBuiltin  = 0x1000, // has custom C++ subclass
    IsCollection  = 0x2000, // it's a collection (and the specific type is
                            // stored in o_subclassData.u16)
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
    : m_cls(cls)
    , o_attribute(0)
    , m_count(0)
  {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    o_id = ++(*os_max_id);
    instanceInit(cls);
  }

 protected:
  explicit ObjectData(Class* cls, uint16_t flags)
    : m_cls(cls)
    , o_attribute(flags)
    , m_count(0)
  {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    o_id = ++(*os_max_id);
    instanceInit(cls);
  }

 private:
  enum class NoInit { noinit };
  explicit ObjectData(Class* cls, NoInit)
    : m_cls(cls)
    , o_attribute(0)
    , m_count(0)
  {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    o_id = ++(*os_max_id);
  }

  // Disallow copy construction and assignemt
  ObjectData(const ObjectData&) = delete;
  ObjectData& operator=(const ObjectData&) = delete;

 public:
  void setStatic() const { assert(false); }
  bool isStatic() const { return false; }
  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

 protected:
  ~ObjectData();
 public:

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
    auto const obj = new (MM().objMallocLogged(size)) ObjectData(cls);
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
   * newInstanceRaw should be called only when size <= kMaxSmartSize,
   * otherwise use newInstanceRawBig.
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
  static void DeleteObject(ObjectData* p);

  void release() {
    assert(!hasMultipleRefs());
    if (LIKELY(destruct())) DeleteObject(this);
  }

  Class* getVMClass() const {
    return m_cls;
  }
  bool instanceof(const Class* c) const {
    return m_cls->classof(c);
  }

  bool isCollection() const {
    return getAttribute(Attribute::IsCollection);
  }

  Collection::Type getCollectionType() const {
    return isCollection() ? static_cast<Collection::Type>(o_subclassData.u16)
                          : Collection::Type::InvalidType;
  }

  size_t getCollectionSize() const {
    return *(uint*)((char*)this + FAST_COLLECTION_SIZE_OFFSET);
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
  ArrayIter begin(const String& context = null_string);
  MutableArrayIter begin(Variant* key, Variant& val,
                         const String& context = null_string);

  /**
   * o_instanceof() can be used for both classes and interfaces.
   */
  bool o_instanceof(const String& s) const;

  // class info
  const String& o_getClassName() const;
  int o_getId() const { return o_id;}

  bool o_toBoolean() const {
    if (UNLIKELY(getAttribute(CallToImpl))) {
      return o_toBooleanImpl();
    }
    return true;
  }

  int64_t o_toInt64() const {
    if (UNLIKELY(getAttribute(CallToImpl) && !isCollection())) {
      return o_toInt64Impl();
    }
    raiseObjToIntNotice(o_getClassName().data());
    return 1;
  }

  double o_toDouble() const {
    if (UNLIKELY(getAttribute(CallToImpl) && !isCollection())) {
      return o_toDoubleImpl();
    }
    return o_toInt64();
  }

  // overridable casting
  bool o_toBooleanImpl() const noexcept;
  int64_t o_toInt64Impl() const noexcept;
  double o_toDoubleImpl() const noexcept;
  Array o_toArray() const;

  bool destruct();

  Array o_toIterArray(const String& context, bool getRef = false);

  Variant* o_realProp(const String& s, int flags,
                      const String& context = null_string);

  Variant o_get(const String& s, bool error = true,
                const String& context = null_string);

  Variant o_set(const String& s, CVarRef v);
  Variant o_set(const String& s, RefResult v);
  Variant o_setRef(const String& s, CVarRef v);

  Variant o_set(const String& s, CVarRef v, const String& context);
  Variant o_set(const String& s, RefResult v, const String& context);
  Variant o_setRef(const String& s, CVarRef v, const String& context);

  void o_setArray(CArrRef properties);
  void o_getArray(Array& props, bool pubOnly = false) const;

  static Object FromArray(ArrayData* properties);

  // TODO Task #2584896: o_invoke and o_invoke_few_args are deprecated. These
  // APIs don't properly take class context into account when looking up the
  // method, and they duplicate some of the functionality from invokeFunc(),
  // invokeFuncFew(), and vm_decode_function(). We should remove these APIs
  // and migrate all callers to use invokeFunc(), invokeFuncFew(), and
  // vm_decode_function() instead.
  Variant o_invoke(const String& s, CVarRef params, bool fatal = true);
  Variant o_invoke_few_args(const String& s, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);

  void serialize(VariableSerializer* serializer) const;
  void serializeImpl(VariableSerializer* serializer) const;
  void dump() const;
  ObjectData* clone();

  Variant offsetGet(Variant key);
  String invokeToString();
  bool hasToString();

  Variant invokeSleep();
  Variant invokeToDebugDisplay();
  Variant invokeWakeup();

  static int GetMaxId();

  /**
   * Used by the ext_zend_compat layer.
   * Identical to o_get but the output is boxed.
   */
  RefData* zGetProp(Class* ctx, const StringData* key,
                    bool& visible, bool& accessible,
                    bool& unset);

  /*
   * Returns whether this object has any dynamic properties.
   */
  bool hasDynProps() const {
    return getAttribute(HasDynPropArr) ? dynPropArray().size() : false;
  }

  /*
   * Returns the dynamic properties array for this object.
   *
   * Note: you're generally not going to want to copy-construct the
   * return value of this function.  If you want to make changes to
   * the property array, we need to keep its ref count at 1.
   *
   * Pre: getAttribute(HasDynPropArr)
   */
  Array& dynPropArray() const;

  /*
   * Create the dynamic property array for this ObjectData if it
   * doesn't already exist yet.
   *
   * Post: getAttribute(HasDynPropArr)
   */
  Array& reserveProperties(int nProp = 2);

  // heap profiling helpers
  void getChildren(std::vector<TypedValue*> &out) {
    Slot nProps = m_cls->numDeclProperties();
    for (Slot i = 0; i < nProps; ++i) {
      out.push_back(&propVec()[i]);
    }
    if (UNLIKELY(getAttribute(HasDynPropArr))) {
      dynPropArray()->getChildren(out);
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
 private:
  void initDynProps(int numDynamic = 0);
  Slot declPropInd(TypedValue* prop) const;

  inline Variant o_getImpl(const String& propName, int flags, bool error = true,
                           const String& context = null_string);
  template <typename T>
  inline Variant o_setImpl(const String& propName, T v, const String& context);
 public:
  TypedValue* getProp(Class* ctx, const StringData* key, bool& visible,
                      bool& accessible, bool& unset);
  const TypedValue* getProp(Class* ctx, const StringData* key, bool& visible,
                            bool& accessible, bool& unset) const;
 private:
  template <bool warn, bool define>
  void propImpl(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
                const StringData* key);
  bool invokeSet(TypedValue* retval, const StringData* key, TypedValue* val);
  bool invokeGet(TypedValue* retval, const StringData* key);
  bool invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                     const StringData* key);
  bool invokeIsset(TypedValue* retval, const StringData* key);
  bool invokeUnset(TypedValue* retval, const StringData* key);
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
  TypedValue* setOpProp(TypedValue& tvRef, Class* ctx, SetOpOp op,
                        const StringData* key, Cell* val);
  template <bool setResult>
  void incDecProp(TypedValue& tvRef, Class* ctx, IncDecOp op,
                  const StringData* key, TypedValue& dest);
  void unsetProp(Class* ctx, const StringData* key);

  static void raiseObjToIntNotice(const char*);
  static void raiseAbstractClassError(Class* cls);
  void raiseUndefProp(const StringData* name);

  static ptrdiff_t getVMClassOffset() { return offsetof(ObjectData, m_cls); }
  static ptrdiff_t attributeOff() { return offsetof(ObjectData, o_attribute); }
  static ptrdiff_t whStateOffset() {
    return offsetof(ObjectData, o_subclassData.u8[0]);
  }

private:
  friend struct MemoryProfile;
  static void compileTimeAssertions() {
    static_assert(offsetof(ObjectData, m_count) == FAST_REFCOUNT_OFFSET, "");
  }

private:
  Class* m_cls;
  mutable uint16_t o_attribute;

  // 16 bits of memory that can be reused by subclasses
protected:
  union {
    uint16_t u16;
    uint8_t u8[2];
  } o_subclassData;

private:
  mutable RefCount m_count;

  // Numeric identifier of this object (used for var_dump())
  int o_id;
} __attribute__((aligned(16)));

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

ALWAYS_INLINE void decRefObj(ObjectData* obj) {
  obj->decRefAndRelease();
}

inline ObjectData* instanceFromTv(TypedValue* tv) {
  assert(tv->m_type == KindOfObject);
  assert(dynamic_cast<ObjectData*>(tv->m_data.pobj));
  return tv->m_data.pobj;
}

///////////////////////////////////////////////////////////////////////////////

template<uint16_t Flags>
struct ExtObjectDataFlags : ObjectData {
  explicit ExtObjectDataFlags(HPHP::Class* cb)
    : ObjectData(cb, Flags | ObjectData::IsCppBuiltin)
  {
    assert(!getVMClass()->callsCustomInstanceInit());
  }

protected:
  ~ExtObjectDataFlags() {}
};

using ExtObjectData = ExtObjectDataFlags<ObjectData::IsCppBuiltin>;

///////////////////////////////////////////////////////////////////////////////

}

#endif
