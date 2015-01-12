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

#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/runtime/vm/jit/vasm-x64.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

bool isBlockEnd(Vinstr& inst) {
  switch (inst.op) {
    // service request-y things
    case Vinstr::bindjcc1st:
    case Vinstr::bindjmp:
    case Vinstr::fallback:
    case Vinstr::svcreq:
    // control flow
    case Vinstr::jcc:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::jmpi:
    case Vinstr::phijmp:
    case Vinstr::phijcc:
    // terminal
    case Vinstr::ud2:
    case Vinstr::unwind:
    case Vinstr::vinvoke:
    case Vinstr::ret:
    case Vinstr::retctrl:
    case Vinstr::fallthru:
    // arm specific
    case Vinstr::hcunwind:
    case Vinstr::cbcc:
    case Vinstr::tbcc:
    case Vinstr::brk:
      return true;
    default:
      return false;
  }
}

bool checkBlockEnd(Vunit& unit, Vlabel b) {
  assert(!unit.blocks[b].code.empty());
  auto& block = unit.blocks[b];
  auto n = block.code.size();
  for (size_t i = 0; i < n - 1; ++i) {
    assert(!isBlockEnd(block.code[i]));
  }
  assert(isBlockEnd(block.code[n - 1]));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
