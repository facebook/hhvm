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
#include "hphp/runtime/vm/iter.h"

#include <algorithm>

#include <folly/Likely.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

// We don't want JIT iterators to take up too much space on the stack.
static_assert(sizeof(IterImpl) == 32, "");
static_assert(sizeof(Iter) == 32, "");

//////////////////////////////////////////////////////////////////////

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

//////////////////////////////////////////////////////////////////////

std::string show(IterSpecialization type) {
  auto const base_const = type.base_const ? "BaseConst" : "BaseMutable";
  auto const base_type  = show((IterSpecialization::BaseType)type.base_type);
  if (type.output_key) {
    auto const key_types  = show((IterSpecialization::KeyTypes)type.key_types);
    return folly::format("{}::{}::{}", base_type, base_const, key_types).str();
  } else {
    return folly::format("{}::{}", base_type, base_const).str();
  }
}

std::string show(IterSpecialization::BaseType type) {
  switch (type) {
    case IterSpecialization::Packed:        return "Packed";
    case IterSpecialization::Mixed:         return "Mixed";
    case IterSpecialization::Vec:           return "Vec";
    case IterSpecialization::Dict:          return "Dict";
    case IterSpecialization::kNumBaseTypes: always_assert(false);
  }
  always_assert(false);
}

std::string show(IterSpecialization::KeyTypes type) {
  switch (type) {
    case IterSpecialization::ArrayKey:     return "ArrayKey";
    case IterSpecialization::Int:          return "Int";
    case IterSpecialization::Str:          return "Str";
    case IterSpecialization::StaticStr:    return "StaticStr";
    case IterSpecialization::kNumKeyTypes: always_assert(false);
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

IterImpl::IterImpl(const ArrayData* data) {
  arrInit(data);
}

IterImpl::IterImpl(ObjectData* obj) {
  objInit<true>(obj);
}

IterImpl::IterImpl(ObjectData* obj, NoInc) {
  objInit<false>(obj);
}

bool IterImpl::checkInvariants(const ArrayData* ad /* = nullptr */) const {
  TRACE(3, "IterImpl::checkInvariants: %lx %lx %lx %lx (ad = %lx)\n",
        uintptr_t(m_data), size_t(m_typeFields), m_pos, m_end, uintptr_t(ad));

  // We can't make many assertions for iterators over objects.
  if (m_itype == IterImpl::TypeIterator) {
    assertx(ad == nullptr);
    assertx(!hasArrayData());
    assertx(m_nextHelperIdx == IterNextIndex::Object);
    return true;
  }

  // Exactly one of the ArrayData pointers {ad, m_data} should be nullptr.
  assertx(m_itype == IterImpl::TypeArray);
  assertx((ad == nullptr) != (m_data == nullptr));
  DEBUG_ONLY auto const arr = ad ? ad : m_data;

  // Check that array's vtable index is compatible with the array's layout.
  if (m_nextHelperIdx == IterNextIndex::ArrayPacked ||
      m_nextHelperIdx == IterNextIndex::ArrayPackedPointer) {
    assertx(arr->hasVanillaPackedLayout());
  } else if (m_nextHelperIdx == IterNextIndex::ArrayMixed) {
    assertx(arr->hasVanillaMixedLayout());
  } else if (m_nextHelperIdx == IterNextIndex::ArrayMixedPointer) {
    assertx(arr->hasVanillaMixedLayout());
    assertx(arr->getSize() == MixedArray::asMixed(arr)->iterLimit());
  } else {
    // We'd like to assert the converse, too: a packed or mixed array should
    // a next helper that makes use of its layout. However, this condition
    // can fail for local iters: e.g. an APC array base can be promoted to a
    // packed or mixed array, with iteration still using array-generic code.
    // We check it for non-local iters.
    assertx(m_nextHelperIdx == IterNextIndex::Array);
    if (m_data != nullptr) {
      assertx(!m_data->hasVanillaPackedLayout());
      assertx(!m_data->hasVanillaMixedLayout());
    }
  }

  // Check the consistency of the pos and end fields.
  if (m_nextHelperIdx == IterNextIndex::ArrayPackedPointer) {
    assertx(m_packed_elm < m_packed_end);
    assertx(m_packed_end == packedData(arr) + arr->getSize());
  } else if (m_nextHelperIdx == IterNextIndex::ArrayMixedPointer) {
    assertx(m_mixed_elm < m_mixed_end);
    assertx(m_mixed_end == MixedArray::asMixed(arr)->data() + arr->getSize());
  } else {
    assertx(m_pos < m_end);
    // RecordArray::IterEnd logs a notice, so avoid it in assertions.
    DEBUG_ONLY auto const end =
      arr->isRecordArrayKind() ? arr->getSize() : arr->iter_end();
    assertx(m_end == end);
  }
  return true;
}

template <bool incRef /* = true */>
void IterImpl::arrInit(const ArrayData* arr) {
  setArrayData(arr);
  if (arr && incRef) arr->incRefCount();
}

template <bool incRef>
void IterImpl::objInit(ObjectData* obj) {
  assertx(obj);

  if (LIKELY(obj->isCollection())) {
    if (auto ad = collections::asArray(obj)) {
      ad->incRefCount();
      if (!incRef) decRefObj(obj);
      setArrayData(ad);
    } else {
      assertx(obj->collectionType() == CollectionType::Pair);
      auto arr = collections::toArray(obj);
      if (!incRef) decRefObj(obj);
      setArrayData(arr.detach());
    }
    return;
  }

  assertx(obj->instanceof(SystemLib::s_HH_IteratorClass));
  setObject(obj);
  if (incRef) obj->incRefCount();
  try {
    obj->o_invoke_few_args(s_rewind, 0);
  } catch (...) {
    // Regardless of whether the incRef template parameter is true or false,
    // at this point, this IterImpl "owns" a reference to the object and is
    // responsible for dec-ref-ing it when the iterator is destroyed.
    //
    // Normally, the destructor takes care of this case, but we'll never invoke
    // it if the exception is thrown before the constructor finishes, so we
    // must manually dec-ref the object here.
    decRefObj(obj);
    kill();
    throw;
  }
}

void IterImpl::kill() {
  if (!debug) return;
  // IterImpl is not POD, so we memset each POD field separately.
  memset(&m_data, kIterTrashFill, sizeof(m_data));
  memset(&m_typeFields, kIterTrashFill, sizeof(m_typeFields));
  memset(&m_pos, kIterTrashFill, sizeof(m_pos));
  memset(&m_end, kIterTrashFill, sizeof(m_end));
}

IterImpl::~IterImpl() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    kill();
    return;
  }
  ObjectData* obj = getObject();
  assertx(obj);
  decRefObj(obj);
  kill();
}

