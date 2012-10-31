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
#include <runtime/base/comparisons.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;

class ZendArray : public ArrayData {
  static const uint LgMinSize = 3;
  static const uint MinSize = 1 << LgMinSize;
  enum Flag { StrongIteratorPastEnd = 1 };
  enum AllocMode { kInline, kSmart, kMalloc };
  enum SortFlavor { IntegerSort, StringSort, GenericSort };
public:
  friend class ArrayInit;
  friend class VectorArray;

  ZendArray() : m_arBuckets(m_inlineBuckets), m_nTableMask(MinSize - 1),
    m_allocMode(kInline), m_nonsmart(false), m_pListHead(0), m_pListTail(0),
    m_nNextFreeElement(0) {
    m_size = 0;
    memset(m_inlineBuckets, 0, MinSize * sizeof(Bucket*));
  }

  ZendArray(uint nSize, bool nonsmart = false);
  virtual ~ZendArray();

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::get;
  using ArrayData::getIndex;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::lvalPtr;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;

  virtual ssize_t vsize() const ATTRIBUTE_COLD;

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;

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

  virtual bool isInvalid() const { return !m_pos; }

  virtual bool exists(int64 k) const;
  virtual bool exists(const StringData* k) const;

  virtual CVarRef get(int64 k, bool error = false) const;
  virtual CVarRef get(const StringData* k, bool error = false) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(const StringData* k) const;

