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

#include <runtime/base/array/arg_array.h>
#include <runtime/base/array/array_iterator.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(ArgArray);
IMPLEMENT_THREAD_LOCAL_NO_CHECK(ArgArray::ArgStack, ArgArray::s_stack);

#define INITIAL_STACK_SIZE 4096
#define INC_STACK_SIZE 1024

ArgArray::ArgStack::ArgStack() : m_size(0), m_alloc(INITIAL_STACK_SIZE) {
  m_stack = new Argument[INITIAL_STACK_SIZE];
}

void ArgArray::ArgStack::checkSize(int size) {
  m_size += size;
  if (m_size < m_alloc) return;
  m_alloc += INC_STACK_SIZE;
  if (size > INC_STACK_SIZE) {
    m_alloc += size - size % INC_STACK_SIZE;
  }
  Argument *args = new Argument[m_alloc];
  for (int i = 0; i < m_size - size; i++) {
    args[i].m_val = m_stack[i].m_val;
  }
  delete m_stack;
  m_stack = args;
}

bool ArgArray::exists(int64 k) const {
  return (k >= 0 && k < size());
}

bool ArgArray::exists(litstr k) const {
  return false;
}

bool ArgArray::exists(CStrRef k) const {
  return false;
}

bool ArgArray::exists(CVarRef k) const {
  if (!k.isNumeric()) return false;
  int64 intKey = k.toInt64();
  return (intKey >= 0 && intKey < size());
}

CVarRef ArgArray::get(int64   k, bool error /* = false */) const {
  if (k >= 0 && k < size()) {
    ArgStack &stack = *s_stack;
    return (stack.m_stack + m_start + k)->m_val;
  }
  if (error) {
    raise_notice("Undefined index: %lld", k);
  }
  return null_variant;
}

CVarRef ArgArray::get(litstr  k, bool error /* = false */) const {
  if (error) {
    raise_notice("Undefined index: %s", k);
  }
  return null_variant;
}

CVarRef ArgArray::get(CStrRef k, bool error /* = false */) const {
  if (error) {
    raise_notice("Undefined index: %s", k->data());
  }
  return null_variant;
}

CVarRef ArgArray::get(CVarRef k, bool error /* = false */) const {
  if (k.isNumeric()) {
    return get(k.toInt64(), error);
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null_variant;
}

ssize_t ArgArray::getIndex(int64 k) const {
  if (k >= 0 && k < size()) return k;
  return ArrayData::invalid_index;
}

ssize_t ArgArray::getIndex(litstr k) const {
  return ArrayData::invalid_index;
}

ssize_t ArgArray::getIndex(CStrRef k) const {
  return ArrayData::invalid_index;
}

ssize_t ArgArray::getIndex(CVarRef k) const {
  if (k.isNumeric()) return getIndex(k.toInt64());
  return ArrayData::invalid_index;
}

ArgArray::~ArgArray() {
  ArgStack &stack = *s_stack;
  for (Argument *argp = stack.m_stack + m_start;
       argp < stack.m_stack + m_start + m_nNumOfElements; argp++) {
    argp->m_val.unset();
  }
  stack.m_size -= m_nNumOfElements;
  ASSERT(stack.m_size >= 0);
}

ArrayData *ArgArray::escalateToZendArray() const {
  ZendArray *ret = NEW(ZendArray)(m_nNumOfElements);
  for(int i = 0; i < size(); i++) {
    Variant *v;
    ret->addLval(i, v, false);
    ArgStack &stack = *s_stack;
    v->setWithRef((stack.m_stack + m_start + i)->m_val);
  }
  return ret;
}

ArrayData *ArgArray::lvalNew(Variant *&ret, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lvalNew(ret, false);
  return a;
}

ArrayData *ArgArray::lval(int64 k, Variant *&ret, bool copy,
                          bool checkExist /* = false */) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lval(k, ret, false);
  return a;
}

ArrayData *ArgArray::lval(litstr k, Variant *&ret, bool copy,
                          bool checkExist /* = false */) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lval(k, ret, false);
  return a;
}

ArrayData *ArgArray::lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist /* = false */) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lval(k, ret, false);
  return a;
}

ArrayData *ArgArray::lvalPtr(CStrRef k, Variant *&ret, bool copy,
                             bool create) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lvalPtr(k, ret, false, create);
  return a;
}

ArrayData *ArgArray::lvalPtr(int64 k, Variant *&ret, bool copy,
                             bool create) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->lvalPtr(k, ret, false, create);
  return a;
}

ArrayData *ArgArray::lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist /* = false */) {
  ASSERT(copy);
  if (k.isNumeric()) {
    return lval(k.toInt64(), ret, copy, checkExist);
  }
  return lval(k.toString(), ret, copy, checkExist);
}

ArrayData *ArgArray::set(int64 k, CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->set(k, v, false);
  return a;
}

ArrayData *ArgArray::set(CStrRef k, CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->set(k, v, false);
  return a;
}

ArrayData *ArgArray::set(CVarRef k, CVarRef v, bool copy) {
  ASSERT(copy);
  if (k.isNumeric()) {
    return set(k.toInt64(), v, copy);
  }
  return set(k.toString(), v, copy);
}

ArrayData *ArgArray::setRef(int64 k, CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->setRef(k, v, false);
  return a;
}

ArrayData *ArgArray::setRef(CStrRef k, CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->setRef(k, v, false);
  return a;
}

ArrayData *ArgArray::setRef(CVarRef k, CVarRef v, bool copy) {
  ASSERT(copy);
  if (k.isNumeric()) {
    return setRef(k.toInt64(), v, copy);
  }
  return setRef(k.toString(), v, copy);
}

ArrayData *ArgArray::copy() const {
  return escalateToZendArray();
}

ArrayData *ArgArray::append(CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->append(v, false);
  return a;
}

ArrayData *ArgArray::appendRef(CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->appendRef(v, false);
  return a;
}

ArrayData *ArgArray::appendWithRef(CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->appendWithRef(v, false);
  return a;
}

ArrayData *ArgArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->append(elems, op, false);
  return a;
}

ArrayData *ArgArray::remove(int64 k, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->remove(k, false);
  return a;
}

ArrayData *ArgArray::remove(CStrRef k, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->remove(k, false);
  return a;
}

ArrayData *ArgArray::remove(CVarRef k, bool copy) {
  ASSERT(copy);
  if (k.isNumeric()) {
    return remove(k.toInt64(), copy);
  }
  return remove(k.toString(), copy);
}

ArrayData *ArgArray::prepend(CVarRef v, bool copy) {
  ASSERT(copy);
  ArrayData *a = escalateToZendArray();
  a->prepend(v, false);
  return a;
}

///////////////////////////////////////////////////////////////////////////////
}
