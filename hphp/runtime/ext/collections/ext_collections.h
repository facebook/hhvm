#ifndef incl_HPHP_EXT_COLLECTIONS_H
#define incl_HPHP_EXT_COLLECTIONS_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

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
  static void instanceDtor(ObjectData* obj, const Class*) { \
    assertx(obj->getVMClass() == c_##name::classof());      \
    auto coll = static_cast<c_##name*>(obj);                \
    coll->~c_##name();                                      \
    tl_heap->objFree(obj, sizeof(c_##name));                \
  }

#define DECLARE_COLLECTIONS_CLASS(name)                     \
  DECLARE_COLLECTIONS_CLASS_NOCTOR(name)                    \
  static ObjectData* instanceCtor(Class* cls) {             \
    assertx(cls == classof());                              \
    return req::make<c_##name>().detach();                  \
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
constexpr ptrdiff_t FAST_SIZE_OFFSET = 16;
inline size_t getSize(const ObjectData* od) {
  assertx(od->isCollection());
  return *reinterpret_cast<const uint32_t*>(
    reinterpret_cast<const char*>(od) + FAST_SIZE_OFFSET
  );
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
    assertx(!cls->instanceCtor<false>());
    assertx(!cls->instanceCtor<true>());
    assertx(!cls->instanceDtor());
    assertx(!cls->hasMemoSlots());
    cls->allocExtraData();
    cls->m_extra.raw()->m_instanceCtor = T::instanceCtor;
    cls->m_extra.raw()->m_instanceCtorUnlocked = T::instanceCtor;
    cls->m_extra.raw()->m_instanceDtor = T::instanceDtor;
    cls->m_releaseFunc = T::instanceDtor;
    cls->initRTAttributes(
        Class::UseGet |
        Class::UseSet |
        Class::UseIsset |
        Class::UseUnset |
        Class::CallToImpl
    );
  }
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
