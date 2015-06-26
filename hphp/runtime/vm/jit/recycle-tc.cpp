/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/recycle-tc.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/trace.h"

#include <folly/MoveWrapper.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(reusetc);

namespace {
struct PrologueCaller {
  bool isGuard;
  bool isProfiled;
  int  numArgs;
};

struct FuncInfo {
  std::vector<TransLoc> prologues;
  std::vector<SrcRec*>  srcRecs;
  std::unordered_map<TCA /* toSmash */, PrologueCaller> callers;
};

std::unordered_map<
  TCA /* toSmash */,
  const Func* /* target */
> s_smashedCalls;

std::unordered_map<
  TCA /* toSmash */,
  SrcRec* /* dest */
> s_smashedBranches;

std::unordered_map<const Func*, FuncInfo> s_funcTCData;

/*
 * Removes meta-data about a caller to a proflogue from prof-data to ensure that
 * a call to an optimized translation isn't wrongly smashed later.
 */
void clearProfCaller(TCA toSmash, const Func* func, int numArgs,
                            bool isGuard) {
  auto data = mcg->tx().profData();
  auto tid = data->prologueTransId(func, numArgs);
  if (tid == kInvalidTransID || !data->hasTransRec(tid) ||
      data->transKind(tid) != TransKind::Proflogue) {
    return;
  }
  auto* pc = data->prologueCallers(tid);
  if (isGuard) {
    pc->removeGuardCaller(toSmash);
    return;
  }
  pc->removeMainCaller(toSmash);
}

/*
 * Clear bound branch and call data associated with range [start, end) in the
 * TC. Also sets all catch-traces to null to ensure that they are reset as
 * appropriate in any future translation (the unwinder always_asserts on null
 * catch trace annotations).
 */
void clearTCMaps(TCA start, TCA end) {
  auto& catchMap = mcg->catchTraceMap();
  auto& jmpMap = mcg->getJmpToTransIDMap();
  while (start < end) {
    DecodedInstruction di (start);
    if (di.isBranch()) {
      auto it = jmpMap.find(start);
      if (it != jmpMap.end()) {
        ITRACE(1, "Erasing JMP @ {}\n", start);
        jmpMap.erase(it);
      }
    }
    if (auto* ct = catchMap.find(start)) {
      // We mark nothrow with a nullptr, which will assert during unwinding,
      // use a separate marker here to indicate the catch has been erased
      *ct = kInvalidCatchTrace;
    }
    if (di.isCall()) {
      auto it = s_smashedCalls.find(start);
      if (it != s_smashedCalls.end()) {
        auto func = it->second;
        ITRACE(1, "Erasing smashed call mapping @ {} to func {} (id = {})\n",
               start, func->fullName()->data(), func->getFuncId());
        auto dataIt = s_funcTCData[func].callers.find(start);
        if (dataIt->second.isProfiled) {
          clearProfCaller(start, func, dataIt->second.numArgs,
                          dataIt->second.isGuard);
        }
        s_funcTCData[func].callers.erase(dataIt);
        s_smashedCalls.erase(it);
      }
    }
    start += di.size();
  }
}

/*
 * Reclaim all translations associated with a SrcRec including the anchor
 * translation.
 */
void reclaimSrcRec(const SrcRec* rec) {
  assertx(Translator::WriteLease().amOwner());

  ITRACE(1, "Reclaiming SrcRec addr={} anchor={}\n", (void*)rec,
         rec->getFallbackTranslation());

  Trace::Indent _i;

  auto anchor = rec->getFallbackTranslation();
  mcg->code.blockFor(anchor).free(anchor, mcg->backEnd().reusableStubSize());

  for (auto& loc : rec->translations()) {
    reclaimTranslation(loc);
  }
}
}

////////////////////////////////////////////////////////////////////////////////

int smashedCalls()    { return s_smashedCalls.size(); }
int smashedBranches() { return s_smashedBranches.size(); }
int recordedFuncs()   { return s_funcTCData.size(); }

////////////////////////////////////////////////////////////////////////////////

void recordFuncCaller(const Func* func, TCA toSmash, bool immutable,
                      bool profiled, int numArgs) {
  assertx(Translator::WriteLease().amOwner());

  FTRACE(1, "Recording smashed call @ {} to func {} (id = {})\n",
         toSmash, func->fullName()->data(), func->getFuncId());

  PrologueCaller pc {!immutable, profiled, numArgs};
  s_funcTCData[func].callers.emplace(toSmash, pc);
  s_smashedCalls[toSmash] = func;
}

void recordFuncSrcRec(const Func* func, SrcRec* rec) {
  assertx(Translator::WriteLease().amOwner());

  FTRACE(1, "Recording SrcRec for func {} (id = {}) addr = {}\n",
         func->fullName()->data(), func->getFuncId(), (void*)rec);
  s_funcTCData[func].srcRecs.emplace_back(rec);
}

