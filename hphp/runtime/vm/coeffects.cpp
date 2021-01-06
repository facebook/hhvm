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

#include "hphp/runtime/vm/coeffects.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticCoeffects StaticCoeffects::fromName(const std::string& a) {
  using SC = StaticCoeffects;
  if (a == "rx_local")   return SC{Level::Local};
  if (a == "rx_shallow") return SC{Level::Shallow};
  if (a == "rx")         return SC{Level::Rx};
  if (a == "pure")       return SC{Level::Pure};
  return StaticCoeffects::none();
}

const char* StaticCoeffects::toString() const {
  switch (m_data) {
    case Level::None:    return nullptr;
    case Level::Local:   return "rx_local";
    case Level::Shallow: return "rx_shallow";
    case Level::Rx:      return "rx";
    case Level::Pure:    return "pure";
  }
  not_reached();
}

const char* StaticCoeffects::toUserDisplayString() const {
  switch (m_data) {
    case Level::None:    return "non-reactive";
    case Level::Local:   return "local reactive";
    case Level::Shallow: return "shallow reactive";
    case Level::Rx:      return "reactive";
    case Level::Pure:    return "pure";
  }
  not_reached();
}

RuntimeCoeffects StaticCoeffects::toAmbient() const {
  using RC = RuntimeCoeffects;
  switch (m_data) {
    case Level::None:
    case Level::Local:   return RC{RC::Level::Default};
    case Level::Shallow: return RC{RC::Level::RxShallow};
    case Level::Rx:      return RC{RC::Level::Rx};
    case Level::Pure:    return RC{RC::Level::Pure};
  }
  not_reached();
}

RuntimeCoeffects StaticCoeffects::toRequired() const {
  using RC = RuntimeCoeffects;
  switch (m_data) {
    case Level::None:    return RC{RC::Level::Default};
    case Level::Local:
    case Level::Shallow: return RC{RC::Level::RxShallow};
    case Level::Rx:      return RC{RC::Level::Rx};
    case Level::Pure:    return RC{RC::Level::Pure};
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
