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

#include "hphp/runtime/base/classname-is.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/weakref-data.h"
#include "hphp/runtime/base/rds-local.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/util/low-ptr.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct MInstrPropState;
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

namespace Native {
  ObjectData* nativeDataInstanceCopyCtor(ObjectData *src, Class* cls,
                                         size_t nProps);
}

// A slot to store memoization data in. This can be either a Cell storing a
// single value, or a pointer to a memoization cache.
struct MemoSlot {
public:
  /*
   * We use the type field of the Cell to determine whether this is a single
   * value or a memo cache. If the type is kInvalidDataType (which cannot occur
   * for a valid Cell), its a memo cache. As a special case, if type is Uninit,
   * and the pointer is null, it can also be a cache (its also a value). This
   * lets us initialize the slots with zero regardless of how it will be
   * used. When its actually used, the correct type will be filed in. The
   * ambiguity isn't an issue because these predicates are just for assertions
   * (the type of the slot is implied by the function).
   */

  bool isCache() const {
    return value.m_type == kInvalidDataType ||
      (value.m_type == KindOfUninit && value.m_data.pcache == nullptr);
  }
  bool isValue() const { return value.m_type != kInvalidDataType; }

  // Get a reference to the pointer to the cache, for the purpose of a set on
  // the cache. Since we're going to be creating a cache in this slot, change
  // its type to indicate this.
  MemoCacheBase*& getCacheForWrite() {
    assertx(isCache());
    value.m_type = kInvalidDataType;
    return value.m_data.pcache;
  }

  MemoCacheBase* getCache() {
    assertx(isCache());
    return value.m_data.pcache;
  }
  const MemoCacheBase* getCache() const {
    assertx(isCache());
    return value.m_data.pcache;
  }

  Cell* getValue() {
    assertx(isValue());
    assertx(cellIsPlausible(value));
    return &value;
  }
  const Cell* getValue() const {
    assertx(isValue());
    assertx(cellIsPlausible(value));
    return &value;
  }

  // Used when we've freed the cache manually
  void resetCache() {
    assertx(isCache());
    value.m_data.pcache = nullptr;
  }
private:
  TYPE_SCAN_CUSTOM() {
    isCache() ? scanner.scan(value.m_data.pcache) : scanner.scan(value);
  }

  Cell value;
};
static_assert(sizeof(MemoSlot) == sizeof(Cell), "");

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

struct ObjectData : Countable, type_scan::MarkCollectable<ObjectData> {
  enum Attribute : uint8_t {
    NoAttrs            = 0x00,
    IsWeakRefed        = 0x02, // Is pointed to by at least one WeakRef
    HasDynPropArr      = 0x04, // has a dynamic properties array
    IsBeingConstructed = 0x08, // Constructor for most derived class has not
                               // finished. Only set during construction when
                               // the class has immutable properties (to
                               // temporarily allow writing to them).
    UsedMemoCache      = 0x10, // Object has had data set in its memo slots
    HasUninitProps     = 0x20  // The object's properties are being initialized
  };

  static constexpr size_t offsetofAttrs() {
    return offsetof(ObjectData, m_aux16);
  }

  static constexpr size_t sizeofAttrs() {
    return sizeof(m_aux16);
  }

  explicit ObjectData(Class*, uint8_t flags = 0,
                      HeaderKind = HeaderKind::Object);
  ~ObjectData();

  // Disallow copy construction and assignemt
  ObjectData(const ObjectData&) = delete;
  ObjectData& operator=(const ObjectData&) = delete;

 protected:
  enum class NoInit {};
  enum class InitRaw {};

  // for JIT-generated instantiation with inlined property init
  explicit ObjectData(Class* cls, InitRaw, uint8_t flags = 0,
                      HeaderKind = HeaderKind::Object) noexcept;

  // for C++ subclasses with no declared properties
  explicit ObjectData(Class* cls, NoInit, uint8_t flags = 0,
                      HeaderKind = HeaderKind::Object) noexcept;

