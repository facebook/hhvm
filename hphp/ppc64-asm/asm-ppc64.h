/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#ifndef incl_HPHP_PPC64_ASM_ASM_PPC64_H_
#define incl_HPHP_PPC64_ASM_ASM_PPC64_H_

#include <cstdint>
#include <cassert>
#include <vector>

#include "hphp/util/data-block.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/ppc64-asm/branch-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"
#include "hphp/ppc64-asm/isa-ppc64.h"

#include "hphp/runtime/base/runtime-option.h"


namespace ppc64_asm {

/* using override */ using HPHP::jit::Reg64;
/* using override */ using HPHP::jit::RegXMM;
/* using override */ using HPHP::jit::RegSF;
/* using override */ using HPHP::jit::MemoryRef;
/* using override */ using HPHP::jit::Immed;
/* using override */ using HPHP::CodeAddress;
/* using override */ using HPHP::jit::ConditionCode;

//////////////////////////////////////////////////////////////////////

/*
 * Constants definition for PPC64
 */

// Must be the same value of AROFF(_dummyB).
constexpr uint8_t min_frame_size            = 4 * 8;
// Must be the same value of AROFF(_savedToc).
constexpr uint8_t toc_position_on_frame     = 3 * 8;

// How many bytes a PPC64 instruction length is.
constexpr uint8_t instr_size_in_bytes       = sizeof(PPC64Instr);
// Amount of bytes to skip after an Assembler::call to grab the return address.
// Currently it skips a "nop" or a "ld 2,24(1)"
constexpr uint8_t call_skip_bytes_for_ret   = 1 * instr_size_in_bytes;


// Allow TOC usage on branches - disabled at the moment.
//#define USE_TOC_ON_BRANCH

//////////////////////////////////////////////////////////////////////

enum class RegNumber : uint32_t {};

namespace reg {
  constexpr Reg64 r0(0);    // volatile, used in function prologue / linkage
  constexpr Reg64 r1(1);    // nonvolatile, stack pointer
  constexpr Reg64 r2(2);    // nonvolatile, TOC
  /* volatile, argument passing registers */
  constexpr Reg64 r3(3);
  constexpr Reg64 r4(4);
  constexpr Reg64 r5(5);
  constexpr Reg64 r6(6);
  constexpr Reg64 r7(7);
  constexpr Reg64 r8(8);
  constexpr Reg64 r9(9);
  constexpr Reg64 r10(10);

  constexpr Reg64 r11(11);  // volatile, environment pointer (scratch)
  constexpr Reg64 r12(12);  // volatile, function entry address
  constexpr Reg64 r13(13);  // reserved, thread pointer
  /* nonvolatile, local variables */
  constexpr Reg64 r14(14);
  constexpr Reg64 r15(15);
  constexpr Reg64 r16(16);
  constexpr Reg64 r17(17);
  constexpr Reg64 r18(18);
  constexpr Reg64 r19(19);
  constexpr Reg64 r20(20);
  constexpr Reg64 r21(21);
  constexpr Reg64 r22(22);
  constexpr Reg64 r23(23);
  constexpr Reg64 r24(24);
  constexpr Reg64 r25(25);
  constexpr Reg64 r26(26);
  constexpr Reg64 r27(27);
  constexpr Reg64 r28(28);
  constexpr Reg64 r29(29);
  constexpr Reg64 r30(30);
  constexpr Reg64 r31(31);

  // RegXMM is used for both FP and SIMD registers. Registers from 0 to 15 are
  // reserved to FP and register from 16 to 29 are reserved to SIMD.
  // Ignoring the vector registers 30, 31 due to kMaxRegs == 64
  constexpr RegXMM f0(0);   // volatile scratch register
  /* volatile, argument passing floating point registers */
  constexpr RegXMM f1(1);
  constexpr RegXMM f2(2);
  constexpr RegXMM f3(3);
  constexpr RegXMM f4(4);
  constexpr RegXMM f5(5);
  constexpr RegXMM f6(6);
  constexpr RegXMM f7(7);
  constexpr RegXMM f8(8);
  constexpr RegXMM f9(9);
  constexpr RegXMM f10(10);
  constexpr RegXMM f11(11);
  constexpr RegXMM f12(12);
  constexpr RegXMM f13(13);
  /* nonvolatile, local variables */
  constexpr RegXMM f14(14);
  constexpr RegXMM f15(15);

  /* volatile, local variables */
  constexpr RegXMM v16(16);
  constexpr RegXMM v17(17);
  constexpr RegXMM v18(18);
  constexpr RegXMM v19(19);
  /* nonvolatile, local variables */
  constexpr RegXMM v20(20);
  constexpr RegXMM v21(21);
  constexpr RegXMM v22(22);
  constexpr RegXMM v23(23);
  constexpr RegXMM v24(24);
  constexpr RegXMM v25(25);
  constexpr RegXMM v26(26);
  constexpr RegXMM v27(27);
  constexpr RegXMM v28(28);
  constexpr RegXMM v29(29);
  constexpr RegXMM v30(30);
  constexpr RegXMM v31(31);

#define RNAME(x) if (r == x) return #x

  inline const char* regname(Reg64 r) {
    RNAME(r0);  RNAME(r1);  RNAME(r2);  RNAME(r3);  RNAME(r4);  RNAME(r5);
    RNAME(r6);  RNAME(r7);  RNAME(r8);  RNAME(r9);  RNAME(r10); RNAME(r11);
    RNAME(r12); RNAME(r13); RNAME(r14); RNAME(r15); RNAME(r16); RNAME(r17);
    RNAME(r18); RNAME(r19); RNAME(r20); RNAME(r21); RNAME(r22); RNAME(r23);
    RNAME(r24); RNAME(r25); RNAME(r26); RNAME(r27); RNAME(r28); RNAME(r29);
    RNAME(r30); RNAME(r31);
    return nullptr;
  }

