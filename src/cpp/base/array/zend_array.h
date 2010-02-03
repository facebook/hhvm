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

#ifndef __HPHP_ZEND_ARRAY_H__
#define __HPHP_ZEND_ARRAY_H__

#include <cpp/base/types.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ZendArray : public ArrayData {
public:
  ZendArray(uint nSize = 0);
  virtual ~ZendArray();

  virtual ssize_t size() const { return m_nNumOfElements;}

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;
  virtual bool supportValueRef() const { return true; }

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
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

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1) const;
  virtual Variant get(litstr  k, int64 prehash = -1) const;
  virtual Variant get(CStrRef k, int64 prehash = -1) const;
  virtual Variant get(CVarRef k, int64 prehash = -1) const;

  virtual ssize_t getIndex(int64 k, int64 prehash = -1) const;
  virtual ssize_t getIndex(litstr k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CStrRef k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

  virtual ArrayData *lval(Variant *&ret, bool copy);
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);

  virtual ArrayData *set(int64   k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(litstr  k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy, int64 prehash = -1);

  virtual ArrayData *remove(int64   k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash = -1);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *pop(Variant &value);
  virtual ArrayData *dequeue(Variant &value);
  virtual ArrayData *insert(ssize_t pos, CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetStatic();

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  class Bucket {
  public:
    Bucket();
    ~Bucket();

    int64       h;
    StringData *key;
    Variant     data;
    Bucket     *pListNext;
    Bucket     *pListLast;
    Bucket     *pNext;
    Bucket     *pLast;

    /**
     * Memory allocator methods.
     */
    DECLARE_SMART_ALLOCATION_NOCALLBACKS(Bucket);
    void dump();
  };

private:
  enum OpFlag {
    HASH_UPDATE       =  (1<<0),
    HASH_ADD          =  (1<<1),
    HASH_NEXT_INSERT  =  (1<<2)
  };

  uint     m_nTableSize;
  uint     m_nTableMask;
  uint     m_nNumOfElements;
  ulong    m_nNextFreeElement;
  Bucket * m_pListHead;
  Bucket * m_pListTail;
  Bucket **m_arBuckets;
  bool     m_linear;

  Bucket *find(int64 h) const;
  Bucket *find(const char *k, int len, int64 prehash = -1,
               int64 *h = NULL) const;

  bool update(OpFlag flag, int64 h, CVarRef data, Variant **pDest = NULL);
  bool update(OpFlag flag, litstr key, int64 h, CVarRef data,
              Variant **pDest = NULL);
  bool update(OpFlag flag, StringData *key, int64 h, CVarRef data,
              Variant **pDest = NULL);

  void erase(Bucket *p);
  ZendArray *copyImpl() const;

  void resize();
  void rehash();

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(ZendArray, SmartAllocatorImpl::NeedRestoreOnce);
  bool calculate(int &size);
  void backup(LinearAllocator &allocator);
  void restore(const char *&data);
  void sweep();

  template<class T>
  ArrayData *lvalImpl(const T& k, Variant *&ret, bool copy, int64 prehash) {
    if (copy) {
      ZendArray *a = copyImpl();
      a->update(HASH_ADD, k, prehash, null, &ret);
      return a;
    }
    update(HASH_ADD, k, prehash, null, &ret);
    return NULL;
  }
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
