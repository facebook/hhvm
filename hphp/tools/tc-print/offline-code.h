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

#ifndef OFFLINE_X86_CODE_
#define OFFLINE_X86_CODE_

#include <string>
#include <unordered_map>

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/tools/tc-print/offline-trans-data.h"
#include "hphp/tools/tc-print/perf-events.h"

extern "C" {
#if defined(HAVE_LIBXED)
#include <xed-interface.h>
#endif
}

namespace HPHP { namespace jit {

enum TCRegion {
  // NOTE: whenever you update this enumeration, please don't forget to fix
  // TCRegionString[] array accordingly.

  TCRHot,
  TCRMain,
  TCRProfile,
  TCRCold,
  TCRFrozen,
  TCRCount, // keep last
};

extern std::string TCRegionString[];

struct TCRegionRec {
  FILE*    file;
  TCA      baseAddr;
  uint32_t len;
};

struct OfflineCode {

  OfflineCode(std::string _dumpDir,
                 TCA _ahotBase,
                 TCA _aBase,
                 TCA _aprofBase,
                 TCA _coldBase,
                 TCA _frozenBase)
      : dumpDir(_dumpDir) {
    TCA tcRegionBases[TCRCount] = {
      _ahotBase, _aBase, _aprofBase, _coldBase, _frozenBase
    };
#if defined(HAVE_LIBXED)
    xedInit();
#endif
    openFiles(tcRegionBases);
    loadSymbolsMap();
  }

  ~OfflineCode() {
    closeFiles();
  }

  void printDisasm(TCA startAddr,
                   uint32_t len,
                   const std::vector<TransBCMapping>& bcMap,
                   const PerfEventsMap<TCA>& perfEvents,
                   bool hostOpcodes);

  // Returns the fall-thru successor from 'a', if any
  TCA getTransJmpTargets(const TransRec *transRec,
                         std::vector<TCA> *jmpTargets);

  TCRegion findTCRegionContaining(TCA addr) const;

  const char* getArchName();

private:
  struct BCMappingInfo {
    TCRegion tcRegion;
    const std::vector<TransBCMapping>& bcMapping;

    BCMappingInfo() = delete;
    BCMappingInfo(TCRegion tcr,
                  const std::vector<TransBCMapping>& map)
      : tcRegion(tcr)
      , bcMapping(map)
    {}
  };

  std::string       dumpDir;
  TCRegionRec       tcRegions[TCRCount];
#if defined(HAVE_LIBXED)
  xed_state_t       xed_state;
  xed_syntax_enum_t xed_syntax;
#endif

  std::unordered_map<TCA, std::string> addr2SymMap;

  void openFiles(TCA tcRegionBases[TCRCount]);
  void closeFiles();
  void xedInit();
  void loadSymbolsMap();

  bool tcRegionContains(TCRegion tcr, TCA addr) const;

  void disasm(FILE*  file,
              TCA    fileStartAddr,
              TCA    codeStartAddr,
              uint64_t codeLen,
              const PerfEventsMap<TCA>& perfEvents,
              BCMappingInfo bcMappingInfo,
              bool   printAddr,
              bool   printBinary);

  TCA collectJmpTargets(FILE* file,
                        TCA fileStartAddr,
                        TCA codeStartAddr,
                        uint64_t codeLen,
                        std::vector<TCA>* jmpTargets);

  std::string getSymbolName(TCA addr);

  size_t printBCMapping(BCMappingInfo bcMappingInfo, size_t currBC, TCA ip);

  void printEventStats(TCA address,
                       uint32_t instrLen,
                       const PerfEventsMap<TCA>& perfEvents);
};

} }

#endif
