/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/classname-is.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/weakref-data.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/util/low-ptr.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;

#define INVOKE_FEW_ARGS_COUNT 6
#define INVOKE_FEW_ARGS_DECL3                        \
  const Variant& a0 = uninit_variant,                \
  const Variant& a1 = uninit_variant,                \
  const Variant& a2 = uninit_variant
#define INVOKE_FEW_ARGS_DECL6                        \
  INVOKE_FEW_ARGS_DECL3,                             \
  const Variant& a3 = uninit_variant,                \
  const Variant& a4 = uninit_variant,                \
  const Variant& a5 = uninit_variant
#define INVOKE_FEW_ARGS_DECL10                       \
  INVOKE_FEW_ARGS_DECL6,                             \
  const Variant& a6 = uninit_variant,                \
  const Variant& a7 = uninit_variant,                \
  const Variant& a8 = uninit_variant,                \
  const Variant& a9 = uninit_variant
#define INVOKE_FEW_ARGS_HELPER(kind,num) kind##num
#define INVOKE_FEW_ARGS(kind,num) \
  INVOKE_FEW_ARGS_HELPER(INVOKE_FEW_ARGS_##kind,num)
#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS(DECL,INVOKE_FEW_ARGS_COUNT)

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps);

struct InvokeResult {
  TypedValue val;
  InvokeResult() {}
  InvokeResult(bool ok, TypedValue v) : val(v) {
    val.m_aux.u_ok = ok;
  }
  InvokeResult(bool ok, Variant&& v);
  bool ok() const { return val.m_aux.u_ok; }
  explicit operator bool() const { return ok(); }
};

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct ObjectData : Countable, type_scan::MarkCountable<ObjectData> {
  enum Attribute : uint16_t {
    NoDestructor  = 0x0001, // __destruct()
    HasSleep      = 0x0002, // __sleep()
    UseSet        = 0x0004, // __set()
    UseGet        = 0x0008, // __get()
    UseIsset      = 0x0010, // __isset()
    UseUnset      = 0x0020, // __unset()
    IsWaitHandle  = 0x0040, // This is a c_WaitHandle or derived
    HasClone      = 0x0100, // if IsCppBuiltin, has custom clone logic
                            // if not IsCppBuiltin, defines __clone PHP method
    CallToImpl    = 0x0200, // call o_to{Boolean,Int64,Double}Impl
    HasNativeData = 0x0400, // HNI Class with <<__NativeData("T")>>
    HasDynPropArr = 0x0800, // has a dynamic properties array
    IsCppBuiltin  = 0x1000, // has a custom instanceCtor and instanceDtor
                            // if you subclass ObjectData, you need this
    IsCollection  = 0x2000, // it's a collection (and the specific type is
                            // one of the CollectionType HeaderKind values
    HasPropEmpty  = 0x4000, // has custom propEmpty logic
    HasNativePropHandler    // class has native magic props handler
                  = 0x8000
  };

  enum {
    RealPropCreate = 1,    // Property should be created if it doesn't exist
    RealPropUnchecked = 8, // Don't check property accessibility
    RealPropExist = 16,    // For property_exists
  };

 private:
  static __thread uint32_t os_max_id;

 public:
  static void resetMaxId();

  explicit ObjectData(Class*, uint16_t flags = 0,
                      HeaderKind = HeaderKind::Object);
  ~ObjectData();

  // Disallow copy construction and assignemt
  ObjectData(const ObjectData&) = delete;
  ObjectData& operator=(const ObjectData&) = delete;

 protected:
  enum class NoInit {};

  explicit ObjectData(Class*, NoInit) noexcept;
  explicit ObjectData(Class* cls,
                      uint16_t flags,
                      HeaderKind kind,
                      NoInit) noexcept;

 public:
  ALWAYS_INLINE void decRefAndRelease() {
    assert(kindIsValid());
    if (decReleaseCheck()) release();
  }
  bool kindIsValid() const { return isObjectKind(headerKind()); }

  void scan(type_scan::Scanner&) const;

  size_t heapSize() const;

  // WeakRef control methods.
  inline void invalidateWeakRef() const {
    if (UNLIKELY(m_weak_refed)) {
      WeakRefData::invalidateWeakRef((uintptr_t)this);
    }
  }

  inline void setWeakRefed(bool flag) const {
    m_weak_refed = flag;
  }

 public:

  /*
   * Call newInstance() to instantiate a PHP object. The initial ref-count will
   * be greater than zero. Since this gives you a raw pointer, it is your
   * responsibility to manage the ref-count yourself. Whenever possible, prefer
   * using the Object class instead, which takes care of this for you.
   */
  static ObjectData* newInstance(Class*);

  /*
   * Instantiate a new object without initializing its declared properties. The
   * given Class must be a concrete, regular Class, without an instanceCtor or
   * customInit.
   */
  static ObjectData* newInstanceNoPropInit(Class*);

  /*
   * Given a Class that is assumed to be a concrete, regular (not a trait or
   * interface), pure PHP class, and an allocation size, return a new,
   * uninitialized object of that class. These are meant to be called from the
   * JIT.
   *
   * newInstanceRaw should be called only when size <= kMaxSmallSize,
   * otherwise use newInstanceRawBig.
   *
   * The initial ref-count will be set to one.
   */
  static ObjectData* newInstanceRaw(Class*, uint32_t);
  static ObjectData* newInstanceRawBig(Class*, size_t);

  void release() noexcept;
  void releaseNoObjDestructCheck() noexcept;

  Class* getVMClass() const;
  void setVMClass(Class* cls);
  StrNR getClassName() const;
  uint32_t getId() const;

  // instanceof() can be used for both classes and interfaces.
  bool instanceof(const String&) const;
  bool instanceof(const Class*) const;

  template <typename T>
  typename std::enable_if<
    std::is_same<ObjectData,T>::value,
    bool
  >::type instanceof() { return true; }

  template <typename T>
  typename std::enable_if<
    !std::is_same<ObjectData,T>::value,
    bool
  >::type instanceof() { return instanceof(T::classof()); }

  // Whether the object implements Iterator.
  bool isIterator() const;

  // Whether the object is a collection, [and [not] mutable].
  bool isCollection() const;
  bool isMutableCollection() const;
  bool isImmutableCollection() const;
  CollectionType collectionType() const; // asserts(isCollection())
  HeaderKind headerKind() const;

  bool getAttribute(Attribute) const;
  void setAttribute(Attribute);
  bool hasInstanceDtor() const;
  bool noDestruct() const;
  void setNoDestruct();
  void clearNoDestruct();

  Object iterableObject(bool& isIterable, bool mayImplementIterator = true);

  /*
   * Type conversions. Some subclasses of ObjectData have custom conversions.
   * (e.g. SimpleXMLElement -> bool)
   */
  bool toBoolean() const;
  int64_t toInt64() const;
  double toDouble() const;
  Array toArray(bool pubOnly = false) const;

  /*
   * Comparisons.
   *
   * Note that for objects !(X < Y) does *not* imply (X >= Y).
   */
  bool equal(const ObjectData&) const;
  bool less(const ObjectData&) const;
  bool more(const ObjectData&) const;
  int64_t compare(const ObjectData&) const;

  /*
   * Call this object's destructor, if it has one. No restrictions are placed
   * on the object's refcount, since this is used on objects still alive at
   * request shutdown.
   */
  void destructForExit();

 private:
  void instanceInit(Class*);
  bool destructImpl();
  Variant* realPropImpl(const String& s, int flags, const String& context,
                        bool copyDynArray);
 public:

  enum IterMode { EraseRefs, CreateRefs, PreserveRefs };
  /*
   * Create an array of object properties suitable for iteration.
   *
   * EraseRefs    - array should contain unboxed properties
   * CreateRefs   - array should contain boxed properties
   * PreserveRefs - reffiness of properties should be preserved in returned
   *                array
   */
  Array o_toIterArray(const String& context, IterMode mode);

  Variant* o_realProp(const String& s, int flags,
                      const String& context = null_string);
  const Variant* o_realProp(const String& s, int flags,
                            const String& context = null_string) const;

  Variant o_get(const String& s, bool error = true,
                const String& context = null_string);

  Variant o_set(const String& s, const Variant& v);
  Variant o_set(const String& s, const Variant& v, const String& context);

  void o_setArray(const Array& properties);
  void o_getArray(Array& props, bool pubOnly = false) const;

  static Object FromArray(ArrayData* properties);

  // TODO Task #2584896: o_invoke and o_invoke_few_args are deprecated. These
  // APIs don't properly take class context into account when looking up the
  // method, and they duplicate some of the functionality from invokeFunc(),
  // invokeFuncFew(), and vm_decode_function(). We should remove these APIs and
  // migrate all callers to use invokeFunc(), invokeFuncFew(), and
  // vm_decode_function() instead.
  Variant o_invoke(const String& s, const Variant& params, bool fatal = true);
  Variant o_invoke_few_args(const String& s, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);

  ObjectData* clone();

  Variant offsetGet(Variant key);
  String invokeToString();
  bool hasToString();

  Variant invokeSleep();
  Variant invokeToDebugDisplay();
  Variant invokeWakeup();

  /*
   * Returns whether this object has any dynamic properties.
   */
  bool hasDynProps() const;

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

  /*
   * Use the given array for this object's dynamic properties. HasDynPropArry
   * must not already be set. Returns a reference to the Array in its final
   * location.
   */
  Array& setDynPropArray(const Array&);

  // accessors for the declared properties area
  TypedValue* propVec();
  const TypedValue* propVec() const;

 public:
  const Func* methodNamed(const StringData*) const;
  static size_t sizeForNProps(Slot);

  //============================================================================
  // Properties.
 private:
  void initDynProps(int numDynamic = 0);
  Slot declPropInd(const TypedValue* prop) const;

  inline Variant o_getImpl(const String& propName, int flags, bool error = true,
                           const String& context = null_string);
  template <typename T>
  inline Variant o_setImpl(const String& propName, T v, const String& context);
 public:

  template <class T>
  struct PropLookup {
    T prop;
    bool accessible;
  };

  PropLookup<TypedValue*> getProp(const Class*, const StringData*);
  PropLookup<const TypedValue*> getProp(const Class*, const StringData*) const;

  PropLookup<TypedValue*> getPropImpl(const Class*, const StringData*,
                                      bool copyDynArray);

  struct PropAccessInfo {
    struct Hash;

    bool operator==(const PropAccessInfo& o) const {
      return obj == o.obj && attr == o.attr && key->same(o.key);
    }

    ObjectData* obj;
    const StringData* key;      // note: not necessarily static
    ObjectData::Attribute attr;
  };

  struct PropRecurInfo {
    using RecurSet = req::hash_set<PropAccessInfo, PropAccessInfo::Hash>;
    const PropAccessInfo* activePropInfo;
    RecurSet* activeSet;
  };

 private:
  template<MOpMode mode>
  TypedValue* propImpl(TypedValue* tvRef, const Class* ctx,
                       const StringData* key);

  bool propEmptyImpl(const Class* ctx, const StringData* key);

  bool invokeSet(const StringData* key, const TypedValue* val);
  InvokeResult invokeGet(const StringData* key);
  InvokeResult invokeIsset(const StringData* key);
  bool invokeUnset(const StringData* key);
  InvokeResult invokeNativeGetProp(const StringData* key);
  bool invokeNativeSetProp(const StringData* key, TypedValue* val);
  InvokeResult invokeNativeIssetProp(const StringData* key);
  bool invokeNativeUnsetProp(const StringData* key);

  void getProp(const Class* klass, bool pubOnly, const PreClass::Prop* prop,
               Array& props, std::vector<bool>& inserted) const;
  void getProps(const Class* klass, bool pubOnly, const PreClass* pc,
                Array& props, std::vector<bool>& inserted) const;
  void getTraitProps(const Class* klass, bool pubOnly, const Class* trait,
                     Array& props, std::vector<bool>& inserted) const;

 public:
  TypedValue* prop(
    TypedValue* tvRef,
    const Class* ctx,
    const StringData* key
  );

  TypedValue* propD(
    TypedValue* tvRef,
    const Class* ctx,
    const StringData* key
  );

  TypedValue* propW(
    TypedValue* tvRef,
    const Class* ctx,
    const StringData* key
  );

  bool propIsset(const Class* ctx, const StringData* key);
  bool propEmpty(const Class* ctx, const StringData* key);

  void setProp(Class* ctx, const StringData* key, TypedValue* val,
               bool bindingAssignment = false);
  TypedValue* setOpProp(TypedValue& tvRef, Class* ctx, SetOpOp op,
                        const StringData* key, Cell* val);

  Cell incDecProp(Class* ctx, IncDecOp op, const StringData* key);

  void unsetProp(Class* ctx, const StringData* key);

  static void raiseObjToIntNotice(const char*);
  static void raiseObjToDoubleNotice(const char*);
  static void raiseAbstractClassError(Class*);
  void raiseUndefProp(const StringData*);

  static constexpr ptrdiff_t getVMClassOffset() {
    return offsetof(ObjectData, m_cls);
  }
  static constexpr ptrdiff_t attributeOff() {
    return offsetof(ObjectData, m_aux16);
  }
  const char* classname_cstr() const;

private:
  friend struct MemoryProfile;

  bool toBooleanImpl() const noexcept;
  int64_t toInt64Impl() const noexcept;
  double toDoubleImpl() const noexcept;

// offset:  0        8       12   16   20          32
// 64bit:   header   cls          id   [subclass]  [props...]
// lowptr:  header   cls     id   [subclass][props...]

private:
  LowPtr<Class> m_cls;
  uint32_t o_id; // id of this object (used for var_dump(), and WeakRefs)
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct CountableHelper {
  explicit CountableHelper(ObjectData* object) : m_object(object) {
    object->incRefCount();
  }
  ~CountableHelper() {
    m_object->decRefCount();
  }

  CountableHelper(const CountableHelper&) = delete;
  CountableHelper& operator=(const CountableHelper&) = delete;

private:
  ObjectData *m_object;
};

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void decRefObj(ObjectData* obj) {
  obj->decRefAndRelease();
}

/*
 * Write a value to `to', with Dup semantics.
 *
 * @see: tv-mutate.h
 */
ALWAYS_INLINE void tvWriteObject(ObjectData* pobj, TypedValue* to) {
  to->m_type = KindOfObject;
  to->m_data.pobj = pobj;
  to->m_data.pobj->incRefCount();
}

inline ObjectData* instanceFromTv(TypedValue* tv) {
  assert(tv->m_type == KindOfObject);
  assert(dynamic_cast<ObjectData*>(tv->m_data.pobj));
  return tv->m_data.pobj;
}

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_CLASS_NO_SWEEP(originalName)                           \
  public:                                                              \
  CLASSNAME_IS(#originalName)                                          \
  friend ObjectData* new_##originalName##_Instance(Class*);            \
  friend void delete_##originalName(ObjectData*, const Class*);        \
  static HPHP::LowPtr<Class> s_classOf;                                \
  static inline HPHP::LowPtr<Class>& classof() {                       \
    return s_classOf;                                                  \
  }

#define IMPLEMENT_CLASS_NO_SWEEP(cls)                                  \
  HPHP::LowPtr<Class> c_##cls::s_classOf;

namespace req {

template<class T, class... Args>
typename std::enable_if<
  std::is_convertible<T*, ObjectData*>::value,
  req::ptr<T>
>::type make(Args&&... args) {
  auto const mem = MM().mallocSmallSize(sizeof(T));
  (void)type_scan::getIndexForMalloc<T>(); // ensure T* ptrs are interesting
  try {
    auto t = new (mem) T(std::forward<Args>(args)...);
    assert(t->hasExactlyOneRef());
    return req::ptr<T>::attach(t);
  } catch (...) {
    MM().freeSmallSize(mem, sizeof(T));
    throw;
  }
}
} // namespace req

///////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////

#define incl_HPHP_OBJECT_DATA_INL_H_
#include "hphp/runtime/base/object-data-inl.h"
#undef incl_HPHP_OBJECT_DATA_INL_H_

///////////////////////////////////////////////////////////////////////////////

#endif
