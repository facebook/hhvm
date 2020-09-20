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
#ifndef TRANSLATOR_DEBUG_H_
#define TRANSLATOR_DEBUG_H_

#include <string>

#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/debug/dwarf.h"

namespace HPHP { namespace Debug {

//////////////////////////////////////////////////////////////////////

struct DebugInfo {
  DebugInfo();
  ~DebugInfo();

  void recordTracelet(TCRange range,
                      const Func* func,
                      PC instr, bool exit,
                      bool inPrologue);
  void recordStub(TCRange range, const std::string&);
  void recordPerfMap(TCRange range, SrcKey sk, const Func* func, bool exit,
                     bool inPrologue, std::string stub = "");
  void recordBCInstr(TCRange range, uint32_t op);

  static void recordDataMap(const void* from, const void* to,
                            const std::string& desc);

  void debugSync();
  static DebugInfo* Get();
  static void setPidMapOverlay(void* from, void* to) {
    pidMapOverlayStart = from;
    pidMapOverlayEnd = to;
  }

 private:
  void recordPerfJitTracelet(TCRange range,
                             const Func* func,
                             bool exit,
                             bool inPrologue);
  void generatePidMapOverlay();
  void initPerfJitDump();
  void closePerfJitDump();
  int perfJitDumpTrace(const void* startAddr,
                       const unsigned int size,
                       const char* symName);

  /* maintain separate dwarf info for a and acold, so that we
   * don't emit dwarf info for the two in the same ELF file.
   * gdb tends to get confused when it sees dwarf info for
   * widely separated addresses ranges in the same ELF file.
   */
  DwarfInfo m_aDwarfInfo;
  DwarfInfo m_acoldDwarfInfo;
  /*
   * Stuff to output symbol names to /tmp/perf-%d.map files.  This stuff
   * can be read by perf top/record, etc.
   */
  FILE* m_perfMap{nullptr};
  std::string m_perfMapName;

  /*
   * jitdump file will store the generated code in /tmp/jit-<pid>.dump
   * perf record will create perf.data
   * perf inject will read jitdump file and generate a corresponding
   * jitted-<pid>-<id>.so ELF file every trace compiled, it will then insert
   * a PERF_RECORD_MMAP2 mmap event for every ELF file created into perf.data
   * perf report will be able to read these ELF files and provide information
   * on the samples collected on generated code.
   */
  FILE* m_perfJitDump{nullptr};
  std::string m_perfJitDumpName;
  void* m_perfMmapMarker{nullptr};

  /*
   * Similar to perfMap, but for data addresses. Perf doesn't use
   * it directly, but we can write tools based on perf script that
   * do.
   */
  FILE* m_dataMap{nullptr};
  std::string m_dataMapName;

  static void* pidMapOverlayStart;
  static void* pidMapOverlayEnd;
};

/*
 * Initialize/destroy the global DebugInfo
 */
void initDebugInfo();
void destroyDebugInfo();

/*
 * Gets the fake symbol name we want to use for a php function.
 */
std::string lookupFunction(const Func* func,
                           bool exit,
                           bool inPrologue,
                           bool pseudoWithFileName);

//////////////////////////////////////////////////////////////////////

}}

#endif
