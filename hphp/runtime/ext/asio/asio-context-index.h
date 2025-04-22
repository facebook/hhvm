/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <compare>
#include <cstdint>
#include <limits>

#include "hphp/util/assertions.h"

namespace HPHP {

struct ContextStateIndex;

/**
 * Each asio session has an `AsioContext` stack, and each context has a regular
 * and low priority `AsioContextState`. The `ContextIndex` struct is used
 * to index into the context, and the `ContextStateIndex` struct is used to
 * index into the context states.
 */

struct ContextIndex {
  uint8_t value;
  constexpr auto operator<=>(const ContextIndex& other) const = default;

  /**
   * Get the regular priority context index state for the given context index.
   */
  constexpr ContextStateIndex toRegular() const;

  /**
   * Get the low priority context index state for the given context index.
   */
  constexpr ContextStateIndex toLow() const;

  /**
   * Get the parent context index.
   */
  constexpr ContextIndex parent() const {
    assertx(value > 0);
    return ContextIndex(value - 1);
  }

  /**
   * Maximum value supported.
   */
  static constexpr ContextIndex max() {
    return {std::numeric_limits<uint8_t>::max() >> 1};
  }
};
static_assert(sizeof(ContextIndex) == 1, "ContextIndex must be 1 byte");

struct ContextStateIndex {
  uint8_t value;
  constexpr auto operator<=>(const ContextStateIndex& other) const = default;

  /**
   * Get the low priority context index. This is equivalent to clearing the low
   * bit.
   */
  constexpr ContextStateIndex toLow() const {
    return ContextStateIndex(value & ~1);
  }

  /**
   * Get the regular context index. This is equivalent to setting the low bit.
   */
  constexpr ContextStateIndex toRegular() const {
    return ContextStateIndex(value | 1);
  }

  /**
   * Check if this is a regular pri context index.
   */
  constexpr bool isRegular() const {
    return value & 1;
  }

  /**
   * Get the wrapper index. This is the index of the containing context,
   * dropping the priority bit (i.e. shr 1).
   */
  ContextIndex contextIndex() const {
    return ContextIndex(value >> 1);
  }

  /**
   * Maximum value supported.
   */
  static constexpr ContextStateIndex max() {
    return {std::numeric_limits<uint8_t>::max()};
  }
};
static_assert(sizeof(ContextStateIndex) == 1, "ContextStateIndex must be 1 byte");

constexpr ContextStateIndex ContextIndex::toLow() const {
  return ContextStateIndex((value << 1) & ~1);
}

constexpr ContextStateIndex ContextIndex::toRegular() const {
  return ContextStateIndex((value << 1) | 1);
}

} // namespace HPHP
