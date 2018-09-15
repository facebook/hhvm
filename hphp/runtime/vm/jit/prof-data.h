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

#ifndef incl_HPHP_PROF_TRANS_DATA_H_
#define incl_HPHP_PROF_TRANS_DATA_H_

#include "hphp/util/atomic-vector.h"

#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"

#include <folly/AtomicHashMap.h>
#include <folly/SharedMutex.h>

#include <vector>
#include <memory>

namespace HPHP { namespace jit {

struct ProfData;

extern __thread ProfData* tl_profData;

/*
 * Perform any process-global initialization required for ProfData.
 */
void processInitProfData();

/*
 * Perform any request init or exit work necessary to manage the lifetime of
 * ProfData.
 */
void requestInitProfData();
void requestExitProfData();

/*
 * Get the current ProfData*.
 *
 * The returned pointer may be nullptr, if PGO is off or if profiling data has
 * been used and freed. If a non-nullptr value is returned, it's guaranteed to
 * survive at least as long as the current request.
 */
inline ProfData* profData() {
  return tl_profData;
}

const ProfData* globalProfData();

/*
 * Mark the current ProfData for deletion.
 *
 * Actual deletion will happen some time in the future, after all currently
 * outstanding requests finish. This may be called repeatedly by multiple
 * threads without synchronization.
 */
void discardProfData();

////////////////////////////////////////////////////////////////////////////////

/**
 * A simple class of a growable number of profiling counters with fixed
 * addresses, suitable for being incremented from the TC.
 */
template<typename T>
struct ProfCounters {
  explicit ProfCounters(T initVal)
    : m_initVal(initVal)
  {}

  ProfCounters(ProfCounters&&) = default;
  ProfCounters& operator=(ProfCounters&&) = default;

  ProfCounters(const ProfCounters&) = delete;
  ProfCounters& operator=(const ProfCounters&) = delete;

  T get(uint32_t id) const {
    assertx(kCountersPerChunk % 11);
    return id / kCountersPerChunk >= m_chunks.size()
      ? m_initVal
      : m_chunks[id / kCountersPerChunk][(id * 11) % kCountersPerChunk];
  }

  T* getAddr(uint32_t id) {
    // allocate a new chunk of counters if necessary
    while (id >= m_chunks.size() * kCountersPerChunk) {
      uint32_t size = sizeof(T) * kCountersPerChunk;
      auto const chunk = new T[size];
      std::fill_n(chunk, kCountersPerChunk, m_initVal);
      m_chunks.emplace_back(chunk);
    }
    assertx(id / kCountersPerChunk < m_chunks.size());
    assertx(kCountersPerChunk % 11);
    return &(m_chunks[id / kCountersPerChunk][(id * 11) % kCountersPerChunk]);
  }

  T getDefault() const { return m_initVal; }

  void resetAllCounters(T value) {
    // We need to set m_initVal so that method transCounter() works, and also so
    // that newly created counters start with `value'.
    m_initVal = value;
    // Reset all counters already created.
    for (auto& chunk : m_chunks) {
      std::fill_n(chunk.get(), kCountersPerChunk, value);
    }
  }

private:
  static const uint32_t kCountersPerChunk = 2 * 1024 * 1024 / sizeof(T);

  T m_initVal;
  std::vector<std::unique_ptr<T[]>> m_chunks;
};

/**
 * A profiling record kept for each translation in JitPGO mode.
 */
struct ProfTransRec {
  /*
   * Construct a ProfTransRec attached to a RegionDescPtr (region must be
   * non-null), for a profiling translation.
   */
  ProfTransRec(Offset lastBcOff, SrcKey sk, RegionDescPtr region);

  /*
   * Construct a ProfTransRec for a ProfPrologue.
   */
  ProfTransRec(SrcKey sk, int nArgs);
  ~ProfTransRec();

  TransKind kind() const { return m_kind; }
  SrcKey srcKey() const { return m_sk; }
  FuncId funcId() const { return m_sk.funcID(); }
  Func* func() const { return const_cast<Func*>(m_sk.func()); }
  bool isProfile() const { return m_kind == TransKind::Profile; }
  bool isProflogue() const { return m_kind == TransKind::ProfPrologue; }

  /*
   * First BC offset in this translation.
   */
  Offset startBcOff() const {
    assertx(m_kind == TransKind::Profile);
    return m_region->start().offset();
  }

