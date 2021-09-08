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

///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

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
// Type aliases.

inline auto const& UnitEmitter::typeAliases() const {
  return m_typeAliases;
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

inline std::vector<Constant>& UnitEmitter::constants() {
  return m_constants;
}

inline const std::vector<Constant>& UnitEmitter::constants() const {
  return m_constants;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool UnitEmitter::isASystemLib() const {
  return FileUtil::isSystemName(m_filepath->slice());
}

///////////////////////////////////////////////////////////////////////////////
}
