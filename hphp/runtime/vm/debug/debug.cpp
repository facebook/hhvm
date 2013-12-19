/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/debug/gdb-jit.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

#include "hphp/runtime/base/execution-context.h"

#include "hphp/util/current-executable.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <bfd.h>

using namespace HPHP::JIT;

namespace HPHP {
namespace Debug {

void* DebugInfo::pidMapOverlayStart;
void* DebugInfo::pidMapOverlayEnd;

DebugInfo* DebugInfo::Get() {
  return tx64->getDebugInfo();
}

DebugInfo::DebugInfo() {
  snprintf(m_perfMapName,
           sizeof m_perfMapName,
           "/tmp/perf-%d.map", getpid());
  m_perfMap = fopen(m_perfMapName, "w");
  generatePidMapOverlay();
}

DebugInfo::~DebugInfo() {
  if (m_perfMap) fclose(m_perfMap);
  if (!RuntimeOption::EvalKeepPerfPidMap) {
    unlink(m_perfMapName);
  }
}

void DebugInfo::generatePidMapOverlay() {
  if (!m_perfMap || !pidMapOverlayStart) return;

  std::string self = current_executable_path();
  bfd* abfd = bfd_openr(self.c_str(), nullptr);
#ifdef BFD_DECOMPRESS
  abfd->flags |= BFD_DECOMPRESS;
#endif
  char **match = nullptr;
  if (!bfd_check_format(abfd, bfd_archive) &&
      bfd_check_format_matches(abfd, bfd_object, &match)) {

    std::vector<asymbol*> sorted;
    long storage_needed = bfd_get_symtab_upper_bound (abfd);

    if (storage_needed <= 0) return;

    auto symbol_table = (asymbol**)malloc(storage_needed);

    long number_of_symbols = bfd_canonicalize_symtab(abfd, symbol_table);

    for (long i = 0; i < number_of_symbols; i++) {
      auto sym = symbol_table[i];
      if (sym->flags &
          (BSF_INDIRECT |
           BSF_SECTION_SYM |
           BSF_FILE |
           BSF_DEBUGGING_RELOC |
           BSF_OBJECT)) {
        continue;
      }
      auto sec = sym->section;
      if (!(sec->flags & (SEC_ALLOC|SEC_LOAD|SEC_CODE))) continue;
      auto addr = sec->vma + sym->value;
      if (addr < uintptr_t(pidMapOverlayStart) ||
          addr >= uintptr_t(pidMapOverlayEnd)) {
        continue;
      }
      sorted.push_back(sym);
    }

    std::sort(sorted.begin(), sorted.end(), [](asymbol* a, asymbol* b) {
        auto addra = a->section->vma + a->value;
        auto addrb = b->section->vma + b->value;
        if (addra != addrb) return addra < addrb;
        return strncmp("_ZN4HPHP", a->name, 8) &&
          !strncmp("_ZN4HPHP", b->name, 8);
      });

    for (size_t i = 0; i < sorted.size(); i++) {
      auto sym = sorted[i];
      auto addr = sym->section->vma + sym->value;
      unsigned size;
      if (i + 1 < sorted.size()) {
        auto s2 = sorted[i + 1];
        size = s2->section->vma + s2->value - addr;
      } else {
        size = uintptr_t(pidMapOverlayEnd) - addr;
      }
      if (!size) continue;
      fprintf(m_perfMap, "%lx %x %s\n",
              long(addr), size, sym->name);
    }

    free(symbol_table);
    free(match);
  }
  bfd_close(abfd);
  return;
}

void DebugInfo::recordStub(TCRange range, const char* name) {
  if (range.isAstubs()) {
    m_astubsDwarfInfo.addTracelet(range, name, nullptr, nullptr, false, false);
  } else {
    m_aDwarfInfo.addTracelet(range, name, nullptr, nullptr, false, false);
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
    m_astubsDwarfInfo.addTracelet(range, nullptr, func, instr, exit, inPrologue);
  } else {
    m_aDwarfInfo.addTracelet(range, nullptr, func, instr, exit, inPrologue);
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