  inline const char* regname(RegXMM r) {
    RNAME(f0);  RNAME(f1);  RNAME(f2);  RNAME(f3);  RNAME(f4);  RNAME(f5);
    RNAME(f6);  RNAME(f7);  RNAME(f8);  RNAME(f9);  RNAME(f10); RNAME(f11);
    RNAME(f12); RNAME(f13); RNAME(f14); RNAME(f15);

    RNAME(v16); RNAME(v17); RNAME(v18); RNAME(v19); RNAME(v20); RNAME(v21);
    RNAME(v22); RNAME(v23); RNAME(v24); RNAME(v25); RNAME(v26); RNAME(v27);
    RNAME(v28); RNAME(v29);
    return nullptr;
  }
 inline const char* regname(RegSF) {
    return "cr0";
 }
#undef RNAME
}

//////////////////////////////////////////////////////////////////////

// Forward declaration to be used in Label
struct Assembler;

enum class ImmType {
              // Supports TOC? | Supports li64? | Fixed size?
              // -------------------------------------------
  AnyCompact, // Yes           | Yes            | No
  AnyFixed,   // Yes           | Yes            | Yes (5 instr)
  TocOnly,    // Yes           | No             | Yes (2 instr)
};

enum class LinkReg {
  Save,
  DoNotTouch
};

struct Label {
  Label() : m_a(nullptr) , m_address(nullptr) {}
  /* implicit */ Label(CodeAddress predefined) : m_a(nullptr) ,
                                                 m_address(predefined) {}

  ~Label();

  Label(const Label&) = delete;
  Label& operator=(const Label&) = delete;

  void branch(Assembler& a, BranchConditions bc, LinkReg lr);
  void branchFar(Assembler& a, BranchConditions bc, LinkReg lr);
  void asm_label(Assembler& a);

  enum class BranchType {
    b,    // unconditional branch up to 26 bits offset
    bc,   // conditional branch up to 16 bits offset
    bctr  // conditional branch by using a 64 bits absolute address
  };

private:
  struct JumpInfo {
    BranchType type;
    Assembler* a;
    CodeAddress addr;
  };

  void addJump(Assembler* a, BranchType type);

  Assembler* m_a;
  CodeAddress m_address;
  std::vector<JumpInfo> m_toPatch;
};

/*
 * Class that represents a virtual TOC
 */
struct VMTOC {

private:
  // VMTOC is a singleton
  VMTOC()
    : m_tocvector(nullptr)
    , m_last_elem_pos(0)

  {}

  ~VMTOC();

public:
  VMTOC(VMTOC const&) = delete;

  VMTOC operator=(VMTOC const&) = delete;

  void setTOCDataBlock(HPHP::DataBlock *db);

  /*
   * Push a 64 bit element into the stack and return its index.
   */
  int64_t pushElem(int64_t elem);

  /*
   * Push a 32 bit element into the stack and return its index.
   */
  int64_t pushElem(int32_t elem);

  /*
   * Get the singleton instance.
   */
  static VMTOC& getInstance();

  /*
   * Return the address of the middle element from the vector.
   *
   * This is done so signed offsets can be used.
   */
  intptr_t getPtrVector();

  /*
   * Return a value previously pushed.
   */
  int64_t getValue(int64_t index, bool qword = false);

private:
  int64_t allocTOC (int32_t target, bool align = false);
  void forceAlignment(HPHP::Address& addr);

  HPHP::DataBlock *m_tocvector;

  /*
   * Vector position of the last element.
   */
  uint64_t m_last_elem_pos;

  /*
   * Map used to avoid insertion of duplicates.
   */
  std::map<int64_t, uint64_t> m_map;
};

struct Assembler {

  friend struct Label;

  explicit Assembler(HPHP::CodeBlock& cb) : codeBlock(cb) {}
  ~Assembler(){}

  HPHP::CodeBlock& code() const { return codeBlock; }

  CodeAddress base() const {
    return codeBlock.base();
  }

  CodeAddress frontier() const {
    return codeBlock.frontier();
  }

  void setFrontier(CodeAddress newFrontier) {
    codeBlock.setFrontier(newFrontier);
  }

  size_t capacity() const {
    return codeBlock.capacity();
  }

  size_t used() const {
    return codeBlock.used();
  }

  size_t available() const {
    return codeBlock.available();
  }

  bool contains(CodeAddress addr) const {
    return codeBlock.contains(addr);
  }

  bool empty() const {
    return codeBlock.empty();
  }

  void clear() {
    codeBlock.clear();
  }

  bool canEmit(size_t nBytes) const {
    assert(capacity() >= used());
    return nBytes < (capacity() - used());
  }

  enum class SpecialReg {
    XER      = 1,
    DSCR     = 3,
    LR       = 8,
    CTR      = 9,
    AMR      = 13,
    TFHAR    = 128,
    TFIAR    = 129,
    TEXASR   = 130,
    TEXASRU  = 131,
    VRSAVE   = 256,
    SPEFSCR  = 512,
    MMCR2    = 769,
    MMCRA    = 770,
    PMC1     = 771,
    PMC2     = 772,
    PMC3     = 773,
    PMC4     = 774,
    PMC5     = 775,
    PMC6     = 776,
    MMCR0    = 779,
    BESCRS   = 800,
    BESCRSU  = 801,
    BESCRR   = 802,
    BESCRRU  = 803,
    EBBHR    = 804,
    EBBRR    = 805,
    BESCR    = 806,
    TAR      = 815,
    PPR      = 896,
    PPR32    = 898
  };