void recordFuncPrologue(const Func* func, TransLoc loc) {
  assertx(Translator::WriteLease().amOwner());

  FTRACE(1, "Recording Prologue for func {} (id = {}) main={}\n",
         func->fullName()->data(), func->getFuncId(), loc.mainStart());
  s_funcTCData[func].prologues.emplace_back(loc);
}

void recordJump(TCA toSmash, SrcRec* sr) {
 assertx(MCGenerator::canWrite());

  FTRACE(1, "Recording smashed branch @ {} to SrcRec addr={}\n",
         toSmash, (void*)sr);
  s_smashedBranches[toSmash] = sr;
}

////////////////////////////////////////////////////////////////////////////////

void reclaimTranslation(TransLoc loc) {
  BlockingLeaseHolder writer(Translator::WriteLease());
  if (!writer) return;

  ITRACE(1, "Reclaiming translation M[{}, {}] C[{}, {}] F[{}, {}]\n",
         loc.mainStart(), loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
         loc.frozenStart(), loc.frozenEnd());

  Trace::Indent _i;

  auto& cache = mcg->code;
  cache.blockFor(loc.mainStart()).free(loc.mainStart(), loc.mainSize());
  cache.blockFor(loc.coldStart()).free(loc.coldStart(), loc.coldSize());
  if (loc.coldStart() != loc.frozenStart()) {
    cache.blockFor(loc.frozenStart()).free(loc.frozenStart(), loc.frozenSize());
  }

  for (auto it = s_smashedBranches.begin(); it != s_smashedBranches.end();) {
    auto br = it++;
    if (loc.contains(br->first)) {
      ITRACE(1, "Erasing smashed branch @ {} from SrcRec addr={}\n",
             br->first, (void*)br->second);
      br->second->removeIncomingBranch(br->first);
      s_smashedBranches.erase(br);
    }
  }

  // Erase meta-data about these regions of the TC
  {
    ITRACE(1, "Clearing translation meta-data\n");
    Trace::Indent _i;
    clearTCMaps(loc.mainStart(), loc.mainEnd());
    clearTCMaps(loc.coldCodeStart(), loc.coldEnd());
    clearTCMaps(loc.frozenCodeStart(), loc.frozenEnd());
  }

  if (debug) {
    // Ensure no one calls into the function
    ITRACE(1, "Overwriting function\n");
    auto clearBlock = [] (CodeBlock& cb) {
      Asm a {cb};
      while (cb.available() >= 2) a.ud2();
      if (cb.available() > 0) a.int3();
      always_assert(!cb.available());
    };

    CodeBlock main, cold, frozen;
    main.init(loc.mainStart(), loc.mainSize(), "Dead Main");
    cold.init(loc.coldStart(), loc.coldSize(), "Dead Cold");
    frozen.init(loc.frozenStart(), loc.frozenSize(), "Dead Frozen");

    clearBlock(main);
    clearBlock(cold);
    clearBlock(frozen);
  }
}

void reclaimFunction(Func* func) {
  BlockingLeaseHolder writer(Translator::WriteLease());
  if (!writer) return;

  auto it = s_funcTCData.find(func);
  if (it == s_funcTCData.end()) return;

  ITRACE(1, "Tearing down func {} (id={})\n",
         func->fullName()->data(), func->getFuncId());

  Trace::Indent _i;

  auto& data = it->second;
  auto& us = mcg->tx().uniqueStubs;

  ITRACE(1, "Smashing prologues\n");
  func->smashPrologues();

  for (auto& caller : data.callers) {
    ITRACE(1, "Unsmashing call @ {} (guard = {})\n",
           caller.first, caller.second.isGuard);

    // It should be impossible to reach a prologue that has been reclaimed
    // through an immutable stub, as this would imply the function is still
    // reachable.
    auto addr = caller.second.isGuard ? us.bindCallStub
                                      : nullptr;
    mcg->backEnd().smashCall(caller.first, addr);
    s_smashedCalls.erase(caller.first);
  }

  auto movedData = folly::makeMoveWrapper(std::move(data));
  auto fname = func->fullName()->data();
  auto fid   = func->getFuncId();
  // We just smashed all of those callers-- treadmill the free to avoid a
  // race (threads executing callers may end up inside the guard even though
  // the function is now unreachable). Once the following block runs the guards
  // should be unreachable.
  Treadmill::enqueue([fname, fid, movedData] {
    BlockingLeaseHolder writer(Translator::WriteLease());
    if (!writer) return;

    ITRACE(1, "Reclaiming func {} (id={})\n", fname, fid);
    Trace::Indent _i;
    {
      ITRACE(1, "Reclaiming Prologues\n");
      Trace::Indent _i;
      for (auto& loc : movedData->prologues) {
        reclaimTranslation(loc);
      }
    }

    for (auto* rec : movedData->srcRecs) {
      reclaimSrcRec(rec);
    }
  });

  s_funcTCData.erase(it);
}

}}
