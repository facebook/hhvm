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

#include "runtime/vm/name_value_table_wrapper.h"
#include "runtime/base/array/array_iterator.h"
#include "runtime/base/array/array_init.h"

namespace HPHP { namespace VM {

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
  return iter.valid() ? Variant(StrNR(iter.curKey())) : null_variant;
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

bool NameValueTableWrapper::exists(int64 k) const {
  return exists(Variant(k));
}

bool NameValueTableWrapper::exists(const StringData* k) const {
  return m_tab->lookup(k);
}

bool NameValueTableWrapper::idxExists(ssize_t idx) const {
  return false;
}

CVarRef NameValueTableWrapper::get(int64 k, bool error) const {
  return get(Variant(k), error);
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

TypedValue* NameValueTableWrapper::nvGet(int64 k) const {
  Variant k2(k);
  return m_tab->lookup(k2.toString().get());
}

ssize_t NameValueTableWrapper::getIndex(int64 k) const {
  return getIndex(Variant(k));
}

ssize_t NameValueTableWrapper::getIndex(const StringData* k) const {
  NameValueTable::Iterator iter(m_tab, k);
  return iter.toInteger();
}

ArrayData* NameValueTableWrapper::lval(int64 k, Variant*& ret, bool copy,
                                       bool checkExist) {
  return lval(Variant(k), ret, copy, checkExist);
}

ArrayData* NameValueTableWrapper::lval(StringData* k, Variant*& ret,
                                       bool copy, bool checkExist) {
  TypedValue* tv = m_tab->lookup(k);
  if (!tv) {
    TypedValue nulVal;
    TV_WRITE_NULL(&nulVal);
    tv = m_tab->set(k, &nulVal);
  }
  ret = &tvAsVariant(tv);
  return 0;
}

ArrayData* NameValueTableWrapper::lvalNew(Variant*& ret, bool copy) {
  ret = &Variant::lvalBlackHole();
  return 0;
}

ArrayData* NameValueTableWrapper::set(int64 k, CVarRef v, bool copy) {
  return set(Variant(k), v, copy);
}

ArrayData* NameValueTableWrapper::set(StringData* k, CVarRef v,
                                      bool copy) {
  tvAsVariant(m_tab->lookupAdd(k)).assignVal(v);
  return 0;
}

ArrayData* NameValueTableWrapper::setRef(int64 k, CVarRef v, bool copy) {
  return setRef(Variant(k), v, copy);
}

ArrayData* NameValueTableWrapper::setRef(StringData* k, CVarRef v, bool copy) {
  tvAsVariant(m_tab->lookupAdd(k)).assignRef(v);
  return 0;
}

ArrayData* NameValueTableWrapper::remove(int64 k, bool copy) {
  return remove(Variant(k), copy);
}

ArrayData* NameValueTableWrapper::remove(const StringData* k, bool copy) {
  m_tab->unset(k);
  return 0;
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

void NameValueTableWrapper::getFullPos(FullPos &fp) {
  ASSERT(fp.container == this);
  fp.pos = m_pos;
}

bool NameValueTableWrapper::setFullPos(const FullPos& fp) {
  ASSERT(fp.container == this);
  if (fp.pos != ArrayData::invalid_index) {
    NameValueTable::Iterator iter(m_tab, fp.pos);
    if (iter.valid()) {
      m_pos = iter.toInteger();
      return true;
    }
  }
  return false;
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

}}
