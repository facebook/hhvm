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
  if (a == "rx_local")   return {RxLevel::Local};
  if (a == "rx_shallow") return {RxLevel::Shallow};
  if (a == "rx")         return {RxLevel::Rx};
  if (a == "pure")       return {RxLevel::Pure};
  return StaticCoeffects::none();
}

const char* StaticCoeffects::toString() const {
  switch (m_data) {
    case RxLevel::None:    return nullptr;
    case RxLevel::Local:   return "rx_local";
    case RxLevel::Shallow: return "rx_shallow";
    case RxLevel::Rx:      return "rx";
    case RxLevel::Pure:    return "pure";
  }
  not_reached();
}

const char* StaticCoeffects::toUserDisplayString() const {
  switch (m_data) {
    case RxLevel::None:    return "non-reactive";
    case RxLevel::Local:   return "local reactive";
    case RxLevel::Shallow: return "shallow reactive";
    case RxLevel::Rx:      return "reactive";
    case RxLevel::Pure:    return "pure";
  }
  not_reached();
}

RuntimeCoeffects StaticCoeffects::toAmbient() const {
  switch (m_data) {
    case RxLevel::None:
    case RxLevel::Local:   return RCDefault;
    case RxLevel::Shallow: return RCRxShallow;
    case RxLevel::Rx:      return RCRx;
    case RxLevel::Pure:    return RCPure;
  }
  not_reached();
}

RuntimeCoeffects StaticCoeffects::toRequired() const {
  switch (m_data) {
    case RxLevel::None:    return RCDefault;
    case RxLevel::Local:
    case RxLevel::Shallow: return RCRxShallow;
    case RxLevel::Rx:      return RCRx;
    case RxLevel::Pure:    return RCPure;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
