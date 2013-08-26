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

using Transl::Tracelet;
using Transl::TransAnchor;
using Transl::TransPrologue;
using Transl::TransProfile;

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
  assert(kind == TransAnchor || kind == TransPrologue);
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
  return m_lastBcOff;
}

Func* ProfTransRec::func() const {
  return const_cast<Func*>(m_region->blocks[0]->func());
}

FuncId ProfTransRec::funcId() const {
  return m_sk.getFuncId();
}

RegionDescPtr ProfTransRec::region() const {
  return m_region;
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

bool ProfData::optimized(const SrcKey& sk) const {
  return mapContains(m_optimized, sk);
}

void ProfData::setOptimized(const SrcKey& sk) {
  m_optimized.insert(sk);
}

RegionDescPtr ProfData::transRegion(TransID id) const {
  assert(id < m_transRecs.size());
  const ProfTransRec& pTransRec = *m_transRecs[id];
  return pTransRec.region();
}

TransID ProfData::addTrans(const Tracelet& tracelet, TransKind kind,
                           const PostConditions& pconds) {
  TransID transId   = m_numTrans++;
  Offset  lastBcOff = tracelet.m_instrStream.last->source.offset();
  auto region = kind == TransProfile ? selectTraceletLegacy(tracelet) : nullptr;
  if (region) {
    size_t nBlocks = region->blocks.size();
    assert(nBlocks == 1 || (nBlocks > 1 && region->blocks[0]->inlinedCallee()));
    region->blocks.back()->setPostConditions(pconds);
  }
  m_transRecs.emplace_back(new ProfTransRec(transId, kind, lastBcOff,
                                            tracelet.m_sk, region));
  return transId;
}

TransID ProfData::addTransPrologue(const SrcKey& sk) {
  TransID transId = m_numTrans++;
  m_transRecs.emplace_back(new ProfTransRec(transId, TransPrologue, sk));
  return transId;
}

TransID ProfData::addTransAnchor(const SrcKey& sk) {
  TransID transId = m_numTrans++;
  m_transRecs.emplace_back(new ProfTransRec(transId, TransAnchor, sk));
  return transId;
}

} }