  virtual ArrayData *lval(int64 k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(StringData* k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lvalPtr(int64 k, Variant *&ret, bool copy,
                             bool create);
  virtual ArrayData *lvalPtr(StringData* k, Variant *&ret, bool copy,
                             bool create);

  virtual ArrayData *lvalNew(Variant *&ret, bool copy);

  virtual ArrayData *set(int64 k, CVarRef v, bool copy);
  virtual ArrayData *set(StringData* k, CVarRef v, bool copy);
  virtual ArrayData *setRef(int64 k, CVarRef v, bool copy);
  virtual ArrayData *setRef(StringData* k, CVarRef v, bool copy);

  virtual ArrayData *add(int64 k, CVarRef v, bool copy);
  virtual ArrayData *add(StringData* k, CVarRef v, bool copy);
  virtual ArrayData *addLval(int64 k, Variant *&ret, bool copy);
  virtual ArrayData *addLval(StringData* k, Variant *&ret, bool copy);

  virtual ArrayData *remove(int64 k, bool copy);
  virtual ArrayData *remove(const StringData* k, bool copy);

  virtual ArrayData *copy() const;
  virtual ArrayData *copyWithStrongIterators() const;
  virtual ArrayData *nonSmartCopy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *appendRef(CVarRef v, bool copy);
  virtual ArrayData *appendWithRef(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *pop(Variant &value);
  virtual ArrayData *dequeue(Variant &value);
  virtual ArrayData *prepend(CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetEvalScalar();

  virtual void getFullPos(FullPos &fp);
  virtual bool setFullPos(const FullPos &fp);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  class Bucket {
  public:
    Bucket() :
      ikey(0), pListNext(NULL), pListLast(NULL), pNext(NULL) {
      data._count = 0;
    }

    Bucket(Variant::NoInit d) :
      ikey(0), data(d), pListNext(NULL), pListLast(NULL), pNext(NULL) {
      data._count = 0;
    }

    Bucket(CVarRef d) :
      ikey(0), data(d), pListNext(NULL), pListLast(NULL), pNext(NULL) {
      data._count = 0;
    }

    Bucket(CVarStrongBind d) :
      ikey(0), data(d), pListNext(NULL), pListLast(NULL), pNext(NULL) {
      data._count = 0;
    }

    Bucket(CVarWithRefBind d) :
      ikey(0), data(d), pListNext(NULL), pListLast(NULL), pNext(NULL) {
      data._count = 0;
    }

    // set the top bit for string hashes to make sure the hash
    // value is never zero. hash value 0 corresponds to integer key.
    static inline int32_t encodeHash(strhash_t h) {
      return int32_t(h) | 0x80000000;
    }

    // These special constructors do not setup all the member fields.
    // They cannot be used along but must be with the following special
    // ZendArray constructor
    Bucket(StringData *k, CVarRef d) :
      skey(k), data(d) {
      ASSERT(k->isStatic());
      data._count = encodeHash(k->getPrecomputedHash());
    }
    Bucket(int64 k, CVarRef d) : ikey(k), data(d) {
      data._count = 0;
    }
    Bucket(int64 k, CVarWithRefBind d) : ikey(k), data(d) {
      data._count = 0;
    }

    ~Bucket();

    /* The key is either a string pointer or an int value, and the _count
     * field in data is used to discriminate the key type. _count = 0 means
     * int, nonzero values contain 31 bits of a string's hashcode.
     * It is critical that when we return &data to clients, that they not
     * read or write the _count field! */
    union {
      int64      ikey;
      StringData *skey;
    };
    Variant     data;
    inline bool hasStrKey() const { return data._count != 0; }
    inline bool hasIntKey() const { return data._count == 0; }
    inline void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      skey->incRefCount();
      data._count = encodeHash(h);
    }
    inline void setIntKey(int64 k) {
      ikey = k;
      data._count = 0;
    }
    inline int64 hashKey() const {
      return data._count == 0 ? ikey : data._count;
    }
    inline int32_t hash() const {
      return data._count;
    }

    Bucket     *pListNext;
    Bucket     *pListLast;
    Bucket     *pNext;

    /**
     * Memory allocator methods.
     */
    DECLARE_SMART_ALLOCATION(Bucket);
    void dump();
  };

  // This constructor should never be called directly, it is only called
  // from generated code.
  ZendArray(uint nSize, int64 n, Bucket *bkts[]);

private:
  Bucket         **m_arBuckets;
  uint             m_nTableMask;
  uint8_t          m_allocMode;
  const bool       m_nonsmart;
  Bucket         * m_pListHead;
  Bucket          *m_inlineBuckets[MinSize];
  Bucket         * m_pListTail;
  int64            m_nNextFreeElement;

  uint tableSize() const { return m_nTableMask + 1; }

  Bucket *find(int64 h) const;
  Bucket *find(const char *k, int len, strhash_t prehash) const;
  Bucket *findForInsert(int64 h) const;
  Bucket *findForInsert(const char *k, int len, strhash_t prehash) const;

  Bucket ** findForErase(int64 h) const;
  Bucket ** findForErase(const char *k, int len, strhash_t prehash) const;
  Bucket ** findForErase(Bucket * bucketPtr) const;

  bool nextInsert(CVarRef data);
  bool nextInsertWithRef(CVarRef data);
  bool nextInsertRef(CVarRef data);
  bool addLvalImpl(int64 h, Variant **pDest, bool doFind = true);
  bool addLvalImpl(StringData *key, strhash_t h, Variant **pDest,
                   bool doFind = true);
  bool addValWithRef(int64 h, CVarRef data);
  bool addValWithRef(StringData *key, CVarRef data);

  bool update(int64 h, CVarRef data);
  bool update(StringData *key, CVarRef data);
  bool updateRef(int64 h, CVarRef data);
  bool updateRef(StringData *key, CVarRef data);

  void erase(Bucket ** prev, bool updateNext = false);
  ZendArray *copyImpl() const;
  ZendArray *copyImplHelper(bool sma) const;

  void init(uint nSize);
  void resize();
  void rehash();

  template <typename AccessorT>
  SortFlavor preSort(Bucket** buffer, const AccessorT& acc, bool checkTypes);

  void postSort(Bucket** buffer, bool resetKeys);

public:
  ArrayData* escalateForSort();
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  void uksort(CVarRef cmp_function);
  void usort(CVarRef cmp_function);
  void uasort(CVarRef cmp_function);

private:
  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(ZendArray);
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
