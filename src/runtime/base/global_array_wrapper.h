/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_GLOBAL_ARRAY_WRAPPER_H__
#define __HPHP_GLOBAL_ARRAY_WRAPPER_H__

#include <runtime/base/array/array_data.h>
#include <runtime/base/hphp_system.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class GlobalArrayWrapper : public ArrayData {
public:
  GlobalArrayWrapper() {
    m_globals = (Globals*)get_global_variables();
  }

  virtual void release() {
  }

  virtual ssize_t size() const {
    return m_globals->size();
  }

  virtual Variant getKey(ssize_t pos) const {
    Variant k;
    m_globals->getByIdx(pos, k);
    return k;
  }
  virtual Variant getValue(ssize_t pos) const {
    Variant k;
    return m_globals->getByIdx(pos, k);
  }
  virtual CVarRef getValueRef(ssize_t pos) const {
    Variant k;
    return m_globals->getRefByIdx(pos, k);
  }
  virtual bool supportValueRef() const { return true; }

  virtual bool exists(int64   k, int64 prehash = -1) const {
    return exists(Variant(k), prehash);
  }
  virtual bool exists(litstr  k, int64 prehash = -1) const {
    return exists(Variant(k), prehash);
  }
  virtual bool exists(CStrRef k, int64 prehash = -1) const {
    return exists(Variant(k), prehash);
  }
  virtual bool exists(CVarRef k, int64 prehash = -1) const {
    return m_globals->exists(k.toString().data());
  }
  virtual bool idxExists(ssize_t idx) const {
    return idx < size();
  }

  virtual Variant get(int64   k, int64 prehash = -1,
                      bool error = false) const {
    return get(Variant(k), prehash);
  }
  virtual Variant get(litstr  k, int64 prehash = -1,
                      bool error = false) const {
    if (exists(k, prehash)) {
      return m_globals->get(k);
    }
    return Variant();
  }
  virtual Variant get(CStrRef k, int64 prehash = -1,
                      bool error = false) const {
    if (exists(k, prehash)) {
      return m_globals->get(k);
    }
    return Variant();
  }
  virtual Variant get(CVarRef k, int64 prehash = -1,
                      bool error = false) const {
    if (exists(k, prehash)) {
      return m_globals->get(k);
    }
    return Variant();
  }

  virtual void load(CVarRef k, Variant &v) const {
    ssize_t idx = getIndex(k);
    if (idx >= 0) {
      CVarRef r = getValueRef(idx);
      if (r.isReferenced()) v = ref(r); else v = r;
    }
  }

  virtual ssize_t getIndex(int64 k, int64 prehash = -1) const {
    return m_globals->getIndex(toString(k), prehash);
  }
  virtual ssize_t getIndex(litstr k, int64 prehash = -1) const {
    return m_globals->getIndex(k, prehash);
  }
  virtual ssize_t getIndex(CStrRef k, int64 prehash = -1) const {
    return m_globals->getIndex(k.data(), prehash);
  }
  virtual ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

  virtual ArrayData *lval(Variant *&ret, bool copy) {
    ret = &m_globals->lval();
    return NULL;
  }
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) {
    return lval(Variant(k), ret, copy);
  }
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) {
    ret = &m_globals->get(k);
    return NULL;
  }
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) {
    ret = &m_globals->get(k);
    return NULL;
  }
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) {
    ret = &m_globals->get(k);
    return NULL;
  }

  virtual ArrayData *set(int64   k, CVarRef v,
                         bool copy, int64 prehash = -1) {
    set(Variant(k), v, copy, prehash);
    return NULL;
  }
  virtual ArrayData *set(litstr  k, CVarRef v,
                         bool copy, int64 prehash = -1) {
    m_globals->get(k) = v;
    return NULL;
  }
  virtual ArrayData *set(CStrRef k, CVarRef v,
                         bool copy, int64 prehash = -1) {
    m_globals->get(k) = v;
    return NULL;
  }
  virtual ArrayData *set(CVarRef k, CVarRef v,
                         bool copy, int64 prehash = -1) {
    m_globals->get(k) = v;
    return NULL;
  }

  virtual ArrayData *remove(int64   k, bool copy, int64 prehash /* = -1 */) {
    return remove(Variant(k), copy, prehash);
  }
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash /* = -1 */) {
    unset(m_globals->get(k));
    return NULL;
  }
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
    unset(m_globals->get(k));
    return NULL;
  }
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
    unset(m_globals->get(k));
    return NULL;
  }

  virtual ArrayData *copy() const {
    return NULL;
  }

  virtual ArrayData *append(CVarRef v, bool copy) {
    m_globals->append(v);
    return NULL;
  }

  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy) {
    ((Array*)m_globals)->get()->append(elems, op, false);
    return NULL;
  }

  virtual ArrayData *pop(Variant &value) {
    throw NotSupportedException(__func__, "manipulating globals array");
  }

  virtual ArrayData *dequeue(Variant &value) {
    throw NotSupportedException(__func__, "manipulating globals array");
  }

  virtual ArrayData *prepend(CVarRef v, bool copy) {
    throw NotSupportedException(__func__, "manipulating globals array");
  }

  ssize_t iter_begin() const {
    return m_globals->iter_begin();
  }
  ssize_t iter_end() const {
    return m_globals->iter_end();
  }
  ssize_t iter_advance(ssize_t prev) const {
    return m_globals->iter_advance(prev);
  }
  ssize_t iter_rewind(ssize_t prev) const {
    return m_globals->iter_rewind(prev);
  }

  virtual Variant next() {
    m_pos = m_globals->iter_advance(m_pos);
    return value(m_pos);
  }

  virtual void getFullPos(FullPos &pos) {
    if (m_pos == ArrayData::invalid_index) {
      pos.primary = ArrayData::invalid_index;
    } else if (m_pos < m_globals->staticSize()) {
      pos.primary = m_pos;
      pos.secondary = ArrayData::invalid_index;
    } else {
      m_globals->getFullPos(pos);
    }
  }

  virtual bool setFullPos(const FullPos &pos) {
    if (pos.primary != ArrayData::invalid_index) {
      if (m_pos < m_globals->staticSize()) return true;
      ArrayData *data = m_globals->getArrayData();
      if (data) {
        data->reset();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

private:
  Globals* m_globals;
};

///////////////////////////////////////////////////////////////////////////////

}
#endif
