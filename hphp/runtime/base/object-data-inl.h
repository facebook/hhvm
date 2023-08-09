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
#include "hphp/runtime/base/tv-conv-notice.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline ObjectData::ObjectData(Class* cls, uint8_t flags, HeaderKind kind)
  : m_cls(cls)
{
  initHeader_16(kind, OneReference, flags);
  if (debug && cls->releaseFunc() == ObjectData::release) {
    DEBUG_ONLY auto const is_small = cls->sizeIdx() < kNumSmallSizes;
    assertx(is_small == bool(m_aux16 & Attribute::SmallAllocSize));
    assertx(is_small != bool(m_aux16 & Attribute::BigAllocSize));
  }
  assertx(isObjectKind(m_kind));
  assertx(!cls->needInitialization() || cls->initialized());
  assertx(!isCollection()); // collections use NoInit{}
  instanceInit(cls);
  assertx(props()->checkInvariants(cls->numDeclProperties()));
}

inline ObjectData::ObjectData(Class* cls, InitRaw, uint8_t flags,
                              HeaderKind kind) noexcept
  : m_cls(cls)
{
  initHeader_16(kind, OneReference, flags);
  if (debug && cls->releaseFunc() == ObjectData::release) {
    DEBUG_ONLY auto const is_small = cls->sizeIdx() < kNumSmallSizes;
    assertx(is_small == bool(m_aux16 & Attribute::SmallAllocSize));
    assertx(is_small != bool(m_aux16 & Attribute::BigAllocSize));
  }
  assertx(isObjectKind(m_kind));
  assertx(!cls->needInitialization() || cls->initialized());
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

ALWAYS_INLINE ObjectData::Alloc ObjectData::allocMemoInit(Class* cls) {
  auto result = Alloc{};
  auto const index = cls->sizeIdx();
  if (LIKELY(index < kNumSmallSizes)) {
    result.mem = tl_heap->mallocSmallIndex(index);
    result.flags = Attribute::SmallAllocSize;
  } else {
    // This cannot ever occur with our current setting for kNumSmallSizes.
    auto size = MemoryManager::sizeIndex2Size(index);
    result.mem = tl_heap->mallocBigSize(size);
    result.flags = Attribute::BigAllocSize;
  }

  if (cls->hasMemoSlots()) {
    auto const objOff = objOffFromMemoNode(cls);
    new (NotNull{}, result.mem) MemoNode(objOff);
    auto cur = reinterpret_cast<MemoSlot*>(
        reinterpret_cast<char*>(result.mem) + sizeof(MemoNode));
    auto end = reinterpret_cast<MemoSlot*>(
        reinterpret_cast<char*>(result.mem) + objOff);
    while (cur < end) {
      (cur++)->init();
    }
    result.mem = reinterpret_cast<char*>(result.mem) + objOff;
  }
  return result;
}

template <bool Unlocked, typename Init>
ALWAYS_INLINE
ObjectData* ObjectData::newInstanceImpl(Class* cls, Init objConstruct) {
  if (cls->needInitialization()) cls->initialize();

  if (auto const ctor = cls->instanceCtor<Unlocked>()) {
    auto obj = ctor(cls);
    assertx(obj->checkCount());
    assertx(obj->hasInstanceDtor());
    return obj;
  }

  auto const alloc = allocMemoInit(cls);

  auto obj = objConstruct(alloc.mem, alloc.flags);
  assertx(obj->hasExactlyOneRef());
  assertx(!obj->hasInstanceDtor());
  return obj;
}

template <bool Unlocked>
inline ObjectData* ObjectData::newInstance(Class* cls) {
  assertx(cls);
  if (UNLIKELY(cls->attrs() &
               (AttrAbstract | AttrInterface | AttrTrait | AttrEnum |
                AttrEnumClass))) {
    raiseAbstractClassError(cls);
  }
  auto obj = ObjectData::newInstanceImpl<Unlocked>(
    cls,
    [&](void* mem, uint8_t sizeFlag) {
      auto const flags = sizeFlag | (Unlocked ? IsBeingConstructed : NoAttrs);
      return new (NotNull{}, mem) ObjectData(cls, flags);
    }
  );
  if (UNLIKELY(cls->needsInitThrowable())) {
    // may incref obj
    throwable_init(obj);
    assertx(obj->checkCount());
  }
  return obj;
}

inline ObjectData* ObjectData::newInstanceNoPropInit(Class* cls) {
  assertx(!(cls->attrs() &
            (AttrAbstract | AttrInterface | AttrTrait | AttrEnum |
             AttrEnumClass)));
  return ObjectData::newInstanceImpl<false>(
    cls,
    [&](void* mem, uint8_t sizeFlag) {
      return new (NotNull{}, mem) ObjectData(cls, InitRaw{}, sizeFlag);
    }
  );
}

inline void ObjectData::instanceInit(Class* cls) {
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() > 0) {
      const Class::PropInitVec* propInitVec = cls->getPropData();
      assertx(propInitVec != nullptr);
      assertx(nProps == propInitVec->size());
      if (!cls->hasDeepInitProps()) {
        memcpy(props(), propInitVec->data(), ObjectProps::sizeFor(nProps));
      } else {
        deepInitHelper(props(), propInitVec, nProps);
      }
    } else {
      assertx(nProps == cls->declPropInit().size());
      memcpy(props(), cls->declPropInit().data(), ObjectProps::sizeFor(nProps));
    }
  }
}

