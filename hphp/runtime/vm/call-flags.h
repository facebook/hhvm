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

#include <cstdint>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * C++ representation of various flags passed from the caller to the callee's
 * prologue used to complete a function call.
 *
 * Bit 0: flag indicating whether generics are on the stack
 * Bit 1: flag indicating whether this is a dynamic call
 * Bit 2: always set to 0
 * Bit 3: flag indicating whether async eager return was requested
 * Bits 4-31: call offset (from the beginning of the function's entry point)
 * Bits 32-47: generics bitmap
 * Bits 48-63: coeffects
 */
struct CallFlags {
  enum Flags {
    HasGenerics,
    IsDynamicCall,
    ReservedZero0,
    ReservedZero1,
    AsyncEagerReturn,
    CallOffsetStart,
    GenericsBitmapStart = 32,
    CoeffectsStart = 48,
  };

  CallFlags(bool hasGenerics, bool isDynamicCall, bool asyncEagerReturn,
            Offset callOffset, uint16_t genericsBitmap,
            RuntimeCoeffects coeffects) {
    auto const callOffsetBits = (uint64_t)callOffset << Flags::CallOffsetStart;
    assertx((callOffsetBits >> Flags::GenericsBitmapStart) == 0);
    assertx(hasGenerics || genericsBitmap == 0);
    m_bits =
      ((hasGenerics ? 1 : 0) << Flags::HasGenerics) |
      ((isDynamicCall ? 1 : 0) << Flags::IsDynamicCall) |
      ((asyncEagerReturn ? 1 : 0) << Flags::AsyncEagerReturn) |
      callOffsetBits |
      ((uint64_t)genericsBitmap << Flags::GenericsBitmapStart) |
      ((uint64_t)coeffects.value() << Flags::CoeffectsStart);
  }

  bool hasGenerics() const { return m_bits & (1 << Flags::HasGenerics); }
  bool isDynamicCall() const { return m_bits & (1 << Flags::IsDynamicCall); }
  bool asyncEagerReturn() const {
    return m_bits & (1 << Flags::AsyncEagerReturn);
  }
  Offset callOffset() const {
    return ((uint32_t)m_bits) >> Flags::CallOffsetStart;
  }
  int64_t value() const { return static_cast<int64_t>(m_bits); }

  RuntimeCoeffects coeffects() const {
    return RuntimeCoeffects::fromValue((uint16_t)(m_bits >> CoeffectsStart));
  }

private:
  uint64_t m_bits;
};

///////////////////////////////////////////////////////////////////////////////

}