  enum class CR {
    CR0      = 0,
    CR1      = 1,
    CR2      = 2,
    CR3      = 3,
    CR4      = 4,
    CR5      = 5,
    CR6      = 6,
    CR7      = 7,
  };

  // Prologue size of call function (mflr, std, std, addi, std)
  static const uint8_t kCallPrologueLen = instr_size_in_bytes * 5;

  // Epilogue size of call function after the return address.
  // ld, addi, ld, mtlr
  static const uint8_t kCallEpilogueLen = instr_size_in_bytes * 4;

  // Jcc length
  static const uint8_t kJccLen = instr_size_in_bytes * 9;

  // Call length prologue + jcc
  static const uint8_t kCallLen = instr_size_in_bytes * 9;

  // Total ammount of bytes that a li64 function emits
  static const uint8_t kLi64InstrLen = 5 * instr_size_in_bytes;

  // TODO(rcardoso): Must create a macro for these similar instructions.
  // This will make code more clean.

  // #define CC_ARITH_REG_OP(name, opcode, x_opcode)
  //   void name##c
  //   void name##co
  //   ...
  // #define CC_ARITH_IMM_OP(name, opcode)

  // #define LOAD_STORE_OP(name)
  // void #name(const Reg64& rt, MemoryRef m);
  // void #name##u(const Reg64& rt, MemoryRef m);
  // void #name##x(const Reg64& rt, MemoryRef m);
  // void #name##ux(const Reg64& rt, MemoryRef m);

  // #define LOAD_STORE_OP_BYTE_REVERSED(name)
  // void #name##brx(const Reg64& rt, MemoryRef m);

  // LOAD_STORE_OP(lbz)
  // LOAD_STORE_OP(lh)
  // LOAD_STORE_OP(lha)
  // LOAD_STORE_OP_BYTE_REVERSED(lh)
  // LOAD_STORE_OP(lwz)
  // LOAD_STORE_OP(lwa)
  // LOAD_STORE_OP(ld)
  // LOAD_STORE_OP_BYTE_REVERSED(ld)
  // LOAD_STORE_OP(stb)
  // LOAD_STORE_OP(sth)
  // LOAD_STORE_OP_BYTE_REVERSED(sth)
  // LOAD_STORE_OP(stw)
  // LOAD_STORE_OP_BYTE_REVERSED(stw)
  // LOAD_STORE_OP(std)
  // LOAD_STORE_OP_BYTE_REVERSED(std)

  // #undef LOAD_STORE_OP
  // #undef LOAD_STORE_OP_BYTE_REVERSED

