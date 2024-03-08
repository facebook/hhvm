#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/text-util.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// HashCollection

HashCollection::HashCollection(Class* cls, HeaderKind kind, uint32_t cap)
  : c_Collection(cls, kind)
  , m_unusedAndSize(0)
{
  setArrayData(cap > 0
    ? VanillaDict::as(VanillaDict::MakeReserveDict(cap))
    : CreateDictAsMixed());
}

NEVER_INLINE
void HashCollection::throwTooLarge() {
  assertx(getClassName().size() == 6);
  auto clsName = getClassName().get()->slice();
  String msg(130, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(buf.data(), buf.size() + 1,
    "%s object has reached its maximum capacity of %u element "
    "slots and does not have room to add a new element",
    clsName.data() + 3, // strip "HH\" prefix
    MaxSize
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

NEVER_INLINE
void HashCollection::throwReserveTooLarge() {
  assertx(getClassName().size() == 6);
  auto clsName = getClassName().get()->slice();
  String msg(80, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(buf.data(), buf.size() + 1,
    "%s does not support reserving room for more than %u elements",
    clsName.data() + 3, // strip "HH\" prefix
    MaxReserveSize
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

NEVER_INLINE
int32_t* HashCollection::warnUnbalanced(size_t n, int32_t* ei) const {
  if (n > size_t(Cfg::Server::MaxArrayChain)) {
    raise_error("%s is too unbalanced (%lu)",
                getClassName().data() + 3, // strip "HH\" prefix
                n);
  }
  return ei;
}

Array HashCollection::toKeysArray() {
  VecInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      ai.append(int64_t{e->ikey});
    } else {
      assertx(e->hasStrKey());
      ai.append(VarNR(e->skey).tv());
    }
  }
  return ai.toArray();
}

Array HashCollection::toValuesArray() {
  if (!m_size) return empty_vec_array();
  return Array{arrayData()}.toVec();
}

void HashCollection::remove(int64_t key) {
  auto p = findForRemove(key, hash_int64(key));
  if (p.valid()) {
    mutate();
    erase(p);
  }
}

void HashCollection::remove(StringData* key) {
  auto p = findForRemove(key, key->hash());
  if (p.valid()) {
    mutate();
    erase(p);
  }
}

void HashCollection::eraseNoCompact(VanillaDict::RemovePos pos) {
  assertx(canMutateBuffer());
  assertx(pos.valid() && !isTombstone(pos.elmIdx));
  assertx(m_size > 0);
  arrayData()->eraseNoCompact(pos);
  --m_size;
}

NEVER_INLINE
void HashCollection::makeRoom() {
  assertx(isFull());
  assertx(!isDensityTooLow());
  if (UNLIKELY(cap() == MaxSize)) throwTooLarge();
  assertx(scale() > 0);
  grow(scale() * 2);
  assertx(canMutateBuffer());
  assertx(!isFull());
}

NEVER_INLINE
void HashCollection::reserve(int64_t sz) {
  assertx(m_size <= posLimit() && posLimit() <= cap());
  auto cap = static_cast<int64_t>(this->cap());
  if (LIKELY(sz > cap)) {
    if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
      throwReserveTooLarge();
    }
    // Fast path: The requested capacity is greater than the current capacity.
    // Grow to the smallest allowed capacity that is sufficient.
    grow(VanillaDict::computeScaleFromSize(sz));
    assertx(canMutateBuffer());
    return;
  }
  if (LIKELY(!hasTombstones())) {
    // Fast path: There are no tombstones and the requested capacity is less
    // than or equal to the current capacity.
    mutate();
    return;
  }
  if (sz + int64_t(posLimit() - m_size) <= cap || isDensityTooLow()) {
    // If we reach this case, then either (1) density is too low (this is
    // possible because of methods like retain()), in which case we compact
    // to make room and return, OR (2) density is not too low and either
    // sz < m_size or there's enough room to add sz-m_size elements, in
    // which case we do nothing and return.
    compactOrShrinkIfDensityTooLow();
    assertx(sz + int64_t(posLimit() - m_size) <= cap);
    mutate();
    return;
  }
  // If we reach this case, then density is not too low and sz > m_size and
  // there is not enough room to add sz-m_size elements. While would could
  // compact to make room, it's better for Hysteresis if we grow capacity
  // by 2x instead.
  assertx(!isDensityTooLow());
  assertx(sz + int64_t(posLimit() - m_size) > cap);
  assertx(cap < MaxSize && tableMask() != 0);
  auto newScale = scale() * 2;
  assertx(sz > 0 && VanillaDict::Capacity(newScale) >= sz);
  grow(newScale);
  assertx(canMutateBuffer());
}

ALWAYS_INLINE
void HashCollection::resizeHelper(uint32_t newCap) {
  assertx(newCap >= m_size);
  assertx(m_immCopy.isNull());
  // Allocate a new ArrayData with the specified capacity and dup
  // all the elements (without copying over tombstones).
  auto ad = arrayData()->isStatic() && arrayData()->empty() ?
    VanillaDict::as(VanillaDict::MakeReserveDict(newCap)) :
    VanillaDict::CopyReserve(arrayData(), newCap);
  decRefArr(arrayData());
  setArrayData(ad);
  assertx(canMutateBuffer());
}

void HashCollection::grow(uint32_t newScale) {
  auto newCap = VanillaDict::Capacity(newScale);
  assertx(m_size <= posLimit() && posLimit() <= cap() && cap() <= newCap);
  assertx(SmallSize <= newCap && newCap <= MaxSize);
  assertx(m_size <= newCap);
  auto oldAd = arrayData();
  dropImmCopy();
  if (m_size > 0 && !oldAd->cowCheck()) {
    setArrayData(VanillaDict::Grow(oldAd, newScale, false));
    decRefArr(oldAd);
  } else {
    // For cases where m_size is zero or the buffer's refcount is
    // greater than 1, call resizeHelper().
    resizeHelper(newCap);
  }
  assertx(canMutateBuffer());
}

void HashCollection::compact() {
  assertx(isDensityTooLow());
  dropImmCopy();
  if (!arrayData()->cowCheck()) {
    // VanillaDict::compact can only handle cases where the buffer's
    // refcount is 1.
    arrayData()->compact();
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper().
    resizeHelper(cap());
  }
  assertx(canMutateBuffer());
  assertx(!isDensityTooLow());
}

void HashCollection::shrink(uint32_t oldCap /* = 0 */) {
  assertx(isCapacityTooHigh() && (oldCap == 0 || oldCap < cap()));
  assertx(m_size <= posLimit() && posLimit() <= cap());
  dropImmCopy();
  uint32_t newCap;
  if (oldCap != 0) {
    // If an old capacity was specified, use that
    newCap = oldCap;
    // .. unless the old capacity is too small, in which case we use the
    // smallest capacity that is large enough to hold the current number
    // of elements.
    for (; newCap < m_size; newCap <<= 1) {}
    assertx(newCap == computeMaxElms(folly::nextPowTwo<uint64_t>(newCap) - 1));
  } else {
    // If no old capacity was provided, we compute the largest capacity
    // where m_size/cap() is less than or equal to 0.5 for good hysteresis
    size_t doubleSz = size_t(m_size) * 2;
    uint32_t capThreshold = (doubleSz < size_t(MaxSize)) ? doubleSz : MaxSize;
    for (newCap = SmallSize * 2; newCap < capThreshold; newCap <<= 1) {}
  }
  assertx(SmallSize <= newCap && newCap <= MaxSize);
  assertx(m_size <= newCap);
  auto* oldAd = arrayData();
  if (!oldAd->cowCheck()) {
    // If the buffer's refcount is 1, we can teleport the elements
    // to a new buffer
    auto oldBuf = data();
    auto oldUsed = posLimit();
    auto oldNextKI = nextKI();
    auto const arr = VanillaDict::as(VanillaDict::MakeReserveDict(newCap));
    setArrayData(arr);
    arr->m_size = m_size;
    arr->mutableKeyTypes()->copyFrom(oldAd->keyTypes(), /*compact=*/true);
    auto data = this->data();
    auto table = hashTab();
    auto table_mask = tableMask();
    setPosLimit(m_size);
    setNextKI(oldNextKI);
    for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
      frPos = skipTombstonesNoBoundsCheck(frPos, oldUsed, oldBuf);
      copyElm(oldBuf[frPos], data[toPos]);
      *findForNewInsert(table, table_mask, data[toPos].probe()) = toPos;
    }
    oldAd->setZombie();
    decRefArr(oldAd);
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper()
    resizeHelper(newCap);
  }
  assertx(canMutateBuffer());
  assertx(!isCapacityTooHigh() || newCap == oldCap);
}

HashCollection::Elm& HashCollection::allocElmFront(VanillaDict::Inserter ei) {
  assertx(VanillaDict::isValidIns(ei) && !VanillaDict::isValidPos(*ei));
  assertx(m_size <= posLimit() && posLimit() < cap());
  // Move the existing elements to make element slot 0 available.
  memmove(data() + 1, data(), posLimit() * sizeof(Elm));
  incPosLimit();
  // Update the hashtable to reflect the fact that everything was
  // moved over one position
  auto* hash = hashTab();
  auto* hashEnd = hash + arrayData()->hashSize();
  for (; hash != hashEnd; ++hash) {
    if (validPos(*hash)) {
      ++(*hash);
    }
  }
  // Set the hash entry we found to point to element slot 0.
  (*ei) = 0;
  // Adjust size to reflect that we're adding a new element.
  incSize();
  // Store the value into element slot 0.
  return data()[0];
}

void HashCollection::mutateImpl() {
  assertx(arrayData()->hasMultipleRefs());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  auto* oldAd = arrayData();
  setArrayData(VanillaDict::as(VanillaDict::Copy(oldAd)));
  assertx(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

/////////////////////////////////////////////////////////////////////////////
}
