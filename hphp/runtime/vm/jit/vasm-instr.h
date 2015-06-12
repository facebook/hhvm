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

#ifndef incl_HPHP_JIT_VASM_INSTR_H_
#define incl_HPHP_JIT_VASM_INSTR_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/vixl/a64/constants-a64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct IRInstruction;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Field actions:
 *
 *    I(f)     immediate
 *    Inone    no immediates
 *    U(s)     use s
 *    UA(s)    use s, but s lifetime extends across the instruction
 *    UH(s,h)  use s, try assigning same register as h
 *    D(d)     define d
 *    DH(d,h)  define d, try assigning same register as h
 *    Un,Dn    no uses, defs
 */
#define VASM_OPCODES\
  /* service requests, PHP-level function calls */\
  O(bindaddr, I(dest) I(sk) I(spOff), Un, Dn)\
  O(bindcall, I(stub), U(args), Dn)\
  O(bindjcc1st, I(cc) I(targets[0]) I(targets[1]) I(spOff), U(sf) U(args), Dn)\
  O(bindjcc, I(cc) I(target) I(spOff) I(trflags), U(sf) U(args), Dn)\
  O(bindjmp, I(target) I(spOff) I(trflags), U(args), Dn)\
  O(callstub, I(target), U(args), Dn)\
  O(contenter, Inone, U(fp) U(target) U(args), Dn)\
  O(fallback, I(dest), U(args), Dn)\
  O(fallbackcc, I(cc) I(dest), U(sf) U(args), Dn)\
  O(svcreq, I(req) I(stub_block), U(args) U(extraArgs), Dn)\
  /* vasm intrinsics */\
  O(callfaststub, I(fix), U(args), Dn)\
  O(copy, Inone, UH(s,d), DH(d,s))\
  O(copy2, Inone, UH(s0,d0) UH(s1,d1), DH(d0,s0) DH(d1,s1))\
  O(copyargs, Inone, UH(s,d), DH(d,s))\
  O(debugtrap, Inone, Un, Dn)\
  O(fallthru, Inone, Un, Dn)\
  O(ldimmb, I(s), Un, D(d))\
  O(ldimml, I(s), Un, D(d))\
  O(ldimmq, I(s), Un, D(d))\
  O(ldimmqs, I(s), Un, D(d))\
  O(load, Inone, U(s), D(d))\
  O(mccall, I(target), U(args), Dn)\
  O(mcprep, Inone, Un, D(d))\
  O(nothrow, Inone, Un, Dn)\
  O(phidef, Inone, Un, D(defs))\
  O(phijmp, Inone, U(uses), Dn)\
  O(phijcc, I(cc), U(uses) U(sf), Dn)\
  O(store, Inone, U(s) U(d), Dn)\
  O(syncpoint, I(fix), Un, Dn)\
  O(unwind, Inone, Un, Dn)\
  O(vcall, I(call) I(destType) I(fixup), U(args), D(d))\
  O(vinvoke, I(call) I(destType) I(fixup), U(args), D(d))\
  O(vcallstub, I(target), U(args) U(extraArgs), Dn)\
  O(landingpad, I(fromPHPCall), Un, Dn)\
  O(countbytecode, Inone, U(base), D(sf))\
  O(defvmsp, Inone, Un, D(d))\
  O(syncvmsp, Inone, U(s), Dn)\
  O(srem, Inone, U(s0) U(s1), D(d))\
  O(sar, Inone, U(s0) U(s1), D(d) D(sf))\
  O(shl, Inone, U(s0) U(s1), D(d) D(sf))\
  O(vretm, Inone, U(retAddr) U(prevFp) U(args), D(d))\
  O(vret, Inone, U(retAddr) U(args), Dn)\
  O(leavetc, Inone, U(args), Dn)\
  O(absdbl, Inone, U(s), D(d))\
  /* arm instructions */\
  O(asrv, Inone, U(sl) U(sr), D(d))\
  O(brk, I(code), Un, Dn)\
  O(cbcc, I(cc), U(s), Dn)\
  O(hcsync, I(fix) I(call), Un, Dn)\
  O(hcnocatch, I(call), Un, Dn)\
  O(hcunwind, I(call), Un, Dn)\
  O(hostcall, I(argc) I(syncpoint), U(args), Dn)\
  O(lslv, Inone, U(sl) U(sr), D(d))\
  O(tbcc, I(cc) I(bit), U(s), Dn)\
  /* x64 instructions */\
  O(addli, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(addlm, Inone, U(s0) U(m), D(sf)) \
  O(addq, Inone, U(s0) U(s1), D(d) D(sf)) \
  O(addqi, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(addqim, I(s0), U(m), D(sf)) \
  O(addsd, Inone, U(s0) U(s1), D(d))\
  O(andb, Inone, U(s0) U(s1), D(d) D(sf)) \
  O(andbi, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(andbim, I(s), U(m), D(sf)) \
  O(andl, Inone, U(s0) U(s1), D(d) D(sf)) \
  O(andli, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(andq, Inone, U(s0) U(s1), D(d) D(sf)) \
  O(andqi, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(call, I(target), U(args), Dn)\
  O(callm, Inone, U(target) U(args), Dn)\
  O(callr, Inone, U(target) U(args), Dn)\
  O(cloadq, I(cc), U(sf) U(f) U(t), D(d))\
  O(cmovq, I(cc), U(sf) U(f) U(t), D(d))\
  O(cmpb, Inone, U(s0) U(s1), D(sf))\
  O(cmpbi, I(s0), U(s1), D(sf))\
  O(cmpbim, I(s0), U(s1), D(sf))\
  O(cmpwim, I(s0), U(s1), D(sf))\
  O(cmpl, Inone, U(s0) U(s1), D(sf))\
  O(cmpli, I(s0), U(s1), D(sf))\
  O(cmplim, I(s0), U(s1), D(sf))\
  O(cmplm, Inone, U(s0) U(s1), D(sf))\
  O(cmpq, Inone, U(s0) U(s1), D(sf))\
  O(cmpqi, I(s0), U(s1), D(sf))\
  O(cmpqim, I(s0), U(s1), D(sf))\
  O(cmpqims, I(s0), U(s1), D(sf))\
  O(cmpqm, Inone, U(s0) U(s1), D(sf))\
  O(cmpsd, I(pred), UA(s0) U(s1), D(d))\
  O(cqo, Inone, Un, Dn)\
  O(cvttsd2siq, Inone, U(s), D(d))\
  O(cvtsi2sd, Inone, U(s), D(d))\
  O(cvtsi2sdm, Inone, U(s), D(d))\
  O(decl, Inone, UH(s,d), DH(d,s) D(sf))\
  O(declm, Inone, U(m), D(sf))\
  O(decq, Inone, UH(s,d), DH(d,s) D(sf))\
  O(decqm, Inone, U(m), D(sf))\
  O(divsd, Inone, UA(s0) U(s1), D(d))\
  O(idiv, Inone, U(s), D(sf))\
  O(imul, Inone, U(s0) U(s1), D(d) D(sf))\
  O(incwm, Inone, U(m), D(sf))\
  O(incl, Inone, UH(s,d), DH(d,s) D(sf))\
  O(inclm, Inone, U(m), D(sf))\
  O(incq, Inone, UH(s,d), DH(d,s) D(sf))\
  O(incqm, Inone, U(m), D(sf))\
  O(incqmlock, Inone, U(m), D(sf))\
  O(jcc, I(cc), U(sf), Dn)\
  O(jcci, I(cc), U(sf), Dn)\
  O(jmp, Inone, Un, Dn)\
  O(jmpr, Inone, U(target) U(args), Dn)\
  O(jmpm, Inone, U(target) U(args), Dn)\
  O(jmpi, I(target), U(args), Dn)\
  O(lea, Inone, U(s), D(d))\
  O(leap, I(s), Un, D(d))\
  O(loadups, Inone, U(s), D(d))\
  O(loadtqb, Inone, U(s), D(d))\
  O(loadl, Inone, U(s), D(d))\
  O(loadqp, I(s), Un, D(d))\
  O(loadsd, Inone, U(s), D(d))\
  O(loadzbl, Inone, U(s), D(d))\
  O(loadzbq, Inone, U(s), D(d))\
  O(loadzlq, Inone, U(s), D(d))\
  O(movb, Inone, UH(s,d), DH(d,s))\
  O(movl, Inone, UH(s,d), DH(d,s))\
  O(movzbl, Inone, UH(s,d), DH(d,s))\
  O(movzbq, Inone, UH(s,d), DH(d,s))\
  O(movtqb, Inone, UH(s,d), DH(d,s))\
  O(movtql, Inone, UH(s,d), DH(d,s))\
  O(mulsd, Inone, U(s0) U(s1), D(d))\
  O(mul, Inone, U(s0) U(s1), D(d))\
  O(neg, Inone, UH(s,d), DH(d,s) D(sf))\
  O(nop, Inone, Un, Dn)\
  O(not, Inone, UH(s,d), DH(d,s))\
  O(notb, Inone, UH(s,d), DH(d,s))\
  O(orwim, I(s0), U(m), D(sf))\
  O(orq, Inone, U(s0) U(s1), D(d) D(sf))\
  O(orqi, I(s0), UH(s1,d), DH(d,s1) D(sf)) \
  O(orqim, I(s0), U(m), D(sf))\
  O(pop, Inone, Un, D(d))\
  O(popm, Inone, U(d), Dn)\
  O(psllq, I(s0), UH(s1,d), DH(d,s1))\
  O(psrlq, I(s0), UH(s1,d), DH(d,s1))\
  O(push, Inone, U(s), Dn)\
  O(ret, Inone, U(args), Dn)\
  O(roundsd, I(dir), U(s), D(d))\
  O(sarq, Inone, UH(s,d), DH(d,s) D(sf))\
  O(sarqi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(setcc, I(cc), U(sf), D(d))\
  O(shlli, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(shlq, Inone, UH(s,d), DH(d,s) D(sf))\
  O(shlqi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(shrli, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(shrqi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(sqrtsd, Inone, U(s), D(d))\
  O(storeb, Inone, U(s) U(m), Dn)\
  O(storebi, I(s), U(m), Dn)\
  O(storeups, Inone, U(s) U(m), Dn)\
  O(storel, Inone, U(s) U(m), Dn)\
  O(storeli, I(s), U(m), Dn)\
  O(storeqi, I(s), U(m), Dn)\
  O(storesd, Inone, U(s) U(m), Dn)\
  O(storew, Inone, U(s) U(m), Dn)\
  O(storewi, I(s), U(m), Dn)\
  O(subbi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(subl, Inone, UA(s0) U(s1), D(d) D(sf))\
  O(subli, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(subq, Inone, UA(s0) U(s1), D(d) D(sf))\
  O(subqi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(subsd, Inone, UA(s0) U(s1), D(d))\
  O(testb, Inone, U(s0) U(s1), D(sf))\
  O(testbi, I(s0), U(s1), D(sf))\
  O(testbim, I(s0), U(s1), D(sf))\
  O(testwim, I(s0), U(s1), D(sf))\
  O(testl, Inone, U(s0) U(s1), D(sf))\
  O(testli, I(s0), U(s1), D(sf))\
  O(testlim, I(s0), U(s1), D(sf))\
  O(testq, Inone, U(s0) U(s1), D(sf))\
  O(testqm, Inone, U(s0) U(s1), D(sf))\
  O(testqim, I(s0), U(s1), D(sf))\
  O(ucomisd, Inone, U(s0) U(s1), D(sf))\
  O(ud2, Inone, Un, Dn)\
  O(unpcklpd, Inone, UA(s0) U(s1), D(d))\
  O(xorb, Inone, U(s0) U(s1), D(d) D(sf))\
  O(xorbi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  O(xorl, Inone, U(s0) U(s1), D(d) D(sf))\
  O(xorq, Inone, U(s0) U(s1), D(d) D(sf))\
  O(xorqi, I(s0), UH(s1,d), DH(d,s1) D(sf))\
  /* */

///////////////////////////////////////////////////////////////////////////////
// Service requests.

/*
 * PHP function call: Smashable call with custom ABI.
 */
struct bindcall {
  explicit bindcall(TCA stub,
                    RegSet args,
                    std::array<Vlabel,2> targets)
    : stub{stub}
    , args{args}
  {
    this->targets[0] = targets[0];
    this->targets[1] = targets[1];
  }

  TCA stub;
  RegSet args;
  Vlabel targets[2];
};

/*
 * PHP function call: Non-smashable call with same ABI as bindcall.
 */
struct callstub { TCA target; RegSet args; };

struct contenter { Vreg64 fp, target; RegSet args; Vlabel targets[2]; };
struct svcreq { ServiceRequest req; RegSet args; Vtuple extraArgs;
                TCA stub_block; };

struct fallbackcc {
  explicit fallbackcc(ConditionCode cc,
                      VregSF sf,
                      SrcKey dest,
                      TransFlags trflags,
                      RegSet args)
    : cc{cc}
    , sf{sf}
    , dest{dest}
    , trflags{trflags}
    , args{args}
  {}

  ConditionCode cc;
  VregSF sf;
  SrcKey dest;
  TransFlags trflags;
  RegSet args;
};

struct fallback {
  explicit fallback(SrcKey dest,
                    TransFlags trflags,
                    RegSet args)
    : dest{dest}
    , trflags{trflags}
    , args{args}
  {}

  SrcKey dest;
  TransFlags trflags;
  RegSet args;
};

struct bindaddr {
  explicit bindaddr(TCA* dest, SrcKey sk, FPInvOffset spOff)
    : dest(dest)
    , sk(sk)
    , spOff(spOff)
  {}

  TCA* dest;
  SrcKey sk;
  FPInvOffset spOff;
};

struct bindjcc1st {
  explicit bindjcc1st(ConditionCode cc,
                      VregSF sf,
                      std::array<SrcKey,2> targets,
                      FPInvOffset spOff,
                      RegSet args)
    : cc{cc}
    , sf{sf}
    , spOff(spOff)
    , args{args}
  {
    this->targets[0] = targets[0];
    this->targets[1] = targets[1];
  }

  ConditionCode cc;
  VregSF sf;
  SrcKey targets[2];
  FPInvOffset spOff;
  RegSet args;
};

struct bindjcc {
  explicit bindjcc(ConditionCode cc,
                   VregSF sf,
                   SrcKey target,
                   FPInvOffset spOff,
                   TransFlags trflags,
                   RegSet args)
    : cc{cc}
    , sf{sf}
    , target{target}
    , spOff(spOff)
    , trflags{trflags}
    , args{args}
  {}

  ConditionCode cc;
  VregSF sf;
  SrcKey target;
  FPInvOffset spOff;
  TransFlags trflags;
  RegSet args;
};

struct bindjmp {
  explicit bindjmp(SrcKey target,
                   FPInvOffset spOff,
                   TransFlags trflags,
                   RegSet args)
    : target{target}
    , spOff(spOff)
    , trflags{trflags}
    , args{args}
  {}

  SrcKey target;
  FPInvOffset spOff;
  TransFlags trflags;
  RegSet args;
};

///////////////////////////////////////////////////////////////////////////////
// VASM intrinsics.

/*
 * Call a "fast" stub, which is a stub that preserves more registers than a
 * normal call. It may still call C++ functions on a slow path (which is why
 * there's a Fixup operand) but it will save any required registers before
 * doing so.
 */
struct callfaststub { TCA target; Fixup fix; RegSet args; };

/*
 * Copies of different arities. All copies happen in parallel, meaning operand
 * order doesn't matter when a PhysReg appears as both a src and dst.
 */
struct copy { Vreg s, d; };
struct copy2 { Vreg64 s0, s1, d0, d1; };
struct copyargs { Vtuple s, d; };

/*
 * Causes any attached debugger to trap. Process may abort if no debugger is
 * attached.
 */
struct debugtrap {};

/*
 * No-op, used for marking the end of a block that is intentionally going to
 * fall-through.  Only for use with Vauto.
 */
struct fallthru {};

/*
 * load an immedate value without mutating status flags.
 */
struct ldimmb { Immed s; Vreg d; };
struct ldimml { Immed s; Vreg d; };
struct ldimmq { Immed64 s; Vreg d; };
struct ldimmqs { Immed64 s; Vreg d; }; // smashable version of ldimmq

struct load { Vptr s; Vreg d; };
struct mccall { CodeAddress target; RegSet args; };
struct mcprep { Vreg64 d; };
struct nothrow {};
struct phidef { Vtuple defs; };
struct phijmp { Vlabel target; Vtuple uses; };
struct phijcc { ConditionCode cc; VregSF sf; Vlabel targets[2]; Vtuple uses; };
struct store { Vreg s; Vptr d; };
struct syncpoint { Fixup fix; };
struct unwind { Vlabel targets[2]; };

/*
 * Function call, without or with exception edges, respectively. Contains
 * information about a C++ helper call needed for lowering to different target
 * architectures.
 */
struct vcall { CppCall call; VcallArgsId args; Vtuple d;
               Fixup fixup; DestType destType; bool nothrow; };
struct vinvoke { CppCall call; VcallArgsId args; Vtuple d; Vlabel targets[2];
                 Fixup fixup; DestType destType; bool smashable; };

/*
 * Non-smashable PHP function call with exception edges and additional
 * integer arguments.
 */
struct vcallstub { TCA target; RegSet args; Vtuple extraArgs;
                   Vlabel targets[2]; };

struct landingpad { bool fromPHPCall; };
struct countbytecode { Vreg base; VregSF sf; };

/*
 * Copy rVmSp into d.
 *
 * Used when reentering translated code after an ABI boundary, such as the
 * beginning of a tracelet or right after a bindcall.
 */
struct defvmsp { Vreg d; };

/*
 * Copy s into rVmSp.
 *
 * Used right before leaving translated code for an ABI boundary, such as
 * bindjmp or fallbackcc.
 */
struct syncvmsp { Vreg s; };

struct srem { Vreg s0, s1, d; };
struct sar { Vreg s0, s1, d; VregSF sf; };
struct shl { Vreg s0, s1, d; VregSF sf; };
struct absdbl { Vreg s, d; };

/*
 * Pushes retAddr, loads prevFp into d, and executes a ret instruction.
 */
struct vretm { Vptr retAddr; Vptr prevFp; Vreg d; RegSet args; };

/*
 * Pushes retAddr and executes a ret instruction.
 */
struct vret { Vreg retAddr; RegSet args; };

/*
 * Execute a ret instruction directly, returning to enterTCHelper.
 */
struct leavetc { RegSet args; };

///////////////////////////////////////////////////////////////////////////////
// ARM.

/*
 * ARM-specific intrinsics.
 */
struct hcsync { Fixup fix; Vpoint call; };
struct hcnocatch { Vpoint call; };
struct hcunwind { Vpoint call; Vlabel targets[2]; };

/*
 * ARM-specific instructions.
 */
struct brk { uint16_t code; };
struct hostcall { RegSet args; uint8_t argc; Vpoint syncpoint; };
struct cbcc { vixl::Condition cc; Vreg64 s; Vlabel targets[2]; };
struct tbcc { vixl::Condition cc; unsigned bit; Vreg64 s; Vlabel targets[2]; };
struct lslv { Vreg64 sl, sr, d; };
struct asrv { Vreg64 sl, sr, d; };
struct mul { Vreg64 s0, s1, d; };

///////////////////////////////////////////////////////////////////////////////
// x64.

/*
 * ATT style operand order. for binary ops:
 *    op   s0 s1 d:  d = s1 op s0    =>   d=s1; d op= s0
 *    op   imm s1 d: d = s1 op imm   =>   d=s1; d op= imm
 *    cmp  s0 s1:    s1 cmp s0
 */

/*
 * Suffix conventions:
 *    b   8-bit
 *    w   16-bit
 *    l   32-bit
 *    q   64-bit
 *    i   immediate
 *    m   Vptr
 *    p   RIPRelativeRef
 *    s   smashable
 */

struct addli { Immed s0; Vreg32 s1, d; VregSF sf; };
struct addlm { Vreg32 s0; Vptr m; VregSF sf; };
struct addq  { Vreg64 s0, s1, d; VregSF sf; };
struct addqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct addqim { Immed s0; Vptr m; VregSF sf; };
struct addsd  { VregDbl s0, s1, d; };
struct andb  { Vreg8 s0, s1, d; VregSF sf; };
struct andbi { Immed s0; Vreg8 s1, d; VregSF sf; };
struct andbim { Immed s; Vptr m; VregSF sf; };
struct andl  { Vreg32 s0, s1, d; VregSF sf; };
struct andli { Immed s0; Vreg32 s1, d; VregSF sf; };
struct andq  { Vreg64 s0, s1, d; VregSF sf; };
struct andqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct call { CodeAddress target; RegSet args; };
struct callm { Vptr target; RegSet args; };
struct callr { Vreg64 target; RegSet args; };

/*
 * Implements the equivalent of:
 *
 *    t1 = load t
 *    d = condition ? t1 : f
 *
 * Note that t is unconditionally dereferenced.
 */
struct cloadq { ConditionCode cc; VregSF sf; Vreg64 f; Vptr t; Vreg64 d; };

/*
 * Implements d = condition ? t : f.
 */
struct cmovq { ConditionCode cc; VregSF sf; Vreg64 f, t, d; };

// compares are att-style: s1-s0 => sf
struct cmpb  { Vreg8  s0; Vreg8  s1; VregSF sf; };
struct cmpbi { Immed  s0; Vreg8  s1; VregSF sf; };
struct cmpbim { Immed s0; Vptr s1; VregSF sf; };
struct cmpwim { Immed s0; Vptr s1; VregSF sf; };
struct cmpl  { Vreg32 s0; Vreg32 s1; VregSF sf; };
struct cmpli { Immed  s0; Vreg32 s1; VregSF sf; };
struct cmplim { Immed s0; Vptr s1; VregSF sf; };
struct cmplm { Vreg32 s0; Vptr s1; VregSF sf; };
struct cmpq  { Vreg64 s0; Vreg64 s1; VregSF sf; };
struct cmpqi { Immed  s0; Vreg64 s1; VregSF sf; };
struct cmpqim { Immed s0; Vptr s1; VregSF sf; };
struct cmpqims { Immed s0; Vptr s1; VregSF sf; };
struct cmpqm { Vreg64 s0; Vptr s1; VregSF sf; };
struct cmpsd { ComparisonPred pred; VregDbl s0, s1, d; };
struct cqo {};
struct cvttsd2siq { VregDbl s; Vreg64 d; };
struct cvtsi2sd { Vreg64 s; VregDbl d; };
struct cvtsi2sdm { Vptr s; VregDbl d; };
struct decl { Vreg32 s, d; VregSF sf; };
struct declm { Vptr m; VregSF sf; };
struct decq { Vreg64 s, d; VregSF sf; };
struct decqm { Vptr m; VregSF sf; };
struct divsd { VregDbl s0, s1, d; };
struct idiv { Vreg64 s; VregSF sf; };
struct imul { Vreg64 s0, s1, d; VregSF sf; };
struct incl { Vreg32 s, d; VregSF sf; };
struct inclm { Vptr m; VregSF sf; };
struct incq { Vreg64 s, d; VregSF sf; };
struct incqm { Vptr m; VregSF sf; };
struct incqmlock { Vptr m; VregSF sf; };
struct incwm { Vptr m; VregSF sf; };
struct jcc { ConditionCode cc; VregSF sf; Vlabel targets[2]; };
struct jcci { ConditionCode cc; VregSF sf; Vlabel target; TCA taken; };
struct jmp { Vlabel target; };
struct jmpr { Vreg64 target; RegSet args; };
struct jmpm { Vptr target; RegSet args; };
struct jmpi { TCA target; RegSet args; };
struct lea { Vptr s; Vreg64 d; };
struct leap { RIPRelativeRef s; Vreg64 d; };
struct loadups { Vptr s; Vreg128 d; };
struct loadtqb { Vptr s; Vreg8 d; };
struct loadl { Vptr s; Vreg32 d; };
struct loadqp { RIPRelativeRef s; Vreg64 d; };
struct loadsd { Vptr s; VregDbl d; };
struct loadzbl { Vptr s; Vreg32 d; };
struct loadzbq { Vptr s; Vreg64 d; };
struct loadzlq { Vptr s; Vreg64 d; };
struct movb { Vreg8 s, d; };
struct movl { Vreg32 s, d; };

// Move zero-extended s to d.
struct movzbl { Vreg8 s; Vreg32 d; };
struct movzbq { Vreg8 s; Vreg64 d; };

// Move truncated s to d.
struct movtqb { Vreg64 s; Vreg8 d; };
struct movtql { Vreg64 s; Vreg32 d; };

struct mulsd  { VregDbl s0, s1, d; };
struct neg { Vreg64 s, d; VregSF sf; };
struct nop {};
struct not { Vreg64 s, d; };
struct notb { Vreg8 s, d; };
struct orwim { Immed s0; Vptr m; VregSF sf; };
struct orq { Vreg64 s0, s1, d; VregSF sf; };
struct orqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct orqim { Immed s0; Vptr m; VregSF sf; };
struct pop { Vreg64 d; };
struct popm { Vptr d; };
struct push { Vreg64 s; };
struct ret { RegSet args; };
struct roundsd { RoundDirection dir; VregDbl s, d; };
struct setcc { ConditionCode cc; VregSF sf; Vreg8 d; };
// shifts are att-style: s1<<{s0|ecx} => d,sf
struct psllq { Immed s0; VregDbl s1, d; };
struct psrlq { Immed s0; VregDbl s1, d; };
struct sarq { Vreg64 s, d; VregSF sf; }; // uses rcx
struct sarqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct shlli { Immed s0; Vreg32 s1, d; VregSF sf; };
struct shlq { Vreg64 s, d; VregSF sf; }; // uses rcx
struct shlqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct shrli { Immed s0; Vreg32 s1, d; VregSF sf; };
struct shrqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct sqrtsd { VregDbl s, d; };
struct storeb { Vreg8 s; Vptr m; };
struct storebi { Immed s; Vptr m; };
struct storeups { Vreg128 s; Vptr m; };
struct storel { Vreg32 s; Vptr m; };
struct storeli { Immed s; Vptr m; };
struct storeqi { Immed s; Vptr m; };
struct storesd { VregDbl s; Vptr m; };
struct storew { Vreg16 s; Vptr m; };
struct storewi { Immed s; Vptr m; };
// sub is att-style: s1-s0 => d,sf
struct subbi { Immed s0; Vreg8 s1, d; VregSF sf; };
struct subl { Vreg32 s0, s1, d; VregSF sf; };
struct subli { Immed s0; Vreg32 s1, d; VregSF sf; };
struct subq { Vreg64 s0, s1, d; VregSF sf; };
struct subqi { Immed s0; Vreg64 s1, d; VregSF sf; };
struct subsd { VregDbl s0, s1, d; };
struct testb { Vreg8 s0, s1; VregSF sf; };
struct testbi { Immed s0; Vreg8 s1; VregSF sf; };
struct testbim { Immed s0; Vptr s1; VregSF sf; };
struct testwim { Immed s0; Vptr s1; VregSF sf; };
struct testl { Vreg32 s0, s1; VregSF sf; };
struct testli { Immed s0; Vreg32 s1; VregSF sf; };
struct testlim { Immed s0; Vptr s1; VregSF sf; };
struct testq { Vreg64 s0, s1; VregSF sf; };
struct testqm { Vreg64 s0; Vptr s1; VregSF sf; };
struct testqim { Immed s0; Vptr s1; VregSF sf; };
// compare is att-style: s1-s0 => sf
struct ucomisd { VregDbl s0, s1; VregSF sf; };
struct ud2 {};
struct unpcklpd { VregDbl s0, s1; Vreg128 d; };
struct xorb { Vreg8 s0, s1, d; VregSF sf; };
struct xorbi { Immed s0; Vreg8 s1, d; VregSF sf; };
struct xorl { Vreg32 s0, s1, d; VregSF sf; };
struct xorq { Vreg64 s0, s1, d; VregSF sf; };
struct xorqi { Immed s0; Vreg64 s1, d; VregSF sf; };

///////////////////////////////////////////////////////////////////////////////

struct Vinstr {
#define O(name, imms, uses, defs) name,
  enum Opcode : uint8_t { VASM_OPCODES };
#undef O

  Vinstr() : op(ud2) {}

#define O(name, imms, uses, defs)                               \
  /* implicit */ Vinstr(jit::name i) : op(name), name##_(i) {}
  VASM_OPCODES
#undef O

  /*
   * Define an operator= for all instructions to preserve origin and pos.
   */
#define O(name, ...)                            \
  Vinstr& operator=(const jit::name& i) {       \
    op = Vinstr::name;                          \
    name##_ = i;                                \
    return *this;                               \
  }
  VASM_OPCODES
#undef O

  template<typename Op>
  struct matcher;

  /*
   * Templated accessors for the union members.
   */
  template<typename Op>
  typename matcher<Op>::type& get() {
    return matcher<Op>::get(*this);
  }
  template<typename Op>
  const typename matcher<Op>::type& get() const {
    return matcher<Op>::get(*this);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  Opcode op;

  /*
   * Instruction position, currently used only in vasm-xls.
   */
  unsigned pos;

  /*
   * If present, the IRInstruction this Vinstr was originally created from.
   */
  const IRInstruction* origin{nullptr};

  /*
   * A union of all possible instructions, descriminated by the op field.
   */
#define O(name, imms, uses, defs) jit::name name##_;
  union { VASM_OPCODES };
#undef O
};

extern const char* vinst_names[];

/*
 * Whether `inst' is a block-terminating instruction.
 */
bool isBlockEnd(const Vinstr& inst);

///////////////////////////////////////////////////////////////////////////////

#define O(name, ...)                             \
  template<> struct Vinstr::matcher<name> {      \
    using type = jit::name;                      \
    static type& get(Vinstr& inst) {             \
      assertx(inst.op == name);                   \
      return inst.name##_;                       \
    }                                            \
    static const type& get(const Vinstr& inst) { \
      assertx(inst.op == name);                   \
      return inst.name##_;                       \
    }                                            \
  };
VASM_OPCODES
#undef O

///////////////////////////////////////////////////////////////////////////////
}}

#endif
