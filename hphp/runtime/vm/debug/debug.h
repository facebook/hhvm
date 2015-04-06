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
#ifndef TRANSLATOR_DEBUG_H_
#define TRANSLATOR_DEBUG_H_

#include <string>

#include "hphp/runtime/base/types.h"
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
                      const Op* instr, bool exit,
                      bool inPrologue);
  void recordStub(TCRange range, const std::string&);
  void recordPerfMap(TCRange range, const Func* func, bool exit,
                     bool inPrologue);
  void recordBCInstr(TCRange range, uint32_t op);

  static void recordDataMap(void* from, void* to, const std::string& desc);
  void recordRelocMap(void* from, void* to, const String& desc);
  FILE* getRelocMap() const { return m_relocMap; }
  const std::string& getRelocMapName() const { return m_relocMapName; }

  void debugSync();
  static DebugInfo* Get();
  static void setPidMapOverlay(void* from, void* to) {
    pidMapOverlayStart = from;
    pidMapOverlayEnd = to;
  }
 private:
  void generatePidMapOverlay();

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
   * Similar to perfMap, but for data addresses. Perf doesn't use
   * it directly, but we can write tools based on perf script that
   * do.
   */
  FILE* m_dataMap{nullptr};
  std::string m_dataMapName;

  /*
   * Similar to perfMap, but with enough information about each
   * translation to relocate it.
   */
  FILE* m_relocMap{nullptr};
  std::string m_relocMapName;

  static void* pidMapOverlayStart;
  static void* pidMapOverlayEnd;
};

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
