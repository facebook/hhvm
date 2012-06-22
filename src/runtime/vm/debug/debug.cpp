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

using namespace HPHP::VM::Transl;

namespace HPHP {
namespace VM {
namespace Debug {

/*
 * Stuff to output symbol names to /tmp/perf-%d.map files.  This stuff
 * can be read by perf top/record, etc.
 */
static char perfMapName[64];
FILE* perfMap;

static void deleteMap() {
  if (perfMap) fclose(perfMap);
  unlink(perfMapName);
}

static void openMap() {
  snprintf(perfMapName, sizeof perfMapName, "/tmp/perf-%d.map", getpid());
  perfMap = fopen(perfMapName, "w");
  atexit(deleteMap);
}

void recordPerfMap(const DwarfChunk* chunk) {
  if (!perfMap) return;
  if (RuntimeOption::EvalProfileBC) return;
  for (FuncPtrDB::const_iterator it = chunk->m_functions.begin();
      it != chunk->m_functions.end();
      ++it) {
    if (!(*it)->perfSynced()) {
      fprintf(perfMap, "%lx %x %s\n",
        reinterpret_cast<uintptr_t>((*it)->start),
        static_cast<uint32_t>((*it)->end - (*it)->start),
        (*it)->name.c_str());
      (*it)->setPerfSynced();
    }
  }
  fflush(perfMap);
}

DebugInfo::DebugInfo() {
  ASSERT(!perfMap);
  openMap();
}

void DebugInfo::recordStub(TCA start, TCA end, const char* name) {
  m_dwarfInfo.addTracelet(start, end, name, NULL, NULL, false, false);
}

void DebugInfo::recordTracelet(TCA start, TCA end, const Unit *unit,
    const Opcode *instr, bool exit, bool inPrologue) {
  m_dwarfInfo.addTracelet(start, end, NULL, unit, instr,
                                              exit, inPrologue);
}

void DebugInfo::debugSync() {
  m_dwarfInfo.syncChunks();
}

std::string lookupFunction(const Unit *unit,
                           const Opcode *instr,
                           bool exit,
                           bool inPrologue,
                           bool pseudoWithFileName) {
  // TODO: mangle the namespace and name?
  std::string fname("PHP::");
  if (unit == NULL || instr == NULL) {
    fname += "#anonFunc";
    return fname;
  }
  const Func *f = unit->getFunc(unit->offsetOf(instr));
  if (f != NULL) {
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
    fname += f->name()->data();
    if (inPrologue)
      fname += "$prologue";
    return fname;
  }
  fname += "#anonFunc";
  return fname;
}

}
}
}