  /*
   * Last BC offset in this translation.
   *
   * Precondition: kind() == TransKind::Profile
   */
  Offset lastBcOff()  const {
    assertx(m_kind == TransKind::Profile);
    return m_lastBcOff;
  }

  /*
   * SrcKey for last offset in translation.
   *
   * Precondition: kind() == TransKind::Profile
   */
  SrcKey lastSrcKey() const {
    assertx(m_kind == TransKind::Profile);
    return SrcKey{m_sk, m_lastBcOff};
  }

  /*
   * Region for translation.
   *
   * Precondition: kind() == TransKind::Profile
   */
  RegionDescPtr region() const {
    assertx(kind() == TransKind::Profile);
    return m_region;
  }

  /*
   * Number of arguments for this proflogue.
   *
   * Precondition: kind() == TransKind::ProfPrologue
   */
  int prologueArgs() const {
    assertx(m_kind == TransKind::ProfPrologue);
    return m_prologueArgs;
  }

  /*
   * All calls in the TC which target this proflogue (directly|via the guard).
   *
   * The vector can only be used while the caller list is locked.
   *
   * Precondition: kind() == TransKind::ProfPrologue
   */
  auto& mainCallers() {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    return m_callers->main;
  }
  auto const& mainCallers() const {
    return const_cast<ProfTransRec*>(this)->mainCallers();
  }
  auto& guardCallers() {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    return m_callers->guard;
  }
  auto const& guardCallers() const {
    return const_cast<ProfTransRec*>(this)->guardCallers();
  }
  auto& profCallers() {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    return m_callers->profCallers;
  }
  auto const& profCallers() const {
    return const_cast<ProfTransRec*>(this)->profCallers();
  }
  std::unique_lock<Mutex> lockCallerList() const {
    assertx(m_kind == TransKind::ProfPrologue);
    return std::unique_lock<Mutex>{m_callers->lock};
  }

  /*
   * (Record|Erase) a call at address caller (directly|via the guard) to this
   * proflogue.
   *
   * These functions may only be called when the caller list is locked.
   *
   * Precondition: kind() == TransKind::ProfPrologue
   */
  void addMainCaller(TCA caller) {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    m_callers->main.emplace_back(caller);
  }
  void addGuardCaller(TCA caller) {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    m_callers->guard.emplace_back(caller);
  }
  void removeMainCaller(TCA caller) { removeCaller(m_callers->main, caller); }
  void removeGuardCaller(TCA caller) { removeCaller(m_callers->guard, caller); }

  /*
   * Erase the record of all calls to this proflogue.
   *
   * This function may only be called when the caller list is locked.
   *
   * Precondition: kind() == TransKind::ProfPrologue
   */
  void clearAllCallers() {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    m_callers->main.clear();
    m_callers->guard.clear();
  }
private:
  struct CallerRec {
    // main and guard are populated by profiling, and are used both to
    // smash callers when we optimize, and to build the
    // call-graph. profCallers is only populated via deserialization
    // (where main and guard are not used), and is only used to build
    // the call-graph.
    CompactVector<TCA> main;
    CompactVector<TCA> guard;
    CompactVector<TransID> profCallers;
    mutable Mutex lock;
  };
  using CallerRecPtr = std::unique_ptr<CallerRec>;

  void removeCaller(CompactVector<TCA>& v, TCA caller) {
    assertx(m_kind == TransKind::ProfPrologue);
    m_callers->lock.assertOwnedBySelf();
    auto const pos = std::find(v.begin(), v.end(), caller);
    if (pos != v.end()) v.erase(pos);
  }

  TransKind m_kind;
  union {
    Offset m_lastBcOff; // offset of the last bytecode instr
                        // for non-prologue translations
    int m_prologueArgs; // for prologues
  };
  SrcKey m_sk;
  union {
    RegionDescPtr   m_region; // for TransProfile translations
    CallerRecPtr    m_callers; // for TransProfPrologue translations
  };
};

////////////////////////////////////////////////////////////////////////////////

/**
 * ProfData encapsulates the profiling data kept by the JIT.
 *
 * Thread safety: All of ProfData's member functions may be called with no
 * external synchronization, with the caveat that care must be taken to not
 * concurrently modify the same ProfTransRec in multiple threads.
 */
struct ProfData {
  ProfData();

  ProfData(const ProfData&) = delete;
  ProfData& operator=(const ProfData&) = delete;

