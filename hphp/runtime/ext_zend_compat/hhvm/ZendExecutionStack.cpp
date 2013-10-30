/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext_zend_compat/hhvm/ZendExecutionStack.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

static __thread HPHP::RequestLocal<ZendExecutionStack> s_stack;

std::vector<ZendStackEntry>& ZendExecutionStack::getStack() {
  return s_stack.get()->m_stack;
}

zval* ZendExecutionStack::getArg(int i) {
  auto& stack = getStack();
  auto& entry = stack.back();
  switch (entry.mode) {
    case ZendStackMode::HHVM_STACK: {
      assert(entry.value == nullptr);
      HPHP::TypedValue *top = HPHP::vmfp() - (i + 1);
      // zPrepArgs should take care of this for us
      assert(top->m_type == HPHP::KindOfRef);
      return top->m_data.pref;
    }

    case ZendStackMode::SIDE_STACK: {
      // Zend puts the number of args as the last thing on the stack
      DEBUG_ONLY int numargs = uintptr_t(entry.value);
      assert(numargs < 4096);
      assert(i < numargs);
      zval* zv = (zval*) stack[stack.size() - 2 - i].value;
      zv->assertValid();
      return zv;
    }
  }
  not_reached();
  return nullptr;
}

void ZendExecutionStack::push(void* z) {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::SIDE_STACK;
  entry.value = z;
  getStack().push_back(entry);
}

void* ZendExecutionStack::pop() {
  auto& stack = getStack();
  auto ret = stack.back();
  stack.pop_back();
  assert(ret.mode == ZendStackMode::SIDE_STACK);
  return ret.value;
}

void ZendExecutionStack::pushHHVMStack() {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::HHVM_STACK;
  entry.value = nullptr;
  getStack().push_back(entry);
}

void ZendExecutionStack::popHHVMStack() {
  auto& stack = getStack();
  DEBUG_ONLY auto& entry = stack.back();
  assert(entry.mode == ZendStackMode::HHVM_STACK);
  stack.pop_back();
}
