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
#include "hphp/runtime/vm/globals-array.h"

#include <algorithm>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/util/rds-local.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static RDS_LOCAL_NO_CHECK(GlobalsArray*, g_variables)(nullptr);

GlobalsArray* get_global_variables() {
  assertx(*g_variables != nullptr);
  return *g_variables;
}

GlobalsArray::GlobalsArray(NameValueTable* tab)
  : ArrayData(kGlobalsKind)
  , m_tab(tab)
{
  Variant arr(ArrayData::CreateDArray());
#define X(s,v) tab->set(makeStaticString(#s), v.asTypedValue());

  X(argc,                 init_null_variant);
  X(argv,                 init_null_variant);
  X(_SERVER,              arr);
  X(_GET,                 arr);
  X(_POST,                arr);
  X(_COOKIE,              arr);
  X(_FILES,               arr);
  X(_ENV,                 arr);
  X(_REQUEST,             arr);
  X(HTTP_RAW_POST_DATA,   init_null_variant);
#undef X

  *g_variables = this;
  assertx(hasExactlyOneRef());
}

inline GlobalsArray* GlobalsArray::asGlobals(ArrayData* ad) {
  assertx(ad->kind() == kGlobalsKind);
  return static_cast<GlobalsArray*>(ad);
}

inline const GlobalsArray*
GlobalsArray::asGlobals(const ArrayData* ad) {
  assertx(ad->kind() == kGlobalsKind);
  return static_cast<const GlobalsArray*>(ad);
}

size_t GlobalsArray::Vsize(const ArrayData* ad) {
  // We need to iterate to find out the actual size, since kNamedLocalDataType
  // elements in the array may have been set to KindOfUninit.
  auto a = asGlobals(ad);
  if (a->m_tab->leaked()) return 0;
  size_t count = 0;
  auto iter_limit = IterEnd(a);
  for (auto iter = IterBegin(a);
       iter != iter_limit;
       iter = IterAdvance(a, iter)) {
    ++count;
  }
  return count;
}

ArrayData* GlobalsArray::keys() {
  auto iter = NameValueTable::Iterator::getEnd(this->m_tab);
  KeysetInit ret(iter.toInteger());
  for (iter.prev(); iter.valid(); iter.prev()) {
    auto const& k = iter.curKey();
    if (k->isRefCounted()) {
      k->rawIncRefCount();
      ret.add(make_tv<KindOfString>(const_cast<StringData*>(k)));
    } else {
      ret.add(make_tv<KindOfPersistentString>(k));
    }
  }
  return ret.create();
}

bool GlobalsArray::keyExists(const StringData* k) {
  return this->m_tab->lookup(k) != nullptr;
}

TypedValue GlobalsArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  auto a = asGlobals(ad);
  NameValueTable::Iterator iter(a->m_tab, pos);
  if (iter.valid()) {
    auto const k = iter.curKey();
    return k->isRefCounted() ? make_tv<KindOfString>(const_cast<StringData*>(k))
                             : make_tv<KindOfPersistentString>(k);
  }
  return make_tv<KindOfUninit>();
}

TypedValue GlobalsArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  auto a = asGlobals(ad);
  NameValueTable::Iterator iter(a->m_tab, pos);
  return iter.valid() ? *iter.curVal() : *uninit_variant.asTypedValue();
}

bool
GlobalsArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return ExistsStr(ad, String(k).get());
}

bool
GlobalsArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  return asGlobals(ad)->m_tab->lookup(k) != nullptr;
}

tv_rval GlobalsArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  return asGlobals(ad)->m_tab->lookup(k);
}

tv_rval GlobalsArray::NvGetInt(const ArrayData* ad, int64_t k) {
  return asGlobals(ad)->m_tab->lookup(String(k).get());
}

ssize_t GlobalsArray::NvGetStrPos(const ArrayData* ad, const StringData* k) {
  return asGlobals(ad)->m_tab->lookupPos(k);
}

ssize_t GlobalsArray::NvGetIntPos(const ArrayData* ad, int64_t k) {
  return asGlobals(ad)->m_tab->lookupPos(String(k).get());
}

arr_lval GlobalsArray::LvalInt(ArrayData* ad, int64_t k, bool copy) {
  return LvalStr(ad, String(k).get(), copy);
}