 public:
  ALWAYS_INLINE void decRefAndRelease() {
    assertx(kindIsValid());
    if (decReleaseCheck()) release();
  }
  bool kindIsValid() const { return isObjectKind(headerKind()); }

  void scan(type_scan::Scanner&) const;

  size_t heapSize() const;

  void setWeakRefed() { setAttribute(IsWeakRefed); }

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
   * JIT, where the cls, size, and attributes are constants at JIT time.
   *
   * The big=true versions should be called when size > kMaxSmallSize.
   *
   * The memo versions should be used if the object has memo slots.
   *
   * The initial ref-count will be set to one.
   */
  static const uint8_t DefaultAttrs = NoAttrs;

  static ObjectData* newInstanceRawSmall(Class*, size_t size, size_t index);
  static ObjectData* newInstanceRawBig(Class*, size_t size);

  static ObjectData* newInstanceRawMemoSmall(Class*, size_t size,
                                             size_t index, size_t objoff);
  static ObjectData* newInstanceRawMemoBig(Class*, size_t size, size_t objoff);

  void release() noexcept;

  Class* getVMClass() const;
  void setVMClass(Class* cls);
  StrNR getClassName() const;

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

  // Has a custom instanceCtor and instanceDtor. If you subclass ObjectData
  // in C++, you need this.
  bool isCppBuiltin() const;

  // Is this an object with (some) immutable properties for which construction
  // has not finished yet?
  bool isBeingConstructed() const;

  // Set if we might re-enter while some of the properties contain
  // garbage, eg after calling newInstanceNoPropInit, and before
  // initializing all the props.
  bool hasUninitProps() const;
  void setHasUninitProps();
  void clearHasUninitProps();

  // Whether the object is a collection, [and [not] mutable].
  bool isCollection() const;
  bool isMutableCollection() const;
  bool isImmutableCollection() const;
  CollectionType collectionType() const; // asserts(isCollection())
  HeaderKind headerKind() const;

  // True if this is a c_Awaitable or derived
  bool isWaitHandle() const;

  bool getAttribute(Attribute) const;
  void setAttribute(Attribute);
  bool hasInstanceDtor() const;
  bool hasNativeData() const;

  Object iterableObject(bool& isIterable, bool mayImplementIterator = true);

  /*
   * Type conversions. Some subclasses of ObjectData have custom conversions.
   * (e.g. SimpleXMLElement -> bool)
   */
  bool toBoolean() const;
  int64_t toInt64() const;
  double toDouble() const;

  template <IntishCast IC = IntishCast::None>
  Array toArray(bool pubOnly = false, bool ignoreLateInit = false) const;

  /*
   * Comparisons.
   *
   * Note that for objects !(X < Y) does *not* imply (X >= Y).
   */
  bool equal(const ObjectData&) const;
  bool less(const ObjectData&) const;
  bool lessEqual(const ObjectData&) const;
  bool more(const ObjectData&) const;
  bool moreEqual(const ObjectData&) const;
  int64_t compare(const ObjectData&) const;

 private:
  void instanceInit(Class*);

 public:

  enum IterMode { EraseRefs, PreserveRefs };
  /*
   * Create an array of object properties suitable for iteration.
   *
   * EraseRefs    - array should contain unboxed properties
   * PreserveRefs - reffiness of properties should be preserved in returned
   *                array
   */
  Array o_toIterArray(const String& context, IterMode mode);

  Variant o_get(const String& s, bool error = true,
                const String& context = null_string);

  void o_set(const String& s, const Variant& v,
             const String& context = null_string);

  void o_setArray(const Array& properties);
  void o_getArray(Array& props,
                  bool pubOnly = false,
                  bool ignoreLateInit = false) const;

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

  String invokeToString();
  bool hasToString();