  struct Session final {
    Session() : m_ts(Treadmill::SessionKind::ProfData)
               { requestInitProfData(); }
    ~Session() { requestExitProfData(); }
    Session(Session&&) = delete;
    Session& operator=(Session&&) = delete;

  private:
    Treadmill::Session m_ts;
  };

  bool wasDeserialized() const { return m_wasDeserialized; }
  void setDeserialized() { m_wasDeserialized = true; }

  /*
   * Allocate a new id for a translation. Depending on the kind of the
   * translation, a TransRec for it may or may not be created later by calling
   * addTransProfile() or addTransProfPrologue().
   */
  TransID allocTransID();

  size_t numTransRecs() {
    folly::SharedMutex::ReadHolder lock{m_transLock};
    return m_transRecs.size();
  }

  ProfTransRec* transRec(TransID id) {
    folly::SharedMutex::ReadHolder lock{m_transLock};
    return m_transRecs.at(id).get();
  }
  const ProfTransRec* transRec(TransID id) const {
    return const_cast<ProfData*>(this)->transRec(id);
  }

  template<class L>
  void forEachTransRec(L&& body) {
    folly::SharedMutex::ReadHolder lock{m_transLock};
    for (auto& rec : m_transRecs) {
      if (rec) body(rec.get());
    }
  }

  TransIDVec funcProfTransIDs(FuncId funcId) const {
    folly::SharedMutex::ReadHolder lock{m_funcProfTransLock};
    auto it = m_funcProfTrans.find(funcId);
    if (it == m_funcProfTrans.end()) return TransIDVec{};

    return it->second;
  }

  /*
   * The absolute number of times that a translation executed.
   */
  int64_t transCounter(TransID id) const {
    folly::SharedMutex::ReadHolder lock{m_transLock};
    assertx(id < m_transRecs.size());
    auto const counter = m_counters.get(id);
    auto const initVal = m_counters.getDefault();
    assert_flog(initVal >= counter,
                "transCounter({}) = {}, initVal = {}\n",
                id, counter, initVal);
    return initVal - counter;
  }

  /*
   * Used for save/restore during serialization.
   */
  int64_t counterDefault() const {
    return m_counters.getDefault();
  }
  void resetCounters(int64_t val) {
    m_counters.resetAllCounters(val);
  }

  ProfCounters<int64_t> takeCounters() {
    return std::move(m_counters);
  }

  /*
   * Address at which the counter for translation id is stored.
   */
  int64_t* transCounterAddr(TransID id) {
    // getAddr() can grow the slab list, so grab a write lock.
    folly::SharedMutex::WriteHolder lock{m_transLock};
    return m_counters.getAddr(id);
  }

  /*
   * As above, if we already hold the lock.
   */
  int64_t* transCounterAddrNoLock(TransID id) {
    return m_counters.getAddr(id);
  }

  /*
   * (TransID|ProfTransRec*) for the prologue of func accepting nArgs
   * arguments.  (kInvalidTransID|nullptr) is returned if the prologue is not
   * associated with a TransID.
   */
  TransID proflogueTransId(const Func* func, int nArgs) const;
  ProfTransRec* prologueTransRec(const Func* func, int nArgs) {
    auto tid = proflogueTransId(func, nArgs);
    return tid != kInvalidTransID ? transRec(tid) : nullptr;
  }
  const ProfTransRec* prologueTransRec(const Func* func, int nArgs) const {
    return const_cast<ProfData*>(this)->prologueTransRec(func, nArgs);
  }

  /*
   * (TransID|ProfTransRec*) for the DV funclet for func when nArgs arguments
   * are passed. If no such funclet has been associated with a TransID,
   * (kInvalidTransID|nullptr) is returned.
   */
  TransID dvFuncletTransId(SrcKey sk) const;

  /*
   * Record a profiling translation: creates a ProfTransRec and returns the
   * associated TransID.
   */
  void addTransProfile(TransID, const RegionDescPtr&, const PostConditions&);
  void addTransProfPrologue(TransID, SrcKey, int);

  /*
   * Add a ProfTransRec. Only used when deserializing the profile data, and
   * must be called in single threaded context.
   */
  void addProfTrans(TransID transID, std::unique_ptr<ProfTransRec> tr);

