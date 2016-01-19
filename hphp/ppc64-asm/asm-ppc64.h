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

#ifndef incl_ASM_PPC64_H_
#define incl_ASM_PPC64_H_

#include <cstdint>
#include <cassert>
#include <vector>
#include <iostream>

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/ppc64-asm/isa-ppc64.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/immed.h"

namespace ppc64_asm {

/* using override */ using HPHP::jit::Reg64;
/* using override */ using HPHP::jit::RegXMM;
/* using override */ using HPHP::jit::RegSF;
/* using override */ using HPHP::jit::MemoryRef;
/* using override */ using HPHP::jit::Immed;
/* using override */ using HPHP::CodeAddress;
/* using override */ using HPHP::jit::ConditionCode;

/* Used to  define a minimal callstack on call/ret vasm */
// Must be the same value of AROFF(_dummyB).
constexpr int min_callstack_size       = 32;
// Must be the same value of AROFF(m_savedRip).
constexpr int lr_position_on_callstack = 16;

#define BRANCHES(cr) \
  CR##cr##_LessThan,         \
  CR##cr##_LessThanEqual,    \
  CR##cr##_GreaterThan,      \
  CR##cr##_GreaterThanEqual, \
  CR##cr##_Equal,            \
  CR##cr##_NotEqual,         \
  CR##cr##_Overflow,         \
  CR##cr##_NoOverflow

enum class BranchConditions {
  BRANCHES(0),
  BRANCHES(1),
  BRANCHES(2),
  BRANCHES(3),
  BRANCHES(4),
  BRANCHES(5),
  BRANCHES(6),
  BRANCHES(7),
  Always,

  // mnemonics for the common case by using CR0:
  LessThan          = CR0_LessThan,
  LessThanEqual     = CR0_LessThanEqual,
  GreaterThan       = CR0_GreaterThan,
  GreaterThanEqual  = CR0_GreaterThanEqual,
  Equal             = CR0_Equal,
  NotEqual          = CR0_NotEqual,
  Overflow          = CR0_Overflow,
  NoOverflow        = CR0_NoOverflow
};

#undef BRANCHES

class BranchParams {
  public:
    /* BO and BI parameter mapping related to BranchConditions */
    enum class BO {
      CRNotSet              = 4,
      CRSet                 = 12,
      Always                = 20
    };

#define CR_CONDITIONS(cr) \
      CR##cr##_LessThan          = (0 + (cr * 4)), \
      CR##cr##_GreaterThan       = (1 + (cr * 4)), \
      CR##cr##_Equal             = (2 + (cr * 4)), \
      CR##cr##_SummaryOverflow   = (3 + (cr * 4))

    enum class BI {
      CR_CONDITIONS(0),
      CR_CONDITIONS(1),
      CR_CONDITIONS(2),
      CR_CONDITIONS(3),
      CR_CONDITIONS(4),
      CR_CONDITIONS(5),
      CR_CONDITIONS(6),
      CR_CONDITIONS(7)
    };

#undef CR_CONDITIONS

    enum class BH {
      CTR_Loop              = 0,
      LR_Loop               = 1,
      Reserved              = 2,
      NoBranchPrediction    = 3
    };

  private:
#define SWITCHES(cr)                                                    \
  case BranchConditions::CR##cr##_LessThan:                             \
    m_bo = BO::CRSet;    m_bi = BI::CR##cr##_LessThan;          break;  \
  case BranchConditions::CR##cr##_LessThanEqual:                        \
    m_bo = BO::CRNotSet; m_bi = BI::CR##cr##_GreaterThan;       break;  \
  case BranchConditions::CR##cr##_GreaterThan:                          \
    m_bo = BO::CRSet;    m_bi = BI::CR##cr##_GreaterThan;       break;  \
  case BranchConditions::CR##cr##_GreaterThanEqual:                     \
    m_bo = BO::CRNotSet; m_bi = BI::CR##cr##_LessThan;          break;  \
  case BranchConditions::CR##cr##_Equal:                                \
    m_bo = BO::CRSet;    m_bi = BI::CR##cr##_Equal;             break;  \
  case BranchConditions::CR##cr##_NotEqual:                             \
    m_bo = BO::CRNotSet; m_bi = BI::CR##cr##_Equal;             break;  \
  case BranchConditions::CR##cr##_Overflow:                             \
    m_bo = BO::CRSet;    m_bi = BI::CR##cr##_SummaryOverflow;   break;  \
  case BranchConditions::CR##cr##_NoOverflow:                           \
    m_bo = BO::CRNotSet; m_bi = BI::CR##cr##_SummaryOverflow;   break

    /* Constructor auxiliary */
    void defineBoBi(BranchConditions bc) {
      switch (bc) {
        SWITCHES(0);
        SWITCHES(1);
        SWITCHES(2);
        SWITCHES(3);
        SWITCHES(4);
        SWITCHES(5);
        SWITCHES(6);
        SWITCHES(7);
        case BranchConditions::Always:
          m_bo = BO::Always; m_bi = BI(0); break;
        default:
          not_implemented();
      }
    }
#undef SWITCHES

  public:
    BranchParams() = delete;

    BranchParams(BranchConditions bc) { defineBoBi(bc); }

    BranchParams(ConditionCode cc) {
      defineBoBi(convertCC(cc));
    }

    static BranchConditions convertCC(ConditionCode cc) {
      BranchConditions ret = BranchConditions::Always;

      switch (cc) {
        case HPHP::jit::CC_O:
          ret = BranchConditions::Overflow;         break;
        case HPHP::jit::CC_NO:
          ret = BranchConditions::NoOverflow;       break;
        case HPHP::jit::CC_B:
          ret = BranchConditions::LessThan;         break;
        case HPHP::jit::CC_AE:
          ret = BranchConditions::GreaterThanEqual; break;
        case HPHP::jit::CC_E:
          ret = BranchConditions::Equal;            break;
        case HPHP::jit::CC_NE:
          ret = BranchConditions::NotEqual;         break;
        case HPHP::jit::CC_BE:
          ret = BranchConditions::LessThanEqual;    break;
        case HPHP::jit::CC_A:
          ret = BranchConditions::GreaterThan;      break;
        case HPHP::jit::CC_S:
          ret = BranchConditions::LessThan;         break;
        case HPHP::jit::CC_NS:
          ret = BranchConditions::GreaterThan;      break;

        /*
         * TODO(gut): Parity on ppc64 is not that easy:
         * http://stackoverflow.com/q/32319673/5013070
         */
        case HPHP::jit::CC_P:
          not_implemented(); /*ret = ;*/            break;
        case HPHP::jit::CC_NP:
          not_implemented(); /*ret = ;*/            break;

        case HPHP::jit::CC_L:
          ret = BranchConditions::LessThan;         break;
        case HPHP::jit::CC_NL:
          ret = BranchConditions::GreaterThanEqual; break;
        case HPHP::jit::CC_NG:
          ret = BranchConditions::LessThanEqual;    break;
        case HPHP::jit::CC_G:
          ret = BranchConditions::GreaterThan;      break;

        default:
          not_implemented();                        break;
      }
      return ret;
    }

    /*
     * Get the BranchParams from an emitted conditional branch
     */
    BranchParams(PPC64Instr instr) {
      // first, guarantee that is a conditional branch
      if (((instr >> 26) == 16) || ((instr >> 26) == 19)) {
        // bc, bclr, bcctr, bctar
        m_bo = (BranchParams::BO)((instr >> 21) & 0x1F);
        m_bi = (BranchParams::BI)((instr >> 16) & 0x1F);
      } else {
        assert(false && "Not a valid conditional branch instruction");
        // also possible: defineBoBi(BranchConditions::Always);
      }
    }

    ~BranchParams() {}

    /*
     * Converts to ConditionCode upon casting to it
     */
    /* implicit */ operator ConditionCode() {
      ConditionCode ret = HPHP::jit::CC_None;

      switch (m_bi) {
        case BI::CR0_LessThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_B;  // CC_S, CC_L
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_AE; // CC_NL
          break;
        case BI::CR0_GreaterThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_A;  // CC_NS, CC_G
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_BE; // CC_NG
          break;
        case BI::CR0_Equal:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_E;
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_NE;
          break;
        case BI::CR0_SummaryOverflow:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_O;
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_NO;
          break;
        default:
          assert(false && "Not a valid conditional branch parameter");
          break;
      }

      return ret;
    }


    uint8_t bo() { return (uint8_t)m_bo; }
    uint8_t bi() { return (uint8_t)m_bi; }

  private:
    BranchParams::BO m_bo;
    BranchParams::BI m_bi;
};


enum class LinkReg {
  Save,
  DoNotTouch
};

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

struct Label;

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

  // How many bytes a PPC64 instruction length is
  static const uint8_t kBytesPerInstr = sizeof(PPC64Instr);

