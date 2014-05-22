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

#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/zend-custom-element.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"

// FIXME: get this from the proper header.
// We need to move proxy-array.cpp to ext_zend_compat/hhvm before
// the Zend headers can be included.
#undef ZVAL_PTR_DTOR
#if !defined(ENABLE_ZEND_COMPAT)
namespace HPHP { void zval_ptr_dtor_dummy(HPHP::RefData **zval_ptr) {} }
#define ZVAL_PTR_DTOR HPHP::zval_ptr_dtor_dummy
#elif defined(DEBUG)
extern "C" void _zval_ptr_dtor_wrapper(HPHP::RefData **zval_ptr);
#define ZVAL_PTR_DTOR _zval_ptr_dtor_wrapper
#else
extern "C" void _zval_ptr_dtor(HPHP::RefData **zval_ptr);
#define  ZVAL_PTR_DTOR _zval_ptr_dtor
#endif

namespace HPHP {

// We make a static copy of the _zval_ptr_dtor_wrapper function pointer to
// avoid the need to declare _zval_ptr_dtor_wrapper in proxy-array.h, which
// can give conflicting declaration warnings when it is included from a file
// which also includes zend_variables.h.
ProxyArray::DtorFunc ProxyArray::ZvalPtrDtor =
  (ProxyArray::DtorFunc)ZVAL_PTR_DTOR;

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
  ret->m_size            = -1;
  ret->m_kind            = kProxyKind;
  ret->m_pos             = ArrayData::invalid_index;
  ret->m_count           = 1;
  ret->m_destructor      = ZvalPtrDtor;
  ret->m_ad = ad;

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

const Variant& ProxyArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
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

const TypedValue*
ProxyArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  return innerArr(ad)->nvGet(k);
}

const TypedValue* ProxyArray::NvGetInt(const ArrayData* ad, int64_t k) {
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

ArrayData* ProxyArray::SetInt(ArrayData* ad,
                              int64_t k,
                              Cell v,
                              bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->set(k,
    tvAsCVarRef(&v), innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetStr(ArrayData* ad,
                              StringData* k,
                              Cell v,
                              bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->set(k,
    tvAsCVarRef(&v), innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetRefInt(ArrayData* ad,
                                 int64_t k,
                                 Variant& v,
                                 bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->setRef(k, v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::SetRefStr(ArrayData* ad,
                                 StringData* k,
                                 Variant& v,
                                 bool copy) {
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
ProxyArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->append(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  ad = copy ? Make(innerArr(ad)) : ad;
  auto r = innerArr(ad)->appendRef(v, innerArr(ad)->hasMultipleRefs());
  assert(!copy);
  return reseatable(ad, r);
}

ArrayData*
ProxyArray::AppendWithRef(ArrayData* ad, const Variant& v, bool copy) {
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

ArrayData* ProxyArray::Prepend(ArrayData* ad, const Variant& v, bool copy) {
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
ProxyArray::ValidMArrayIter(const ArrayData* ad, const MArrayIter & fp) {
  return innerArr(ad)->validMArrayIter(fp);
}

bool ProxyArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  return innerArr(ad)->advanceMArrayIter(fp);
}

ArrayData* ProxyArray::EscalateForSort(ArrayData* ad) {
  auto r = innerArr(ad)->escalateForSort();
  return reseatable(ad, r);
}

void ProxyArray::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->ksort(sort_flags, ascending);
}

void ProxyArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->sort(sort_flags, ascending);
}

void ProxyArray::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->asort(sort_flags, ascending);
}

bool ProxyArray::Uksort(ArrayData* ad, const Variant& cmp_function) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->uksort(cmp_function);
}

bool ProxyArray::Usort(ArrayData* ad, const Variant& cmp_function) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->usort(cmp_function);
}

bool ProxyArray::Uasort(ArrayData* ad, const Variant& cmp_function) {
  ad = reseatable(ad, innerArr(ad)->escalateForSort());
  return innerArr(ad)->uasort(cmp_function);
}

bool ProxyArray::IsVectorData(const ArrayData* ad) {
  return innerArr(ad)->isVectorData();
}

ArrayData* ProxyArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto r = innerArr(ad)->zSet(k, v);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto r = innerArr(ad)->zSet(k, v);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  auto r = innerArr(ad)->zAppend(v, key_ptr);
  return reseatable(ad, r);
}

ArrayData* ProxyArray::CopyWithStrongIterators(const ArrayData* ad) {
  return innerArr(ad)->copyWithStrongIterators();
}

ArrayData* ProxyArray::NonSmartCopy(const ArrayData* ad) {
  return innerArr(ad)->nonSmartCopy();
}

void ProxyArray::proxyAppend(void* data, uint32_t data_size, void** dest) {
  ArrayData * r;
  if (hasZvalValues()) {
    assert(data_size == sizeof(void*));
    int64_t k = 0;
    r = m_ad->zAppend(*(RefData**)data, &k);
    if (dest) {
      *dest = (void*)(&m_ad->nvGet(k)->m_data.pref);
    }
  } else {
    ResourceData * elt = makeElementResource(data, data_size, dest);
    r = m_ad->append(elt, false);
  }
  reseatable(this, r);
}

void ProxyArray::proxyInit(uint32_t nSize,
    DtorFunc pDestructor, bool persistent) {
  if (persistent) {
    throw FatalErrorException("zend_hash_init: \"persistent\" is \
                              unimplemented");
  }
  if (nSize) {
    decRefArr(m_ad);
    m_ad = MixedArray::MakeReserve(nSize);
  }
  m_destructor = pDestructor;
}

ResourceData * ProxyArray::makeElementResource(
    void *pData, uint nDataSize, void **pDest) const {
  ZendCustomElement * elt = new ZendCustomElement(pData, nDataSize,
                                                  pDest, m_destructor);
  return static_cast<ResourceData*>(elt);
}

void * ProxyArray::proxyGet(StringData * str) const {
  // FIXME: const_cast is a bug, may destroy shared data
  return elementToData(const_cast<TypedValue*>(m_ad->nvGet(str)));
}

void * ProxyArray::proxyGet(int64_t k) const {
  // FIXME: const_cast is a bug, may destroy shared data
  return elementToData(const_cast<TypedValue*>(m_ad->nvGet(k)));
}

void * ProxyArray::proxyGetValueRef(ssize_t pos) const {
  auto& val = m_ad->getValueRef(pos);
  // FIXME: we shouldn't be modifying this TypedValue
  return elementToData(const_cast<HPHP::TypedValue*>(val.asTypedValue()));
}

void * ProxyArray::elementToData(TypedValue * tv) const {
  if (!tv) {
    return nullptr;
  }
  if (hasZvalValues()) {
    zBoxAndProxy(tv);
    return (void*)(&tv->m_data.pref);
  } else {
    always_assert(tv->m_type == KindOfResource);
    ZendCustomElement * elt = dynamic_cast<ZendCustomElement*>(tv->m_data.pres);
    always_assert(elt);
    return elt->data();
  }
}

//////////////////////////////////////////////////////////////////////

}
