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

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/native.h"

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

inline folly::Range<PreClassEmitter* const*> UnitEmitter::preclasses() const {
  return { m_pceVec.data(), m_pceVec.size() };
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
// Modules.

inline std::vector<Module>& UnitEmitter::modules() {
  return m_modules;
}

inline const std::vector<Module>& UnitEmitter::modules() const {
  return m_modules;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool UnitEmitter::isASystemLib() const {
  return FileUtil::isSystemName(m_filepath->slice());
}

///////////////////////////////////////////////////////////////////////////////

template <typename SerDe>
void UnitEmitterSerdeWrapper::serde(SerDe& sd,
                                    const Extension* extension) {
  if constexpr (SerDe::deserializing) {
    assertx(!m_ue);

    bool present;
    sd(present);
    if (present) {
      SHA1 sha1;
      const StringData* filepath;
      sd(sha1);
      sd(filepath);
      auto ue = std::make_unique<UnitEmitter>(
        sha1, SHA1{}, RepoOptions::defaults().packageInfo()
      );
      ue->m_extension = extension;
      ue->m_filepath = makeStaticString(filepath);
      ue->serde(sd, false);

      // Make sure that for systemlib units people pass down extension.
      assertx(ue->isASystemLib() == (extension != nullptr));

      m_ue = std::move(ue);
    }
  } else {
    if (m_ue) {
      sd(true);
      sd(m_ue->sha1());
      sd(m_ue->m_filepath);
      m_ue->serde(sd, false);
    } else {
      sd(false);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}
