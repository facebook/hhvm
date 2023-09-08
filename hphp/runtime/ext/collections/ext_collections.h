#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/native-prop-handler.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

ArrayIter getArrayIterHelper(const Variant& v, size_t& sz);

struct c_Collection : ObjectData {
  explicit c_Collection(Class* cls, HeaderKind kind) noexcept :
    ObjectData(cls, NoInit{}, ObjectData::NoAttrs, kind) {}
};

namespace collections {

/////////////////////////////////////////////////////////////////////////////


#define DECLARE_COLLECTIONS_CLASS_NOCTOR(name)              \
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
  CollectionsExtension(): Extension("collections", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleInit() override {
    initPair();
    initVector();
    initMap();
    initSet();
  }

  std::vector<std::string> hackFiles() const override {
    return {
      "collections-pair",
      "collections-vector",
      "collections-map",
      "collections-set",
    };
  }

 private:
  void initPair();
  void initVector();
  void initMap();
  void initSet();
};

const StaticString s_isset{"isset"};

struct CollectionPropHandler: Native::BasePropHandler {
  static Variant issetProp(const Object&, const String&) {
    return false;
  }
  static bool isPropSupported(const String&, const String& op) {
    if (op.same(s_isset)) return true;
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot access a property on a collection");
  }
};

/////////////////////////////////////////////////////////////////////////////
}

template<class T> typename
  std::enable_if<std::is_base_of<c_Collection, T>::value, void>::type
finish_class(Class* cls) {
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
  cls->initRTAttributes(Class::CallToImpl);
}

}
