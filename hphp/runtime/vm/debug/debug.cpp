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

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/debug/gdb-jit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/runtime/base/execution-context.h"

#include "hphp/util/current-executable.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <cxxabi.h>

#ifdef HAVE_LIBBFD
#include <bfd.h>
#endif

using namespace HPHP::jit;

namespace HPHP {
namespace Debug {

void* DebugInfo::pidMapOverlayStart;
void* DebugInfo::pidMapOverlayEnd;

DebugInfo* DebugInfo::Get() {
  return mcg->getDebugInfo();
}

DebugInfo::DebugInfo() {
  m_perfMapName = folly::sformat("/tmp/perf-{}.map", getpid());
  if (RuntimeOption::EvalPerfPidMap) {
    m_perfMap = fopen(m_perfMapName.c_str(), "w");
  }
  m_dataMapName = folly::sformat("/tmp/perf-data-{}.map", getpid());
  if (RuntimeOption::EvalPerfDataMap) {
    m_dataMap = fopen(m_dataMapName.c_str(), "w");
  }
  m_relocMapName = folly::sformat("/tmp/hhvm-reloc-{}.map", getpid());
  if (RuntimeOption::EvalPerfRelocate) {
    m_relocMap = fopen(m_relocMapName.c_str(), "w+");
  }
  generatePidMapOverlay();
}

DebugInfo::~DebugInfo() {
  if (m_perfMap) {
    fclose(m_perfMap);
    if (!RuntimeOption::EvalKeepPerfPidMap) {
      unlink(m_perfMapName.c_str());
    }
  }

  if (m_dataMap) {
    fclose(m_dataMap);
    if (!RuntimeOption::EvalKeepPerfPidMap) {
      unlink(m_dataMapName.c_str());
    }
  }

  if (m_relocMap) {
    fclose(m_relocMap);
    unlink(m_relocMapName.c_str());
  }
}

void DebugInfo::generatePidMapOverlay() {
#ifdef HAVE_LIBBFD
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
      int status;
      char* demangled =
        abi::__cxa_demangle(sym->name, nullptr, nullptr, &status);
      if (status != 0) demangled = const_cast<char*>(sym->name);
      unsigned size;
      if (i + 1 < sorted.size()) {
        auto s2 = sorted[i + 1];
        size = s2->section->vma + s2->value - addr;
      } else {
        size = uintptr_t(pidMapOverlayEnd) - addr;
      }
      if (!size) continue;
      fprintf(m_perfMap, "%lx %x %s\n",
              long(addr), size, demangled);
      if (status == 0) free(demangled);
    }

    free(symbol_table);
    free(match);
  }
  bfd_close(abfd);
  return;
#endif // HAVE_LIBBFD
}

void DebugInfo::recordStub(TCRange range, const std::string& name) {
  if (range.isAcold()) {
    m_acoldDwarfInfo.addTracelet(range, name, nullptr, nullptr, false, false);
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

  static const char* acoldOpcodeName[] = {
    "OpAcoldStart",
#define O(name, imm, push, pop, flags) \
#name "-Acold",
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
    } else if (op < OpAcoldCount) {
      name = acoldOpcodeName[op - OpAcoldStart];
    } else {
      name = highOpcodeName[op - OpHighStart];
    }
    fprintf(m_perfMap, "%lx %x %s\n",
            uintptr_t(range.begin()), range.size(), name);
    fflush(m_perfMap);
  }
}

void DebugInfo::recordTracelet(TCRange range, const Func* func,
    const Op* instr, bool exit, bool inPrologue) {
  if (range.isAcold()) {
    m_acoldDwarfInfo.addTracelet(range, folly::none, func,
                                 instr, exit, inPrologue);
  } else {
    m_aDwarfInfo.addTracelet(range, folly::none, func, instr, exit,
                             inPrologue);
  }
}

void DebugInfo::recordDataMap(void* from, void* to, const std::string& desc) {
  if (!mcg) return;
  if (auto* dataMap = Get()->m_dataMap) {
    fprintf(dataMap, "%" PRIxPTR " %" PRIx64 " %s\n",
            uintptr_t(from),
            uint64_t((char*)to - (char*)from),
            desc.c_str());
    fflush(dataMap);
  }
}

void DebugInfo::recordRelocMap(void* from, void* to,
                               const String& transInfo) {
  if (m_relocMap) {
    fprintf(m_relocMap, "%" PRIxPTR " %" PRIx64 " %s\n",
            uintptr_t(from),
            uintptr_t(to),
            transInfo.c_str());
    fflush(m_relocMap);
  }
}

void DebugInfo::debugSync() {
  m_aDwarfInfo.syncChunks();
  m_acoldDwarfInfo.syncChunks();
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
