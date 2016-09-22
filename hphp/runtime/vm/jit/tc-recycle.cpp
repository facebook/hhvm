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

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

#include <folly/MoveWrapper.h>

/*
 * This module implements garbage collection for the translation cache so that
 * unreachable translations may be overridden by new translations.
 *
 * Unreachable translations are created by either:
 *  (1) Freeing a function through the treadmill
 *  (2) Replacing profiling translations in a SrcRec
 *
 * SrcRecs and prologues are recorded as they are emitted in to the TC so that
 * when their associated function becomes unreachable they may be freed. In the
 * case of profiling translations, these are sometimes freed eagerly when they
 * become unreachable, as they will be erased from their associated SrcRec and
 * are not tracked elsewhere.
 *
 * Function callers and inter-translation jumps are recorded so that they may
 * be smashed when space is reclaimed with the TC.
 *
 * Freed memory is tracked and allocated using the policy defined in DataBlock,
 * and allocation is performed in MCGenerator following the creation of a new
 * translation.
 *
 * Rather than emit new translations directly into freed memory they are written
 * at the end of the TC and then relocated into freed memory. As the space
 * required for a translation will be unknown until it is complete this strategy
 * allows allocation of an appropriately sized block.
 *
 * Currently all allocation and deallocation is done eagerly, therefore the
 * performance of the module is dependent on accurately detecting unreachable
 * functions and translations.
 *
 * This module exports diagnostic data in the form of counts of smashed calls
 * and branches, and recorded functions. Higher level diagnostic data exported
 * by DataBlock may be of more use in tracking TC health. In particular, the
 * number of free bytes and free blocks give a rough measure of fragmentation
 * within the allocator.
 *
 * See DataBlock for details about the allocation strategy and free memory
 * tracking.
 */
namespace HPHP { namespace jit { namespace tc {

///////////////////////////////////////////////////////////////////////////////

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
  auto data = jit::profData();
  assertx(data);
  auto const tid = data->proflogueTransId(func, numArgs);
  if (tid == kInvalidTransID) return;

  auto rec = data->transRec(tid);
  if (!rec || !rec->isProflogue()) return;

  if (isGuard) {
    rec->removeGuardCaller(toSmash);
    return;
  }
  rec->removeMainCaller(toSmash);
}

/*
 * Clear bound branch and call data associated with range [start, end) in the
 * TC. Also sets all catch-traces to null to ensure that they are reset as
 * appropriate in any future translation (the unwinder always_asserts on null
 * catch trace annotations).
 */
void clearTCMaps(TCA start, TCA end) {
  auto const profData = jit::profData();
  while (start < end) {
#if defined(__powerpc64__)
    ppc64_asm::DecodedInstruction di(start);
#else
    x64::DecodedInstruction di(start);
#endif
    if (profData && (di.isBranch() || di.isNop())) {
      auto const id = profData->clearJmpTransID(start);
      if (id != kInvalidTransID) {
        ITRACE(1, "Erasing jmpTransID @ {} to {}\n",
               start, id);
      }
    }
    eraseCatchTrace(start);
    if (di.isCall()) {
      auto it = s_smashedCalls.find(start);
      if (it != s_smashedCalls.end()) {
        auto func = it->second;
        ITRACE(1, "Erasing smashed call mapping @ {} to func {} (id = {})\n",
               start, func->fullName(), func->getFuncId());
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

void reclaimTranslation(TransLoc loc) {
  assertOwnsCodeLock();
  assertOwnsMetadataLock();

  ITRACE(1, "Reclaiming translation M[{}, {}] C[{}, {}] F[{}, {}]\n",
         loc.mainStart(), loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
         loc.frozenStart(), loc.frozenEnd());

  Trace::Indent _i;

  auto& cache = code();
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
      CGMeta fixups;
      SCOPE_EXIT { assert(fixups.empty()); };

      DataBlock db;
      Vauto vasm { cb, cb, db, fixups };
      vasm.unit().padding = true;
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

/*
 * Reclaim all translations associated with a SrcRec including the anchor
 * translation.
 */
void reclaimSrcRec(const SrcRec* rec) {
  assertOwnsCodeLock();
  assertOwnsMetadataLock();

  ITRACE(1, "Reclaiming SrcRec addr={} anchor={}\n", (void*)rec,
         rec->getFallbackTranslation());

  Trace::Indent _i;

  auto anchor = rec->getFallbackTranslation();
  code().blockFor(anchor).free(anchor, svcreq::stub_size());

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
  assertOwnsCodeLock();

  FTRACE(1, "Recording smashed call @ {} to func {} (id = {})\n",
         toSmash, func->fullName()->data(), func->getFuncId());

  PrologueCaller pc {!immutable, profiled, numArgs};
  s_funcTCData[func].callers.emplace(toSmash, pc);
  s_smashedCalls[toSmash] = func;
}

void recordFuncSrcRec(const Func* func, SrcRec* rec) {
  assertOwnsCodeLock();

  FTRACE(1, "Recording SrcRec for func {} (id = {}) addr = {}\n",
         func->fullName()->data(), func->getFuncId(), (void*)rec);
  s_funcTCData[func].srcRecs.emplace_back(rec);
}

void recordFuncPrologue(const Func* func, TransLoc loc) {
  assertOwnsCodeLock();

  FTRACE(1, "Recording Prologue for func {} (id = {}) main={}\n",
         func->fullName()->data(), func->getFuncId(), loc.mainStart());
  s_funcTCData[func].prologues.emplace_back(loc);
}

void recordJump(TCA toSmash, SrcRec* sr) {
 assertOwnsCodeLock();

  FTRACE(1, "Recording smashed branch @ {} to SrcRec addr={}\n",
         toSmash, (void*)sr);
  s_smashedBranches[toSmash] = sr;
}

////////////////////////////////////////////////////////////////////////////////

void reclaimFunction(const Func* func) {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  auto it = s_funcTCData.find(func);
  if (it == s_funcTCData.end()) return;

  ITRACE(1, "Tearing down func {} (id={})\n",
         func->fullName()->data(), func->getFuncId());

  Trace::Indent _i;

  auto& data = it->second;
  auto& us = ustubs();

  ITRACE(1, "Smashing prologues\n");
  clobberFuncGuards(func);

  for (auto& caller : data.callers) {
    ITRACE(1, "Unsmashing call @ {} (guard = {})\n",
           caller.first, caller.second.isGuard);

    // It should be impossible to reach a prologue that has been reclaimed
    // through an immutable stub, as this would imply the function is still
    // reachable.
    auto addr = caller.second.isGuard ? us.bindCallStub
                                      : caller.first;
    smashCall(caller.first, addr);
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
    auto codeLock = lockCode();
    auto metaLock = lockMetadata();

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

void reclaimTranslations(GrowableVector<TransLoc>&& trans) {
    auto twrap = folly::makeMoveWrapper(std::move(trans));
    Treadmill::enqueue([twrap]() mutable {
      auto codeLock = lockCode();
      auto metaLock = lockMetadata();
      for (auto& loc : *twrap) {
        reclaimTranslation(loc);
      }
      twrap->clear();
    });
}

///////////////////////////////////////////////////////////////////////////////

}}}
