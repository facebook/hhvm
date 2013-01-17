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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <runtime/vm/translator/translator-x64.h>

using namespace HPHP::VM::Transl;

namespace HPHP {
namespace VM {
namespace Debug {

DebugInfo* DebugInfo::Get() {
  return tx64->getDebugInfo();
}

DebugInfo::DebugInfo() {
  snprintf(m_perfMapName,
           sizeof m_perfMapName,
           "/tmp/perf-%d.map", getpid());
  m_perfMap = fopen(m_perfMapName, "w");
}

DebugInfo::~DebugInfo() {
  if (m_perfMap) fclose(m_perfMap);
  unlink(m_perfMapName);
}

void DebugInfo::recordStub(TCRange range, const char* name) {
  if (range.isAstubs()) {
    m_astubsDwarfInfo.addTracelet(range, name, NULL, NULL, false, false);
  } else {
    m_aDwarfInfo.addTracelet(range, name, NULL, NULL, false, false);
  }
}

void DebugInfo::recordPerfMap(TCRange range, const Func* func,
                              bool exit, bool inPrologue) {
  if (!m_perfMap) return;
  if (RuntimeOption::EvalProfileBC) return;
  std::string name = lookupFunction(func, exit, inPrologue, true);
  fprintf(m_perfMap, "%lx %x %s\n",
    reinterpret_cast<uintptr_t>(range.begin()),
    range.size(),
    name.c_str());
  fflush(m_perfMap);
}

void DebugInfo::recordBCInstr(TCRange range, uint32_t op) {
  static const char* opcodeName[] = {
#define O(name, imm, push, pop, flags) \
#name,
    OPCODES
#undef O
  };

  static const char* astubOpcodeName[] = {
    "OpAstubStart",
#define O(name, imm, push, pop, flags) \
#name "-Astub",
    OPCODES
#undef O
  };

  static const char* highOpcodeName[] = {
    "OpHighStart",
#define O(name) \
#name,
    HIGH_OPCODES
#undef O
  };


  if (RuntimeOption::EvalProfileBC) {
    if (!m_perfMap) return;
    const char* name;
    if (op < Op_count) {
      name = opcodeName[op];
    } else if (op < OpAstubCount) {
      name = astubOpcodeName[op - OpAstubStart];
    } else {
      name = highOpcodeName[op - OpHighStart];
    }
    fprintf(m_perfMap, "%lx %x %s\n",
	    uintptr_t(range.begin()), range.size(), name);
  }
}

void DebugInfo::recordTracelet(TCRange range, const Func* func,
    const Opcode *instr, bool exit, bool inPrologue) {
  if (range.isAstubs()) {
    m_astubsDwarfInfo.addTracelet(range, NULL, func, instr, exit, inPrologue);
  } else {
    m_aDwarfInfo.addTracelet(range, NULL, func, instr, exit, inPrologue);
  }
}

void DebugInfo::debugSync() {
  m_aDwarfInfo.syncChunks();
  m_astubsDwarfInfo.syncChunks();
}

std::string lookupFunction(const Func* f,
                           bool exit,
                           bool inPrologue,
                           bool pseudoWithFileName) {
  // TODO: mangle the namespace and name?
  std::string fname("PHP::");
  if (pseudoWithFileName) {
    fname += f->unit()->filepath()->data();
    fname += "::";
  }
  if (!strcmp(f->name()->data(), "")) {
    if (!exit) {
      fname += "__pseudoMain";
    } else {
      fname += "__exit";
    }
    return fname;
  }
  fname += f->fullName()->data();
  if (inPrologue)
    fname += "$prologue";
  return fname;
}

}
}
}
