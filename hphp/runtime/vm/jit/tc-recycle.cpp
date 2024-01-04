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

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc-intercept.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/match.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/instructions-a64.h"

#include <condition_variable>

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
namespace HPHP::jit::tc {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(reusetc);

namespace {
struct FuncInfo {
  FuncInfo() = default;
  FuncInfo(FuncInfo&&) = default;
  FuncInfo& operator=(FuncInfo&&) = default;

  std::vector<TransLoc> prologues;
  std::vector<SrcRec*>  srcRecs;
  jit::fast_set<TCA> callers;
};

struct SmashedCall {
  FuncId fid;
  ProfTransRec* rec;
};

std::mutex s_dataLock;

jit::fast_map<
  TCA /* toSmash */,
  SmashedCall /* target */
> s_smashedCalls;

jit::fast_map<
  TCA /* toSmash */,
  SrcRec* /* dest */
> s_smashedBranches;

// Keyed on FuncId as these are never reused
jit::fast_map<FuncId, FuncInfo> s_funcTCData;

struct FuncJob {
  const StringData* fname;
  FuncId fid;
};

using Job = boost::variant<FuncJob, const SrcRec*, TransLoc>;
std::atomic<bool> s_running{false};
std::queue<Job> s_jobq;
std::condition_variable s_qcv;
std::mutex s_qlock;
std::thread s_reaper;

void enqueueJob(Job j) {
  std::unique_lock<std::mutex> l{s_qlock};
  s_jobq.emplace(j);
  l.unlock();
  s_qcv.notify_all();
}

Optional<Job> dequeueJob() {
  std::unique_lock<std::mutex> l{s_qlock};
  s_qcv.wait(l, [] {
    return !s_running.load(std::memory_order_acquire) || !s_jobq.empty();
  });

  if (!s_running.load(std::memory_order_relaxed)) return std::nullopt;
  assertx(!s_jobq.empty());
  auto ret = s_jobq.front();
  s_jobq.pop();
  return ret;
}

std::unique_lock<std::mutex> lockData() {
  return std::unique_lock<std::mutex>(s_dataLock);
}

/*
 * Removes meta-data about a caller to a proflogue from prof-data to ensure that
 * a call to an optimized translation isn't wrongly smashed later.
 */
void clearProfCaller(TCA toSmash, ProfTransRec* rec) {
  if (!rec || !rec->isProflogue()) return;

  auto lock = rec->lockCallerList();
  rec->removeMainCaller(toSmash);
}

/*
 * Erase any metadata referencing a call at address start and return the
 * SmashedCall record if the call referenced a ProfTransRec.
 */
Optional<SmashedCall> eraseSmashedCall(TCA start) {
  auto dataLock = lockData();
  auto it = s_smashedCalls.find(start);
  if (it != s_smashedCalls.end()) {
    auto scall = std::move(it->second);
    ITRACE(1, "Erasing smashed call mapping @ {} to ProfTransRec {}\n",
           start, scall.rec);
    s_funcTCData[scall.fid].callers.erase(start);
    s_smashedCalls.erase(it);
    if (scall.rec) return scall;
  }
  return std::nullopt;
}

/*
 * Clear bound branch and call data associated with range [start, end) in the
 * TC. Also sets all catch-traces to null to ensure that they are reset as
 * appropriate in any future translation (the unwinder always_asserts on null
 * catch trace annotations).
 */
void clearTCMaps(TCA start, TCA end) {
  auto const profData = jit::profData();
  deleteRangeInterceptTCA(start, end);
  while (start < end) {
    bool isBranch, isNop, isCall;
    size_t instSz;
    switch (arch()) {
      case Arch::ARM: {
        using namespace vixl;
        Instruction* instr = Instruction::Cast(start);
        isBranch = instr->IsCondBranchImm() || instr->IsUncondBranchImm() ||
          instr->IsUncondBranchReg() || instr->IsCompareBranch() ||
          instr->IsTestBranch();
        isNop = instr->Mask(SystemHintFMask) == SystemHintFixed &&
          instr->ImmHint() == NOP;
        isCall = instr->Mask(UnconditionalBranchMask) == BL ||
          instr->Mask(UnconditionalBranchToRegisterMask) == BLR;
        instSz = vixl::kInstructionSize;
        break;
      }
      case Arch::X64: {
        x64::DecodedInstruction di(start);
        isBranch = di.isBranch();
        isNop = di.isNop();
        isCall = di.isCall();
        instSz = di.size();
        break;
      }
    }

    if (profData && (isBranch || isNop || isCall)) {
      auto const id = profData->clearJmpTransID(start);
      if (id != kInvalidTransID) {
        ITRACE(1, "Erasing jmpTransID @ {} to {}\n", start, id);
      }
    }

    FuncOrder::clearCallFuncId(start);

    eraseCatchTrace(start);
    eraseInlineStack(start);
    if (isCall) {
      if (auto call = eraseSmashedCall(start)) {
        clearProfCaller(start, call->rec);
      }
    }
    start += instSz;
  }
}

/*
 * Erase all metadata associated with branches to loc. This does not update the
 * associated IB records on the SrcRec for loc. Returns a vector of the erased
 * records.
 */
std::vector<std::pair<TCA, SrcRec*>> eraseSmashedBranches(TransLoc loc) {
  auto dataLock = lockData();
  std::vector<std::pair<TCA, SrcRec*>> ibRecs;
  for (auto it = s_smashedBranches.begin(); it != s_smashedBranches.end();) {
    auto br = it++;
    if (loc.contains(br->first)) {
      ITRACE(1, "Erasing smashed branch @ {} from SrcRec addr={}\n",
             br->first, (void*)br->second);
      ibRecs.emplace_back(*br);
      s_smashedBranches.erase(br);
    }
  }

  return ibRecs;
}

/*
 * Erase any metadata associated with loc from the TC.
 */
void clearTransLocMaps(TransLoc loc) {
  ITRACE(1, "Clearing translation meta-data\n");
  Trace::Indent _i2;
  clearTCMaps(loc.mainStart(), loc.mainEnd());
  clearTCMaps(loc.coldCodeStart(), loc.coldEnd());
  clearTCMaps(loc.frozenCodeStart(), loc.frozenEnd());
}

/*
 * DEBUG_ONLY: write ud2/int3 over a region of the TC beginning at start and
 * extending length bytes. Use info as the name of the associated CodeBlock.
 */
void clearRange(TCA start, size_t len, const char* info) {
  CodeBlock cb;
  cb.init(start, len, info);

  CGMeta fixups;
  SCOPE_EXIT { assertx(fixups.empty()); };

  DataBlock db;
  Vauto vasm { cb, cb, db, fixups };
  vasm.unit().padding = true;
}

/*
 * Free loc from the TC. It will be made available for reuse immediately, all
 * associated metadata must be cleared prior to calling this function.
 */
void freeTransLoc(TransLoc loc) {
  auto codeLock = lockCode();
  auto& cache = code();
  cache.blockFor(loc.mainStart()).free(loc.mainStart(), loc.mainSize());
  cache.blockFor(loc.coldStart()).free(loc.coldStart(), loc.coldSize());
  if (loc.coldStart() != loc.frozenStart()) {
    cache.blockFor(loc.frozenStart()).free(loc.frozenStart(), loc.frozenSize());
  }

  if (debug) {
    // Ensure no one calls into the function
    clearRange(loc.mainStart(), loc.mainSize(), "Dead Main");
    clearRange(loc.coldStart(), loc.coldSize(), "Dead Cold");
    if (loc.coldStart() != loc.frozenStart()) {
      clearRange(loc.frozenStart(), loc.frozenSize(), "Dead Frozen");
    }
  }
}

void reclaimTranslationSync(TransLoc loc, const SrcRec* freedSr = nullptr) {
  ITRACE(1, "Reclaiming translation M[{}, {}] C[{}, {}] F[{}, {}]\n",
         loc.mainStart(), loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
         loc.frozenStart(), loc.frozenEnd());

  Trace::Indent _i;

  // Extract the SrcRecs with smashed branches first to avoid a rank violation
  // between the data lock and SrcRec lock.
  auto ibRecs = eraseSmashedBranches(loc);

  for (auto sr : ibRecs) {
    // When called from reclaimSrcRec freedSr is the SrcRec being freed
    if (sr.second == freedSr) continue;
    sr.second->removeIncomingBranch(sr.first);
  }

  clearTransLocMaps(loc);

  // Do this last, it will make the TransLoc available for reuse.
  freeTransLoc(loc);
}

/*
 * Reclaim all translations associated with a SrcRec.
 */
void reclaimSrcRecSync(const SrcRec* rec) {
  auto srLock = rec->readlock();
  ITRACE(1, "Reclaiming SrcRec addr={}\n", (void*)rec);

  Trace::Indent _i;

  for (auto& loc : rec->translations()) {
    reclaimTranslationSync(loc, rec);
  }
}

void reclaimTranslation(TransLoc loc) { enqueueJob(loc); }
void reclaimSrcRec(const SrcRec* sr) { enqueueJob(sr); }

Optional<FuncInfo> eraseFuncInfo(FuncId fid) {
  auto dataLock = lockData();

  auto it = s_funcTCData.find(fid);
  if (it == s_funcTCData.end()) return std::nullopt;

  auto data = std::move(it->second);
  s_funcTCData.erase(it);

  for (auto& caller : data.callers) {
    s_smashedCalls.erase(caller);
  }

  return std::move(data);
}

void reclaimFunctionSync(const StringData* fname, FuncId fid) {
  ITRACE(1, "Tearing down func {} (id={})\n", fname->data(), fid);
  Trace::Indent _i;

  auto data = eraseFuncInfo(fid);
  auto& us = ustubs();

  if (!data) return;

  for (auto& caller : data->callers) {
    ITRACE(1, "Unsmashing call @ {}\n", caller);
    smashCall(caller, us.immutableBindCallStub);
  }

  // We just smashed all of those callers-- treadmill the free to avoid a
  // race (threads executing callers may end up inside the guard even though
  // the function is now unreachable). Once the following block runs the guards
  // should be unreachable.
  Treadmill::enqueue([fname, fid, data = std::move(*data)] {
    ITRACE(1, "Reclaiming func {} (id={})\n", fname, fid);
    Trace::Indent _i2;
    {
      ITRACE(1, "Reclaiming Prologues\n");
      Trace::Indent _i3;
      for (auto& loc : data.prologues) {
        reclaimTranslation(loc);
      }
    }

    for (auto* rec : data.srcRecs) {
      reclaimSrcRec(rec);
    }
  });
}
}

////////////////////////////////////////////////////////////////////////////////

int smashedCalls()    { return s_smashedCalls.size(); }
int smashedBranches() { return s_smashedBranches.size(); }
int recordedFuncs()   { return s_funcTCData.size(); }

namespace {
ServiceData::CounterCallback s_counters(
  [](std::map<std::string, int64_t>& counters) {
    if (!RuntimeOption::EvalEnableReusableTC) return;

    counters["jit.tc.smashed_calls"] = s_smashedCalls.size();
    counters["jit.tc.recorded_funcs"] = s_funcTCData.size();
    counters["jit.tc.smashed_branches"] = s_smashedBranches.size();
  }
);
}

////////////////////////////////////////////////////////////////////////////////

void recordFuncCaller(const Func* func, TCA toSmash, ProfTransRec* rec) {
  auto dataLock = lockData();

  FTRACE(1, "Recording smashed call @ {} to func {} (id = {})\n",
         toSmash, func->fullName()->data(), func->getFuncId());

  s_funcTCData[func->getFuncId()].callers.emplace(toSmash);
  s_smashedCalls[toSmash] = SmashedCall{func->getFuncId(), rec};
}

void recordFuncSrcRec(const Func* func, SrcRec* rec) {
  auto dataLock = lockData();

  FTRACE(1, "Recording SrcRec for func {} (id = {}) addr = {}\n",
         func->fullName()->data(), func->getFuncId(), (void*)rec);
  s_funcTCData[func->getFuncId()].srcRecs.emplace_back(rec);
}

void recordFuncPrologue(const Func* func, TransLoc loc) {
  auto dataLock = lockData();

  FTRACE(1, "Recording Prologue for func {} (id = {}) main={}\n",
         func->fullName()->data(), func->getFuncId(), loc.entry());
  s_funcTCData[func->getFuncId()].prologues.emplace_back(loc);
}

void recordJump(TCA toSmash, SrcRec* sr) {
  auto dataLock = lockData();

  FTRACE(1, "Recording smashed branch @ {} to SrcRec addr={}\n",
         toSmash, (void*)sr);
  s_smashedBranches[toSmash] = sr;
}

////////////////////////////////////////////////////////////////////////////////

void reclaimFunction(const Func* func) {
  enqueueJob(FuncJob {func->name(), func->getFuncId()});
}

void reclaimTranslations(GrowableVector<TransLoc>&& trans) {
  Treadmill::enqueue([trans = std::move(trans)]() mutable {
    for (auto& loc : trans) {
      reclaimTranslation(loc);
    }
  });
}


void recycleInit() {
  if (!RuntimeOption::EvalEnableReusableTC) return;

  s_running.store(true, std::memory_order_release);
  s_reaper = std::thread([] {
    rds::local::init();
    SCOPE_EXIT { rds::local::fini(); };
    while (auto j = dequeueJob()) {
      ProfData::Session pds;
      match<void>(
        *j,
        [] (TransLoc loc) { reclaimTranslationSync(loc); },
        [] (const SrcRec* sr) { reclaimSrcRecSync(sr); },
        [] (FuncJob j) { reclaimFunctionSync(j.fname, j.fid); }
      );
    }
  });
}

void recycleStop() {
  if (!s_running.load(std::memory_order_acquire)) return;
  s_running.store(false, std::memory_order_release);
  s_qcv.notify_all();
  s_reaper.join();
}

///////////////////////////////////////////////////////////////////////////////

}