  //PPC64 Instructions
  void add(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void addi(const Reg64& rt, const Reg64& ra, Immed imm);
  void addis(const Reg64& rt, const Reg64& ra, Immed imm);
  void addo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void and(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void andi(const Reg64& ra, const Reg64& rs, Immed imm);
  void b(int32_t offset);
  void bl(int32_t offset);
  void bc(uint8_t bo, uint8_t bi, int16_t offset);
  void bcctr(uint8_t bo, uint8_t bi, uint16_t bh);
  void bctrl();
  void blr();
  void bctar(uint8_t bo, uint8_t bi, uint16_t bh);
  void bctarl(uint8_t bo, uint8_t bi, uint16_t bh);
  void cmp(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb);
  void cmpi(uint16_t bf, bool l, const Reg64& ra, Immed imm);
  void cmpb(const Reg64& rs, const Reg64& ra, const Reg64& rb);
  void cmpl(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb);
  void cmpli(uint16_t bf, bool l, const Reg64& ra, Immed imm);
  void divd(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void extsb(const Reg64& ra, const Reg64& rs, bool rc = 0);
  void extsh(const Reg64& ra, const Reg64& rs, bool rc = 0);
  void extsw(const Reg64& ra, const Reg64& rs, bool rc = 0);
  void fadd(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                                                                  bool rc = 0);
  void fsub(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                                                                  bool rc = 0);
  void fmul(const RegXMM& frt, const RegXMM& fra, const RegXMM& frc,
                                                                  bool rc = 0);
  void fdiv(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                                                                  bool rc = 0);
  void lfs(const RegXMM& frt, MemoryRef m) {
    assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
    EmitDForm(48, rn(frt), rn(m.r.base), m.r.disp);
  }
  void lxvd2x(const RegXMM& Xt, const MemoryRef& m) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXX1Form(31, rn(Xt), rn(m.r.base), rn(m.r.index), 844, 0);
  }
  void lxvw4x(const RegXMM& Xt, const MemoryRef& m) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXX1Form(31, rn(Xt), rn(m.r.base), rn(m.r.index), 780, 0);
  }
  void isel(const Reg64& rt, const Reg64& ra, const Reg64& rb, uint8_t bc);
  void lbz(const Reg64& rt, MemoryRef m);
  void lbzx(const Reg64& rt, MemoryRef m);
  void ld(const Reg64& rt, MemoryRef m);
  void ldx(const Reg64& rt, MemoryRef m);
  void lhz(const Reg64& rt, MemoryRef m);
  void lhzx(const Reg64& rt, MemoryRef m);
  void lwz(const Reg64& rt, MemoryRef m);
  void lwzx(const Reg64& rt, MemoryRef m);
  void mfspr(const SpecialReg spr, const Reg64& rs);
  void mtspr(const SpecialReg spr, const Reg64& rs);
  void mulldo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void neg(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void nor(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void or(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void ori(const Reg64& ra, const Reg64& rs, Immed imm);
  void oris(const Reg64& ra, const Reg64& rs, Immed imm);
  void rldicl(const Reg64& ra, const Reg64& rs, uint8_t sh,
              uint8_t mb, bool rc = 0);
  void rldicr(const Reg64& ra, const Reg64& rs, uint8_t sh,
              uint8_t mb, bool rc = 0);
  void rlwinm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
              uint16_t me, bool rc = 0);
  void sld(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void srad(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void stb(const Reg64& rs, MemoryRef m);
  void stbx(const Reg64& rs, MemoryRef m);
  void stfs(const RegXMM& frt, MemoryRef m) {
    EmitDForm(52, rn(frt), rn(m.r.base), m.r.disp);
  }
  void sth(const Reg64& rs, MemoryRef m);
  void sthx(const Reg64& rs, MemoryRef m);
  void stw(const Reg64& rs, MemoryRef m);
  void stwx(const Reg64& rs, MemoryRef m);
  void std(const Reg64& rs, MemoryRef m);
  void stdu(const Reg64& rs, MemoryRef m);
  void stdx(const Reg64& rs, MemoryRef m);
  void stxvw4x(const RegXMM& xs, const MemoryRef& m) {
    EmitXX1Form(31, rn(xs), rn(m.r.base), rn(m.r.index), 972, 0);
  }
  void subf(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void subfo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void td(uint16_t to, const Reg64& ra, const Reg64& rb);
  void tw(uint16_t to, const Reg64& ra, const Reg64& rb);
  void xor(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void xscvdpuxds(const RegXMM& xt, const RegXMM& xb) {
    //TODO(rcardoso): bx tx bits
    EmitXX2Form(60, rn(xt), 0, rn(xb), 328, 0, 0);
  }
  void xvdivdp(const RegXMM& xt,  const RegXMM& xa, const RegXMM& xb) {
    EmitXX3Form(60, rn(xt), rn(xa), rn(xb), 56,  0, 0 ,0);
  }
  void xvdivsp(const RegXMM& xt,  const RegXMM& xa, const RegXMM& xb) {
    EmitXX3Form(60, rn(xt), rn(xa), rn(xb), 24, 0, 0 ,0);
  }

  // Unimplemented Instructions
  void dcmpu(const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(59, rn(0), rn(fra), rn(frb), 642);
  }
  void fabs(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 264, rc);
  }
  void fcfid(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 846, rc);
  }

  void fcfids(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(59, rn(frt), rn(0), rn(frb), 846, rc);
  }
  void fcmpo(const RegSF& sf, const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(63, rn(int(sf) << 2), rn(fra), rn(frb), 32);
  }
  void fcmpu(const RegSF& sf, const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(63, rn(int(sf) << 2), rn(fra), rn(frb), 0);
  }
  void fctid(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 814, rc);
  }
  void fctidz(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 815, rc);
  }
  void fmr(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 72, rc);
  }
  //TODO(rcardoso); check default for EH bit
  void ldarx(const Reg64& rt, MemoryRef m, bool eh = 1) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 84, eh);
  }
  void lfd(const RegXMM& frt, MemoryRef m) {
    assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
    EmitDForm(50, rn(frt), rn(m.r.base), m.r.disp);
  }
  void lfdx(const RegXMM& rt, MemoryRef m) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 599);
  }
  void mcrfs(uint8_t bf, uint8_t bfa) {
    EmitXForm(63, (bf << 2), (bfa << 2), 0, 64);
  }
  void mfcr(const Reg64& rt) {
    EmitXFXForm(31, rn(rt), static_cast<SpecialReg>(0), 19);
  }
  void mfvsrd(const Reg64& ra, const RegXMM& xs) {
   EmitXX1Form(31, rn(xs), rn(ra), rn(0) /* reserved */, 51, 0);
  }
  void mtcrf(uint16_t fxm, const Reg64& ra) {
    EmitXFXForm(31, rn(ra), (fxm << 1), 144);
  }
  void mtocrf(uint16_t fxm, const Reg64& rt)          {
    EmitXFXForm(31, rn(rt), ( ((fxm << 1) & 0x1fe) |0x200), 144);
  }
  void mtfsb0(uint8_t bt) { EmitXForm(63, bt, 0 , 0, 70); }
  void mtvsrd(const RegXMM& xt, const Reg64& ra) {
   EmitXX1Form(31, rn(xt), rn(ra), rn(0) /* reserved */, 179, 0);
  }
  void sradi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc = 0);
  void stdcx(const Reg64& rt, MemoryRef m) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 214, 1);
  }
  void stfd(const RegXMM& frt, MemoryRef m) {
    assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
    EmitDForm(54, rn(frt), rn(m.r.base), m.r.disp);
  }
  void stfdx(const RegXMM& rt, MemoryRef m) {
    assertx(!m.r.disp);  // doesn't support immediate displacement
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 727);
  }
  void xscvdpsxds(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 344, 0, 0);
  }
  void xscvsxddp(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 376, 0, 0);
  }
  void xsrdpi(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 73, 0, 0);
  }
  void xssqrtdp(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 75, 0, 0);
  }
  void xvcvspsxds(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 408, 0, 0);
  }
  void xvcvspsxws(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 152, 0, 0);
  }
  void xxlxor(const RegXMM& xt, const RegXMM& xa, const RegXMM& xb) {
   EmitXX3Form(60, rn(xt), rn(xa), rn(xb), 154, 0, 0, 0);
  }
  void xxpermdi(const RegXMM& tx, const RegXMM& xa, const RegXMM& xb) {
   EmitXX3Form(60, rn(tx), rn(xa), rn(xb),  10, 0, 0, 0);
   // Note that I decided to hardcode DM bit as 0
   // (xo field = 10), because it's sufficent for now.
   // However, I might not be the case in the future
  }

  //Extended/Synthetic PPC64 Instructions
  void bctr() {
    BranchParams bp(BranchConditions::Always);
    bcctr(bp.bo(), bp.bi(), 0);
  }
  void li(const Reg64& rt, Immed imm) {
    addi(rt, Reg64(0), imm);
  }
  void subi(const Reg64& rt, const Reg64& ra, Immed imm) {
    addi(rt, ra, -imm);
  }
  void lis(const Reg64& rt, Immed imm) {
    addis(rt, Reg64(0), imm);
  }
  void sub(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0) {
    subf(rt, rb, ra, rc);
  }
  void subo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0) {
    subfo(rt, rb, ra, rc);
  }
  void cmpdi(const Reg64& ra, Immed imm) {
    cmpi(0, 1, ra, imm);
  }
  void cmpwi(const Reg64& ra, Immed imm) {
    //Extended cmpi 3,0,Rx,value
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmpi(0, 0, ra, imm);
  }
  void cmpd(const Reg64& ra, const Reg64& rb) {
    cmp(0, 1, ra, rb);
  }
  void cmpw(const Reg64& ra, const Reg64& rb) {
    //Extended cmp 3,0,Rx,Ry
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmp(0, 0, ra, rb);
  }
  void cmpldi(const Reg64& ra, Immed imm, CR CRnum = CR::CR0) {
    cmpli(static_cast<uint16_t>(CRnum), 1, ra, imm);
  }
  void cmplwi(const Reg64& ra, Immed imm, CR CRnum = CR::CR0) {
    //Extended cmpli 3,0,Rx,value
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmpli(static_cast<uint16_t>(CRnum), 0, ra, imm);
  }
  void cmpld(const Reg64& ra, const Reg64& rb, CR CRnum = CR::CR0) {
    cmpl(static_cast<uint16_t>(CRnum), 1, ra, rb);
  }
  void cmplw(const Reg64& ra, const Reg64& rb, CR CRnum = CR::CR0) {
    //Extended cmpl 3,0,Rx,Ry
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmpl(static_cast<uint16_t>(CRnum), 0, ra, rb);
  }
  void trap() {
    tw(31, Reg64(0), Reg64(0));
  }
  void nop() {
    ori(Reg64(0),Reg64(0),0);
  }
  void mr(const Reg64& rs, const Reg64& ra) {
    or(rs, ra, ra);
  }
  void srwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rlwinm(ra, rs, 32-sh, sh, 31, rc);
  }
  void slwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    /* non-existing mnemonic on ISA, but it's pratical to have it here */
    rlwinm(ra, rs, sh, 0, 31-sh, rc);
  }
  void srdi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicl(ra, rs, 64-sh, sh, rc);
  }
  void clrldi(const Reg64& ra, const Reg64& rs, int8_t mb, bool rc = 0) {
    rldicl(ra, rs, 0, mb, rc);
  }
  void sldi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicr(ra, rs, sh, 63-sh, rc);
  }
  void clrrdi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicr(ra, rs, 0, 63-sh, rc);
  }
  void clrrwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rlwinm(ra, rs, 0, 0, 31-sh, rc);
  }
  void mtctr(const Reg64& rx) {
    mtspr(SpecialReg::CTR, rx);
  }
  void mtlr(const Reg64& rx) {
    mtspr(SpecialReg::LR, rx);
  }
  void mfctr(const Reg64& rx) {
    mfspr(SpecialReg::CTR, rx);
  }
  void mflr(const Reg64& rx) {
    mfspr(SpecialReg::LR, rx);
  }

  // Label variants

  // Simplify to conditional branch that always branch
  void b(Label& l)  { bc(l, BranchConditions::Always); }
  void bl(Label& l)  { bcl(l, BranchConditions::Always); }

  void bc(Label& l, BranchConditions bc) {
    l.branch(*this, bc, LinkReg::DoNotTouch);
  }
  void bc(Label& l, ConditionCode cc) {
    l.branch(*this, BranchParams::convertCC(cc), LinkReg::DoNotTouch);
  }
  void bcl(Label& l, BranchConditions bc) {
    l.branch(*this, bc, LinkReg::Save);
  }

  void branchAuto(Label& l,
                  BranchConditions bc = BranchConditions::Always,
                  LinkReg lr = LinkReg::DoNotTouch) {
    l.branch(*this, bc, lr);
  }

  void branchAuto(CodeAddress c,
                  BranchConditions bc = BranchConditions::Always,
                  LinkReg lr = LinkReg::DoNotTouch) {
    Label l(c);
    l.branch(*this, bc, lr);
  }

  void branchAuto(CodeAddress c,
                  ConditionCode cc,
                  LinkReg lr = LinkReg::DoNotTouch) {
    branchAuto(c, BranchParams::convertCC(cc), lr);
  }

  void branchFar(Label& l,
                 BranchConditions bc = BranchConditions::Always,
                 LinkReg lr = LinkReg::DoNotTouch) {
    l.branchFar(*this, bc, lr);
  }

  void branchFar(CodeAddress c,
                 BranchConditions bc = BranchConditions::Always,
                 LinkReg lr = LinkReg::DoNotTouch) {
    Label l(c);
    l.branchFar(*this, bc, lr);
  }

  void branchFar(CodeAddress c,
                 ConditionCode cc,
                 LinkReg lr = LinkReg::DoNotTouch) {
    branchFar(c, BranchParams::convertCC(cc), lr);
  }

  // ConditionCode variants
  void bc(ConditionCode cc, int16_t address) {
    BranchParams bp(cc);
    bc(bp.bo(), bp.bi(), address);
  }

