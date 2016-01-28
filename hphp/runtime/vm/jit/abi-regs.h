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

#ifndef incl_HPHP_JIT_ABI_REGS_H
#define incl_HPHP_JIT_ABI_REGS_H

#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Machine-specific register conventions.
 */
struct Abi {
  RegSet gpUnreserved;   // unreserved general purpose 64-bit registers
  RegSet gpReserved;     // reserved general purpose 64-bit registers
  RegSet simdUnreserved; // unreserved floating point / simd 128-bit registers
  RegSet simdReserved;   // reserved floating point / simd 128-bit registers
  RegSet calleeSaved;    // callee-saved (gp and simd)
  RegSet sf;             // status flags
  bool   canSpill;       // are we allowed to spill values to the stack?

  // convenience methods
  RegSet unreserved() const { return gpUnreserved | simdUnreserved | sf; }
  RegSet gp() const { return gpUnreserved | gpReserved; }
  RegSet simd() const { return simdUnreserved | simdReserved; }
  RegSet all() const { return gp() | simd() | sf; }
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
