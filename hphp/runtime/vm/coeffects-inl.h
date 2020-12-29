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

#ifndef incl_HPHP_VM_RX_INL_H_
#error "rx-inl.h should only be included by rx.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline StaticCoeffects coeffectFromName(const std::string& a) {
  if (a == "rx_local")               return rxMakeAttr(RxLevel::Local);
  if (a == "rx_shallow")             return rxMakeAttr(RxLevel::Shallow);
  if (a == "rx")                     return rxMakeAttr(RxLevel::Rx);
  if (a == "pure")                   return rxMakeAttr(RxLevel::Pure);
  return static_cast<StaticCoeffects>(0);
}

inline const char* coeffectToString(StaticCoeffects coeffects) {
  switch (rxLevelFromAttr(coeffects)) {
    case RxLevel::None:    return nullptr;
    case RxLevel::Local:   return "rx_local";
    case RxLevel::Shallow: return "rx_shallow";
    case RxLevel::Rx:      return "rx";
    case RxLevel::Pure:    return "pure";
  }
  not_reached();
}

inline const char* rxLevelToString(RxLevel level) {
  switch (level) {
    case RxLevel::None:    return "non-reactive";
    case RxLevel::Local:   return "local reactive";
    case RxLevel::Shallow: return "shallow reactive";
    case RxLevel::Rx:      return "reactive";
    case RxLevel::Pure:    return "pure";
  }
  not_reached();
}

inline RxLevel rxRequiredCalleeLevel(RxLevel level) {
  assertx(coeffectsCallEnforcementLevel());
  switch (level) {
    case RxLevel::None:
    case RxLevel::Local:   return RxLevel::None;
    case RxLevel::Shallow: return RxLevel::Local;
    case RxLevel::Rx:      return RxLevel::Rx;
    case RxLevel::Pure:    return RxLevel::Pure;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
