/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/name_value_table_wrapper.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/array/array_init.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ssize_t NameValueTableWrapper::vsize() const {
  // We need to iterate to find out the actual size, since
  // KindOfIndirect elements in the array may have been set to
  // KindOfUninit.
  ssize_t count = 0;
  for (ssize_t iter = iter_begin();
      iter != ArrayData::invalid_index;
      iter = iter_advance(iter)) {
    ++count;
  }
  return count;
}

Variant NameValueTableWrapper::getKey(ssize_t pos) const {
  NameValueTable::Iterator iter(m_tab, pos);
  return iter.valid() ? Variant(StrNR(iter.curKey())) : uninit_null();
}

Variant NameValueTableWrapper::getValue(ssize_t pos) const {
  NameValueTable::Iterator iter(m_tab, pos);
  return iter.valid() ? tvAsCVarRef(iter.curVal()) : null_variant;
}

CVarRef NameValueTableWrapper::getValueRef(ssize_t pos) const {
  NameValueTable::Iterator iter(m_tab, pos);
  return iter.valid() ? tvAsCVarRef(iter.curVal()) : null_variant;
}

bool NameValueTableWrapper::noCopyOnWrite() const {
  // This just disables a few places that will call copy() on an array
  // if it has more than one reference.
  return true;
}

bool NameValueTableWrapper::exists(int64_t k) const {
  return exists(String(k));
}

bool NameValueTableWrapper::exists(const StringData* k) const {
  return m_tab->lookup(k);
}

bool NameValueTableWrapper::idxExists(ssize_t idx) const {
  return false;
}

CVarRef NameValueTableWrapper::get(int64_t k, bool error) const {
  return get(String(k), error);
}

CVarRef NameValueTableWrapper::get(const StringData* k, bool error) const {
  TypedValue* tv = m_tab->lookup(k);
  if (tv) {
    return tvAsCVarRef(tv);
  }
  // NOTE: ignoring error on these, as global_array_wrapper does so too,
  // but I'm not sure why.
  // if (error) {
  //   raise_notice("Undefined index: %s", k->data());
  // }
  return null_variant;
}

TypedValue* NameValueTableWrapper::nvGet(const StringData* k) const {
  return m_tab->lookup(k);
}

TypedValue* NameValueTableWrapper::nvGet(int64_t k) const {
  return m_tab->lookup(String(k).get());
}

ssize_t NameValueTableWrapper::getIndex(int64_t k) const {
  return getIndex(String(k));
}

ssize_t NameValueTableWrapper::getIndex(const StringData* k) const {
  NameValueTable::Iterator iter(m_tab, k);
  return iter.toInteger();
}

ArrayData* NameValueTableWrapper::lval(int64_t k, Variant*& ret, bool copy,
                                       bool checkExist) {
  return lval(String(k), ret, copy, checkExist);
}

ArrayData* NameValueTableWrapper::lval(StringData* k, Variant*& ret,
                                       bool copy, bool checkExist) {
  TypedValue* tv = m_tab->lookup(k);
  if (!tv) {
    TypedValue nulVal;
    tvWriteNull(&nulVal);
    tv = m_tab->set(k, &nulVal);
  }
  ret = &tvAsVariant(tv);
  return this;
}

ArrayData* NameValueTableWrapper::lvalNew(Variant*& ret, bool copy) {
  ret = &Variant::lvalBlackHole();
  return this;
}

ArrayData* NameValueTableWrapper::set(int64_t k, CVarRef v, bool copy) {
  return set(String(k), v, copy);
}

ArrayData* NameValueTableWrapper::set(StringData* k, CVarRef v,
                                      bool copy) {
  tvAsVariant(m_tab->lookupAdd(k)).assignVal(v);
  return this;
}

ArrayData* NameValueTableWrapper::setRef(int64_t k, CVarRef v, bool copy) {
  return setRef(String(k), v, copy);
}

ArrayData* NameValueTableWrapper::setRef(StringData* k, CVarRef v, bool copy) {
  tvAsVariant(m_tab->lookupAdd(k)).assignRef(v);
  return this;
}

ArrayData* NameValueTableWrapper::remove(int64_t k, bool copy) {
  return remove(String(k), copy);
}

ArrayData* NameValueTableWrapper::remove(const StringData* k, bool copy) {
  m_tab->unset(k);
  return this;
}

/*
 * The messages in the user-visible exceptions below claim we are
 * $GLOBALS, because the only user-visible NameValueTableWrapper array
 * is currently $GLOBALS.
 */

ArrayData* NameValueTableWrapper::append(CVarRef v, bool copy) {
  throw NotImplementedException("append on $GLOBALS");
}