bool IterImpl::endHelper() const  {
  auto obj = getObject();
  return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
}

void IterImpl::nextHelper() {
  auto obj = getObject();
  obj->o_invoke_few_args(s_next, 0);
}

Variant IterImpl::firstHelper() {
  auto obj = getObject();
  return obj->o_invoke_few_args(s_key, 0);
}

Variant IterImpl::second() {
  if (LIKELY(hasArrayData())) return getArrayData()->getValue(m_pos);
  return getObject()->o_invoke_few_args(s_current, 0);
}

TypedValue IterImpl::secondVal() const {
  if (LIKELY(hasArrayData())) return getArrayData()->nvGetVal(m_pos);
  raise_fatal_error("taking reference on iterator objects");
}

//////////////////////////////////////////////////////////////////////

bool Iter::init(TypedValue* base) {
  // Get easy cases out of the way. Class methods promote to arrays and both
  // of them are just an IterImpl contructor. end() never throws for arrays.
  if (isClsMethType(base->m_type)) {
    new (&m_iter) IterImpl(tvAsVariant(base).toArray().get());
    return !m_iter.end();
  }
  if (isArrayLikeType(base->m_type)) {
    new (&m_iter) IterImpl(base->m_data.parr);
    return !m_iter.end();
  }

  // Get more easy cases out of the way: non-objects are not iterable.
  // For these cases, we warn and branch to done.
  if (!isObjectType(base->m_type)) {
    raise_warning("Invalid argument supplied for foreach()");
    return false;
  }

  if (base->m_data.pobj->isCollection()) {
    new (&m_iter) IterImpl(base->m_data.pobj);
  } else {
    bool isIterator;
    Object obj = base->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      new (&m_iter) IterImpl(obj.detach(), IterImpl::noInc);
    } else {
      Class* ctx = arGetContextClass(vmfp());
      auto ctxStr = ctx ? ctx->nameStr() : StrNR();
      Array iterArray(obj->o_toIterArray(ctxStr));
      ArrayData* ad = iterArray.get();
      new (&m_iter) IterImpl(ad);
    }
  }

  // If the object was empty, or if end throws, dec-ref it and branch to done.
  try {
    if (m_iter.end()) {
      m_iter.~IterImpl();
      return false;
    }
  } catch (...) {
    m_iter.~IterImpl();
    throw;
  }
  return true;
}

bool Iter::next() {
  // The emitter should never generate bytecode where the iterator is at the
  // end before IterNext is executed. checkInvariants tests this invariant.
  assertx(m_iter.checkInvariants());
  assertx(m_iter.getHelperIndex() != IterNextIndex::ArrayPackedPointer);
  assertx(m_iter.getHelperIndex() != IterNextIndex::ArrayMixedPointer);
  m_iter.next();
  // If the iterator is now at the end, dec-ref the base. (For local iterators,
  // m_data is null, so the destructor won't free it.)
  if (m_iter.end()) {
    m_iter.~IterImpl();
    return false;
  }
  return true;
}

