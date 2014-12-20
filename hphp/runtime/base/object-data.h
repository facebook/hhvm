/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/classname-is.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/util/low-ptr.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;

#define INVOKE_FEW_ARGS_COUNT 6
#define INVOKE_FEW_ARGS_DECL3                        \
  const Variant& a0 = null_variant,                  \
  const Variant& a1 = null_variant,                  \
  const Variant& a2 = null_variant
#define INVOKE_FEW_ARGS_DECL6                        \
  INVOKE_FEW_ARGS_DECL3,                             \
  const Variant& a3 = null_variant,                  \
  const Variant& a4 = null_variant,                  \
  const Variant& a5 = null_variant
#define INVOKE_FEW_ARGS_DECL10                       \
  INVOKE_FEW_ARGS_DECL6,                             \
  const Variant& a6 = null_variant,                  \
  const Variant& a7 = null_variant,                  \
  const Variant& a8 = null_variant,                  \
  const Variant& a9 = null_variant
#define INVOKE_FEW_ARGS_HELPER(kind,num) kind##num
#define INVOKE_FEW_ARGS(kind,num) \
  INVOKE_FEW_ARGS_HELPER(INVOKE_FEW_ARGS_##kind,num)
#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS(DECL,INVOKE_FEW_ARGS_COUNT)

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps);

struct ObjectData {
  enum Attribute : uint16_t {
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
    HasNativeData = 0x0400, // HNI Class with <<__NativeData("T")>>
    HasDynPropArr = 0x0800, // has a dynamic properties array
    IsCppBuiltin  = 0x1000, // has custom C++ subclass
    IsCollection  = 0x2000, // it's a collection (and the specific type is
                            // stored in o_subclass_u8)
    HasPropEmpty  = 0x4000, // has custom propEmpty logic
    HasNativePropHandler    // class has native magic props handler
                  = 0x8000,
    InstanceDtor  = 0x1400, // HasNativeData | IsCppBuiltin
  };

  enum {
    RealPropCreate = 1,    // Property should be created if it doesn't exist
    RealPropUnchecked = 8, // Don't check property accessibility
    RealPropExist = 16,    // For property_exists
  };

  static const StaticString s_serializedNativeDataKey;

 private:
  static __thread int os_max_id;

 public:
  static void resetMaxId();

  explicit ObjectData(Class*);
  ~ObjectData();

  // Disallow copy construction and assignemt
  ObjectData(const ObjectData&) = delete;
  ObjectData& operator=(const ObjectData&) = delete;

 protected:
  explicit ObjectData(Class*, uint16_t flags, HeaderKind = HeaderKind::Object);

 private:
  enum class NoInit {};

  explicit ObjectData(Class*, NoInit);

 public:
  void setStatic() const;
  bool isStatic() const;
  void setUncounted() const;
  bool isUncounted() const;

  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

  size_t heapSize() const;

 public:

  // Call newInstance() to instantiate a PHP object
  static ObjectData* newInstance(Class*);

  /*
   * Given a Class that is assumed to be a concrete, regular (not a
   * trait or interface), pure PHP class, and an allocation size,
   * return a new, uninitialized object of that class.
   *
   * newInstanceRaw should be called only when size <= kMaxSmartSize,
   * otherwise use newInstanceRawBig.
   */
  static ObjectData* newInstanceRaw(Class*, uint32_t);
  static ObjectData* newInstanceRawBig(Class*, size_t);

 private:
  void instanceInit(Class*);

 public:
  static void DeleteObject(ObjectData*);

  void release();

  Class* getVMClass() const;
  StrNR getClassName() const;
  int getId() const;

  // instanceof() can be used for both classes and interfaces.
  bool instanceof(const String&) const;
  bool instanceof(const Class*) const;

  // Whether the object implements Iterator.
  bool isIterator() const;

  // Whether the object is a collection.
  bool isCollection() const;
  bool isMutableCollection() const;
  bool isImmutableCollection() const;

  Collection::Type getCollectionType() const;

  bool getAttribute(Attribute) const;
  void setAttribute(Attribute) const;

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

  bool destruct();

  Array o_toIterArray(const String& context, bool getRef = false);

  Variant* o_realProp(const String& s, int flags,
                      const String& context = null_string);

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

  void serialize(VariableSerializer*) const;
  void serializeImpl(VariableSerializer*) const;
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

 protected:
  TypedValue* propVec();
  const TypedValue* propVec() const;

  uint8_t& subclass_u8();
  uint8_t subclass_u8() const;

 public:
  ObjectData* callCustomInstanceInit();

  //============================================================================
  // Miscellaneous.

  void cloneSet(ObjectData*);
  ObjectData* cloneImpl();

  const Func* methodNamed(const StringData*) const;

  static size_t sizeForNProps(Slot);

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

  template <class T>
  struct PropLookup {
    T prop;
    bool accessible;
  };

  PropLookup<TypedValue*> getProp(const Class*, const StringData*);
  PropLookup<const TypedValue*> getProp(const Class*, const StringData*) const;

