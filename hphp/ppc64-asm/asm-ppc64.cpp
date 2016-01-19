/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015                                   |
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

#include "hphp/ppc64-asm/asm-ppc64.h"

namespace ppc64_asm {

/*
 * Macro definition for EmitXOForm functions
 * Format:
 *  X(name,   arg3,  oe,  xop)
 *    name: function name
 *    arg3: ARG if needed, otherwise NONE to skip
 *    oe:   parameter value
 *    xop:  parameter value
 */

#define ADDS\
  X(add,     ARG,   0,  266)  \
  X(addme,   NONE,  0,  234)  \
  X(addc,    ARG,   0,  10)   \
  X(addco,   ARG,   1,  10)   \
  X(adde,    ARG,   0,  138)  \
  X(addeo,   ARG,   1,  138)  \
  X(addmeo,  NONE,  1,  234)  \
  X(addo,    ARG,   1,  266)  \
  X(addze,   NONE,  0,  202)  \
  X(addzeo,  NONE,  1,  202)  \
  X(divd,    ARG,   0,  489)  \
  X(divde,   ARG,   0,  425)  \
  X(divdeo,  ARG,   1,  425)  \
  X(divdeu,  ARG,   0,  393)  \
  X(divdeuo, ARG,   1,  393)  \
  X(divdo,   ARG,   1,  489)  \
  X(divdu,   ARG,   0,  457)  \
  X(divduo,  ARG,   1,  457)  \
  X(divw,    ARG,   0,  491)  \
  X(divwe,   ARG,   0,  427)  \
  X(divweo,  ARG,   1,  427)  \
  X(divweu,  ARG,   0,  395)  \
  X(divweuo, ARG,   1,  395)  \
  X(divwo,   ARG,   1,  491)  \
  X(divwu,   ARG,   0,  459)  \
  X(divwuo,  ARG,   1,  459)  \
  X(mulhd,   ARG,   0,  73)   \
  X(mulhdu,  ARG,   0,  9)    \
  X(mulhw,   ARG,   0,  75)   \
  X(mulhwu,  ARG,   0,  11)   \
  X(mulld,   ARG,   0,  233)  \
  X(mulldo,  ARG,   1,  233)  \
  X(mullw,   ARG,   0,  235)  \
  X(mullwo,  ARG,   1,  235)  \
  X(neg,     NONE,  0,  104)  \
  X(nego,    NONE,  1,  104)  \
  X(subf,    ARG,   0,  40)   \
  X(subfo,   ARG,   1,  40)   \
  X(subfc,   ARG,   0,  8)    \
  X(subfco,  ARG,   1,  8)    \
  X(subfe,   ARG,   0,  136)  \
  X(subfeo,  ARG,   1,  136)  \
  X(subfme,  NONE,  0,  232)  \
  X(subfmeo, NONE,  1,  232)  \
  X(subfze,  NONE,  0,  200)  \
  X(subfzeo, NONE,  1,  200)

/* Function header: XO1 */
#define HEADER_ARG  const Reg64& rb,
#define HEADER_NONE

#define XO1(name, arg3, oe, xop)                                          \
void Assembler::name(const Reg64& rt, const Reg64& ra, arg3 bool rc) {

/* Function body: XO2 */
#define BODY_ARG  rb
#define BODY_NONE 0

#define XO2(name, arg3, oe, xop)                                          \
    EmitXOForm(31, rn(rt), rn(ra), rn(arg3), oe, xop, rc);                \
}

/* Macro expansion for function parts */
#define X(name, arg3, oe, xop)  \
      XO1(name, HEADER_##arg3,  oe, xop)  \
      XO2(name, BODY_##arg3,    oe, xop)
ADDS
#undef X

#undef HEADER_ARG
#undef HEADER_NONE

#undef BODY_ARG
#undef BODY_NONE

#undef ADDS

void Assembler::addi(const Reg64& rt, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(14, rn(rt), rn(ra), imm.w());
}

void Assembler::addic(const Reg64& rt, const Reg64& ra, uint16_t imm, bool rc) {
  EmitDForm(12 + (uint8_t) rc, rn(rt), rn(ra), imm);
}

void Assembler::addis(const Reg64& rt, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(15, rn(rt), rn(ra), imm.w());
}

void Assembler::and(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 28, rc);
}

void Assembler::andc(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 60, rc);
}

void Assembler::andi(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(28, rn(rs), rn(ra), imm.w());
}

void Assembler::andis(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(29, rn(rs), rn(ra), imm.w());
}

void Assembler::b(int32_t offset) {
  EmitIForm(18, uint32_t(offset));
}

void Assembler::ba(uint32_t target_addr) {
  EmitIForm(18, target_addr, 1, 0);
}

void Assembler::bl(int32_t offset) {
  EmitIForm(18, uint32_t(offset), 0, 1);
}

void Assembler::bla(uint32_t target_addr) {
  EmitIForm(18, target_addr, 1, 1);
}

void Assembler::bc(uint8_t bo, uint8_t bi, int16_t offset) {
  EmitBForm(16, bo, bi, uint32_t(offset), 0, 0);
}

void Assembler::bca(uint8_t bo, uint8_t bi, uint16_t target_addr) {
  EmitBForm(16, bo, bi, target_addr, 1, 0);
}

void Assembler::bcctr(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 528);
}

