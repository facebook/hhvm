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

#ifndef __HPHP_HPHP_ARRAY_H__
#define __HPHP_HPHP_ARRAY_H__

#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;

class HphpArray : public ArrayData {
public:
  friend class ArrayInit;

public:
  HphpArray(uint nSize = 0);
private:
  HphpArray(int,int);
public:
  virtual ~HphpArray();

  virtual ssize_t size() const;

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  ssize_t iter_advance_helper(ssize_t prev) const ATTRIBUTE_COLD;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual Variant key() const;
  virtual Variant value(ssize_t& pos) const;
  virtual Variant each();

  virtual bool isHead() const;
  virtual bool isTail() const;
  virtual bool isInvalid() const { return m_pos == invalid_index; }

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual CVarRef get(int64   k, bool error=false) const
    __attribute__((flatten));
  virtual CVarRef get(litstr  k, bool error=false) const
    __attribute__((flatten));
  virtual CVarRef get(CStrRef k, bool error=false) const
    __attribute__((flatten));
  virtual CVarRef get(CVarRef k, bool error=false) const
    __attribute__((flatten));

  virtual void load(CVarRef k, Variant& v) const;

  Variant fetch(CStrRef k) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const;
  virtual ssize_t getIndex(CStrRef k) const;
  virtual ssize_t getIndex(CVarRef k) const;

  virtual ArrayData* lval(int64   k, Variant*& ret, bool copy,
                          bool checkExist=false);
  virtual ArrayData* lval(litstr  k, Variant*& ret, bool copy,
                          bool checkExist=false);
  virtual ArrayData* lval(CStrRef k, Variant*& ret, bool copy,
                          bool checkExist=false);
  virtual ArrayData* lval(CVarRef k, Variant*& ret, bool copy,
                          bool checkExist=false);
  virtual ArrayData* lvalPtr(CStrRef k, Variant*& ret, bool copy,
                             bool create);
  virtual ArrayData* lvalPtr(int64   k, Variant*& ret, bool copy,
                             bool create);

  virtual ArrayData* lvalNew(Variant*& ret, bool copy);