//////////////////////////////////////////////////////////////////////

  enum class CallArg {
                // Saves TOC?    | Smashable?
                // --------------------------
    Internal,   // No            | No
    External,   // Yes           | No
    Smashable,  // No (internal) | Yes
  };

  void callEpilogue(CallArg ca) {
    // Several vasms like nothrow, unwind and syncpoint will skip one
    // instruction after call and use it as expected return address. Use a nop
    // to guarantee this consistency even if toc doesn't need to be saved
    if (CallArg::External != ca) nop();
    else ld(reg::r2, reg::r1[toc_position_on_frame]);
  }

  // generic template, for CodeAddress and Label
  template <typename T>
  void call(T& target, CallArg ca = CallArg::Internal) {
    if (CallArg::Smashable == ca) {
      // To make a branch smashable, the most conservative method needs to be
      // used so the target can be changed later or on bindCall.
      branchFar(target, BranchConditions::Always, LinkReg::Save);
    } else {
      // tries best performance possible
      branchAuto(target, BranchConditions::Always, LinkReg::Save);
    }
    callEpilogue(ca);
  }

  // specialization of call for Reg64
  void call(Reg64 target, CallArg ca = CallArg::Internal) {
    mr(reg::r12, target);
    mtctr(reg::r12);
    bctrl();
    callEpilogue(ca);
  }

  // checks if the @inst is pointing to a call
  static inline bool isCall(HPHP::jit::TCA inst) {
    DecodedInstruction di(inst);
    return di.isCall();
  }

