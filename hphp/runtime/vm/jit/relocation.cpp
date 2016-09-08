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

#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/relocation-x64.h"

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
  if (it->first == start) {
    // if it->second.first is set, start is also the end
    // of an existing region. Don't erase it in that case
    if (it->second.first) {
      it++->second.second = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
  while (it != m_adjustedAddresses.end() && it->first < end) {
    m_adjustedAddresses.erase(it++);
  }
  if (it == m_adjustedAddresses.end()) return;
  if (it->first == end) {
    // Similar to start above, end could be the start of an
    // existing region.
    if (it->second.second) {
      it++->second.first = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Wrappers. Not using ARCH_SWITCH_CALL as not all platforms implemented
 * relocation.
 */

void adjustForRelocation(RelocationInfo& rel) {
  switch (arch()) {
  case Arch::X64:
    x64::adjustForRelocation(rel);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    break;
  }
}
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd) {
  switch (arch()) {
  case Arch::X64:
    x64::adjustForRelocation(rel, srcStart, srcEnd);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    break;
  }
}
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& fixups) {
  switch (arch()) {
  case Arch::X64:
    x64::adjustCodeForRelocation(rel, fixups);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    break;
  }
}
void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CGMeta& fixups) {
  switch (arch()) {
  case Arch::X64:
    x64::adjustMetaDataForRelocation(rel, asmInfo, fixups);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    break;
  }
}
void findFixups(TCA start, TCA end, CGMeta& fixups) {
  switch (arch()) {
  case Arch::X64:
    x64::findFixups(start, end, fixups);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    break;
  }
}
size_t relocate(RelocationInfo& rel,
                CodeBlock& destBlock,
                TCA start, TCA end,
                CGMeta& fixups,
                TCA* exitAddr) {
  switch (arch()) {
  case Arch::X64:
    return x64::relocate(rel, destBlock, start, end, fixups, exitAddr);
    break;
  case Arch::PPC64:
  case Arch::ARM:
    return 0;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}}
