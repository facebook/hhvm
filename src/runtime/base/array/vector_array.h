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
#ifndef __HPHP_VECTOR_ARRAY_H__
#define __HPHP_VECTOR_ARRAY_H__

#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class VectorArray : public ArrayData {
public:
  friend class ArrayData;

  static const uint FixedSize = 4;

  VectorArray(uint size = 0);
  VectorArray(const VectorArray *src, uint start = 0, uint size = 0);
  VectorArray(const VectorArray *src, bool sma) ATTRIBUTE_COLD;

  // This constructor should never be called directly, it is only called
  // from generated code.
  VectorArray(uint size, const Variant *values[]);
  virtual ~VectorArray();

  virtual ssize_t vsize() const ATTRIBUTE_COLD;
  virtual bool isVectorData() const { return true; }
  virtual bool isVectorArray() const { return true; }

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;

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

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;

  virtual CVarRef get(int64   k, bool error = false) const;
  virtual CVarRef get(litstr  k, bool error = false) const;
  virtual CVarRef get(CStrRef k, bool error = false) const;
  virtual CVarRef get(CVarRef k, bool error = false) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const ATTRIBUTE_COLD;
  virtual ssize_t getIndex(CStrRef k) const ATTRIBUTE_COLD;
  virtual ssize_t getIndex(CVarRef k) const ATTRIBUTE_COLD;

  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lvalPtr(CStrRef k, Variant *&ret, bool copy,
                             bool create) ATTRIBUTE_COLD;
  virtual ArrayData *lvalPtr(int64   k, Variant *&ret, bool copy,
                             bool create) ATTRIBUTE_COLD;

  virtual ArrayData *lvalNew(Variant *&ret, bool copy) ATTRIBUTE_COLD;

  virtual ArrayData *set(int64   k, CVarRef v, bool copy);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *setRef(int64   k, CVarRef v, bool copy);
  virtual ArrayData *setRef(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *setRef(CVarRef k, CVarRef v, bool copy);

  virtual ArrayData *add(int64   k, CVarRef v, bool copy);
  virtual ArrayData *add(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *add(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *addLval(int64   k, Variant *&ret, bool copy);
  virtual ArrayData *addLval(CStrRef k, Variant *&ret, bool copy);
  virtual ArrayData *addLval(CVarRef k, Variant *&ret, bool copy);

  virtual ArrayData *remove(int64   k, bool copy);
  virtual ArrayData *remove(CStrRef k, bool copy);
  virtual ArrayData *remove(CVarRef k, bool copy);

  virtual ArrayData *copy() const;
  virtual ArrayData *nonSmartCopy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *appendRef(CVarRef v, bool copy);
  virtual ArrayData *appendWithRef(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *pop(Variant &value);
  virtual ArrayData *dequeue(Variant &value);
  virtual ArrayData *prepend(CVarRef v, bool copy);
  virtual void onSetEvalScalar();

  virtual void getFullPos(FullPos &fp);
  virtual bool setFullPos(const FullPos &fp);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();
  virtual ArrayData *escalate(bool mutableIteration = false) const;

  virtual ArrayData* escalateForSort();

  DECLARE_SMART_ALLOCATION(VectorArray);

private:
  enum AllocMode { kInline, kSmart, kMalloc };
  TypedValue     m_fixed[FixedSize];
  TypedValue    *m_elems;
  uint           m_capacity;
  int8_t         m_allocMode; // AllocMode
  const bool     m_nonsmart;

  ZendArray *escalateToNonEmptyZendArray() const NEVER_INLINE;
  ZendArray *escalateToZendArray() const NEVER_INLINE;

  void alloc(uint cap);
  void grow(uint newSize) NEVER_INLINE;
  void checkSize(uint n = 1);
  void checkInsertIterator(ssize_t pos);
};

class StaticEmptyVectorArray : public VectorArray {
public:
  StaticEmptyVectorArray() { setStatic();}
  ~StaticEmptyVectorArray() { }

  static VectorArray *Get() { return &s_theEmptyVectorArray; }

private:
  static StaticEmptyVectorArray s_theEmptyVectorArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VECTOR_ARRAY_H__
