/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/vm/jit/mcgen.h"

#include "hphp/runtime/base/execution-context.h"

#include "hphp/util/current-executable.h"
#include "hphp/util/logger.h"
#include "hphp/util/portability.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <cxxabi.h>

#if defined USE_FOLLY_SYMBOLIZER

#include <folly/experimental/symbolizer/Symbolizer.h>

#elif defined HAVE_LIBBFD

#include <bfd.h>

#endif

#include <folly/portability/Filesystem.h>
#include <folly/portability/Unistd.h>
#include <folly/Demangle.h>

using namespace HPHP::jit;

namespace HPHP {
namespace Debug {
namespace {
DebugInfo* s_info;

bool checkValidDir(const char* dirName) {
  if (!dirName || !strlen(dirName))  return false;

  folly::fs::path dirPath(dirName);
  return folly::fs::is_directory(dirPath);
}

}

void* DebugInfo::pidMapOverlayStart;
void* DebugInfo::pidMapOverlayEnd;

void initDebugInfo() {
  s_info = new DebugInfo();
}

void destroyDebugInfo() {
  if (s_info) {
    delete s_info;
    s_info = nullptr;
  }
}

DebugInfo* DebugInfo::Get() {
  return s_info;
}

DebugInfo::DebugInfo() {
  auto const perfmaps_dir = RuntimeOption::EvalPerfMapsDir.data();

  if (checkValidDir(perfmaps_dir)) {
    m_perfMapName = folly::sformat("{}/perf-{}.map", perfmaps_dir, getpid());
    m_dataMapName = folly::sformat("{}/perf-data-{}.map", perfmaps_dir, getpid());
  } else {
    m_perfMapName = folly::sformat("/tmp/perf-{}.map", getpid());
    m_dataMapName = folly::sformat("/tmp/perf-data-{}.map", getpid());
  }
  Logger::Info("perfmap file: %s", m_perfMapName.c_str());
  Logger::Info("perf-data-map file: %s", m_dataMapName.c_str());

  if (RuntimeOption::EvalPerfPidMap) {
    m_perfMap = fopen(m_perfMapName.c_str(), "w");
  }
  if (RuntimeOption::EvalPerfJitDump) {
    initPerfJitDump();
  }
  if (RuntimeOption::EvalPerfDataMap) {
    m_dataMap = fopen(m_dataMapName.c_str(), "w");
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

  if (m_perfJitDump) {
    closePerfJitDump();
    if (!RuntimeOption::EvalKeepPerfPidMap) {
      unlink(m_perfJitDumpName.c_str());
    }
  }

  if (m_dataMap) {
    fclose(m_dataMap);
    if (!RuntimeOption::EvalKeepPerfPidMap) {
      unlink(m_dataMapName.c_str());
    }
  }
}

void DebugInfo::generatePidMapOverlay() {
  if (!m_perfMap || !pidMapOverlayStart) return;

  struct SymInfo {
    const char* name;
    uintptr_t addr;
    size_t size;
  };
  std::vector<SymInfo> sorted;

#if defined USE_FOLLY_SYMBOLIZER

  auto self = current_executable_path();
  using folly::symbolizer::ElfFile;
  ElfFile file;
  using ElfSym = ElfW(Sym);
  using ElfShdr = ElfW(Shdr);
  if (file.openNoThrow(self.c_str()) != ElfFile::kSuccess) return;
  file.iterateSectionsWithType(SHT_SYMTAB, [&] (const ElfShdr& section) {
    auto strings = file.getSectionByIndex(section.sh_link);
    if (!strings) return false;
    file.iterateSymbolsWithType(section, STT_FUNC, [&] (const ElfSym& sym) {
      if (sym.st_shndx == SHN_UNDEF) return false;
      if (!sym.st_name) return false;
      if (sym.st_value >= uintptr_t(pidMapOverlayStart) &&
          sym.st_value <= uintptr_t(pidMapOverlayEnd)) {
        sorted.push_back(SymInfo {
            file.getString(*strings, sym.st_name),
              sym.st_value,
              sym.st_size});
      }
      return false;
    });
    return false;
  });

#elif defined HAVE_LIBBFD

  auto self = current_executable_path();
  bfd* abfd = bfd_openr(self.c_str(), nullptr);
#ifdef BFD_DECOMPRESS
  abfd->flags |= BFD_DECOMPRESS;
#endif
  SCOPE_EXIT { bfd_close(abfd); };
  char **match = nullptr;
  if (bfd_check_format(abfd, bfd_archive) ||
      !bfd_check_format_matches(abfd, bfd_object, &match)) {
    return;
  }

  long storage_needed = bfd_get_symtab_upper_bound (abfd);

  if (storage_needed <= 0) return;

  auto symbol_table = (asymbol**)malloc(storage_needed);

  SCOPE_EXIT { free(symbol_table); free(match); };

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
    sorted.push_back(SymInfo{sym->name, addr, 0});
  }
#endif // HAVE_LIBBFD

  std::sort(
    sorted.begin(), sorted.end(),
    [](const SymInfo& a, const SymInfo& b) {
      if (a.addr != b.addr) return a.addr < b.addr;
      if (a.size != b.size) return a.size > b.size;
      return
        strncmp("_ZN4HPHP", a.name, 8) &&
        !strncmp("_ZN4HPHP", b.name, 8);
    }
  );

  for (size_t i = 0; i < sorted.size(); i++) {
    auto const& sym = sorted[i];
    auto demangled = folly::demangle(sym.name);
    unsigned size;
    if (i + 1 < sorted.size()) {
      auto const& s2 = sorted[i + 1];
      size = s2.addr - sym.addr;
    } else {
      size = uintptr_t(pidMapOverlayEnd) - sym.addr;
    }
    if (!size) continue;
    fprintf(m_perfMap, "%lx %x %s\n",
            long(sym.addr), size, demangled.c_str());
  }

  return;
}

void DebugInfo::recordStub(TCRange range, const std::string& name) {
  if (range.isAcold()) {
    m_acoldDwarfInfo.addTracelet(range, name, nullptr, -1, false);
  } else {
    m_aDwarfInfo.addTracelet(range, name, nullptr, -1, false);
  }
}

void DebugInfo::recordPerfMap(TCRange range, SrcKey sk, std::string name) {
  if (!m_perfMap) return;
  if (RuntimeOption::EvalProfileBC) return;
  if (name.empty()) {
    name = lookupFunction(sk.func(), sk.prologue(),
                          RuntimeOption::EvalPerfPidMapIncludeFilePath);
  }
  fprintf(m_perfMap, "%lx %x %s\n",
    reinterpret_cast<uintptr_t>(range.begin()),
    range.size(),
    name.c_str());
  fflush(m_perfMap);

  //Dump the object code into the specified file
  if (m_perfJitDump) {
    perfJitDumpTrace(range.begin(), range.size(), name.c_str());
  }
}

void DebugInfo::recordBCInstr(TCRange range, uint32_t op) {
  static const char* opcodeName[] = {
#define O(name, imm, push, pop, flags) \
#name,
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
    } else {
      name = highOpcodeName[op - OpHighStart];
    }
    fprintf(m_perfMap, "%lx %x %s\n",
            uintptr_t(range.begin()), range.size(), name);
    fflush(m_perfMap);
  }
}

void DebugInfo::recordTracelet(TCRange range, SrcKey sk) {
  if (range.isAcold()) {
    m_acoldDwarfInfo.addTracelet(range, std::nullopt, sk.func(), sk.lineNumber(),
                                 sk.prologue());
  } else {
    m_aDwarfInfo.addTracelet(range, std::nullopt, sk.func(), sk.lineNumber(),
                             sk.prologue());
  }
}

void DebugInfo::recordDataMap(const void* from, const void* to,
                              const std::string& desc) {
  if (!mcgen::initialized()) return;
  if (auto dataMap = Get()->m_dataMap) {
    fprintf(dataMap, "%" PRIxPTR " %" PRIx64 " %s\n",
            uintptr_t(from),
            uint64_t((char*)to - (char*)from),
            desc.c_str());
    fflush(dataMap);
  }
}

void DebugInfo::debugSync() {
  m_aDwarfInfo.syncChunks();
  m_acoldDwarfInfo.syncChunks();
}

std::string lookupFunction(const Func* f,
                           bool inPrologue,
                           bool pseudoWithFileName) {
  // TODO: mangle the namespace and name?
  std::string fname("PHP::");
  if (pseudoWithFileName) {
    fname += f->unit()->origFilepath()->data();
    fname += "::";
  }
  if (!strcmp(f->name()->data(), "")) {
    // TODO: should no longer happen
    fname += "__pseudoMain";
    return fname;
  }
  if (f->isClosureBody()) {
    fname += f->baseCls()->name()->toCppString();
    fname = fname.substr(0, fname.find(';'));
    fname += "::__invoke";
  } else {
    fname += f->fullName()->data();
  }
  if (inPrologue) {
    fname += "$prologue";
  }
  return fname;
}

}
}
