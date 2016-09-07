/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/data-block.h"

#include <map>
#include <set>
#include <vector>

namespace HPHP { namespace jit {

struct AsmInfo;
struct CGMeta;

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
    addressImmediates.insert(ai.begin(), ai.end());
  }
  bool isAddressImmediate(TCA ip) {
    return addressImmediates.count(ip);
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
  std::set<TCA> addressImmediates;
};

namespace x64 {

void adjustForRelocation(RelocationInfo&);
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd);
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& fixups);
void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CGMeta& fixups);
void findFixups(TCA start, TCA end, CGMeta& fixups);
size_t relocate(RelocationInfo& rel,
                CodeBlock& destBlock,
                TCA start, TCA end,
                CGMeta& fixups,
                TCA* exitAddr);

}}}


#endif
