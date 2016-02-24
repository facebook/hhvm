/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef HPHP_UTIL_DWARF_REG_H_
#define HPHP_UTIL_DWARF_REG_H_

#include <cstdint>

namespace HPHP { namespace dw_reg {

///////////////////////////////////////////////////////////////////////////////

enum class x64 : uint8_t {
  RAX, RDX, RCX, RBX, RSI, RDI, RBP, RSP,
  R8,  R9,  R10, R11, R12, R13, R14, R15, RIP
};

constexpr auto FP = static_cast<uint8_t>(
#if defined(__x86_64__)
  x64::RBP
#else
  0
#endif
);

constexpr auto SP = static_cast<uint8_t>(
#if defined(__x86_64__)
  x64::RSP
#else
  0
#endif
);

constexpr auto IP = static_cast<uint8_t>(
#if defined(__x86_64__)
  x64::RIP
#else
  0
#endif
);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
