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

inline ObjectData::ObjectData(Class* cls, uint8_t flags, HeaderKind kind)
  : m_cls(cls)
{
  initHeader_16(kind, OneReference, flags | cls->getODAttrs());
  assertx(isObjectKind(m_kind));
  assertx(!cls->needInitialization() || cls->initialized());
  assertx(!isCollection()); // collections use NoInit{}
  o_id = ++os_max_id;
  instanceInit(cls);
}

inline ObjectData::ObjectData(Class* cls, InitRaw, uint8_t flags,
                              HeaderKind kind) noexcept
  : m_cls(cls)
{
  initHeader_16(kind, OneReference, flags);
  assertx(isObjectKind(m_kind));
  assertx(!cls->needInitialization() || cls->initialized());
  assertx(!(cls->getODAttrs() & ~static_cast<uint8_t>(flags)));
  o_id = ++os_max_id;
}

inline ObjectData::ObjectData(Class* cls, NoInit, uint8_t flags,
                              HeaderKind kind) noexcept
  : ObjectData(cls, InitRaw{}, flags, kind)
{
  assertx(cls->numDeclProperties() == 0);
  assertx(!cls->hasMemoSlots());
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
    assertx(obj->checkCount());
    assertx(obj->hasInstanceDtor());
  } else if (cls->hasMemoSlots()) {
    auto const size = sizeForNProps(cls->numDeclProperties());
    auto const objOff = objOffFromMemoNode(cls);
    auto mem = tl_heap->objMalloc(size + objOff);
    new (NotNull{}, mem) MemoNode(objOff);
    std::memset(
      reinterpret_cast<char*>(mem) + sizeof(MemoNode),
      0,
      objOff - sizeof(MemoNode)
    );
    obj = new (NotNull{}, reinterpret_cast<char*>(mem) + objOff)
      ObjectData(cls);
    assertx(obj->hasExactlyOneRef());
    assertx(!obj->hasInstanceDtor());
  } else {
    auto const size = sizeForNProps(cls->numDeclProperties());
    auto& mm = *tl_heap;
    obj = new (NotNull{}, mm.objMalloc(size)) ObjectData(cls);
    assertx(obj->hasExactlyOneRef());
    assertx(!obj->hasInstanceDtor());
  }

  if (UNLIKELY(cls->needsInitThrowable())) {
    // may incref obj
    throwable_init(obj);
    assertx(obj->checkCount());
  }

  return obj;
}

inline ObjectData* ObjectData::newInstanceNoPropInit(Class* cls) {
  if (cls->needInitialization()) cls->initialize();

  assertx(!cls->instanceCtor() &&
         !(cls->attrs() &
           (AttrAbstract | AttrInterface | AttrTrait | AttrEnum)));

  ObjectData* obj;
  auto const size = sizeForNProps(cls->numDeclProperties());
  if (cls->hasMemoSlots()) {
    auto const objOff = objOffFromMemoNode(cls);
    auto mem = tl_heap->objMalloc(size + objOff);
    new (NotNull{}, mem) MemoNode(objOff);
    std::memset(
      reinterpret_cast<char*>(mem) + sizeof(MemoNode),
      0,
      objOff - sizeof(MemoNode)
    );
    obj = new (NotNull{}, reinterpret_cast<char*>(mem) + objOff)
      ObjectData(cls, InitRaw{}, cls->getODAttrs());
  } else {
    obj = new (NotNull{}, tl_heap->objMalloc(size))
      ObjectData(cls, InitRaw{}, cls->getODAttrs());
  }
  assertx(obj->hasExactlyOneRef());
  return obj;
}

inline void ObjectData::instanceInit(Class* cls) {
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() > 0) {
      const Class::PropInitVec* propInitVec = m_cls->getPropData();
      assertx(propInitVec != nullptr);
      assertx(nProps == propInitVec->size());
      if (!cls->hasDeepInitProps()) {
        memcpy16_inline(propVecForConstruct(),
                        &(*propInitVec)[0], nProps * sizeof(TypedValue));
      } else {
        deepInitHelper(propVecForConstruct(), &(*propInitVec)[0], nProps);
      }
    } else {
      assertx(nProps == cls->declPropInit().size());
      memcpy16_inline(propVecForConstruct(),
                      &cls->declPropInit()[0], nProps * sizeof(TypedValue));
    }
  }
}

inline Class* ObjectData::getVMClass() const {
  assertx(kindIsValid());
  return m_cls;
}

inline void ObjectData::setVMClass(Class* cls) {
  m_cls = cls;
}

inline bool ObjectData::instanceof(const Class* c) const {
  return m_cls->classof(c);
}