void Iter::free() {
  // We can't make any assertions about other iterator fields here, because we
  // want memory effects to know that IterFree only reads the base (so we can
  // eliminate storing the rest of the fields in most cases).
  m_iter.~IterImpl();
}

std::string Iter::toString() const {
  switch (m_iter.getIterType()) {
    case IterImpl::TypeArray:
      return "I:Array";
    case IterImpl::TypeIterator:
      return "I:Iterator";
    default:
      return "I:Trash";
  }
}

/*
 * For the specialized iterator helpers below, we need to peek into the raw
 * IterImpl to manipulate its fields. We could make all of these methods
 * friends of the Iter struct, but that involves listing them in the class.
 *
 * To give us more flexibility to modify these helpers, we instead create this
 * method that exposes the underlying IterImpl by casting.
 */
IterImpl* unwrap(Iter* iter) { return &iter->m_iter; }

namespace {

/*
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */

template <bool typeArray>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  auto& arrIter = *unwrap(iter);
  assertx((typeArray && arrIter.getIterType() == IterImpl::TypeArray) ||
         (!typeArray && arrIter.getIterType() == IterImpl::TypeIterator));
  if (typeArray) {
    tvDup(arrIter.nvSecond(), *out);
  } else {
    Variant val = arrIter.second();
    tvDup(*val.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

template <bool typeArray>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  auto& arrIter = *unwrap(iter);
  assertx((typeArray && arrIter.getIterType() == IterImpl::TypeArray) ||
         (!typeArray && arrIter.getIterType() == IterImpl::TypeIterator));
  if (typeArray) {
    tvDup(arrIter.nvFirst(), *out);
  } else {
    Variant key = arrIter.first();
    tvDup(*key.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

inline void liter_value_cell_local_impl(Iter* iter,
                                        TypedValue* out,
                                        const ArrayData* ad) {
  auto const oldVal = *out;
  auto const& arrIter = *unwrap(iter);
  assertx(arrIter.getIterType() == IterImpl::TypeArray);
  assertx(!arrIter.getArrayData());
  tvDup(arrIter.nvSecondLocal(ad), *out);
  tvDecRefGen(oldVal);
}

inline void liter_key_cell_local_impl(Iter* iter,
                                      TypedValue* out,
                                      const ArrayData* ad) {
  auto const oldVal = *out;
  auto const& arrIter = *unwrap(iter);
  assertx(arrIter.getIterType() == IterImpl::TypeArray);
  assertx(!arrIter.getArrayData());
  tvDup(arrIter.nvFirstLocal(ad), *out);
  tvDecRefGen(oldVal);
}

}

// These release methods are called by the iter_next_* implementations below
// that know the particular layout of the array they're iterating over. We pull
// them out into a separate method here so that a) we can inline the destructor
// for each array type into these methods and b) to make the call a tail call.
//
// These methods all return false (= 0) to signify that iteration is over.

NEVER_INLINE int64_t iter_next_free_packed(Iter* iter, ArrayData* arr) {
  assertx(arr->decWillRelease());
  assertx(arr->hasVanillaPackedLayout());
  PackedArray::Release(arr);
  iter->kill();
  return 0;
}

NEVER_INLINE int64_t iter_next_free_mixed(Iter* iter, ArrayData* arr) {
  assertx(arr->hasVanillaMixedLayout());
  assertx(arr->decWillRelease());
  MixedArray::Release(arr);
  iter->kill();
  return 0;
}

NEVER_INLINE int64_t iter_next_free_apc(Iter* iter, APCLocalArray* arr) {
  assertx(arr->decWillRelease());
  APCLocalArray::Release(arr->asArrayData());
  iter->kill();
  return 0;
}

/*
 * new_iter_array creates an iterator for the specified array iff the
 * array is not empty.  If new_iter_array creates an iterator, it does
 * not increment the refcount of the specified array.  If
 * new_iter_array does not create an iterator, it decRefs the array.
 */
template <bool Local>
NEVER_INLINE
int64_t new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    auto const iter = unwrap(dest);
    if (Local) {
      new (iter) IterImpl(arr, IterImpl::local);
      liter_value_cell_local_impl(dest, valOut, arr);
      if (keyOut) {
        liter_key_cell_local_impl(dest, keyOut, arr);
      }
    } else {
      new (iter) IterImpl(arr, IterImpl::noInc);
      iter_value_cell_local_impl<true>(dest, valOut);
      if (keyOut) {
        iter_key_cell_local_impl<true>(dest, keyOut);
      }
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  if (!Local) decRefArr(arr);
  return 0LL;
}

template <IterTypeOp Type>
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  auto constexpr BaseConst = Type != IterTypeOp::LocalBaseMutable;
  auto constexpr Local     = Type != IterTypeOp::NonLocal;

  auto const size = ad->getSize();
  if (UNLIKELY(size == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasVanillaPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasVanillaMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    }
    dest->kill();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<Local>(dest, ad, valOut, nullptr);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = *unwrap(dest);
  aiter.m_data = Local ? nullptr : ad;

  if (LIKELY(ad->hasVanillaPackedLayout())) {
    if (BaseConst) {
      aiter.m_packed_elm = packedData(ad);
      aiter.m_packed_end = aiter.m_packed_elm + size;
      aiter.setArrayNext(IterNextIndex::ArrayPackedPointer);
    } else {
      aiter.m_pos = 0;
      aiter.m_end = size;
      aiter.setArrayNext(IterNextIndex::ArrayPacked);
    }
    tvDup(PackedArray::GetPosVal(ad, 0), *valOut);
    return 1;
  }

  if (LIKELY(ad->hasVanillaMixedLayout())) {
    auto const mixed = MixedArray::asMixed(ad);
    if (BaseConst && LIKELY(mixed->iterLimit() == size)) {
      aiter.m_mixed_elm = mixed->data();
      aiter.m_mixed_end = aiter.m_mixed_elm + size;
      aiter.setArrayNext(IterNextIndex::ArrayMixedPointer);
      mixed->getArrayElm(0, valOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  return new_iter_array_cold<Local>(dest, ad, valOut, nullptr);
}

IterInitArr new_iter_array_helper(IterTypeOp type) {
  switch (type) {
    case IterTypeOp::LocalBaseConst:
      return new_iter_array<IterTypeOp::LocalBaseConst>;
    case IterTypeOp::LocalBaseMutable:
      return new_iter_array<IterTypeOp::LocalBaseMutable>;
    case IterTypeOp::NonLocal:
      return new_iter_array<IterTypeOp::NonLocal>;
  }
  always_assert(false);
}

template<IterTypeOp Type>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  auto constexpr BaseConst = Type != IterTypeOp::LocalBaseMutable;
  auto constexpr Local     = Type != IterTypeOp::NonLocal;

  auto const size = ad->getSize();
  if (UNLIKELY(size == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasVanillaPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasVanillaMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    }
    dest->kill();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<Local>(
      dest, ad, valOut, keyOut
    );
  }
  if (UNLIKELY(isRefcountedType(keyOut->m_type))) {
    return new_iter_array_cold<Local>(
      dest, ad, valOut, keyOut
    );
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = *unwrap(dest);
  aiter.m_data = Local ? nullptr : ad;

  if (ad->hasVanillaPackedLayout()) {
    aiter.m_pos = 0;
    aiter.m_end = size;
    aiter.setArrayNext(IterNextIndex::ArrayPacked);
    tvDup(PackedArray::GetPosVal(ad, 0), *valOut);
    keyOut->m_type = KindOfInt64;
    keyOut->m_data.num = 0;
    return 1;
  }

  if (ad->hasVanillaMixedLayout()) {
    auto const mixed = MixedArray::asMixed(ad);
    if (BaseConst && LIKELY(mixed->iterLimit() == size)) {
      aiter.m_mixed_elm = mixed->data();
      aiter.m_mixed_end = aiter.m_mixed_elm + size;
      aiter.setArrayNext(IterNextIndex::ArrayMixedPointer);
      mixed->getArrayElm(0, valOut, keyOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut, keyOut);
    return 1;
  }

  return new_iter_array_cold<Local>(dest, ad, valOut, keyOut);
}

IterInitArrKey new_iter_array_key_helper(IterTypeOp type) {
  switch (type) {
    case IterTypeOp::LocalBaseConst:
      return new_iter_array_key<IterTypeOp::LocalBaseConst>;
    case IterTypeOp::LocalBaseMutable:
      return new_iter_array_key<IterTypeOp::LocalBaseMutable>;
    case IterTypeOp::NonLocal:
      return new_iter_array_key<IterTypeOp::NonLocal>;
  }
  always_assert(false);
}

struct FreeObj {
  FreeObj() : m_obj(0) {}
  void operator=(ObjectData* obj) { m_obj = obj; }
  ~FreeObj() { if (UNLIKELY(m_obj != nullptr)) decRefObj(m_obj); }
 private:
  ObjectData* m_obj;
};

/**
 * new_iter_object_any creates an iterator for the specified object if the
 * object is iterable and it is non-empty (has properties). If
 * new_iter_object_any creates an iterator, it does not increment the refcount
 * of the specified object. If new_iter_object does not create an iterator,
 * it decRefs the object.
 *
 * If exceptions are thrown, new_iter_object_any takes care of decRefing the
 * object.
 */
static int64_t new_iter_object_any(Iter* dest, ObjectData* obj, Class* ctx,
                                   TypedValue* valOut, TypedValue* keyOut) {
  IterImpl::Type itType;
  auto const iter = unwrap(dest);
  {
    FreeObj fo;
    if (obj->isIterator()) {
      TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator\n",
            __func__, dest, obj, ctx);
      new (iter) IterImpl(obj, IterImpl::noInc);
      itType = IterImpl::TypeIterator;
    } else {
      bool isIteratorAggregate;
      /*
       * We are not going to transfer ownership of obj to the iterator,
       * so arrange to decRef it later. The actual decRef has to happen
       * after the call to arr().end() below, because both can have visible side
       * effects (calls to valid()). Similarly it has to happen before the
       * iter_*_cell_local_impl calls below, because they call current() and
       * key() (hence the explicit scope around FreeObj fo;)
       */
      fo = obj;

      Object itObj = obj->iterableObject(isIteratorAggregate, false);
      if (isIteratorAggregate) {
        TRACE(2, "%s: I %p, obj %p, ctx %p, IteratorAggregate\n",
              __func__, dest, obj, ctx);
        new (iter) IterImpl(itObj.detach(), IterImpl::noInc);
        itType = IterImpl::TypeIterator;
      } else {
        TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
              __func__, dest, obj, ctx);
        auto ctxStr = ctx ? ctx->nameStr() : StrNR();
        Array iterArray(itObj->o_toIterArray(ctxStr));
        ArrayData* ad = iterArray.get();
        new (iter) IterImpl(ad);
        itType = IterImpl::TypeArray;
      }
    }
    try {
      if (dest->end()) {
        // Iterator was empty; call the destructor on the iterator we just
        // constructed.
        dest->free();
        return 0LL;
      }
    } catch (...) {
      dest->free();
      throw;
    }
  }

  if (itType == IterImpl::TypeIterator) {
    iter_value_cell_local_impl<false>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(dest, keyOut);
    }
  } else {
    iter_value_cell_local_impl<true>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, keyOut);
    }
  }
  return 1LL;
}

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator or Object\n",
        __func__, dest, obj, ctx);
  if (UNLIKELY(!obj->isCollection())) {
    return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  }
  auto constexpr Type = IterTypeOp::NonLocal;

  if (auto ad = collections::asArray(obj)) {
    ad->incRefCount();
    decRefObj(obj);
    return keyOut ? new_iter_array_key<Type>(dest, ad, valOut, keyOut)
                  : new_iter_array<Type>(dest, ad, valOut);
  }

  assertx(obj->collectionType() == CollectionType::Pair);
  auto arr = collections::toArray(obj);
  decRefObj(obj);
  return keyOut ? new_iter_array_key<Type>(dest, arr.detach(), valOut, keyOut)
                : new_iter_array<Type>(dest, arr.detach(), valOut);
}

// Generic next implementation for non-local iterators. This method is used for
// both value and key-value iterators; for value iterators, keyOut is nullptr.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
NEVER_INLINE
int64_t iter_next_cold(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  auto const ai = unwrap(iter);
  assertx(ai->getIterType() == IterImpl::TypeArray ||
          ai->getIterType() == IterImpl::TypeIterator);
  assertx(ai->hasArrayData() || !ai->getObject()->isCollection());
  ai->next();
  if (ai->end()) {
    // The IterImpl destructor will decRef the array
    ai->~IterImpl();
    return 0;
  }
  if (unwrap(iter)->getIterType() == IterImpl::TypeArray) {
    iter_value_cell_local_impl<true>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(iter, keyOut);
    }
  } else {
    iter_value_cell_local_impl<false>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(iter, keyOut);
    }
  }
  return 1;
}

