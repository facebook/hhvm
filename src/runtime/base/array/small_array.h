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
#ifndef __HPHP_SMALL_ARRAY_H__
#define __HPHP_SMALL_ARRAY_H__

#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SmallArray : public ArrayData {
public:
  static const int SARR_SIZE = 7;
  static const int SARR_TABLE_SIZE = 8;

  SmallArray();
  virtual ~SmallArray() { }

  virtual ssize_t size() const { return m_nNumOfElements; }

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual void fetchValue(ssize_t pos, Variant & v) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;
  virtual bool supportValueRef() const { return true; }

  virtual ssize_t iter_begin() const { return m_nListHead; }
  virtual ssize_t iter_end() const { return m_nListTail; }
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual Variant key() const;
  virtual Variant value(ssize_t &pos) const;
  virtual Variant each();

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, bool error = false) const;
  virtual Variant get(litstr  k, bool error = false) const;
  virtual Variant get(CStrRef k, bool error = false) const;
  virtual Variant get(CVarRef k, bool error = false) const;

  virtual void load(CVarRef k, Variant &v) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const;
  virtual ssize_t getIndex(CStrRef k) const;
  virtual ssize_t getIndex(CVarRef k) const;

  virtual ArrayData *lval(Variant *&ret, bool copy);
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lvalPtr(CStrRef k, Variant *&ret, bool copy,
                             bool create);

  virtual ArrayData *set(int64   k, CVarRef v, bool copy);
  virtual ArrayData *set(litstr  k, CVarRef v, bool copy);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy);

  virtual ArrayData *remove(int64   k, bool copy);
  virtual ArrayData *remove(litstr  k, bool copy);
  virtual ArrayData *remove(CStrRef k, bool copy);
  virtual ArrayData *remove(CVarRef k, bool copy);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *pop(Variant &value);
  virtual ArrayData *dequeue(Variant &value);
  virtual ArrayData *prepend(CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetStatic();

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  virtual ArrayData *escalate(bool mutableIteration = false) const;

  DECLARE_SMART_ALLOCATION_NOCALLBACKS(SmallArray);

  enum Kind {
    Empty,
    IntKey,
    StrKey
  };

  class Bucket {
  public:
    Kind        kind;
    int8        prev;
    int8        next;
    int64       h;
    StringData *key;
    Variant     data;

    Bucket() : kind(Empty), key(NULL) { }
    Bucket(const Bucket &other) : kind(other.kind), key(other.key) {
      if (kind != Empty) {
        prev = other.prev; next = other.next;
        h = other.h;
        if (other.data.isReferenced()) other.data.setContagious();
        data = other.data;
        if (key) key->incRefCount();
      }
    }
    ~Bucket() {
      if (key && key->decRefCount() == 0) DELETE(StringData)(key);
    }
  };

private:
  int8            m_nNumOfElements;
  int8            m_nListHead;
  int8            m_nListTail;
  unsigned long   m_nNextFreeElement;

  Bucket  m_arBuckets[SARR_TABLE_SIZE];

  static int int_ihash(int64 h) { return h & (SARR_TABLE_SIZE - 1); }
  static int str_ihash(int   h) { return h & (SARR_TABLE_SIZE - 1); }

  void connect_to_global_dllist(int p, Bucket &b) {
    ASSERT(p >= 0 && p < SARR_TABLE_SIZE);
    b.prev = m_nListTail;
    b.next = ArrayData::invalid_index;
    m_nListTail = p;
    if (b.prev >= 0) m_arBuckets[(int)b.prev].next = p;
    if (m_nListHead < 0) m_nListHead = p;
    if (m_pos < 0) m_pos = p;
  }

  ArrayData *escalateToZendArray() const;

  inline int find(int64 h) const;
  inline int find(const char *k, int len, int64 prehash) const;

  SmallArray *copyImpl() const {
    SmallArray *a = NEW(SmallArray)(*this);
    a->_count = 0;
    return a;
  }

  // m_arBuckets[p].kind == Empty
  inline Bucket *addKey(int p, int64 h);
  inline Bucket *addKey(int p, StringData *key);

  // no-op if the key already exists
  inline bool add(int64 h, CVarRef data);
  inline bool add(StringData *key, CVarRef data);

  inline void erase(Bucket *pb);
  inline void nextInsert(CVarRef v);
};

///////////////////////////////////////////////////////////////////////////////
// Small empty arrays

class StaticEmptySmallArray : public SmallArray {
public:
  StaticEmptySmallArray() { setStatic(); }

  static SmallArray *Get() { return &s_theEmptyArray; }

private:
  static StaticEmptySmallArray s_theEmptyArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMALL_ZEND_ARRAY_H__
