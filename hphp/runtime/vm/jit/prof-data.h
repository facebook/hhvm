/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/base.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP {
namespace JIT {


/**
 * A simple class of a growable number of profiling counters with
 * fixed addresses, suitable for being incremented from the TC.
 */
template<typename T>
class ProfCounters {
 public:
  explicit ProfCounters(T initVal)
      : m_initVal(initVal)
    {}

  ProfCounters(const ProfCounters&)            = delete;
  ProfCounters& operator=(const ProfCounters&) = delete;

  ~ProfCounters() {
    for (size_t i = 0; i < m_chunks.size(); i++) {
      free(m_chunks[i]);
    }
  }

  T  get(uint32_t id) const;
  T* getAddr(uint32_t id);

 private:
  static const uint32_t kCountersPerChunk = 2 * 1024 * 1024 / sizeof(T);

  T                     m_initVal;
  vector<T*>            m_chunks;
};

typedef std::vector<TCA> PrologueCallersVec;

/**
 * A record with the callers for each profiling prologue.  Besides
 * their main entry points, prologues optionally have a guard entry
 * point that checks that we're in the right function before falling
 * through to the main prologue entry (see
 * TranslatorX64::emitFuncGuard).  We need to keep track of both kinds
 * of callers for each prologue, so that we can smash them
 * appropriately when regenerating prologues.
 */
class PrologueCallersRec : private boost::noncopyable {
 public:
  const PrologueCallersVec& mainCallers()  const;
  const PrologueCallersVec& guardCallers() const;
  void                      addMainCaller(TCA caller);
  void                      addGuardCaller(TCA caller);
  void                      clearAllCallers();

 private:
  PrologueCallersVec m_mainCallers;
  PrologueCallersVec m_guardCallers;
};

typedef std::unique_ptr<PrologueCallersRec> PrologueCallersRecPtr;

struct PrologueID {
  PrologueID(FuncId funcId, int nArgs)
      : m_funcId(funcId)
      , m_nArgs(nArgs)
    { }

  FuncId funcId() const { return m_funcId; }
  int    nArgs()  const { return m_nArgs;  }

  bool operator==(const PrologueID& other) const {
    return m_funcId == other.m_funcId && m_nArgs == other.m_nArgs;
  }

  bool operator<(const PrologueID& other) const {
    return ((m_funcId <  other.m_funcId) ||
            (m_funcId == other.m_funcId && m_nArgs < other.m_nArgs));
  }

  struct Hasher {
    size_t operator()(PrologueID pid) const {
      return hash_int64_pair(pid.funcId(), pid.nArgs());
    }
  };

 private:
  FuncId m_funcId;
  int    m_nArgs;
};

/**
 * A simple wrapper for a map from profiling prologues to TransIDs.
 */
class PrologueToTransMap {
 public:
  void    add(FuncId funcId, int numArgs, TransID transId);
  TransID get(FuncId funcId, int numArgs) const;

 private:
  hphp_hash_map<PrologueID, TransID, PrologueID::Hasher> m_prologueIdToTransId;
};

/**
 * A profiling record kept for each translation in JitPGO mode.
 */
class ProfTransRec {
 public:
  ProfTransRec(TransID id, TransKind kind, Offset lastBcOff, const SrcKey& sk,
               RegionDescPtr region);
  ProfTransRec(TransID id, TransKind kind, const SrcKey& sk);
  ProfTransRec(TransID id, TransKind kind, const SrcKey& sk, int nArgs);

  TransID              transId()    const;
  TransKind            kind()       const;
  SrcKey               srcKey()     const;
  Offset               startBcOff() const;
  Offset               lastBcOff()  const;
  Func*                func()       const;
  FuncId               funcId()     const;
  RegionDescPtr        region()     const;
  PrologueCallersRec*  prologueCallers() const;
  int                  prologueArgs() const;

 private:
  TransID              m_id;  // sequential ID of the associated translation
  TransKind            m_kind;
  union {
    Offset             m_lastBcOff;     // offset of the last bytecode instr
                                        // for non-prologue translations
    int                m_prologueArgs;  // for prologues
  };
  RegionDescPtr        m_region;           // for TransProfile translations
  PrologueCallersRecPtr m_prologueCallers; // for TransProflogue translations
  SrcKey               m_sk;
};

typedef std::unique_ptr<ProfTransRec> ProfTransRecPtr;
typedef std::unordered_map<FuncId, TransIDVec> FuncProfTransMap;

/**
 * ProfData encapsulates the profiling data kept by the JIT.
 */
class ProfData {
public:
  ProfData();

  ProfData(const ProfData&)            = delete;
  ProfData& operator=(const ProfData&) = delete;

  TransID                 numTrans()                  const;
  TransID                 curTransID()                const;

  SrcKey                  transSrcKey(TransID id)     const;
  Offset                  transStartBcOff(TransID id) const;
  Offset                  transLastBcOff(TransID id)  const;
  Op*                     transLastInstr(TransID id)  const;
  Offset                  transStopBcOff(TransID id)  const;
  FuncId                  transFuncId(TransID id)     const;
  Func*                   transFunc(TransID id)       const;
  const TransIDVec&       funcProfTransIDs(FuncId funcId) const;
  RegionDescPtr           transRegion(TransID id)     const;
  TransKind               transKind(TransID id)       const;
  int64_t                 transCounter(TransID id)    const;
  int64_t*                transCounterAddr(TransID id);
  TransID                 prologueTransId(const Func* func,
                                          int nArgs)  const;
  TransID                 dvFuncletTransId(const Func* func,
                                           int nArgs) const;
  PrologueCallersRec*     prologueCallers(TransID id) const;
  PrologueCallersRec*     prologueCallers(const Func* func, int nArgs) const;
  int                     prologueArgs(TransID id)    const;

  TransID                 addTransProfile(const Tracelet&       tracelet,
                                          Offset                initSpOffset,
                                          const PostConditions& pconds);
  TransID                 addTransNonProf(TransKind kind,
                                          const SrcKey& sk);
  TransID                 addTransPrologue(TransKind kind, const SrcKey& sk,
                                           int nArgs);
  PrologueCallersRec*     findPrologueCallersRec(const Func* func,
                                                 int nArgs) const;
  void                    addPrologueMainCaller(const Func* func, int nArgs,
                                                TCA caller);
  void                    addPrologueGuardCaller(const Func* func, int nArgs,
                                                 TCA caller);
  bool                    optimized(const SrcKey& sk) const;
  bool                    optimized(FuncId funcId) const;
  void                    setOptimized(const SrcKey& sk);
  void                    setOptimized(FuncId funcId);
  bool                    profiling(FuncId funcId) const;
  void                    setProfiling(FuncId funcId);

private:
  uint32_t                m_numTrans;
  vector<ProfTransRecPtr> m_transRecs;
  FuncProfTransMap        m_funcProfTrans;
  ProfCounters<int64_t>   m_counters;
  SrcKeySet               m_optimizedSKs;   // set of SrcKeys already optimized
  FuncIdSet               m_optimizedFuncs; // set of funcs already optimized
  FuncIdSet               m_profilingFuncs; // set of funcs being profiled
  PrologueToTransMap      m_prologueDB;  // maps (Func,nArgs) => prolog TransID
  PrologueToTransMap      m_dvFuncletDB; // maps (Func,nArgs) => DV funclet
                                         //                      TransID
};

} }

#endif