// Generic next implementation for non-local iterators. This method is used for
// both value and key-value iterators; for value iterators, keyOut is nullptr.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
//
// Since local iterators are always over arrays, we take an ArrayData here.
NEVER_INLINE
int64_t liter_next_cold(Iter* iter,
                        const ArrayData* ad,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  auto const ai = unwrap(iter);
  assertx(ai->getIterType() == IterImpl::TypeArray);
  assertx(!ai->getArrayData());
  if (ai->nextLocal(ad)) {
    ai->~IterImpl();
    return 0;
  }
  liter_value_cell_local_impl(iter, valOut, ad);
  if (keyOut) liter_key_cell_local_impl(iter, keyOut, ad);
  return 1;
}

// vtable implementation for APC arrays. This method is quite cold, and we
// might be better off just using the generic implementation instead.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template <bool Local>
NEVER_INLINE
static int64_t iter_next_apc_array(Iter* iter,
                                   TypedValue* valOut,
                                   TypedValue* keyOut,
                                   ArrayData* ad) {
  assertx(ad->kind() == ArrayData::kApcKind);

  auto const arrIter = unwrap(iter);
  auto const arr = APCLocalArray::asApcArray(ad);
  ssize_t const pos = arr->iterAdvanceImpl(arrIter->getPos());
  if (UNLIKELY(pos == arrIter->getEnd())) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_apc(iter, arr);
      }
      arr->decRefCount();
    }
    iter->kill();
    return 0;
  }
  arrIter->setPos(pos);

  auto const val = APCLocalArray::GetPosVal(ad, pos);
  tvSet(val, *valOut);
  if (LIKELY(!keyOut)) return 1;

  auto const key = APCLocalArray::GetPosKey(ad, pos);
  auto const oldKey = *keyOut;
  tvDup(key, *keyOut);
  tvDecRefGen(oldKey);

  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers

