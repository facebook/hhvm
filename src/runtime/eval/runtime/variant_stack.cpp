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
#include <runtime/eval/runtime/variant_stack.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

VariantStack::VariantStack() : m_ptr(0), m_cap(400) {
  m_stack = (Variant*)calloc(m_cap, sizeof(Variant));
}

VariantStack::~VariantStack() {
  free(m_stack);
}

void VariantStack::pop(uint n /* = 1 */) {
  ASSERT(m_ptr >= n);
  for (uint i = m_ptr - n; i < m_ptr; i++) {
    m_stack[i].unset();
  }
  m_ptr -= n;
}

Variant VariantStack::topPop() {
  Variant r(top());
  pop();
  return r;
}

Array VariantStack::pull(uint s, uint n) const {
  ASSERT(m_ptr >= s + n);
  Array r = Array::Create();
  for (uint i = 0; i < n; i++) {
    r.append(m_stack[s + i]);
  }
  return r;
}

void VariantStack::clear() {
  ASSERT(m_ptr == 0);
  m_ptr = 0;
}

void VariantStack::grow() {
  uint oldcap = m_cap;
  m_cap *= 2;
  m_stack = (Variant*)realloc(m_stack, m_cap * sizeof(Variant));
  memset(m_stack + oldcap, 0, oldcap * sizeof(Variant));
}

///////////////////////////////////////////////////////////////////////////////
}
}