void Assembler::bcctrl(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 528, 1);
}

void Assembler::bcl(uint8_t bo, uint8_t bi, int16_t offset) {
  EmitBForm(16, bo, bi, offset, 0, 1);
}

void Assembler::bcla(uint8_t bo, uint8_t bi, uint16_t target_addr) {
  EmitBForm(16, bo, bi, target_addr, 1, 1);
}

void Assembler::bclr(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 16, 0);
}

void Assembler::bclrl(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 16, 1);
}

void Assembler::bctar(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 560, 0);
}

void Assembler::bctarl(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 560, 1);
}

void Assembler::bpermd(const Reg64& ra, const Reg64& rs, const Reg64& rv) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 252);
}

void Assembler::cmp(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn((bf+(uint16_t)l) & 0x1d), rn(ra), rn(rb), 0);
}

void Assembler::cmpi(uint16_t bf, bool l, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(11, rn((bf+(uint16_t)l) & 0x1d), rn(ra), imm.w());
}

void Assembler::cmpb(const Reg64& rs, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 508);
}

void Assembler::cmpl(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn((bf+(uint16_t)l) & 0x1d), rn(ra), rn(rb), 32);
}

void Assembler::cmpli(uint16_t bf, bool l, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(10, rn((bf+(uint16_t)l) & 0x1d), rn(ra), imm.w());
}

void Assembler::cntlzd(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 26, rc);
}
void Assembler::cntlzw(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 58, rc);
}

void Assembler::crand(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 257);
}

void Assembler::crandc(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 129);
}

void Assembler::creqv(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 289);
}

void Assembler::crnand(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 225);
}

void Assembler::crnor(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 33);
}

void Assembler::cror(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 449);
}

void Assembler::crorc(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 417);
}

void Assembler::crxor(uint16_t bt, uint16_t ba, uint16_t bb) {
  EmitXLForm(19, bt, ba, bb, 193);
}

void Assembler::eqv(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 284, rc);
}

void Assembler::extsb(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 954, rc);
}

void Assembler::extsh(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 922);
}

void Assembler::extsw(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 986);
}

void Assembler::isel(const Reg64& rt, const Reg64& ra, const Reg64& rb,
                     uint8_t bc) {
  EmitAForm(31, rn(rt), rn(ra), rn(rb), rn(bc), 15);
}

void Assembler::lbz(const Reg64& rt, MemoryRef m) {
  EmitDForm(34, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lbzu(const Reg64& rt, MemoryRef m) {
  EmitDForm(35, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lbzux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 119);
}

void Assembler::lbzx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 87);
}

void Assembler::ld(const Reg64& rt, MemoryRef m) {
  EmitDSForm(58, rn(rt), rn(m.r.base), m.r.disp, 0);
}

void Assembler::ldbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 532);
}

void Assembler::ldu(const Reg64& rt, MemoryRef m) {
  EmitDSForm(58, rn(rt), rn(m.r.base), m.r.disp, 1);
}

void Assembler::ldux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 53);
}

void Assembler::ldx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 21);
}

void Assembler::lhbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 790);
}

