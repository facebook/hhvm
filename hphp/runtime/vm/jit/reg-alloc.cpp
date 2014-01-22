/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/reg-alloc.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/trace-builder.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include <boost/noncopyable.hpp>

namespace HPHP {
namespace JIT{

using namespace JIT::reg;

static_assert(kReservedRSPSpillSpace ==
              NumPreAllocatedSpillLocs * sizeof(void*),
              "kReservedRSPSpillSpace changes require updates in "
              "LinearScan");

PhysReg forceAlloc(SSATmp& dst) {
  auto inst = dst.inst();
  auto opc = inst->op();

  // Note that the point of StashGeneratorSP is to save a StkPtr
  // somewhere other than rVmSp.  (TODO(#2288359): make rbx not
  // special.)
  bool abnormalStkPtr = opc == StashGeneratorSP;

  if (!abnormalStkPtr && dst.isA(Type::StkPtr)) {
    assert(opc == DefSP ||
           opc == ReDefSP ||
           opc == ReDefGeneratorSP ||
           opc == PassSP ||
           opc == DefInlineSP ||
           opc == Call ||
           opc == CallArray ||
           opc == SpillStack ||
           opc == SpillFrame ||
           opc == CufIterSpillFrame ||
           opc == ExceptionBarrier ||
           opc == RetAdjustStack ||
           opc == InterpOne ||
           opc == InterpOneCF ||
           opc == GenericRetDecRefs ||
           opc == CheckStk ||
           opc == GuardStk ||
           opc == AssertStk ||
           opc == CastStk ||
           opc == CoerceStk ||
           opc == SideExitGuardStk  ||
           MInstrEffects::supported(opc));
    return arch() == Arch::X64 ? X64::rVmSp : ARM::rVmSp;
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  bool abnormalFramePtr = opc == LdContActRec || opc == LdAFWHActRec;

  if (!abnormalFramePtr && dst.isA(Type::FramePtr)) {
    return arch() == Arch::X64 ? X64::rVmFp : ARM::rVmFp;
  }

  if (opc == DefMIStateBase) {
    assert(dst.isA(Type::PtrToCell));
    return arch() == Arch::X64 ? PhysReg(reg::rsp) : PhysReg(vixl::sp);
  }
  return InvalidReg;
}

}} // HPHP::JIT
