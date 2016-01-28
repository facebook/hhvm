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

#ifndef incl_HPHP_JIT_PHYS_REG_SAVER_H_
#define incl_HPHP_JIT_PHYS_REG_SAVER_H_

#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct Vout;

///////////////////////////////////////////////////////////////////////////////

/*
 * RAII wrapper for pushing and popping registers around a call in vasm.
 *
 * This is useful if we are emitting code with an ABI that bans vasm-xls from
 * spilling to the stack (e.g., cross-trace code), or if we need to be able to
 * introspect about how many spills we made (which vasm-xls hides).
 *
 * PhysRegSaver also maintains stack alignment; see unique-stubs.h for some
 * documentation around this behavior.
 */
struct PhysRegSaver {
  PhysRegSaver(Vout& v, RegSet regs);
  ~PhysRegSaver();

  PhysRegSaver(const PhysRegSaver&) = delete;
  PhysRegSaver(PhysRegSaver&&) noexcept = default;
  PhysRegSaver& operator=(const PhysRegSaver&) = delete;
  PhysRegSaver& operator=(PhysRegSaver&&) = default;

  size_t rspAdjustment() const;
  size_t dwordsPushed() const;

private:
  Vout& m_v;
  RegSet m_regs;
  int m_adjust;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
