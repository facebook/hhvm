/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_JIT_TEST_MATCH_H
#define incl_HPHP_VM_JIT_TEST_MATCH_H

#include "hphp/runtime/vm/jit/ir-instruction.h"

#include <gtest/gtest.h>

#define EXPECT_MATCH(...) EXPECT_TRUE(match(__VA_ARGS__))

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////
// match()
//
// Use this function to pattern-match against an IR instruction.

inline bool match(const IRInstruction*) { return true; }

template<typename... Args>
bool match(const IRInstruction* inst, Opcode op, Args... args) {
  return (inst->op() == op) && match(inst, std::forward<Args>(args)...);
}

template<typename... Args>
bool match(const IRInstruction* inst, Type type, Args... args) {
  return (inst->hasTypeParam() && inst->typeParam() == type) &&
    match(inst, std::forward<Args>(args)...);
}

template<typename... Args>
bool match(const IRInstruction* inst, Block* taken, Args... args) {
  return (inst->taken() == taken) && match(inst, std::forward<Args>(args)...);
}

template<typename... SSATmps>
bool match(const IRInstruction* inst, SSATmp* hd, SSATmps... tl) {
  if (inst->numSrcs() < sizeof...(tl) + 1) {
    return false;
  }

  auto tmpsSame = [] (const SSATmp* expected, const SSATmp* actual) {
    // If a tmp is expected to have a constant value, identity comparison (i.e.
    // by pointer) is too restrictive; compare the constant values.
    auto const type = expected->type();
    if (type.hasConstVal() ||
        type.subtypeOfAny(TUninit, TInitNull, TNullptr)) {
      return type == actual->type();
    }
    return expected == actual;
  };

  int i = 0;
  for (auto const* src : {hd, tl...}) {
    if (!tmpsSame(src, inst->src(i))) {
      return false;
    }
    ++i;
  }
  return true;
}

template<typename ... Args>
bool match(const IRInstruction& inst, Args... args) {
  return match(&inst, std::forward<Args>(args)...);
}

}}

#endif
