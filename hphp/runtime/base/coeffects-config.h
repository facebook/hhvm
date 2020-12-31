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

#include "hphp/util/assertions.h"

#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CoeffectsConfig {
  static void init(const std::unordered_map<std::string, int>& map) {
    assertx(!s_instance);
    s_instance = std::make_unique<CoeffectsConfig>();
    // Purity enforcement must be at least as strong as the highest level of
    // enforcement otherwise the whole coeffect system breaks
    s_instance->m_pureLevel = 0;
    s_instance->m_rxLevel = 0;
    for (auto const [name, level] : map) {
      if (name == s_rx) s_instance->m_rxLevel = level;
      s_instance->m_pureLevel = std::max(s_instance->m_pureLevel, level);
    }
  }

  static bool enabled() {
    assertx(s_instance);
    return s_instance->m_pureLevel > 0;
  }
  static int pureEnforcementLevel() {
    assertx(s_instance);
    return s_instance->m_pureLevel;
  }
  static int rxEnforcementLevel() {
    assertx(s_instance);
    return s_instance->m_rxLevel;
  }

  static std::string mangle() {
    assertx(s_instance);
    return folly::to<std::string>(
      s_instance->s_pure, std::to_string(s_instance->m_pureLevel),
      s_instance->s_rx, std::to_string(s_instance->m_rxLevel)
    );
  }

private:
  inline static const std::string s_pure = "pure";
  inline static const std::string s_rx = "rx";

public:
  static std::unique_ptr<CoeffectsConfig> s_instance;

private:
  int m_pureLevel;
  int m_rxLevel;
};

///////////////////////////////////////////////////////////////////////////////
}

