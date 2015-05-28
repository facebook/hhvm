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

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

#define O(name, ...)  \
  static_assert(sizeof(name) <= 48, "vasm struct " #name " is too big");
VASM_OPCODES
#undef O
static_assert(sizeof(Vinstr) <= 64, "Vinstr should be <= 64 bytes");

const char* vinst_names[] = {
#define O(name, imms, uses, defs) #name,
  VASM_OPCODES
#undef O
};

///////////////////////////////////////////////////////////////////////////////

bool isBlockEnd(const Vinstr& inst) {
  switch (inst.op) {
    // service request-y things
    case Vinstr::bindcall:
    case Vinstr::contenter:
    case Vinstr::bindjcc1st:
    case Vinstr::bindjmp:
    case Vinstr::fallback:
    case Vinstr::svcreq:
    // control flow
    case Vinstr::jcc:
    case Vinstr::jcci:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::jmpi:
    case Vinstr::phijmp:
    case Vinstr::phijcc:
    // terminal
    case Vinstr::ud2:
    case Vinstr::unwind:
    case Vinstr::vcallstub:
    case Vinstr::vinvoke:
    case Vinstr::ret:
    case Vinstr::vretm:
    case Vinstr::vret:
    case Vinstr::leavetc:
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

///////////////////////////////////////////////////////////////////////////////
}}
