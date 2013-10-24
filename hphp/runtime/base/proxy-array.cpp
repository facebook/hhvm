/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/request-arena.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline ProxyArray* ProxyArray::asProxyArray(ArrayData* ad) {
  assert(ad->kind() == kProxyKind);
  return static_cast<ProxyArray*>(ad);
}

inline const ProxyArray*
ProxyArray::asProxyArray(const ArrayData* ad) {
  assert(ad->kind() == kProxyKind);
  return static_cast<const ProxyArray*>(ad);
}

ArrayData* ProxyArray::innerArr(ArrayData* ad) {
  return asProxyArray(ad)->m_ad;
}

ArrayData* ProxyArray::innerArr(const ArrayData* ad) {
  return asProxyArray(ad)->m_ad;
}

ProxyArray* ProxyArray::Make(ArrayData* ad) {
  auto ret = static_cast<ProxyArray*>(MM().objMallocLogged(sizeof(ProxyArray)));
  ret->m_kindModeAndSize = static_cast<uint64_t>(-1) << 32 |
                           static_cast<uint32_t>(AllocationMode::smart) << 8 |
                           kProxyKind;
  ret->m_posAndCount     = uint64_t{1} << 32 |
                           static_cast<uint32_t>(ArrayData::invalid_index);
  ret->m_strongIterators = nullptr;

  ad->incRefCount();
  ret->m_ad = ad;

  assert(ret->m_kind == kProxyKind);
  assert(ret->m_allocMode == AllocationMode::smart);
  assert(ret->m_size == -1);
  assert(ret->m_pos == ArrayData::invalid_index);
  assert(ret->m_count == 1);
  return ret;
}

void ProxyArray::Release(ArrayData*ad) {
  decRefArr(innerArr(ad));
  MM().objFreeLogged(ad, sizeof(ProxyArray));
}

ProxyArray* ProxyArray::reseatable(ArrayData* oldArr, ArrayData* newArr) {
  if (innerArr(oldArr) != newArr) {
    decRefArr(innerArr(oldArr));
    newArr->incRefCount();
    asProxyArray(oldArr)->m_ad = newArr;
  }
  return asProxyArray(oldArr);
}

size_t ProxyArray::Vsize(const ArrayData* ad) {
  return innerArr(ad)->size();
}

void ProxyArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  return innerArr(ad)->nvGetKey(out, pos);
}

CVarRef ProxyArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  return innerArr(ad)->getValueRef(pos);
}

bool
ProxyArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return innerArr(ad)->exists(k);
}

bool
ProxyArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return innerArr(ad)->exists(k);
}

TypedValue*
ProxyArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  return innerArr(ad)->nvGet(k);
}

TypedValue* ProxyArray::NvGetInt(const ArrayData* ad, int64_t k) {
  return innerArr(ad)->nvGet(k);
}

ArrayData*
ProxyArray::LvalInt(ArrayData* ad, int64_t k, Variant*& ret, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->lval(k, ret, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::LvalStr(ArrayData* ad, StringData* k, Variant*& ret, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->lval(k, ret, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->lvalNew(ret, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetInt(ArrayData* ad, int64_t k,
                                         CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->set(k, v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetStr(ArrayData* ad, StringData* k,
                                         CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->set(k, v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetRefInt(ArrayData* ad, int64_t k,
                                            CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->setRef(k, v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetRefStr(ArrayData* ad, StringData* k,
                                            CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->setRef(k, v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->remove(k, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::RemoveStr(ArrayData* ad, const StringData* k,
                                 bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->remove(k, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::Copy(const ArrayData* ad) {
  return innerArr(ad)->copy();
}

ArrayData*
ProxyArray::Append(ArrayData* ad, CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->append(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::AppendRef(ArrayData* ad, CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->appendRef(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::AppendWithRef(ArrayData* ad, CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->appendWithRef(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  auto const ret = ad->hasMultipleRefs() ? Make(innerArr(ad))
                                         : asProxyArray(ad);
  auto r = ret->m_ad->plusEq(elems);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::Merge(ArrayData* ad, const ArrayData* elems) {
  auto r = innerArr(ad)->merge(elems);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::Pop(ArrayData* ad, Variant &value) {
  auto r = innerArr(ad)->pop(value);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::Dequeue(ArrayData* ad, Variant &value) {
  auto r = innerArr(ad)->dequeue(value);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::Prepend(ArrayData* ad, CVarRef v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->prepend(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

void ProxyArray::Renumber(ArrayData* ad) {
  innerArr(ad)->renumber();
}

void ProxyArray::OnSetEvalScalar(ArrayData* ad) {
  innerArr(ad)->onSetEvalScalar();
}

ArrayData* ProxyArray::Escalate(const ArrayData* ad) {
  auto r = innerArr(ad)->escalate();
  return reseatable(const_cast<ArrayData*>(ad), r);
}

ssize_t ProxyArray::IterBegin(const ArrayData* ad) {
  return innerArr(ad)->iter_begin();
}

ssize_t ProxyArray::IterEnd(const ArrayData* ad) {
  return innerArr(ad)->iter_end();
}

ssize_t ProxyArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  return innerArr(ad)->iter_advance(prev);
}

ssize_t ProxyArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  return innerArr(ad)->iter_rewind(prev);
}

bool
ProxyArray::ValidFullPos(const ArrayData* ad, const FullPos & fp) {
  return innerArr(ad)->validFullPos(fp);
}

bool ProxyArray::AdvanceFullPos(ArrayData* ad, FullPos& fp) {
  return innerArr(ad)->advanceFullPos(fp);
}

ArrayData* ProxyArray::EscalateForSort(ArrayData* ad) {
  auto r = innerArr(ad)->escalateForSort();
  return reseatable(ad, r);
}

void ProxyArray::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  return innerArr(ad)->ksort(sort_flags, ascending);
}

void ProxyArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  return innerArr(ad)->sort(sort_flags, ascending);
}

void ProxyArray::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  return innerArr(ad)->asort(sort_flags, ascending);
}

bool ProxyArray::Uksort(ArrayData* ad, CVarRef cmp_function) {
  return innerArr(ad)->uksort(cmp_function);
}

bool ProxyArray::Usort(ArrayData* ad, CVarRef cmp_function) {
  return innerArr(ad)->usort(cmp_function);
}

bool ProxyArray::Uasort(ArrayData* ad, CVarRef cmp_function) {
  return innerArr(ad)->uasort(cmp_function);
}

bool ProxyArray::IsVectorData(const ArrayData* ad) {
  return innerArr(ad)->isVectorData();
}

APCHandle *ProxyArray::GetAPCHandle(const ArrayData* ad) {
  return innerArr(ad)->getAPCHandle();
}

ArrayData* ProxyArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto r = innerArr(ad)->zSet(k, v);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto r = innerArr(ad)->zSet(k, v);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::ZAppend(ArrayData* ad, RefData* v) {
  auto r = innerArr(ad)->zAppend(v);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::CopyWithStrongIterators(const ArrayData* ad) {
  return innerArr(ad)->copyWithStrongIterators();
}

ArrayData* ProxyArray::NonSmartCopy(const ArrayData* ad) {
  return innerArr(ad)->nonSmartCopy();
}


//////////////////////////////////////////////////////////////////////

}
