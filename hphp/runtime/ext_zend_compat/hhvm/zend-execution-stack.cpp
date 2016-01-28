/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-execution-stack.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"
#include <vector>
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

IMPLEMENT_STATIC_REQUEST_LOCAL(ZendExecutionStack, tl_stack);

ZendExecutionStack & ZendExecutionStack::getStack() {
  return *tl_stack.get();
}

zval** ZendExecutionStack::getArg(int i) {
  auto& stack = getStack();
  auto& entry = stack.m_stack.back();
  switch (entry.mode) {
    case ZendStackMode::HHVM_STACK: {
      ActRec* ar = (ActRec*)entry.value;
      const int numNonVaradic = ar->m_func->numNonVariadicParams();
      TypedValue* arg;
      if (i < numNonVaradic) {
        arg = (TypedValue*)ar - i - 1;
      } else if (i < ar->numArgs()) {
        arg = ar->getExtraArg(i - numNonVaradic);
      } else {
        if (!stack.m_nullArg) {
          stack.m_nullArg = RefData::Make(make_tv<KindOfNull>());
        }
        return &stack.m_nullArg;
      }

      zBoxAndProxy(arg);
      // The 'Z' type specifier in zend_parse_parameters() demands a zval**
      // which remains valid until the caller returns. We will give it a
      // pointer to the pref member of the TypedValue which is stored on the
      // HHVM stack.
      return &arg->m_data.pref;
    }

    case ZendStackMode::SIDE_STACK: {
      // Zend puts the number of args as the last thing on the stack
      int numargs = uintptr_t(entry.value);
      assert(numargs < 4096);
      assert(i < numargs);
      zval** zvpp =
        (zval**) &stack.m_stack[stack.m_stack.size() - 1 - numargs + i].value;
      (*zvpp)->assertValid();
      return zvpp;
    }
  }
  not_reached();
  return nullptr;
}

int32_t ZendExecutionStack::numArgs() {
  auto& stack = getStack();
  auto& entry = stack.m_stack.back();
  switch (entry.mode) {
    case ZendStackMode::HHVM_STACK:
      return ((ActRec*)entry.value)->numArgs();
    case ZendStackMode::SIDE_STACK:
      // Zend puts the number of args as the last thing on the stack
      return uintptr_t(entry.value);
  }
  not_reached();
  return 0;
}

void ZendExecutionStack::push(void* z) {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::SIDE_STACK;
  entry.value = z;
  getStack().m_stack.push_back(entry);
}

void* ZendExecutionStack::pop() {
  auto& stack = getStack();
  auto ret = stack.m_stack.back();
  stack.m_stack.pop_back();
  assert(ret.mode == ZendStackMode::SIDE_STACK);
  return ret.value;
}

void ZendExecutionStack::pushHHVMStack(void * ar) {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::HHVM_STACK;
  entry.value = ar;
  getStack().m_stack.push_back(entry);
}

void ZendExecutionStack::popHHVMStack() {
  auto& stack = getStack();
  DEBUG_ONLY auto& entry = stack.m_stack.back();
  assert(entry.mode == ZendStackMode::HHVM_STACK);
  stack.m_stack.pop_back();
}

}
