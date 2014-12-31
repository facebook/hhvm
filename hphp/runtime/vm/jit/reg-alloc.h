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

#ifndef incl_HPHP_VM_REG_ALLOC_H_
#define incl_HPHP_VM_REG_ALLOC_H_

#include <vector>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

#ifdef VOID
#undef VOID
#endif

namespace HPHP { namespace jit {

class IRUnit;
struct Vunit;
struct CodegenState;
class BackEnd;

// Native stack layout:
// |               |
// +---------------+
// |               |
// | MInstr state  |
// |               |
// +---------------+
// |               |  <-- spill[kReservedRSPSpillSpace - 1]
// |  spill slots  |  <-- spill[..]
// |               |  <-- spill[1]
// |               |  <-- spill[0]
// +---------------+
// |  return addr  |
// +---------------+
//
// We need to increase spill indexes by 1 to avoid overwriting the
// return address.

/*
 * Return the byte offset to a spill slot
 */
inline uint32_t slotOffset(uint32_t slot) {
  return (slot + 1) * sizeof(uint64_t);
}

/*
 * return true if the offset of this spill slot is 16-byte aligned
 */
inline bool isSlotAligned(uint32_t slot) {
  return slot % 2 == 1;
}

// This value must be consistent with the number of pre-allocated
// bytes for spill locations in __enterTCHelper in mc-generator.cpp.
// Be careful when changing this value.
const size_t NumPreAllocatedSpillLocs = kReservedRSPSpillSpace /
                                        sizeof(uint64_t);

// Return InvalidReg, or a specific register to force tmp to use
PhysReg forceAlloc(const SSATmp& tmp);

// Assign virtual registers to all SSATmps used or defined in reachable blocks.
void assignRegs(IRUnit& unit, Vunit& vunit, CodegenState& state,
                const BlockList& blocks, BackEnd*);

}}

#endif