 private:
  template <bool warn, bool define>
  void propImpl(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
                const StringData* key);
  bool propEmptyImpl(Class* ctx, const StringData* key);
  bool invokeSet(TypedValue* retval, const StringData* key, TypedValue* val);
  bool invokeGet(TypedValue* retval, const StringData* key);
  bool invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                     const StringData* key);
  bool invokeIsset(TypedValue* retval, const StringData* key);
  bool invokeUnset(TypedValue* retval, const StringData* key);
  bool invokeNativeGetProp(TypedValue* retval, const StringData* key);
  bool invokeNativeSetProp(TypedValue* retval, const StringData* key,
                           TypedValue* val);
  bool invokeNativeIssetProp(TypedValue* retval, const StringData* key);
  bool invokeNativeUnsetProp(TypedValue* retval, const StringData* key);
  void getProp(const Class* klass, bool pubOnly, const PreClass::Prop* prop,
               Array& props, std::vector<bool>& inserted) const;
  void getProps(const Class* klass, bool pubOnly, const PreClass* pc,
                Array& props, std::vector<bool>& inserted) const;
  void getTraitProps(const Class* klass, bool pubOnly, const Class* trait,
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
  static void raiseAbstractClassError(Class*);
  void raiseUndefProp(const StringData*);

  static constexpr ptrdiff_t getVMClassOffset() {
    return offsetof(ObjectData, m_cls);
  }
  static constexpr ptrdiff_t attributeOff() {
    return offsetof(ObjectData, o_attribute);
  }
  static constexpr ptrdiff_t whStateOffset() {
    return offsetof(ObjectData, o_subclass_u8);
  }

private:
  friend struct MemoryProfile;

  const char* classname_cstr() const;

  static void compileTimeAssertions();

  bool toBooleanImpl() const noexcept;
  int64_t toInt64Impl() const noexcept;
  double toDoubleImpl() const noexcept;

// offset:  0    4    8     10  11    12     16   20          32
// 64bit:   cls       attr  u8  kind  count  id   [subclass]  [props...]
// lowptr:  cls  id   attr  u8  kind  count  [subclass][props...]

private:
#ifdef USE_LOWPTR
  LowClassPtr m_cls;
  int o_id; // Numeric identifier of this object (used for var_dump())
  union {
    struct {
      mutable uint16_t o_attribute;
      uint8_t o_subclass_u8; // for subclasses
      HeaderKind m_kind;
      mutable RefCount m_count;
    };
    uint64_t m_attr_kind_count;
  };
#else
  LowClassPtr m_cls;
  union {
    struct {
      mutable uint16_t o_attribute;
      uint8_t o_subclass_u8; // for subclasses
      HeaderKind m_kind;
      mutable RefCount m_count;
    };
    uint64_t m_attr_kind_count;
  };
  int o_id; // Numeric identifier of this object (used for var_dump())
#endif
} __attribute__((__aligned__(16)));

typedef GlobalsArray GlobalVariables;

struct CountableHelper : private boost::noncopyable {
  explicit CountableHelper(ObjectData* object) : m_object(object) {
    object->incRefCount();
  }
  ~CountableHelper() {
    m_object->decRefCount();
  }
private:
  ObjectData *m_object;
};

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
  explicit ExtObjectDataFlags(HPHP::Class* cb,
                              HeaderKind kind = HeaderKind::Object)
    : ObjectData(cb, Flags | ObjectData::IsCppBuiltin, kind)
  {
    assert(!getVMClass()->callsCustomInstanceInit());
  }

protected:
  ~ExtObjectDataFlags() {}
};

using ExtObjectData = ExtObjectDataFlags<ObjectData::IsCppBuiltin>;

template<class T, class... Args> T* newobj(Args&&... args) {
  static_assert(std::is_convertible<T*,ObjectData*>::value, "");
  auto const mem = MM().smartMallocSizeLogged(sizeof(T));
  try {
    return new (mem) T(std::forward<Args>(args)...);
  } catch (...) {
    MM().smartFreeSizeLogged(mem, sizeof(T));
    throw;
  }
}

#define DECLARE_OBJECT_ALLOCATION(T)                                    \
  static void typeCheck() {                                             \
    static_assert(std::is_base_of<ObjectData,T>::value, "");            \
  }                                                                     \
  virtual void sweep() override;

#define IMPLEMENT_OBJECT_ALLOCATION(T) \
  static_assert(std::is_base_of<ObjectData,T>::value, ""); \
  void HPHP::T::sweep() { this->~T(); }

#define DECLARE_CLASS_NO_SWEEP(originalName)                    \
  public:                                                       \
  CLASSNAME_IS(#originalName)                                   \
  friend ObjectData* new_##originalName##_Instance(Class*);     \
  friend void delete_##originalName(ObjectData*, const Class*); \
  static inline HPHP::LowClassPtr& classof() {                  \
    static HPHP::LowClassPtr result;                            \
    return result;                                              \
  }

/**
 * By this declaration a class introduced with DECLARE_CLASS can only
 * be smart-allocated.
 */
#define DECLARE_CLASS(cls)                      \
  DECLARE_OBJECT_ALLOCATION(c_##cls)            \
  DECLARE_CLASS_NO_SWEEP(cls)

#define IMPLEMENT_CLASS_NO_SWEEP(cls)

#define IMPLEMENT_CLASS(cls)                    \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls)

///////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////

#define incl_HPHP_OBJECT_DATA_INL_H_
#include "hphp/runtime/base/object-data-inl.h"
#undef incl_HPHP_OBJECT_DATA_INL_H_

///////////////////////////////////////////////////////////////////////////////

#endif
