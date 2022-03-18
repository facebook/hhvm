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

#pragma once

#include "hphp/runtime/vm/coeffects.h"

#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"

#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CoeffectsConfig {
  static void init(const std::unordered_map<std::string, int>&);

  static bool enabled() {
    assertx(s_instance);
    return s_instance->m_pureLevel > 0;
  }
  static bool throws() {
    assertx(s_instance);
    return s_instance->m_pureLevel > 1;
  }
  static int pureEnforcementLevel() {
    assertx(s_instance);
    return s_instance->m_pureLevel;
  }
  static int rxEnforcementLevel() {
    assertx(s_instance);
    return s_instance->m_rxLevel;
  }
  static int zonedEnforcementLevel() {
    assertx(s_instance);
    return s_instance->m_zonedLevel;
  }

  static RuntimeCoeffects::storage_t escapeMask() {
    assertx(s_instance);
    return s_instance->m_escapeMask;
  }
  static RuntimeCoeffects::storage_t warningMask() {
    assertx(s_instance);
    return s_instance->m_warningMask;
  }

  static int numUsedBits() {
    assertx(s_instance);
    return s_instance->m_numUsedBits;
  }

  static bool isPure(const StringData*);
  static bool isAnyRx(const StringData*);
  static StaticCoeffects fromName(const std::string&);
  static StaticCoeffects combine(const StaticCoeffects, const StaticCoeffects);
  static RuntimeCoeffects escapesTo(const std::string&);
  static std::vector<std::string> toStringList(const StaticCoeffects data);
  static std::string mangle();

  static CoeffectsConfig exportForParse() {
    assertx(s_instance);
    return *s_instance;
  }
  static void importForParse(const CoeffectsConfig&);

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_escapeMask)
      (m_warningMask)
      (m_pureLevel)
      (m_rxLevel)
      (m_zonedLevel)
      (m_numUsedBits);
  }

private:
  static void initEnforcementLevel(const std::unordered_map<std::string, int>&);
  static void initCapabilities();

private:
  static std::unique_ptr<CoeffectsConfig> s_instance;

private:
  RuntimeCoeffects::storage_t m_escapeMask;
  RuntimeCoeffects::storage_t m_warningMask;
  int m_pureLevel;
  int m_rxLevel;
  int m_zonedLevel;
  int m_numUsedBits;
};

///////////////////////////////////////////////////////////////////////////////
}