namespace {

NEVER_INLINE
int64_t iter_next_cold_inc_val(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(*valOut);
  return iter_next_cold(it, valOut, keyOut);
}

NEVER_INLINE
int64_t liter_next_cold_inc_val(Iter* it,
                                TypedValue* valOut,
                                TypedValue* keyOut,
                                const ArrayData* ad) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(*valOut);
  return liter_next_cold(it, ad, valOut, keyOut);
}

// We call this function if valOut->decWillRelease(). It should be relatively
// cold but it happens if the value is an array which gets COWed in the loop.
// Not releasing the value here can cause a significant memory regression.
//
// This method return trues (= 1) because we only call it after advancing the
// iterator's position and checking that the new position is in bounds.
NEVER_INLINE
int64_t iter_next_packed_pointer_cold(Iter* it,
                                      TypedValue* valOut,
                                      TypedValue* elm) {
  auto const oldVal = *valOut;
  tvDup(*elm, *valOut);
  tvDecRefGen(oldVal);
  return 1;
}

// We call this function if valOut->decWillRelease(). It should be relatively
// cold but it happens if the value is an array which gets COWed in the loop.
// Not releasing the value here can cause a significant memory regression.
//
// This method return trues (= 1) because we only call it after advancing the
// iterator's position and checking that the new position is in bounds.
NEVER_INLINE
int64_t iter_next_mixed_pointer_cold(Iter* it,
                                     TypedValue* valOut,
                                     TypedValue* keyOut,
                                     MixedArrayElm* elm) {
  auto const oldVal = *valOut;
  tvDup(*elm->datatv(), *valOut);
  tvDecRefGen(oldVal);
  if (keyOut != nullptr) {
    auto const oldKey = *keyOut;
    tvDup(elm->getKey(), *keyOut);
    tvDecRefGen(oldKey);
  }
  return 1;
}