inline void ObjectData::verifyPropTypeHintImpl(tv_lval val,
                                               const Class::Prop& prop) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(tvIsPlausible(val.tv()));

  auto const& tc = prop.typeConstraint;
  if (!tc.isCheckable()) return;

  if (UNLIKELY(type(val) == KindOfUninit)) {
    if ((prop.attrs & AttrLateInit) || tc.isMixedResolved()) return;
    raise_property_typehint_unset_error(
      prop.cls,
      prop.name,
      tc.isSoft(),
      tc.isUpperBound()
    );
    return;
  }

  tc.verifyProperty(val, m_cls, prop.cls, prop.name);

  if (debug && RuntimeOption::RepoAuthoritative) {
    // The fact that uninitialized LateInit props are uninit isn't
    // reflected in the repo-auth-type.
    if (type(val) != KindOfUninit || !(prop.attrs & AttrLateInit)) {
      always_assert(tvMatchesRepoAuthType(val.tv(), prop.repoAuthType));
    }
  }
}

inline void ObjectData::verifyPropTypeHints(size_t end) {
  assertx(end <= m_cls->declProperties().size());

  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;

  auto const declProps = m_cls->declProperties();
  for (size_t slot = 0; slot < end; ++slot) {
    auto index = m_cls->propSlotToIndex(slot);
    verifyPropTypeHintImpl(props()->at(index), declProps[slot]);
  }
}

inline void ObjectData::verifyPropTypeHints() {
  verifyPropTypeHints(m_cls->declProperties().size());
}

inline void ObjectData::verifyPropTypeHint(Slot slot) {
  assertx(slot < m_cls->declProperties().size());
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  auto index = m_cls->propSlotToIndex(slot);
  verifyPropTypeHintImpl(props()->at(index), m_cls->declProperties()[slot]);
}

inline bool ObjectData::assertPropTypeHints() const {
  auto const end = m_cls->declProperties().size();
  for (size_t slot = 0; slot < end; ++slot) {
    auto const prop = props()->at(m_cls->propSlotToIndex(slot));
    if (!isLazyProp(prop) && !assertTypeHint(prop, slot)) return false;
  }
  return true;
}

inline Class* ObjectData::getVMClass() const {
  assertx(kindIsValid());
  return m_cls;
}

inline bool ObjectData::instanceof(const Class* c) const {
  return m_cls->classof(c);
}

inline bool ObjectData::isBeingConstructed() const {
  return getAttribute(IsBeingConstructed);
}

inline void ObjectData::lockObject() {
  m_aux16 &= ~IsBeingConstructed;
}

inline void ObjectData::unlockObject() {
   m_aux16 |= IsBeingConstructed;
}

inline bool ObjectData::isCollection() const {
  return m_kind >= HeaderKind::Vector && m_kind <= HeaderKind::ImmSet;
}

inline bool ObjectData::isCppBuiltin() const {
  return HPHP::isCppBuiltin(m_kind);
}

inline bool ObjectData::isWaitHandle() const {
  return isWaitHandleKind(m_kind);
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
  return instanceof(SystemLib::getHH_IteratorClass());
}

inline bool ObjectData::getAttribute(Attribute attr) const {
  return m_aux16 & attr;
}

inline void ObjectData::setAttribute(Attribute attr) {
  m_aux16 |= attr;
}

inline bool ObjectData::hasInstanceDtor() const {
  return HPHP::hasInstanceDtor(m_kind);
}

inline bool ObjectData::hasNativeData() const {
  return m_kind == HeaderKind::NativeObject;
}

inline bool ObjectData::toBoolean() const {
  if (UNLIKELY(m_cls->rtAttribute(Class::CallToImpl))) {
    return toBooleanImpl();
  }
  return true;
}

inline const Func* ObjectData::methodNamed(const StringData* sd) const {
  return getVMClass()->lookupMethod(sd);
}

[[noreturn]] void throw_cannot_modify_const_object(const char* className);

inline ObjectProps* ObjectData::props() {
  return reinterpret_cast<ObjectProps*>(this + 1);
}

inline const ObjectProps* ObjectData::props() const {
  return const_cast<ObjectData*>(this)->props();
}

inline tv_lval ObjectData::propLvalAtOffset(Slot slot) {
  assertx(slot < m_cls->numDeclProperties());
  assertx(!(m_cls->declProperties()[slot].attrs & AttrIsConst));
  auto index = m_cls->propSlotToIndex(slot);
  return props()->at(index);
}

inline tv_rval ObjectData::propRvalAtOffset(Slot slot) const {
  assertx(slot < m_cls->numDeclProperties());
  auto index = m_cls->propSlotToIndex(slot);
  return props()->at(index);
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
  return alignTypedValue(sizeof(ObjectData) + ObjectProps::sizeFor(nProps));
}

inline size_t ObjectData::objOffFromMemoNode(const Class* cls) {
  assertx(cls->hasMemoSlots());
  assertx(!cls->getNativeDataInfo());
  return cls->numMemoSlots() * sizeof(MemoSlot) + sizeof(MemoNode);
}

///////////////////////////////////////////////////////////////////////////////
}
