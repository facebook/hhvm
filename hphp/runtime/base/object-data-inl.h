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

#ifndef incl_HPHP_OBJECT_DATA_INL_H_
#error "object-data-inl.h should only be included by object-data.h"
#endif

#include "hphp/runtime/base/exceptions.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void ObjectData::resetMaxId() {
  os_max_id = 0;
}

inline ObjectData::ObjectData(Class* cls, uint16_t flags, HeaderKind kind)
  : m_cls(cls)
{
  initHeader(flags, kind, 1);
  assert(m_aux16 == flags && hasExactlyOneRef());
  assert(isObjectKind(m_kind));
  assert(!cls->needInitialization() || cls->initialized());
  o_id = ++os_max_id;

  if (flags & Attribute::IsCollection) {
    // Whatever attribute we need to set, do it via flags and void runtime
    // loading.  These assertions guarantee that `instanceInit(cls)' is not
    // needed for collections.
    assertx(cls->numDeclProperties() == 0);
    return;
  }
  instanceInit(cls);
}

inline ObjectData::ObjectData(Class* cls, NoInit) noexcept
  : m_cls(cls)
{
  initHeader(uint16_t(0), HeaderKind::Object, 1);
  assert(m_aux16 == 0 && hasExactlyOneRef());
  assert(!cls->needInitialization() || cls->initialized());
  o_id = ++os_max_id;
}

inline ObjectData::ObjectData(Class* cls,
                              uint16_t flags,
                              HeaderKind kind,
                              NoInit) noexcept
  : m_cls(cls)
{
  initHeader(flags, kind, 1);
  assert(m_aux16 == flags && hasExactlyOneRef());
  assert(isObjectKind(m_kind));
  assert(!cls->needInitialization() || cls->initialized());
  assert(!(cls->getODAttrs() & ~static_cast<uint16_t>(flags)));
  assert(cls->numDeclProperties() == 0);
  o_id = ++os_max_id;
}

inline size_t ObjectData::heapSize() const {
  return sizeForNProps(m_cls->numDeclProperties());
}

inline ObjectData* ObjectData::newInstance(Class* cls) {
  Attr attrs = cls->attrs();
  if (UNLIKELY(attrs &
               (AttrAbstract | AttrInterface | AttrTrait | AttrEnum))) {
    raiseAbstractClassError(cls);
  }
  if (cls->needInitialization()) {
    cls->initialize();
  }

  ObjectData* obj;
  if (auto const ctor = cls->instanceCtor()) {
    obj = ctor(cls);
    assert(obj->checkCount());
    assertx(obj->hasInstanceDtor());
  } else {
    size_t nProps = cls->numDeclProperties();
    size_t size = sizeForNProps(nProps);
    auto& mm = MM();
    obj = new (mm.objMalloc(size)) ObjectData(cls);
    assert(obj->hasExactlyOneRef());
    assertx(!obj->hasInstanceDtor());
  }

  if (UNLIKELY(cls->needsInitThrowable())) {
    // may incref obj
    throwable_init(obj);
    assert(obj->checkCount());
  }

  return obj;
}

inline ObjectData* ObjectData::newInstanceNoPropInit(Class* cls) {
  if (cls->needInitialization()) cls->initialize();

  assert(!cls->instanceCtor() &&
         !(cls->attrs() &
           (AttrAbstract | AttrInterface | AttrTrait | AttrEnum)));

  size_t nProps = cls->numDeclProperties();
  size_t size = sizeForNProps(nProps);
  auto& mm = MM();
  auto const obj = new (mm.objMalloc(size)) ObjectData(cls, NoInit{});
  obj->m_aux16 |= cls->getODAttrs();
  assert(obj->hasExactlyOneRef());
  return obj;
}

inline void ObjectData::instanceInit(Class* cls) {
  m_aux16 |= cls->getODAttrs();

  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() > 0) {
      const Class::PropInitVec* propInitVec = m_cls->getPropData();
      assert(propInitVec != nullptr);
      assert(nProps == propInitVec->size());
      if (!cls->hasDeepInitProps()) {
        memcpy16_inline(propVec(),
                        &(*propInitVec)[0], nProps * sizeof(TypedValue));
      } else {
        deepInitHelper(propVec(), &(*propInitVec)[0], nProps);
      }
    } else {
      assert(nProps == cls->declPropInit().size());
      memcpy16_inline(propVec(),
                      &cls->declPropInit()[0], nProps * sizeof(TypedValue));
    }
  }
}

inline Class* ObjectData::getVMClass() const {
  assert(kindIsValid());
  return m_cls;
}

inline void ObjectData::setVMClass(Class* cls) {
  m_cls = cls;
}

inline bool ObjectData::instanceof(const Class* c) const {
  return m_cls->classof(c);
}

inline bool ObjectData::isCollection() const {
  return getAttribute(Attribute::IsCollection);
}

inline bool ObjectData::isMutableCollection() const {
  return isCollection() && HPHP::isMutableCollection(collectionType());
}

inline bool ObjectData::isImmutableCollection() const {
  return isCollection() && HPHP::isImmutableCollection(collectionType());
}

inline CollectionType ObjectData::collectionType() const {
  assert(isValidCollection(static_cast<CollectionType>(m_kind)));
  return static_cast<CollectionType>(m_kind);
}

inline HeaderKind ObjectData::headerKind() const {
  return m_kind;
}

inline bool ObjectData::isIterator() const {
  return instanceof(SystemLib::s_IteratorClass);
}

inline bool ObjectData::getAttribute(Attribute attr) const {
  return m_aux16 & attr;
}

inline void ObjectData::setAttribute(Attribute attr) {
  m_aux16 |= attr;
}

inline bool ObjectData::noDestruct() const {
  return getAttribute(NoDestructor);
}

inline void ObjectData::setNoDestruct() {
  setAttribute(NoDestructor);
}

inline void ObjectData::clearNoDestruct() {
  m_aux16 &= ~NoDestructor;
}

inline bool ObjectData::hasInstanceDtor() const {
  return m_aux16 & (IsCppBuiltin | HasNativeData);
}

inline uint32_t ObjectData::getId() const {
  return o_id;
}

inline bool ObjectData::toBoolean() const {
  if (UNLIKELY(getAttribute(CallToImpl))) {
    return toBooleanImpl();
  }
  return true;
}

inline int64_t ObjectData::toInt64() const {
  if (UNLIKELY(getAttribute(CallToImpl) && !isCollection())) {
    return toInt64Impl();
  }
  raiseObjToIntNotice(classname_cstr());
  return 1;
}

inline double ObjectData::toDouble() const {
  if (UNLIKELY(getAttribute(CallToImpl) && !isCollection())) {
    return toDoubleImpl();
  }
  raiseObjToDoubleNotice(classname_cstr());
  return 1;
}

inline const Func* ObjectData::methodNamed(const StringData* sd) const {
  return getVMClass()->lookupMethod(sd);
}

inline TypedValue* ObjectData::propVec() {
  return reinterpret_cast<TypedValue*>(uintptr_t(this + 1));
}

inline const TypedValue* ObjectData::propVec() const {
  return const_cast<ObjectData*>(this)->propVec();
}

inline bool ObjectData::hasDynProps() const {
  return getAttribute(HasDynPropArr) && dynPropArray().size() != 0;
}

inline size_t ObjectData::sizeForNProps(Slot nProps) {
  return sizeof(ObjectData) + sizeof(TypedValue) * nProps;
}

///////////////////////////////////////////////////////////////////////////////
}
