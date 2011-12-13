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
#ifndef _TRANSLATOR_DEBUG_H_
#define _TRANSLATOR_DEBUG_H_

#include <runtime/base/types.h>
#include <runtime/vm/translator/translator.h>
#include "dwarf.h"

namespace HPHP {
namespace VM {
namespace Debug {

using namespace HPHP::VM::Transl;

struct DebugInfo {
  DwarfInfo m_dwarfInfo;
  void recordTracelet(TCA start, TCA end, const Unit *unit,
    const Opcode *instr, bool exit, bool inPrologue);
  void debugSync();
  DebugInfo() {}
};


}
}
}

#endif
