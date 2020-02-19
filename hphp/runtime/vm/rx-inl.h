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

inline Attr rxAttrsFromAttrString(const std::string& a) {
  if (a == "conditional_rx_local")   return rxMakeAttr(RxLevel::Local, true);
  if (a == "conditional_rx_shallow") return rxMakeAttr(RxLevel::Shallow, true);
  if (a == "conditional_rx")         return rxMakeAttr(RxLevel::Rx, true);
  if (a == "conditional_pure")       return rxMakeAttr(RxLevel::Pure, true);
  if (a == "rx_local")               return rxMakeAttr(RxLevel::Local, false);
  if (a == "rx_shallow")             return rxMakeAttr(RxLevel::Shallow, false);
  if (a == "rx")                     return rxMakeAttr(RxLevel::Rx, false);
  if (a == "pure")                   return rxMakeAttr(RxLevel::Pure, false);
  return static_cast<Attr>(0);
}

inline const char* rxAttrsToAttrString(Attr attrs) {
  auto const c = rxConditionalFromAttr(attrs);
  switch (rxLevelFromAttr(attrs)) {
    case RxLevel::None:    return nullptr;
    case RxLevel::Local:   return c ? "conditional_rx_local" : "rx_local";
    case RxLevel::Shallow: return c ? "conditional_rx_shallow" : "rx_shallow";
    case RxLevel::Rx:      return c ? "conditional_rx" : "rx";
    case RxLevel::Pure:    return c ? "conditional_pure" : "pure";
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

inline bool rxEnforceCallsInLevel(RxLevel level) {
  return level >= RxLevel::Shallow;
}

inline RxLevel rxRequiredCalleeLevel(RxLevel level) {
  assertx(rxEnforceCallsInLevel(level));
  switch (level) {
    case RxLevel::None:
    case RxLevel::Local:   not_reached();
    case RxLevel::Shallow: return RxLevel::Local;
    case RxLevel::Rx:      return RxLevel::Rx;
    case RxLevel::Pure:    return RxLevel::Pure;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