//////////////////////////////////////////////////////////////////////
// Auxiliary for loading immediates in the best way

private:
  void loadTOC(const Reg64& rt, const Reg64& rttoc, int64_t imm64,
      uint64_t offset, bool fixedSize, bool fits32);

public:
  void limmediate(const Reg64& rt,
                  int64_t imm64,
                  ImmType immt = ImmType::AnyCompact);

  // Auxiliary for loading a complete 64bits immediate into a register
  void li64(const Reg64& rt, int64_t imm64, bool fixedSize = true);

  // Retrieve the target defined by li64 instruction
  static int64_t getLi64(PPC64Instr* pinstr);
  static int64_t getLi64(CodeAddress pinstr) {
    return getLi64(reinterpret_cast<PPC64Instr*>(pinstr));
  }

  // Retrieve the register used by li64 instruction
  static Reg64 getLi64Reg(PPC64Instr* instr);
  static Reg64 getLi64Reg(CodeAddress instr) {
    return getLi64Reg(reinterpret_cast<PPC64Instr*>(instr));
  }

  // Auxiliary for loading a 32bits immediate into a register
  void li32 (const Reg64& rt, int32_t imm32);

  // Retrieve the target defined by li32 instruction
  static int32_t getLi32(PPC64Instr* pinstr);
  static int32_t getLi32(CodeAddress pinstr) {
    return getLi32(reinterpret_cast<PPC64Instr*>(pinstr));
  }

  // Retrieve the register used by li32 instruction
  static Reg64 getLi32Reg(PPC64Instr* instr) {
    // it also starts with li or lis, so the same as getLi64
    return getLi64Reg(instr);
  }
  static Reg64 getLi32Reg(CodeAddress instr) {
    return getLi32Reg(reinterpret_cast<PPC64Instr*>(instr));
  }

  void emitNop(int nbytes) {
    assert((nbytes % 4 == 0) && "This arch supports only 4 bytes alignment");
    for (; nbytes > 0; nbytes -= 4) nop();
  }

  // Secure high level instruction emitter
  void Emit(PPC64Instr instruction){
    assert(codeBlock.canEmit(sizeof(instruction)));
    assert(sizeof(instruction) == sizeof(uint32_t));
    dword(instruction);
  }

  // Can be used to generate or force a unimplemented opcode exception
  void unimplemented();

//////////////////////////////////////////////////////////////////////

  /*
   * Patch a branch to the correct target.
   *
   * It decodes the branch @jmp to decide whether it's an absolute branch or an
   * offset branch and patches it properly.
   */
  static void patchBranch(CodeAddress jmp, CodeAddress dest);

//////////////////////////////////////////////////////////////////////

