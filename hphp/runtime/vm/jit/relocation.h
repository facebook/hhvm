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
#ifndef incl_HPHP_CODE_RELOCATION_H_
#define incl_HPHP_CODE_RELOCATION_H_

#include <set>
#include <cstdio>
#include <vector>

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/srcdb.h"

namespace HPHP {

struct CodeCache;

namespace jit {

struct AsmInfo;

//////////////////////////////////////////////////////////////////////

struct RelocationInfo {
  RelocationInfo() {}

  void recordRange(TCA start, TCA end,
                   TCA destStart, TCA destEnd);
  void recordAddress(TCA src, TCA dest, int range);
  TCA adjustedAddressAfter(TCA addr) const;
  TCA adjustedAddressBefore(TCA addr) const;
  CTCA adjustedAddressAfter(CTCA addr) const {
    return adjustedAddressAfter(const_cast<TCA>(addr));
  }
  CTCA adjustedAddressBefore(CTCA addr) const {
    return adjustedAddressBefore(const_cast<TCA>(addr));
  }
  void rewind(TCA start, TCA end);
  void markAddressImmediates(const std::set<TCA>& ai) {
    m_addressImmediates.insert(ai.begin(), ai.end());
  }
  bool isAddressImmediate(TCA ip) {
    return m_addressImmediates.count(ip);
  }
  typedef std::vector<std::pair<TCA,TCA>> RangeVec;
  const RangeVec& srcRanges() { return m_srcRanges; }
  const RangeVec& dstRanges() { return m_dstRanges; }
 private:
  RangeVec m_srcRanges;
  RangeVec m_dstRanges;
  /*
   * maps from src address, to range of destination address
   * This is because we could insert nops before the instruction
   * corresponding to src. Most things want the address of the
   * instruction corresponding to the src instruction; but eg
   * the fixup map would want the address of the nop.
   */
  std::map<TCA,std::pair<TCA,TCA>> m_adjustedAddresses;
  std::set<TCA> m_addressImmediates;
};

/*
  relocate using data from perf.
  If time is non-negative, its used as the time to run perf record.
  If time is -1, we pick a random subset of translations, and relocate them
  in a random order.
  If time is -2, we relocate all of the translations.

  Currently we don't ever relocate anything from frozen (or prof). We also
  don't relocate the cold portion of translations; but we still need to know
  where those are in order to relocate back-references to the code that was
  relocated.
*/
void liveRelocate(int time);
inline void liveRelocate(bool random) {
  return liveRelocate(random ? -1 : 20);
}
void recordPerfRelocMap(
  TCA start, TCA end,
  TCA coldStart, TCA coldEnd,
  SrcKey sk, int argNum,
  const GrowableVector<IncomingBranch> &incomingBranches,
  CodeGenFixups& fixups);
String perfRelocMapInfo(
  TCA start, TCA end,
  TCA coldStart, TCA coldEnd,
  SrcKey sk, int argNum,
  const GrowableVector<IncomingBranch> &incomingBranches,
  CodeGenFixups& fixups);

struct TransRelocInfo;
void readRelocations(
  FILE* relocFile,
  std::set<TCA>* liveStubs,
  void (*callback)(TransRelocInfo&& tri, void* data),
  void* data);
void relocate(std::vector<TransRelocInfo>& relocs, CodeBlock& hot);

/*
 * Relocate a new translation into a free region in the TC and update the
 * TransLoc.
 *
 * Attempt to relocate the main, cold, and frozen portions of the translation
 * loc into memory freed memory in the TC their respective code blocks. In
 * addition, reusable stubs associated with this translation will be relocated
 * to be outside of loc so that they can be managed separately.
 *
 * If set *adjust will be updated to its post relocation address.
 */
bool relocateNewTranslation(TransLoc& loc, CodeCache& cache,
                            TCA* adjust = nullptr);

//////////////////////////////////////////////////////////////////////

/*
 * X64-specific portions of the code are separated out here.
 */
namespace x64 {
void adjustForRelocation(RelocationInfo&);
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd);
void adjustCodeForRelocation(RelocationInfo& rel, CodeGenFixups& fixups);
void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CodeGenFixups& fixups);
void findFixups(TCA start, TCA end, CodeGenFixups& fixups);
size_t relocate(RelocationInfo& rel,
                CodeBlock& destBlock,
                TCA start, TCA end,
                CodeGenFixups& fixups,
                TCA* exitAddr);

}

//////////////////////////////////////////////////////////////////////

}}


#endif
