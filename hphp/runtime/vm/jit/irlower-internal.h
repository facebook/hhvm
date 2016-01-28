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

#ifndef incl_HPHP_JIT_IRLOWER_INTERNAL_H_
#define incl_HPHP_JIT_IRLOWER_INTERNAL_H_

#include "hphp/runtime/vm/jit/ir-opcode.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct ArgGroup;
struct Block;
struct IRInstruction;
struct Vlabel;
struct Vloc;
struct Vout;
struct Vreg;

namespace irlower { struct IRLS; }

///////////////////////////////////////////////////////////////////////////////

namespace irlower {

///////////////////////////////////////////////////////////////////////////////

/*
 * The current vasm instruction stream cursors.
 */
Vout& vmain(IRLS& env);
Vout& vcold(IRLS& env);

/*
 * Get the Vlabel for IR block `b'.
 */
Vlabel label(IRLS& env, Block* b);

/*
 * Get the SSATmp location descriptor for the i-th src or dst of `inst'.
 */
Vloc srcLoc(IRLS& env, const IRInstruction* inst, unsigned i);
Vloc dstLoc(IRLS& env, const IRInstruction* inst, unsigned i);

/*
 * Empty ArgGroup for `inst'.
 */
ArgGroup argGroup(IRLS& env, const IRInstruction* inst);

///////////////////////////////////////////////////////////////////////////////

/*
 * Emit a conditional jmp to `target', which falls through to a new block if
 * the branch is not taken.
 */
void fwdJcc(Vout& v, IRLS& env, ConditionCode cc, Vreg sf, Block* target);

///////////////////////////////////////////////////////////////////////////////

#define O(name, ...)  \
  void cg##name(IRLS& env, const IRInstruction* inst);
IR_OPCODES
#undef O

///////////////////////////////////////////////////////////////////////////////

}}}

#include "irlower-internal-inl.h"

#endif
