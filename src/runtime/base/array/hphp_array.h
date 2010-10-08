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

  virtual ssize_t size() const { return (ssize_t)m_nElms;}

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual void fetchValue(ssize_t pos, Variant& v) const;
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
  virtual Variant value(ssize_t& pos) const;
  virtual Variant each();

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, bool error=false) const;
  virtual Variant get(litstr  k, bool error=false) const;
  virtual Variant get(CStrRef k, bool error=false) const;
  virtual Variant get(CVarRef k, bool error=false) const;

  virtual void load(CVarRef k, Variant& v) const;

  Variant fetch(CStrRef k) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const;
  virtual ssize_t getIndex(CStrRef k) const;
  virtual ssize_t getIndex(CVarRef k) const;

  virtual ArrayData* lval(Variant*& ret, bool copy);
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
  virtual ArrayData* append(const ArrayData* elems, ArrayOp op, bool copy);
  virtual ArrayData* pop(Variant& value);
  virtual ArrayData* dequeue(Variant& value);
  virtual ArrayData* prepend(CVarRef v, bool copy);
  virtual void renumber();
  virtual void onSetStatic();

  virtual void getFullPos(FullPos& pos);
  virtual bool setFullPos(const FullPos& pos);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  // Used in Elm's data.m_type field to denote an invalid Elm.
  static const HPHP::DataType KindOfTombstone = MaxNumDataTypes;

  // Array element.
  struct Elm {
    int64       h;    // (key == NULL) ? <integer key value> : <string hash>
    StringData* key;  // (key != NULL) ? <string key value>
    TypedValue  data; // (data.m_type != KindOfTombstone) ? <value> : <invalid>
  };

  // Element index, with special values < 0 used for hash tables.
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
  // Element addresses are computed relative to m_hash.
  //
  //            +--------------------+
  // m_data --> | alignment padding? |
  //            +--------------------+
  // element 0  |                    | 2^m_lgTableSize - 2^(m_lgTableSize-2)
  // element 1  |                    | elements.
  // ...        |                    |
  //            +--------------------+
  // m_hash --> |                    | 2^m_lgTableSize hash table entries.
  //            +--------------------+
  //            | alignment padding? |
  //            +--------------------+
  void*   m_data;        // Contains elements and hash table.
  ElmInd* m_hash;        // Hash table.
  int64   m_nextKI;      // Next integer key to use for append.
  uint32  m_lgTableSize; // Hash table has 2^m_lgTableSize slots.
  ElmInd  m_nElms;       // Total number of elements in array.
  uint32  m_hLoad;       // Hash table load (# of non-empty slots).
  ElmInd  m_lastE;       // Index of last used element.
  bool    m_linear;      // (true) ? m_data came from linear allocator.
  bool    m_siPastEnd;   // (true) ? strong iterators possibly past end.

  void dumpDebugInfo() const;

  ElmInd nextElm(Elm* elms, ElmInd ei) const;

  ElmInd find(int64 ki) const;
  ElmInd find(const char* k, int len, int64 prehash) const;
  ElmInd* findForInsert(int64 ki) const;
  ElmInd* findForInsert(const char* k, int len, int64 prehash) const;

  bool nextInsert(CVarRef data);
  bool addLvalImpl(int64 ki, Variant** pDest, bool doFind=true);
  bool addLvalImpl(StringData* key, int64 h, Variant** pDest, bool doFind=true);
  bool addVal(int64 ki, CVarRef data, bool checkExists=true);
  bool addVal(StringData* key, CVarRef data, bool checkExists=true);

  bool update(int64 ki, CVarRef data);
  bool update(litstr key, CVarRef data);
  bool update(StringData* key, CVarRef data);

  void erase(ElmInd* ei);
  HphpArray* copyImpl() const;

  Elm* allocElm(ElmInd* ei);
  void reallocData(size_t maxElms, size_t tableSize);
  void delinearize();
  inline void resize();
  void grow();
  void compact(bool renumber=false);

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

#endif // __HPHP_ZEND_ARRAY_H__
