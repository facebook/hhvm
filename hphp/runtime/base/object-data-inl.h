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

#ifndef incl_HPHP_OBJECT_DATA_INL_H_
#error "object-data-inl.h should only be included by object-data.h"
#endif

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void ObjectData::resetMaxId() {
  os_max_id = 0;
}

inline ObjectData::ObjectData(Class* cls)
  : ObjectData(cls, 0, HeaderKind::Object)
{}

inline ObjectData::ObjectData(Class* cls, uint16_t flags, HeaderKind kind)
  : m_cls(cls)
{
  m_hdr.init(flags, kind, 1);
  assert(m_hdr.aux == flags && hasExactlyOneRef());
  assert(isObjectKind(kind));
  assert(!cls->needInitialization() || cls->initialized());
  o_id = ++os_max_id;
  if (flags & Attribute::IsCollection) {
    // Whatever attribute we need to set, do it via flags and void runtime
    // loading.  These assertions guarantee that `instanceInit(cls)' is not
    // needed for collections.
    assertx(!(cls->getODAttrs() & ~static_cast<uint16_t>(flags)));
    assertx(cls->numDeclProperties() == 0);
    return;
  }
  instanceInit(cls);
}

inline ObjectData::ObjectData(Class* cls, NoInit) noexcept
  : m_cls(cls)
{
  m_hdr.init(0, HeaderKind::Object, 1);
  assert(!m_hdr.aux && m_hdr.kind == HeaderKind::Object && hasExactlyOneRef());
  assert(!cls->needInitialization() || cls->initialized());
  o_id = ++os_max_id;
}

inline ObjectData::ObjectData(Class* cls,
                              uint16_t flags,
                              HeaderKind kind,
                              NoInit) noexcept
  : m_cls(cls)
{
  m_hdr.init(flags, kind, 1);
  assert(m_hdr.aux == flags && hasExactlyOneRef());
  assert(isObjectKind(kind));
  assert(!cls->needInitialization() || cls->initialized());
  assert(!(cls->getODAttrs() & ~static_cast<uint16_t>(flags)));
  assert(cls->numDeclProperties() == 0);
  o_id = ++os_max_id;
}

inline void ObjectData::setStatic() const {
  assert(false);
}

inline bool ObjectData::isStatic() const {
  return false;
}

inline void ObjectData::setUncounted() const {
  assert(false);
}

inline bool ObjectData::isUncounted() const {
  return false;
}

inline size_t ObjectData::heapSize() const {
  return m_cls->builtinODTailSize() + sizeForNProps(m_cls->numDeclProperties());
}

inline ObjectData* ObjectData::newInstance(Class* cls) {
  if (cls->needInitialization()) {
    cls->initialize();
  }
  if (auto const ctor = cls->instanceCtor()) {
    auto obj = ctor(cls);
    assert(obj->getCount() > 0);
    return obj;
  }
  Attr attrs = cls->attrs();
  if (UNLIKELY(attrs &
               (AttrAbstract | AttrInterface | AttrTrait | AttrEnum))) {
    raiseAbstractClassError(cls);
  }
  size_t nProps = cls->numDeclProperties();
  size_t size = sizeForNProps(nProps);
  auto& mm = MM();
  auto const obj = new (mm.objMalloc(size)) ObjectData(cls);
  assert(obj->hasExactlyOneRef());
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

  // callCustomInstanceInit may have inc-refd.
  assert(obj->getCount() > 0);
  return obj;
}

inline void ObjectData::instanceInit(Class* cls) {
  m_hdr.aux |= cls->getODAttrs();

  size_t nProps = cls->numDeclProperties();
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

inline Class* ObjectData::getVMClass() const {
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
  assert(isValidCollection(static_cast<CollectionType>(m_hdr.kind)));
  return static_cast<CollectionType>(m_hdr.kind);
}

inline bool ObjectData::isIterator() const {
  return instanceof(SystemLib::s_IteratorClass);
}

inline bool ObjectData::getAttribute(Attribute attr) const {
  return m_hdr.aux & attr;
}

inline uint16_t ObjectData::getAttributes() const { return m_hdr.aux; }

inline void ObjectData::setAttribute(Attribute attr) {
  m_hdr.aux |= attr;
}

inline bool ObjectData::noDestruct() const {
  return getAttribute(NoDestructor);
}

inline void ObjectData::setNoDestruct() {
  setAttribute(NoDestructor);
}

inline void ObjectData::clearNoDestruct() {
  m_hdr.aux &= ~NoDestructor;
}

inline int ObjectData::getId() const {
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
  return toInt64();
}

inline const Func* ObjectData::methodNamed(const StringData* sd) const {
  return getVMClass()->lookupMethod(sd);
}

inline size_t ObjectData::sizeForNProps(Slot nProps) {
  return sizeof(ObjectData) + sizeof(TypedValue) * nProps;
}

///////////////////////////////////////////////////////////////////////////////
}