protected:

  // type instruction emitters
  // TODO(rcardoso): try remove cast for uint32_t
  // TODO(rcardoso): make those functions inline
  void EmitXOForm(const uint8_t op,
                  const RegNumber rt,
                  const RegNumber ra,
                  const RegNumber rb,
                  const bool oe,
                  const uint16_t xop,
                  const bool rc = 0) {

    // GP Register cannot be greater than 31
    assert(static_cast<uint32_t>(rb) < 32);
    assert(static_cast<uint32_t>(ra) < 32);
    assert(static_cast<uint32_t>(rt) < 32);

    XO_form_t xo_formater {
                            rc,
                            xop,
                            oe,
                            static_cast<uint32_t>(rb),
                            static_cast<uint32_t>(ra),
                            static_cast<uint32_t>(rt),
                            op
                          };

    dword(xo_formater.instruction);
  }

  void EmitDForm(const uint8_t op,
                 const RegNumber rt,
                 const RegNumber ra,
                 const int16_t imm) {

    // GP Register cannot be greater than 31
    assert(static_cast<uint32_t>(rt) < 32);
    assert(static_cast<uint32_t>(ra) < 32);

    D_form_t d_formater {
                          static_cast<uint32_t>(imm),
                          static_cast<uint32_t>(ra),
                          static_cast<uint32_t>(rt),
                          op
                         };

    dword(d_formater.instruction);
  }

  void EmitIForm(const uint8_t op,
                 const uint32_t imm,
                 const bool aa = 0,
                 const bool lk = 0) {

      I_form_t i_formater {
                            lk,
                            aa,
                            imm >> 2,
                            op
                          };

      dword(i_formater.instruction);
  }

   void EmitBForm(const uint8_t op,
                  const uint8_t bo,
                  const uint8_t bi,
                  const uint32_t bd,
                  const bool aa = 0,
                  const bool lk = 0) {
      B_form_t b_formater {
                            lk,
                            aa,
                            bd >> 2,
                            bi,
                            bo,
                            op
                          };

       dword(b_formater.instruction);
   }

   void EmitSCForm(const uint8_t op,
                   const uint16_t lev) {
      SC_form_t sc_formater {
                              1,
                              lev,
                              op
                            };

      dword(sc_formater.instruction);
   }

   void EmitXForm(const uint8_t op,
                  const RegNumber rt,
                  const RegNumber ra,
                  const RegNumber rb,
                  const uint16_t xop,
                  const bool rc = 0){

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(rb) < 32);
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rt) < 32);

      X_form_t x_formater {
                            rc,
                            xop,
                            static_cast<uint32_t>(rb),
                            static_cast<uint32_t>(ra),
                            static_cast<uint32_t>(rt),
                            op
                          };

      dword(x_formater.instruction);
   }

   void EmitXForm(const uint8_t op,
                  const uint32_t rt,
                  const uint32_t ra,
                  const uint32_t rb,
                  const uint16_t xop,
                  const bool rc = 0){

      X_form_t x_formater {
                            rc,
                            xop,
                            static_cast<uint32_t>(rb),
                            static_cast<uint32_t>(ra),
                            static_cast<uint32_t>(rt),
                            op
                          };

      dword(x_formater.instruction);
   }


   void EmitDSForm(const uint8_t op,
                   const RegNumber rt,
                   const RegNumber ra,
                   const uint16_t imm,
                   const uint16_t xop) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rt) < 32);
     assert(static_cast<uint16_t>(imm << 14) == 0);

      DS_form_t ds_formater {
                             xop,
                             static_cast<uint32_t>(imm) >> 2,
                             static_cast<uint32_t>(ra),
                             static_cast<uint32_t>(rt),
                             op
                            };

      dword(ds_formater.instruction);
   }

   void EmitDQForm(const uint8_t op,
                   const RegNumber rtp,
                   const RegNumber ra,
                   uint16_t imm){

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint16_t>(imm << 12) == 0);

      DQ_form_t dq_formater {
                             0x0, //Reserved
                             static_cast<uint32_t>(rtp),
                             static_cast<uint32_t>(ra),
                             static_cast<uint32_t>(imm) >> 4,
                             op
                            };

      dword(dq_formater.instruction);
   }


   void EmitXLForm(const uint8_t op,
                   const uint8_t bt,
                   const uint8_t ba,
                   const uint8_t bb,
                   const uint16_t xop,
                   const bool lk = 0) {

      XL_form_t xl_formater {
                             lk,
                             xop,
                             bb,
                             ba,
                             bt,
                             op
                            };

      dword(xl_formater.instruction);
   }

   void EmitAForm(const uint8_t op,
                  const RegNumber rt,
                  const RegNumber ra,
                  const RegNumber rb,
                  const RegNumber bc,
                  const uint16_t xop,
                  const bool rc = 0) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(rt) < 32);
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rb) < 32);
     assert(static_cast<uint32_t>(bc) < 32);

      A_form_t a_formater {
                           rc,
                           xop,
                           static_cast<uint32_t>(bc),
                           static_cast<uint32_t>(rb),
                           static_cast<uint32_t>(ra),
                           static_cast<uint32_t>(rt),
                           op
                          };

      dword(a_formater.instruction);
   }

   void EmitMForm(const uint8_t op,
                  const RegNumber rs,
                  const RegNumber ra,
                  const RegNumber rb,
                  const uint8_t mb,
                  const uint8_t me,
                  const bool rc = 0) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(rb) < 32);
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rb) < 32);

      M_form_t m_formater {
                           rc,
                           me,
                           mb,
                           static_cast<uint32_t>(rb),
                           static_cast<uint32_t>(ra),
                           static_cast<uint32_t>(rs),
                           op
                          };

      dword(m_formater.instruction);
   }

   void EmitMDForm(const uint8_t op,
                   const RegNumber rs,
                   const RegNumber ra,
                   const uint8_t sh,
                   const uint8_t mb,
                   const uint8_t xop,
                   const bool rc = 0) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rs) < 32);

      MD_form_t md_formater {
        rc,
        static_cast<uint32_t>(sh >> 5),                         // sh5
        xop,
        static_cast<uint32_t>(((mb >> 5) | (mb << 1)) & 0x3F),  // me5 || me0:4
        static_cast<uint32_t>(sh & 0x1F),                       // sh0:4
        static_cast<uint32_t>(ra),
        static_cast<uint32_t>(rs),
        op
      };

      dword(md_formater.instruction);
   }

   void EmitMDSForm(const uint8_t op,
                    const RegNumber rs,
                    const RegNumber ra,
                    const RegNumber rb,
                    const uint8_t mb,
                    const uint8_t xop,
                    const bool rc = 0) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(rb) < 32);
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rs) < 32);

      MDS_form_t mds_formater {
                               rc,
                               xop,
                               mb,
                               static_cast<uint32_t>(rb),
                               static_cast<uint32_t>(ra),
                               static_cast<uint32_t>(rs),
                               op
                              };

      dword(mds_formater.instruction);
   }

  void EmitXFXForm(const uint8_t op,
                   const RegNumber rs,
                   const SpecialReg spr,
                   const uint16_t xo,
                   const uint8_t rsv = 0) {

    // GP Register cannot be greater than 31
    assert(static_cast<uint32_t>(rs) < 32);

    XFX_form_t xfx_formater {
      rsv,
      xo,
      (static_cast<uint32_t>(spr) >> 5) & 0x1F,
      static_cast<uint32_t>(spr) & 0x1F,
      static_cast<uint32_t>(rs),
      op
    };

    dword(xfx_formater.instruction);
  }
  void EmitXFXForm(const uint8_t op,
                   const RegNumber rs,
                   const uint16_t mask,
                   const uint16_t xo,
                   const uint8_t rsv = 0) {

    // GP Register cannot be greater than 31
    assert(static_cast<uint32_t>(rs) < 32);

    XFX_form_t xfx_formater {
      rsv,
      xo,
      static_cast<uint32_t>((mask) & 0x1f),
      static_cast<uint32_t>(((mask) >> 5) & 0x1F),
      static_cast<uint32_t>(rs),
      op
    };

    dword(xfx_formater.instruction);
  }

  void EmitXX2Form(const uint8_t op,
                   const RegNumber t,
                   const uint8_t uim,
                   const RegNumber b,
                   const uint16_t xo,
                   const bool bx,
                   const bool tx)  {
    XX2_form_t xx2_formater {
      tx,
      bx,
      xo,
      static_cast<uint32_t>(b),
      static_cast<uint32_t>(uim & 0x3),
      static_cast<uint32_t>(t),
      op
    };
    dword(xx2_formater.instruction);
  }

  void EmitXX3Form(const uint8_t op,
                   const RegNumber t,
                   const RegNumber a,
                   const RegNumber b,
                   const uint16_t xo,
                   const bool ax,
                   const bool bx,
                   const bool tx) {
    XX3_form_t xx3_formater {
      tx,
      bx,
      ax,
      xo,
      static_cast<uint32_t>(b),
      static_cast<uint32_t>(a),
      static_cast<uint32_t>(t),
      op
    };
    dword(xx3_formater.instruction);
  }

  void EmitXX1Form(const uint8_t op,
                   const RegNumber s,
                   const RegNumber ra,
                   const RegNumber rb,
                   const uint16_t xo,
                   const bool tx) {

    // GP Register cannot be greater than 31
    assert(static_cast<uint32_t>(s) < 32);
    assert(static_cast<uint32_t>(ra) < 32);
    assert(static_cast<uint32_t>(rb) < 32);

    XX1_form_t xx1_formater {
      tx,
      xo,
      static_cast<uint32_t>(rb),
      static_cast<uint32_t>(ra),
      static_cast<uint32_t>(s),
      op
    };

    dword(xx1_formater.instruction);
  }

  void EmitVXForm(const uint8_t op,
                  const RegNumber rt,
                  const RegNumber ra,
                  const RegNumber rb,
                  const uint16_t xo) {

    assert(static_cast<uint32_t>(rt) < 32);
    assert(static_cast<uint32_t>(ra) < 32);
    assert(static_cast<uint32_t>(rb) < 32);

    VX_form_t vx_formater {
      xo,
      static_cast<uint32_t>(rb),
      static_cast<uint32_t>(ra),
      static_cast<uint32_t>(rt),
      op
    };

    dword(vx_formater.instruction);
  }
  //TODO(rcardoso): Unimplemented instruction formaters
  void EmitXSForm(const uint8_t op,
                  const RegNumber rt,
                  const RegNumber ra,
                  const uint8_t sh,
                  const uint16_t xop,
                  const bool rc = 0){

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(rt) < 32);
     assert(static_cast<uint32_t>(ra) < 32);

      XS_form_t xs_formater {
                            rc,
                            static_cast<uint32_t>(sh >> 5),
                            xop,
                            static_cast<uint32_t>(sh & 0x1F),
                            static_cast<uint32_t>(ra),
                            static_cast<uint32_t>(rt),
                            op
                          };

      dword(xs_formater.instruction);
   }

private:
  //Low-level emitter functions.
  void byte(uint8_t b) {
    codeBlock.byte(b);
  }
  void word(uint16_t w) {
    codeBlock.word(w);
  }
  void dword(uint32_t dw) {
    codeBlock.dword(dw);
  }
  void qword(uint64_t qw) {
    codeBlock.qword(qw);
  }
  void bytes(size_t n, const uint8_t* bs) {
    codeBlock.bytes(n, bs);
  }

  HPHP::CodeBlock& codeBlock;

  RegNumber rn(Reg64 r) {
    return RegNumber(int(r));
  }
  RegNumber rn(RegXMM r) {
    return RegNumber(int(r));
  }
  RegNumber rn(RegSF r) {
    return RegNumber(int(r));
  }
  RegNumber rn(int n) {
    return RegNumber(int(n));
  }

};

} // namespace ppc64_asm

#endif
