/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_JIT_JUMP_SMASH_H
#define incl_HPHP_JIT_JUMP_SMASH_H

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT {

namespace X64 {
constexpr int kJmpLen = 5;
constexpr int kCallLen = 5;
constexpr int kJmpccLen = 6;
constexpr int kJmpImmBytes = 4;
}

/*
 * This module supports both X64 and ARM behind a platform-agnostic interface.
 *
 * On X64, concurrent modification and execution of instructions is safe if all
 * of the following hold:
 *
 * 1) The modification is done with a single processor store
 *
 * 2) Only one instruction in the original stream is modified
 *
 * 3) The modified instruction does not cross a cacheline boundary
 */

enum class TestAndSmashFlags {
  kAlignJccImmediate,
  kAlignJcc,
  kAlignJccAndJmp
};

/*
 * Call before emitting a test-jcc sequence. Inserts a nop gap such that after
 * writing a testBytes-long instruction, the frontier will be smashable.
 */
void prepareForTestAndSmash(CodeBlock&, int testBytes, TestAndSmashFlags flags);
void prepareForSmash(CodeBlock&, int nBytes, int offset = 0);

/*
 * Returns true if the given current frontier can have an nBytes-long
 * instruction written that will be smashable later.
 */
bool isSmashable(TCA frontier, int nBytes, int offset = 0);

void smashJmp(TCA jmpAddr, TCA newDest);
void smashCall(TCA callAddr, TCA newDest);
void smashJcc(TCA jccAddr, TCA newDest);

/*
 * Emits a jump or call that satisfies the smash* routines above.
 */
void emitSmashableJump(CodeBlock&, TCA dest, ConditionCode cc);
void emitSmashableCall(CodeBlock&, TCA dest);

/*
 * Decodes jump instructions and returns their target. This includes handling
 * for ARM's multi-instruction "smashable jump" sequences. If the code does not
 * encode the right kind of jump, these functions return nullptr.
 */
TCA jmpTarget(TCA addr);
TCA jccTarget(TCA addr);
TCA callTarget(TCA addr);

}}

#endif