  /*
   * Check if a (function|SrcKey) has been marked as optimized.
   */
  bool optimized(FuncId funcId) const {
    if (funcId >= m_optimizedFuncs.size()) return false;
    return m_optimizedFuncs[funcId].load(std::memory_order_acquire);
  }
  bool optimized(SrcKey sk) const {
    auto const it = m_optimizedSKs.find(sk.toAtomicInt());
    return it != m_optimizedSKs.end() && it->second;
  }

  /*
   * Indicate that an optimized translation was emitted for a (function|SrcKey).
   */
  void setOptimized(FuncId funcId) {
    m_optimizedFuncs.ensureSize(funcId + 1);
    assertx(!m_optimizedFuncs[funcId].load(std::memory_order_relaxed));
    m_optimizedFuncs[funcId].store(true, std::memory_order_release);
    m_optimizedFuncCount.fetch_add(1, std::memory_order_relaxed);
  }
  void setOptimized(SrcKey sk) {
    m_optimizedSKs.emplace(sk.toAtomicInt(), true).first->second = true;
  }

  /*
   * Returns true on the first call for the given `funcId', false for all
   * subsequent calls.
   *
   * Used to ensure that each FuncId is only put in the retranslation queue
   * once.
   */
  bool shouldQueue(FuncId funcId) {
    m_queuedFuncs.ensureSize(funcId + 1);
    return !m_queuedFuncs[funcId].exchange(true, std::memory_order_relaxed);
  }

  /*
   * Forget that a SrcKey is optimized.
   */
  void clearOptimized(SrcKey sk) {
    auto const it = m_optimizedSKs.find(sk.toAtomicInt());
    if (it == m_optimizedSKs.end()) return;

    it->second = false;
  }

  /*
   * Check if a function is being profiled.
   */
  bool profiling(FuncId funcId) const {
    if (funcId >= m_profilingFuncs.size()) return false;
    return m_profilingFuncs[funcId].load(std::memory_order_acquire);
  }

  /*
   * Indicate that a function is being profiled.
   */
  void setProfiling(FuncId funcId) {
    if (profiling(funcId)) return;

    m_profilingFuncs.ensureSize(funcId + 1);
    m_profilingFuncs[funcId].store(true, std::memory_order_release);
    m_profilingFuncCount.fetch_add(1, std::memory_order_relaxed);

    auto const func = Func::fromFuncId(funcId);
    auto const bcSize = func->past() - func->base();
    m_profilingBCSize.fetch_add(bcSize, std::memory_order_relaxed);

    static auto const bcSizeCounter =
      ServiceData::createCounter("jit.profile-bc-size");
    bcSizeCounter->setValue(profilingBCSize());
  }

  /*
   * This returns an upper bound for the FuncIds of all the functions that are
   * being profiled.  A proper call to profiling(funcId) still needs to be made
   * to check whether each funcId was indeed profiled.
   */
  FuncId maxProfilingFuncId() const {
    auto const s = m_profilingFuncs.size();
    // Avoid wrapping around and returning a large integer.
    if (s == 0) return 0;
    // Avoid returning obviously invalid FuncIds.
    if (s >= Func::nextFuncId()) return Func::nextFuncId() - 1;
    return s - 1;
  }

  /*
   * Returns the count of functions that are or were profiling or have been
   * optimized, respectively.
   */
  int64_t profilingFuncs() const {
    return m_profilingFuncCount.load(std::memory_order_relaxed);
  }
  int64_t optimizedFuncs() const {
    return m_optimizedFuncCount.load(std::memory_order_relaxed);
  }

  /*
   * Returns the total size, in bytes of bytecode, of all functions marked as
   * profiling.
   */
  int64_t profilingBCSize() const {
    return m_profilingBCSize.load(std::memory_order_relaxed);
  }

  /*
   * Returns whether any block in the given func ends at the supplied offset.
   * This is provided in this format because the region selector wants to
   * terminate profiling translations at block ends (so it doesn't care where
   * blocks start, just where they end).
   */
  bool anyBlockEndsAt(const Func*, Offset offset);

  /*
   * Check if the profile counters should be reset and, if so, do it.  This is
   * used in server mode, and it triggers once the server executes
   * RuntimeOption::EvalJitResetProfCountersRequest requests.  In the requests
   * executed before reaching this limit, the profile counters are set very high
   * so that no retranslation in optimized mode is triggered.  This allows more
   * profile translations to be produced before the counters effectively start,
   * which, in light of contention on the write lease, can both improve the
   * accuracy of the counters and allow for more portions of a function and
   * different combinations of types to be seen before retranslating the
   * function in optimized mode.
   */
  void maybeResetCounters();

