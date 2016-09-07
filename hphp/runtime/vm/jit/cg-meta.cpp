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

#include "hphp/runtime/vm/jit/cg-meta.h"

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/tread-hash-map.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(mcg);

namespace {
// Map from integral literals to their location in the TC data section.
using LiteralMap = TreadHashMap<uint64_t,const uint64_t*,std::hash<uint64_t>>;
LiteralMap s_literals{128};

// Landingpads for TC catch traces; used by the unwinder.
using CatchTraceMap = TreadHashMap<uint32_t, uint32_t, std::hash<uint32_t>>;
CatchTraceMap s_catchTraceMap{128};

constexpr uint32_t kInvalidCatchTrace = 0x0;
}

const uint64_t* addrForLiteral(uint64_t val) {
  if (auto it = s_literals.find(val)) {
    assertx(**it == val);
    return *it;
  }
  return nullptr;
}

size_t numCatchTraces() {
  return s_catchTraceMap.size();
}

void eraseCatchTrace(CTCA addr) {
  if (auto ct = s_catchTraceMap.find(tc::addrToOffset(addr))) {
    *ct = kInvalidCatchTrace;
  }
}

folly::Optional<TCA> getCatchTrace(CTCA ip) {
  auto const found = s_catchTraceMap.find(tc::addrToOffset(ip));
  if (found && *found != kInvalidCatchTrace) return tc::offsetToAddr(*found);
  return folly::none;
}

////////////////////////////////////////////////////////////////////////////////

void CGMeta::setJmpTransID(TCA jmp, TransID transID, TransKind kind) {
  if (kind != TransKind::Profile) return;

  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transID);
  jmpTransIDs.emplace_back(jmp, transID);
}

void CGMeta::process(
  GrowableVector<IncomingBranch>* inProgressTailBranches
) {
  process_only(inProgressTailBranches);
  clear();
}

void CGMeta::process_only(
  GrowableVector<IncomingBranch>* inProgressTailBranches
) {
  tc::assertOwnsMetadataLock();

  for (auto const& pair : fixups) {
    assertx(tc::isValidCodeAddress(pair.first));
    FixupMap::recordFixup(pair.first, pair.second);
  }
  fixups.clear();

  for (auto const& pair : catches) {
    auto const key = tc::addrToOffset(pair.first);
    auto const val = tc::addrToOffset(pair.second);
    if (auto pos = s_catchTraceMap.find(key)) {
      *pos = val;
    } else {
      s_catchTraceMap.insert(key, val);
    }
  }
  catches.clear();

  if (auto profData = jit::profData()) {
    for (auto const& elm : jmpTransIDs) {
      profData->setJmpTransID(elm.first, elm.second);
    }
  }
  jmpTransIDs.clear();

  for (auto& pair : literals) {
    if (s_literals.find(pair.first)) continue;
    s_literals.insert(pair.first, pair.second);
  }
  literals.clear();

  if (inProgressTailBranches) {
    inProgressTailJumps.swap(*inProgressTailBranches);
  }
  assertx(inProgressTailJumps.empty());

  for (auto& stub : reusedStubs) {
    Debug::DebugInfo::Get()->recordRelocMap(stub, nullptr, "NewStub");
  }
  reusedStubs.clear();
}

void CGMeta::clear() {
  watchpoints.clear();
  fixups.clear();
  catches.clear();
  jmpTransIDs.clear();
  literals.clear();
  alignments.clear();
  reusedStubs.clear();
  addressImmediates.clear();
  codePointers.clear();
  inProgressTailJumps.clear();
  bcMap.clear();
}

bool CGMeta::empty() const {
  return
    watchpoints.empty() &&
    fixups.empty() &&
    catches.empty() &&
    jmpTransIDs.empty() &&
    literals.empty() &&
    alignments.empty() &&
    reusedStubs.empty() &&
    addressImmediates.empty() &&
    codePointers.empty() &&
    inProgressTailJumps.empty() &&
    bcMap.empty() &&
    true;
}

}}
