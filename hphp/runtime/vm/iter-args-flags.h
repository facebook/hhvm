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

#include <stdint.h>

namespace HPHP {

enum class IterArgsFlags : uint8_t {
  None      = 0,
  // The base is stored in a local, and that local is unmodified in the loop.
  BaseConst = (1 << 0),
  // Indicates whether IterGetKeys migh be used during iteration. Forces
  // iterator to be index-based rather than elemptr-based for vectors.
  WithKeys  = (1 << 1),
};

constexpr IterArgsFlags operator&(IterArgsFlags a, IterArgsFlags b) {
  return IterArgsFlags(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr IterArgsFlags operator|(IterArgsFlags a, IterArgsFlags b) {
  return IterArgsFlags(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr IterArgsFlags operator~(IterArgsFlags a) {
  return IterArgsFlags(~static_cast<uint8_t>(a));
}

constexpr bool has_flag(IterArgsFlags flags, IterArgsFlags flag) {
  return (flags & flag) != IterArgsFlags::None;
}

}
