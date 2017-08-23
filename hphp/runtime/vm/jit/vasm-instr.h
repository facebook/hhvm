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

#ifndef incl_HPHP_JIT_VASM_INSTR_H_
#define incl_HPHP_JIT_VASM_INSTR_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-data.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/vixl/a64/constants-a64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

#include <limits>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct IRInstruction;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Field actions:
 *
 *    I(f)      immediate
 *    Inone     no immediates
 *    U(s)      use s
 *    UM(s)     s is a Vptr used to read-modify-write memory
 *    UW(s)     s is a Vptr used to write memory
 *    UA(s)     use s, but s lifetime extends across the instruction
 *    UH(s,h)   use s, try assigning same register as h
 *    D(d)      define d
 *    DH(d,h)   define d, try assigning same register as h
 *    Un,Dn     no uses, defs
 */
#define VASM_OPCODES\
  /* service requests */\
  O(bindjmp, I(target) I(spOff) I(trflags), U(args), Dn)\
  O(bindjcc, I(cc) I(target) I(spOff) I(trflags), U(sf) U(args), Dn)\
  O(bindaddr, I(addr) I(target) I(spOff), Un, Dn)\
  O(fallback, I(target) I(spOff) I(trflags), U(args), Dn)\
  O(fallbackcc, I(cc) I(target) I(spOff) I(trflags), U(sf) U(args), Dn)\
  O(retransopt, I(sk) I(spOff), U(args), Dn)\
  /* vasm intrinsics */\
  O(copy, Inone, UH(s,d), DH(d,s))\
  O(copy2, Inone, UH(s0,d0) UH(s1,d1), DH(d0,s0) DH(d1,s1))\
  O(copyargs, Inone, UH(s,d), DH(d,s))\
  O(debugtrap, Inone, Un, Dn)\
  O(fallthru, Inone, U(args), Dn)\
  O(ldimmb, I(s), Un, D(d))\
  O(ldimmw, I(s), Un, D(d))\
  O(ldimml, I(s), Un, D(d))\
  O(ldimmq, I(s), Un, D(d))\
  O(load, Inone, U(s), D(d))\
  O(store, Inone, U(s) UW(d), Dn)\
  O(mcprep, Inone, Un, D(d))\
  O(phidef, Inone, Un, D(defs))\
  O(phijcc, I(cc), U(uses) U(sf), Dn)\
  O(phijmp, Inone, U(uses), Dn)\
  O(conjure, Inone, Un, D(c))\
  O(conjureuse, Inone, U(c), Dn)\
  O(funcguard, Inone, Un, Dn)\
  /* native function abi */\
  O(vcall, I(call) I(destType) I(fixup), U(args), D(d))\
  O(vinvoke, I(call) I(destType) I(fixup), U(args), D(d))\
  O(call, I(target), U(args), Dn)\
  O(callm, Inone, U(target) U(args), Dn)\
  O(callr, Inone, U(target) U(args), Dn)\
  O(calls, I(target), U(args), Dn)\
  O(ret, Inone, U(args), Dn)\
  /* stub function abi */\
  O(stublogue, Inone, Un, Dn)\
  O(stubret, Inone, U(args), Dn)\
  O(callstub, I(target), U(args), Dn)\
  O(callfaststub, I(fix), U(args), Dn)\
  O(tailcallstub, I(target), U(args), Dn)\
  O(stubunwind, Inone, Un, Dn)\
  O(stubtophp, Inone, U(fp), Dn)\
  O(loadstubret, Inone, Un, D(d))\
  /* php function abi */\
  O(defvmsp, Inone, Un, D(d))\
  O(syncvmsp, Inone, U(s), Dn)\
  O(defvmretdata, Inone, Un, D(data))\
  O(defvmrettype, Inone, Un, D(type))\
  O(syncvmret, Inone, U(data) U(type), Dn)\
  O(phplogue, Inone, U(fp), Dn)\
  O(phpret, Inone, U(fp) U(args), D(d))\
  O(callphp, I(stub), U(args), Dn)\
  O(tailcallphp, Inone, U(target) U(fp) U(args), Dn)\
  O(callarray, I(target), U(args), Dn)\
  O(vcallarray, I(target), U(args) U(extraArgs), Dn)\
  O(contenter, Inone, U(fp) U(target) U(args), Dn)\
  /* vm entry intrinsics */\
  O(calltc, Inone, U(target) U(fp) U(args), Dn)\
  O(resumetc, Inone, U(target) U(args), Dn)\
  O(inittc, Inone, Un, Dn)\
  O(leavetc, Inone, U(args), Dn)\
  /* exception intrinsics */\
  O(landingpad, Inone, Un, Dn)\
  O(nothrow, Inone, Un, Dn)\
  O(syncpoint, I(fix), Un, Dn)\
  O(unwind, Inone, Un, Dn)\
  /* nop and trap */\
  O(nop, Inone, Un, Dn)\
  O(ud2, Inone, Un, Dn)\
  /* restrict/unrestrict new virtuals */\
  O(vregrestrict, Inone, Un, Dn)\
  O(vregunrestrict, Inone, Un, Dn)\
  /* arithmetic instructions */\
  O(addl, I(fl), U(s0) U(s1), D(d) D(sf)) \
  O(addli, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(addlm, I(fl), U(s0) UM(m), D(sf)) \
  O(addlim, I(s0) I(fl), UM(m), D(sf)) \
  O(addq, I(fl), U(s0) U(s1), D(d) D(sf)) \
  O(addqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(addqim, I(s0) I(fl), UM(m), D(sf)) \
  O(addsd, Inone, U(s0) U(s1), D(d))\
  O(andb, I(fl), U(s0) U(s1), D(d) D(sf)) \
  O(andbi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(andbim, I(s) I(fl), UM(m), D(sf)) \
  O(andl, I(fl), U(s0) U(s1), D(d) D(sf)) \
  O(andli, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(andq, I(fl), U(s0) U(s1), D(d) D(sf)) \
  O(andqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(andqi64, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(decl, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(declm, I(fl), UM(m), D(sf))\
  O(decq, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(decqm, I(fl), UM(m), D(sf))\
  O(decqmlock, I(fl), UM(m), D(sf))\
  O(incw, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(incwm, I(fl), UM(m), D(sf))\
  O(incl, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(inclm, I(fl), UM(m), D(sf))\
  O(incq, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(incqm, I(fl), UM(m), D(sf))\
  O(imul, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(divint, Inone, U(s0) U(s1), D(d))\
  O(srem, Inone, U(s0) U(s1), D(d))\
  O(neg, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(notb, Inone, UH(s,d), DH(d,s))\
  O(not, Inone, UH(s,d), DH(d,s))\
  O(orbim, I(s0) I(fl), UM(m), D(sf))\
  O(orwim, I(s0) I(fl), UM(m), D(sf))\
  O(orq, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(orqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf)) \
  O(orqim, I(s0) I(fl), UM(m), D(sf))\
  O(sar, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(shl, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(sarqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(shlli, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(shlqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(shrli, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(shrqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(subl, I(fl), UA(s0) U(s1), D(d) D(sf))\
  O(subli, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(subq, I(fl), UA(s0) U(s1), D(d) D(sf))\
  O(subqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(subsd, Inone, UA(s0) U(s1), D(d))\
  O(xorb, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(xorbi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  O(xorl, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(xorq, I(fl), U(s0) U(s1), D(d) D(sf))\
  O(xorqi, I(s0) I(fl), UH(s1,d), DH(d,s1) D(sf))\
  /* compares and tests */\
  O(cmpb, I(fl), U(s0) U(s1), D(sf))\
  O(cmpbi, I(s0) I(fl), U(s1), D(sf))\
  O(cmpbim, I(s0) I(fl), U(s1), D(sf))\
  O(cmpbm, I(fl), U(s0) U(s1), D(sf))\
  O(cmpw, I(fl), U(s0) U(s1), D(sf))\
  O(cmpwi, I(s0) I(fl), U(s1), D(sf))\
  O(cmpwim, I(s0) I(fl), U(s1), D(sf))\
  O(cmpwm, I(fl), U(s0) U(s1), D(sf))\
  O(cmpl, I(fl), U(s0) U(s1), D(sf))\
  O(cmpli, I(s0) I(fl), U(s1), D(sf))\
  O(cmplm, I(fl), U(s0) U(s1), D(sf))\
  O(cmplim, I(s0) I(fl), U(s1), D(sf))\
  O(cmpq, I(fl), U(s0) U(s1), D(sf))\
  O(cmpqi, I(s0) I(fl), U(s1), D(sf))\
  O(cmpqm, I(fl), U(s0) U(s1), D(sf))\
  O(cmpqim, I(s0) I(fl), U(s1), D(sf))\
  O(cmpsd, I(pred), UA(s0) U(s1), D(d))\
  O(ucomisd, I(fl), U(s0) U(s1), D(sf))\
  O(testb, I(fl), U(s0) U(s1), D(sf))\
  O(testbi, I(s0) I(fl), U(s1), D(sf))\
  O(testbim, I(s0) I(fl), U(s1), D(sf))\
  O(testw, I(fl), U(s0) U(s1), D(sf))\
  O(testwi, I(s0) I(fl), U(s1), D(sf))\
  O(testwim, I(s0) I(fl), U(s1), D(sf))\
  O(testl, I(fl), U(s0) U(s1), D(sf))\
  O(testli, I(s0) I(fl), U(s1), D(sf))\
  O(testlim, I(s0) I(fl), U(s1), D(sf))\
  O(testq, I(fl), U(s0) U(s1), D(sf))\
  O(testqi, I(s0) I(fl), U(s1), D(sf))\
  O(testqm, I(fl), U(s0) U(s1), D(sf))\
  O(testqim, I(s0) I(fl), U(s1), D(sf))\
  /* conditional operations */\
  O(cloadq, I(cc), U(sf) U(f) U(t), D(d))\
  O(cmovb, I(cc), U(sf) UH(f,d) U(t), DH(d,f))\
  O(cmovw, I(cc), U(sf) UH(f,d) U(t), DH(d,f))\
  O(cmovl, I(cc), U(sf) UH(f,d) U(t), DH(d,f))\
  O(cmovq, I(cc), U(sf) UH(f,d) U(t), DH(d,f))\
  O(setcc, I(cc), U(sf), D(d))\
  /* load effective address */\
  O(lea, Inone, U(s), D(d))\
  O(leap, I(s), Un, D(d))\
  O(lead, I(s), Un, D(d))\
  /* copies */\
  O(movb, Inone, UH(s,d), DH(d,s))\
  O(movw, Inone, UH(s,d), DH(d,s))\
  O(movl, Inone, UH(s,d), DH(d,s))\
  O(movzbw, Inone, UH(s,d), DH(d,s))\
  O(movzbl, Inone, UH(s,d), DH(d,s))\
  O(movzbq, Inone, UH(s,d), DH(d,s))\
  O(movzwl, Inone, UH(s,d), DH(d,s))\
  O(movzwq, Inone, UH(s,d), DH(d,s))\
  O(movzlq, Inone, UH(s,d), DH(d,s))\
  O(movtdb, Inone, UH(s,d), DH(d,s))\
  O(movtdq, Inone, UH(s,d), DH(d,s))\
  O(movtqb, Inone, UH(s,d), DH(d,s))\
  O(movtqw, Inone, UH(s,d), DH(d,s))\
  O(movtql, Inone, UH(s,d), DH(d,s))\
  /* loads/stores */\
  O(loadb, Inone, U(s), D(d))\
  O(loadw, Inone, U(s), D(d))\
  O(loadl, Inone, U(s), D(d))\
  O(loadqp, I(s), Un, D(d))\
  O(loadqd, I(s), Un, D(d))\
  O(loadups, Inone, U(s), D(d))\
  O(loadsd, Inone, U(s), D(d))\
  O(loadzbl, Inone, U(s), D(d))\
  O(loadzbq, Inone, U(s), D(d))\
  O(loadzlq, Inone, U(s), D(d))\
  O(loadtqb, Inone, U(s), D(d))\
  O(loadtql, Inone, U(s), D(d))\
  O(storeb, Inone, U(s) UW(m), Dn)\
  O(storebi, I(s), UW(m), Dn)\
  O(storew, Inone, U(s) UW(m), Dn)\
  O(storewi, I(s), UW(m), Dn)\
  O(storel, Inone, U(s) UW(m), Dn)\
  O(storeli, I(s), UW(m), Dn)\
  O(storeqi, I(s), UW(m), Dn)\
  O(storeups, Inone, U(s) UW(m), Dn)\
  O(storesd, Inone, U(s) UW(m), Dn)\
  /* branches */\
  O(jcc, I(cc), U(sf), Dn)\
  O(jcci, I(cc), U(sf), Dn)\
  O(jmp, Inone, Un, Dn)\
  O(jmpr, Inone, U(target) U(args), Dn)\
  O(jmpm, Inone, U(target) U(args), Dn)\
  O(jmpi, I(target), U(args), Dn)\
  /* push/pop */\
  O(pop, Inone, Un, D(d))\
  O(popf, Inone, Un, D(d))\
  O(popm, Inone, UW(d), Dn)\
  O(popp, Inone, Un, D(d0) D(d1))\
  O(poppm, Inone, UW(d0) UW(d1), Dn)\
  O(push, Inone, U(s), Dn)\
  O(pushf, Inone, U(s), Dn)\
  O(pushm, Inone, U(s), Dn)\
  O(pushp, Inone, U(s0) U(s1), Dn)\
  O(pushpm, Inone, U(s0) U(s1), Dn)\
  /* floating-point conversions */\
  O(cvttsd2siq, Inone, U(s), D(d))\
  O(cvtsi2sd, Inone, U(s), D(d))\
  O(cvtsi2sdm, Inone, U(s), D(d))\
  O(unpcklpd, Inone, UA(s0) U(s1), D(d))\
  /* other floating-point */\
  O(absdbl, Inone, UH(s,d), DH(d,s))\
  O(divsd, Inone, UA(s0) U(s1), D(d))\
  O(mulsd, Inone, U(s0) U(s1), D(d))\
  O(roundsd, I(dir), U(s), D(d))\
  O(sqrtsd, Inone, U(s), D(d))\
  /* x64 instructions */\
  O(cqo, Inone, Un, Dn)\
  O(idiv, I(fl), U(s), D(sf))\
  O(sarq, I(fl), UH(s,d), DH(d,s) D(sf))\
  O(shlq, I(fl), UH(s,d), DH(d,s) D(sf))\
  /* arm instructions */\
  O(csincb, I(cc), U(sf) U(f) U(t), D(d))\
  O(csincw, I(cc), U(sf) U(f) U(t), D(d))\
  O(csincl, I(cc), U(sf) U(f) U(t), D(d))\
  O(csincq, I(cc), U(sf) U(f) U(t), D(d))\
  O(fcvtzs, Inone, U(s), D(d))\
  O(mrs, I(s), Un, D(r))\
  O(msr, I(s), U(r), Dn)\
  O(ubfmli, I(mr) I(ms), U(s), D(d))\
  /* ppc64 instructions */\
  O(extsb, Inone, UH(s,d), DH(d,s))\
  O(extsw, Inone, UH(s,d), DH(d,s))\
  O(extsl, Inone, UH(s,d), DH(d,s))\
  O(fcmpo, Inone, U(s0) U(s1), D(sf))\
  O(fcmpu, Inone, U(s0) U(s1), D(sf))\
  O(fctidz, Inone, U(s), D(d) D(sf))\
  O(mflr, Inone, Un, D(d))\
  O(mtlr, Inone, U(s), Dn)\
  /* */

/*
 * ATT style operand order.  For binary ops:
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
 *    sd  double
 *    i   immediate
 *    m   Vptr
 *    p   RIPRelativeRef
 *    d   VdataPtr
 *    s   smashable
 */

///////////////////////////////////////////////////////////////////////////////
// Service requests.
//
// `spOff' is the bytecode eval stack pointer's offset from rvmfp(); we use it
// to sync rvmsp().

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

struct bindaddr {
  explicit bindaddr(VdataPtr<TCA> addr, SrcKey target, FPInvOffset spOff)
    : addr(addr)
    , target(target)
    , spOff(spOff)
  {}

  VdataPtr<TCA> addr;
  SrcKey target;
  FPInvOffset spOff;
};

struct fallback {
  explicit fallback(SrcKey target,
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

struct fallbackcc {
  explicit fallbackcc(ConditionCode cc,
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

struct retransopt {
  explicit retransopt(SrcKey sk,
                      FPInvOffset spOff,
                      RegSet args)
    : sk(sk)
    , spOff(spOff)
    , args{args}
  {}

  SrcKey sk;
  FPInvOffset spOff;
  RegSet args;
};

///////////////////////////////////////////////////////////////////////////////
// VASM intrinsics.

/*
 * Copies of different arities.
 *
 * All copies happen in parallel, meaning operand order doesn't matter when a
 * PhysReg appears as both a src and dst.
 */
struct copy { Vreg s, d; };
struct copy2 { Vreg64 s0, s1, d0, d1; };
struct copyargs { Vtuple s, d; };

/*
 * Cause any attached debugger to trap.
 *
 * Process may abort if no debugger is attached.
 */
struct debugtrap {};

/*
 * No-op.
 *
 * Used for marking the end of a block that is intentionally going to fall
 * through.  Only for use with Vauto.
 */
struct fallthru { RegSet args; };

/*
 * Load an immedate value without mutating status flags.
 */
struct ldimmb { Immed s; Vreg d; };
struct ldimmw { Immed s; Vreg16 d; };
struct ldimml { Immed s; Vreg d; };
struct ldimmq { Immed64 s; Vreg d; };

/*
 * Memory operand load and store.
 */
struct load { Vptr s; Vreg d; };
struct store { Vreg s; Vptr d; };

/*
 * Method cache smashable prime data.
 *
 * @see: cgLdObjMethod()
 */
struct mcprep { Vreg64 d; };

/*
 * Phis.
 *
 * @see: doWhile(), for an example usage.
 */
struct phidef { Vtuple defs; };
struct phijmp { Vlabel target; Vtuple uses; };
struct phijcc { ConditionCode cc; VregSF sf; Vlabel targets[2]; Vtuple uses; };

/*
 * These marker instructions are used to model dataflow in pseudo-translations.
 * They should not be used in any translations that will eventually be emitted.
 */
struct conjure { Vreg c; };
struct conjureuse { Vreg c; };

/*
 * Emit a function prologue guard.
 * *watch will be set to the address following the guard.
 */
struct funcguard { const Func* func; TCA* watch; };

///////////////////////////////////////////////////////////////////////////////
// Native function ABI.

/*
 * Native or stub function call, without or with exception edges, respectively.
 *
 * Contains information about a helper call (i.e., any non-PHP function call)
 * needed for lowering to different target architectures.
 */
struct vcall { CallSpec call; VcallArgsId args; Vtuple d;
               Fixup fixup; DestType destType; bool nothrow; };
struct vinvoke { CallSpec call; VcallArgsId args; Vtuple d; Vlabel targets[2];
                 Fixup fixup; DestType destType; };

/*
 * C++ function call using the native ABI.
 *
 * Comes in five flavors:
 *    call:  direct call
 *    callm: indirect call via memory operand
 *    callr: indirect call via register
 *    calls: direct call with smashable target
 *
 * (These follow the same suffix conventions described above.)
 *
 * If `watch' is set, *watch will be set to the address immediately following
 * the call instruction---useful for various unwinder hijinks.
 */
struct call  { CodeAddress target; RegSet args; TCA* watch; };
struct callm { Vptr target; RegSet args; };
struct callr { Vreg64 target; RegSet args; };
struct calls { CodeAddress target; RegSet args; };

/*
 * Native function return.
 */
struct ret { RegSet args; };

///////////////////////////////////////////////////////////////////////////////
// Stub function ABI.

/*
 * Stub function prologue.
 *
 * Set up the stub call frame---which has the same layout as the native call
 * frame (and hence is platform dependent), except not all components are
 * required to be valid.
 *
 * On x64, for example, this leaves the native stack as follows:
 *
 *    +-----------------------+   <- native stack pointer, pre-call
 *    |     return address    |
 *    +-----------------------+
 *    | <junk> or saved rvmfp |
 *    +-----------------------+   <- native stack pointer, after stublogue{}
 *
 * The return address is always required, but the frame pointer is only
 * optionally saved, when `saveframe' is set because the stub (or some native
 * helper that it calls) needs to use it.
 *
 * Stubs are not allowed to spill registers to the stack, so the native stack
 * pointer will only be adjusted if the stub code does so intentionally.
 */
struct stublogue { bool saveframe; };

/*
 * Return from a stub.
 *
 * Return to the address saved on the stack, and restore the native stack
 * pointer to wherever it was before the stub call.  See the diagram for
 * stublogue{}, above; `saveframe' has the same meaning as it does there.
 */
struct stubret { RegSet args; bool saveframe; };

/*
 * Direct call to the unique stub at `target'.
 *
 * The `target' should begin with a stublogue{} instruction, which together
 * with callstub{} should implement the ABI described above.
 */
struct callstub { CodeAddress target; RegSet args; };

/*
 * Call a "fast" stub, a stub that preserves more registers than a normal call.
 *
 * It may still call C++ functions on a slow path (which is why there's a Fixup
 * operand) but it will save any required registers before doing so.
 */
struct callfaststub { TCA target; Fixup fix; RegSet args; };

/*
 * Make a direct tail call to a stub.
 *
 * As in the usual sense of tail call, this is really a jmp which will cause
 * the callee's return to serve as the caller's return.
 *
 * This instruction jumps from a context dominated by stublogue{} to a context
 * which wants to execute a logically identical prologue, so it needs to revert
 * the world to a pre-stublogue{} state before jumping.
 */
struct tailcallstub { CodeAddress target; RegSet args; };

/*
 * Restore %rsp when leaving a stub context via an exception edge.
 *
 * When we unwind into normal TC frames (i.e., for PHP functions), we require
 * that %rsp be restored correctly, since we use spill space as our means of
 * restoring all other registers.  Thus, when we leave a stub because of an
 * exception, we have to undo the stack effects of both the stublogue{} and the
 * callstub{}.
 */
struct stubunwind {};

/*
 * Convert from a stublogue{} context to a phplogue{} context.  `fp' is the
 * target PHP context's frame.
 *
 * This is only used by fcallArrayHelper, which needs to begin with a
 * stublogue{} (see unique-stubs.cpp) and later perform the work of phplogue{}.
 *
 * This instruction should, in theory, teleport the stub frame's saved %rip
 * onto the PHP callee's frame.  However, since fcallArrayHelper is the only
 * user, and since the PHP frame's m_savedRip always gets updated by a native
 * helper before stubtophp{} is hit, for now, implementations of stubtophp{}
 * needn't touch the callee frame at all.
 */
struct stubtophp { Vreg fp; };

/*
 * Load the saved return address from the stub's frame record.
 *
 * This is only valid from a stublogue{} context, and when the native stack
 * pointer has not been further adjusted.
 */
struct loadstubret { Vreg d; };

///////////////////////////////////////////////////////////////////////////////
// PHP function ABI.

/*
 * Copy rvmsp() into `d'.
 *
 * Used once per region when reentering a resumed function after an ABI
 * boundary.
 */
struct defvmsp { Vreg d; };

/*
 * Copy `s' into rvmsp().
 *
 * Used right before leaving translated code for an ABI boundary, such as
 * bindjmp{} or fallbackcc{}.
 */
struct syncvmsp { Vreg s; };

/*
 * Copy the PHP return value from the return registers into `data' and `type'.
 *
 * Used right after an instruction that makes a PHP call (like the
 * suggestively-named callphp{}) to receive the values as Vregs.
 */
struct defvmretdata { Vreg data; };
struct defvmrettype { Vreg type; };

/*
 * Copy a PHP return value into the return registers (rreg(0) and rreg(1)).
 *
 * This should be used right before we execute a phpret{}.
 */
struct syncvmret { Vreg data; Vreg type; };

/*
 * PHP function prologue.
 *
 * Save the return instruction pointer in m_savedRip on the current VM frame,
 * `fp', and ensure that the native stack pointer is in the same position as it
 * was before the instruction that transferred control to us.
 *
 * The phplogue should dominate all code that is logically part of a PHP func
 * prologue or func body (but /not/ the func guard, which precedes it).  Note
 * that this includes unique stubs like fcallHelperThunk, which are reached by
 * PHP function call.
 *
 * Ultimately, anytime we hit a phplogue, we came from enterTCHelper, which
 * means that after the phplogue (since we maintain the native stack pointer),
 * the stack looks like this:
 *
 *    +-----------------------+
 *    |  <enterTCHelper+???>  |
 *    +-----------------------+   <- native stack pointer
 *
 * i.e., the return address in enterTCHelper of the call that put us in the TC.
 * The native stack continues to point here as long as we are in the TC, modulo
 * register allocator spill space.
 */
struct phplogue { Vreg fp; };

/*
 * Load fp[m_sfp] into `d' and return to m_savedRip on `fp'.
 *
 * If `noframe' is set, `d' is not changed.
 */
struct phpret { Vreg fp; Vreg d; RegSet args; bool noframe; };

/*
 * Call a PHP function.
 *
 * This is a smashable call that begins its life as a request to translate the
 * callee, and winds up as a direct call to the callee's func guard or
 * prologue.
 */
struct callphp {
  explicit callphp(TCA stub,
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
 * Make an indirect tail call to a PHP function.
 *
 * Analogous to tailcallstub{}; undoes phplogue{} and then jumps to `target',
 * which begins with a logically identical phplogue{}.
 */
struct tailcallphp { Vreg target; Vreg fp; RegSet args; };

/*
 * Non-smashable PHP function call with (almost) the same ABI as callphp{}.
 *
 * NB: The only difference is that callarray preserves vmfp.  Currently only
 * used by the CallArray instruction.
 */
struct callarray { TCA target; RegSet args; };

/*
 * High-level version of callarray.
 *
 * Has exception edges and additional integer args (used by the `target' stub).
 */
struct vcallarray { TCA target; RegSet args; Vtuple extraArgs;
                    Vlabel targets[2]; };

/*
 * Enter a continuation (with exception edges).
 *
 * `fp' is the continuation's frame pointer, and `target' is the code address
 * at which to resume execution.
 *
 * Since `target' is dominated by a phplogue{}, we must implement to its ABI.
 * For most architectures, this will probably require calling a small stub
 * function.
 */
struct contenter { Vreg64 fp, target; RegSet args; Vlabel targets[2]; };

///////////////////////////////////////////////////////////////////////////////
// VM entry ABI.

/*
 * Call into the TC at a function prologue.
 *
 * This sets up for a phplogue{} and transfers control to `target'---which
 * logically executes said phplogue{} before doing anything else.
 *
 * Before calltc{} is executed, the stack will always be set up like this:
 *
 *    +-----------------------+   <- 16-byte alignment
 *    |   <8 bytes of junk>   |
 *    +-----------------------+   <- native stack pointer
 *
 * i.e., the native stack pointer will be misaligned coming in (but must, of
 * course, be aliged once phplogue{} finishes executing).  Using a native call
 * in the implementation (and likewise, using native returns for leavetc{}) is
 * recommended, in order to take advantage of return branch predictions.
 *
 * `fp' is the callee's ActRec, which will have already been set up
 * appropriately.  `exittc' is the address to resume execution at after
 * returning from the TC.
 */
struct calltc { Vreg64 target, fp; TCA exittc; RegSet args; };

/*
 * Resume execution in the middle of a TC function.
 *
 * This must set up the native stack in the same way as a phplogue{} would,
 * before transferring control to `target'.  As with calltc{}, the native stack
 * pointer is misaligned coming in, and using a native call is recommended.
 *
 * `exittc' is the address to resume execution at after returning from the TC.
 */
struct resumetc { Vreg64 target; TCA exittc; RegSet args; };

/*
 * Architecture specific initialization to be done at the beginning of
 * enterTCHelper, if needed.
 */
struct inittc {};

/*
 * Pop the address of enterTCExit off the stack, and return to it.
 *
 * Used to relinquish control to the async scheduler from an async function.
 */
struct leavetc { RegSet args; };

///////////////////////////////////////////////////////////////////////////////
// Exception intrinsics.

/*
 * Header for catch blocks.
 */
struct landingpad {};

/*
 * Register a null catch trace at this position.
 *
 * This tells the unwinder that the function call returning to here isn't
 * allowed to throw.
 */
struct nothrow {};

/*
 * Register a fixup at this position.
 *
 * These are used by the unwinder to reconstruct the state of the VM registers.
 */
struct syncpoint { Fixup fix; };

/*
 * Terminate a block after a call that can throw, with edges to a catch block
 * and a fallthrough block.
 */
struct unwind { Vlabel targets[2]; };

///////////////////////////////////////////////////////////////////////////////

/*
 * Unless specifically noted otherwise, instructions with Vreg{8,16,32} dsts
 * can do whatever they please with the upper bits.
 */

/*
 * Nop and trap.
 */
struct nop {};
struct ud2 {};

/*
 * Restrict/unrestrict new virtuals.
 */
struct vregrestrict {};
struct vregunrestrict {};

/*
 * Arithmetic instructions.
 */
// add: s0 + {s1|m} => {d|m}, sf
struct addl   { Vreg32 s0, s1, d; VregSF sf; Vflags fl; };
struct addli  { Immed s0; Vreg32 s1, d; VregSF sf; Vflags fl; };
struct addlm  { Vreg32 s0; Vptr m; VregSF sf; Vflags fl; };
struct addlim { Immed s0; Vptr m; VregSF sf; Vflags fl; };
struct addq  { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct addqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct addqim { Immed s0; Vptr m; VregSF sf; Vflags fl; };
struct addsd  { VregDbl s0, s1, d; };
// and: s0 & {s1|m} => {d|m}, sf
struct andb  { Vreg8 s0, s1, d; VregSF sf; Vflags fl; };
struct andbi { Immed s0; Vreg8 s1, d; VregSF sf; Vflags fl; };
struct andbim { Immed s; Vptr m; VregSF sf; Vflags fl; };
struct andl  { Vreg32 s0, s1, d; VregSF sf; Vflags fl; };
struct andli { Immed s0; Vreg32 s1, d; VregSF sf; Vflags fl; };
struct andq  { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct andqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
// dec: {s|m} - 1 => {d|m}, sf
struct decl { Vreg32 s, d; VregSF sf; Vflags fl; };
struct declm { Vptr m; VregSF sf; Vflags fl; };
struct decq { Vreg64 s, d; VregSF sf; Vflags fl; };
struct decqm { Vptr m; VregSF sf; Vflags fl; };
struct decqmlock { Vptr m; VregSF sf; Vflags fl; };
// inc: {s|m} + 1 => {d|m}, sf
struct incw { Vreg16 s, d; VregSF sf; Vflags fl; };
struct incwm { Vptr m; VregSF sf; Vflags fl; };
struct incl { Vreg32 s, d; VregSF sf; Vflags fl; };
struct inclm { Vptr m; VregSF sf;  Vflags fl;};
struct incq { Vreg64 s, d; VregSF sf; Vflags fl; };
struct incqm { Vptr m; VregSF sf; Vflags fl; };
// mul: s0 * s1 => d, sf
struct imul { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
// div/mod: s0 / s1 => d
struct divint { Vreg64 s0, s1, d; };
struct srem { Vreg64 s0, s1, d; };
// neg: 0 - s => d, sf
struct neg { Vreg64 s, d; VregSF sf; Vflags fl; };
// not: ~s => d
struct notb { Vreg8 s, d; };
struct not { Vreg64 s, d; };
// or: s0 | {s1|m} => {d|m}, sf
struct orbim { Immed s0; Vptr m; VregSF sf; Vflags fl; };
struct orwim { Immed s0; Vptr m; VregSF sf; Vflags fl; };
struct orq { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct orqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct orqim { Immed s0; Vptr m; VregSF sf; Vflags fl; };
// shift: s1 << s0 => d, sf
struct sar { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct shl { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct sarqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct shlli { Immed s0; Vreg32 s1, d; VregSF sf; Vflags fl; };
struct shlqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct shrli { Immed s0; Vreg32 s1, d; VregSF sf; Vflags fl; };
struct shrqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
// sub: s1 - s0 => d, sf
struct subl { Vreg32 s0, s1, d; VregSF sf; Vflags fl; };
struct subli { Immed s0; Vreg32 s1, d; VregSF sf; Vflags fl; };
struct subq { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct subqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct subsd { VregDbl s0, s1, d; };
// xor: s0 ^ s1 => d, sf
struct xorb { Vreg8 s0, s1, d; VregSF sf; Vflags fl; };
struct xorbi { Immed s0; Vreg8 s1, d; VregSF sf; Vflags fl; };
struct xorl { Vreg32 s0, s1, d; VregSF sf; Vflags fl; };
struct xorq { Vreg64 s0, s1, d; VregSF sf; Vflags fl; };
struct xorqi { Immed s0; Vreg64 s1, d; VregSF sf; Vflags fl; };

/*
 * Compares and tests.
 */
// s1 - s0 => sf
struct cmpb { Vreg8 s0; Vreg8 s1; VregSF sf; Vflags fl; };
struct cmpbi { Immed s0; Vreg8 s1; VregSF sf; Vflags fl; };
struct cmpbim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpbm { Vreg8 s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpw { Vreg16 s0; Vreg16 s1; VregSF sf; Vflags fl; };
struct cmpwi { Immed s0; Vreg16 s1; VregSF sf; Vflags fl; };
struct cmpwim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpwm { Vreg16 s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpl { Vreg32 s0; Vreg32 s1; VregSF sf; Vflags fl; };
struct cmpli { Immed s0; Vreg32 s1; VregSF sf; Vflags fl; };
struct cmplm { Vreg32 s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmplim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpq { Vreg64 s0; Vreg64 s1; VregSF sf; Vflags fl; };
struct cmpqi { Immed s0; Vreg64 s1; VregSF sf; Vflags fl; };
struct cmpqm { Vreg64 s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpqim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct cmpsd { ComparisonPred pred; VregDbl s0, s1, d; };
struct ucomisd { VregDbl s0, s1; VregSF sf; Vflags fl; };
// s1 & s0 => sf
struct testb { Vreg8 s0, s1; VregSF sf; Vflags fl; };
struct testbi { Immed s0; Vreg8 s1; VregSF sf; Vflags fl; };
struct testbim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct testw { Vreg16 s0, s1; VregSF sf; Vflags fl; };
struct testwi { Immed s0; Vreg16 s1; VregSF sf; Vflags fl; };
struct testwim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct testl { Vreg32 s0, s1; VregSF sf; Vflags fl; };
struct testli { Immed s0; Vreg32 s1; VregSF sf; Vflags fl; };
struct testlim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };
struct testq { Vreg64 s0, s1; VregSF sf; Vflags fl; };
struct testqi { Immed s0; Vreg64 s1; VregSF sf; Vflags fl; };
struct testqm { Vreg64 s0; Vptr s1; VregSF sf; Vflags fl; };
struct testqim { Immed s0; Vptr s1; VregSF sf; Vflags fl; };

/*
 * Conditional operations.
 */
// t1 = load t; d = condition ? t1 : f
struct cloadq { ConditionCode cc; VregSF sf; Vreg64 f; Vptr t; Vreg64 d; };
// d = condition ? t : f
struct cmovb { ConditionCode cc; VregSF sf; Vreg8 f, t, d; };
struct cmovw { ConditionCode cc; VregSF sf; Vreg16 f, t, d; };
struct cmovl { ConditionCode cc; VregSF sf; Vreg32 f, t, d; };
struct cmovq { ConditionCode cc; VregSF sf; Vreg64 f, t, d; };
// d = condition ? 1 : 0
struct setcc { ConditionCode cc; VregSF sf; Vreg8 d; };

/*
 * Load effective address.
 */
struct lea { Vptr s; Vreg64 d; };
struct leap { RIPRelativeRef s; Vreg64 d; };
struct lead { VdataPtr<void> s; Vreg64 d; };

/*
 * Copies.
 */
// moves
struct movb { Vreg8 s, d; };
struct movw { Vreg16 s, d; };
struct movl { Vreg32 s, d; };
// zero-extended s to d
struct movzbw { Vreg8 s; Vreg16 d; };
struct movzbl { Vreg8 s; Vreg32 d; };
struct movzbq { Vreg8 s; Vreg64 d; };
struct movzwl { Vreg16 s; Vreg32 d; };
struct movzwq { Vreg16 s; Vreg64 d; };
struct movzlq { Vreg32 s; Vreg64 d; };
// truncated s to d
struct movtdb { VregDbl s; Vreg8 d; };
struct movtdq { VregDbl s; Vreg64 d; };
struct movtqb { Vreg64 s; Vreg8 d; };
struct movtqw { Vreg64 s; Vreg16 d; };
struct movtql { Vreg64 s; Vreg32 d; };


/*
 * Loads and stores.
 */
// loads
struct loadb { Vptr s; Vreg8 d; };
struct loadw { Vptr s; Vreg16 d; };
struct loadl { Vptr s; Vreg32 d; };
struct loadqp { RIPRelativeRef s; Vreg64 d; };
struct loadqd { VdataPtr<uint64_t> s; Vreg64 d; };
struct loadups { Vptr s; Vreg128 d; };
struct loadsd { Vptr s; VregDbl d; };
// zero-extended s to d
struct loadzbl { Vptr s; Vreg32 d; };
struct loadzbq { Vptr s; Vreg64 d; };
struct loadzlq { Vptr s; Vreg64 d; };
// truncated s to d
struct loadtqb { Vptr s; Vreg8 d; };
struct loadtql { Vptr s; Vreg32 d; };
// stores
struct storeb { Vreg8 s; Vptr m; };
struct storebi { Immed s; Vptr m; };
struct storew { Vreg16 s; Vptr m; };
struct storewi { Immed s; Vptr m; };
struct storel { Vreg32 s; Vptr m; };
struct storeli { Immed s; Vptr m; };
struct storeqi { Immed s; Vptr m; };
struct storeups { Vreg128 s; Vptr m; };
struct storesd { VregDbl s; Vptr m; };

/*
 * Branch instructions.
 *
 * In vasm, targets are always ordered {next, taken}.
 */
struct jcc { ConditionCode cc; VregSF sf; Vlabel targets[2]; StringTag tag; };
struct jcci { ConditionCode cc; VregSF sf; Vlabel target; TCA taken; };
struct jmp { Vlabel target; };
struct jmpr { Vreg64 target; RegSet args; };
struct jmpm { Vptr target; RegSet args; };
struct jmpi { TCA target; RegSet args; };

/*
 * Push/pop to rsp().
 */
struct pop { Vreg64 d; };
struct popf { VregSF d; };
struct popm { Vptr d; };
// popp[m]{d0, d1} -> pop[m]{d0}, pop[m]{d1}
struct popp { Vreg64 d0, d1; };
struct poppm { Vptr d0, d1; };
struct push { Vreg64 s; };
struct pushf { VregSF s; };
struct pushm { Vptr s; };
// pushp[m]{s0, s1} -> push[m]{s0}, push[m]{s1}
struct pushp { Vreg64 s0, s1; };
struct pushpm { Vptr s0, s1; };

/*
 * Integer-float conversions.
 */
struct cvttsd2siq { VregDbl s; Vreg64 d; };
struct cvtsi2sd { Vreg64 s; VregDbl d; };
struct cvtsi2sdm { Vptr s; VregDbl d; };
struct unpcklpd { VregDbl s0, s1; Vreg128 d; };

/*
 * Undocumented floating-point instructions.
 */
struct absdbl { VregDbl s, d; };
struct divsd { VregDbl s0, s1, d; };
struct mulsd  { VregDbl s0, s1, d; };
struct roundsd { RoundDirection dir; VregDbl s, d; };
struct sqrtsd { VregDbl s, d; };

///////////////////////////////////////////////////////////////////////////////

/*
 * x64 intrinsics.
 */
struct cqo {};
struct idiv { Vreg64 s; VregSF sf; Vflags fl; };
struct sarq { Vreg64 s, d; VregSF sf; Vflags fl; }; // uses rcx
struct shlq { Vreg64 s, d; VregSF sf; Vflags fl; }; // uses rcx

/*
 * arm intrinsics.
 */
struct andqi64 { Immed64 s0; Vreg64 s1, d; VregSF sf; Vflags fl; };
struct csincb { ConditionCode cc; VregSF sf; Vreg8 f, t, d; };
struct csincw { ConditionCode cc; VregSF sf; Vreg16 f, t, d; };
struct csincl { ConditionCode cc; VregSF sf; Vreg32 f, t, d; };
struct csincq { ConditionCode cc; VregSF sf; Vreg64 f, t, d; };
struct fcvtzs { VregDbl s; Vreg64 d;};
struct mrs { Immed s; Vreg64 r; };
struct msr { Vreg64 r; Immed s; };
struct ubfmli { Immed mr, ms; Vreg32 s, d; };

/*
 * ppc64 intrinsics.
 */
struct extsb { Vreg8 s; Vreg64 d; };  // Extend byte sign
struct extsw { Vreg16 s; Vreg64 d; }; // Extend word sign
struct extsl { Vreg32 s; Vreg64 d; }; // Extend dword sign
struct fcmpo { VregDbl s0; VregDbl s1; VregSF sf; };
struct fcmpu { VregDbl s0; VregDbl s1; VregSF sf; };
struct fctidz { VregDbl s; VregDbl d; VregSF sf; };
struct mflr { Vreg64 d; };
struct mtlr { Vreg64 s; };

///////////////////////////////////////////////////////////////////////////////

struct Vinstr {
#define O(name, imms, uses, defs) name,
  enum Opcode : uint8_t { VASM_OPCODES };
#undef O

  /*
   * Helper struct for transferring the IR context of a Vinstr during
   * optimization passes.
   */
  struct ir_context {
    const IRInstruction* origin;
    uint16_t voff;
  };

  static constexpr auto kInvalidVoff = std::numeric_limits<uint16_t>::max();

  /////////////////////////////////////////////////////////////////////////////

  Vinstr() : op(ud2) {}

#define O(name, imms, uses, defs) \
  /* implicit */ Vinstr(jit::name i, ir_context ctx = ir_context{}) \
    : op(name)                    \
    , voff(ctx.voff)              \
    , origin(ctx.origin)          \
    , name##_(i)                  \
  {}
  VASM_OPCODES
#undef O

  /*
   * Define an assignment operator for all instructions that preserves origin,
   * voff, and pos.
   */
#define O(name, ...)                            \
  Vinstr& operator=(const jit::name& i) {       \
    op = Vinstr::name;                          \
    name##_ = i;                                \
    return *this;                               \
  }
  VASM_OPCODES
#undef O

  template<typename Op> struct matcher;
  template<Opcode op> struct op_matcher;

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
  template<Opcode op>
  typename op_matcher<op>::type& get() {
    return op_matcher<op>::get(*this);
  }
  template<Opcode op>
  const typename op_matcher<op>::type& get() const {
    return op_matcher<op>::get(*this);
  }

  /*
   * Get and set the IR "context" members.
   */
  ir_context irctx() const {
    return ir_context { origin, voff };
  }
  void set_irctx(ir_context ctx) {
    origin = ctx.origin;
    voff = ctx.voff;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  Opcode op;

  /*
   * The index of this instruction within the code for `origin'.
   */
  uint16_t voff;

  // 2-byte hole here.

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

///////////////////////////////////////////////////////////////////////////////

#define O(name, ...)                              \
  template<> struct Vinstr::matcher<name> {       \
    using type = jit::name;                       \
    static type& get(Vinstr& inst) {              \
      assertx(inst.op == name);                   \
      return inst.name##_;                        \
    }                                             \
    static const type& get(const Vinstr& inst) {  \
      assertx(inst.op == name);                   \
      return inst.name##_;                        \
    }                                             \
  };                                              \
  template<> struct Vinstr::op_matcher<Vinstr::name> {  \
    using type = jit::name;                       \
    static type& get(Vinstr& inst) {              \
      assertx(inst.op == name);                   \
      return inst.name##_;                        \
    }                                             \
    static const type& get(const Vinstr& inst) {  \
      assertx(inst.op == name);                   \
      return inst.name##_;                        \
    }                                             \
  };
VASM_OPCODES
#undef O

///////////////////////////////////////////////////////////////////////////////

/*
 * Whether `inst' is a block-terminating instruction.
 */
bool isBlockEnd(const Vinstr& inst);

/*
 * Whether `op' or `inst' is a call instruction.
 */
bool isCall(Vinstr::Opcode op);
inline bool isCall(const Vinstr& inst) { return isCall(inst.op); }

/*
 * The register width specification of `op'.
 *
 * If `op' is an instruction whose non-flags register arguments are all a
 * certain width, return that width; otherwise, return Width::AnyNF (anything
 * but a flags reg).
 *
 * In particular, Width::AnyNF is returned for intrinsics, architecture-specific
 * instructions, zero-extending or truncating reg moves, branches, pushes/pops,
 * and floating-point conversions.  All other instructions have operands of
 * fixed and uniform width.
 */
Width width(Vinstr::Opcode op);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