  virtual ArrayData* set(int64   k, CVarRef v, bool copy);
  virtual ArrayData* set(litstr  k, CVarRef v, bool copy);
  virtual ArrayData* set(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData* set(CVarRef k, CVarRef v, bool copy);

  virtual ArrayData *add(int64   k, CVarRef v, bool copy);
  virtual ArrayData *add(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *add(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *addLval(int64   k, Variant*& ret, bool copy);
  virtual ArrayData *addLval(CStrRef k, Variant*& ret, bool copy);
  virtual ArrayData *addLval(CVarRef k, Variant*& ret, bool copy);

  virtual ArrayData* remove(int64   k, bool copy);
  virtual ArrayData* remove(litstr  k, bool copy);
  virtual ArrayData* remove(CStrRef k, bool copy);
  virtual ArrayData* remove(CVarRef k, bool copy);

  virtual ArrayData* copy() const;
  virtual ArrayData* append(CVarRef v, bool copy);
  virtual ArrayData* appendWithRef(CVarRef v, bool copy);
  virtual ArrayData* append(const ArrayData* elems, ArrayOp op, bool copy);
  virtual ArrayData* pop(Variant& value);
  virtual ArrayData* dequeue(Variant& value);
  virtual ArrayData* prepend(CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetStatic();

  virtual void getFullPos(FullPos& fp);
  virtual bool setFullPos(const FullPos& fp);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  /**
   * Assumes 'tv' is dead and preserves the element's original value
   * if the key is already present in the array. If 'tv' is NULL, this
   * method will migrate the element back to the array.
   *
   * Returns the previous fixed memory location that the element lived at,
   * or NULL if the element used to live in the array.
   */
  TypedValue* migrate(StringData* k, TypedValue* tv);

  /**
   * Assumes 'tv' is live, overwrites the element's value if the key
   * is already present in the array. 'tv' must not be NULL.
   *
   * Returns the previous fixed memory location that the element lived at,
   * or NULL if the element used to live in the array.
   */
  TypedValue* migrateAndSet(StringData* k, TypedValue* tv);

  // Used in Elm's data.m_type field to denote an invalid Elm.
  static const HPHP::DataType KindOfTombstone = MaxNumDataTypes;
  static const HPHP::DataType KindOfIndirect =
      (HPHP::DataType)(MaxNumDataTypes + 1);

  // Array element.
  struct Elm {
    int64       h;    // (key == NULL) ? <integer key value> : <string hash>
    StringData* key;  // (key != NULL) ? <string key value>
    TypedValue  data; // (data.m_type != KindOfTombstone) ? <value> : <invalid>
  };

  // Element index, with special values < 0 used for hash tables.
  // NOTE: Unfortunately, g++ on x64 tends to generate worse machine code for
  // 32-bit ints than it does for 64-bit ints. As such, we have deliberately
  // chosen to use ssize_t in some places where ideally we *should* have used
  // ElmInd.
  typedef int32 ElmInd;
  static const ElmInd ElmIndEmpty      = -1; // == ArrayData::invalid_index
  static const ElmInd ElmIndTombstone  = -2;

  // Use a minimum of an 8-element hash table.  Valid range: [2..32]
  static const uint32 MinLgTableSize   = 3;

  // The element array is aligned such that elements do not needlessly straddle
  // cacheline boundaries.
  static const size_t ElmAlignment     = sizeof(Elm);
  static const size_t ElmAlignmentMask = ElmAlignment-1;

private:
  // Array elements and the hash table are contiguously allocated, such that
  // elements are naturally aligned.  If necessary, m_data starts and ends with
  // padding in order to meet the element alignment requirements.
  //
  // Given 2^K == m_tableMask+1 && K >= 2 && K <= 32 &&
  //       block == (void*)(uintptr_t(m_data) - uintptr_t(m_dataPad)):
  //
  //            +--------------------+
  // block -->  | alignment padding? |
  //            +--------------------+
  // m_data --> | slot 0             | 0.75 * 2^K slots for elements.
  //            | slot 1             |
  //            | ...                |
  //            +--------------------+
  // m_hash --> |                    | 2^K hash table entries.
  //            +--------------------+
  //            | alignment padding? |
  //            +--------------------+
  void*   m_data;        // Contains elements and hash table.
  ElmInd* m_hash;        // Hash table.
  int64   m_nextKI;      // Next integer key to use for append.
  uint32  m_tableMask;   // Bitmask used when indexing into the Hash table.
  ElmInd  m_nElms;       // Total number of elements in array.
  uint32  m_hLoad;       // Hash table load (# of non-empty slots).
  ElmInd  m_lastE;       // Index of last used element.
  char    m_linear;      // (true) ? m_data came from linear allocator.
  char    m_siPastEnd;   // (true) ? strong iterators possibly past end.
#ifndef USE_JEMALLOC
  uchar   m_dataPad;     // Number of bytes that m_data was advanced to
                         //   achieve the required alignment
#endif
  ElmInd  m_nIndirectElms; // Total number of elements in the array with
                           //   m_type == KindOfIndirect

  inline void* getBlock() const {
    return ((void*)(uintptr_t(m_data)
#ifndef USE_JEMALLOC
            - uintptr_t(m_dataPad)
#endif
            ));
  }

  void dumpDebugInfo() const;

  ssize_t /*ElmInd*/ nextElm(Elm* elms, ssize_t /*ElmInd*/ ei) const;
  ssize_t /*ElmInd*/ prevElm(Elm* elms, ssize_t /*ElmInd*/ ei) const;

  inline ssize_t /*ElmInd*/ find(int64 ki) const;
  inline ssize_t /*ElmInd*/ find(const char* k, int len, int64 prehash) const;
  inline ElmInd* findForInsert(int64 ki) const;
  inline ElmInd* findForInsert(const char* k, int len, int64 prehash) const;

  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution.
   */
  inline ElmInd* ALWAYS_INLINE findForNewInsert(size_t h0) const;

  bool nextInsert(CVarRef data);
  bool nextInsertWithRef(CVarRef data);
  bool addLvalImpl(int64 ki, Variant** pDest, bool doFind=true);
  bool addLvalImpl(StringData* key, int64 h, Variant** pDest, bool doFind=true);
  bool addVal(int64 ki, CVarRef data, bool checkExists=true);
  bool addVal(StringData* key, CVarRef data, bool checkExists=true);
  bool addValWithRef(int64 ki, CVarRef data, bool checkExists=true);
  bool addValWithRef(StringData* key, CVarRef data, bool checkExists=true);

  bool update(int64 ki, CVarRef data);
  bool update(litstr key, CVarRef data);
  bool update(StringData* key, CVarRef data);

  void erase(ElmInd* ei, bool updateNext = false);
  HphpArray* copyImpl() const;

  inline Elm* ALWAYS_INLINE allocElm(ElmInd* ei);
  void reallocData(size_t maxElms, size_t tableSize);
  void delinearize() ATTRIBUTE_COLD;

  /**
   * grow() increases the hash table size and the number of slots for
   * elements by a factor of 2. grow() rebuilds the hash table, but it
   * does not compact the elements.
   */
  void grow() ATTRIBUTE_COLD;

  /**
   * compact() does not change the hash table size or the number of slots
   * for elements. compact() rebuilds the hash table and compacts the
   * elements into the slots with lower addresses.
   */
  void compact(bool renumber=false) ATTRIBUTE_COLD;

  /**
   * resize() and resizeIfNeeded() will grow or compact the array as
   * necessary to ensure that there is room for a new element and a
   * new hash entry.
   *
   * resize() assumes that the array does not have room for a new element
   * or a new hash entry. resizeIfNeeded() will first check if there is room
   * for a new element and hash entry before growing or compacting the array.
   */
  void resize();
  inline void ALWAYS_INLINE resizeIfNeeded();

  // Memory allocator methods.
  DECLARE_SMART_ALLOCATION(HphpArray, SmartAllocatorImpl::NeedRestoreOnce);
  bool calculate(int& size);
  void backup(LinearAllocator& allocator);
  void restore(const char*& data);
  void sweep();
};

class StaticEmptyHphpArray : public HphpArray {
public:
  StaticEmptyHphpArray() {
    setStatic();
  }

  static HphpArray* Get() {
    return &s_theEmptyArray;
  }

private:
  static StaticEmptyHphpArray s_theEmptyArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHP_ARRAY_H__