  Variant invokeSleep();
  Variant invokeToDebugDisplay();
  Variant invokeWakeup();
  Variant invokeDebugInfo();

  Variant static InvokeSimple(ObjectData* data, const StaticString& name);

  /*
   * Returns whether this object has any dynamic properties.
   */
  bool hasDynProps() const;

  /*
   * Returns a reference to dynamic properties Array for this object.
   * The reference points into an entry in ExecutionContext::dynPropArray,
   * so is only valid for a short lifetime, until another entry is inserted
   * or erased (anything that moves entries).
   *
   * Note: you're generally not going to want to copy-construct the
   * return value of this function.  If you want to make changes to
   * the property array, we need to keep its ref count at 1.
   *
   * Pre: getAttribute(HasDynPropArr)
   */
  Array& dynPropArray() const;

  /*
   * Use the given array for this object's dynamic properties. HasDynPropArry
   * must not already be set. Returns a reference to the Array in its final
   * location.
   */
  void setDynProps(const Array&);
  void reserveDynProps(int nProp);

  // Accessors for the declared properties area. Note that if the caller writes
  // to these properties, they are responsible for validating the values with
  // any type-hints on the properties. Likewise the caller is responsible for
  // enforcing AttrLateInit.
  TypedValue* propVecForWrite();
  TypedValue* propVecForConstruct();
  const TypedValue* propVec() const;

  void verifyPropTypeHints() const;
  void verifyPropTypeHints(size_t end) const;
  void verifyPropTypeHint(Slot slot) const;

  bool assertPropTypeHints() const;

  // Accessors for declared properties at statically known offsets. In the lval
  // case, the property must be statically known to be mutable. If the caller
  // modifies the lval, they are responsible for validating the value with any
  // type-hint on that property. Likewise the caller is responsible for
  // enforcing AttrLateInit.
  tv_lval propLvalAtOffset(Slot);
  tv_rval propRvalAtOffset(Slot) const;

  // Get a pointer to the i-th memo slot. The object must not have native data.
  MemoSlot* memoSlot(Slot);
  const MemoSlot* memoSlot(Slot) const;

  // Get a pointer to the i-th memo slot. Use these if the object has native
  // data. The second parameter is the size of the object's native data.
  MemoSlot* memoSlotNativeData(Slot, size_t);
  const MemoSlot* memoSlotNativeData(Slot, size_t) const;

 public:
  const Func* methodNamed(const StringData*) const;
  static size_t sizeForNProps(Slot);

  static size_t objOffFromMemoNode(const Class*);

  //============================================================================
  // Properties.
 private:
  /*
   * Use the given array for this object's dynamic properties. HasDynPropArry
   * must not already be set. Returns a reference to the Array in its final
   * location.
   */
  Array& setDynPropArray(const Array&);

  /*
   * Create the dynamic property array for this ObjectData if it
   * doesn't already exist yet.
   *
   * Post: getAttribute(HasDynPropArr)
   */
  Array& reserveProperties(int nProp = 2);

  [[noreturn]] NEVER_INLINE
  void throwMutateImmutable(Slot prop) const;
  [[noreturn]] NEVER_INLINE
  void throwBindImmutable(Slot prop) const;

 public:
  // never box the lval returned from getPropLval; use propB instead
  tv_lval getPropLval(const Class*, const StringData*);
  tv_rval getProp(const Class*, const StringData*) const;
  // Like getProp() but does not throw for <<__LateInit>>. Value can be
  // KindOfUninit.
  tv_rval getPropIgnoreLateInit(const Class* ctx,
                                const StringData* key) const;
  // don't use vGetPropIgnoreAccessibility in new code
  tv_lval vGetPropIgnoreAccessibility(const StringData*);

 private:
  struct PropLookup {
    tv_lval val;
    const Class::Prop* prop;
    Slot slot;
    bool accessible;
    bool immutable;
  };

