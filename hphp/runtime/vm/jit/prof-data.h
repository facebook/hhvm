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

#ifndef incl_HPHP_PROF_TRANS_DATA_H_
#define incl_HPHP_PROF_TRANS_DATA_H_

#include <vector>
#include <memory>
#include <unordered_map>

#include "hphp/util/hash-map-typedefs.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP { namespace jit {

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

  ProfCounters(const ProfCounters&) = delete;
  ProfCounters& operator=(const ProfCounters&) = delete;

  ~ProfCounters() {
    for (auto chunk : m_chunks) {
      free(chunk);
    }
  }

  T get(uint32_t id) const {
    return id / kCountersPerChunk > m_chunks.size()
      ? m_initVal
      : m_chunks[id / kCountersPerChunk][id % kCountersPerChunk];
  }

  T* getAddr(uint32_t id) {
    // allocate a new chunk of counters if necessary
    while (id >= m_chunks.size() * kCountersPerChunk) {
      uint32_t size = sizeof(T) * kCountersPerChunk;
      T* chunk = (T*)malloc(size);
      std::fill_n(chunk, kCountersPerChunk, m_initVal);
      m_chunks.push_back(chunk);
    }
    assertx(id / kCountersPerChunk < m_chunks.size());
    return &(m_chunks[id / kCountersPerChunk][id % kCountersPerChunk]);
  }

  T getDefault() const { return m_initVal; }

private:
  static const uint32_t kCountersPerChunk = 2 * 1024 * 1024 / sizeof(T);

  T m_initVal;
  std::vector<T*> m_chunks;
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
   * Construct a ProfTransRec for a Proflogue.
   */
  ProfTransRec(SrcKey sk, int nArgs);
  ~ProfTransRec();

  TransKind kind() const { return m_kind; }
  SrcKey srcKey() const { return m_sk; }
  FuncId funcId() const { return m_sk.funcID(); }
  Func* func() const { return const_cast<Func*>(m_sk.func()); }
  bool isProfile() const { return m_kind == TransKind::Profile; }
  bool isProflogue() const { return m_kind == TransKind::Proflogue; }

  /*
   * First BC offset in this translation.
   */
  Offset startBcOff() const { return m_region->start().offset(); }

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
    return SrcKey(m_sk.func(), m_lastBcOff, m_sk.resumed());
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
   * Precondition: kind() == TransKind::Proflogue
   */
  int prologueArgs() const {
    assertx(m_kind == TransKind::Proflogue);
    return m_prologueArgs;
  }

  /*
   * All calls in the TC which target this proflogue (directly|via the guard).
   *
   * Precondition: kind() == TransKind::Proflogue
   */
  const std::vector<TCA>& mainCallers() const {
    assertx(m_kind == TransKind::Proflogue);
    return m_callers.main;
  }
  const std::vector<TCA>& guardCallers() const {
    assertx(m_kind == TransKind::Proflogue);
    return m_callers.guard;
  }

  /*
   * (Record|Erase) a call at address caller (directly|via the guard) to this
   * proflogue.
   *
   * Precondition: kind() == TransKind::Proflogue
   */
  void addMainCaller(TCA caller) {
    assertx(m_kind == TransKind::Proflogue);
    m_callers.main.emplace_back(caller);
  }
  void addGuardCaller(TCA caller) {
    assertx(m_kind == TransKind::Proflogue);
    m_callers.guard.emplace_back(caller);
  }
  void removeMainCaller(TCA caller) { removeCaller(m_callers.main, caller); }
  void removeGuardCaller(TCA caller) { removeCaller(m_callers.guard, caller); }

  /*
   * Erase the record of all calls to this proflogue.
   *
   * Precondition: kind() == TransKind::Proflogue
   */
  void clearAllCallers() {
    assertx(m_kind == TransKind::Proflogue);
    m_callers.main.clear();
    m_callers.guard.clear();
  }
private:
  struct CallerRec {
    std::vector<TCA> main;
    std::vector<TCA> guard;
  };

  void removeCaller(std::vector<TCA>& v, TCA caller) {
    assertx(m_kind == TransKind::Proflogue);
    auto pos = std::find(v.begin(), v.end(), caller);
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
    RegionDescPtr m_region; // for TransProfile translations
    CallerRec m_callers; // for TransProflogue translations
  };
};

////////////////////////////////////////////////////////////////////////////////

/**
 * ProfData encapsulates the profiling data kept by the JIT.
 */
struct ProfData {
  ProfData();
  ProfData(const ProfData&) = delete;
  ProfData& operator=(const ProfData&) = delete;

