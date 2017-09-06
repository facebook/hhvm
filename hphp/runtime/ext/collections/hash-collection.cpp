#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/text-util.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// HashCollection

HashCollection::HashCollection(Class* cls, HeaderKind kind, uint32_t cap)
  : ObjectData(cls, NoInit{}, collections::objectFlags, kind)
  , m_versionAndSize(0)
  , m_arr(cap == 0 ? staticEmptyDictArrayAsMixed() :
          MixedArray::asMixed(MixedArray::MakeReserveDict(cap)))
{}

NEVER_INLINE
void HashCollection::throwTooLarge() {
  assert(getClassName().size() == 6);
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
  assert(getClassName().size() == 6);
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
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("%s is too unbalanced (%lu)",
                getClassName().data() + 3, // strip "HH\" prefix
                n);
  }
  return ei;
}

NEVER_INLINE
void HashCollection::warnOnStrIntDup() const {
  req::hash_set<int64_t> seenVals;

  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    int64_t newVal = 0;

    if (e->hasIntKey()) {
      newVal = e->ikey;
    } else {
      assert(e->hasStrKey());
      // isStriclyInteger() puts the int value in newVal as a side effect.
      if (!e->skey->isStrictlyInteger(newVal)) continue;
    }

    if (seenVals.find(newVal) != seenVals.end()) {
      auto cls = getVMClass()->name()->toCppString();
      auto pos = cls.rfind('\\');
      if (pos != std::string::npos) {
        cls = cls.substr(pos + 1);
      }
      raise_warning(
        "%s::toArray() for a %s containing both int(%" PRId64 ") "
        "and string('%" PRId64 "')",
        cls.c_str(),
        toLower(cls).c_str(),
        newVal,
        newVal
      );

      return;
    }

    seenVals.insert(newVal);
  }
  // Do nothing if no 'duplicates' were found.
}

Array HashCollection::toArray() {
  if (!m_size) {
    return empty_array();
  }
  auto ad = arrayData()->toPHPArray(true);
  if (UNLIKELY(ad->size() < m_size)) warnOnStrIntDup();
  assert(m_size);
  assert(ad->m_pos == 0);
  return Array::attach(ad);
}

Array HashCollection::toKeysArray() {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      ai.append(int64_t{e->ikey});
    } else {
      assert(e->hasStrKey());
      ai.append(VarNR(e->skey).tv());
    }
  }
  return ai.toArray();
}

Array HashCollection::toValuesArray() {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    ai.append(tvAsCVarRef(&e->data));
  }
  return ai.toArray();
}

void HashCollection::remove(int64_t key) {
  mutateAndBump();
  auto p = findForRemove(key, hash_int64(key));
  if (validPos(p)) {
    erase(p);
  }
}

void HashCollection::remove(StringData* key) {
  mutateAndBump();
  auto p = findForRemove(key, key->hash());
  if (validPos(p)) {
    erase(p);
  }
}

void HashCollection::eraseNoCompact(ssize_t pos) {
  assert(canMutateBuffer());
  assert(validPos(pos) && !isTombstone(pos));
  assert(m_size > 0);
  arrayData()->eraseNoCompact(pos);
  --m_size;
}

NEVER_INLINE
void HashCollection::makeRoom() {
  assert(isFull());
  assert(posLimit() == cap());
  if (LIKELY(!isDensityTooLow())) {
    if (UNLIKELY(cap() == MaxSize)) {
      throwTooLarge();
    }
    assertx(scale() > 0);
    grow(scale() * 2);
  } else {
    compact();
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isFull());
}

NEVER_INLINE
void HashCollection::reserve(int64_t sz) {
  assert(m_size <= posLimit() && posLimit() <= cap());
  auto cap = static_cast<int64_t>(this->cap());
  if (LIKELY(sz > cap)) {
    if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
      throwReserveTooLarge();
    }
    // Fast path: The requested capacity is greater than the current capacity.
    // Grow to the smallest allowed capacity that is sufficient.
    grow(MixedArray::computeScaleFromSize(sz));
    assert(canMutateBuffer());
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
    assert(sz + int64_t(posLimit() - m_size) <= cap);
    mutate();
    return;
  }
  // If we reach this case, then density is not too low and sz > m_size and
  // there is not enough room to add sz-m_size elements. While would could
  // compact to make room, it's better for Hysteresis if we grow capacity
  // by 2x instead.
  assert(!isDensityTooLow());
  assert(sz + int64_t(posLimit() - m_size) > cap);
  assert(cap < MaxSize && tableMask() != 0);
  auto newScale = scale() * 2;
  assert(sz > 0 && MixedArray::Capacity(newScale) >= sz);
  grow(newScale);
  assert(canMutateBuffer());
}