  template <bool forWrite, bool forRead, bool ignoreLateInit>
  ALWAYS_INLINE
  PropLookup getPropImpl(const Class*, const StringData*);

  enum class PropMode : int {
    ReadNoWarn,
    ReadWarn,
    DimForWrite,
    Bind,
  };

  template<PropMode mode>
  tv_lval propImpl(TypedValue* tvRef, const Class* ctx,
                   const StringData* key, MInstrPropState* pState);

  bool propEmptyImpl(const Class* ctx, const StringData* key);

  void setDynProp(const StringData* key, Cell val);

  bool invokeSet(const StringData* key, Cell val);
  InvokeResult invokeGet(const StringData* key);
  InvokeResult invokeIsset(const StringData* key);
  bool invokeUnset(const StringData* key);
  InvokeResult invokeNativeGetProp(const StringData* key);
  bool invokeNativeSetProp(const StringData* key, Cell val);
  InvokeResult invokeNativeIssetProp(const StringData* key);
  bool invokeNativeUnsetProp(const StringData* key);

 public:
  tv_lval prop(TypedValue* tvRef, const Class* ctx, const StringData* key);
  tv_lval propW(TypedValue* tvRef, const Class* ctx, const StringData* key);
  tv_lval propU(TypedValue* tvRef, const Class* ctx, const StringData* key);
  tv_lval propD(TypedValue* tvRef, const Class* ctx,
                const StringData* key, MInstrPropState* pState);
  tv_lval propB(TypedValue* tvRef, const Class* ctx,
                const StringData* key, MInstrPropState* pState);

  bool propIsset(const Class* ctx, const StringData* key);
  bool propEmpty(const Class* ctx, const StringData* key);

  void setProp(Class* ctx, const StringData* key, Cell val);
  tv_lval setOpProp(TypedValue& tvRef, Class* ctx, SetOpOp op,
                    const StringData* key, Cell* val);

  Cell incDecProp(Class* ctx, IncDecOp op, const StringData* key);

  void unsetProp(Class* ctx, const StringData* key);

  tv_lval makeDynProp(const StringData* key);

  static void raiseObjToIntNotice(const char*);
  static void raiseObjToDoubleNotice(const char*);
  static void raiseAbstractClassError(Class*);
  void raiseUndefProp(const StringData*) const;
  void raiseCreateDynamicProp(const StringData*) const;
  void raiseReadDynamicProp(const StringData*) const;
  void raiseImplicitInvokeToString() const;

  static constexpr ptrdiff_t getVMClassOffset() {
    return offsetof(ObjectData, m_cls);
  }
  const char* classname_cstr() const;

private:
  friend struct MemoryProfile;
  friend ObjectData* Native::nativeDataInstanceCopyCtor(
    ObjectData* src, Class* cls, size_t nProps);

  bool toBooleanImpl() const noexcept;
  int64_t toInt64Impl() const noexcept;
  double toDoubleImpl() const noexcept;

  bool slowDestroyCheck() const;

  bool assertTypeHint(tv_rval, Slot) const;

  void verifyPropTypeHintImpl(tv_rval, const Class::Prop&) const;

// offset:  0       8       12      16
// 64bit:   header  cls             [subclass][props...]
// lowptr:  header  cls     [subclass][props...]

private:
  LowPtr<Class> m_cls;
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
static_assert(sizeof(ObjectData) == (use_lowptr ? 12 : 16),
              "Change this only on purpose");
#else
static_assert(sizeof(ObjectData) == 16, "Change this only on purpose");
#endif
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
  auto const mem = tl_heap->objMalloc(sizeof(T));
  (void)type_scan::getIndexForMalloc<T>(); // ensure T* ptrs are interesting
  try {
    auto t = new (mem) T(std::forward<Args>(args)...);
    assertx(t->hasExactlyOneRef());
    return req::ptr<T>::attach(t);
  } catch (...) {
    tl_heap->objFree(mem, sizeof(T));
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
