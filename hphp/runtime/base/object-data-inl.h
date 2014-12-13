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
  : m_cls(cls)
  , m_attr_kind_count(HeaderKind::Object << 24)
{
  assert(!o_attribute && m_kind == HeaderKind::Object && !m_count);
  assert(uintptr_t(this) % sizeof(TypedValue) == 0);
  if (cls->needInitialization()) {
    // Needs to happen before we assign this object an o_id.
    cls->initialize();
  }
  o_id = ++os_max_id;
  instanceInit(cls);
}

inline ObjectData::ObjectData(Class* cls, uint16_t flags)
  : m_cls(cls)
  , m_attr_kind_count(flags | HeaderKind::Object << 24)
{
  assert(o_attribute == flags && m_kind == HeaderKind::Object && !m_count);
  assert(uintptr_t(this) % sizeof(TypedValue) == 0);
  if (cls->needInitialization()) {
    // Needs to happen before we assign this object an o_id.
    cls->initialize();
  }
  o_id = ++os_max_id;
  instanceInit(cls);
}

inline ObjectData::ObjectData(Class* cls, NoInit)
  : m_cls(cls)
  , m_attr_kind_count(HeaderKind::Object << 24)
{
  assert(!o_attribute && m_kind == HeaderKind::Object && !m_count);
  assert(uintptr_t(this) % sizeof(TypedValue) == 0);
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

// Call newInstance() to instantiate a PHP object
inline ObjectData* ObjectData::newInstance(Class* cls) {
  if (auto const ctor = cls->instanceCtor()) {
    return ctor(cls);
  }
  Attr attrs = cls->attrs();
  if (UNLIKELY(attrs &
               (AttrAbstract | AttrInterface | AttrTrait | AttrEnum))) {
    raiseAbstractClassError(cls);
  }
  size_t nProps = cls->numDeclProperties();
  size_t size = sizeForNProps(nProps);
  auto& mm = MM();
  auto const obj = new (mm.objMallocLogged(size)) ObjectData(cls);
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
  if (UNLIKELY(cls->hasNativePropHandler())) {
    obj->setAttribute(ObjectData::Attribute::HasNativePropHandler);
  }
  return obj;
}

inline void ObjectData::instanceInit(Class* cls) {
  o_attribute |= cls->getODAttrs();

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

inline void ObjectData::release() {
  assert(!hasMultipleRefs());
  if (LIKELY(destruct())) DeleteObject(this);
}

inline Class* ObjectData::getVMClass() const {
  return m_cls;
}

inline bool ObjectData::instanceof(const Class* c) const {
  return m_cls->classof(c);
}

inline bool ObjectData::isCollection() const {
  return getAttribute(Attribute::IsCollection);
}

inline bool ObjectData::isMutableCollection() const {
  return Collection::isMutableType(getCollectionType());
}

inline bool ObjectData::isImmutableCollection() const {
  return Collection::isImmutableType(getCollectionType());
}

inline Collection::Type ObjectData::getCollectionType() const {
  return isCollection() ?
    static_cast<Collection::Type>(o_subclass_u8) :
    Collection::Type::InvalidType;
}

inline bool ObjectData::isIterator() const {
  return instanceof(SystemLib::s_IteratorClass);
}

inline bool ObjectData::getAttribute(Attribute attr) const {
  return o_attribute & attr;
}

inline void ObjectData::setAttribute(Attribute attr) const {
  o_attribute |= attr;
}

inline bool ObjectData::noDestruct() const {
  return getAttribute(NoDestructor);
}

inline void ObjectData::setNoDestruct() {
  setAttribute(NoDestructor);
}

inline void ObjectData::clearNoDestruct() {
  o_attribute &= ~NoDestructor;
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

inline uint8_t& ObjectData::subclass_u8() {
  return o_subclass_u8;
}

inline uint8_t ObjectData::subclass_u8() const {
  return o_subclass_u8;
}

inline const Func* ObjectData::methodNamed(const StringData* sd) const {
  return getVMClass()->lookupMethod(sd);
}

inline size_t ObjectData::sizeForNProps(Slot nProps) {
  size_t sz = sizeof(ObjectData) + (sizeof(TypedValue) * nProps);
  assert((sz & (sizeof(TypedValue) - 1)) == 0);
  return sz;
}

///////////////////////////////////////////////////////////////////////////////
}
