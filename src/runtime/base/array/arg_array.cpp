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

ArgArray::~ArgArray() {
  ArgStack &stack = *s_stack;
  for (Argument *argp = stack.m_stack + m_start;
       argp < stack.m_stack + m_start + m_nNumOfElements; argp++) {
    argp->m_val.unset();
  }
  stack.m_size -= m_nNumOfElements;
  ASSERT(stack.m_size >= 0);
}

///////////////////////////////////////////////////////////////////////////////
}
