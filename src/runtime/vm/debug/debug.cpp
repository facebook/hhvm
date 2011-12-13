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

#include <runtime/base/execution_context.h>

#include "debug.h"
#include "gdb-jit.h"
#include "elfwriter.h"

using namespace HPHP::VM::Transl;

namespace HPHP {
namespace VM {
namespace Debug {

void DebugInfo::recordTracelet(TCA start, TCA end, const Unit *unit,
  const Opcode *instr, bool exit, bool inPrologue) {
  DwarfChunk* chunk = m_dwarfInfo.addTracelet(start, end, unit, instr,
                                              exit, inPrologue);
  if (chunk->m_functions.size() == BASE_FUNCS_PER_CHUNK) {
    ElfWriter e = ElfWriter(chunk);
  }
}

void DebugInfo::debugSync() {
  if (m_dwarfInfo.m_dwarfChunks.size() != 0) {
    ElfWriter(m_dwarfInfo.m_dwarfChunks[0]);
  }
}

}
}
}
