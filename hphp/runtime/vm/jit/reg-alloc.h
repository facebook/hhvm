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

#ifndef incl_HPHP_VM_REG_ALLOC_H_
#define incl_HPHP_VM_REG_ALLOC_H_

#include <vector>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct Abi;
struct IRUnit;
struct Vinstr;
struct Vunit;

namespace irlower { struct IRLS; }

///////////////////////////////////////////////////////////////////////////////

// Native stack layout:
// | enterTCHelper |
// +---------------+
// |  return addr  |
// |  saved %rbp   |
// +---------------+
// |               | ...up to kMaxSpillSlots
// |  spill slots  | <-- spill[1]
// |               | <-- spill[0]
// +---------------+
//

/*
 * Return the byte offset to a spill slot.
 */
inline uint32_t slotOffset(uint32_t slot) {
  return slot * sizeof(uint64_t);
}

/*
 * Return true if the offset of this spill slot is 16-byte aligned.
 */
inline bool isSlotAligned(uint32_t slot) {
  return slot % 2 == 0;
}

const size_t kMaxSpillSlots = 128;

/*
 * Return InvalidReg, or a specific register to force `tmp' to use.
 */
PhysReg forceAlloc(const SSATmp& tmp);

/*
 * Assign virtual registers to all SSATmps used or defined in reachable blocks.
 */
void assignRegs(IRUnit& unit, Vunit& vunit, irlower::IRLS& state,
                const BlockList& blocks);

/*
 * Return the set of physical registers implicitly accessed (used or defined).
 */
void getEffects(const Abi& abi, const Vinstr& i,
                RegSet& uses, RegSet& across, RegSet& defs);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