inline bool ObjectData::isBeingConstructed() const {
  return getAttribute(Attribute::IsBeingConstructed);
}

inline bool ObjectData::isCollection() const {
  return m_kind >= HeaderKind::Vector && m_kind <= HeaderKind::ImmSet;
}

inline bool ObjectData::isCppBuiltin() const {
  return HPHP::isCppBuiltin(m_kind);
}

inline bool ObjectData::isWaitHandle() const {
  return isWaithandleKind(m_kind);
}

inline bool ObjectData::isMutableCollection() const {
  return isCollection() && HPHP::isMutableCollection(collectionType());
}

inline bool ObjectData::isImmutableCollection() const {
  return isCollection() && HPHP::isImmutableCollection(collectionType());
}

inline CollectionType ObjectData::collectionType() const {
  assertx(isValidCollection(static_cast<CollectionType>(m_kind)));
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
  return HPHP::hasInstanceDtor(m_kind);
}

inline bool ObjectData::hasNativeData() const {
  return m_kind == HeaderKind::NativeObject;
}

inline uint32_t ObjectData::getId() const {
  return o_id;
}

inline bool ObjectData::toBoolean() const {
  if (UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    return toBooleanImpl();
  }
  return true;
}

inline int64_t ObjectData::toInt64() const {
  if (!isCollection() && UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    return toInt64Impl();
  }
  raiseObjToIntNotice(classname_cstr());
  return 1;
}

inline double ObjectData::toDouble() const {
  if (!isCollection() && UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    return toDoubleImpl();
  }
  raiseObjToDoubleNotice(classname_cstr());
  return 1;
}

inline const Func* ObjectData::methodNamed(const StringData* sd) const {
  return getVMClass()->lookupMethod(sd);
}

[[noreturn]] void throw_cannot_modify_immutable_object(const char* className);

inline TypedValue* ObjectData::propVecForWrite() {
  if (UNLIKELY(m_cls->hasImmutableProps()) && !isBeingConstructed()) {
    throw_cannot_modify_immutable_object(getClassName().data());
  }
  return const_cast<TypedValue*>(propVec());
}

inline TypedValue* ObjectData::propVecForConstruct() {
  return const_cast<TypedValue*>(propVec());
}

inline const TypedValue* ObjectData::propVec() const {
  return reinterpret_cast<const TypedValue*>(uintptr_t(this + 1));
}

inline tv_lval ObjectData::propLvalAtOffset(Slot idx) {
  assertx(idx < m_cls->numDeclProperties());
  assertx(!(m_cls->declProperties()[idx].attrs & AttrIsImmutable));
  return tv_lval { const_cast<TypedValue*>(&propVec()[idx]) };
}

inline tv_rval ObjectData::propRvalAtOffset(Slot idx) const {
  assertx(idx < m_cls->numDeclProperties());
  return tv_rval { &propVec()[idx] };
}

inline bool ObjectData::hasDynProps() const {
  return getAttribute(HasDynPropArr) && !dynPropArray().empty();
}

inline MemoSlot* ObjectData::memoSlot(Slot slot) {
  assertx(!hasNativeData());
  assertx(slot < m_cls->numMemoSlots());
  return reinterpret_cast<MemoSlot*>(this) - slot - 1;
}

inline const MemoSlot* ObjectData::memoSlot(Slot slot) const {
  assertx(!hasNativeData());
  assertx(slot < m_cls->numMemoSlots());
  return reinterpret_cast<const MemoSlot*>(this) - slot - 1;
}

inline MemoSlot* ObjectData::memoSlotNativeData(Slot slot,
                                                size_t ndiSize) {
  assertx(slot < m_cls->numMemoSlots());
  return reinterpret_cast<MemoSlot*>(
    reinterpret_cast<char*>(this) - alignTypedValue(ndiSize)
  ) - slot - 1;
}

inline const MemoSlot* ObjectData::memoSlotNativeData(
  Slot slot, size_t ndiSize
) const {
  assertx(slot < m_cls->numMemoSlots());
  return reinterpret_cast<const MemoSlot*>(
    reinterpret_cast<const char*>(this) - alignTypedValue(ndiSize)
  ) - slot - 1;
}

inline size_t ObjectData::sizeForNProps(Slot nProps) {
  return sizeof(ObjectData) + sizeof(TypedValue) * nProps;
}

inline size_t ObjectData::objOffFromMemoNode(const Class* cls) {
  assertx(cls->hasMemoSlots());
  assertx(!cls->getNativeDataInfo());
  return cls->numMemoSlots() * sizeof(MemoSlot) + sizeof(MemoNode);
}

///////////////////////////////////////////////////////////////////////////////
}
