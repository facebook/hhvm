/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Basic info.

inline const MD5& UnitEmitter::md5() const {
  return m_md5;
}

inline const unsigned char* UnitEmitter::bc() const {
  return m_bc;
}

inline Offset UnitEmitter::bcPos() const {
  return m_bclen;
}

///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

inline FuncEmitter* UnitEmitter::getMain() {
  return m_fes[0];
}

inline const std::vector<FuncEmitter*>& UnitEmitter::fevec() const {
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
// Type aliases.

inline const std::vector<TypeAlias>& UnitEmitter::typeAliases() const {
  return m_typeAliases;
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

inline void UnitEmitter::emitOp(Op op, int64_t pos) {
  emitByte((unsigned char)op, pos);
}

inline void UnitEmitter::emitByte(unsigned char n, int64_t pos) {
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
    emitByte((unsigned char)n << 1);
  } else {
    assert((n & 0x7fffffff) == n);
    emitInt32((n << 1) | 0x1);
  }
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
    assert(pos + sizeof(T) <= m_bclen);
    for (uint32_t i = 0; i < sizeof(T); ++i) {
      m_bc[pos + i] = c[i];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool UnitEmitter::isASystemLib() const {
  static const char systemlib_prefix[] = "/:systemlib";
  return !strncmp(m_filepath->data(),
                  systemlib_prefix,
                  sizeof systemlib_prefix - 1);
}

///////////////////////////////////////////////////////////////////////////////
}
