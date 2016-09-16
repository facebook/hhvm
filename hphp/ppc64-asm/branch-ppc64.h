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

#ifndef incl_HPHP_PPC64_ASM_BRANCH_PPC64_H_
#define incl_HPHP_PPC64_ASM_BRANCH_PPC64_H_

#include "hphp/util/asm-x64.h"

#include "hphp/ppc64-asm/isa-ppc64.h"

namespace ppc64_asm {

/* using override */ using HPHP::jit::ConditionCode;

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
  LessThan                    = CR0_LessThan,
  LessThanEqual               = CR0_LessThanEqual,
  GreaterThan                 = CR0_GreaterThan,
  GreaterThanEqual            = CR0_GreaterThanEqual,
  LessThan_Unsigned           = CR1_LessThan,
  LessThanEqual_Unsigned      = CR1_LessThanEqual,
  GreaterThan_Unsigned        = CR1_GreaterThan,
  GreaterThanEqual_Unsigned   = CR1_GreaterThanEqual,
  Equal                       = CR0_Equal,
  NotEqual                    = CR0_NotEqual,
  Overflow                    = CR0_Overflow,
  NoOverflow                  = CR0_NoOverflow
};

#undef BRANCHES

struct BranchParams {
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

    /* Constructor auxiliary */
    void defineBoBi(BranchConditions bc) {
      switch (bc) {
      /* Signed comparison */
      case BranchConditions::LessThan:
        m_bo = BO::CRSet;
        m_bi = BI::CR0_LessThan;
        break;
      case BranchConditions::LessThanEqual:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR0_GreaterThan;
        break;
      case BranchConditions::GreaterThan:
        m_bo = BO::CRSet;
        m_bi = BI::CR0_GreaterThan;
        break;
      case BranchConditions::GreaterThanEqual:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR0_LessThan;
        break;

      /* Unsigned comparison */
      case BranchConditions::LessThan_Unsigned:
        m_bo = BO::CRSet;
        m_bi = BI::CR1_LessThan;
        break;
      case BranchConditions::LessThanEqual_Unsigned:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR1_GreaterThan;
        break;
      case BranchConditions::GreaterThan_Unsigned:
        m_bo = BO::CRSet;
        m_bi = BI::CR1_GreaterThan;
        break;
      case BranchConditions::GreaterThanEqual_Unsigned:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR1_LessThan;
        break;

      case BranchConditions::Equal:
        m_bo = BO::CRSet;
        m_bi = BI::CR0_Equal;
        break;
      case BranchConditions::NotEqual:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR0_Equal;
        break;
      case BranchConditions::Overflow:
        m_bo = BO::CRSet;
        m_bi = BI::CR0_SummaryOverflow;
        break;
      case BranchConditions::NoOverflow:
        m_bo = BO::CRNotSet;
        m_bi = BI::CR0_SummaryOverflow;
        break;
      case BranchConditions::Always:
        m_bo = BO::Always;
        m_bi = BI(0);
        break;

      default:
          not_implemented();
      }
      m_lr = false;
    }
#undef SWITCHES

  public:
    BranchParams() = delete;

    BranchParams(BranchConditions bc) { defineBoBi(bc); }
    BranchParams(ConditionCode cc)    { defineBoBi(convertCC(cc)); }

    static BranchConditions convertCC(ConditionCode cc) {
      BranchConditions ret = BranchConditions::Always;

      switch (cc) {
        case HPHP::jit::CC_O:
          ret = BranchConditions::Overflow;
          break;
        case HPHP::jit::CC_NO:
          ret = BranchConditions::NoOverflow;
          break;
        case HPHP::jit::CC_B:
          ret = BranchConditions::LessThan_Unsigned;
          break;
        case HPHP::jit::CC_AE:
          ret = BranchConditions::GreaterThanEqual_Unsigned;
          break;
        case HPHP::jit::CC_E:
          ret = BranchConditions::Equal;
          break;
        case HPHP::jit::CC_NE:
          ret = BranchConditions::NotEqual;
          break;
        case HPHP::jit::CC_BE:
          ret = BranchConditions::LessThanEqual_Unsigned;
          break;
        case HPHP::jit::CC_A:
          ret = BranchConditions::GreaterThan_Unsigned;
          break;
        case HPHP::jit::CC_S:
          ret = BranchConditions::LessThan;
          break;
        case HPHP::jit::CC_NS:
          ret = BranchConditions::GreaterThan;
          break;

        /*
         * Parity works only for unordered double comparison
         * Todo: fixed point comparison parity
         * http://stackoverflow.com/q/32319673/5013070
         */
        case HPHP::jit::CC_P:
          ret = BranchConditions::Overflow;
          break;
        case HPHP::jit::CC_NP:
          ret = BranchConditions::NoOverflow;
          break;

        case HPHP::jit::CC_L:
          ret = BranchConditions::LessThan;
          break;
        case HPHP::jit::CC_NL:
          ret = BranchConditions::GreaterThanEqual;
          break;
        case HPHP::jit::CC_NG:
          ret = BranchConditions::LessThanEqual;
          break;
        case HPHP::jit::CC_G:
          ret = BranchConditions::GreaterThan;
          break;

        case HPHP::jit::CC_None:
          ret = BranchConditions::Always;
          break;

        default:
          not_implemented();
          break;
      }
      return ret;
    }

    /*
     * Get the BranchParams from an emitted conditional branch
     * Also set m_lr accordingly.
     */
    BranchParams(const PPC64Instr* const pinstr) { decodeInstr(pinstr); }
    BranchParams(const uint8_t* const pinstr) {
      decodeInstr(reinterpret_cast<const PPC64Instr* const>(pinstr));
    }

    void decodeInstr(const PPC64Instr* const pinstr);

    ~BranchParams() {}

    /*
     * Converts to ConditionCode upon casting to it
     */
    /* implicit */ operator ConditionCode() {
      ConditionCode ret = HPHP::jit::CC_None;

      switch (m_bi) {
        case BI::CR0_LessThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_L;  // CC_S
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_NL;
          break;
        case BI::CR0_GreaterThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_G;  // CC_NS
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_NG;
          break;
        case BI::CR1_LessThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_B;  // CC_S
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_AE;
          break;
        case BI::CR1_GreaterThan:
          if (m_bo == BO::CRSet)          ret = HPHP::jit::CC_A;  // CC_NS
          else if (m_bo == BO::CRNotSet)  ret = HPHP::jit::CC_BE;
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

    explicit operator BranchConditions() {
      return convertCC(static_cast<ConditionCode>(*this));
    }

    uint8_t bo()    { return (uint8_t)m_bo; }
    uint8_t bi()    { return (uint8_t)m_bi; }
    bool savesLR()  { return m_lr;          }

  private:
    BranchParams::BO m_bo;
    BranchParams::BI m_bi;
    bool m_lr;
};

} // namespace ppc64_asm

#endif
