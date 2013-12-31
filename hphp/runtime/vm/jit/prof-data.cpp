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

#include "hphp/runtime/vm/jit/prof-data.h"

#include <vector>
#include "hphp/util/base.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP {
namespace JIT {

static const Trace::Module TRACEMOD = Trace::pgo;


///////////   Counters   //////////

template<typename T>
T ProfCounters<T>::get(uint32_t id) const {
  if (id / kCountersPerChunk >= m_chunks.size()) {
    return m_initVal;
  }
  return m_chunks[id / kCountersPerChunk][id % kCountersPerChunk];
}

template<typename T>
T* ProfCounters<T>::getAddr(uint32_t id) {
  // allocate a new chunk of counters if necessary
  if (id >= m_chunks.size() * kCountersPerChunk) {
    uint32_t size = sizeof(T) * kCountersPerChunk;
    T* chunk = (T*)malloc(size);
    std::fill_n(chunk, kCountersPerChunk, m_initVal);
    m_chunks.push_back(chunk);
  }
  assert(id / kCountersPerChunk < m_chunks.size());
  return &(m_chunks[id / kCountersPerChunk][id % kCountersPerChunk]);
}


///////////   PrologueCallersRec   //////////

const PrologueCallersVec& PrologueCallersRec::mainCallers() const {
  return m_mainCallers;
}

const PrologueCallersVec& PrologueCallersRec::guardCallers() const {
  return m_guardCallers;
}

void PrologueCallersRec::addMainCaller(TCA caller) {
  m_mainCallers.push_back(caller);
}

void PrologueCallersRec::addGuardCaller(TCA caller) {
  m_guardCallers.push_back(caller);
}

void PrologueCallersRec::clearAllCallers() {
  m_mainCallers.clear();
  m_guardCallers.clear();
}


///////////   PrologueToTransMap   //////////

void PrologueToTransMap::add(FuncId funcId, int numArgs, TransID transId) {
  auto pid = PrologueID(funcId, numArgs);
  assert(m_prologueIdToTransId.find(pid) == m_prologueIdToTransId.end());
  m_prologueIdToTransId[pid] = transId;
}

TransID PrologueToTransMap::get(FuncId funcId, int numArgs) const {
  auto pid = PrologueID(funcId, numArgs);
  return mapGet(m_prologueIdToTransId, pid, InvalidID);
}


///////////   ProfTransRec   //////////

ProfTransRec::ProfTransRec(TransID       id,
                           TransKind     kind,
                           Offset        lastBcOff,
                           const SrcKey& sk,
                           RegionDescPtr region)
    : m_id(id)
    , m_kind(kind)
    , m_lastBcOff(lastBcOff)
    , m_region(region)
    , m_sk(sk) {
  assert(region == nullptr ||
         (region->blocks.size() > 0 && region->blocks[0]->start() == sk));
}

ProfTransRec::ProfTransRec(TransID       id,
                           TransKind     kind,
                           const SrcKey& sk)
    : m_id(id)
    , m_kind(kind)
    , m_lastBcOff(-1)
    , m_region(nullptr)
    , m_sk(sk) {
  assert(kind == TransAnchor || kind == TransOptimize ||
         kind == TransInterp || kind == TransLive);
}

ProfTransRec::ProfTransRec(TransID       id,
                           TransKind     kind,
                           const SrcKey& sk,
                           int           nArgs)
    : m_id(id)
    , m_kind(kind)
    , m_prologueArgs(nArgs)
    , m_region(nullptr)
    , m_sk(sk) {
  assert(kind == TransPrologue || kind == TransProflogue);
  if (kind == TransProflogue) {
    // we only need to keep track of the callers for Proflogues
    m_prologueCallers = folly::make_unique<PrologueCallersRec>();
  }
}

TransID ProfTransRec::transId() const {
  return m_id;
}

TransKind ProfTransRec::kind() const {
  return m_kind;
}

SrcKey ProfTransRec::srcKey() const {
  return m_sk;
}

Offset ProfTransRec::startBcOff() const {
  return m_region->blocks[0]->start().offset();;
}

Offset ProfTransRec::lastBcOff() const {
  assert(m_kind == TransProfile);
  return m_lastBcOff;
}

int ProfTransRec::prologueArgs() const {
  assert(m_kind == TransProflogue);
  return m_prologueArgs;
}

Func* ProfTransRec::func() const {
  return const_cast<Func*>(m_sk.func());
}

FuncId ProfTransRec::funcId() const {
  return m_sk.getFuncId();
}

RegionDescPtr ProfTransRec::region() const {
  assert(kind() == TransProfile);
  return m_region;
}

PrologueCallersRec* ProfTransRec::prologueCallers() const {
  assert(kind() == TransProflogue);
  return m_prologueCallers.get();
}


///////////   ProfData   //////////

ProfData::ProfData()
    : m_numTrans(0)
    , m_counters(RuntimeOption::EvalJitPGOThreshold) {
}

uint32_t ProfData::numTrans() const {
  return m_numTrans;
}

TransID ProfData::curTransID() const {
  return numTrans();
}

SrcKey ProfData::transSrcKey(TransID id) const {
  assert(id < m_transRecs.size());
  return m_transRecs[id]->srcKey();
}

Offset ProfData::transStartBcOff(TransID id) const {
  assert(id < m_transRecs.size());
  return m_transRecs[id]->startBcOff();
}

Offset ProfData::transLastBcOff(TransID id) const {
  assert(id < m_transRecs.size());
  return m_transRecs[id]->lastBcOff();
}

Op* ProfData::transLastInstr(TransID id) const {
  Unit* unit = transFunc(id)->unit();
  Offset lastBcOff = transLastBcOff(id);
  return (Op*)(unit->at(lastBcOff));
}

Offset ProfData::transStopBcOff(TransID id) const {
  Unit*  unit      = m_transRecs[id]->func()->unit();
  Offset lastBcOff = transLastBcOff(id);
  return lastBcOff + instrLen((Op*)(unit->at(lastBcOff)));
}

FuncId ProfData::transFuncId(TransID id) const {
  assert(id < m_transRecs.size());
  return m_transRecs[id]->funcId();
}

Func* ProfData::transFunc(TransID id) const {
  assert(id < m_transRecs.size());
  return m_transRecs[id]->func();
}

const TransIDVec& ProfData::funcProfTransIDs(FuncId funcId) const {
  auto it = m_funcProfTrans.find(funcId);
  assert(it != m_funcProfTrans.end());
  return it->second;
}

TransKind ProfData::transKind(TransID id) const {
  assert(id < m_numTrans);
  return m_transRecs[id]->kind();
}

int64_t ProfData::transCounter(TransID id) const {
  assert(id < m_numTrans);
  return m_counters.get(id);
}

int64_t* ProfData::transCounterAddr(TransID id) {
  return m_counters.getAddr(id);
}

TransID ProfData::prologueTransId(const Func* func, int nArgs) const {
  int numParams = func->numParams();
  if (nArgs > numParams) nArgs = numParams + 1;
  FuncId funcId = func->getFuncId();
  return m_prologueDB.get(funcId, nArgs);
}

TransID ProfData::dvFuncletTransId(const Func* func, int nArgs) const {
  return m_dvFuncletDB.get(func->getFuncId(), nArgs);
}

PrologueCallersRec* ProfData::prologueCallers(TransID id) const {
  return m_transRecs[id]->prologueCallers();
}

PrologueCallersRec* ProfData::prologueCallers(const Func* func,
                                              int nArgs) const {
  TransID id = prologueTransId(func, nArgs);
  return prologueCallers(id);
}

int ProfData::prologueArgs(TransID id) const {
  return m_transRecs[id]->prologueArgs();
}

bool ProfData::optimized(const SrcKey& sk) const {
  return mapContains(m_optimizedSKs, sk);
}

bool ProfData::optimized(FuncId funcId) const {
  return mapContains(m_optimizedFuncs, funcId);
}

void ProfData::setOptimized(const SrcKey& sk) {
  m_optimizedSKs.insert(sk);
}

void ProfData::setOptimized(FuncId funcId) {
  m_optimizedFuncs.insert(funcId);
}

bool ProfData::profiling(FuncId funcId) const {
  return mapContains(m_profilingFuncs, funcId);
}

void ProfData::setProfiling(FuncId funcId) {
  m_profilingFuncs.insert(funcId);
}

RegionDescPtr ProfData::transRegion(TransID id) const {
  assert(id < m_transRecs.size());
  const ProfTransRec& pTransRec = *m_transRecs[id];
  return pTransRec.region();
}

TransID ProfData::addTransProfile(const Tracelet&       tracelet,
                                  Offset                initSpOffset,
                                  const PostConditions& pconds) {
  TransID transId   = m_numTrans++;
  Offset  lastBcOff = tracelet.m_instrStream.last->source.offset();
  auto       region = selectTraceletLegacy(initSpOffset, tracelet);

  assert(region);
  DEBUG_ONLY size_t nBlocks = region->blocks.size();
  assert(nBlocks == 1 || (nBlocks > 1 && region->blocks[0]->inlinedCallee()));

  region->blocks.back()->setPostConditions(pconds);
  m_transRecs.emplace_back(new ProfTransRec(transId, TransProfile, lastBcOff,
                                            tracelet.m_sk, region));

  // If the translation corresponds to a DV Funclet, then add an entry
  // into dvFuncletDB.
  const Func* func = tracelet.m_sk.func();
  FuncId    funcId = func->getFuncId();
  Offset  bcOffset = tracelet.m_sk.offset();
  if (func->isDVEntry(bcOffset)) {
    int nParams = func->getDVEntryNumParams(bcOffset);
    // Normal DV funclets don't have type guards, and thus have a
    // single translation.  However, some special functions written
    // in hhas (e.g. array_map) have complex DV funclets that get
    // retranslated for different types.  For those functions,
    // m_dvFuncletDB keeps the TransID for their first translation.
    if (m_dvFuncletDB.get(funcId, nParams) == InvalidID) {
      m_dvFuncletDB.add(funcId, nParams, transId);
    }
  }

  m_funcProfTrans[funcId].push_back(transId);
  return transId;
}

TransID ProfData::addTransPrologue(TransKind kind, const SrcKey& sk,
                                   int nArgs) {
  assert(kind == TransPrologue || kind == TransProflogue);
  TransID transId = m_numTrans++;
  m_transRecs.emplace_back(new ProfTransRec(transId, kind, sk, nArgs));
  if (kind == TransProflogue) {
    // only Proflogue translations need an entry in the m_prologueDB
    m_prologueDB.add(sk.getFuncId(), nArgs, transId);
  }
  return transId;
}

TransID ProfData::addTransNonProf(TransKind kind, const SrcKey& sk) {
  assert(kind == TransAnchor || kind == TransInterp ||
         kind == TransLive   || kind == TransOptimize);
  TransID transId = m_numTrans++;
  m_transRecs.emplace_back(new ProfTransRec(transId, kind, sk));
  return transId;
}

PrologueCallersRec* ProfData::findPrologueCallersRec(const Func* func,
                                                     int nArgs) const {
  TransID tid = prologueTransId(func, nArgs);
  if (tid == InvalidID) {
    assert(RuntimeOption::EvalJitPGOHotOnly && !(func->attrs() & AttrHot));
    return nullptr;
  }
  assert(transKind(tid) == TransProflogue);
  PrologueCallersRec* prologueCallers = m_transRecs[tid]->prologueCallers();
  assert(prologueCallers);
  return prologueCallers;
}

void ProfData::addPrologueMainCaller(const Func* func, int nArgs, TCA caller) {
  PrologueCallersRec* prologueCallers = findPrologueCallersRec(func, nArgs);
  if (prologueCallers) {
    prologueCallers->addMainCaller(caller);
  }
}

void ProfData::addPrologueGuardCaller(const Func* func, int nArgs, TCA caller) {
  PrologueCallersRec* prologueCallers = findPrologueCallersRec(func, nArgs);
  if (prologueCallers) {
    prologueCallers->addGuardCaller(caller);
  }
}

} }
