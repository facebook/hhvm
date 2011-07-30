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
#ifndef __HPHP_ARG_ARRAY_H__
#define __HPHP_ARG_ARRAY_H__

#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArgArray : public ArrayData {
public:
  class Argument {
  public:
    Variant m_val;
    Argument() {}
  };
  class ArgStack {
  public:
    ArgStack();
    void checkSize(int size);
    int m_size;
    int m_alloc;
    Argument *m_stack;
  };
  static DECLARE_THREAD_LOCAL_NO_CHECK(ArgStack, s_stack);
  ArgArray(int size = 0) {
    ArgStack &stack = *s_stack;
    stack.checkSize(size);
    m_nNumOfElements = size;
    m_start = s_stack->m_size;
  }
  Argument *getStack() {
    ArgStack &stack = *s_stack;
    return stack.m_stack + stack.m_size;
  }
  virtual ~ArgArray();

  virtual ssize_t size() const { return m_nNumOfElements; }

  virtual Variant getKey(ssize_t pos) const {
    assert(pos >= 0 && pos < size());
    return pos;
  }
  virtual Variant getValue(ssize_t pos) const {
    ArgStack &stack = *s_stack;
    return (stack.m_stack + m_start + pos)->m_val;
  }
  virtual CVarRef getValueRef(ssize_t pos) const {
    ArgStack &stack = *s_stack;
    return (stack.m_stack + m_start + pos)->m_val;
  }
  virtual bool isVectorData() const { assert(false); }

  // virtual ssize_t iter_begin() const;
  // virtual ssize_t iter_end() const;
  // virtual ssize_t iter_advance(ssize_t prev) const { assert(false); }
  // virtual ssize_t iter_rewind(ssize_t prev) const { assert(false); }

  virtual Variant reset() { assert(false); }
  virtual Variant prev() { assert(false); }
  virtual Variant current() const { assert(false); }
  virtual Variant next() { assert(false); }
  virtual Variant end() { assert(false); }
  virtual Variant key() const { assert(false); }
  virtual Variant value(ssize_t &pos) const { assert(false); }
  virtual Variant each() { assert(false); }

  virtual bool isHead() const { assert(false); }
  virtual bool isTail() const { assert(false); }
  virtual bool isInvalid() const { assert(false); }

  virtual bool exists(int64   k) const { assert(false); }
  virtual bool exists(litstr  k) const { assert(false); } 
  virtual bool exists(CStrRef k) const { assert(false); }
  virtual bool exists(CVarRef k) const { assert(false); }

  virtual bool idxExists(ssize_t idx) const { assert(false); }

  virtual CVarRef get(int64   k, bool error = false) const;
  virtual CVarRef get(litstr  k, bool error = false) const { assert(false); }
  virtual CVarRef get(CStrRef k, bool error = false) const { assert(false); }
  virtual CVarRef get(CVarRef k, bool error = false) const { assert(false); }

  virtual void load(CVarRef k, Variant &v) const { assert(false); }

  virtual ssize_t getIndex(int64 k) const { assert(false); }
  virtual ssize_t getIndex(litstr k) const { assert(false); }
  virtual ssize_t getIndex(CStrRef k) const { assert(false); }
  virtual ssize_t getIndex(CVarRef k) const { assert(false); }

  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          bool checkExist = false) { assert(false); }
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          bool checkExist = false) { assert(false); }
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist = false) { assert(false); }
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist = false) { assert(false); }
  virtual ArrayData *lvalPtr(CStrRef k, Variant *&ret, bool copy,
                             bool create) { assert(false); }
  virtual ArrayData *lvalPtr(int64   k, Variant *&ret, bool copy,
                             bool create) { assert(false); }

  virtual ArrayData *lvalNew(Variant *&ret, bool copy) { assert(false); }

  virtual ArrayData *set(int64   k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *setRef(int64   k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *setRef(CStrRef k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *setRef(CVarRef k, CVarRef v, bool copy) { assert(false); }

  virtual ArrayData *add(int64   k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *add(CStrRef k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *add(CVarRef k, CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *addLval(int64   k, Variant *&ret, bool copy) {
    assert(false);
  }
  virtual ArrayData *addLval(CStrRef k, Variant *&ret, bool copy) {
    assert(false);
  }
  virtual ArrayData *addLval(CVarRef k, Variant *&ret, bool copy) {
    assert(false);
  }

  virtual ArrayData *remove(int64   k, bool copy) { assert(false); }
  virtual ArrayData *remove(CStrRef k, bool copy) { assert(false); }
  virtual ArrayData *remove(CVarRef k, bool copy) { assert(false); }

  virtual ArrayData *copy() const { assert(false); }
  virtual ArrayData *append(CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *appendRef(CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *appendWithRef(CVarRef v, bool copy) { assert(false); }
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy) {
    assert(false);
  }
  virtual ArrayData *pop(Variant &value) { assert(false); }
  virtual ArrayData *dequeue(Variant &value) { assert(false); }
  virtual ArrayData *prepend(CVarRef v, bool copy) { assert(false); }
  virtual void renumber() { assert(false); }
  virtual void onSetStatic() { assert(false); }

  virtual void getFullPos(FullPos &fp) { assert(false); } 
  virtual bool setFullPos(const FullPos &fp) { assert(false); }
  virtual CVarRef currentRef() { assert(false); }
  virtual CVarRef endRef() { assert(false); }
  virtual ArrayData *escalate(bool mutableIteration = false) const {
    // ArgArray doesn't need to be escalated for most of the time.
    return const_cast<ArgArray *>(this);
  }
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(ArgArray);

  ArgArray(const ArgArray &other) { assert(false); }
private:
  int             m_nNumOfElements;
  int             m_start;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARG_ARRAY_H__