arr_lval GlobalsArray::LvalStr(ArrayData* ad, StringData* k, bool /*copy*/) {
  auto a = asGlobals(ad);
  TypedValue* tv = a->m_tab->lookup(k);
  if (!tv) {
    TypedValue nulVal;
    tvWriteNull(nulVal);
    tv = a->m_tab->set(k, &nulVal);
  }
  return arr_lval { ad, tv };
}

ArrayData* GlobalsArray::SetIntMove(ArrayData* ad, int64_t k, TypedValue v) {
  return SetStrMove(ad, String(k).get(), v);
}

ArrayData* GlobalsArray::SetInt(ArrayData* ad, int64_t k, TypedValue v) {
  return SetStr(ad, String(k).get(), v);
}

ArrayData* GlobalsArray::SetStrMove(ArrayData* ad, StringData* k, TypedValue v) {
  tvMove(v, *asGlobals(ad)->m_tab->lookupAdd(k));
  return ad;
}

ArrayData* GlobalsArray::SetStr(ArrayData* ad, StringData* k, TypedValue v) {
  tvSet(v, *asGlobals(ad)->m_tab->lookupAdd(k));
  return ad;
}

ArrayData*
GlobalsArray::RemoveInt(ArrayData* ad, int64_t k) {
  return RemoveStr(ad, String(k).get());
}

ArrayData*
GlobalsArray::RemoveStr(ArrayData* ad, const StringData* k) {
  auto a = asGlobals(ad);
  a->m_tab->unset(k);
  return a;
}

/*
 * The messages in the user-visible exceptions below claim we are
 * $GLOBALS, because the only user-visible GlobalsArray array
 * is currently $GLOBALS.
 */

ArrayData* GlobalsArray::Append(ArrayData*, TypedValue /*v*/) {
  throw_not_implemented("append on $GLOBALS");
}

ArrayData* GlobalsArray::PlusEq(ArrayData*, const ArrayData*) {
  throw_not_implemented("plus on $GLOBALS");
}

ArrayData* GlobalsArray::Merge(ArrayData*, const ArrayData*) {
  throw_not_implemented("merge on $GLOBALS");
}

ArrayData* GlobalsArray::Prepend(ArrayData*, TypedValue) {
  throw_not_implemented("prepend on $GLOBALS");
}

ssize_t GlobalsArray::IterBegin(const ArrayData* ad) {
  auto a = asGlobals(ad);
  NameValueTable::Iterator iter(a->m_tab);
  return iter.toInteger();
}

ssize_t GlobalsArray::IterLast(const ArrayData* ad) {
  auto a = asGlobals(ad);
  return NameValueTable::Iterator::getLast(a->m_tab).toInteger();
}

ssize_t GlobalsArray::IterEnd(const ArrayData* ad) {
  auto a = asGlobals(ad);
  return NameValueTable::Iterator::getEnd(a->m_tab).toInteger();
}

ssize_t GlobalsArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asGlobals(ad);
  NameValueTable::Iterator iter(a->m_tab, prev);
  iter.next();
  return iter.toInteger();
}

ssize_t GlobalsArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  auto a = asGlobals(ad);
  NameValueTable::Iterator iter(a->m_tab, prev);
  iter.prev();
  return iter.toInteger();
}

ArrayData* GlobalsArray::EscalateForSort(ArrayData* ad, SortFunction /*sf*/) {
  raise_warning("Sorting the $GLOBALS array is not supported");
  return ad;
}
void GlobalsArray::Ksort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {}
void GlobalsArray::Sort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {}
void GlobalsArray::Asort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {}
bool GlobalsArray::Uksort(ArrayData*, const Variant& /*cmp_function*/) {
  return false;
}
bool GlobalsArray::Usort(ArrayData*, const Variant& /*cmp_function*/) {
  return false;
}
bool GlobalsArray::Uasort(ArrayData*, const Variant& /*cmp_function*/) {
  return false;
}

bool GlobalsArray::IsVectorData(const ArrayData*) {
  return false;
}

ArrayData* GlobalsArray::CopyStatic(const ArrayData*) {
  raise_fatal_error("GlobalsArray::copyStatic "
    "not implemented.");
}

void GlobalsArray::Renumber(ArrayData*) {}

void GlobalsArray::OnSetEvalScalar(ArrayData*) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
