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

#include "hphp/runtime/vm/jit/edge.h"

#include <utility>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

inline IRInstruction::IRInstruction(Opcode op,
                                    BCMarker marker,
                                    Edge* edges,
                                    uint32_t numSrcs,
                                    SSATmp** srcs)
  : m_typeParam{}
  , m_op(op)
  , m_numSrcs(numSrcs)
  , m_numDsts(0)
  , m_hasTypeParam{false}
  , m_marker(marker)
  , m_id(kTransient)
  , m_srcs(srcs)
  , m_dest(nullptr)
  , m_block(nullptr)
  , m_edges(edges)
  , m_extra(nullptr)
{
  if (op != DefConst) {
    // DefConst is the only opcode that's allowed to not have a marker, since
    // it's not part of the instruction stream.
    assertx(m_marker.valid());
  }
}

///////////////////////////////////////////////////////////////////////////////

inline bool IRInstruction::hasDst() const {
  return opcodeHasFlags(op(), HasDest);
}

inline bool IRInstruction::naryDst() const {
  return opcodeHasFlags(op(), NaryDest);
}

inline bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(op(), ConsumesRC);
}

inline bool IRInstruction::mayRaiseError() const {
  return opcodeHasFlags(op(), MayRaiseError);
}

inline bool IRInstruction::isTerminal() const {
  return opcodeHasFlags(op(), Terminal);
}

inline bool IRInstruction::hasEdges() const {
  return jit::hasEdges(op());
}

inline bool IRInstruction::isPassthrough() const {
  return opcodeHasFlags(op(), Passthrough);
}

inline bool IRInstruction::producesReference() const {
  return opcodeHasFlags(op(), ProducesRC);
}

inline SSATmp* IRInstruction::getPassthroughValue() const {
  assertx(isPassthrough());
  assertx(is(IncRef,
             CheckType, AssertType, AssertNonNull,
             MapAddElemC, ColAddNewElemC,
             CastCtxThis,
             Mov));
  return src(0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint32_t IRInstruction::id() const {
  assertx(m_id != kTransient);
  return m_id;
}

inline bool IRInstruction::isTransient() const {
  return m_id == kTransient;
}

template<typename... Args>
bool IRInstruction::is(Opcode op, Args&&... args) const {
  return m_op == op || is(std::forward<Args>(args)...);
}

inline bool IRInstruction::is() const {
  return false;
}

inline Opcode IRInstruction::op() const {
  return m_op;
}

inline const BCMarker& IRInstruction::marker() const {
  return m_marker;
}

inline BCMarker& IRInstruction::marker() {
  return m_marker;
}

inline bool IRInstruction::hasTypeParam() const { return m_hasTypeParam; }

inline Type IRInstruction::typeParam() const {
  assertx(m_hasTypeParam);
  return m_typeParam;
}

inline void IRInstruction::setTypeParam(Type t) {
  m_hasTypeParam = true;
  m_typeParam = t;
}

///////////////////////////////////////////////////////////////////////////////

inline void IRInstruction::initializeSrcs(uint32_t numSrcs, SSATmp** srcs) {
  assertx(!m_srcs && !m_numSrcs);
  m_numSrcs = numSrcs;
  m_srcs = srcs;
}

inline uint32_t IRInstruction::numSrcs() const {
  return m_numSrcs;
}

inline uint32_t IRInstruction::numDsts() const {
  return m_numDsts;
}

inline SSATmp* IRInstruction::src(uint32_t i) const {
  always_assert(i < numSrcs());
  return m_srcs[i];
}

inline SSATmp* IRInstruction::dst() const {
  assertx(!naryDst());
  return m_dest;
}

inline folly::Range<SSATmp**> IRInstruction::srcs() const {
  return folly::Range<SSATmp**>(m_srcs, m_numSrcs);
}

inline folly::Range<SSATmp**> IRInstruction::dsts() {
  assertx(naryDst() || m_numDsts <= 1);
  if (hasDst()) return folly::Range<SSATmp**>(&m_dest, m_numDsts);
  return folly::Range<SSATmp**>(m_dsts, m_numDsts);
}

inline void IRInstruction::setSrc(uint32_t i, SSATmp* newSrc) {
  always_assert(i < numSrcs());
  m_srcs[i] = newSrc;
}

inline void IRInstruction::setDst(SSATmp* newDst) {
  assertx(hasDst());
  m_dest = newDst;
  m_numDsts = newDst ? 1 : 0;
}

inline void IRInstruction::setDsts(uint32_t numDsts, SSATmp** newDsts) {
  assertx(naryDst());
  m_numDsts = numDsts;
  m_dsts = newDsts;
}

///////////////////////////////////////////////////////////////////////////////

inline bool IRInstruction::hasExtra() const {
  return m_extra;
}

template<Opcode opc>
const typename IRExtraDataType<opc>::type* IRInstruction::extra() const {
  assertx(opc == op() && "ExtraData type error");
  assertx(m_extra != nullptr);
  return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
}

template<Opcode opc>
typename IRExtraDataType<opc>::type* IRInstruction::extra() {
  assertx(opc == op() && "ExtraData type error");
  return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
}

template<class T>
const T* IRInstruction::extra() const {
  if (debug) assert_opcode_extra<T>(op());
  return static_cast<const T*>(m_extra);
}

inline const IRExtraData* IRInstruction::rawExtra() const {
  return m_extra;
}

inline void IRInstruction::setExtra(IRExtraData* data) {
  assertx(!m_extra);
  m_extra = data;
}

inline void IRInstruction::clearExtra() {
  m_extra = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

inline Block* IRInstruction::block() const {
  return m_block;
}

inline void IRInstruction::setBlock(Block* b) {
  m_block = b;
}

inline Block* IRInstruction::next() const {
  return succ(0);
}

inline Edge* IRInstruction::nextEdge() {
  return succEdge(0);
}

inline void IRInstruction::setNext(Block* b) {
  return setSucc(0, b);
}

inline Block* IRInstruction::taken() const {
  return succ(1);
}

inline Edge* IRInstruction::takenEdge() {
  return succEdge(1);
}

inline void IRInstruction::setTaken(Block* b) {
  return setSucc(1, b);
}

inline bool IRInstruction::isControlFlow() const {
  return bool(taken());
}

inline bool IRInstruction::isBlockEnd() const {
  return taken() || isTerminal();
}

inline Block* IRInstruction::succ(int i) const {
  assertx(!m_edges || hasEdges());
  return m_edges ? m_edges[i].to() : nullptr;
}

inline Edge* IRInstruction::succEdge(int i) {
  assertx(!m_edges || hasEdges());
  return m_edges && m_edges[i].to() ? &m_edges[i] : nullptr;
}

inline void IRInstruction::setSucc(int i, Block* b) {
  if (hasEdges()) {
    if (isTransient()) m_edges[i].setTransientTo(b);
    else m_edges[i].setTo(b);
  } else {
    assertx(!b && !m_edges);
  }
}

inline void IRInstruction::clearEdges() {
  setSucc(0, nullptr);
  setSucc(1, nullptr);
  m_edges = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}}