void Assembler::lhz(const Reg64& rt, MemoryRef m) {
  EmitDForm(40, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lhzu(const Reg64& rt, MemoryRef m) {
  EmitDForm(41, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lhzux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 331);
}

void Assembler::lhzx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 279);
}

void Assembler::lha(const Reg64& rt, MemoryRef m) {
  EmitDForm(42, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lhau(const Reg64& rt, MemoryRef m) {
  EmitDForm(43, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lhaux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 375);
}

void Assembler::lhax(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 343);
}

void Assembler::lmw(const Reg64& rt, MemoryRef m) {
  EmitDForm(46, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lq(const Reg64& rtp, MemoryRef m) {
  //assert invalid instruction form
  assert(rn(rtp) == rn(m.r.base));
  EmitDQForm(56, rn(rtp), rn(m.r.base), m.r.disp);
}

void Assembler::lswi(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 597);
}

void Assembler::lswx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 533);
}

void Assembler::lwz(const Reg64& rt, MemoryRef m) {
  EmitDForm(32, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lwzu(const Reg64& rt, MemoryRef m) {
  EmitDForm(33, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lwzux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 55);
}

void Assembler::lwzx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 23);
}

void Assembler::lwa(const Reg64& rt, MemoryRef m) {
  EmitDSForm(58, rn(rt), rn(m.r.base), m.r.disp, 2);
}

void Assembler::lwaux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 373);
}

void Assembler::lwax(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 341);
}

void Assembler::lwbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 534);
}

void Assembler::mcrf(uint16_t bf, uint16_t bfa) {
  EmitXLForm(19, (bf & 0x1c), (bfa & 0x1c), 0, 0);
}

void Assembler::mfspr(const SpecialReg spr, const Reg64& rs) {
  EmitXFXForm(31, rn(rs), spr, 339);
}

void Assembler::mtspr(const SpecialReg spr, const Reg64& rs) {
  EmitXFXForm(31, rn(rs), spr, 467);
}

void Assembler::mulli(const Reg64& rt, const Reg64& ra, uint16_t imm) {
  EmitDForm(7, rn(rt), rn(ra), imm);
}

void Assembler::nand(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 476, rc);
}

void Assembler::nor(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 124, rc);
}

void Assembler::or(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 444, rc);
}

void Assembler::orc(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 412, rc);
}

void Assembler::ori(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(24, rn(rs), rn(ra), imm.w());
}

void Assembler::oris(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(25, rn(rs), rn(ra), imm.w());
}

void Assembler::popcntb(const Reg64& ra, const Reg64& rs) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 122);
}

void Assembler::popcntd(const Reg64& ra, const Reg64& rs) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 439);
}

void Assembler::popcntw(const Reg64& ra, const Reg64& rs) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 356);
}

void Assembler::prtyd(const Reg64& ra, const Reg64& rs) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 186);
}

void Assembler::prtyw(const Reg64& ra, const Reg64& rs) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 154);
}

void Assembler::rldcl(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                      uint8_t mb, bool rc) {
  EmitMDSForm(30, rn(rs), rn(ra), rn(rb), mb, 8, rc);
}

void Assembler::rldcr(const Reg64& ra, const Reg64& rs,  const Reg64& rb,
                      uint8_t mb, bool rc) {
  EmitMDSForm(30, rn(rs), rn(ra), rn(rb), mb, 9, rc);
}

void Assembler::rldic(const Reg64& ra, const Reg64& rs, uint8_t sh,
                      uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 2, rc);
}

void Assembler::rldicl(const Reg64& ra, const Reg64& rs, uint8_t sh,
                       uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 0, rc);
}

void Assembler::rldicr(const Reg64& ra, const Reg64& rs, uint8_t sh,
                       uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 1, rc);
}

void Assembler::rldimi(const Reg64& ra, const Reg64& rs, uint8_t sh,
                       uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 3, rc);
}

void Assembler::rlwimi(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
                       uint16_t me, bool rc) {
  EmitMForm(20, rn(rs), rn(ra), rn(sh), mb, me, rc);
}

void Assembler::rlwinm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
                       uint16_t me, bool rc) {
  EmitMForm(21, rn(rs), rn(ra), rn(sh), mb, me, rc);
}

