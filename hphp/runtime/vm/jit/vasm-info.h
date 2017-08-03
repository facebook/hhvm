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

#ifndef incl_HPHP_JIT_VASM_INFO_H_
#define incl_HPHP_JIT_VASM_INFO_H_

#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct Vreg;
struct Vinstr;

///////////////////////////////////////////////////////////////////////////////

/*
 * Return the sf reg read by inst, or an invalid register if the
 * instruction doesn't read an sf.
 */
Vreg getSFUseReg(const Vinstr& inst);

/*
 * Get a reference to the condition code for inst, which must be known
 * to have one (eg by calling getSFUseReg and getting a valid register).
 */
ConditionCode& getConditionCode(Vinstr& inst);

/*
 * Returns true if the instruction reads or writes memory.
 */
bool touchesMemory(Vinstr::Opcode op);

/*
 * Returns true if the instruction writes memory.
 */
bool writesMemory(Vinstr::Opcode op);
///////////////////////////////////////////////////////////////////////////////

}}

#endif