// We call this function if keyOut->decWillRelease() and we've dec-ref-ed
// valOut already. It should be *very* cold, because the key is rarely changed.
//
// This method return trues (= 1) because we only call it after advancing the
// iterator's position and checking that the new position is in bounds.
NEVER_INLINE
int64_t iter_next_mixed_pointer_cold_key(Iter* it,
                                         TypedValue* valOut,
                                         TypedValue* keyOut,
                                         MixedArrayElm* elm) {
  tvDup(*elm->datatv(), *valOut);
  if (keyOut != nullptr) {
    auto const oldKey = *keyOut;
    tvDup(elm->getKey(), *keyOut);
    tvDecRefGen(oldKey);
  }
  return 1;
}

}

// "virtual" method implementation of *IterNext* for ArrayPackedPointer.
// We don't use this iteration mode for key-value iterators, so it's simple.
//
// See iter.h for the meaning of a "local" iterator. At this point,
// we have the base, but we only dec-ref it when non-local iters hit the end.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool Local>
ALWAYS_INLINE
int64_t iter_next_packed_pointer(Iter* it, TypedValue* valOut, ArrayData* arr) {
  auto& iter = *unwrap(it);
  auto const elm = iter.m_packed_elm + 1;
  if (elm == iter.m_packed_end) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_packed(it, arr);
      }
      arr->decRefCount();
    }
    iter.kill();
    return 0;
  }
  iter.m_packed_elm = elm;

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return iter_next_packed_pointer_cold(it, valOut, elm);
    }
    valOut->m_data.pcnt->decRefCount();
  }

  tvDup(*elm, *valOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for ArrayMixedPointer.
// Since we know the base is mixed and free of tombstones, we can simply
// increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
// See iter.h for the meaning of a "local" iterator. At this point,
// we have the base, but we only dec-ref it when non-local iters hit the end.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey, bool Local>
ALWAYS_INLINE
int64_t iter_next_mixed_pointer(Iter* it,
                                TypedValue* valOut,
                                TypedValue* keyOut,
                                ArrayData* arr) {
  auto& iter = *unwrap(it);
  auto const elm = iter.m_mixed_elm + 1;
  if (elm == iter.m_mixed_end) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_mixed(it, arr);
      }
      arr->decRefCount();
    }
    iter.kill();
    return 0;
  }
  iter.m_mixed_elm = elm;

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return iter_next_mixed_pointer_cold(it, valOut, keyOut, elm);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return iter_next_mixed_pointer_cold_key(it, valOut, keyOut, elm);
    }
    keyOut->m_data.pcnt->decRefCount();
  }

  tvDup(*elm->datatv(), *valOut);
  if (HasKey) tvDup(elm->getKey(), *keyOut);
  return 1;
}

