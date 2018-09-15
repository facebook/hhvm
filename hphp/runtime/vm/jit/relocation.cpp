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

#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/relocation-arm.h"
#include "hphp/runtime/vm/jit/relocation-ppc64.h"
#include "hphp/runtime/vm/jit/relocation-x64.h"
#include "hphp/runtime/vm/jit/asm-info.h"

#include "hphp/util/arch.h"

namespace HPHP { namespace jit {

void RelocationInfo::recordRange(TCA start, TCA end,
                                 TCA destStart, TCA destEnd) {
  m_srcRanges.emplace_back(start, end);
  m_dstRanges.emplace_back(destStart, destEnd);
  m_adjustedAddresses[start].second = destStart;
  m_adjustedAddresses[end].first = destEnd;
}

void RelocationInfo::recordAddress(TCA src, TCA dest, int range) {
  m_adjustedAddresses.emplace(src, std::make_pair(dest, dest + range));
}

TcaRange RelocationInfo::fixupRange(const TcaRange& rng) {
  /*
   * We have to be careful with before/after here.
   * If we relocate two consecutive regions of memory,
   * but relocate them to two different destinations, then
   * the end address of the first region is also the start
   * address of the second region; so adjustedAddressBefore(end)
   * gives us the relocated address of the end of the first
   * region, while adjustedAddressAfter(end) gives us the
   * relocated address of the start of the second region.
   */
  auto s = adjustedAddressAfter(rng.begin());
  auto e = adjustedAddressBefore(rng.end());
  if (s && e) {
    return TcaRange(s, e);
  }
  if (s && !e) {
    return TcaRange(s, s + rng.size());
  }
  if (!s && e) {
    return TcaRange(e - rng.size(), e);
  }
  return rng;
}

void RelocationInfo::fixupRanges(AsmInfo* asmInfo, AreaIndex area) {
  asmInfo->clearBlockRangesForArea(area);
  for (auto& ii : asmInfo->instRangesForArea(area)) {
    ii.second = fixupRange(ii.second);
    asmInfo->updateForBlock(area, ii.first, ii.second);
  }
}

TCA RelocationInfo::adjustedAddressAfter(TCA addr) const {
  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.second;
}

TCA RelocationInfo::adjustedAddressBefore(TCA addr) const {
  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.first;
}

void RelocationInfo::rewind(TCA start, TCA end) {
  if (m_srcRanges.size() && m_srcRanges.back().first == start) {
    assertx(m_dstRanges.size() == m_srcRanges.size());
    assertx(m_srcRanges.back().second == end);
    m_srcRanges.pop_back();
    m_dstRanges.pop_back();
  }
  auto it = m_adjustedAddresses.lower_bound(start);
  if (it == m_adjustedAddresses.end()) return;

  // convenience function for erasing a m_smashableRelocations entry
  auto eraseSmashableRelocation = [this](TCA addr) {
    auto it = m_smashableRelocations.find(addr);
    if (it != m_smashableRelocations.end()) m_smashableRelocations.erase(it);
  };

  if (it->first == start) {
    // if it->second.first is set, start is also the end
    // of an existing region. Don't erase it in that case
    if (it->second.first) {
      eraseSmashableRelocation(it->second.second);
      it++->second.second = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
  while (it != m_adjustedAddresses.end() && it->first < end) {
    eraseSmashableRelocation(it->second.second);
    m_adjustedAddresses.erase(it++);
  }
  if (it == m_adjustedAddresses.end()) return;
  if (it->first == end) {
    // Similar to start above, end could be the start of an
    // existing region.
    if (it->second.second) {
      eraseSmashableRelocation(it->second.second);
      it++->second.first = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Adjusts the addresses in asmInfo and fixups to match the new
 * location of the code.
 * This will not "hook up" the relocated code in any way, so is safe
 * to call before the relocated code is ready to run.
 */
void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CGMeta& meta) {
  auto& ip = meta.inProgressTailJumps;
  for (size_t i = 0; i < ip.size(); ++i) {
    IncomingBranch& ib = const_cast<IncomingBranch&>(ip[i]);
    if (TCA adjusted = rel.adjustedAddressAfter(ib.toSmash())) {
      ib.adjust(adjusted);
    }
  }

  for (auto watch : meta.watchpoints) {
    if (auto const adjusted = rel.adjustedAddressBefore(*watch)) {
      *watch = adjusted;
    }
  }

  for (auto& fixup : meta.fixups) {
    /*
     * Pending fixups always point after the call instruction,
     * so use the "before" address, since there may be nops
     * before the next actual instruction.
     */
    if (TCA adjusted = rel.adjustedAddressBefore(fixup.first)) {
      fixup.first = adjusted;
    }
  }

  for (auto& ct : meta.catches) {
    /*
     * Similar to fixups - this is a return address so get
     * the address returned to.
     */
    if (auto const adjusted = rel.adjustedAddressBefore(ct.first)) {
      ct.first = adjusted;
    }
    /*
     * But the target is an instruction, so skip over any nops
     * that might have been inserted (eg for alignment).
     */
     if (auto const adjusted = rel.adjustedAddressAfter(ct.second)) {
       ct.second = adjusted;
     }
  }

  for (auto& is : meta.inlineStacks) {
    /*
     * As with fixups and catches these are return addresses.
     */
     if (auto const adjusted = rel.adjustedAddressBefore(is.first)) {
       is.first = adjusted;
     }
   }


  for (auto& jt : meta.jmpTransIDs) {
    if (auto const adjusted = rel.adjustedAddressAfter(jt.first)) {
      jt.first = adjusted;
    }
  }

  for (auto& r : meta.trapReasons) {
    if (auto const adjusted = rel.adjustedAddressAfter(r.first)) {
      r.first = adjusted;
    }
  }

  if (!meta.bcMap.empty()) {
    /*
     * Most of the time we want to adjust to a corresponding "before" address
     * with the exception of the start of the range where "before" can point to
     * the end of a previous range.
     */
    auto const aStart = meta.bcMap[0].aStart;
    auto const acoldStart = meta.bcMap[0].acoldStart;
    auto const afrozenStart = meta.bcMap[0].afrozenStart;
    auto adjustAddress = [&](TCA& address, TCA blockStart) {
      if (TCA adjusted = (address == blockStart
                            ? rel.adjustedAddressAfter(blockStart)
                            : rel.adjustedAddressBefore(address))) {
        address = adjusted;
      }
    };
    for (auto& tbc : meta.bcMap) {
      adjustAddress(tbc.aStart, aStart);
      adjustAddress(tbc.acoldStart, acoldStart);
      adjustAddress(tbc.afrozenStart, afrozenStart);
    }
  }

  decltype(meta.addressImmediates) updatedAI;
  for (auto addrImm : meta.addressImmediates) {
    if (TCA adjusted = rel.adjustedAddressAfter(addrImm)) {
      updatedAI.insert(adjusted);
    } else if (TCA odd = rel.adjustedAddressAfter((TCA)~uintptr_t(addrImm))) {
      // just for cgLdObjMethod
      updatedAI.insert((TCA)~uintptr_t(odd));
    } else {
      updatedAI.insert(addrImm);
    }
  }
  updatedAI.swap(meta.addressImmediates);

  if (meta.fallthru) {
    if (TCA adjusted = rel.adjustedAddressAfter(*meta.fallthru)) {
      meta.fallthru = adjusted;
    }
  }

  decltype(meta.alignments) updatedAF;
  for (auto af : meta.alignments) {
    if (TCA adjusted = rel.adjustedAddressAfter(af.first)) {
      updatedAF.emplace(adjusted, af.second);
    } else {
      updatedAF.emplace(af);
    }
  }
  updatedAF.swap(meta.alignments);

  for (auto& af : meta.reusedStubs) {
    if (TCA adjusted = rel.adjustedAddressAfter(af)) {
      af = adjusted;
    }
  }

  decltype(meta.smashableCallData) updatedCD;
  for (auto& cd : meta.smashableCallData) {
    if (auto adjusted = rel.adjustedAddressAfter(cd.first)) {
      updatedCD[adjusted] = cd.second;
      FTRACE_MOD(Trace::mcg, 3,
                 "adjustMetaDataForRelocation(smashableCallData): {} => {}\n",
                 cd.first, adjusted);
    } else {
      updatedCD[cd.first] = cd.second;
    }
  }
  updatedCD.swap(meta.smashableCallData);

  decltype(meta.smashableJumpData) updatedJD;
  for (auto& jd : meta.smashableJumpData) {
    if (auto adjusted = rel.adjustedAddressAfter(jd.first)) {
      updatedJD[adjusted] = jd.second;
      FTRACE_MOD(Trace::mcg, 3,
                 "adjustMetaDataForRelocation(smashableJumpData): {} => {}\n",
                 jd.first, adjusted);
    } else {
      updatedJD[jd.first] = jd.second;
    }
  }
  updatedJD.swap(meta.smashableJumpData);

  for (auto& li : meta.literalAddrs) {
    if (auto adjusted = rel.adjustedAddressAfter((TCA)li.second)) {
      li.second = (uint64_t*)adjusted;
    }
  }

  for (auto& v : meta.veneers) {
    bool updated = false;
    DEBUG_ONLY auto const before = v;
    if (auto adjustedSource = rel.adjustedAddressAfter(v.source)) {
      v.source = adjustedSource;
      updated = true;
    }
    if (auto adjustedTarget = rel.adjustedAddressAfter(v.target)) {
      v.target = adjustedTarget;
      updated = true;
    }
    if (updated) {
      FTRACE_MOD(Trace::mcg, 3,
                 "adjustMetaDataForRelocation(veneers): ({}, {}) => ({}, {})\n",
                 before.source, before.target, v.source, v.target);
    }
  }

  decltype(meta.smashableLocations) updatedSL;
  for (auto sl : meta.smashableLocations) {
    if (auto adjusted = rel.adjustedAddressAfter(sl)) {
      updatedSL.insert(adjusted);
    } else {
      updatedSL.insert(sl);
    }
  }
  updatedSL.swap(meta.smashableLocations);

  decltype(meta.codePointers) updatedCP;
  for (auto cp : meta.codePointers) {
    if (auto adjusted = (TCA*)rel.adjustedAddressAfter((TCA)cp)) {
      updatedCP.emplace(adjusted);
    } else {
      updatedCP.emplace(cp);
    }
  }
  updatedCP.swap(meta.codePointers);

  if (asmInfo) {
    assertx(asmInfo->validate());
    rel.fixupRanges(asmInfo, AreaIndex::Main);
    rel.fixupRanges(asmInfo, AreaIndex::Cold);
    rel.fixupRanges(asmInfo, AreaIndex::Frozen);
    assertx(asmInfo->validate());
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Wrappers.
 */

void adjustForRelocation(RelocationInfo& rel) {
  return ARCH_SWITCH_CALL(adjustForRelocation, rel);
}
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd) {
  return ARCH_SWITCH_CALL(adjustForRelocation, rel, srcStart, srcEnd);
}
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& fixups) {
  return ARCH_SWITCH_CALL(adjustCodeForRelocation, rel, fixups);
}
void findFixups(TCA start, TCA end, CGMeta& fixups) {
  return ARCH_SWITCH_CALL(findFixups, start, end, fixups);
}
size_t relocate(RelocationInfo& rel,
                CodeBlock& destBlock,
                TCA start, TCA end,
                CodeBlock& srcBlock,
                CGMeta& fixups,
                TCA* exitAddr,
                AreaIndex codeArea) {
  return ARCH_SWITCH_CALL(relocate, rel, destBlock, start, end, srcBlock,
                          fixups, exitAddr, codeArea);
}

//////////////////////////////////////////////////////////////////////

}}