void Assembler::rlwnm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
                      uint16_t me, bool rc) {
  EmitMForm(23, rn(rs), rn(ra), rn(sh), mb, me, rc);
}

void Assembler::sc(uint16_t lev) {
  EmitSCForm(17, (lev & 0x1));
}

void Assembler::sld(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 27, rc);
}

void Assembler::slw(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 24, rc);
}

void Assembler::srad(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 794, rc);
}

void Assembler::sradi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc) {
  EmitXSForm(31, rn(rs), rn(ra), sh, 413, rc);
}

void Assembler::sraw(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 792, rc);
}

void Assembler::srawi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(sh), 824, rc);
}

void Assembler::srd(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 539, rc);
}

void Assembler::srw(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 536, rc);
}

  void Assembler::stb(const Reg64& rt, MemoryRef m) {
  EmitDForm(38, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stbu(const Reg64& rt, MemoryRef m) {
  EmitDForm(39, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stbux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 247);
}

void Assembler::stbx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 215);
}

void Assembler::sth(const Reg64& rt, MemoryRef m) {
  EmitDForm(44, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::sthu(const Reg64& rt, MemoryRef m) {
  EmitDForm(45, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::sthux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 439);
}

void Assembler::sthx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 407);
}

void Assembler::stw(const Reg64& rt, MemoryRef m) {
  EmitDForm(36, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stwu(const Reg64& rt, MemoryRef m) {
  EmitDForm(37, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stwux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 183);
}

void Assembler::stwx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 151);
}

void Assembler::std(const Reg64& rt, MemoryRef m) {
  EmitDSForm(62, rn(rt), rn(m.r.base), m.r.disp, 0);
}

void Assembler::stdu(const Reg64& rt, MemoryRef m) {
  EmitDSForm(62, rn(rt), rn(m.r.base), m.r.disp, 1);
}

void Assembler::stdux(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 181);
}

void Assembler::stdx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 149);
}

void Assembler::stq(const Reg64& rt, MemoryRef m) {
  EmitDSForm(62, rn(rt), rn(m.r.base), m.r.disp, 2);
}

void Assembler::sthbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 918);
}

void Assembler::stwbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 662);
}

void Assembler::stdbrx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 660);
}

void Assembler::stmw(const Reg64& rt, MemoryRef m) {
  EmitDForm(47, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stswi(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 725);
}

void Assembler::stswx(const Reg64& rt, MemoryRef m) {
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 661);
}
void Assembler::subfic(const Reg64& rt, const Reg64& ra,  uint16_t imm) {
  EmitDForm(8, rn(rt), rn(ra), imm);
}

void Assembler::td(uint16_t to, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(to), rn(ra), rn(rb), 68);
}

void Assembler::tdi(uint16_t to, const Reg64& ra, uint16_t imm) {
  EmitDForm(2, rn(to), rn(ra), imm);
}

void Assembler::tw(uint16_t to, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(to), rn(ra), rn(rb), 4);
}

void Assembler::twi(uint16_t to, const Reg64& ra, uint16_t imm) {
  EmitDForm(3, rn(to), rn(ra), imm);
}

void Assembler::xor(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 316, rc);
}

void Assembler::xori(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(26, rn(rs), rn(ra), imm.w());
}

void Assembler::xoris(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(27, rn(rs), rn(ra), imm.w());
}

/* Floating point operations */
void Assembler::fadd(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 21, rc);
}

void Assembler::fsub(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 20, rc);
}

void Assembler::fmul(const RegXMM& frt, const RegXMM& fra, const RegXMM& frc,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(0), rn(frc), 25, rc);
}

void Assembler::fdiv(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 18, rc);
}


void Assembler::unimplemented(){
  //Emit a instruction with invalid opcode 0x0
  EmitDForm(0, rn(0), rn(0), 0);
}

// Create prologue when calling
void Assembler::prologue (const Reg64& rsp,
                          const Reg64& rfuncln,
                          const Reg64& rvmfp) {
  mflr(rfuncln);
  std(rfuncln, rsp[lr_position_on_callstack]);
  // Carry the rvmfp to VM stack around on all vasm calls.
  std(rvmfp, rsp[-min_callstack_size]);
  addi(rsp, rsp, -min_callstack_size);
}