namespace {

// "virtual" method implementation of *IterNext* for ArrayMixed iterators.
// Since we know the array is mixed, we can do "while (elm[pos].isTombstone())"
// inline here, and we can use MixedArray helpers to extract the key and value.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
// See iter.h for the meaning of a "local" iterator. At this point,
// we have the base, but we only dec-ref it when non-local iters hit the end.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey, bool Local>
ALWAYS_INLINE
int64_t iter_next_mixed_impl(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut,
                             ArrayData* arrData) {
  auto& iter         = *unwrap(it);
  auto const arr     = MixedArray::asMixed(arrData);
  ssize_t pos        = iter.getPos();

  do {
    if ((++pos) == iter.getEnd()) {
      if (!Local) {
        if (UNLIKELY(arr->decWillRelease())) {
          return iter_next_free_mixed(it, arr->asArrayData());
        }
        arr->decRefCount();
      }
      iter.kill();
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold(it, arrData, valOut, keyOut)
        : iter_next_cold(it, valOut, keyOut);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold_inc_val(it, valOut, keyOut, arrData)
        : iter_next_cold_inc_val(it, valOut, keyOut);
    }
    keyOut->m_data.pcnt->decRefCount();
  }

  iter.setPos(pos);
  if (HasKey) {
    arr->getArrayElm(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

// "virtual" method implementation of *IterNext* for ArrayPacked iterators.
// Since we know the array is packed, we just need to increment the position
// and do a bounds check. The key is the position; for the value, we index.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
// See iter.h for the meaning of a "local" iterator. At this point,
// we have the base, but we only dec-ref it when non-local iters hit the end.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey, bool Local>
int64_t iter_next_packed_impl(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut,
                              ArrayData* ad) {
  auto& iter = *unwrap(it);
  assertx(PackedArray::checkInvariants(ad));

  ssize_t pos = iter.getPos() + 1;
  if (LIKELY(pos < iter.getEnd())) {
    if (isRefcountedType(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold(it, ad, valOut, keyOut)
          : iter_next_cold(it, valOut, keyOut);
      }
      valOut->m_data.pcnt->decRefCount();
    }
    if (HasKey && UNLIKELY(isRefcountedType(keyOut->m_type))) {
      if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold_inc_val(it, valOut, keyOut, ad)
          : iter_next_cold_inc_val(it, valOut, keyOut);
      }
      keyOut->m_data.pcnt->decRefCount();
    }
    iter.setPos(pos);
    tvDup(PackedArray::GetPosVal(ad, pos), *valOut);
    if (HasKey) {
      keyOut->m_data.num = pos;
      keyOut->m_type = KindOfInt64;
    }
    return 1;
  }

  // Finished iterating---we need to free the array.
  if (!Local) {
    if (UNLIKELY(ad->decWillRelease())) {
      return iter_next_free_packed(it, ad);
    }
    ad->decRefCount();
  }
  iter.kill();
  return 0;
}

}

int64_t iterNextArrayPacked(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayPacked: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_packed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayPacked(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayPacked: I %p\n", it);
  return iter_next_packed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayPacked(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayPacked: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_packed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayPacked(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut,
                              ArrayData* ad) {
  TRACE(2, "literNextKArrayPacked: I %p\n", it);
  return iter_next_packed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArrayMixed(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayMixed: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_mixed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayMixed(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayMixed: I %p\n", it);
  return iter_next_mixed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayMixed(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayMixed: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_mixed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayMixed(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut,
                             ArrayData* ad) {
  TRACE(2, "literNextKArrayMixed: I %p\n", it);
  return iter_next_mixed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArray(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArray: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  if (ad->isApcArrayKind()) {
    return iter_next_apc_array<false>(it, valOut, nullptr, ad);
  }
  return iter_next_cold(it, valOut, nullptr);
}

int64_t literNextArray(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArray: I %p\n", it);
  if (ad->isApcArrayKind()) {
    return iter_next_apc_array<true>(it, valOut, nullptr, ad);
  }
  return liter_next_cold(it, ad, valOut, nullptr);
}

int64_t iterNextKArray(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iterNextKArray: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  if (ad->isApcArrayKind()) {
    return iter_next_apc_array<false>(it, valOut, keyOut, ad);
  }
  return iter_next_cold(it, valOut, keyOut);
}

int64_t literNextKArray(Iter* it, TypedValue* valOut, TypedValue* keyOut, ArrayData* ad) {
  TRACE(2, "literNextKArray: I %p\n", it);
  if (ad->isApcArrayKind()) {
    return iter_next_apc_array<true>(it, valOut, keyOut, ad);
  }
  return liter_next_cold(it, ad, valOut, keyOut);
}

int64_t iterNextObject(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextObject: I %p\n", it);
  // We can't just put the address of iter_next_cold in the table
  // below right now because we need to get a nullptr into the third
  // argument register for it.
  return iter_next_cold(it, valOut, nullptr);
}

int64_t literNextObject(Iter*, TypedValue*, ArrayData*) {
  always_assert(false);
}
int64_t literNextKObject(Iter*, TypedValue*, TypedValue*, ArrayData*) {
  always_assert(false);
}

int64_t iterNextArrayPackedPointer(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayPackedPointer: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_packed_pointer<false>(it, valOut, ad);
}

int64_t iterNextKArrayPackedPointer(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  always_assert(false);
}

int64_t literNextArrayPackedPointer(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayPackedPointer: I %p\n", it);
  return iter_next_packed_pointer<true>(it, valOut, ad);
}

int64_t literNextKArrayPackedPointer(Iter* it, TypedValue* valOut,
                                     TypedValue* keyOut, ArrayData* ad) {
  always_assert(false);
}

int64_t iterNextArrayMixedPointer(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayMixedPointer: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_mixed_pointer<false, false>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayMixedPointer(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayMixedPointer: I %p\n", it);
  auto const ad = const_cast<ArrayData*>(unwrap(it)->getArrayData());
  return iter_next_mixed_pointer<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextArrayMixedPointer(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayMixedPointer: I %p\n", it);
  return iter_next_mixed_pointer<false, true>(it, valOut, nullptr, ad);
}

int64_t literNextKArrayMixedPointer(Iter* it, TypedValue* valOut,
                                    TypedValue* keyOut, ArrayData* ad) {
  TRACE(2, "literNextKArrayMixedPointer: I %p\n", it);
  return iter_next_mixed_pointer<true, true>(it, valOut, keyOut, ad);
}

using IterNextHelper  = int64_t (*)(Iter*, TypedValue*);
using IterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*);
using LIterNextHelper  = int64_t (*)(Iter*, TypedValue*, ArrayData*);
using LIterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*, ArrayData*);

const IterNextHelper g_iterNextHelpers[] = {
  &iterNextArrayPacked,
  &iterNextArrayMixed,
  &iterNextArray,
  &iterNextObject,
  &iterNextArrayPackedPointer,
  &iterNextArrayMixedPointer,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  &iterNextKArrayPacked,
  &iterNextKArrayMixed,
  &iterNextKArray,
  &iter_next_cold, // iterNextKObject
  &iterNextKArrayPackedPointer,
  &iterNextKArrayMixedPointer,
};

const LIterNextHelper g_literNextHelpers[] = {
  &literNextArrayPacked,
  &literNextArrayMixed,
  &literNextArray,
  &literNextObject,
  &literNextArrayPackedPointer,
  &literNextArrayMixedPointer,
};

const LIterNextKHelper g_literNextKHelpers[] = {
  &literNextKArrayPacked,
  &literNextKArrayMixed,
  &literNextKArray,
  &literNextKObject,
  &literNextKArrayPackedPointer,
  &literNextKArrayMixedPointer,
};

int64_t iter_next_ind(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants());
  assertx(tvIsPlausible(*valOut));
  auto const index = unwrap(iter)->getHelperIndex();
  IterNextHelper helper = g_iterNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut);
}

int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants());
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  auto const index = unwrap(iter)->getHelperIndex();
  IterNextKHelper helper = g_iterNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, keyOut);
}

int64_t liter_next_ind(Iter* iter, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "liter_next_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  auto const index = unwrap(iter)->getHelperIndex();
  LIterNextHelper helper = g_literNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, ad);
}

int64_t liter_next_key_ind(Iter* iter,
                           TypedValue* valOut,
                           TypedValue* keyOut,
                           ArrayData* ad) {
  TRACE(2, "liter_next_key_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  auto const index = unwrap(iter)->getHelperIndex();
  LIterNextKHelper helper = g_literNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, keyOut, ad);
}

///////////////////////////////////////////////////////////////////////////////

}