  uint32_t numTrans() const { return m_numTrans; }
  TransID curTransID() const { return static_cast<TransID>(m_numTrans); }
  ProfTransRec* transRec(TransID id) { return m_transRecs.at(id).get(); }
  const ProfTransRec* transRec(TransID id) const {
    return m_transRecs.at(id).get();
  }

  const TransIDVec& funcProfTransIDs(FuncId funcId) const {
    auto it = m_funcProfTrans.find(funcId);
    assertx(it != m_funcProfTrans.end());
    return it->second;
  }

  /*
   * The actual value of translation counter, which starts at JitPGOThreshold
   * and goes down.
   */
  int64_t transCounterRaw(TransID id) const {
    assertx(id < m_numTrans);
    return m_counters.get(id);
  }

  /*
   * The absolute number of times that a translation executed.
   */
  int64_t transCounter(TransID id) const {
    assertx(id < m_numTrans);
    return m_counters.getDefault() - m_counters.get(id);
  }

  /*
   * Address at which the counter for translation id is stored.
   */
  int64_t* transCounterAddr(TransID id) { return m_counters.getAddr(id); }

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
  TransID dvFuncletTransId(const Func* func, int nArgs) const;
  ProfTransRec* dvFuncletTransRec(const Func* func, int nArgs) {
    auto tid = dvFuncletTransId(func, nArgs);
    return tid != kInvalidTransID ? transRec(tid) : nullptr;
  }
  const ProfTransRec* dvFuncletTransRec(const Func* func, int nArgs) const {
    return const_cast<ProfData*>(this)->dvFuncletTransRec(func, nArgs);
  }

  /*
   * Record a profiling translation: creates a ProfTransRec and returns the
   * associated TransID.
   */
  TransID addTransProfile(const RegionDescPtr&, const PostConditions&);
  TransID addTransProflogue(SrcKey, int);

  /*
   * Check if a (function|SrcKey) has been marked as optimized.
   */
  bool optimized(FuncId funcId) const { return m_optimizedFuncs.count(funcId); }
  bool optimized(SrcKey sk) const { return m_optimizedSKs.count(sk); }

  /*
   * Indicate that an optimized translation was emitted for a (function|SrcKey).
   */
  void setOptimized(FuncId funcId) { m_optimizedFuncs.emplace(funcId); }
  void setOptimized(SrcKey sk) { m_optimizedSKs.emplace(sk); }

  /*
   * Forget that a SrcKey is optimized.
   */
  void clearOptimized(SrcKey sk) { m_optimizedSKs.erase(sk); }

  /*
   * Check if a function is being profiled.
   */
  bool profiling(FuncId funcId) const { return m_profilingFuncs.count(funcId); }

  /*
   * Indicate that a function is being profiled.
   */
  void setProfiling(FuncId funcId);

  /*
   * Erase all profiling data.
   */
  void free();

  /*
   * Check if profiling data has been cleared.
   */
  bool freed() const { return m_freed; }

  /*
   * Called when we've finished promoting all the profiling translations for
   * `funcId' to optimized translations.  This means we can throw away any
   * allocations we made that we won't need any more for this Func.
   */
  void freeFuncData(FuncId funcId);

  /*
   * Returns whether any block in the given func ends at the supplied offset.
   * This is provided in this format because the region selector wants to
   * terminate profiling translations at block ends (so it doesn't care where
   * blocks start, just where they end).
   */
  bool anyBlockEndsAt(const Func*, Offset offset);

private:
  // PrologueID: (funcId, nArgs)
  using PrologueID = std::tuple<FuncId, int>;

  struct PrologueIDHash {
    size_t operator()(PrologueID pid) const {
      return hash_int64_pair(std::get<0>(pid), std::get<1>(pid));
    }
  };

  using FuncIdSet = std::unordered_set<FuncId>;
  using PrologueToTransMap = std::unordered_map<
    PrologueID,
    TransID,
    PrologueIDHash
  >;

  uint32_t m_numTrans{0};
  std::vector<std::unique_ptr<ProfTransRec>> m_transRecs;
  bool m_freed{false};
  std::unordered_map<FuncId, TransIDVec> m_funcProfTrans;
  ProfCounters<int64_t>  m_counters;
  SrcKeySet m_optimizedSKs;   // set of SrcKeys already optimized
  FuncIdSet m_optimizedFuncs; // set of funcs already optimized
  FuncIdSet m_profilingFuncs; // set of funcs being profiled
  PrologueToTransMap m_proflogueDB; // maps (Func,nArgs) => prolog TransID
  PrologueToTransMap m_dvFuncletDB; // maps (Func,nArgs) => DV funclet
                                    //                      TransID

  // func -> block end offsets
  std::unordered_map<FuncId,std::unordered_set<Offset>> m_blockEndOffsets;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
