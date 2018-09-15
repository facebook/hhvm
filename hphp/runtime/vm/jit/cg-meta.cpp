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

#include "hphp/runtime/vm/jit/cg-meta.h"

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/tread-hash-map.h"

#include "hphp/util/atomic-vector.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(mcg);

namespace {

std::atomic<IFrameID> s_nextFrameKey;

// Map from integral literals to their location in the TC data section.
using LiteralMap = TreadHashMap<uint64_t,const uint64_t*,std::hash<uint64_t>>;
LiteralMap s_literals{128};

// Landingpads for TC catch traces; used by the unwinder.
using CatchTraceMap = TreadHashMap<uint32_t, uint32_t, std::hash<uint32_t>>;
CatchTraceMap s_catchTraceMap{128};

using AbortReasonMap = TreadHashMap<uint32_t, Reason, std::hash<uint32_t>>;
AbortReasonMap s_trapReasonMap{128};

using InlineStackMap = TreadHashMap<uint32_t, IStack, std::hash<uint32_t>>;
InlineStackMap s_inlineStacks{1024};

using InlineFrameVec = AtomicVector<IFrame>;
InlineFrameVec s_inlineFrames{4096,IFrame{}};

constexpr uint32_t kInvalidCatchTrace = 0x0;

IFrameID insertFrames(const std::vector<IFrame>& frames) {
  auto const start = s_nextFrameKey.fetch_add(frames.size());
  s_inlineFrames.ensureSize(start + frames.size());

  for (IFrameID i = 0; i < frames.size(); ++i) {
    auto& f = frames[i];
    auto newFrame = IFrame{f.func, f.soff, f.parent + start};
    s_inlineFrames.exchange(start + i, newFrame);
  }

  return start;
}

bool isFakeAddr(CTCA addr) {
  return (int64_t)addr < 0;
}

Offset fromFakeAddr(CTCA addr) {
  return (int32_t)reinterpret_cast<int64_t>(addr);
}

Offset stackAddrToOffset(CTCA addr) {
  return isFakeAddr(addr) ? fromFakeAddr(addr) : tc::addrToOffset(addr);
}

void insertStacks(
  IFrameID start, const std::vector<std::pair<TCA,IStack>>& stacks
) {
  for (auto& stk : stacks) {
    if (!stk.second.nframes) continue;
    auto off = stackAddrToOffset(stk.first);
    auto val = stk.second;
    val.frame += start;
    if (auto pos = s_inlineStacks.find(off)) {
      *pos = val;
    } else {
      s_inlineStacks.insert(off, val);
    }
  }
}

void processInlineFrames(const CGMeta& cm) {
  auto const start = insertFrames(cm.inlineFrames);
  insertStacks(start, cm.inlineStacks);
}

}

folly::Optional<IStack> inlineStackAt(CTCA addr) {
  auto off = stackAddrToOffset(addr);
  if (auto pos = s_inlineStacks.find(off)) {
    return *pos;
  }
  return folly::none;
}

IFrame getInlineFrame(IFrameID id) {
  return s_inlineFrames[id];
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

Reason* getTrapReason(CTCA addr) {
  return s_trapReasonMap.find(tc::addrToOffset(addr));
}

void poolLiteral(CodeBlock& cb, CGMeta& meta, uint64_t val, uint8_t width,
                  bool smashable) {
  meta.literalsToPool.emplace_back(
    CGMeta::PoolLiteralMeta {
      val,
      cb.frontier(),
      smashable,
      width
    }
  );
}

void addVeneer(CGMeta& meta, TCA source, TCA target) {
  FTRACE(5, "addVeneer: source = {}, target = {}\n", source, target);
  meta.veneers.emplace_back(CGMeta::VeneerData{source, target});
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

void CGMeta::process_literals() {
  assertx(literalsToPool.empty());
  for (auto& pair : literalAddrs) {
    if (s_literals.find(pair.first)) {
      // TreadHashMap doesn't allow re-inserting existing keys
      continue;
    }
    s_literals.insert(pair.first, pair.second);
  }
  literalAddrs.clear();
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

  processInlineFrames(*this);
  inlineFrames.clear();
  inlineStacks.clear();

  for (auto const& ct : catches) {
    auto const key = tc::addrToOffset(ct.first);
    auto const val = tc::addrToOffset(ct.second);
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

  for (auto const& pair : trapReasons) {
    auto addr = tc::addrToOffset(pair.first);
    if (auto r = s_trapReasonMap.find(addr)) {
      *r = pair.second;
    } else {
      s_trapReasonMap.insert(addr, pair.second);
    }
  }
  trapReasons.clear();

  process_literals();

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
  inlineFrames.clear();
  inlineStacks.clear();
  jmpTransIDs.clear();
  literalsToPool.clear();
  literalAddrs.clear();
  veneers.clear();
  alignments.clear();
  reusedStubs.clear();
  addressImmediates.clear();
  fallthru.clear();
  codePointers.clear();
  inProgressTailJumps.clear();
  bcMap.clear();
  smashableCallData.clear();
  smashableJumpData.clear();
}

bool CGMeta::empty() const {
  return
    watchpoints.empty() &&
    fixups.empty() &&
    catches.empty() &&
    inlineFrames.empty() &&
    inlineStacks.empty() &&
    jmpTransIDs.empty() &&
    literalsToPool.empty() &&
    literalAddrs.empty() &&
    veneers.empty() &&
    alignments.empty() &&
    reusedStubs.empty() &&
    addressImmediates.empty() &&
    !fallthru.hasValue() &&
    codePointers.empty() &&
    inProgressTailJumps.empty() &&
    bcMap.empty() &&
    smashableCallData.empty() &&
    smashableJumpData.empty();
}

}}
