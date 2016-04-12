#ifndef incl_HPHP_EXT_COLLECTIONS_H
#define incl_HPHP_EXT_COLLECTIONS_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct HashCollection;
struct BaseVector;
struct c_Pair;
struct c_Vector;
void triggerCow(c_Vector* vec);
ArrayIter getArrayIterHelper(const Variant& v, size_t& sz);

namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_HH_Pair, s_HH_Vector, s_HH_ImmVector,
  s_HH_Map, s_HH_ImmMap, s_HH_Set, s_HH_ImmSet;

#define DECLARE_COLLECTIONS_CLASS_NOCTOR(name)              \
  static Class* s_cls;                                      \
                                                            \
  static Class* classof() {                                 \
    assertx(s_cls);                                         \
    return s_cls;                                           \
  }                                                         \
                                                            \
  static bool instanceof(const ObjectData* obj) {           \
    return obj->instanceof(classof());                      \
  }                                                         \
                                                            \
  static void instanceDtor(ObjectData* obj, const Class*) { \
    auto coll = collections::coll_cast<c_##name>(obj);      \
    coll->~c_##name();                                      \
    MM().objFree(obj, sizeof(c_##name));                    \
  }

// c_Pair uses a slightly different constructor invocation
#define DECLARE_COLLECTIONS_CLASS(name)                           \
  DECLARE_COLLECTIONS_CLASS_NOCTOR(name)                          \
  static ObjectData* instanceCtor(Class* cls) {                   \
    assertx(cls);                                                 \
    assertx(cls->isCollectionClass());                            \
    assertx(cls->classof(c_##name::classof()));                   \
    assertx(cls->attrs() & AttrFinal);                            \
    return new (MM().objMalloc(sizeof(c_##name))) c_##name(cls);  \
  }

constexpr ObjectData::Attribute objectFlags =
  static_cast<ObjectData::Attribute>(
    ObjectData::IsCollection |
    ObjectData::CallToImpl |
    ObjectData::NoDestructor |
    ObjectData::HasClone |
    ObjectData::UseGet |
    ObjectData::UseSet |
    ObjectData::UseIsset |
    ObjectData::UseUnset |
    ObjectData::IsCppBuiltin
  );

size_t heapSize(HeaderKind kind);

template<class T>
T* coll_cast(ObjectData* obj) {
  assertx(obj->isCollection());
  assertx(T::instanceof(obj));
  return static_cast<T*>(obj);
}

/**
 * The "materialization" methods have the form "to[CollectionName]()" and
 * allow us to get an instance of a collection type from another.
 * This template provides a default implementation.
 */
template<typename TCollection>
Object materialize(ObjectData* obj) {
  auto col = req::make<TCollection>();
  col->init(VarNR(obj));
  return Object{std::move(col)};
}

/*
 * All native collection class have their m_size field at the same
 * offset in the object.
 */
constexpr ptrdiff_t FAST_SIZE_OFFSET = use_lowptr ? 16 : 24;
inline size_t getSize(const ObjectData* od) {
  assertx(od->isCollection());
  return *reinterpret_cast<const uint32_t*>(
    reinterpret_cast<const char*>(od) + FAST_SIZE_OFFSET
  );
}

// Collection constructors do not throw exceptions, let's not try to catch
// exceptions here.
template<class T, class... Args> T* newCollectionObj(Args&&... args) {
  static_assert(std::is_convertible<T*,BaseVector*>::value ||
                std::is_convertible<T*,HashCollection*>::value ||
                std::is_convertible<T*,c_Pair*>::value, "");
  auto const mem = MM().mallocSmallSize(sizeof(T));
  auto col = new (mem) T(std::forward<Args>(args)...);
  assert(col->hasExactlyOneRef());
  return col;
}

[[noreturn]] void throwOOB(int64_t key);
[[noreturn]] void throwUndef(int64_t key);
[[noreturn]] void throwUndef(const StringData* key);

/////////////////////////////////////////////////////////////////////////////

struct CollectionsExtension : Extension {
  CollectionsExtension(): Extension("collections") {}

  void moduleInit() override {
    initPair();
    initVector();
    initMap();
    initSet();
  }

 private:
  void initPair();
  void initVector();
  void initMap();
  void initSet();

  template<class T>
  void finishClass() {
    auto const cls = const_cast<Class*>(T::classof());
    assertx(cls);
    assertx(cls->isCollectionClass());
    assertx(cls->attrs() & AttrFinal);
    assertx(!cls->getNativeDataInfo());
    assertx(!cls->instanceCtor());
    assertx(!cls->instanceDtor());
    cls->allocExtraData();
    cls->m_extra.raw()->m_instanceCtor = T::instanceCtor;
    cls->m_extra.raw()->m_instanceDtor = T::instanceDtor;
  }
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