  // Total ammount of bytes that a li64 function emits
  static const uint8_t kLi64InstrLen = 5 * kBytesPerInstr;

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
  void addc(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void addco(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void adde(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void addeo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void addi(const Reg64& rt, const Reg64& ra, Immed imm);
  void addic(const Reg64& rt, const Reg64& ra, uint16_t imm, bool rc = 0);
  void addis(const Reg64& rt, const Reg64& ra, Immed imm);
  void addme(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void addmeo(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void addo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void addze(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void addzeo(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void and(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void andc(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void andi(const Reg64& ra, const Reg64& rs, Immed imm);
  void andis(const Reg64& ra, const Reg64& rs, Immed imm);
  void b(int32_t offset);
  void ba(uint32_t target_addr);
  void bl(int32_t offset);
  void bla(uint32_t target_addr);
  void bc(uint8_t bo, uint8_t bi, int16_t offset);
  void bca(uint8_t bo, uint8_t bi, uint16_t target_addr);
  void bcctr(uint8_t bo, uint8_t bi, uint16_t bh);
  void bcctrl(uint8_t bo, uint8_t bi, uint16_t bh);
  void bcl(uint8_t bo, uint8_t bi, int16_t offset);
  void bcla(uint8_t bo, uint8_t bi, uint16_t target_addr);
  void bclr(uint8_t bo, uint8_t bi, uint16_t bh);
  void bclrl(uint8_t bo, uint8_t bi, uint16_t bh);
  void bctar(uint8_t bo, uint8_t bi, uint16_t bh);
  void bctarl(uint8_t bo, uint8_t bi, uint16_t bh);
  void bpermd(const Reg64& ra, const Reg64& rs, const Reg64& rv);
  void cmp(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb);
  void cmpi(uint16_t bf, bool l, const Reg64& ra, Immed imm);
  void cmpb(const Reg64& rs, const Reg64& ra, const Reg64& rb);
  void cmpl(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb);
  void cmpli(uint16_t bf, bool l, const Reg64& ra, Immed imm);
  void cntlzd(const Reg64& ra, const Reg64& rs, bool rc = 0);
  void cntlzw(const Reg64& ra, const Reg64& rs, bool rc = 0);
  void crand(uint16_t bt, uint16_t ba, uint16_t bb);
  void crandc(uint16_t bt, uint16_t ba, uint16_t bb);
  void creqv(uint16_t bt, uint16_t ba, uint16_t bb);
  void crnand(uint16_t bt, uint16_t ba, uint16_t bb);
  void crnor(uint16_t bt, uint16_t ba, uint16_t bb);
  void cror(uint16_t bt, uint16_t ba, uint16_t bb);
  void crorc(uint16_t bt, uint16_t ba, uint16_t bb);
  void crxor(uint16_t bt, uint16_t ba, uint16_t bb);
  void divd(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divde(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divdeo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divdeu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divdeuo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divdo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divdu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divduo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divw(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divwe(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divweo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divweu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divweuo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divwo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divwu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void divwuo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void eqv(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
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
    EmitDForm(48, rn(frt), rn(m.r.base), m.r.disp);
  }
  void lxvw4x(const RegXMM& Xt, const MemoryRef& m) {
    EmitXX1Form(31, rn(Xt), rn(m.r.base), rn(m.r.index), 780, 0);
  }
  void isel(const Reg64& rt, const Reg64& ra, const Reg64& rb, uint8_t bc);
  void lbz(const Reg64& rt, MemoryRef m);
  void lbzu(const Reg64& rt, MemoryRef m);
  void lbzux(const Reg64& rt, MemoryRef m);
  void lbzx(const Reg64& rt, MemoryRef m);
  void ld(const Reg64& rt, MemoryRef m);
  void ldbrx(const Reg64& rt, MemoryRef m);
  void ldu(const Reg64& rt, MemoryRef m);
  void ldux(const Reg64& rt, MemoryRef m);
  void ldx(const Reg64& rt, MemoryRef m);
  void lhbrx(const Reg64& rt, MemoryRef m);
  void lhz(const Reg64& rt, MemoryRef m);
  void lhzu(const Reg64& rt, MemoryRef m);
  void lhzux(const Reg64& rt, MemoryRef m);
  void lhzx(const Reg64& rt, MemoryRef m);
  void lha(const Reg64& rt, MemoryRef m);
  void lhau(const Reg64& rt, MemoryRef m);
  void lhaux(const Reg64& rt, MemoryRef m);
  void lhax(const Reg64& rt, MemoryRef m);
  void lmw(const Reg64& rt, MemoryRef m);
  void lq(const Reg64& rtp, MemoryRef m);
  void lswi(const Reg64& rt, MemoryRef m);
  void lswx(const Reg64& rt, MemoryRef m);
  void lwz(const Reg64& rt, MemoryRef m);
  void lwzu(const Reg64& rt, MemoryRef m);
  void lwzux(const Reg64& rt, MemoryRef m);
  void lwzx(const Reg64& rt, MemoryRef m);
  void lwa(const Reg64& rt, MemoryRef m);
  void lwaux(const Reg64& rt, MemoryRef m);
  void lwax(const Reg64& rt, MemoryRef m);
  void lwbrx(const Reg64& rt, MemoryRef m);
  void mcrf(uint16_t bf, uint16_t bfa);
  void mfspr(const SpecialReg spr, const Reg64& rs);
  void mtspr(const SpecialReg spr, const Reg64& rs);
  void mulhd(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulhdu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulhw(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulhwu(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulld(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulldo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mulli(const Reg64& rt, const Reg64& ra, uint16_t imm);
  void mullw(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void mullwo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void nand(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void neg(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void nego(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void nor(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void or(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void orc(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void ori(const Reg64& ra, const Reg64& rs, Immed imm);
  void oris(const Reg64& ra, const Reg64& rs, Immed imm);
  void popcntb(const Reg64& ra, const Reg64& rs);
  void popcntd(const Reg64& ra, const Reg64& rs);
  void popcntw(const Reg64& ra, const Reg64& rs);
  void prtyd(const Reg64& ra, const Reg64& rs);
  void prtyw(const Reg64& ra, const Reg64& rs);
  void rldcl(const Reg64& ra, const Reg64& rs, const Reg64& rb,
             uint8_t mb, bool rc = 0);
  void rldcr(const Reg64& ra, const Reg64& rs,  const Reg64& rb,
             uint8_t mb, bool rc = 0);
  void rldic(const Reg64& ra, const Reg64& rs, uint8_t sh,
             uint8_t mb, bool rc = 0);
  void rldicl(const Reg64& ra, const Reg64& rs, uint8_t sh,
              uint8_t mb, bool rc = 0);
  void rldicr(const Reg64& ra, const Reg64& rs, uint8_t sh,
              uint8_t mb, bool rc = 0);
  void rldimi(const Reg64& ra, const Reg64& rs, uint8_t sh,
              uint8_t mb, bool rc = 0);
  void rlwimi(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
              uint16_t me, bool rc = 0);
  void rlwinm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
              uint16_t me, bool rc = 0);
  void rlwnm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
             uint16_t me, bool rc = 0);
  void sc(uint16_t lev);
  void sld(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void slw(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void srad(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void sraw(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void srawi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc = 0);
  void srd(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void srw(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void stb(const Reg64& rs, MemoryRef m);
  void stbu(const Reg64& rs, MemoryRef m);
  void stbux(const Reg64& rs, MemoryRef m);
  void stbx(const Reg64& rs, MemoryRef m);
  void stfs(const RegXMM& frt, MemoryRef m) {
    EmitDForm(52, rn(frt), rn(m.r.base), m.r.disp);
  }
  void sth(const Reg64& rs, MemoryRef m);
  void sthu(const Reg64& rs, MemoryRef m);
  void sthux(const Reg64& rs, MemoryRef m);
  void sthx(const Reg64& rs, MemoryRef m);
  void stw(const Reg64& rs, MemoryRef m);
  void stwu(const Reg64& rs, MemoryRef m);
  void stwux(const Reg64& rs, MemoryRef m);
  void stwx(const Reg64& rs, MemoryRef m);
  void std(const Reg64& rs, MemoryRef m);
  void stdu(const Reg64& rs, MemoryRef m);
  void stdux(const Reg64& rs, MemoryRef m);
  void stdx(const Reg64& rs, MemoryRef m);
  void stq(const Reg64& rs, MemoryRef m);
  void sthbrx(const Reg64& rs, MemoryRef m);
  void stwbrx(const Reg64& rs, MemoryRef m);
  void stdbrx(const Reg64& rs, MemoryRef m);
  void stmw(const Reg64& rs, MemoryRef m);
  void stswi(const Reg64& rs, MemoryRef m);
  void stswx(const Reg64& rs, MemoryRef m);
  void stxvw4x(const RegXMM& xs, const MemoryRef& m) {
    EmitXX1Form(31, rn(xs), rn(m.r.base), rn(m.r.index), 972, 0);
  }
  void subf(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void subfo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void subfc(const Reg64& rt, const Reg64& ra, const Reg64& rb,  bool rc = 0);
  void subfco(const Reg64& rt, const Reg64& ra, const Reg64& rb,  bool rc = 0);
  void subfe(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void subfeo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0);
  void subfic(const Reg64& rt, const Reg64& ra,  uint16_t imm);
  void subfme(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void subfmeo(const Reg64& rt,  const Reg64& ra, bool rc = 0);
  void subfze(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void subfzeo(const Reg64& rt, const Reg64& ra, bool rc = 0);
  void td(uint16_t to, const Reg64& ra, const Reg64& rb);
  void tdi(uint16_t to, const Reg64& ra, uint16_t imm);
  void tw(uint16_t to, const Reg64& ra, const Reg64& rb);
  void twi(uint16_t to, const Reg64& ra, uint16_t imm);
  void xor(const Reg64& ra, const Reg64& rs, const Reg64& rb, bool rc = 0);
  void xori(const Reg64& ra, const Reg64& rs, Immed imm);
  void xoris(const Reg64& ra, const Reg64& rs, Immed imm);
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
  void addg6s()         { not_implemented(); }
  void bcdadd()         { not_implemented(); }
  void bcdsub()         { not_implemented(); }
  void brinc()          { not_implemented(); }
  void cbcdtd()         { not_implemented(); }
  void cdtbcd()         { not_implemented(); }
  void clrbhrb()        { not_implemented(); }
  void dadd()           { not_implemented(); }
  void daddq()          { not_implemented(); }
  void dcba()           { not_implemented(); }
  void dcbf()           { not_implemented(); }
  void dcbfep()         { not_implemented(); }
  void dcbi()           { not_implemented(); }
  void dcblc()          { not_implemented(); }
  void dcblq()          { not_implemented(); }
  void dcbst()          { not_implemented(); }
  void dcbstep()        { not_implemented(); }
  void dcbt()           { not_implemented(); }
  void dcbtep()         { not_implemented(); }
  void dcbtls()         { not_implemented(); }
  void dcbtst()         { not_implemented(); }
  void dcbtstep()       { not_implemented(); }
  void dcbtstls()       { not_implemented(); }
  void dcbz()           { not_implemented(); }
  void dcbzep()         { not_implemented(); }
  void dcffix()         { not_implemented(); }
  void dcffixq()        { not_implemented(); }
  void dci()            { not_implemented(); }
  void dcmpo()          { not_implemented(); }
  void dcmpoq()         { not_implemented(); }
  void dcmpu(const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(59, rn(0), rn(fra), rn(frb), 642);
  }
  void dcmpuq()         { not_implemented(); }
  void dcread()         { not_implemented(); }
  void dctdp()          { not_implemented(); }
  void dctfix()         { not_implemented(); }
  void dctfixq()        { not_implemented(); }
  void dctqpq()         { not_implemented(); }
  void ddedpd()         { not_implemented(); }
  void ddedpdq()        { not_implemented(); }
  void ddiv()           { not_implemented(); }
  void ddivq()          { not_implemented(); }
  void denbcd()         { not_implemented(); }
  void denbcdq()        { not_implemented(); }
  void diex()           { not_implemented(); }
  void diexq()          { not_implemented(); }
  void dlmzb()          { not_implemented(); }
  void dmul()           { not_implemented(); }
  void dmulq()          { not_implemented(); }
  void dnh()            { not_implemented(); }
  void doze()           { not_implemented(); }
  void dqua()           { not_implemented(); }
  void dquai()          { not_implemented(); }
  void dquaiq()         { not_implemented(); }
  void dquaq()          { not_implemented(); }
  void drdpq()          { not_implemented(); }
  void drintn()         { not_implemented(); }
  void drintnq()        { not_implemented(); }
  void drintx()         { not_implemented(); }
  void drintxq()        { not_implemented(); }
  void drrnd()          { not_implemented(); }
  void drrndq()         { not_implemented(); }
  void drsp()           { not_implemented(); }
  void dscli()          { not_implemented(); }
  void dscliq()         { not_implemented(); }
  void dscri()          { not_implemented(); }
  void dscriq()         { not_implemented(); }
  void dsn()            { not_implemented(); }
  void dsub()           { not_implemented(); }
  void dsubq()          { not_implemented(); }
  void dtstdc()         { not_implemented(); }
  void dtstdcq()        { not_implemented(); }
  void dtstdg()         { not_implemented(); }
  void dtstdgq()        { not_implemented(); }
  void dtstex()         { not_implemented(); }
  void dtstexq()        { not_implemented(); }
  void dtstsf()         { not_implemented(); }
  void dtstsfq()        { not_implemented(); }
  void dxex()           { not_implemented(); }
  void dxexq()          { not_implemented(); }
  void eciwx()          { not_implemented(); }
  void ecowx()          { not_implemented(); }
  void efdabs()         { not_implemented(); }
  void efdadd()         { not_implemented(); }
  void efdcfs()         { not_implemented(); }
  void efdcfsf()        { not_implemented(); }
  void efdcfsi()        { not_implemented(); }
  void efdcfsid()       { not_implemented(); }
  void efdcfuf()        { not_implemented(); }
  void efdcfui()        { not_implemented(); }
  void efdcfuid()       { not_implemented(); }
  void efdcmpeq()       { not_implemented(); }
  void efdcmpgt()       { not_implemented(); }
  void efdcmplt()       { not_implemented(); }
  void efdctsf()        { not_implemented(); }
  void efdctsi()        { not_implemented(); }
  void efdctsidz()      { not_implemented(); }
  void efdctsiz()       { not_implemented(); }
  void efdctuf()        { not_implemented(); }
  void efdctui()        { not_implemented(); }
  void efdctuidz()      { not_implemented(); }
  void efdctuiz()       { not_implemented(); }
  void efddiv()         { not_implemented(); }
  void efdmul()         { not_implemented(); }
  void efdnabs()        { not_implemented(); }
  void efdneg()         { not_implemented(); }
  void efdsub()         { not_implemented(); }
  void efdtsteq()       { not_implemented(); }
  void efdtstgt()       { not_implemented(); }
  void efdtstlt()       { not_implemented(); }
  void efsabs()         { not_implemented(); }
  void efsadd()         { not_implemented(); }
  void efscfd()         { not_implemented(); }
  void efscfsf()        { not_implemented(); }
  void efscfsi()        { not_implemented(); }
  void efscfuf()        { not_implemented(); }
  void efscfui()        { not_implemented(); }
  void efscmpeq()       { not_implemented(); }
  void efscmpgt()       { not_implemented(); }
  void efscmplt()       { not_implemented(); }
  void efsctsf()        { not_implemented(); }
  void efsctsi()        { not_implemented(); }
  void efsctsiz()       { not_implemented(); }
  void efsctuf()        { not_implemented(); }
  void efsctui()        { not_implemented(); }
  void efsctuiz()       { not_implemented(); }
  void efsdiv()         { not_implemented(); }
  void efsmul()         { not_implemented(); }
  void efsnabs()        { not_implemented(); }
  void efsneg()         { not_implemented(); }
  void efssub()         { not_implemented(); }
  void efststeq()       { not_implemented(); }
  void efststgt()       { not_implemented(); }
  void efststlt()       { not_implemented(); }
  void ehpriv()         { not_implemented(); }
  void eieio()          { not_implemented(); }
  void evabs()          { not_implemented(); }
  void evaddiw()        { not_implemented(); }
  void evaddsmiaaw()    { not_implemented(); }
  void evaddssiaaw()    { not_implemented(); }
  void evaddumiaaw()    { not_implemented(); }
  void evaddusiaaw()    { not_implemented(); }
  void evaddw()         { not_implemented(); }
  void evand()          { not_implemented(); }
  void evandc()         { not_implemented(); }
  void evcmpeq()        { not_implemented(); }
  void evcmpgts()       { not_implemented(); }
  void evcmpgtu()       { not_implemented(); }
  void evcmplts()       { not_implemented(); }
  void evcmpltu()       { not_implemented(); }
  void evcntlsw()       { not_implemented(); }
  void evcntlzw()       { not_implemented(); }
  void evdivws()        { not_implemented(); }
  void evdivwu()        { not_implemented(); }
  void eveqv()          { not_implemented(); }
  void evextsb()        { not_implemented(); }
  void evextsh()        { not_implemented(); }
  void evfsabs()        { not_implemented(); }
  void evfsadd()        { not_implemented(); }
  void evfscfsf()       { not_implemented(); }
  void evfscfsi()       { not_implemented(); }
  void evfscfuf()       { not_implemented(); }
  void evfscfui()       { not_implemented(); }
  void evfscmpeq()      { not_implemented(); }
  void evfscmpgt()      { not_implemented(); }
  void evfscmplt()      { not_implemented(); }
  void evfsctsf()       { not_implemented(); }
  void evfsctsi()       { not_implemented(); }
  void evfsctsiz()      { not_implemented(); }
  void evfsctuf()       { not_implemented(); }
  void evfsctui()       { not_implemented(); }
  void evfsctuiz()      { not_implemented(); }
  void evfsdiv()        { not_implemented(); }
  void evfsmul()        { not_implemented(); }
  void evfsnabs()       { not_implemented(); }
  void evfsneg()        { not_implemented(); }
  void evfssub()        { not_implemented(); }
  void evfststeq()      { not_implemented(); }
  void evfststgt()      { not_implemented(); }
  void evfststlt()      { not_implemented(); }
  void evldd()          { not_implemented(); }
  void evlddepx()       { not_implemented(); }
  void evlddx()         { not_implemented(); }
  void evldh()          { not_implemented(); }
  void evldhx()         { not_implemented(); }
  void evldw()          { not_implemented(); }
  void evldwx()         { not_implemented(); }
  void evlhhesplat()    { not_implemented(); }
  void evlhhesplatx()   { not_implemented(); }
  void evlhhossplat()   { not_implemented(); }
  void evlhhossplatx()  { not_implemented(); }
  void evlhhousplat()   { not_implemented(); }
  void evlhhousplatx()  { not_implemented(); }
  void evlwhe()         { not_implemented(); }
  void evlwhex()        { not_implemented(); }
  void evlwhos()        { not_implemented(); }
  void evlwhosx()       { not_implemented(); }
  void evlwhou()        { not_implemented(); }
  void evlwhoux()       { not_implemented(); }
  void evlwhsplat()     { not_implemented(); }
  void evlwhsplatx()    { not_implemented(); }
  void evlwwsplat()     { not_implemented(); }
  void evlwwsplatx()    { not_implemented(); }
  void evmergehi()      { not_implemented(); }
  void evmergehilo()    { not_implemented(); }
  void evmergelo()      { not_implemented(); }
  void evmergelohi()    { not_implemented(); }
  void evmhegsmfaa()    { not_implemented(); }
  void evmhegsmfan()    { not_implemented(); }
  void evmhegsmiaa()    { not_implemented(); }
  void evmhegsmian()    { not_implemented(); }
  void evmhegumiaa()    { not_implemented(); }
  void evmhegumian()    { not_implemented(); }
  void evmhesmf()       { not_implemented(); }
  void evmhesmfa()      { not_implemented(); }
  void evmhesmfaaw()    { not_implemented(); }
  void evmhesmfanw()    { not_implemented(); }
  void evmhesmi()       { not_implemented(); }
  void evmhesmia()      { not_implemented(); }
  void evmhesmiaaw()    { not_implemented(); }
  void evmhesmianw()    { not_implemented(); }
  void evmhessf()       { not_implemented(); }
  void evmhessfa()      { not_implemented(); }
  void evmhessfaaw()    { not_implemented(); }
  void evmhessfanw()    { not_implemented(); }
  void evmhessiaaw()    { not_implemented(); }
  void evmhessianw()    { not_implemented(); }
  void evmheumi()       { not_implemented(); }
  void evmheumia()      { not_implemented(); }
  void evmheumiaaw()    { not_implemented(); }
  void evmheumianw()    { not_implemented(); }
  void evmheusiaaw()    { not_implemented(); }
  void evmheusianw()    { not_implemented(); }
  void evmhogsmfaa()    { not_implemented(); }
  void evmhogsmfan()    { not_implemented(); }
  void evmhogsmiaa()    { not_implemented(); }
  void evmhogsmian()    { not_implemented(); }
  void evmhogumiaa()    { not_implemented(); }
  void evmhogumian()    { not_implemented(); }
  void evmhosmf()       { not_implemented(); }
  void evmhosmfa()      { not_implemented(); }
  void evmhosmfaaw()    { not_implemented(); }
  void evmhosmfanw()    { not_implemented(); }
  void evmhosmi()       { not_implemented(); }
  void evmhosmia()      { not_implemented(); }
  void evmhosmiaaw()    { not_implemented(); }
  void evmhosmianw()    { not_implemented(); }
  void evmhossf()       { not_implemented(); }
  void evmhossfa()      { not_implemented(); }
  void evmhossfaaw()    { not_implemented(); }
  void evmhossfanw()    { not_implemented(); }
  void evmhossiaaw()    { not_implemented(); }
  void evmhossianw()    { not_implemented(); }
  void evmhoumi()       { not_implemented(); }
  void evmhoumia()      { not_implemented(); }
  void evmhoumiaaw()    { not_implemented(); }
  void evmhoumianw()    { not_implemented(); }
  void evmhousiaaw()    { not_implemented(); }
  void evmhousianw()    { not_implemented(); }
  void evmra()          { not_implemented(); }
  void evmwhsmf()       { not_implemented(); }
  void evmwhsmfa()      { not_implemented(); }
  void evmwhsmi()       { not_implemented(); }
  void evmwhsmia()      { not_implemented(); }
  void evmwhssf()       { not_implemented(); }
  void evmwhssfa()      { not_implemented(); }
  void evmwhumi()       { not_implemented(); }
  void evmwhumia()      { not_implemented(); }
  void evmwlsmiaaw()    { not_implemented(); }
  void evmwlsmianw()    { not_implemented(); }
  void evmwlssiaaw()    { not_implemented(); }
  void evmwlssianw()    { not_implemented(); }
  void evmwlumi()       { not_implemented(); }
  void evmwlumia()      { not_implemented(); }
  void evmwlumiaaw()    { not_implemented(); }
  void evmwlumianw()    { not_implemented(); }
  void evmwlusiaaw()    { not_implemented(); }
  void evmwlusianw()    { not_implemented(); }
  void evmwsmf()        { not_implemented(); }
  void evmwsmfa()       { not_implemented(); }
  void evmwsmfaa()      { not_implemented(); }
  void evmwsmfan()      { not_implemented(); }
  void evmwsmi()        { not_implemented(); }
  void evmwsmia()       { not_implemented(); }
  void evmwsmiaa()      { not_implemented(); }
  void evmwsmian()      { not_implemented(); }
  void evmwssf()        { not_implemented(); }
  void evmwssfa()       { not_implemented(); }
  void evmwssfaa()      { not_implemented(); }
  void evmwssfan()      { not_implemented(); }
  void evmwumi()        { not_implemented(); }
  void evmwumia()       { not_implemented(); }
  void evmwumiaa()      { not_implemented(); }
  void evmwumian()      { not_implemented(); }
  void evnand()         { not_implemented(); }
  void evneg()          { not_implemented(); }
  void evnor()          { not_implemented(); }
  void evor()           { not_implemented(); }
  void evorc()          { not_implemented(); }
  void evrlw()          { not_implemented(); }
  void evrlwi()         { not_implemented(); }
  void evrndw()         { not_implemented(); }
  void evsel()          { not_implemented(); }
  void evslw()          { not_implemented(); }
  void evslwi()         { not_implemented(); }
  void evsplatfi()      { not_implemented(); }
  void evsplati()       { not_implemented(); }
  void evsrwis()        { not_implemented(); }
  void evsrwiu()        { not_implemented(); }
  void evsrws()         { not_implemented(); }
  void evsrwu()         { not_implemented(); }
  void evstdd()         { not_implemented(); }
  void evstddepx()      { not_implemented(); }
  void evstddx()        { not_implemented(); }
  void evstdh()         { not_implemented(); }
  void evstdhx()        { not_implemented(); }
  void evstdw()         { not_implemented(); }
  void evstdwx()        { not_implemented(); }
  void evstwhe()        { not_implemented(); }
  void evstwhex()       { not_implemented(); }
  void evstwho()        { not_implemented(); }
  void evstwhox()       { not_implemented(); }
  void evstwwe()        { not_implemented(); }
  void evstwwex()       { not_implemented(); }
  void evstwwo()        { not_implemented(); }
  void evstwwox()       { not_implemented(); }
  void evsubfsmiaaw()   { not_implemented(); }
  void evsubfssiaaw()   { not_implemented(); }
  void evsubfumiaaw()   { not_implemented(); }
  void evsubfusiaaw()   { not_implemented(); }
  void evsubfw()        { not_implemented(); }
  void evsubifw()       { not_implemented(); }
  void evxor()          { not_implemented(); }
  void fabs(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 364, rc);
  }
  void fadds()          { not_implemented(); }
  void fcfid()          { not_implemented(); }
  void fcfids(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(59, rn(frt), rn(0), rn(frb), 846, rc);
  }
  void fcfidu()         { not_implemented(); }
  void fcfidus()        { not_implemented(); }
  void fcmpo(const RegSF& sf, const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(63, rn(int(sf) << 2), rn(fra), rn(frb), 32);
  }
  void fcmpu(const RegSF& sf, const RegXMM& fra, const RegXMM& frb) {
    EmitXForm(63, rn(int(sf) << 2), rn(fra), rn(frb), 0);
  }
  void fcpsgn()         { not_implemented(); }
  void fctid(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 814, rc);
  }
  void fctidu()         { not_implemented(); }
  void fctiduz()        { not_implemented(); }
  void fctidz()         { not_implemented(); }
  void fctiw()          { not_implemented(); }
  void fctiwu()         { not_implemented(); }
  void fctiwuz()        { not_implemented(); }
  void fctiwz()         { not_implemented(); }
  void fdiv()           { not_implemented(); }
  void fdivs()          { not_implemented(); }
  void fmadd()          { not_implemented(); }
  void fmadds()         { not_implemented(); }
  void fmr(const RegXMM& frt, const RegXMM& frb, bool rc = 0) {
    EmitXForm(63, rn(frt), rn(0), rn(frb), 72, rc);
  }
  void fmrgew()         { not_implemented(); }
  void fmrgow()         { not_implemented(); }
  void fmsub()          { not_implemented(); }
  void fmsubs()         { not_implemented(); }
  void fmul()           { not_implemented(); }
  void fmuls()          { not_implemented(); }
  void fnabs()          { not_implemented(); }
  void fneg()           { not_implemented(); }
  void fnmadd()         { not_implemented(); }
  void fnmadds()        { not_implemented(); }
  void fnmsub()         { not_implemented(); }
  void fnmsubs()        { not_implemented(); }
  void fre()            { not_implemented(); }
  void fres()           { not_implemented(); }
  void frim()           { not_implemented(); }
  void frin()           { not_implemented(); }
  void frip()           { not_implemented(); }
  void friz()           { not_implemented(); }
  void frsp()           { not_implemented(); }
  void frsqrte()        { not_implemented(); }
  void fsel()           { not_implemented(); }
  void fsqrt()          { not_implemented(); }
  void fsqrts()         { not_implemented(); }
  void fsub()           { not_implemented(); }
  void fsubs()          { not_implemented(); }
  void ftdiv()          { not_implemented(); }
  void ftsqrt()         { not_implemented(); }
  void hrfid()          { not_implemented(); }
  void icbi()           { not_implemented(); }
  void icbiep()         { not_implemented(); }
  void icblc()          { not_implemented(); }
  void icblq()          { not_implemented(); }
  void icbt()           { not_implemented(); }
  void icbtls()         { not_implemented(); }
  void ici()            { not_implemented(); }
  void icread()         { not_implemented(); }
  void isync()          { not_implemented(); }
  void lbarx()          { not_implemented(); }
  void lbdx()           { not_implemented(); }
  void lbepx()          { not_implemented(); }
  void lbzcix()         { not_implemented(); }
  void ldarx()          { not_implemented(); }
  void ldcix()          { not_implemented(); }
  void lddx()           { not_implemented(); }
  void ldepx()          { not_implemented(); }
  void lfd(const RegXMM& frt, MemoryRef m) {
    EmitDForm(50, rn(frt), rn(m.r.base), m.r.disp);
  }
  void lfddx()          { not_implemented(); }
  void lfdepx()         { not_implemented(); }
  void lfdp()           { not_implemented(); }
  void lfdpx()          { not_implemented(); }
  void lfdu()           { not_implemented(); }
  void lfdux()          { not_implemented(); }
  void lfdx(const RegXMM& rt, MemoryRef m) {
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 599);
  }
  void lfiwax()         { not_implemented(); }
  void lfiwzx()         { not_implemented(); }
  void lfsu()           { not_implemented(); }
  void lfsux()          { not_implemented(); }
  void lfsx()           { not_implemented(); }
  void lharx()          { not_implemented(); }
  void lvebx()          { not_implemented(); }
  void lvehx()          { not_implemented(); }
  void lvepx()          { not_implemented(); }
  void lvepxl()         { not_implemented(); }
  void lvewx()          { not_implemented(); }
  void lvsl()           { not_implemented(); }
  void lvsr()           { not_implemented(); }
  void lvx()            { not_implemented(); }
  void lvxl()           { not_implemented(); }
  void lwarx()          { not_implemented(); }
  void lwdx()           { not_implemented(); }
  void lwepx()          { not_implemented(); }
  void lwzcix()         { not_implemented(); }
  void lhdx()           { not_implemented(); }
  void lhepx()          { not_implemented(); }
  void lhzcix()         { not_implemented(); }
  void lqarx()          { not_implemented(); }
  void lxsdx()          { not_implemented(); }
  void lxsiwax()        { not_implemented(); }
  void lxsiwzx()        { not_implemented(); }
  void lxsspx()         { not_implemented(); }
  void lxvdsx()         { not_implemented(); }
  void lxvdx()          { not_implemented(); }
  void lxvwx()          { not_implemented(); }
  void macchw()         { not_implemented(); }
  void macchwo()        { not_implemented(); }
  void macchws()        { not_implemented(); }
  void macchwso()       { not_implemented(); }
  void macchwsu()       { not_implemented(); }
  void macchwsuo()      { not_implemented(); }
  void macchwu()        { not_implemented(); }
  void macchwuo()       { not_implemented(); }
  void machhw()         { not_implemented(); }
  void machhwo()        { not_implemented(); }
  void machhws()        { not_implemented(); }
  void machhwso()       { not_implemented(); }
  void machhwsu()       { not_implemented(); }
  void machhwsuo()      { not_implemented(); }
  void machhwu()        { not_implemented(); }
  void machhwuo()       { not_implemented(); }
  void maclhw()         { not_implemented(); }
  void maclhwo()        { not_implemented(); }
  void maclhws()        { not_implemented(); }
  void maclhwso()       { not_implemented(); }
  void maclhwsu()       { not_implemented(); }
  void maclhwsuo()      { not_implemented(); }
  void maclhwu()        { not_implemented(); }
  void maclhwuo()       { not_implemented(); }
  void mbar()           { not_implemented(); }
  void mcrfs()          { not_implemented(); }
  void mcrxr()          { not_implemented(); }
  void mfbhrbe()        { not_implemented(); }
  void mfcr(const Reg64& rt) {
    EmitXFXForm(31, rn(rt), static_cast<SpecialReg>(0), 19);
  }
  void mfdcr()          { not_implemented(); }
  void mfdcrux ()       { not_implemented(); }
  void mfdcrx()         { not_implemented(); }
  void mffs()           { not_implemented(); }
  void mfocrf()         { not_implemented(); }
  void mfpmr ()         { not_implemented(); }
  void mfsr()           { not_implemented(); }
  void mfsrin()         { not_implemented(); }
  void mftbmfvscr()     { not_implemented(); }
  void mfvsrd(const Reg64& ra, const RegXMM& xs) {
   EmitXX1Form(31, rn(xs), rn(ra), rn(0) /* reserved */, 51, 0);
  }
  void mfvsrwz()        { not_implemented(); }
  void msgclr()         { not_implemented(); }
  void msgclrp()        { not_implemented(); }
  void msgsnd()         { not_implemented(); }
  void msgsndp()        { not_implemented(); }
  void mtcrf()          { not_implemented(); }
  void mtdcr()          { not_implemented(); }
  void mtdcrux()        { not_implemented(); }
  void mtdcrx ()        { not_implemented(); }
  void mtfsb0()         { not_implemented(); }
  void mtfsb1()         { not_implemented(); }
  void mtfsf()          { not_implemented(); }
  void mtfsfi()         { not_implemented(); }
  void mtsr()           { not_implemented(); }
  void mtsrin()         { not_implemented(); }
  void mtvscr()         { not_implemented(); }
  void mtvsrd(const RegXMM& xt, const Reg64& ra) {
   EmitXX1Form(31, rn(xt), rn(ra), rn(0) /* reserved */, 179, 0);
  }
  void mtvsrwa()        { not_implemented(); }
  void mtvsrwz()        { not_implemented(); }
  void mulchw()         { not_implemented(); }
  void mulchwu()        { not_implemented(); }
  void mulhhw()         { not_implemented(); }
  void mulhhwu()        { not_implemented(); }
  void mullhw()         { not_implemented(); }
  void mullhwu()        { not_implemented(); }
  void nap()            { not_implemented(); }
  void nmacchw()        { not_implemented(); }
  void nmacchwo()       { not_implemented(); }
  void nmacchws()       { not_implemented(); }
  void nmacchwso()      { not_implemented(); }
  void nmachhw()        { not_implemented(); }
  void nmachhwo()       { not_implemented(); }
  void nmachhws()       { not_implemented(); }
  void nmachhwso()      { not_implemented(); }
  void nmaclhw()        { not_implemented(); }
  void nmaclhwo()       { not_implemented(); }
  void nmaclhws()       { not_implemented(); }
  void nmaclhwso()      { not_implemented(); }
  void rfci()           { not_implemented(); }
  void rfdi()           { not_implemented(); }
  void rfebb()          { not_implemented(); }
  void rfgi()           { not_implemented(); }
  void rfi()            { not_implemented(); }
  void rfid()           { not_implemented(); }
  void rfmci()          { not_implemented(); }
  void rvwinkle()       { not_implemented(); }
  void rvwinklesc()     { not_implemented(); }
  void slbfee()         { not_implemented(); }
  void slbia()          { not_implemented(); }
  void slbie()          { not_implemented(); }
  void slbmfee()        { not_implemented(); }
  void slbmfev()        { not_implemented(); }
  void slbmte()         { not_implemented(); }
  void sleep()          { not_implemented(); }
  void sradi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc = 0);
  void stbcix()         { not_implemented(); }
  void stbcx()          { not_implemented(); }
  void stbdx()          { not_implemented(); }
  void stbepx()         { not_implemented(); }
  void stdcix()         { not_implemented(); }
  void stdcx()          { not_implemented(); }
  void stddx()          { not_implemented(); }
  void stdepx()         { not_implemented(); }
  void stfd(const RegXMM& frt, MemoryRef m) {
    EmitDForm(54, rn(frt), rn(m.r.base), m.r.disp);
  }
  void stfddx()         { not_implemented(); }
  void stfdepx()        { not_implemented(); }
  void stfdp()          { not_implemented(); }
  void stfdpx()         { not_implemented(); }
  void stfdu()          { not_implemented(); }
  void stfdux()         { not_implemented(); }
  void stfdx(const RegXMM& rt, MemoryRef m) {
    EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 727);
  }
  void stfiwx()         { not_implemented(); }
  void stfsu()          { not_implemented(); }
  void stfsux()         { not_implemented(); }
  void stfsx()          { not_implemented(); }
  void sthcix()         { not_implemented(); }
  void sthcx()          { not_implemented(); }
  void sthdx()          { not_implemented(); }
  void sthepx()         { not_implemented(); }
  void stqcx()          { not_implemented(); }
  void stvebx()         { not_implemented(); }
  void stvehx()         { not_implemented(); }
  void stvepx()         { not_implemented(); }
  void stvepxl()        { not_implemented(); }
  void stvewx()         { not_implemented(); }
  void stvx()           { not_implemented(); }
  void stvxl()          { not_implemented(); }
  void stwcix()         { not_implemented(); }
  void stwcx()          { not_implemented(); }
  void stwdx()          { not_implemented(); }
  void stwepx()         { not_implemented(); }
  void stxsdx()         { not_implemented(); }
  void stxsiwx()        { not_implemented(); }
  void stxsspx()        { not_implemented(); }
  void stxvd2x()        { not_implemented(); }
  void sync()           { not_implemented(); }
  void tabort()         { not_implemented(); }
  void tabortdc()       { not_implemented(); }
  void tabortdci()      { not_implemented(); }
  void tabortwc()       { not_implemented(); }
  void tabortwci()      { not_implemented(); }
  void tbegin()         { not_implemented(); }
  void tcheck()         { not_implemented(); }
  void tend()           { not_implemented(); }
  void tlbia()          { not_implemented(); }
  void tlbie()          { not_implemented(); }
  void tlbiel()         { not_implemented(); }
  void tlbilx()         { not_implemented(); }
  void tlbivax()        { not_implemented(); }
  void tlbre()          { not_implemented(); }
  void tlbsrx()         { not_implemented(); }
  void tlbsx()          { not_implemented(); }
  void tlbsync()        { not_implemented(); }
  void tlbwe()          { not_implemented(); }
  void trechkpt()       { not_implemented(); }
  void treclaim()       { not_implemented(); }
  void tsr()            { not_implemented(); }
  void vaddcuq()        { not_implemented(); }
  void vaddcuw()        { not_implemented(); }
  void vaddecuq()       { not_implemented(); }
  void vaddeuqm()       { not_implemented(); }
  void vaddfp()         { not_implemented(); }
  void vaddsbs()        { not_implemented(); }
  void vaddshs()        { not_implemented(); }
  void vaddsws()        { not_implemented(); }
  void vaddubm()        { not_implemented(); }
  void vaddubs()        { not_implemented(); }
  void vaddudm()        { not_implemented(); }
  void vadduhm()        { not_implemented(); }
  void vadduhs()        { not_implemented(); }
  void vadduqm()        { not_implemented(); }
  void vadduwm()        { not_implemented(); }
  void vadduws()        { not_implemented(); }
  void vand()           { not_implemented(); }
  void vandc()          { not_implemented(); }
  void vavgsb()         { not_implemented(); }
  void vavgsh()         { not_implemented(); }
  void vavgsw()         { not_implemented(); }
  void vavgub()         { not_implemented(); }
  void vavguh()         { not_implemented(); }
  void vavguw()         { not_implemented(); }
  void vbpermq()        { not_implemented(); }
  void vcfsx()          { not_implemented(); }
  void vcfux()          { not_implemented(); }
  void vcipher()        { not_implemented(); }
  void vcipherlast()    { not_implemented(); }
  void vclzb()          { not_implemented(); }
  void vclzd()          { not_implemented(); }
  void vclzh()          { not_implemented(); }
  void vclzw()          { not_implemented(); }
  void vcmpbfp()        { not_implemented(); }
  void vcmpeqfp()       { not_implemented(); }
  void vcmpequb()       { not_implemented(); }
  void vcmpequd()       { not_implemented(); }
  void vcmpequh()       { not_implemented(); }
  void vcmpequw()       { not_implemented(); }
  void vcmpgefp()       { not_implemented(); }
  void vcmpgtfp()       { not_implemented(); }
  void vcmpgtsb()       { not_implemented(); }
  void vcmpgtsd()       { not_implemented(); }
  void vcmpgtsh()       { not_implemented(); }
  void vcmpgtsw()       { not_implemented(); }
  void vcmpgtub()       { not_implemented(); }
  void vcmpgtud()       { not_implemented(); }
  void vcmpgtuh()       { not_implemented(); }
  void vcmpgtuw()       { not_implemented(); }
  void vctsxs()         { not_implemented(); }
  void vctuxs()         { not_implemented(); }
  void veqv()           { not_implemented(); }
  void vexptefp()       { not_implemented(); }
  void vgbbd()          { not_implemented(); }
  void vlogefp()        { not_implemented(); }
  void vmaddfp()        { not_implemented(); }
  void vmaxfp()         { not_implemented(); }
  void vmaxsb()         { not_implemented(); }
  void vmaxsd()         { not_implemented(); }
  void vmaxsh()         { not_implemented(); }
  void vmaxsw()         { not_implemented(); }
  void vmaxub()         { not_implemented(); }
  void vmaxud()         { not_implemented(); }
  void vmaxuh()         { not_implemented(); }
  void vmaxuw()         { not_implemented(); }
  void vmhaddshs()      { not_implemented(); }
  void vmhraddshs()     { not_implemented(); }
  void vminfp()         { not_implemented(); }
  void vminsb()         { not_implemented(); }
  void vminsd()         { not_implemented(); }
  void vminsh()         { not_implemented(); }
  void vminsw()         { not_implemented(); }
  void vminub()         { not_implemented(); }
  void vminud()         { not_implemented(); }
  void vminuh()         { not_implemented(); }
  void vminuw()         { not_implemented(); }
  void vmladduhm()      { not_implemented(); }
  void vmrgew()         { not_implemented(); }
  void vmrghb()         { not_implemented(); }
  void vmrghh()         { not_implemented(); }
  void vmrghw()         { not_implemented(); }
  void vmrglb()         { not_implemented(); }
  void vmrglh()         { not_implemented(); }
  void vmrglw()         { not_implemented(); }
  void vmrgow()         { not_implemented(); }
  void vmsummbm()       { not_implemented(); }
  void vmsumshm()       { not_implemented(); }
  void vmsumshs()       { not_implemented(); }
  void vmsumubm()       { not_implemented(); }
  void vmsumuhm()       { not_implemented(); }
  void vmsumuhs()       { not_implemented(); }
  void vmulesb()        { not_implemented(); }
  void vmulesh()        { not_implemented(); }
  void vmulesw()        { not_implemented(); }
  void vmuleub()        { not_implemented(); }
  void vmuleuh()        { not_implemented(); }
  void vmuleuw()        { not_implemented(); }
  void vmulosb()        { not_implemented(); }
  void vmulosh()        { not_implemented(); }
  void vmulosw()        { not_implemented(); }
  void vmuloub()        { not_implemented(); }
  void vmulouh()        { not_implemented(); }
  void vmulouw()        { not_implemented(); }
  void vmuluwm()        { not_implemented(); }
  void vnand()          { not_implemented(); }
  void vncipher()       { not_implemented(); }
  void vncipherlast()   { not_implemented(); }
  void vnmsubfp()       { not_implemented(); }
  void vnor()           { not_implemented(); }
  void vor()            { not_implemented(); }
  void vorc()           { not_implemented(); }
  void vperm()          { not_implemented(); }
  void vpermxor()       { not_implemented(); }
  void vpkpx()          { not_implemented(); }
  void vpksdss()        { not_implemented(); }
  void vpksdus()        { not_implemented(); }
  void vpkshss()        { not_implemented(); }
  void vpkshus()        { not_implemented(); }
  void vpkswss()        { not_implemented(); }
  void vpkswus()        { not_implemented(); }
  void vpkudum()        { not_implemented(); }
  void vpkudus()        { not_implemented(); }
  void vpkuhum()        { not_implemented(); }
  void vpkuhus()        { not_implemented(); }
  void vpkuwum()        { not_implemented(); }
  void vpkuwus()        { not_implemented(); }
  void vpmsumb()        { not_implemented(); }
  void vpmsumd()        { not_implemented(); }
  void vpmsumh()        { not_implemented(); }
  void vpmsumw()        { not_implemented(); }
  void vpopcntb()       { not_implemented(); }
  void vpopcntd()       { not_implemented(); }
  void vpopcnth()       { not_implemented(); }
  void vpopcntw()       { not_implemented(); }
  void vrefp()          { not_implemented(); }
  void vrfim()          { not_implemented(); }
  void vrfin()          { not_implemented(); }
  void vrfip()          { not_implemented(); }
  void vrfiz()          { not_implemented(); }
  void vrlb()           { not_implemented(); }
  void vrld()           { not_implemented(); }
  void vrlh()           { not_implemented(); }
  void vrlw()           { not_implemented(); }
  void vrsqrtefp()      { not_implemented(); }
  void vsbox()          { not_implemented(); }
  void vsel()           { not_implemented(); }
  void vshasigmad()     { not_implemented(); }
  void vshasigmaw()     { not_implemented(); }
  void vsl()            { not_implemented(); }
  void vslb()           { not_implemented(); }
  void vsld()           { not_implemented(); }
  void vsldoi()         { not_implemented(); }
  void vslh()           { not_implemented(); }
  void vslo()           { not_implemented(); }
  void vslw()           { not_implemented(); }
  void vspltb()         { not_implemented(); }
  void vsplth()         { not_implemented(); }
  void vspltisb()       { not_implemented(); }
  void vspltish()       { not_implemented(); }
  void vspltisw()       { not_implemented(); }
  void vspltw()         { not_implemented(); }
  void vsr()            { not_implemented(); }
  void vsrab()          { not_implemented(); }
  void vsrad()          { not_implemented(); }
  void vsrah()          { not_implemented(); }
  void vsraw()          { not_implemented(); }
  void vsrb()           { not_implemented(); }
  void vsrd()           { not_implemented(); }
  void vsrh()           { not_implemented(); }
  void vsro()           { not_implemented(); }
  void vsrw()           { not_implemented(); }
  void vsubcuq()        { not_implemented(); }
  void vsubcuw()        { not_implemented(); }
  void vsubecuq()       { not_implemented(); }
  void vsubeuqm()       { not_implemented(); }
  void vsubfp()         { not_implemented(); }
  void vsubsbs()        { not_implemented(); }
  void vsubshs()        { not_implemented(); }
  void vsubsws()        { not_implemented(); }
  void vsububm()        { not_implemented(); }
  void vsububs()        { not_implemented(); }
  void vsubudm()        { not_implemented(); }
  void vsubuhm()        { not_implemented(); }
  void vsubuhs()        { not_implemented(); }
  void vsubuqm()        { not_implemented(); }
  void vsubuwm()        { not_implemented(); }
  void vsubuws()        { not_implemented(); }
  void vsum2sws()       { not_implemented(); }
  void vsum4sbs()       { not_implemented(); }
  void vsum4shs()       { not_implemented(); }
  void vsum4ubs()       { not_implemented(); }
  void vsumsws()        { not_implemented(); }
  void vupkhpx()        { not_implemented(); }
  void vupkhsb()        { not_implemented(); }
  void vupkhsh()        { not_implemented(); }
  void vupkhsw()        { not_implemented(); }
  void vupklpx()        { not_implemented(); }
  void vupklsb()        { not_implemented(); }
  void vupklsh()        { not_implemented(); }
  void vupklsw()        { not_implemented(); }
  void vxor()           { not_implemented(); }
  void wait()           { not_implemented(); }
  void wrtee()          { not_implemented(); }
  void wrteei()         { not_implemented(); }
  void xsabsdp()        { not_implemented(); }
  void xsadddp()        { not_implemented(); }
  void xsaddsp()        { not_implemented(); }
  void xscmpodp()       { not_implemented(); }
  void xscmpudp()       { not_implemented(); }
  void xscpsgndp()      { not_implemented(); }
  void xscvdpsp()       { not_implemented(); }
  void xscvdpspn()      { not_implemented(); }
  void xscvdpsxds(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 344, 0, 0);
  }
  void xscvdpsxws()     { not_implemented(); }
  void xscvdpuxws()     { not_implemented(); }
  void xscvspdp()       { not_implemented(); }
  void xscvspdpn()      { not_implemented(); }
  void xscvsxddp(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 376, 0, 0);
  }
  void xscvsxdsp()      { not_implemented(); }
  void xscvuxddp()      { not_implemented(); }
  void xscvuxdsp()      { not_implemented(); }
  void xsdivdp()        { not_implemented(); }
  void xsdivsp()        { not_implemented(); }
  void xsmaddadp()      { not_implemented(); }
  void xsmaddasp()      { not_implemented(); }
  void xsmaddmdp()      { not_implemented(); }
  void xsmaddmsp()      { not_implemented(); }
  void xsmaxdp()        { not_implemented(); }
  void xsmindp()        { not_implemented(); }
  void xsmsubadp()      { not_implemented(); }
  void xsmsubasp()      { not_implemented(); }
  void xsmsubmdp()      { not_implemented(); }
  void xsmsubmsp()      { not_implemented(); }
  void xsmuldp()        { not_implemented(); }
  void xsmulsp()        { not_implemented(); }
  void xsnabsdp()       { not_implemented(); }
  void xsnegdp()        { not_implemented(); }
  void xsnmaddadp()     { not_implemented(); }
  void xsnmaddasp()     { not_implemented(); }
  void xsnmaddmdp()     { not_implemented(); }
  void xsnmaddmsp()     { not_implemented(); }
  void xsnmsubadp()     { not_implemented(); }
  void xsnmsubasp()     { not_implemented(); }
  void xsnmsubmdp()     { not_implemented(); }
  void xsnmsubmsp()     { not_implemented(); }
  void xsrdpi(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 73, 0, 0);
  }
  void xsrdpic()        { not_implemented(); }
  void xsrdpim()        { not_implemented(); }
  void xsrdpip()        { not_implemented(); }
  void xsrdpiz()        { not_implemented(); }
  void xsredp()         { not_implemented(); }
  void xsresp()         { not_implemented(); }
  void xsrsp()          { not_implemented(); }
  void xsrsqrtedp()     { not_implemented(); }
  void xsrsqrtesp()     { not_implemented(); }
  void xssqrtdp(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 75, 0, 0);
  }
  void xssqrtsp()       { not_implemented(); }
  void xssubdp()        { not_implemented(); }
  void xssubsp()        { not_implemented(); }
  void xstdivdp()       { not_implemented(); }
  void xstsqrtdp()      { not_implemented(); }
  void xvabsdp()        { not_implemented(); }
  void xvabssp()        { not_implemented(); }
  void xvadddp()        { not_implemented(); }
  void xvaddsp()        { not_implemented(); }
  void xvcmpeqdp()      { not_implemented(); }
  void xvcmpeqsp()      { not_implemented(); }
  void xvcmpgedp()      { not_implemented(); }
  void xvcmpgesp()      { not_implemented(); }
  void xvcmpgtdp()      { not_implemented(); }
  void xvcmpgtsp()      { not_implemented(); }
  void xvcpsgndp()      { not_implemented(); }
  void xvcpsgnsp()      { not_implemented(); }
  void xvcvdpsp()       { not_implemented(); }
  void xvcvdpsxds()     { not_implemented(); }
  void xvcvdpsxws()     { not_implemented(); }
  void xvcvdpuxds()     { not_implemented(); }
  void xvcvdpuxws()     { not_implemented(); }
  void xvcvspdp()       { not_implemented(); }
  void xvcvspsxds(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 408, 0, 0);
  }
  void xvcvspsxws(const RegXMM& xt, const RegXMM& xb) {
   EmitXX2Form(60, rn(xt), 0, rn(xb), 152, 0, 0);
  }
  void xvcvspuxds()     { not_implemented(); }
  void xvcvspuxws()     { not_implemented(); }
  void xvcvsxddp()      { not_implemented(); }
  void xvcvsxdsp()      { not_implemented(); }
  void xvcvsxwdp()      { not_implemented(); }
  void xvcvsxwsp()      { not_implemented(); }
  void xvcvuxddp()      { not_implemented(); }
  void xvcvuxdsp()      { not_implemented(); }
  void xvcvuxwdp()      { not_implemented(); }
  void xvcvuxwsp()      { not_implemented(); }
  void xvmaddadp()      { not_implemented(); }
  void xvmaddasp()      { not_implemented(); }
  void xvmaddmdp()      { not_implemented(); }
  void xvmaddmsp()      { not_implemented(); }
  void xvmaxdp()        { not_implemented(); }
  void xvmaxsp()        { not_implemented(); }
  void xvmindp()        { not_implemented(); }
  void xvminsp()        { not_implemented(); }
  void xvmsubadp()      { not_implemented(); }
  void xvmsubasp()      { not_implemented(); }
  void xvmsubmdp()      { not_implemented(); }
  void xvmsubmsp()      { not_implemented(); }
  void xvmuldp()        { not_implemented(); }
  void xvmulsp()        { not_implemented(); }
  void xvnabsdp()       { not_implemented(); }
  void xvnabssp()       { not_implemented(); }
  void xvnegdp()        { not_implemented(); }
  void xvnegsp()        { not_implemented(); }
  void xvnmaddadp()     { not_implemented(); }
  void xvnmaddasp()     { not_implemented(); }
  void xvnmaddmdp()     { not_implemented(); }
  void xvnmaddmsp()     { not_implemented(); }
  void xvnmsubadp()     { not_implemented(); }
  void xvnmsubasp()     { not_implemented(); }
  void xvnmsubmdp()     { not_implemented(); }
  void xvnmsubmsp()     { not_implemented(); }
  void xvrdpi()         { not_implemented(); }
  void xvrdpic()        { not_implemented(); }
  void xvrdpim()        { not_implemented(); }
  void xvrdpip()        { not_implemented(); }
  void xvrdpiz()        { not_implemented(); }
  void xvredp()         { not_implemented(); }
  void xvresp()         { not_implemented(); }
  void xvrspi()         { not_implemented(); }
  void xvrspic()        { not_implemented(); }
  void xvrspim()        { not_implemented(); }
  void xvrspip()        { not_implemented(); }
  void xvrspiz()        { not_implemented(); }
  void xvrsqrtedp()     { not_implemented(); }
  void xvrsqrtesp()     { not_implemented(); }
  void xvsqrtdp()       { not_implemented(); }
  void xvsqrtsp()       { not_implemented(); }
  void xvsubdp()        { not_implemented(); }
  void xvsubsp()        { not_implemented(); }
  void xvtdivdp()       { not_implemented(); }
  void xvtdivsp()       { not_implemented(); }
  void xvtsqrtdp()      { not_implemented(); }
  void xvtsqrtsp()      { not_implemented(); }
  void xxland()         { not_implemented(); }
  void xxlandc()        { not_implemented(); }
  void xxleqv()         { not_implemented(); }
  void xxlnand()        { not_implemented(); }
  void xxlnor()         { not_implemented(); }
  void xxlor()          { not_implemented(); }
  void xxlorc()         { not_implemented(); }
  void xxlxor(const RegXMM& xt, const RegXMM& xa, const RegXMM& xb) {
   EmitXX3Form(60, rn(xt), rn(xa), rn(xb), 154, 0, 0, 0);
  }
  void xxmrghw()        { not_implemented(); }
  void xxmrglw()        { not_implemented(); }
  void xxpermdi(const RegXMM& tx, const RegXMM& xa, const RegXMM& xb) {
   EmitXX3Form(60, rn(tx), rn(xa), rn(xb),  10, 0, 0, 0);
   // Note that I decided to hardcode DM bit as 0
   // (xo field = 10), because it's sufficent for now.
   // However, I might not be the case in the future
  }
  void xxsel()          { not_implemented(); }
  void xxsldwi()        { not_implemented(); }
  void xxspltw()        { not_implemented(); }

  //Extended/Synthetic PPC64 Instructions
  void blt()            { not_implemented(); }  //Extended bc 12,0,target
  void bne()            { not_implemented(); }  //Extended bc 4,10,target
  void bdnz()           { not_implemented(); }  //Extended bc 16,0,target
  void blr() {
    BranchParams bp(BranchConditions::Always);
    bclr(bp.bo(), bp.bi(), 0);
  }
  void bltlr()          { not_implemented(); }  //Extended bclr 12,0,0
  void bnelr()          { not_implemented(); }  //Extended bclr 4,10,0
  void bdnzlr()         { not_implemented(); }  //Extended bclr 16,0,0
  void bltctr()         { not_implemented(); }  //Extended bcctr 12,0,0
  void bnectr()         { not_implemented(); }  //Extended bcctr 4,10,0
  void bctr() {
    BranchParams bp(BranchConditions::Always);
    bcctr(bp.bo(), bp.bi(), 0);
  }
  void bctrl() {
    BranchParams bp(BranchConditions::Always);
    bcctrl(bp.bo(), bp.bi(), 0);
  }
  void crmov()          { not_implemented(); }  //Extended cror Bx,By,By
  void crclr()          { not_implemented(); }  //Extended crxor Bx,Bx,BX
  void crnot()          { not_implemented(); }  //Extended crnor Bx,By,By
  void crset()          { not_implemented(); }  //Extended creqv Bx,Bx,Bx
  void li(const Reg64& rt, Immed imm) {
    addi(rt, Reg64(0), imm);
  }
  void la()             { not_implemented(); }  //Extended addi Rx,Ry,disp
  void subi(const Reg64& rt, const Reg64& ra, Immed imm) {
    addi(rt, ra, -imm);
  }
  void lis(const Reg64& rt, Immed imm) {
    addis(rt, Reg64(0), imm);
  }
  void subis()          { not_implemented(); }  //Extended addis Rx,Ry,-value
  void sub(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0) {
    subf(rt, rb, ra, rc);
  }
  void subo(const Reg64& rt, const Reg64& ra, const Reg64& rb, bool rc = 0) {
    subfo(rt, rb, ra, rc);
  }
  void subic()          { not_implemented(); }  //Extended addic Rx,Ry,-value
  void subc()           { not_implemented(); }  //Extended subfc Rx,Rz,Ry
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
  void cmpldi(const Reg64& ra, Immed imm) {
    cmpli(0, 1, ra, imm);
  }
  void cmplwi(const Reg64& ra, Immed imm) {
    //Extended cmpli 3,0,Rx,value
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmpli(0, 0, ra, imm);
  }
  void cmpld(const Reg64& ra, const Reg64& rb) {
    cmpl(0, 1, ra, rb);
  }
  void cmplw(const Reg64& ra, const Reg64& rb) {
    //Extended cmpl 3,0,Rx,Ry
    // TODO(CRField): if other CRs than 0 is used, please change this to 3
    cmpl(0, 0, ra, rb);
  }
  void twgti()          { not_implemented(); }  //Extended twi 8,Rx,value
  void twllei()         { not_implemented(); }  //Extended twi 6,Rx,value
  void tweq()           { not_implemented(); }  //Extended tw 4,Rx,Ry
  void twlge()          { not_implemented(); }  //Extended tw 5,Rx,Ry
  void trap() {
    tw(31, Reg64(0), Reg64(0));
  }
  void tdlti()          { not_implemented(); }  //Extended tdi 16,Rx,value
  void tdnei()          { not_implemented(); }  //Extended tdi 24,Rx,value
  void tdeg()           { not_implemented(); }  //Extended td 12,Rx,Ry
  void isellt()         { not_implemented(); }  //Extended isel Rx,Ry,Rz,0
  void iselgt()         { not_implemented(); }  //Extended isel Rx,Ry,Rz,1
  void iseleq()         { not_implemented(); }  //Extended isel Rx,Ry,Rz,1
  void nop() {
    ori(Reg64(0),Reg64(0),0);
  }
  void xnop()           { not_implemented(); }  //Extended
  void mr(const Reg64& rs, const Reg64& ra) {
    or(rs, ra, ra);
  }
  void not()            { not_implemented(); }  //Extended
  void srwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rlwinm(ra, rs, 32-sh, sh, 31, rc);
  }
  void slwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    /* non-existing mnemonic on ISA, but it's pratical to have it here */
    rlwinm(ra, rs, sh, 0, 31-sh, rc);
  }
  void clrwi()          { not_implemented(); }  //Extended
  void extwi()          { not_implemented(); }  //Extended
  void rotlw()          { not_implemented(); }  //Extended
  void inslwi()         { not_implemented(); }  //Extended
  void extrdi()         { not_implemented(); }  //Extended
  void srdi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicl(ra, rs, 64-sh, sh, rc);
  }
  void clrldi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicl(ra, rs, 0, sh, rc);
  }
  void extldi()         { not_implemented(); }  //Extended
  void sldi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rldicr(ra, rs, sh, 63-sh, rc);
  }
  void clrrdi()         { not_implemented(); }  //Extended
  void clrrwi(const Reg64& ra, const Reg64& rs, int8_t sh, bool rc = 0) {
    rlwinm(ra, rs, 0, 0, 31-sh, rc);
  }
  void clrlsldi()       { not_implemented(); }  //Extended
  void rotld()          { not_implemented(); }  //Extended
  void insrdi()         { not_implemented(); }  //Extended
  void mtcr()           { not_implemented(); }  //Extended
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
  void b(Label& l);
  void ba(Label& l);
  void bl(Label& l);
  void bla(Label& l);

  void bc(Label& l, BranchConditions bc);
  void bc(Label& l, ConditionCode cc);
  void bca(Label& l, BranchConditions bc);
  void bcl(Label& l, BranchConditions bc);
  void bcla(Label& l, BranchConditions bc);

  void branchAuto(Label& l,
                  BranchConditions bc = BranchConditions::Always,
                  LinkReg lr = LinkReg::DoNotTouch);

  void branchAuto(CodeAddress c,
                  BranchConditions bc = BranchConditions::Always,
                  LinkReg lr = LinkReg::DoNotTouch);

  void branchAuto(CodeAddress c,
                  ConditionCode cc,
                  LinkReg lr = LinkReg::DoNotTouch);

  // ConditionCode variants
  void bc(ConditionCode cc, int16_t address) {
    BranchParams bp(cc);
    bc(bp.bo(), bp.bi(), address);
  }

  // Auxiliary for loading a complete 64bits immediate into a register
  void li64 (const Reg64& rt, int64_t imm64);

  // Create prologue when calling.
  void prologue (const Reg64& rsp, const Reg64& rfuncln, const Reg64& rvmfp);

  // Create epilogue when calling.
  void epilogue (const Reg64& rsp, const Reg64& rfuncln);

  void call (const Reg64& rsp,
             const Reg64& rfuncln,
             const Reg64& rvmfp,
             CodeAddress target);

  // checks if the @inst is pointing to a call
  static inline bool isCall(HPHP::jit::TCA inst) {
    // a call always begin with a mflr and it's rarely used elsewhere: good tag
    return ((((inst[3] >> 2) & 0x3F) == 31) &&                          // OPCD
      ((((inst[1] & 0x3) << 7) | ((inst[0] >> 1) & 0xFF)) == 339));     // XO
  }

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

  // Auxiliary for loading a 32bits unsigned immediate into a register
  void li32un (const Reg64& rt, uint32_t imm32);

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

  //Can be used to generate or force a unimplemented opcode exception
  void unimplemented();

  static void patchBc(CodeAddress jmp, CodeAddress dest) {
    // Opcode located at the 6 most significant bits
    assert(((jmp[3] >> 2) & 0x3F) == 16);  // B-Form
    ssize_t diff = dest - jmp;
    assert(HPHP::jit::deltaFits(diff, HPHP::sz::word) &&
        "Patching offset is too big");
    int16_t* BD = (int16_t*)(jmp);    // target address location in instruction

    // Keep AA and LK values
    *BD = static_cast<int16_t>(diff & 0xFFFC) | ((*BD) & 0x3);
  }

  static void patchBctr(CodeAddress jmp, CodeAddress dest) {
    // Check Label::branchAuto for details
    HPHP::CodeBlock cb2;

#ifndef NDEBUG  // avoid "unused variable 'bctr_addr'" warning on release build
    // skips the li64 and a mtctr instruction
    CodeAddress bctr_addr = jmp + kLi64InstrLen + 1 * kBytesPerInstr;
    // check for instruction opcode
    assert(((bctr_addr[3] >> 2) & 0x3F) == 19);
#endif

    // Initialize code block cb2 pointing to li64
    cb2.init(jmp, kLi64InstrLen, "patched bctr");
    Assembler b{ cb2 };
    b.li64(reg::r12, ssize_t(dest));
  }

  void emitNop(int nbytes) {
    assert((nbytes % 4 == 0) && "This arch supports only 4 bytes alignment");
    for (; nbytes > 0; nbytes -= 4)
      nop();
  }

  // Secure high level instruction emitter
  void Emit(PPC64Instr instruction){
    assert(codeBlock.canEmit(sizeof(instruction)));
    assert(sizeof(instruction) == sizeof(uint32_t));
    dword(instruction);
  }

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

   void EmitDSForm(const uint8_t op,
                   const RegNumber rt,
                   const RegNumber ra,
                   const uint16_t imm,
                   const uint16_t xop) {

     // GP Register cannot be greater than 31
     assert(static_cast<uint32_t>(ra) < 32);
     assert(static_cast<uint32_t>(rt) < 32);

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
  void EmitXFLForm()  { not_implemented(); }
  void EmitXX4Form()  { not_implemented(); }
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
  void EmitVAForm()   { not_implemented(); }
  void EmitVCForm()   { not_implemented(); }
  void EmitZ23Form()  { not_implemented(); }
  void EmitEVXForm()  { not_implemented(); }
  void EmitEVSForm()  { not_implemented(); }
  void EmitZ22Form()  { not_implemented(); }

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

//////////////////////////////////////////////////////////////////////
// Branches
//////////////////////////////////////////////////////////////////////

struct Label {
  Label() : m_a(nullptr) , m_address(nullptr) {}
  /* implicit */ Label(CodeAddress predefined) : m_a(nullptr) ,
                                                 m_address(predefined) {}

  ~Label() {
    if (!m_toPatch.empty()) {
      assert(m_a && m_address && "Label had jumps but was never set");
    }
    for (auto& ji : m_toPatch) {
      switch (ji.type) {
      case BranchType::bc:
        ji.a->patchBc(ji.addr, m_address);
        break;
      case BranchType::bctr:
        ji.a->patchBctr(ji.addr, m_address);
        break;
      }
    }
  }

  Label(const Label&) = delete;
  Label& operator=(const Label&) = delete;

  void branchOffset(Assembler& a,
                    BranchConditions bc,
                    LinkReg lr) {
    BranchParams bp(bc);
    addJump(&a, BranchType::bc);

    int16_t offset;
    if (m_address) {
      ssize_t diff = ssize_t(m_address - a.frontier());
      assert(HPHP::jit::deltaFits(diff, HPHP::sz::word) && "Offset too big");

      offset = diff;
    } else {
      // offset will be redefined on patchBc()
      offset = ssize_t(a.frontier());
    }

    // TODO(gut): Use a typedef or something to avoid copying code like below:
    if (LinkReg::Save == lr)
      a.bcl(bp.bo(), bp.bi(), offset);
    else
      a.bc(bp.bo(), bp.bi(), offset);
  }

  void branchAbsolute(Assembler& a,
                    BranchConditions bc,
                    LinkReg lr) {
    BranchParams bp(bc);
    addJump(&a, BranchType::bc);

    // Address is going to be redefined on patchBc()
    const ssize_t address = ssize_t(m_address ? m_address : a.frontier());
    assert(HPHP::jit::deltaFits(address, HPHP::sz::word) && "Address too big");

    // TODO(gut): Use a typedef or something to avoid copying code like below:
    if (LinkReg::Save == lr)
      a.bcla(bp.bo(), bp.bi(), uint16_t(address));
    else
      a.bca(bp.bo(), bp.bi(), uint16_t(address));
  }

  void branchAuto(Assembler& a, BranchConditions bc, LinkReg lr) {
    // use CTR to perform absolute branch
    BranchParams bp(bc);
    const ssize_t address = ssize_t(m_address);
    // Use reserved function linkage register
    addJump(&a, BranchType::bctr);  // marking THIS address for patchBctr
    a.li64(reg::r12, address);
    // When branching to another context, r12 need to keep the target address
    // to correctly set r2 (TOC reference).
    a.mtctr(reg::r12);
    if (LinkReg::Save == lr)
      a.bcctrl(bp.bo(), bp.bi(), 0);
    else
      a.bcctr(bp.bo(), bp.bi(), 0);
  }

  void asm_label(Assembler& a) {
    assert(!m_address && !m_a && "Label was already set");
    m_a = &a;
    m_address = a.frontier();
  }

private:
  enum class BranchType {
    bc,
    bctr
  };

  struct JumpInfo {
    BranchType type;
    Assembler* a;
    CodeAddress addr;
  };

private:
  void addJump(Assembler* a, BranchType type) {
    if (m_address) return;
    JumpInfo info;
    info.type = type;
    info.a = a;
    info.addr = a->codeBlock.frontier();
    m_toPatch.push_back(info);
  }

private:
  Assembler* m_a;
  CodeAddress m_address;
  std::vector<JumpInfo> m_toPatch;
};

/* Simplify to conditional branch that always branch */
// TODO: implement also a patchB if b is not a mnemonic to bc
inline void Assembler::b(Label& l)  { bc(l, BranchConditions::Always); }
inline void Assembler::ba(Label& l) { bca(l, BranchConditions::Always); }
inline void Assembler::bl(Label& l)  { bcl(l, BranchConditions::Always); }
inline void Assembler::bla(Label& l) { bcla(l, BranchConditions::Always); }


inline void Assembler::bc(Label& l, BranchConditions bc) {
  l.branchOffset(*this, bc, LinkReg::DoNotTouch);
}
inline void Assembler::bc(Label& l, ConditionCode cc) {
  l.branchOffset(*this, BranchParams::convertCC(cc), LinkReg::DoNotTouch);
}
inline void Assembler::bca(Label& l, BranchConditions bc) {
  l.branchAbsolute(*this, bc, LinkReg::DoNotTouch);
}
inline void Assembler::bcl(Label& l, BranchConditions bc) {
  l.branchOffset(*this, bc, LinkReg::Save);
}
inline void Assembler::bcla(Label& l, BranchConditions bc) {
  l.branchAbsolute(*this, bc, LinkReg::Save);
}

inline void Assembler::branchAuto(Label& l,
                                  BranchConditions bc,
                                  LinkReg lr) {
  l.branchAuto(*this, bc, lr);
}
inline void Assembler::branchAuto(CodeAddress c,
                                  BranchConditions bc,
                                  LinkReg lr) {
  Label l(c);
  l.branchAuto(*this, bc, lr);
}
inline void Assembler::branchAuto(CodeAddress c,
                                  ConditionCode cc,
                                  LinkReg lr) {
  branchAuto(c, BranchParams::convertCC(cc), lr);
}

} // namespace ppc64_asm

#endif
