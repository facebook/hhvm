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

#ifndef __HPHP_ZEND_ARRAY_H__
#define __HPHP_ZEND_ARRAY_H__

#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;

class ZendArray : public ArrayData {
public:
  friend class ArrayInit;

  ZendArray(uint nSize = 0);
  virtual ~ZendArray();

  virtual ssize_t size() const;

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  virtual void iter_dirty_set() const;
  virtual void iter_dirty_reset() const;
  virtual void iter_dirty_check() const;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual Variant key() const;
  virtual Variant value(ssize_t &pos) const;
  virtual Variant each();

  virtual bool isHead() const { return m_pos == (ssize_t)m_pListHead; }
  virtual bool isTail() const { return m_pos == (ssize_t)m_pListTail; }
  virtual bool isInvalid() const { return !m_pos; }

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual CVarRef get(int64   k, bool error = false) const;
  virtual CVarRef get(litstr  k, bool error = false) const;
  virtual CVarRef get(CStrRef k, bool error = false) const;
  virtual CVarRef get(CVarRef k, bool error = false) const;

  virtual void load(CVarRef k, Variant &v) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const;
  virtual ssize_t getIndex(CStrRef k) const;
  virtual ssize_t getIndex(CVarRef k) const;

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
  virtual ArrayData *lvalPtr(int64   k, Variant *&ret, bool copy,
                             bool create);

  virtual ArrayData *lvalNew(Variant *&ret, bool copy);

  virtual ArrayData *set(int64   k, CVarRef v, bool copy);
  virtual ArrayData *set(litstr  k, CVarRef v, bool copy);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy);

  virtual ArrayData *add(int64   k, CVarRef v, bool copy);
  virtual ArrayData *add(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *add(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *addLval(int64   k, Variant *&ret, bool copy);
  virtual ArrayData *addLval(CStrRef k, Variant *&ret, bool copy);
  virtual ArrayData *addLval(CVarRef k, Variant *&ret, bool copy);

  virtual ArrayData *remove(int64   k, bool copy);
  virtual ArrayData *remove(litstr  k, bool copy);
  virtual ArrayData *remove(CStrRef k, bool copy);
  virtual ArrayData *remove(CVarRef k, bool copy);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *appendWithRef(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *pop(Variant &value);
  virtual ArrayData *dequeue(Variant &value);
  virtual ArrayData *prepend(CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetStatic();

  virtual void getFullPos(FullPos &fp);
  virtual bool setFullPos(const FullPos &fp);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  class Bucket {
  public:
    Bucket() :
      h(0), key(NULL), pListNext(NULL), pListLast(NULL), pNext(NULL) { }

    Bucket(CVarRef d) :
      h(0), key(NULL), data(d), pListNext(NULL), pListLast(NULL), pNext(NULL)
      { }

    // These two constructors should never be called directly, they are
    // only called from generated code.
    Bucket(StringData *k, CVarRef d) :
      key(k), data(d) {
      ASSERT(k->isLiteral());
      ASSERT(k->isStatic());
      h = k->getPrecomputedHash();
    }
    Bucket(int64 k, CVarRef d) : h(k), key(NULL), data(d) { }

    ~Bucket();

    int64       h;
    StringData *key;
    Variant     data;
    Bucket     *pListNext;
    Bucket     *pListLast;
    Bucket     *pNext;

    /**
     * Memory allocator methods.
     */
    DECLARE_SMART_ALLOCATION_NOCALLBACKS(Bucket);
    void dump();
  };

  // This constructor should never be called directly, it is only called
  // from generated code.
  ZendArray(uint nSize, int64 n, Bucket *bkts[]);

private:
  enum Flag {
    LinearAllocated       = 1,
    StrongIteratorPastEnd = 2,
    IterationDirty        = 4,
  };

  uint             m_nTableSize;
  uint             m_nTableMask;
  uint             m_nNumOfElements;
  int64            m_nNextFreeElement;
  Bucket         * m_pListHead;
  Bucket         * m_pListTail;
  Bucket         **m_arBuckets;
  mutable uint16   m_flag;

  Bucket *find(int64 h) const;
  Bucket *find(const char *k, int len, int64 prehash) const;

  Bucket ** findForErase(int64 h) const;
  Bucket ** findForErase(const char *k, int len, int64 prehash) const;
  Bucket ** findForErase(Bucket * bucketPtr) const;

  bool nextInsert(CVarRef data);
  bool nextInsertWithRef(CVarRef data);
  bool addLvalImpl(int64 h, Variant **pDest, bool doFind = true);
  bool addLvalImpl(StringData *key, int64 h, Variant **pDest,
                   bool doFind = true);
  bool addValWithRef(int64 h, CVarRef data);
  bool addValWithRef(StringData *key, CVarRef data);

  bool update(int64 h, CVarRef data);
  bool update(litstr key, CVarRef data);
  bool update(StringData *key, CVarRef data);

  void erase(Bucket ** prev, bool updateNext = false);
  ZendArray *copyImpl() const;

  void resize();
  void rehash();

  void prepareBucketHeadsForWrite();

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(ZendArray, SmartAllocatorImpl::NeedRestoreOnce);
  bool calculate(int &size);
  void backup(LinearAllocator &allocator);
  void restore(const char *&data);
  void sweep();
};

class StaticEmptyZendArray : public ZendArray {
public:
  StaticEmptyZendArray() { setStatic();}

  static ZendArray *Get() { return &s_theEmptyArray; }

private:
  static StaticEmptyZendArray s_theEmptyArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_ARRAY_H__
