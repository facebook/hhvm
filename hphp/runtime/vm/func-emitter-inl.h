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

#ifndef incl_HPHP_VM_FUNC_EMITTER_INL_H_
#error "func-emitter-inl.h should only be included by func-emitter.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline UnitEmitter& FuncEmitter::ue() const {
  return m_ue;
}

inline PreClassEmitter* FuncEmitter::pce() const {
  return m_pce;
}

inline int FuncEmitter::sn() const {
  return m_sn;
}

inline Id FuncEmitter::id() const {
  assertx(m_pce == nullptr);
  return m_id;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode

inline const unsigned char* FuncEmitter::bc() const {
  return m_bc.ptr();
}

inline Offset FuncEmitter::bcPos() const {
  return m_bclen;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode emit.

inline void FuncEmitter::emitOp(Op op) {
  encode_op(op, [&](uint8_t byte) { emitByte(byte); });
}

inline void FuncEmitter::emitByte(unsigned char n, int64_t pos) {
  emitImpl(n, pos);
}

inline void FuncEmitter::emitInt16(uint16_t n, int64_t pos) {
  emitImpl(n, pos);
}

inline void FuncEmitter::emitInt32(int n, int64_t pos) {
  emitImpl(n, pos);
}

inline void FuncEmitter::emitInt64(int64_t n, int64_t pos) {
  emitImpl(n, pos);
}

inline void FuncEmitter::emitDouble(double n, int64_t pos) {
  emitImpl(n, pos);
}

template<typename T>
void FuncEmitter::emitIVA(T n) {
  if (LIKELY((n & 0x7f) == n)) {
    emitByte((unsigned char)n);
  } else {
    assertx((n & 0x7fffffff) == n);
    emitInt32((n & 0x7fffff80) << 1 | 0x80 | (n & 0x7f));
  }
}

inline void FuncEmitter::emitNamedLocal(NamedLocal loc) {
  emitIVA(loc.name + 1);
  emitIVA(loc.id);
}

template<class T>
void FuncEmitter::emitImpl(T n, int64_t pos) {
  assertx(m_bc.isPtr());
  auto c = (unsigned char*)&n;
  if (pos == -1) {
    // Make sure m_bc is large enough.
    auto p = m_bc.ptr();
    while (m_bclen + sizeof(T) > m_bcmax) {
      // If m_bcmax is 0 we haven't allocated a buffer yet so start with a size
      // of BCMaxInit
      m_bcmax = m_bcmax == 0 ? BCMaxInit : m_bcmax << 1;
      p = (unsigned char*)realloc(p, m_bcmax);
    }
    memcpy(&p[m_bclen], c, sizeof(T));
    m_bc = Func::BCPtr::FromPtr(p);
    m_bclen += sizeof(T);
  } else {
    assertx(pos + sizeof(T) <= m_bclen);
    auto p = m_bc.ptr();
    for (uint32_t i = 0; i < sizeof(T); ++i) {
      p[pos + i] = c[i];
    }
    m_bc = Func::BCPtr::FromPtr(p);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and parameters.

inline Id FuncEmitter::numLocals() const {
  return m_numLocals;
}

inline Id FuncEmitter::numNamedLocals() const {
  // Don't use m_numUnnamedLocals here, it isn't serialized to the repo
  return m_localNames.size();
}

inline Id FuncEmitter::numIterators() const {
  return m_numIterators;
}

inline Id FuncEmitter::numLiveIterators() const {
  return m_nextFreeIterator;
}

inline void FuncEmitter::setNumIterators(Id numIterators) {
  assertx(m_numIterators == 0);
  m_numIterators = numIterators;
}

inline void FuncEmitter::setNumLiveIterators(Id id) {
  m_nextFreeIterator = id;
}

inline bool FuncEmitter::hasVar(const StringData* name_2) const {
  assertx(name_2 != nullptr);
  return m_localNames.contains(name_2);
}

inline Id FuncEmitter::lookupVarId(const StringData* name_2) const {
  assertx(hasVar(name_2));
  return m_localNames.find(name_2)->second;
}

inline void FuncEmitter::freeIterator(Id id) {
  --m_nextFreeIterator;
  assertx(id == m_nextFreeIterator);
}

inline void FuncEmitter::appendParam(const StringData* name_2,
                                     const ParamInfo& info) {
  allocVarId(name_2);
  params.push_back(info);
}

inline const Func::NamedLocalsMap::Builder& FuncEmitter::localNameMap() const {
  return m_localNames;
}

///////////////////////////////////////////////////////////////////////////////
// Helper accessors.

inline bool FuncEmitter::isMethod() const {
  return pce();
}

inline std::pair<int,int> FuncEmitter::getLocation() const {
  return std::make_pair(line1, line2);
}

///////////////////////////////////////////////////////////////////////////////
// Setters.

inline void FuncEmitter::setLocation(int l1, int l2) {
  line1 = l1;
  line2 = l2;
}

///////////////////////////////////////////////////////////////////////////////
// Source locations.

inline bool FuncEmitter::hasSourceLocInfo() const {
  return !m_sourceLocTab.empty();
}

inline const LineTable& FuncEmitter::lineTable() const {
  auto const p = m_lineTable.ptr();
  if (!p) {
    static LineTable empty;
    return empty;
  }
  return *p;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode.

inline Offset FuncEmitter::offsetOf(const unsigned char* pc) const {
  return pc - bc();
}

///////////////////////////////////////////////////////////////////////////////

}
