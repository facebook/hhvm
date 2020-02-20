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

#ifndef incl_HPHP_VM_UNIT_EMITTER_INL_H_
#error "unit-emitter-inl.h should only be included by unit-emitter.h"
#endif

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/vm/hhbc-codec.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Basic info.

inline const SHA1& UnitEmitter::sha1() const {
  return m_sha1;
}

inline const SHA1& UnitEmitter::bcSha1() const {
  return m_bcSha1;
}

inline const unsigned char* UnitEmitter::bc() const {
  return m_bc;
}

inline Offset UnitEmitter::bcPos() const {
  return m_bclen;
}

inline Offset UnitEmitter::offsetOf(const unsigned char* pc) const {
  return pc - m_bc;
}

///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

inline FuncEmitter* UnitEmitter::getMain() const {
  return m_fes[0].get();
}

inline auto const& UnitEmitter::fevec() const {
  return m_fes;
}

///////////////////////////////////////////////////////////////////////////////
// PreClassEmitters.

inline size_t UnitEmitter::numPreClasses() const {
  return m_pceVec.size();
}

inline PreClassEmitter* UnitEmitter::pce(Id preClassId) {
  return m_pceVec[preClassId];
}

inline const PreClassEmitter* UnitEmitter::pce(Id preClassId) const {
  return m_pceVec[preClassId];
}

///////////////////////////////////////////////////////////////////////////////
// RecordEmitters.

inline size_t UnitEmitter::numRecords() const {
  return m_reVec.size();
}

inline RecordEmitter* UnitEmitter::re(Id recordId) {
  return m_reVec[recordId];
}

inline const RecordEmitter* UnitEmitter::re(Id recordId) const {
  return m_reVec[recordId];
}

///////////////////////////////////////////////////////////////////////////////
// Type aliases.

inline const std::vector<TypeAlias>& UnitEmitter::typeAliases() const {
  return m_typeAliases;
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

inline const std::vector<Constant>& UnitEmitter::constants() const {
  return m_constants;
}

///////////////////////////////////////////////////////////////////////////////
// Source locations.

inline bool UnitEmitter::hasSourceLocInfo() const {
  return !m_sourceLocTab.empty();
}

inline const LineTable& UnitEmitter::lineTable() const {
  return m_lineTable;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode emit.

inline void UnitEmitter::emitOp(Op op) {
  encode_op(op, [&](uint8_t byte) { emitByte(byte); });
}

inline void UnitEmitter::emitByte(unsigned char n, int64_t pos) {
  emitImpl(n, pos);
}

inline void UnitEmitter::emitInt16(uint16_t n, int64_t pos) {
  emitImpl(n, pos);
}

inline void UnitEmitter::emitInt32(int n, int64_t pos) {
  emitImpl(n, pos);
}

inline void UnitEmitter::emitInt64(int64_t n, int64_t pos) {
  emitImpl(n, pos);
}

inline void UnitEmitter::emitDouble(double n, int64_t pos) {
  emitImpl(n, pos);
}

template<typename T>
void UnitEmitter::emitIVA(T n) {
  if (LIKELY((n & 0x7f) == n)) {
    emitByte((unsigned char)n);
  } else {
    assertx((n & 0x7fffffff) == n);
    emitInt32((n & 0x7fffff80) << 1 | 0x80 | (n & 0x7f));
  }
}

inline void UnitEmitter::emitNamedLocal(NamedLocal loc) {
  emitIVA(loc.name + 1);
  emitIVA(loc.id);
}

template<class T>
void UnitEmitter::emitImpl(T n, int64_t pos) {
  auto c = (unsigned char*)&n;
  if (pos == -1) {
    // Make sure m_bc is large enough.
    while (m_bclen + sizeof(T) > m_bcmax) {
      m_bc = (unsigned char*)realloc(m_bc, m_bcmax << 1);
      m_bcmax <<= 1;
    }
    memcpy(&m_bc[m_bclen], c, sizeof(T));
    m_bclen += sizeof(T);
  } else {
    assertx(pos + sizeof(T) <= m_bclen);
    for (uint32_t i = 0; i < sizeof(T); ++i) {
      m_bc[pos + i] = c[i];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool UnitEmitter::isASystemLib() const {
  return FileUtil::isSystemName(m_filepath->slice());
}

///////////////////////////////////////////////////////////////////////////////
}