  /*
   * Set the TransID for the translation owning the jmp at the given address.
   */
  void setJmpTransID(TCA jmp, TransID id) {
    m_jmpToTransID.emplace(jmp, id).first->second = id;
  }

  /*
   * Forget the TransID for the translation owning the jmp at the given address.
   */
  TransID clearJmpTransID(TCA jmp) {
    auto const it = m_jmpToTransID.find(jmp);
    if (it == m_jmpToTransID.end()) return kInvalidTransID;
    auto const ret = it->second;
    it->second = kInvalidTransID;
    return ret;
  }

  /*
   * Look up the TransID for the translation owning the jmp at the given
   * address, returning kInvalidTransID if it can't be found or has been
   * forgotten.
   */
  TransID jmpTransID(TCA jmp) const {
    auto const it = m_jmpToTransID.find(jmp);
    return it == m_jmpToTransID.end() ? kInvalidTransID : it->second;
  }

  /*
   * Support storing debug info about target profiles in profiling translations.
   */
  struct TargetProfileInfo { rds::Profile<void> key; std::string debugInfo; };
  void addTargetProfile(const TargetProfileInfo& info);
  std::vector<TargetProfileInfo> getTargetProfiles(TransID transID) const;

private:
  struct PrologueID {
    FuncId func;
    int nArgs;

    /* implicit */ operator uint64_t() const {
      assertx(nArgs >= 0);
      return (uint64_t(func) << 32) | nArgs;
    }
  };

  /*
   * m_transLock is used to protect m_transRecs, and m_counters, which are all
   * involved in the process of creating a new translation. It must be held
   * even by threads with the global write lease, to synchronize with threads
   * that don't have the write lease.
   */
  mutable folly::SharedMutex m_transLock;
  std::vector<std::unique_ptr<ProfTransRec>> m_transRecs;
  ProfCounters<int64_t> m_counters;
  std::atomic<bool> m_countersReset{false};

  /*
   * Funcs that are being profiled or have already been optimized,
   * respectively. Values in m_profilingFuncs and m_optimizedFuncs only ever
   * transition from false -> true, and as a result, the atomic counters that
   * go along with them are monotonically increasing.
   */
  AtomicVector<bool> m_profilingFuncs;
  AtomicVector<bool> m_optimizedFuncs;
  std::atomic<int64_t> m_profilingFuncCount{0};
  std::atomic<int64_t> m_profilingBCSize{0};
  std::atomic<int64_t> m_optimizedFuncCount{0};

  /*
   * Funcs that have been queued for asynchronous retranslation.
   */
  AtomicVector<bool> m_queuedFuncs;

  /*
   * SrcKeys that have already been optimized. SrcKeys are marked as not
   * optimized by setting their entry to false rather than erasing it from the
   * map, since repeatedly erasing and inserting the same key in an
   * AtomicHashMap can cause performance issues.
   */
  folly::AtomicHashMap<SrcKey::AtomicInt, bool> m_optimizedSKs;

  /*
   * Map from (FuncId, nArgs) pairs to prologue TransID.
   */
  folly::AtomicHashMap<uint64_t, TransID> m_proflogueDB;

  /*
   * Map from SrcKey.toAtomicInt() to DV funclet TransID.
   */
  folly::AtomicHashMap<uint64_t, TransID> m_dvFuncletDB;

  /*
   * Lists of profiling translations for each Func, and a lock to protect it.
   */
  mutable folly::SharedMutex m_funcProfTransLock;
  jit::fast_map<FuncId, TransIDVec> m_funcProfTrans;

  /*
   * Map from jump addresses to the ID of the translation containing them.
   */
  folly::AtomicHashMap<TCA, TransID> m_jmpToTransID;

  /*
   * Cache for Func -> block end offsets. Values in this map cannot be modified
   * after insertion so no locking is necessary for lookups.
   */
  folly::AtomicHashMap<FuncId, const jit::fast_set<Offset>>
    m_blockEndOffsets;

  mutable folly::SharedMutex m_targetProfilesLock;
  jit::fast_map<TransID, std::vector<TargetProfileInfo>> m_targetProfiles;

  bool m_wasDeserialized{false};
};

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether or not we've collected enough profile data to trigger
 * retranslateAll.
 */
bool hasEnoughProfDataToRetranslateAll();

//////////////////////////////////////////////////////////////////////

}}

#endif