// Create epilogue when calling.
void Assembler::epilogue (const Reg64& rsp, const Reg64& rfuncln) {
  // pop caller's return address.
  addi(rsp, rsp, min_callstack_size);
  ld(rfuncln, rsp[lr_position_on_callstack]);
  mtlr(rfuncln);
}

void Assembler::call (const Reg64& rsp,
                      const Reg64& rfuncln,
                      const Reg64& rvmfp,
                      CodeAddress target) {

  prologue(rsp, rfuncln, rvmfp);
  branchAuto(target, BranchConditions::Always, LinkReg::Save);
  epilogue(rsp, rfuncln);
}

void Assembler::li64 (const Reg64& rt, int64_t imm64) {
  // li64 always emits 5 instructions i.e. 20 bytes of instructions.
  // Assumes that 0 bytes will be missing in the end.
  uint8_t missing = 0;

  // for assert purposes
  DEBUG_ONLY CodeAddress li64StartPos = frontier();

  if (HPHP::jit::deltaFits(imm64, HPHP::sz::word)) {
    // immediate has only low 16 bits set, use simple load immediate
    li(rt, static_cast<int16_t>(imm64));
    if (imm64 & (1ULL << 15) && !(imm64 & (1ULL << 16))) {
      // clear extended sign that should not be set
      // (32bits number. Sets the 16th bit but not the 17th, it's not negative!)
      clrldi(rt, rt, 48);
      missing = kLi64InstrLen - 2 * kBytesPerInstr;
    } else {
      missing = kLi64InstrLen - 1 * kBytesPerInstr;
    }
  } else if (HPHP::jit::deltaFits(imm64, HPHP::sz::dword)) {
    // immediate has only low 32 bits set
    lis(rt, static_cast<int16_t>(imm64 >> 16));
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
    if (imm64 & (1ULL << 31) && !(imm64 & (1ULL << 32))) {
      // clear extended sign
      // (64bits number. Sets the 32th bit but not the 33th, it's not negative!)
      clrldi(rt, rt, 32);
      missing = kLi64InstrLen - 3 * kBytesPerInstr;
    } else {
      missing = kLi64InstrLen - 2 * kBytesPerInstr;
    }
  } else if (imm64 >> 48 == 0) {
    // immediate has only low 48 bits set
    lis(rt, static_cast<int16_t>(imm64 >> 32));
    ori(rt, rt, static_cast<int16_t>((imm64 >> 16) & UINT16_MAX));
    sldi(rt,rt,16);
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
    if (imm64 & (1ULL << 47)) {
      // clear extended sign
      clrldi(rt, rt, 16);
    } else {
      missing = kLi64InstrLen - 4 * kBytesPerInstr;
    }
  } else {
    // load all 64 bits
    lis(rt, static_cast<int16_t>(imm64 >> 48));
    ori(rt, rt, static_cast<int16_t>((imm64 >> 32) & UINT16_MAX));
    sldi(rt,rt,32);
    oris(rt, rt, static_cast<int16_t>((imm64 >> 16) & UINT16_MAX));
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
  }
  emitNop(missing);

  // guarantee our math with kLi64InstrLen is working
  assert(kLi64InstrLen == frontier() - li64StartPos);
}

