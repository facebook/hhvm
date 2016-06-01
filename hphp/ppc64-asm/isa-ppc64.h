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

#ifndef INCLUDE_PPC64_ISA_H_
#define INCLUDE_PPC64_ISA_H_

namespace ppc64_asm {

typedef uint32_t PPC64Instr;

enum class Form {
  kInvalid = 0,
  kX,
  kXO,
  kXS,
  kD,
  kI,
  kB,
  kSC,
  kDS,
  kDQ,
  kXL,
  kXFX,
  kXFL,
  kXX1,
  kXX2,
  kXX3,
  kXX4,
  kA,
  kM,
  kMD,
  kMDS,
  kVA,
  kVC,
  kVX,
  kEVX,
  kEVS,
  kZ22,
  kZ23,
};

/**
 * Instruction Format encoders
 */
typedef union XO_format {
  struct {
    uint32_t Rc:1;
    uint32_t XO:9;
    uint32_t OE:1;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XO_form_t;
static_assert(sizeof(XO_format) == sizeof(uint32_t), "XO_form_t size != 4");

typedef union X_format {
  struct {
    uint32_t Rc:1;
    uint32_t XO:10;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} X_form_t;
static_assert(sizeof(X_format) == sizeof(uint32_t), "X_form_t size != 4");

typedef union D_format {
  struct {
    uint32_t D:16;
    uint32_t RA:5;
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} D_form_t;
static_assert(sizeof(D_format) == sizeof(uint32_t), "D_form_t size != 4");

typedef union I_format {
  struct {
    uint32_t LK:1;
    uint32_t AA:1;
    uint32_t LI:24;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} I_form_t;
static_assert(sizeof(I_format) == sizeof(uint32_t), "I_form_t size != 4");

typedef union B_format {
  struct {
    uint32_t LK:1;
    uint32_t AA:1;
    uint32_t BD:14;
    uint32_t BI:5;
    uint32_t BO:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} B_form_t;
static_assert(sizeof(B_format) == sizeof(uint32_t), "B_form_t size != 4");

typedef union SC_format {
  struct {
    uint32_t B31:1;
    uint32_t B30:1;
    uint32_t LEV:7;
    uint32_t RSV3:4;
    uint32_t RSV2:5;
    uint32_t RSV1:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} SC_form_t;
static_assert(sizeof(SC_format) == sizeof(uint32_t), "SC_form_t size != 4");

typedef union DS_format {
  struct {
    uint32_t XO:2;
    uint32_t DS:14;
    uint32_t RA:5;
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} DS_form_t;
static_assert(sizeof(DS_format) == sizeof(uint32_t), "DS_form_t size != 4");

typedef union DQ_format {
  struct {
    uint32_t RSV:4;
    uint32_t DQ:12;
    uint32_t RA:5;
    uint32_t RTp:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} DQ_form_t;
static_assert(sizeof(DQ_format) == sizeof(uint32_t), "DQ_form_t size != 4");

typedef union XL_form {
  struct {
    uint32_t LK:1;
    uint32_t XO:10;
    uint32_t BB:5;
    uint32_t BA:5;
    uint32_t BT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XL_form_t;
static_assert(sizeof(XL_form) == sizeof(uint32_t), "XL_form size != 4");

typedef union XFX_format {
  struct {
    uint32_t RSV:1;
    uint32_t XO:10;
    uint32_t SPRlo:5;  // see note 1: the order of the two 5-bit halves
    uint32_t SPRhi:5;  // of the SPR number is reversed
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XFX_form_t;
static_assert(sizeof(XFX_format) == sizeof(uint32_t), "XFX_form_t size != 4");

typedef union XFL_format {
  struct {
    uint32_t Rc:1;
    uint32_t XO:10;
    uint32_t FRB:5;
    uint32_t W:1;
    uint32_t FLM:8;
    uint32_t L:1;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XFL_form_t;
static_assert(sizeof(XFL_format) == sizeof(uint32_t), "XFL_form_t size != 4");

typedef union XX1_format {
  struct {
    uint32_t TX:1;
    uint32_t XO:10;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t T:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XX1_form_t;
static_assert(sizeof(XX1_format) == sizeof(uint32_t), "XX1_form_t size != 4");

typedef union XX2_format {
  struct {
    uint32_t TX:1;
    uint32_t BX:1;
    uint32_t XO:9;
    uint32_t B:5;
    uint32_t RSV:5;
    uint32_t T:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XX2_form_t;
static_assert(sizeof(XX2_format) == sizeof(uint32_t), "XX2_form_t size != 4");

typedef union XX3_format {
  struct {
    uint32_t TX:1;
    uint32_t BX:1;
    uint32_t AX:1;
    uint32_t XO:8;
    uint32_t B:5;
    uint32_t A:5;
    uint32_t T:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XX3_form_t;
static_assert(sizeof(XX3_format) == sizeof(uint32_t), "XX3_form_t size != 4");

typedef union XX4_format {
  struct {
    uint32_t TX:1;
    uint32_t BX:1;
    uint32_t AX:1;
    uint32_t CX:1;
    uint32_t XO:2;
    uint32_t C:5;
    uint32_t B:5;
    uint32_t A:5;
    uint32_t T:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XX4_form_t;
static_assert(sizeof(XX4_format) == sizeof(uint32_t), "XX4_form_t size != 4");

typedef union XS_format {
  struct {
    uint32_t Rc:1;
    uint32_t sh:1;
    uint32_t XO:9;
    uint32_t SH:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} XS_form_t;
static_assert(sizeof(XS_format) == sizeof(uint32_t), "XS_form_t size != 4");

typedef union A_format {
  struct {
    uint32_t Rc:1;
    uint32_t XO:5;
    uint32_t FRC:5;
    uint32_t FRB:5;
    uint32_t FRA:5;
    uint32_t FRT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} A_form_t;
static_assert(sizeof(A_format) == sizeof(uint32_t), "A_form_t size != 4");

typedef union M_format {
  struct {
    uint32_t Rc:1;
    uint32_t ME:5;
    uint32_t MB:5;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} M_form_t;
static_assert(sizeof(M_format) == sizeof(uint32_t), "M_form_t size != 4");

typedef union MD_format {
  struct {
    uint32_t Rc:1;
    uint32_t sh:1;
    uint32_t XO:3;
    uint32_t MB:6;
    uint32_t SH:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} MD_form_t;
static_assert(sizeof(MD_format) == sizeof(uint32_t), "MD_form_t size != 4");

typedef union MDS_format {
  struct {
    uint32_t Rc:1;
    uint32_t XO:4;
    uint32_t MB:6;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} MDS_form_t;
static_assert(sizeof(MDS_format) == sizeof(uint32_t), "MDS_form_t size != 4");

typedef union VA_format {
  struct {
    uint32_t XO:6;
    uint32_t VRC:5;
    uint32_t VRB:5;
    uint32_t VRA:5;
    uint32_t VRT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} VA_form_t;
static_assert(sizeof(VA_format) == sizeof(uint32_t), "VA_form_t size != 4");

typedef union VC_format {
  struct {
    uint32_t XO:10;
    uint32_t Rc:1;
    uint32_t VRB:5;
    uint32_t VRA:5;
    uint32_t VRT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} VC_form_t;
static_assert(sizeof(VC_format) == sizeof(uint32_t), "VC_form_t size != 4");

typedef union VX_format {
  struct {
    uint32_t XO:11;
    uint32_t VRB:5;
    uint32_t VRA:5;
    uint32_t VRT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} VX_form_t;
static_assert(sizeof(VX_format) == sizeof(uint32_t), "VX_form_t size != 4");

typedef union EVX_format {
  struct {
    uint32_t XO:11;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} EVX_form_t;
static_assert(sizeof(EVX_format) == sizeof(uint32_t), "EVX_form_t size != 4");

typedef union EVS_format {
  struct {
    uint32_t BFA:3;
    uint32_t XO:8;
    uint32_t RB:5;
    uint32_t RA:5;
    uint32_t RS:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} EVS_form_t;
static_assert(sizeof(EVS_format) == sizeof(uint32_t), "EVS_form_t size != 4");

typedef union Z22_format {
  struct {
    //TODO:
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} Z22_form_t;
static_assert(sizeof(Z22_format) == sizeof(uint32_t), "Z22_form_t size != 4");

typedef union Z23_format {
  struct {
    //TODO:
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} Z23_form_t;
static_assert(sizeof(Z23_format) == sizeof(uint32_t), "Z23_form_t size != 4");

} // namespace ppc64_asm

#endif
