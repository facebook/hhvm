/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

zval** ZendExecutionStack::getArg(size_t i) {
  auto& stack = getStack();
  auto& entry = stack.m_stack.back();
  switch (entry.mode) {
    case ZendStackMode::HHVM_STACK: {
      ActRec* ar = entry.ar;
      const auto numNonVaradic = ar->m_func->numNonVariadicParams();
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
      auto numargs = entry.numargs;
      assert(numargs < 4096);
      assert(i < numargs);
      auto zvpp = &stack.m_stack[stack.m_stack.size() - 1 - numargs + i].zvp;
      (*zvpp)->assertValid();
      return zvpp;
    }
  }
  not_reached();
  return nullptr;
}

size_t ZendExecutionStack::numArgs() {
  auto& stack = getStack();
  auto& entry = stack.m_stack.back();
  switch (entry.mode) {
    case ZendStackMode::HHVM_STACK:
      return entry.ar->numArgs();
    case ZendStackMode::SIDE_STACK:
      // Zend puts the number of args as the last thing on the stack
      return entry.numargs;
  }
  not_reached();
  return 0;
}

void ZendExecutionStack::push(void* z) {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::SIDE_STACK;
  entry.zvp = (zval*)z;
  getStack().m_stack.push_back(entry);
}

void* ZendExecutionStack::pop() {
  auto& stack = getStack();
  auto ret = stack.m_stack.back();
  stack.m_stack.pop_back();
  assert(ret.mode == ZendStackMode::SIDE_STACK);
  return ret.zvp;
}

void ZendExecutionStack::pushHHVMStack(ActRec* ar) {
  ZendStackEntry entry;
  entry.mode = ZendStackMode::HHVM_STACK;
  entry.ar = ar;
  getStack().m_stack.push_back(entry);
}

void ZendExecutionStack::popHHVMStack() {
  auto& stack = getStack();
  assert(stack.m_stack.back().mode == ZendStackMode::HHVM_STACK);
  stack.m_stack.pop_back();
}

}