int64_t Assembler::getLi64(PPC64Instr* pinstr) {
  // @pinstr should be pointing to the beginning of the li64 block
  //
  // It's easier to know how many 16bits of data the immediate uses
  // by counting how many nops there are inside of the code

  uint8_t nops = [&]() {
    // TODO(gut) use Decoder, but for now, do it hardcoded
    auto isNop = [&](PPC64Instr instr) -> bool {
      D_form_t d_formater {0, 0, 0, 24 }; // check Assembler::ori
      return instr == d_formater.instruction;
    };

    uint8_t nNops = 0;
    for (PPC64Instr* i = pinstr; i < pinstr + kLi64InstrLen/kBytesPerInstr;
        i++) {
      nNops += isNop(*i) ? 1 : 0;
    }
    return nNops;
  }();

  // TODO(gut) use Decoder, but for now, do it hardcoded
  auto hasClearSignBit = [&](PPC64Instr* instr) -> bool {
    bool op = (*instr >> 26) == 30;         // check opcode
    bool xop = ((*instr >> 2) & 0x7) == 0;  // check extended opcode
    return op && xop;
  };

  // TODO(gut) use Decoder, but for now, do it hardcoded
  auto getImm = [&](PPC64Instr* instr) -> uint16_t {
    return *instr & UINT16_MAX;
  };

  uint8_t immParts = 0;
  switch (nops) {
    case 4:
      immParts = 1;                                   // 16bits, sign bit = 0
      break;
    case 3:
      if (hasClearSignBit(pinstr + 1)) immParts = 1;  // 16bits, sign bit = 1
      else immParts = 2;                              // 32bits, sign bit = 0
      break;
    case 2:
      immParts = 2;                                   // 32bits, sign bit = 1
      break;
    case 1:
      immParts = 3;                                   // 48bits, sign bit = 0
      break;
    case 0:
      if (hasClearSignBit(pinstr + 4)) immParts = 3;  // 48bits, sign bit = 1
      else immParts = 4;                              // 64bits, sign bit = 0
      break;
    default:
      assert(false && "Unexpected number of nops in getLi64");
      break;
  }

  // first getImm is suppose to get the sign
  uint64_t imm64 = static_cast<uint64_t>(
                        static_cast<int16_t>(getImm(pinstr)));
  switch (immParts) {
    case 1:
      break;
    case 2:
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 1);
      break;
    case 3:
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 1);
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 3);  // jumps the sldi
      break;
    case 4:
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 1);
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 3);  // jumps the sldi
      imm64 <<= 16;
      imm64 |= getImm(pinstr + 4);
      break;
    default:
      assert(false && "No immediate detected on getLi64");
      break;
  }

  return static_cast<int64_t>(imm64);
}

Reg64 Assembler::getLi64Reg(PPC64Instr* instr) {
  // First instruction is always either li or lis, both are D-form
  D_form_t d_instr;
  d_instr.instruction = *instr;
  return Reg64(d_instr.RT);
}

void Assembler::li32 (const Reg64& rt, int32_t imm32) {

  if (HPHP::jit::deltaFits(imm32, HPHP::sz::word)) {
    // immediate has only low 16 bits set, use simple load immediate
    li(rt, static_cast<int16_t>(imm32));
    if (imm32 & (1ULL << 15) && !(imm32 & (1ULL << 16))) {
      // clear extended sign that should not be set
      // (32bits number. Sets the 16th bit but not the 17th, it's not negative!)
      clrldi(rt, rt, 48);
    } else {
      emitNop(kBytesPerInstr); // emit nop for a balanced li32 with 2 instr
    }
  } else {
    // immediate has 32 bits set
    lis(rt, static_cast<int16_t>(imm32 >> 16));
    ori(rt, rt, static_cast<int16_t>(imm32 & UINT16_MAX));
  }
}

void Assembler::li32un (const Reg64& rt, uint32_t imm32) {
  xor(rt, rt, rt);
  if ((imm32 >> 16) == 0) {
    // immediate has only low 16 bits set, use simple load immediate
    ori(rt, rt, static_cast<int16_t>(imm32));
    emitNop(kBytesPerInstr); // emit nop for a balanced li32un with 3 instr
  } else {
    // immediate has 32 bits set
    oris(rt, rt, static_cast<int16_t>(imm32 >> 16));
    ori(rt, rt, static_cast<int16_t>(imm32 & UINT16_MAX));
  }
}

int32_t Assembler::getLi32(PPC64Instr* pinstr) {
  // @pinstr should be pointing to the beginning of the li32 block

  // TODO(gut) use Decoder, but for now, do it hardcoded
  auto getImm = [&](PPC64Instr* instr) -> uint32_t {
    return *instr & UINT16_MAX;
  };

  // if first instruction is a li, it's using 16bits only
  bool is_16b_only = [&](PPC64Instr instr) -> bool {
    return (instr >> 26) == 14;        // check opcode
  }(*pinstr);

  uint32_t imm32 = 0;
  if (is_16b_only) {
    imm32 |= static_cast<int16_t>(getImm(pinstr));
  } else {
    imm32 |= getImm(pinstr)     << 16;  // lis
    imm32 |= getImm(pinstr + 1);        // ori
  }
  return static_cast<int32_t>(imm32);
}
} // namespace ppc64_asm
