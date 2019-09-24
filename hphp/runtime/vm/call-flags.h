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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * C++ representation of various flags passed from the caller to the callee's
 * prologue used to complete a function call.
 *
 * Bits 0-63: currently unused
 */
struct CallFlags {
  CallFlags() {
    m_bits = 0;
  }

  int64_t value() const { return static_cast<int64_t>(m_bits); }

private:
  uint64_t m_bits;
};

///////////////////////////////////////////////////////////////////////////////

}
