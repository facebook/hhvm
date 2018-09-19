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

inline RxLevel rxLevelFromAttrString(const std::string& a) {
  if (a == "conditional_rx_local")   return RxLevel::ConditionalLocal;
  if (a == "conditional_rx_shallow") return RxLevel::ConditionalShallow;
  if (a == "conditional_rx")         return RxLevel::ConditionalRx;
  if (a == "rx_local")               return RxLevel::Local;
  if (a == "rx_shallow")             return RxLevel::Shallow;
  if (a == "rx")                     return RxLevel::Rx;
  return RxLevel::None;
}

inline const char* rxLevelToAttrString(RxLevel r) {
  switch (r) {
    case RxLevel::None:               return nullptr;
    case RxLevel::ConditionalLocal:   return "conditional_rx_local";
    case RxLevel::ConditionalShallow: return "conditional_rx_shallow";
    case RxLevel::ConditionalRx:      return "conditional_rx";
    case RxLevel::Local:              return "rx_local";
    case RxLevel::Shallow:            return "rx_shallow";
    case RxLevel::Rx:                 return "rx";
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
