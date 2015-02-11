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

#ifndef incl_HPHP_VM_REG_ALLOC_X64_H_
#define incl_HPHP_VM_REG_ALLOC_X64_H_

#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/native-calls.h"

#ifdef VOID
#undef VOID
#endif

namespace HPHP { namespace jit {

using NativeCalls::CallMap;

namespace x64 {

/*
 * Return true if this instruction can load a TypedValue using a 16-byte
 * load into a SIMD register.  Note that this function returns
 * false for instructions that load internal meta-data, such as Func*,
 * Class*, etc.
 */
bool loadsCell(Opcode op) {
  switch (op) {
    case LdStk:
    case LdLoc:
    case LdMem:
    case LdContField:
    case LdElem:
    case LdPackedArrayElem:
    case LdRef:
    case LdStaticLocCached:
    case LookupCns:
    case LookupClsCns:
    case CGetProp:
    case VGetProp:
    case ArrayGet:
    case MapGet:
    case CGetElem:
    case VGetElem:
    case ArrayIdx:
    case GenericIdx:
      return true;

    default:
      return false;
  }
}

/*
 * Returns true if the instruction can store source operand srcIdx to
 * memory as a cell using a 16-byte store.  (implying its okay to
 * clobber TypedValue.m_aux)
 */
bool storesCell(const IRInstruction& inst, uint32_t srcIdx) {
  // If this function returns true for an operand, then the register allocator
  // may give it an XMM register, and the instruction will store the whole 16
  // bytes into memory.  Therefore it's important *not* to return true if the
  // TypedValue.m_aux field in memory has important data.  This is the case for
  // MixedArray elements, Map elements, and RefData inner values.  We don't
  // have StMem in here since it sometimes stores to RefDatas.
  switch (inst.op()) {
    case StRetVal:
    case StLoc:
    case StLocNT:
      return srcIdx == 1;

    case StElem:
      return srcIdx == 2;

    case StStk:
      return srcIdx == 1;

    case CallBuiltin:
      return srcIdx < inst.numSrcs();

    default:
      return false;
  }
}

}}}

#endif
