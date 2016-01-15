#ifndef incl_HPHP_EXT_COLLECTIONS_H
#define incl_HPHP_EXT_COLLECTIONS_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/header-kind.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_HH_Pair, s_HH_Vector, s_HH_ImmVector,
  s_HH_Map, s_HH_ImmMap, s_HH_Set, s_HH_ImmSet;

#define DECLARE_COLLECTIONS_CLASS(name) \
  static Class* s_cls; \
  static Class* classof() { \
    assertx(s_cls); \
    return s_cls; \
  } \
  static bool instanceof(const ObjectData* obj) { \
    return obj->instanceof(classof()); \
  } \
  static ObjectData* instanceCtor(Class* cls) { \
    assert(cls); \
    assert(cls->isCollectionClass()); \
    assert(cls->classof(c_##name::classof())); \
    assert(cls->attrs() & AttrFinal); \
    if (std::is_same<c_##name, c_Pair>::value) { \
      return \
        new (MM().objMalloc(sizeof(c_Pair))) c_Pair(c_Pair::NoInit{}, cls); \
    } else { \
      return new (MM().objMalloc(sizeof(c_##name))) c_##name(cls); \
    } \
  } \
  static void instanceDtor(ObjectData* obj, const Class*) { \
    auto coll = collections::coll_cast<c_##name>(obj); \
    coll->~c_##name(); \
    MM().objFree(obj, sizeof(c_##name)); \
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
  assert(od->isCollection());
  return *reinterpret_cast<const uint32_t*>(
    reinterpret_cast<const char*>(od) + FAST_SIZE_OFFSET
  );
}

ATTRIBUTE_NORETURN void throwOOB(int64_t key);
ATTRIBUTE_NORETURN void throwUndef(int64_t key);
ATTRIBUTE_NORETURN void throwUndef(const StringData* key);

/////////////////////////////////////////////////////////////////////////////

class CollectionsExtension : public Extension {
 public:
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
    auto cls = const_cast<Class*>(T::classof());
    assert(cls);
    assert(cls->isCollectionClass());
    assert(cls->attrs() & AttrFinal);
    assert(!cls->getNativeDataInfo());
    assert(!cls->instanceCtor());
    assert(!cls->instanceDtor());
    cls->allocExtraData();
    cls->m_extra.raw()->m_instanceCtor = T::instanceCtor;
    cls->m_extra.raw()->m_instanceDtor = T::instanceDtor;
  }
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
