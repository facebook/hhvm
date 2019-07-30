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

#include <folly/Optional.h>

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

using EventCounts = std::vector<uint64_t>;

/*
 * Information on a single disasm instruction.
 */
struct TCDisasmInfo {
  folly::dynamic toDynamic() const {
    using folly::dynamic;

    dynamic eventsObj = dynamic::object();
    for (int i = 0; i < eventCounts.size(); i++) {
      auto const event = static_cast<PerfEventType>(i);
      auto const eventName = eventTypeToCommandLineArgument(event);
      eventsObj[eventName] = eventCounts[i];
    }

    return dynamic::object("binary", folly::trimWhitespace(binaryStr))
                          ("callDest", callDest)
                          ("code", codeStr)
                          ("perfEvents", eventsObj)
                          ("ip", folly::sformat("{}", static_cast<void*>(ip)))
                          ("instrLen", instrLen);
  }

  const std::string binaryStr;
  const std::string callDest;
  const std::string codeStr;
  const EventCounts eventCounts;
  const TCA ip;
  const uint32_t instrLen;
};

/*
 * Information shared across the specified range of disasm instructions within a
 * single TC region.
 */
struct TCRangeInfo {
  folly::dynamic toDynamic() const {
      using folly::dynamic;

      dynamic disasmObjs = dynamic::array;
      for (auto const& disasmInfo : disasm) {
        disasmObjs.push_back(disasmInfo.toDynamic());
      }

      auto const startStr = folly::sformat("{}", static_cast<void*>(start));
      auto const endStr = folly::sformat("{}", static_cast<void*>(end));

      // TODO(elijahrivera) - maybe also include func and unit info?
      return dynamic::object("start", startStr)
                            ("end", endStr)
                            ("bc", bc ? *bc : dynamic())
                            ("sha1", sha1 ? sha1->toString() : dynamic())
                            ("instrStr", instrStr ? *instrStr : dynamic())
                            ("lineNum", lineNum ? *lineNum : dynamic())
                            ("disasm", disasmObjs);
  }

  const TCA start;
  const TCA end;

  const folly::Optional<Offset> bc;
  const folly::Optional<SHA1> sha1;

  folly::Optional<const Func*> func;
  folly::Optional<std::string> instrStr;
  folly::Optional<int> lineNum;
  folly::Optional<const Unit*> unit;

  std::vector<TCDisasmInfo> disasm;
};

/*
 * Information about an entire region of the TC
 */
struct TCRegionInfo {
  folly::dynamic toDynamic() const {
    using folly::dynamic;

    dynamic rangeObjs = dynamic::array;
    for (auto const& rangeInfo : ranges) {
      rangeObjs.push_back(rangeInfo.toDynamic());
    }

    always_assert(tcRegion < TCRCount);
    return dynamic::object("tcRegion", TCRegionString[tcRegion])
                          ("ranges", rangeObjs);
  }

  const TCRegion tcRegion;
  std::vector<TCRangeInfo> ranges;
};

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

  void printDisasm(std::ostream&,
                   TCA startAddr,
                   uint32_t len,
                   const std::vector<TransBCMapping>& bcMap,
                   const PerfEventsMap<TCA>& perfEvents,
                   bool hostOpcodes);

  folly::dynamic getDisasm(TCA startAddr,
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

  void disasm(std::ostream&,
              FILE*  file,
              TCA    fileStartAddr,
              TCA    codeStartAddr,
              uint64_t codeLen,
              const PerfEventsMap<TCA>& perfEvents,
              BCMappingInfo bcMappingInfo,
              bool   printAddr,
              bool   printBinary);

  /*
   * Read in the specified FILE, starting at the given offset and reading
   * `codeLen` bytes, and store those bytes in the location pointed to by
   * `code`. Throws an error if the file is unable to be read at that location.
   */
  void readDisasmFile(FILE*, const Offset, const uint64_t codeLen, void* code);

  /*
   * Get all of the disassembly information for the region
   * [codeStartAddr, codeStartAddr + codeLen) as read from the provided FILE.
   */
  TCRegionInfo getRegionInfo(FILE*  file,
                              TCA fileStartAddr,
                              TCA codeStartAddr,
                              uint64_t codeLen,
                              const PerfEventsMap<TCA>& perfEvents,
                              BCMappingInfo bcMappingInfo);

  /*
   * Get information on the different divisions of ranges within [start, end),
   * as specified by bcMappingInfo.
   *
   * bcMappingInfo.bcMapping is expected to be in order, such that if region i
   * in the mapping covers range [a, b), region i+1 is expected to cover some
   * range [b, c).
   */
  std::vector<TCRangeInfo> getRanges(const BCMappingInfo& bcMappingInfo,
                                      const TCA start,
                                      const TCA end);

  /*
   * Get information about this specific range [start, end)
   */
  TCRangeInfo getRangeInfo(const TransBCMapping& transBCMap,
                            const TCA start,
                            const TCA end);

  /*
   * Collate information about a single instruction
   */
  TCDisasmInfo getDisasmInfo(const TCA ip,
                              const uint32_t instrLen,
                              const PerfEventsMap<TCA>& perfEvents,
                              const std::string& binaryStr,
                              const std::string& callDest,
                              const std::string& codeStr);

  TCA collectJmpTargets(FILE* file,
                        TCA fileStartAddr,
                        TCA codeStartAddr,
                        uint64_t codeLen,
                        std::vector<TCA>* jmpTargets);

  std::string getSymbolName(TCA addr);

  /*
   * Find the specific address which starts the given region
   */
  TCA getRegionStart(TCRegion region, TransBCMapping transBCMap);

  /*
   * Format and print information about a range into os
   */
  void printRangeInfo(std::ostream& os,
                      const TCRangeInfo& rangeInfo,
                      const bool printAddr,
                      const bool printBinary);

  /*
   * Format and print information about a disasm instruction into os
   */
  void printDisasmInfo(std::ostream& os,
                      const TCDisasmInfo& disasmInfo,
                      const bool printAddr,
                      const bool printBinary);

  void printEventStats(std::ostream&,
                       EventCounts events);

  EventCounts getEventCounts(TCA address,
                             uint32_t instrLen,
                             const PerfEventsMap<TCA>& perfEvents);
};

} }

#endif