ArrayData* NameValueTableWrapper::appendRef(CVarRef v, bool copy) {
  throw NotImplementedException("appendRef on $GLOBALS");
}

ArrayData* NameValueTableWrapper::appendWithRef(CVarRef v, bool copy) {
  throw NotImplementedException("appendWithRef on $GLOBALS");
}

ArrayData* NameValueTableWrapper::append(const ArrayData* elems,
                                         ArrayOp op,
                                         bool copy) {
  throw NotImplementedException("append on $GLOBALS");
}

ArrayData* NameValueTableWrapper::prepend(CVarRef v, bool copy) {
  throw NotImplementedException("prepend on $GLOBALS");
}

ssize_t NameValueTableWrapper::iter_begin() const {
  NameValueTable::Iterator iter(m_tab);
  return iter.toInteger();
}

ssize_t NameValueTableWrapper::iter_end() const {
  return NameValueTable::Iterator::getEnd(m_tab).toInteger();
}

ssize_t NameValueTableWrapper::iter_advance(ssize_t prev) const {
  NameValueTable::Iterator iter(m_tab, prev);
  iter.next();
  return iter.toInteger();
}

ssize_t NameValueTableWrapper::iter_rewind(ssize_t prev) const {
  NameValueTable::Iterator iter(m_tab, prev);
  iter.prev();
  return iter.toInteger();
}

Variant NameValueTableWrapper::reset() {
  m_pos = iter_begin();
  return current();
}

Variant NameValueTableWrapper::prev() {
  if (m_pos != ArrayData::invalid_index) {
    m_pos = iter_rewind(m_pos);
    return current();
  }
  return Variant(false);
}

Variant NameValueTableWrapper::current() const {
  return m_pos != ArrayData::invalid_index
    ? getValueRef(m_pos)
    : Variant(false);
}

CVarRef NameValueTableWrapper::currentRef() {
  if (m_pos != ArrayData::invalid_index) {
    return getValueRef(m_pos);
  }
  throw FatalErrorException("invalid ArrayData::m_pos");
}

Variant NameValueTableWrapper::next() {
  if (m_pos != ArrayData::invalid_index) {
    m_pos = iter_advance(m_pos);
    return current();
  }
  return Variant(false);
}

Variant NameValueTableWrapper::end() {
  m_pos = iter_end();
  return current();
}

CVarRef NameValueTableWrapper::endRef() {
  m_pos = iter_end();
  return currentRef();
}

Variant NameValueTableWrapper::key() const {
  if (m_pos != ArrayData::invalid_index) {
    return getKey(m_pos);
  }
  return null_variant;
}

Variant NameValueTableWrapper::value(ssize_t& pos) const {
  return current();
}

static StaticString s_value("value");
static StaticString s_key("key");

Variant NameValueTableWrapper::each() {
  if (m_pos != ArrayData::invalid_index) {
    ArrayInit init(4);
    Variant key = getKey(m_pos);
    Variant value = getValue(m_pos);
    init.set(1, value);
    init.set(s_value, value, true);
    init.set(0, key);
    init.set(s_key, key, true);
    m_pos = iter_advance(m_pos);
    return Array(init.create());
  }
  return Variant(false);
}

bool NameValueTableWrapper::validFullPos(const FullPos & fp) const {
  assert(fp.getContainer() == (ArrayData*)this);
  if (fp.getResetFlag()) return false;
  if (fp.m_pos == ArrayData::invalid_index) return false;
  NameValueTable::Iterator iter(m_tab, fp.m_pos);
  return (iter.valid());
}

bool NameValueTableWrapper::advanceFullPos(FullPos& fp) {
  bool reset = fp.getResetFlag();
  NameValueTable::Iterator iter = reset ?
    NameValueTable::Iterator(m_tab) :
    NameValueTable::Iterator(m_tab, fp.m_pos);
  if (reset) {
    fp.setResetFlag(false);
  } else {
    if (!iter.valid()) {
      return false;
    }
    iter.next();
  }
  fp.m_pos = iter.toInteger();
  if (!iter.valid()) return false;
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  iter.next();
  m_pos = iter.toInteger();
  return true;
}

ArrayData* NameValueTableWrapper::escalateForSort() {
  raise_warning("Sorting the $GLOBALS array is not supported");
  return this;
}
void NameValueTableWrapper::ksort(int sort_flags, bool ascending) {}
void NameValueTableWrapper::sort(int sort_flags, bool ascending) {}
void NameValueTableWrapper::asort(int sort_flags, bool ascending) {}
void NameValueTableWrapper::uksort(CVarRef cmp_function) {}
void NameValueTableWrapper::usort(CVarRef cmp_function) {}
void NameValueTableWrapper::uasort(CVarRef cmp_function) {}

//////////////////////////////////////////////////////////////////////

}