ALWAYS_INLINE
void HashCollection::resizeHelper(uint32_t newCap) {
  assert(newCap >= m_size);
  assert(m_immCopy.isNull());
  // Allocate a new ArrayData with the specified capacity and dup
  // all the elements (without copying over tombstones).
  auto ad = arrayData() == staticEmptyDictArrayAsMixed() ?
    MixedArray::asMixed(MixedArray::MakeReserveDict(newCap)) :
    MixedArray::CopyReserve(m_arr, newCap);
  decRefArr(m_arr);
  m_arr = ad;
  assert(canMutateBuffer());
}

void HashCollection::grow(uint32_t newScale) {
  auto newCap = MixedArray::Capacity(newScale);
  assert(m_size <= posLimit() && posLimit() <= cap() && cap() <= newCap);
  assert(SmallSize <= newCap && newCap <= MaxSize);
  assert(m_size <= newCap);
  auto oldAd = arrayData();
  dropImmCopy();
  if (m_size > 0 && !oldAd->cowCheck()) {
    m_arr = MixedArray::Grow(oldAd, newScale, false);
    decRefArr(oldAd);
  } else {
    // For cases where m_size is zero or the buffer's refcount is
    // greater than 1, call resizeHelper().
    resizeHelper(newCap);
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
}

void HashCollection::compact() {
  assert(isDensityTooLow());
  dropImmCopy();
  if (!arrayData()->cowCheck()) {
    // MixedArray::compact can only handle cases where the buffer's
    // refcount is 1.
    arrayData()->compact(false);
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper().
    resizeHelper(cap());
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isDensityTooLow());
}

void HashCollection::shrink(uint32_t oldCap /* = 0 */) {
  assert(isCapacityTooHigh() && (oldCap == 0 || oldCap < cap()));
  assert(m_size <= posLimit() && posLimit() <= cap());
  dropImmCopy();
  uint32_t newCap;
  if (oldCap != 0) {
    // If an old capacity was specified, use that
    newCap = oldCap;
    // .. unless the old capacity is too small, in which case we use the
    // smallest capacity that is large enough to hold the current number
    // of elements.
    for (; newCap < m_size; newCap <<= 1) {}
    assert(newCap == computeMaxElms(folly::nextPowTwo<uint64_t>(newCap) - 1));
  } else {
    if (m_size == 0 && nextKI() == 0) {
      decRefArr(m_arr);
      m_arr = staticEmptyDictArrayAsMixed();
      return;
    }
    // If no old capacity was provided, we compute the largest capacity
    // where m_size/cap() is less than or equal to 0.5 for good hysteresis
    size_t doubleSz = size_t(m_size) * 2;
    uint32_t capThreshold = (doubleSz < size_t(MaxSize)) ? doubleSz : MaxSize;
    for (newCap = SmallSize * 2; newCap < capThreshold; newCap <<= 1) {}
  }
  assert(SmallSize <= newCap && newCap <= MaxSize);
  assert(m_size <= newCap);
  auto* oldAd = arrayData();
  if (!oldAd->cowCheck()) {
    // If the buffer's refcount is 1, we can teleport the elements
    // to a new buffer
    auto oldBuf = data();
    auto oldUsed = posLimit();
    auto oldNextKI = nextKI();
    m_arr = MixedArray::asMixed(MixedArray::MakeReserveDict(newCap));
    m_arr->m_size = m_size;
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
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isCapacityTooHigh() || newCap == oldCap);
}

HashCollection::Elm& HashCollection::allocElmFront(MixedArray::Inserter ei) {
  assert(MixedArray::isValidIns(ei) && !MixedArray::isValidPos(*ei));
  assert(m_size <= posLimit() && posLimit() < cap());
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
  // Adjust m_pos so that is points at this new first element.
  arrayData()->m_pos = 0;
  // Adjust size to reflect that we're adding a new element.
  incSize();
  // Store the value into element slot 0.
  return data()[0];
}

/**
 * preSort() does an initial pass to do some preparatory work before the
 * sort algorithm runs. For sorts that use builtin comparators, the types
 * of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized
 * comparator and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
SortFlavor HashCollection::preSort(const AccessorT& acc, bool checkTypes) {
  assert(m_size > 0);
  if (!checkTypes && !hasTombstones()) {
    // No need to loop over the elements, we're done
    return GenericSort;
  }
  auto* start = data();
  auto* end = data() + posLimit();
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (!isTombstone(start)) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (!isTombstone(start)) {
        ++start;
        if (start == end) {
          goto done;
        }
      }
    }
    --end;
    if (start == end) {
      goto done;
    }
    while (isTombstone(end)) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    copyElm(*end, *start);
  }
  done:
  setPosLimit(start - data());
  // The logic above possibly moved elements and tombstones around
  // within the buffer, so we make sure m_pos is not pointing at
  // garbage by resetting it. The logic above ensures that the first
  // slot is not a tombstone, so it's safe to set m_pos to 0.
  arrayData()->m_pos = 0;
  assert(!hasTombstones());
  if (checkTypes) {
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  } else {
    return GenericSort;
  }
}

/**
 * postSort() runs after the sort has been performed. For c_Map,
 * postSort() handles rebuilding the hash.
 */
void HashCollection::postSort() {  // Must provide the nothrow guarantee
  arrayData()->postSort(false);
}

#define SORT_CASE(flag, cmp_type, acc_type)                     \
  case flag: {                                                  \
    if (ascending) {                                            \
      cmp_type##Compare<acc_type, flag, true> comp;             \
      HPHP::Sort::sort(data(), data() + m_size, comp);          \
    } else {                                                    \
      cmp_type##Compare<acc_type, flag, false> comp;            \
      HPHP::Sort::sort(data(), data() + m_size, comp);          \
    }                                                           \
    break;                                                      \
  }
#define SORT_CASE_BLOCK(cmp_type, acc_type)                     \
  switch (sort_flags) {                                         \
    default: /* fall through to SORT_REGULAR case */            \
      SORT_CASE(SORT_REGULAR, cmp_type, acc_type)               \
        SORT_CASE(SORT_NUMERIC, cmp_type, acc_type)             \
        SORT_CASE(SORT_STRING, cmp_type, acc_type)              \
        SORT_CASE(SORT_LOCALE_STRING, cmp_type, acc_type)       \
        SORT_CASE(SORT_NATURAL, cmp_type, acc_type)             \
        SORT_CASE(SORT_NATURAL_CASE, cmp_type, acc_type)        \
        }
#define CALL_SORT(acc_type)                     \
  if (flav == StringSort) {                     \
    SORT_CASE_BLOCK(StrElm, acc_type)           \
      } else if (flav == IntegerSort) {         \
    SORT_CASE_BLOCK(IntElm, acc_type)           \
      } else {                                  \
    SORT_CASE_BLOCK(Elm, acc_type)              \
      }
#define SORT_BODY(acc_type)                                     \
  do {                                                          \
    SortFlavor flav = preSort<acc_type>(acc_type(), true);      \
    try {                                                       \
      CALL_SORT(acc_type);                                      \
    } catch (...) {                                             \
      /* make sure the map is left in a consistent state */     \
      postSort();                                               \
      throw;                                                    \
    }                                                           \
    postSort();                                                 \
  } while(0)

void HashCollection::asort(int sort_flags, bool ascending) {
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(AssocValAccessor<HashCollection::Elm>);
}

void HashCollection::ksort(int sort_flags, bool ascending) {
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(AssocKeyAccessor<HashCollection::Elm>);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type)                                \
  do {                                                          \
    CallCtx ctx;                                                \
    CallerFrame cf;                                             \
    vm_decode_function(cmp_function, cf(), false, ctx);         \
    if (!ctx.func) {                                            \
      return false;                                             \
    }                                                           \
    preSort<acc_type>(acc_type(), false);                       \
    SCOPE_EXIT {                                                \
      /* make sure the map is left in a consistent state */     \
      postSort();                                               \
    };                                                          \
    ElmUCompare<acc_type> comp;                                 \
    comp.ctx = &ctx;                                            \
    HPHP::Sort::sort(data(), data() + m_size, comp);            \
    return true;                                                \
  } while (0)

bool HashCollection::uasort(const Variant& cmp_function) {
  if (m_size <= 1) return true;
  mutateAndBump();
  USER_SORT_BODY(AssocValAccessor<HashCollection::Elm>);
}

bool HashCollection::uksort(const Variant& cmp_function) {
  if (m_size <= 1) return true;
  mutateAndBump();
  USER_SORT_BODY(AssocKeyAccessor<HashCollection::Elm>);
}

#undef USER_SORT_BODY

void HashCollection::mutateImpl() {
  assert(arrayData()->hasMultipleRefs());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  auto* oldAd = arrayData();
  m_arr = MixedArray::asMixed(MixedArray::Copy(oldAd));
  assert(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

/////////////////////////////////////////////////////////////////////////////
}
