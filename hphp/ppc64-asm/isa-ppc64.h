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

#ifndef INCLUDE_PPC64_ISA_H_
#define INCLUDE_PPC64_ISA_H_

#include <cstdint>

namespace ppc64_asm {

typedef uint32_t PPC64Instr;

const int kDecoderSize = 1374;

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

// Instruction type decoder masks
const uint32_t kDecoderMask1  = (0x3F << 26);
const uint32_t kDecoderMask2  = (0x3F << 26) | (0x1);
const uint32_t kDecoderMask3  = (0x3F << 26) | (0x3);
const uint32_t kDecoderMask4  = (0x3F << 26) | (0x1 << 1);
const uint32_t kDecoderMask5  = (0x3F << 26) | (0x3FF << 1) | (0x1);
const uint32_t kDecoderMask6  = (0x3F << 26) | (0x1FF << 2);
const uint32_t kDecoderMask7  = (0x3F << 26) | (0x1F << 2) | (0x1);
const uint32_t kDecoderMask8  = (0x3F << 26) | (0x1 << 2) | (0x1 << 1);
const uint32_t kDecoderMask9  = (0x3F << 26) | (0x3FF << 1);
const uint32_t kDecoderMask10 = (0x3F << 26) | (0x3FF << 1) | (0x1 << 20);
const uint32_t kDecoderMask11 = (0x3F << 26) | (0xFF << 3);
const uint32_t kDecoderMask12 = (0x3F << 26) | (0x1 << 10) | (0xF << 3);
const uint32_t kDecoderMask13 = (0x3F << 26) | (0x3 << 4);
const uint32_t kDecoderMask14 = (0x3F << 26) | (0x1F << 1);
const uint32_t kDecoderMask15 = (0x3F << 26) | (0x7 << 2) | (0x1);
const uint32_t kDecoderMask16 = (0x3F << 26) | (0xF << 1) | (0x1);
const uint32_t kDecoderMask17 = (0x3F << 26) | (0x3F);
const uint32_t kDecoderMask18 = (0x3F << 26) | (0x1 << 10) | (0x3FF);
const uint32_t kDecoderMask19 = (0x3F << 26) | (0x7FF);
const uint32_t kDecoderMask20 = (0x3F << 26) | (0x1FF << 1) | (0x1);
const uint32_t kDecoderMask21 = (0x3F << 26) | (0xFF << 1) | (0x1);

const uint32_t kDecoderListSize = 21;
// Decoder List
const uint32_t DecoderList[] = { kDecoderMask1, kDecoderMask2,   kDecoderMask3,
                                 kDecoderMask4, kDecoderMask5,   kDecoderMask6,
                                 kDecoderMask7, kDecoderMask8,   kDecoderMask9,
                                 kDecoderMask10, kDecoderMask11, kDecoderMask12,
                                 kDecoderMask13, kDecoderMask14, kDecoderMask15,
                                 kDecoderMask16, kDecoderMask17, kDecoderMask18,
                                 kDecoderMask19, kDecoderMask20, kDecoderMask21
                               };

/*
  Defines operands mask and type for a instruction. The flags can be
  used to encode information about a operand like kind of register or
  if a immediate will be print as a signed or unsigned value.
*/
struct Operands {
  uint32_t mask_;
  uint32_t flags_;

  Operands(uint32_t mask, uint32_t flags)
  : mask_(mask)
  , flags_(flags)
  {}
  Operands()
  : mask_(0)
  , flags_(0)
  {}
};

// TODO(rcardoso): Define operand masks
#define A         { 0x0, 0x0 }
#define A_L       { 0x0, 0x0 }
#define BA        { 0x001f0000, 0x0 }
#define BB        { 0x0000f800, 0x0 }
#define BD        { 0x0000fffc, 0x0 }
#define BDA       { 0x0, 0x0 }
#define BF        { 0x0, 0x0 }
#define BFA       { 0x0, 0x0 }
#define BFF       { 0x0, 0x0 }
#define BH        { 0x00001800, 0x0 }
#define BHRBE     { 0x0, 0x0 }
#define BI        { 0x001f0000, 0x0 }
#define BO        { 0x03e00000, 0x0 }
#define BT        { 0x03e00000, 0x0 }
#define CRFD      { 0x0, 0x0 }
#define CRB       { 0x0, 0x0 }
#define CT        { 0x0, 0x0 }
#define D         { 0x0, 0x0 }
#define DCM       { 0x0, 0x0 }
#define DGM       { 0x0, 0x0 }
#define DM        { 0x0, 0x0 }
#define DQ        { 0x0, 0x0 }
#define DS        { 0x0, 0x0 }
#define DUI       { 0x0, 0x0 }
#define DUIS      { 0x0, 0x0 }
#define E         { 0x0, 0x0 }
#define EH        { 0x0, 0x0 }
#define EVUIMM    { 0x0, 0x0 }
#define EVUIMM_2  { 0x0, 0x0 }
#define EVUIMM_4  { 0x0, 0x0 }
#define EVUIMM_8  { 0x0, 0x0 }
#define FLM       { 0x0, 0x0 }
#define FRA       { 0x0, 0x0 }
#define FRB       { 0x0, 0x0 }
#define FRC       { 0x0, 0x0 }
#define FRS       { 0x0, 0x0 }
#define FRT       { 0x0, 0x0 }
#define FXM       { 0x0, 0x0 }
#define L         { 0x0, 0x0 }
#define LEV       { 0x0, 0x0 }
#define LI        { 0x0, 0x0 }
#define LIA       { 0x0, 0x0 }
#define LS        { 0x0, 0x0 }
#define MB6       { 0x0, 0x0 }
#define ME6       { 0x0, 0x0 }
#define MBE       { 0x0, 0x0 }
#define ME        { 0x0, 0x0 }
#define MO        { 0x0, 0x0 }
#define NB        { 0x0, 0x0 }
#define OC        { 0x0, 0x0 }
#define PMR       { 0x0, 0x0 }
#define PS        { 0x0, 0x0 }
#define R         { 0x0, 0x0 }
#define RA        { 0x0, 0x0 }
#define RA0       { 0x0, 0x0 }
#define RAL       { 0x0, 0x0 }
#define RAM       { 0x0, 0x0 }
#define RAOPT     { 0x0, 0x0 }
#define RAQ       { 0x0, 0x0 }
#define RAS       { 0x0, 0x0 }
#define RB        { 0x0, 0x0 }
#define RMC       { 0x0, 0x0 }
#define RS        { 0x0, 0x0 }
#define RSO       { 0x0, 0x0 }
#define RSQ       { 0x0, 0x0 }
#define RT        { 0x0, 0x0 }
#define RTO       { 0x0, 0x0 }
#define RTQ       { 0x0, 0x0 }
#define S         { 0x0, 0x0 }
#define SH        { 0x0, 0x0 }
#define SH16      { 0x0, 0x0 }
#define SH6       { 0x0, 0x0 }
#define SHB       { 0x0, 0x0 }
#define SHO       { 0x0, 0x0 }
#define SHW       { 0x0, 0x0 }
#define SI        { 0x0, 0x0 }
#define SIMM      { 0x0, 0x0 }
#define SISIGNOPT { 0x0, 0x0 }
#define SIX       { 0x0, 0x0 }
#define SP        { 0x0, 0x0 }
#define SPR       { 0x0, 0x0 }
#define SR        { 0x0, 0x0 }
#define ST        { 0x0, 0x0 }
#define TBR       { 0x0, 0x0 }
#define TE        { 0x0, 0x0 }
#define TH        { 0x0, 0x0 }
#define TO        { 0x0, 0x0 }
#define U         { 0x0, 0x0 }
#define UI        { 0x0, 0x0 }
#define UIMM      { 0x0, 0x0 }
#define UN        { 0x0, 0x0 }
#define VA        { 0x0, 0x0 }
#define VB        { 0x0, 0x0 }
#define VC        { 0x0, 0x0 }
#define VD        { 0x0, 0x0 }
#define VS        { 0x0, 0x0 }
#define W         { 0x0, 0x0 }
#define WC        { 0x0, 0x0 }
#define XA        { 0x0, 0x0 }
#define XB        { 0x0, 0x0 }
#define XC        { 0x0, 0x0 }
#define XFL_L     { 0x0, 0x0 }
#define XFL_L     { 0x0, 0x0 }
#define XS        { 0x0, 0x0 }
#define XT        { 0x0, 0x0 }

struct DecoderInfo {
private:
    uint32_t opcode_;
    Form form_;
    std::string mnemonic_;
    uint32_t operand_size_;
    Operands* operand_list_;
    DecoderInfo* next_;
public:
  DecoderInfo(uint32_t op, Form form, std::string mn ,
    std::initializer_list<Operands> oper)
  : opcode_(op)
  , form_(form)
  , mnemonic_(mn)
  , operand_size_(oper.size())
  , next_(nullptr){
    operand_list_ = new Operands[oper.size()];
    for(auto i = oper.begin(); i != oper.end(); i++)
       operand_list_[i - oper.begin()] = *i;
  }

  ~DecoderInfo(){
    delete next_;
    next_ = nullptr;
  }

  DecoderInfo() = delete;

  inline Form form() const { return form_; }
  inline uint32_t opcode() const { return opcode_; }
  inline std::string mnemonic() const { return mnemonic_; }
  void next(DecoderInfo* i) { next_ = i;}
  DecoderInfo* next() { return next_; }

  inline bool operator==(const DecoderInfo& i) {
    return (i.form() == form_ &&
            i.opcode() == opcode_ &&
            i.mnemonic() == mnemonic_);
  }

  inline bool operator!=(const DecoderInfo& i) {
    return (i.form() != form_ ||
            i.opcode() != opcode_ ||
            i.mnemonic() != mnemonic_);
  }
};

class DecoderTable {
public:
  DecoderTable() {
    decoder_table = new DecoderInfo*[kDecoderSize];
    for(int i = 0; i < kDecoderSize; i++)
      decoder_table[i] = nullptr;

#define DE(name, op, type, mnemonic, ... )\
  DecoderInfo instr_##name {op, type, #mnemonic, __VA_ARGS__ };\
  SetInstruction(instr_##name)

  DE(add,           0x7C000214, Form::kXO,  add,         { RT, RA, RB });
  DE(adddot,        0x7C000215, Form::kXO,  add.,        { RT, RA, RB });
  DE(addc,          0x7C000014, Form::kXO,  addc,        { RT, RA, RB });
  DE(addcdot,       0x7C000015, Form::kXO,  addc.,       { RT, RA, RB });
  DE(addco,         0x7C000414, Form::kXO,  addco,       { RT, RA, RB });
  DE(addcodot,      0x7C000415, Form::kXO,  addco.,      { RT, RA, RB });
  DE(adde,          0x7C000114, Form::kXO,  adde,        { RT, RA, RB });
  DE(addedot,       0x7C000115, Form::kXO,  adde.,       { RT, RA, RB });
  DE(addeo,         0x7C000514, Form::kXO,  addeo,       { RT, RA, RB });
  DE(addeodot,      0x7C000515, Form::kXO,  addeo.,      { RT, RA, RB });
  DE(addg6s,        0x7C000094, Form::kXO,  addg6s,      { RT, RA, RB });
  DE(addi,          0x38000000, Form::kD,   addi,        { RT, RA0, SI });
  DE(addic,         0x30000000, Form::kD,   addic,       { RT, RA, SI });
  DE(addicdot,      0x34000000, Form::kD,   addic.,      { RT, RA, SI });
  DE(addis,         0x3C000000, Form::kD,   addis,      { RT, RA0, SISIGNOPT });
  DE(addme,         0x7C0001D4, Form::kXO,  addme,       { RT, RA });
  DE(addmedot,      0x7C0001D5, Form::kXO,  addme.,      { RT, RA });
  DE(addmeo,        0x7C0005D4, Form::kXO,  addmeo,      { RT, RA });
  DE(addmeodot,     0x7C0005D5, Form::kXO,  addmeo.,     { RT, RA });
  DE(addo,          0x7C000614, Form::kXO,  addo,        { RT, RA, RB });
  DE(addodot,       0x7C000615, Form::kXO,  addo.,       { RT, RA, RB });
  DE(addze,         0x7C000194, Form::kXO,  addze,       { RT, RA });
  DE(addzedot,      0x7C000195, Form::kXO,  addze.,      { RT, RA });
  DE(addzeo,        0x7C000594, Form::kXO,  addzeo,      { RT, RA });
  DE(addzeodot,     0x7C000595, Form::kXO,  addzeo.,     { RT, RA });
  DE(and,           0x7C000038, Form::kX,   and,         { RA, RS, RB });
  DE(anddot,        0x7C000039, Form::kX,   and.,        { RA, RS, RB });
  DE(andc,          0x7C000078, Form::kX,   andc,        { RA, RS, RB });
  DE(andcdot,       0x7C000079, Form::kX,   andc.,       { RA, RS, RB });
  DE(andidot,       0x70000000, Form::kD,   andi.,       { RA, RS, UI });
  DE(andisdot,      0x74000000, Form::kD,   andis.,      { RA, RS, UI });
  DE(b,             0x48000000, Form::kI,   b,           { LI });
  DE(ba,            0x48000002, Form::kI,   ba,          { LIA });
  DE(bc,            0x40000000, Form::kB,   bc,          { BO, BI, BD });
  DE(bca,           0x40000002, Form::kB,   bca,         { BO, BI, BDA });
  DE(bcctr,         0x4C000420, Form::kXL,  bcctr,       { BO, BI, BH });
  DE(bcctrl,        0x4C000421, Form::kXL,  bcctrl,      { BO, BI, BH });
  DE(bcdadddot,     0x10000401, Form::kVX,  bcdadd.,     { VD, VA, VB, PS });
  DE(bcdsubdot,     0x10000441, Form::kVX,  bcdsub.,     { VD, VA, VB, PS });
  DE(bcl,           0x40000001, Form::kB,   bcl,         { BO, BI, BD });
  DE(bcla,          0x40000003, Form::kB,   bcla,        { BO, BI, BDA });
  DE(bclr,          0x4C000020, Form::kXL,  bclr,        { BO, BI, BH });
  DE(bclrl,         0x4C000021, Form::kXL,  bclrl,       { BO, BI, BH });
  DE(bctar,         0x4C000460, Form::kX,   bctar,       { BO, BI, BH });
  DE(bctarl,        0x4C000461, Form::kX,   bctarl,      { BO, BI, BH });
  DE(bl,            0x48000001, Form::kI,   bl,          { LI });
  DE(bla,           0x48000003, Form::kI,   bla,         { LIA });
  DE(bpermd,        0x7C0001F8, Form::kX,   bpermd,      { RS, RA, RB });
  DE(brinc,         0x1000020F, Form::kEVX, brinc,       { RS, RA, RB });
  DE(cbcdtd,        0x7C000274, Form::kX,   cbcdtd,      { RA, RS });
  DE(cdtbcd,        0x7C000234, Form::kX,   cdtbcd,      { RA, RS });
  DE(clrbhrb,       0x7C00035C, Form::kX,   clrbhrb,     { UN });
  DE(cmp,           0x7C000000, Form::kX,   cmp,         { BF, L, RA, RB });
  DE(cmpb,          0x7C0003F8, Form::kX,   cmpb,        { RA, RS, RB });
  DE(cmpi,          0x2C000000, Form::kD,   cmpi,        { BF, L, RA, SI });
  DE(cmpl,          0x7C000040, Form::kX,   cmpl,        { BF, L, RA, RB });
  DE(cmpli,         0x28000000, Form::kD,   cmpli,       { BF, L, RA, UI });
  DE(cntlzd,        0x7C000074, Form::kX,   cntlzd,      { RA, RS });
  DE(cntlzddot,     0x7C000075, Form::kX,   cntlzd.,     { RA, RS });
  DE(cntlzw,        0x7C000034, Form::kX,   cntlzw,      { RA, RS });
  DE(cntlzwdot,     0x7C000035, Form::kX,   cntlzw.,     { RA, RS });
  DE(crand,         0x4C000202, Form::kXL,  crand,       { BT, BA, BB });
  DE(crandc,        0x4C000102, Form::kXL,  crandc,      { BT, BA, BB });
  DE(creqv,         0x4C000242, Form::kXL,  creqv,       { BT, BA, BB });
  DE(crnand,        0x4C0001C2, Form::kXL,  crnand,      { BT, BA, BB });
  DE(crnor,         0x4C000042, Form::kXL,  crnor,       { BT, BA, BB });
  DE(cror,          0x4C000382, Form::kXL,  cror,        { BT, BA, BB });
  DE(crorc,         0x4C000342, Form::kXL,  crorc,       { BT, BA, BB });
  DE(crxor,         0x4C000182, Form::kXL,  crxor,       { BT, BA, BB });
  DE(dadd,          0xEC000004, Form::kX,   dadd,        { FRT, FRA, FRB });
  DE(dadddot,       0xEC000005, Form::kX,   dadd.,       { FRT, FRA, FRB });
  DE(daddq,         0xFC000004, Form::kX,   daddq,       { FRT, FRA, FRB });
  DE(daddqdot,      0xFC000005, Form::kX,   daddq.,      { FRT, FRA, FRB });
  DE(dcba,          0x7C0005EC, Form::kX,   dcba,        { RA, RB });
  DE(dcbf,          0x7C0000AC, Form::kX,   dcbf,        { RA, RB, L });
  DE(dcbfep,        0x7C0000FE, Form::kX,   dcbfep,      { RA, RB, L });
  DE(dcbi,          0x7C0003AC, Form::kX,   dcbi,        { RA, RB });
  DE(dcblc,         0x7C00030C, Form::kX,   dcblc,       { CT, RA, RB });
  DE(dcblqdot,      0x7C00034D, Form::kX,   dcblq.,      { CT, RA, RB });
  DE(dcbst,         0x7C00006C, Form::kX,   dcbst,       { RA, RB });
  DE(dcbstep,       0x7C00007E, Form::kX,   dcbstep,     { RA, RB });
  DE(dcbt,          0x7C00022C, Form::kX,   dcbt,        { CT, RA, RB });
  DE(dcbtep,        0x7C00027E, Form::kX,   dcbtep,      { TH, RA, RB });
  DE(dcbtls,        0x7C00014C, Form::kX,   dcbtls,      { CT, RA, RB });
  DE(dcbtst,        0x7C0001EC, Form::kX,   dcbtst,      { CT, RA, RB });
  DE(dcbtstep,      0x7C0001FE, Form::kX,   dcbtstep,    { TH, RA, RB });
  DE(dcbtstls,      0x7C00010C, Form::kX,   dcbtstls,    { CT, RA, RB });
  DE(dcbz,          0x7C0007EC, Form::kX,   dcbz,        { RA, RB });
  DE(dcbzep,        0x7C0007FE, Form::kX,   dcbzep,      { RA, RB });
  DE(dcffix,        0xEC000644, Form::kX,   dcffix,      { FRT, FRB });
  DE(dcffixdot,     0xEC000645, Form::kX,   dcffix.,     { FRT, FRB });
  DE(dcffixq,       0xFC000644, Form::kX,   dcffixq,     { FRT, FRB });
  DE(dcffixqdot,    0xFC000645, Form::kX,   dcffixq.,    { FRT, FRB });
  DE(dci,           0x7C00038C, Form::kX,   dci,         { CT });
  DE(dcmpo,         0xEC000104, Form::kX,   dcmpo,       { BF,  FRA, FRB });
  DE(dcmpoq,        0xFC000104, Form::kX,   dcmpoq,      { BF,  FRA, FRB });
  DE(dcmpu,         0xEC000504, Form::kX,   dcmpu,       { BF,  FRA, FRB });
  DE(dcmpuq,        0xFC000504, Form::kX,   dcmpuq,      { BF,  FRA, FRB });
  DE(dcread,        0x7C00028C, Form::kX,   dcread,      { RT, RA, RB });
  DE(dcread2,       0x7C0003CC, Form::kX,   dcread,      { RT, RA, RB });
  DE(dctdp,         0xEC000204, Form::kX,   dctdp,       { FRT, FRB });
  DE(dctdpdot,      0xEC000205, Form::kX,   dctdp.,      { FRT, FRB });
  DE(dctfix,        0xEC000244, Form::kX,   dctfix,      { FRT, FRB });
  DE(dctfixdot,     0xEC000245, Form::kX,   dctfix.,     { FRT, FRB });
  DE(dctfixq,       0xFC000244, Form::kX,   dctfixq,     { FRT, FRB });
  DE(dctfixqdot,    0xFC000245, Form::kX,   dctfixq.,    { FRT, FRB });
  DE(dctqpq,        0xFC000204, Form::kX,   dctqpq,      { FRT, FRB });
  DE(dctqpqdot,     0xFC000205, Form::kX,   dctqpq.,     { FRT, FRB });
  DE(ddedpd,        0xEC000284, Form::kX,   ddedpd,      { SP, FRT, FRB });
  DE(ddedpddot,     0xEC000285, Form::kX,   ddedpd.,     { SP, FRT, FRB });
  DE(ddedpdq,       0xFC000284, Form::kX,   ddedpdq,     { SP, FRT, FRB });
  DE(ddedpdqdot,    0xFC000285, Form::kX,   ddedpdq.,    { SP, FRT, FRB });
  DE(ddiv,          0xEC000444, Form::kX,   ddiv,        { FRT, FRA, FRB });
  DE(ddivdot,       0xEC000445, Form::kX,   ddiv.,       { FRT, FRA, FRB });
  DE(ddivq,         0xFC000444, Form::kX,   ddivq,       { FRT, FRA, FRB });
  DE(ddivqdot,      0xFC000445, Form::kX,   ddivq.,      { FRT, FRA, FRB });
  DE(denbcd,        0xEC000684, Form::kX,   denbcd,      { S, FRT, FRB });
  DE(denbcddot,     0xEC000685, Form::kX,   denbcd.,     { S, FRT, FRB });
  DE(denbcdq,       0xFC000684, Form::kX,   denbcdq,     { S, FRT, FRB });
  DE(denbcdqdot,    0xFC000685, Form::kX,   denbcdq.,    { S, FRT, FRB });
  DE(diex,          0xEC0006C4, Form::kX,   diex,        { FRT, FRA, FRB });
  DE(diexdot,       0xEC0006C5, Form::kX,   diex.,       { FRT, FRA, FRB });
  DE(diexq,         0xFC0006C4, Form::kX,   diexq,       { FRT, FRA, FRB });
  DE(diexqdot,      0xFC0006C5, Form::kX,   diexq.,      { FRT, FRA, FRB });
  DE(divd,          0x7C0003D2, Form::kXO,  divd,        { RT, RA, RB });
  DE(divddot,       0x7C0003D3, Form::kXO,  divd.,       { RT, RA, RB });
  DE(divde,         0x7C000352, Form::kXO,  divde,       { RT, RA, RB });
  DE(divdedot,      0x7C000353, Form::kXO,  divde.,      { RT, RA, RB });
  DE(divdeo,        0x7C000752, Form::kXO,  divdeo,      { RT, RA, RB });
  DE(divdeodot,     0x7C000753, Form::kXO,  divdeo.,     { RT, RA, RB });
  DE(divdeu,        0x7C000312, Form::kXO,  divdeu,      { RT, RA, RB });
  DE(divdeudot,     0x7C000313, Form::kXO,  divdeu.,     { RT, RA, RB });
  DE(divdeuo,       0x7C000712, Form::kXO,  divdeuo,     { RT, RA, RB });
  DE(divdeuodot,    0x7C000713, Form::kXO,  divdeuo.,    { RT, RA, RB });
  DE(divdo,         0x7C0007D2, Form::kXO,  divdo,       { RT, RA, RB });
  DE(divdodot,      0x7C0007D3, Form::kXO,  divdo.,      { RT, RA, RB });
  DE(divdu,         0x7C000392, Form::kXO,  divdu,       { RT, RA, RB });
  DE(divdudot,      0x7C000393, Form::kXO,  divdu.,      { RT, RA, RB });
  DE(divduo,        0x7C000792, Form::kXO,  divduo,      { RT, RA, RB });
  DE(divduodot,     0x7C000793, Form::kXO,  divduo.,     { RT, RA, RB });
  DE(divw,          0x7C0003D6, Form::kXO,  divw,        { RT, RA, RB });
  DE(divwdot,       0x7C0003D7, Form::kXO,  divw.,       { RT, RA, RB });
  DE(divwe,         0x7C000356, Form::kXO,  divwe,       { RT, RA, RB });
  DE(divwedot,      0x7C000357, Form::kXO,  divwe.,      { RT, RA, RB });
  DE(divweo,        0x7C000756, Form::kXO,  divweo,      { RT, RA, RB });
  DE(divweodot,     0x7C000757, Form::kXO,  divweo.,     { RT, RA, RB });
  DE(divweu,        0x7C000316, Form::kXO,  divweu,      { RT, RA, RB });
  DE(divweudot,     0x7C000317, Form::kXO,  divweu.,     { RT, RA, RB });
  DE(divweuo,       0x7C000716, Form::kXO,  divweuo,     { RT, RA, RB });
  DE(divweuodot,    0x7C000717, Form::kXO,  divweuo.,    { RT, RA, RB });
  DE(divwo,         0x7C0007D6, Form::kXO,  divwo,       { RT, RA, RB });
  DE(divwodot,      0x7C0007D7, Form::kXO,  divwo.,      { RT, RA, RB });
  DE(divwu,         0x7C000396, Form::kXO,  divwu,       { RT, RA, RB });
  DE(divwudot,      0x7C000397, Form::kXO,  divwu.,      { RT, RA, RB });
  DE(divwuo,        0x7C000796, Form::kXO,  divwuo,      { RT, RA, RB });
  DE(divwuodot,     0x7C000797, Form::kXO,  divwuo.,     { RT, RA, RB });
  DE(dlmzb,         0x7C00009C, Form::kX,   dlmzb,       { RA, RS, RB });
  DE(dlmzbdot,      0x7C00009D, Form::kX,   dlmzb.,      { RA, RS, RB });
  DE(dmul,          0xEC000044, Form::kX,   dmul,        { FRT, FRA, FRB });
  DE(dmuldot,       0xEC000045, Form::kX,   dmul.,       { FRT, FRA, FRB });
  DE(dmulq,         0xFC000044, Form::kX,   dmulq,       { FRT, FRA, FRB });
  DE(dmulqdot,      0xFC000045, Form::kX,   dmulq.,      { FRT, FRA, FRB });
  DE(dnh,           0x4C00018C, Form::kXFX, dnh,         { DUI, DUIS});
  DE(doze,          0x4C000324, Form::kXL,  doze,        { UN });
  DE(dqua,          0xEC000006, Form::kZ23, dqua,       { FRT, FRA, FRB, RMC });
  DE(dquadot,       0xEC000007, Form::kZ23, dqua.,      { FRT, FRA, FRB, RMC });
  DE(dquai,         0xEC000086, Form::kZ23, dquai,      { TE,  FRT, FRB, RMC });
  DE(dquaidot,      0xEC000087, Form::kZ23, dquai.,     { TE,  FRT, FRB, RMC });
  DE(dquaiq,        0xFC000086, Form::kZ23, dquaiq,     { TE,  FRT, FRB, RMC });
  DE(dquaiqdot,     0xFC000087, Form::kZ23, dquaiq.,    { FRT, FRA, FRB, RMC });
  DE(dquaq,         0xFC000006, Form::kZ23, dquaq,      { FRT, FRA, FRB, RMC });
  DE(dquaqdot,      0xFC000007, Form::kZ23, dquaq.,     { FRT, FRA, FRB, RMC });
  DE(drdpq,         0xFC000604, Form::kX,   drdpq,       { FRT, FRB });
  DE(drdpqdot,      0xFC000605, Form::kX,   drdpq.,      { FRT, FRB });
  DE(drintn,        0xEC0001C6, Form::kZ23, drintn,      { R, FRT, FRB, RMC });
  DE(drintndot,     0xEC0001C7, Form::kZ23, drintn.,     { R, FRT, FRB, RMC });
  DE(drintnq,       0xFC0001C6, Form::kZ23, drintnq,     { R, FRT, FRB, RMC });
  DE(drintnqdot,    0xFC0001C7, Form::kZ23, drintnq.,    { R, FRT, FRB, RMC });
  DE(drintx,        0xEC0000C6, Form::kZ23, drintx,      { R, FRT, FRB, RMC });
  DE(drintxdot,     0xEC0000C7, Form::kZ23, drintx.,     { R, FRT, FRB, RMC });
  DE(drintxq,       0xFC0000C6, Form::kZ23, drintxq,     { R, FRT, FRB, RMC });
  DE(drintxqdot,    0xFC0000C7, Form::kZ23, drintxq.,    { R, FRT, FRB, RMC });
  DE(drrnd,         0xEC000046, Form::kZ23, drrnd,      { FRT, FRA, FRB, RMC });
  DE(drrnddot,      0xEC000047, Form::kZ23, drrnd.,     { FRT, FRA, FRB, RMC });
  DE(drrndq,        0xFC000046, Form::kZ23, drrndq,     { FRT, FRA, FRB, RMC });
  DE(drrndqdot,     0xFC000047, Form::kZ23, drrndq.,    { FRT, FRA, FRB, RMC });
  DE(drsp,          0xEC000604, Form::kX,   drsp,        { FRT, FRB });
  DE(drspdot,       0xEC000605, Form::kX,   drsp.,       { FRT, FRB });
  DE(dscli,         0xEC000084, Form::kZ22, dscli,       { FRT, FRA, SH16 });
  DE(dsclidot,      0xEC000085, Form::kZ22, dscli.,      { FRT, FRA, SH16 });
  DE(dscliq,        0xFC000084, Form::kZ22, dscliq,      { FRT, FRA, SH16 });
  DE(dscliqdot,     0xFC000085, Form::kZ22, dscliq.,     { FRT, FRA, SH16 });
  DE(dscri,         0xEC0000C4, Form::kZ22, dscri,       { FRT, FRA, SH16 });
  DE(dscridot,      0xEC0000C5, Form::kZ22, dscri.,      { FRT, FRA, SH16 });
  DE(dscriq,        0xFC0000C4, Form::kZ22, dscriq,      { FRT, FRA, SH16 });
  DE(dscriqdot,     0xFC0000C5, Form::kZ22, dscriq.,     { FRT, FRA, SH16 });
  DE(dsn,           0x7C0003C6, Form::kX,   dsn,         { FRT, FRA, SH16 });
  DE(dsub,          0xEC000404, Form::kX,   dsub,        { RA, RB });
  DE(dsubdot,       0xEC000405, Form::kX,   dsub.,       { FRT, FRA, FRB });
  DE(dsubq,         0xFC000404, Form::kX,   dsubq,       { FRT, FRA, FRB });
  DE(dsubqdot,      0xFC000405, Form::kX,   dsubq.,      { FRT, FRA, FRB });
  DE(dtstdc,        0xEC000184, Form::kZ22, dtstdc,      { BF,  FRA, DCM });
  DE(dtstdcq,       0xFC000184, Form::kZ22, dtstdcq,     { BF,  FRA, DCM });
  DE(dtstdg,        0xEC0001C4, Form::kZ22, dtstdg,      { BF,  FRA, DGM });
  DE(dtstdgq,       0xFC0001C4, Form::kZ22, dtstdgq,     { BF,  FRA, DGM });
  DE(dtstex,        0xEC000144, Form::kX,   dtstex,      { BF,  FRA, FRB });
  DE(dtstexq,       0xFC000144, Form::kX,   dtstexq,     { BF,  FRA, FRB });
  DE(dtstsf,        0xEC000544, Form::kX,   dtstsf,      { BF,  FRA, FRB });
  DE(dtstsfq,       0xFC000544, Form::kX,   dtstsfq,     { BF,  FRA, FRB });
  DE(dxex,          0xEC0002C4, Form::kX,   dxex,        { FRT, FRB });
  DE(dxexdot,       0xEC0002C5, Form::kX,   dxex.,       { FRT, FRB });
  DE(dxexq,         0xFC0002C4, Form::kX,   dxexq,       { FRT, FRB });
  DE(dxexqdot,      0xFC0002C5, Form::kX,   dxexq.,      { FRT, FRB });
  DE(eciwx,         0x7C00026C, Form::kX,   eciwx,       { RT, RA, RB });
  DE(ecowx,         0x7C00036C, Form::kX,   ecowx,       { RT, RA, RB });
  DE(efdabs,        0x100002E4, Form::kEVX, efdabs,      { RS, RA });
  DE(efdadd,        0x100002E0, Form::kEVX, efdadd,      { RS, RA, RB });
  DE(efdcfs,        0x100002EF, Form::kEVX, efdcfs,      { RS, RB });
  DE(efdcfsf,       0x100002F3, Form::kEVX, efdcfsf,     { RS, RB });
  DE(efdcfsi,       0x100002F1, Form::kEVX, efdcfsi,     { RS, RB });
  DE(efdcfsid,      0x100002E3, Form::kEVX, efdcfsid,    { RS, RB });
  DE(efdcfuf,       0x100002F2, Form::kEVX, efdcfuf,     { RS, RB });
  DE(efdcfui,       0x100002F0, Form::kEVX, efdcfui,     { RS, RB });
  DE(efdcfuid,      0x100002E2, Form::kEVX, efdcfuid,    { RS, RB });
  DE(efdcmpeq,      0x100002EE, Form::kEVX, efdcmpeq,    { CRFD, RA, RB });
  DE(efdcmpgt,      0x100002EC, Form::kEVX, efdcmpgt,    { CRFD, RA, RB });
  DE(efdcmplt,      0x100002ED, Form::kEVX, efdcmplt,    { CRFD, RA, RB });
  DE(efdctsf,       0x100002F7, Form::kEVX, efdctsf,     { RS, RB });
  DE(efdctsi,       0x100002F5, Form::kEVX, efdctsi,     { RS, RB });
  DE(efdctsidz,     0x100002EB, Form::kEVX, efdctsidz,   { RS, RB });
  DE(efdctsiz,      0x100002FA, Form::kEVX, efdctsiz,    { RS, RB });
  DE(efdctuf,       0x100002F6, Form::kEVX, efdctuf,     { RS, RB });
  DE(efdctui,       0x100002F4, Form::kEVX, efdctui,     { RS, RB });
  DE(efdctuidz,     0x100002EA, Form::kEVX, efdctuidz,   { RS, RB });
  DE(efdctuiz,      0x100002F8, Form::kEVX, efdctuiz,    { RS, RB });
  DE(efddiv,        0x100002E9, Form::kEVX, efddiv,      { RS, RA, RB });
  DE(efdmul,        0x100002E8, Form::kEVX, efdmul,      { RS, RA, RB });
  DE(efdnabs,       0x100002E5, Form::kEVX, efdnabs,     { RS, RA });
  DE(efdneg,        0x100002E6, Form::kEVX, efdneg,      { RS, RA });
  DE(efdsub,        0x100002E1, Form::kEVX, efdsub,      { RS, RA, RB });
  DE(efdtsteq,      0x100002FE, Form::kEVX, efdtsteq,    { CRFD, RA, RB });
  DE(efdtstgt,      0x100002FC, Form::kEVX, efdtstgt,    { CRFD, RA, RB });
  DE(efdtstlt,      0x100002FD, Form::kEVX, efdtstlt,    { CRFD, RA, RB });
  DE(efsabs,        0x100002C4, Form::kEVX, efsabs,      { RS, RA });
  DE(efsadd,        0x100002C0, Form::kEVX, efsadd,      { RS, RA, RB });
  DE(efscfd,        0x100002CF, Form::kEVX, efscfd,      { RS, RB });
  DE(efscfsf,       0x100002D3, Form::kEVX, efscfsf,     { RS, RB });
  DE(efscfsi,       0x100002D1, Form::kEVX, efscfsi,     { RS, RB });
  DE(efscfuf,       0x100002D2, Form::kEVX, efscfuf,     { RS, RB });
  DE(efscfui,       0x100002D0, Form::kEVX, efscfui,     { RS, RB });
  DE(efscmpeq,      0x100002CE, Form::kEVX, efscmpeq,    { CRFD, RA, RB });
  DE(efscmpgt,      0x100002CC, Form::kEVX, efscmpgt,    { CRFD, RA, RB });
  DE(efscmplt,      0x100002CD, Form::kEVX, efscmplt,    { CRFD, RA, RB });
  DE(efsctsf,       0x100002D7, Form::kEVX, efsctsf,     { RS, RB });
  DE(efsctsi,       0x100002D5, Form::kEVX, efsctsi,     { RS, RB });
  DE(efsctsiz,      0x100002DA, Form::kEVX, efsctsiz,    { RS, RB });
  DE(efsctuf,       0x100002D6, Form::kEVX, efsctuf,     { RS, RB });
  DE(efsctui,       0x100002D4, Form::kEVX, efsctui,     { RS, RB });
  DE(efsctuiz,      0x100002D8, Form::kEVX, efsctuiz,    { RS, RB });
  DE(efsdiv,        0x100002C9, Form::kEVX, efsdiv,      { RS, RA, RB });
  DE(efsmul,        0x100002C8, Form::kEVX, efsmul,      { RS, RA, RB });
  DE(efsnabs,       0x100002C5, Form::kEVX, efsnabs,     { RS, RA });
  DE(efsneg,        0x100002C6, Form::kEVX, efsneg,      { RS, RA });
  DE(efssub,        0x100002C1, Form::kEVX, efssub,      { RS, RA, RB });
  DE(efststeq,      0x100002DE, Form::kEVX, efststeq,    { CRFD, RA, RB });
  DE(efststgt,      0x100002DC, Form::kEVX, efststgt,    { CRFD, RA, RB });
  DE(efststlt,      0x100002DD, Form::kEVX, efststlt,    { CRFD, RA, RB });
  DE(ehpriv,        0x7C00021C, Form::kXL,  ehpriv,      { OC });
  DE(eieio,         0x7C0006AC, Form::kX,   eieio,       { UN });
  DE(eqv,           0x7C000238, Form::kX,   eqv,         { RA, RS, RB });
  DE(eqvdot,        0x7C000239, Form::kX,   eqv.,        { RA, RS, RB });
  DE(evabs,         0x10000208, Form::kEVX, evabs,       { RS, RA });
  DE(evaddiw,       0x10000202, Form::kEVX, evaddiw,     { RS, RB, UIMM });
  DE(evaddsmiaaw,   0x100004C9, Form::kEVX, evaddsmiaaw, { RS, RA });
  DE(evaddssiaaw,   0x100004C1, Form::kEVX, evaddssiaaw, { RS, RA });
  DE(evaddumiaaw,   0x100004C8, Form::kEVX, evaddumiaaw, { RS, RA });
  DE(evaddusiaaw,   0x100004C0, Form::kEVX, evaddusiaaw, { RS, RA });
  DE(evaddw,        0x10000200, Form::kEVX, evaddw,      { RS, RA, RB });
  DE(evand,         0x10000211, Form::kEVX, evand,       { RS, RA, RB });
  DE(evandc,        0x10000212, Form::kEVX, evandc,      { RS, RA, RB });
  DE(evcmpeq,       0x10000234, Form::kEVX, evcmpeq,     { CRFD, RA, RB });
  DE(evcmpgts,      0x10000231, Form::kEVX, evcmpgts,    { CRFD, RA, RB });
  DE(evcmpgtu,      0x10000230, Form::kEVX, evcmpgtu,    { CRFD, RA, RB });
  DE(evcmplts,      0x10000233, Form::kEVX, evcmplts,    { CRFD, RA, RB });
  DE(evcmpltu,      0x10000232, Form::kEVX, evcmpltu,    { CRFD, RA, RB });
  DE(evcntlsw,      0x1000020E, Form::kEVX, evcntlsw,    { RS, RA });
  DE(evcntlzw,      0x1000020D, Form::kEVX, evcntlzw,    { RS, RA });
  DE(evdivws,       0x100004C6, Form::kEVX, evdivws,     { RS, RA, RB });
  DE(evdivwu,       0x100004C7, Form::kEVX, evdivwu,     { RS, RA, RB });
  DE(eveqv,         0x10000219, Form::kEVX, eveqv,       { RS, RA, RB });
  DE(evextsb,       0x1000020A, Form::kEVX, evextsb,     { RS, RA });
  DE(evextsh,       0x1000020B, Form::kEVX, evextsh,     { RS, RA });
  DE(evfsabs,       0x10000284, Form::kEVX, evfsabs,     { RS, RA });
  DE(evfsadd,       0x10000280, Form::kEVX, evfsadd,     { RS, RA, RB });
  DE(evfscfsf,      0x10000293, Form::kEVX, evfscfsf,    { RS, RB });
  DE(evfscfsi,      0x10000291, Form::kEVX, evfscfsi,    { RS, RB });
  DE(evfscfuf,      0x10000292, Form::kEVX, evfscfuf,    { RS, RB });
  DE(evfscfui,      0x10000290, Form::kEVX, evfscfui,    { RS, RB });
  DE(evfscmpeq,     0x1000028E, Form::kEVX, evfscmpeq,   { CRFD, RA, RB });
  DE(evfscmpgt,     0x1000028C, Form::kEVX, evfscmpgt,   { CRFD, RA, RB });
  DE(evfscmplt,     0x1000028D, Form::kEVX, evfscmplt,   { CRFD, RA, RB });
  DE(evfsctsf,      0x10000297, Form::kEVX, evfsctsf,    { RS, RB });
  DE(evfsctsi,      0x10000295, Form::kEVX, evfsctsi,    { RS, RB });
  DE(evfsctsiz,     0x1000029A, Form::kEVX, evfsctsiz,   { RS, RB });
  DE(evfsctuf,      0x10000296, Form::kEVX, evfsctuf,    { RS, RB });
  DE(evfsctui,      0x10000294, Form::kEVX, evfsctui,    { RS, RB });
  DE(evfsctuiz,     0x10000298, Form::kEVX, evfsctuiz,   { RS, RB });
  DE(evfsdiv,       0x10000289, Form::kEVX, evfsdiv,     { RS, RA, RB });
  DE(evfsmul,       0x10000288, Form::kEVX, evfsmul,     { RS, RA, RB });
  DE(evfsnabs,      0x10000285, Form::kEVX, evfsnabs,    { RS, RA });
  DE(evfsneg,       0x10000286, Form::kEVX, evfsneg,     { RS, RA });
  DE(evfssub,       0x10000281, Form::kEVX, evfssub,     { RS, RA, RB });
  DE(evfststeq,     0x1000029E, Form::kEVX, evfststeq,   { CRFD, RA, RB });
  DE(evfststgt,     0x1000029C, Form::kEVX, evfststgt,   { CRFD, RA, RB });
  DE(evfststlt,     0x1000029D, Form::kEVX, evfststlt,   { CRFD, RA, RB });
  DE(evldd,         0x10000301, Form::kEVX, evldd,       { RS, EVUIMM_8, RA });
  DE(evlddepx,      0x7C00063E, Form::kEVX, evlddepx,    { RT, RA, RB });
  DE(evlddx,        0x10000300, Form::kEVX, evlddx,      { RS, RA, RB });
  DE(evldh,         0x10000305, Form::kEVX, evldh,       { RS, EVUIMM_8, RA });
  DE(evldhx,        0x10000304, Form::kEVX, evldhx,      { RS, RA, RB });
  DE(evldw,         0x10000303, Form::kEVX, evldw,       { RS, EVUIMM_8, RA });
  DE(evldwx,        0x10000302, Form::kEVX, evldwx,      { RS, RA, RB });
  DE(evlhhesplat,   0x10000309, Form::kEVX, evlhhesplat, { RS, EVUIMM_2, RA });
  DE(evlhhesplatx,  0x10000308, Form::kEVX, evlhhesplat, { RS, RA, RB });
  DE(evlhhossplat,  0x1000030F, Form::kEVX, evlhhosspla, { RS, EVUIMM_2, RA });
  DE(evlhhosspla1,  0x1000030E, Form::kEVX, evlhhosspla, { RS, RA, RB });
  DE(evlhhouspla2,  0x1000030D, Form::kEVX, evlhhouspla, { RS, EVUIMM_2, RA });
  DE(evlhhouspla3,  0x1000030C, Form::kEVX, evlhhouspla, { RS, RA, RB });
  DE(evlwhe,        0x10000311, Form::kEVX, evlwhe,      { RS, EVUIMM_4, RA });
  DE(evlwhex,       0x10000310, Form::kEVX, evlwhex,     { RS, RA, RB });
  DE(evlwhos,       0x10000317, Form::kEVX, evlwhos,     { RS, EVUIMM_4, RA });
  DE(evlwhosx,      0x10000316, Form::kEVX, evlwhosx,    { RS, RA, RB });
  DE(evlwhou,       0x10000315, Form::kEVX, evlwhou,     { RS, EVUIMM_4, RA });
  DE(evlwhoux,      0x10000314, Form::kEVX, evlwhoux,    { RS, RA, RB });
  DE(evlwhsplat,    0x1000031D, Form::kEVX, evlwhsplat,  { RS, EVUIMM_4, RA });
  DE(evlwhsplatx,   0x1000031C, Form::kEVX, evlwhsplatx, { RS, RA, RB });
  DE(evlwwsplat,    0x10000319, Form::kEVX, evlwwsplat,  { RS, EVUIMM_4, RA });
  DE(evlwwsplatx,   0x10000318, Form::kEVX, evlwwsplatx, { RS, RA, RB });
  DE(evmergehi,     0x1000022C, Form::kEVX, evmergehi,   { RS, RA, RB });
  DE(evmergehilo,   0x1000022E, Form::kEVX, evmergehilo, { RS, RA, RB });
  DE(evmergelo,     0x1000022D, Form::kEVX, evmergelo,   { RS, RA, RB });
  DE(evmergelohi,   0x1000022F, Form::kEVX, evmergelohi, { RS, RA, RB });
  DE(evmhegsmfaa,   0x1000052B, Form::kEVX, evmhegsmfaa, { RS, RA, RB });
  DE(evmhegsmfan,   0x100005AB, Form::kEVX, evmhegsmfan, { RS, RA, RB });
  DE(evmhegsmiaa,   0x10000529, Form::kEVX, evmhegsmiaa, { RS, RA, RB });
  DE(evmhegsmian,   0x100005A9, Form::kEVX, evmhegsmian, { RS, RA, RB });
  DE(evmhegumiaa,   0x10000528, Form::kEVX, evmhegumiaa, { RS, RA, RB });
  DE(evmhegumian,   0x100005A8, Form::kEVX, evmhegumian, { RS, RA, RB });
  DE(evmhesmf,      0x1000040B, Form::kEVX, evmhesmf,    { RS, RA, RB });
  DE(evmhesmfa,     0x1000042B, Form::kEVX, evmhesmfa,   { RS, RA, RB });
  DE(evmhesmfaaw,   0x1000050B, Form::kEVX, evmhesmfaaw, { RS, RA, RB });
  DE(evmhesmfanw,   0x1000058B, Form::kEVX, evmhesmfanw, { RS, RA, RB });
  DE(evmhesmi,      0x10000409, Form::kEVX, evmhesmi,    { RS, RA, RB });
  DE(evmhesmia,     0x10000429, Form::kEVX, evmhesmia,   { RS, RA, RB });
  DE(evmhesmiaaw,   0x10000509, Form::kEVX, evmhesmiaaw, { RS, RA, RB });
  DE(evmhesmianw,   0x10000589, Form::kEVX, evmhesmianw, { RS, RA, RB });
  DE(evmhessf,      0x10000403, Form::kEVX, evmhessf,    { RS, RA, RB });
  DE(evmhessfa,     0x10000423, Form::kEVX, evmhessfa,   { RS, RA, RB });
  DE(evmhessfaaw,   0x10000503, Form::kEVX, evmhessfaaw, { RS, RA, RB });
  DE(evmhessfanw,   0x10000583, Form::kEVX, evmhessfanw, { RS, RA, RB });
  DE(evmhessiaaw,   0x10000501, Form::kEVX, evmhessiaaw, { RS, RA, RB });
  DE(evmhessianw,   0x10000581, Form::kEVX, evmhessianw, { RS, RA, RB });
  DE(evmheumi,      0x10000408, Form::kEVX, evmheumi,    { RS, RA, RB });
  DE(evmheumia,     0x10000428, Form::kEVX, evmheumia,   { RS, RA, RB });
  DE(evmheumiaaw,   0x10000508, Form::kEVX, evmheumiaaw, { RS, RA, RB });
  DE(evmheumianw,   0x10000588, Form::kEVX, evmheumianw, { RS, RA, RB });
  DE(evmheusiaaw,   0x10000500, Form::kEVX, evmheusiaaw, { RS, RA, RB });
  DE(evmheusianw,   0x10000580, Form::kEVX, evmheusianw, { RS, RA, RB });
  DE(evmhogsmfaa,   0x1000052F, Form::kEVX, evmhogsmfaa, { RS, RA, RB });
  DE(evmhogsmfan,   0x100005AF, Form::kEVX, evmhogsmfan, { RS, RA, RB });
  DE(evmhogsmiaa,   0x1000052D, Form::kEVX, evmhogsmiaa, { RS, RA, RB });
  DE(evmhogsmian,   0x100005AD, Form::kEVX, evmhogsmian, { RS, RA, RB });
  DE(evmhogumiaa,   0x1000052C, Form::kEVX, evmhogumiaa, { RS, RA, RB });
  DE(evmhogumian,   0x100005AC, Form::kEVX, evmhogumian, { RS, RA, RB });
  DE(evmhosmf,      0x1000040F, Form::kEVX, evmhosmf,    { RS, RA, RB });
  DE(evmhosmfa,     0x1000042F, Form::kEVX, evmhosmfa,   { RS, RA, RB });
  DE(evmhosmfaaw,   0x1000050F, Form::kEVX, evmhosmfaaw, { RS, RA, RB });
  DE(evmhosmfanw,   0x1000058F, Form::kEVX, evmhosmfanw, { RS, RA, RB });
  DE(evmhosmi,      0x1000040D, Form::kEVX, evmhosmi,    { RS, RA, RB });
  DE(evmhosmia,     0x1000042D, Form::kEVX, evmhosmia,   { RS, RA, RB });
  DE(evmhosmiaaw,   0x1000050D, Form::kEVX, evmhosmiaaw, { RS, RA, RB });
  DE(evmhosmianw,   0x1000058D, Form::kEVX, evmhosmianw, { RS, RA, RB });
  DE(evmhossf,      0x10000407, Form::kEVX, evmhossf,    { RS, RA, RB });
  DE(evmhossfa,     0x10000427, Form::kEVX, evmhossfa,   { RS, RA, RB });
  DE(evmhossfaaw,   0x10000507, Form::kEVX, evmhossfaaw, { RS, RA, RB });
  DE(evmhossfanw,   0x10000587, Form::kEVX, evmhossfanw, { RS, RA, RB });
  DE(evmhossiaaw,   0x10000505, Form::kEVX, evmhossiaaw, { RS, RA, RB });
  DE(evmhossianw,   0x10000585, Form::kEVX, evmhossianw, { RS, RA, RB });
  DE(evmhoumi,      0x1000040C, Form::kEVX, evmhoumi,    { RS, RA, RB });
  DE(evmhoumia,     0x1000042C, Form::kEVX, evmhoumia,   { RS, RA, RB });
  DE(evmhoumiaaw,   0x1000050C, Form::kEVX, evmhoumiaaw, { RS, RA, RB });
  DE(evmhoumianw,   0x1000058C, Form::kEVX, evmhoumianw, { RS, RA, RB });
  DE(evmhousiaaw,   0x10000504, Form::kEVX, evmhousiaaw, { RS, RA, RB });
  DE(evmhousianw,   0x10000584, Form::kEVX, evmhousianw, { RS, RA, RB });
  DE(evmra,         0x100004C4, Form::kEVX, evmra,       { RS, RA });
  DE(evmwhsmf,      0x1000044F, Form::kEVX, evmwhsmf,    { RS, RA, RB });
  DE(evmwhsmfa,     0x1000046F, Form::kEVX, evmwhsmfa,   { RS, RA, RB });
  DE(evmwhsmi,      0x1000044D, Form::kEVX, evmwhsmi,    { RS, RA, RB });
  DE(evmwhsmia,     0x1000046D, Form::kEVX, evmwhsmia,   { RS, RA, RB });
  DE(evmwhssf,      0x10000447, Form::kEVX, evmwhssf,    { RS, RA, RB });
  DE(evmwhssfa,     0x10000467, Form::kEVX, evmwhssfa,   { RS, RA, RB });
  DE(evmwhumi,      0x1000044C, Form::kEVX, evmwhumi,    { RS, RA, RB });
  DE(evmwhumia,     0x1000046C, Form::kEVX, evmwhumia,   { RS, RA, RB });
  DE(evmwlsmiaaw,   0x10000549, Form::kEVX, evmwlsmiaaw, { RS, RA, RB });
  DE(evmwlsmianw,   0x100005C9, Form::kEVX, evmwlsmianw, { RS, RA, RB });
  DE(evmwlssiaaw,   0x10000541, Form::kEVX, evmwlssiaaw, { RS, RA, RB });
  DE(evmwlssianw,   0x100005C1, Form::kEVX, evmwlssianw, { RS, RA, RB });
  DE(evmwlumi,      0x10000448, Form::kEVX, evmwlumi,    { RS, RA, RB });
  DE(evmwlumia,     0x10000468, Form::kEVX, evmwlumia,   { RS, RA, RB });
  DE(evmwlumiaaw,   0x10000548, Form::kEVX, evmwlumiaaw, { RS, RA, RB });
  DE(evmwlumianw,   0x100005C8, Form::kEVX, evmwlumianw, { RS, RA, RB });
  DE(evmwlusiaaw,   0x10000540, Form::kEVX, evmwlusiaaw, { RS, RA, RB });
  DE(evmwlusianw,   0x100005C0, Form::kEVX, evmwlusianw, { RS, RA, RB });
  DE(evmwsmf,       0x1000045B, Form::kEVX, evmwsmf,     { RS, RA, RB });
  DE(evmwsmfa,      0x1000047B, Form::kEVX, evmwsmfa,    { RS, RA, RB });
  DE(evmwsmfaa,     0x1000055B, Form::kEVX, evmwsmfaa,   { RS, RA, RB });
  DE(evmwsmfan,     0x100005DB, Form::kEVX, evmwsmfan,   { RS, RA, RB });
  DE(evmwsmi,       0x10000459, Form::kEVX, evmwsmi,     { RS, RA, RB });
  DE(evmwsmia,      0x10000479, Form::kEVX, evmwsmia,    { RS, RA, RB });
  DE(evmwsmiaa,     0x10000559, Form::kEVX, evmwsmiaa,   { RS, RA, RB });
  DE(evmwsmian,     0x100005D9, Form::kEVX, evmwsmian,   { RS, RA, RB });
  DE(evmwssf,       0x10000453, Form::kEVX, evmwssf,     { RS, RA, RB });
  DE(evmwssfa,      0x10000473, Form::kEVX, evmwssfa,    { RS, RA, RB });
  DE(evmwssfaa,     0x10000553, Form::kEVX, evmwssfaa,   { RS, RA, RB });
  DE(evmwssfan,     0x100005D3, Form::kEVX, evmwssfan,   { RS, RA, RB });
  DE(evmwumi,       0x10000458, Form::kEVX, evmwumi,     { RS, RA, RB });
  DE(evmwumia,      0x10000478, Form::kEVX, evmwumia,    { RS, RA, RB });
  DE(evmwumiaa,     0x10000558, Form::kEVX, evmwumiaa,   { RS, RA, RB });
  DE(evmwumian,     0x100005D8, Form::kEVX, evmwumian,   { RS, RA, RB });
  DE(evnand,        0x1000021E, Form::kEVX, evnand,      { RS, RA, RB });
  DE(evneg,         0x10000209, Form::kEVX, evneg,       { RS, RA });
  DE(evnor,         0x10000218, Form::kEVX, evnor,       { RS, RA, RB });
  DE(evor,          0x10000217, Form::kEVX, evor,        { RS, RA, RB });
  DE(evorc,         0x1000021B, Form::kEVX, evorc,       { RS, RA, RB });
  DE(evrlw,         0x10000228, Form::kEVX, evrlw,       { RS, RA, RB });
  DE(evrlwi,        0x1000022A, Form::kEVX, evrlwi,      { RS, RA, EVUIMM });
  DE(evrndw,        0x1000020C, Form::kEVX, evrndw,      { RS, RA });
  DE(evsel,         0x10000278, Form::kEVS, evsel,       { RS, RA, RB, CRFD });
  DE(evslw,         0x10000224, Form::kEVX, evslw,       { RS, RA, RB });
  DE(evslwi,        0x10000226, Form::kEVX, evslwi,      { RS, RA, EVUIMM });
  DE(evsplatfi,     0x1000022B, Form::kEVX, evsplatfi,   { RS, SIMM });
  DE(evsplati,      0x10000229, Form::kEVX, evsplati,    { RS, SIMM });
  DE(evsrwis,       0x10000223, Form::kEVX, evsrwis,     { RS, RA, EVUIMM });
  DE(evsrwiu,       0x10000222, Form::kEVX, evsrwiu,     { RS, RA, EVUIMM });
  DE(evsrws,        0x10000221, Form::kEVX, evsrws,      { RS, RA, RB });
  DE(evsrwu,        0x10000220, Form::kEVX, evsrwu,      { RS, RA, RB });
  DE(evstdd,        0x10000321, Form::kEVX, evstdd,      { RS, EVUIMM_8, RA });
  DE(evstddepx,     0x7C00073E, Form::kEVX, evstddepx,   { RS, RA, RB });
  DE(evstddx,       0x10000320, Form::kEVX, evstddx,     { RS, RA, RB });
  DE(evstdh,        0x10000325, Form::kEVX, evstdh,      { RS, EVUIMM_8, RA });
  DE(evstdhx,       0x10000324, Form::kEVX, evstdhx,     { RS, RA, RB });
  DE(evstdw,        0x10000323, Form::kEVX, evstdw,      { RS, EVUIMM_8, RA });
  DE(evstdwx,       0x10000322, Form::kEVX, evstdwx,     { RS, RA, RB });
  DE(evstwhe,       0x10000331, Form::kEVX, evstwhe,     { RS, EVUIMM_4, RA });
  DE(evstwhex,      0x10000330, Form::kEVX, evstwhex,    { RS, RA, RB });
  DE(evstwho,       0x10000335, Form::kEVX, evstwho,     { RS, EVUIMM_4, RA });
  DE(evstwhox,      0x10000334, Form::kEVX, evstwhox,    { RS, RA, RB });
  DE(evstwwe,       0x10000339, Form::kEVX, evstwwe,     { RS, EVUIMM_4, RA });
  DE(evstwwex,      0x10000338, Form::kEVX, evstwwex,    { RS, RA, RB });
  DE(evstwwo,       0x1000033D, Form::kEVX, evstwwo,     { RS, EVUIMM_4, RA });
  DE(evstwwox,      0x1000033C, Form::kEVX, evstwwox,    { RS, RA, RB });
  DE(evsubfsmiaaw,  0x100004CB, Form::kEVX, evsubfsmiaa, { RS, RA });
  DE(evsubfssiaaw,  0x100004C3, Form::kEVX, evsubfssiaa, { RS, RA });
  DE(evsubfumiaaw,  0x100004CA, Form::kEVX, evsubfumiaa, { RS, RA });
  DE(evsubfusiaaw,  0x100004C2, Form::kEVX, evsubfusiaa, { RS, RA });
  DE(evsubfw,       0x10000204, Form::kEVX, evsubfw,     { RS, RA, RB });
  DE(evsubifw,      0x10000206, Form::kEVX, evsubifw,    { RS, UIMM, RB });
  DE(evxor,         0x10000216, Form::kEVX, evxor,       { RS, RA, RB });
  DE(extsb,         0x7C000774, Form::kX,   extsb,       { RA, RS});
  DE(extsbdot,      0x7C000775, Form::kX,   extsb.,      { RA, RS});
  DE(extsh,         0x7C000734, Form::kX,   extsh,       { RA, RS });
  DE(extshdot,      0x7C000735, Form::kX,   extsh.,      { RA, RS });
  DE(extsw,         0x7C0007B4, Form::kX,   extsw,       { RA, RS });
  DE(extswdot,      0x7C0007B5, Form::kX,   extsw.,      { RA, RS });
  DE(fabs,          0xFC000210, Form::kX,   fabs,        { FRT, FRB });
  DE(fabsdot,       0xFC000211, Form::kX,   fabs.,       { FRT, FRB });
  DE(fadd,          0xFC00002A, Form::kA,   fadd,        { FRT, FRA, FRB });
  DE(fadddot,       0xFC00002B, Form::kA,   fadd.,       { FRT, FRA, FRB });
  DE(fadds,         0xEC00002A, Form::kA,   fadds,       { FRT, FRA, FRB });
  DE(faddsdot,      0xEC00002B, Form::kA,   fadds.,      { FRT, FRA, FRB });
  DE(fcfid,         0xFC00069C, Form::kX,   fcfid,       { FRT, FRB });
  DE(fcfiddot,      0xFC00069D, Form::kX,   fcfid.,      { FRT, FRB });
  DE(fcfids,        0xEC00069C, Form::kX,   fcfids,      { FRT, FRB });
  DE(fcfidsdot,     0xEC00069D, Form::kX,   fcfids.,     { FRT, FRB });
  DE(fcfidu,        0xFC00079C, Form::kX,   fcfidu,      { FRT, FRB });
  DE(fcfidudot,     0xFC00079D, Form::kX,   fcfidu.,     { FRT, FRB });
  DE(fcfidus,       0xEC00079C, Form::kX,   fcfidus,     { FRT, FRB });
  DE(fcfidusdot,    0xEC00079D, Form::kX,   fcfidus.,    { FRT, FRB });
  DE(fcmpo,         0xFC000040, Form::kX,   fcmpo,       { BF, FRA, FRB });
  DE(fcmpu,         0xFC000000, Form::kX,   fcmpu,       { BF, FRA, FRB });
  DE(fcpsgn,        0xFC000010, Form::kX,   fcpsgn,      { FRT, FRA, FRB });
  DE(fcpsgndot,     0xFC000011, Form::kX,   fcpsgn.,     { FRT, FRA, FRB });
  DE(fctid,         0xFC00065C, Form::kX,   fctid,       { FRT, FRB });
  DE(fctiddot,      0xFC00065D, Form::kX,   fctid.,      { FRT, FRB });
  DE(fctidu,        0xFC00075C, Form::kX,   fctidu,      { FRT, FRB });
  DE(fctidudot,     0xFC00075D, Form::kX,   fctidu.,     { FRT, FRB });
  DE(fctiduz,       0xFC00075E, Form::kX,   fctiduz,     { FRT, FRB });
  DE(fctiduzdot,    0xFC00075F, Form::kX,   fctiduz.,    { FRT, FRB });
  DE(fctidz,        0xFC00065E, Form::kX,   fctidz,      { FRT, FRB });
  DE(fctidzdot,     0xFC00065F, Form::kX,   fctidz.,     { FRT, FRB });
  DE(fctiw,         0xFC00001C, Form::kX,   fctiw,       { FRT, FRB });
  DE(fctiwdot,      0xFC00001D, Form::kX,   fctiw.,      { FRT, FRB });
  DE(fctiwu,        0xFC00011C, Form::kX,   fctiwu,      { FRT, FRB });
  DE(fctiwudot,     0xFC00011D, Form::kX,   fctiwu.,     { FRT, FRB });
  DE(fctiwuz,       0xFC00011E, Form::kX,   fctiwuz,     { FRT, FRB });
  DE(fctiwuzdot,    0xFC00011F, Form::kX,   fctiwuz.,    { FRT, FRB });
  DE(fctiwz,        0xFC00001E, Form::kX,   fctiwz,      { FRT, FRB });
  DE(fctiwzdot,     0xFC00001F, Form::kX,   fctiwz.,     { FRT, FRB });
  DE(fdiv,          0xFC000024, Form::kA,   fdiv,        { FRT, FRA, FRB });
  DE(fdivdot,       0xFC000025, Form::kA,   fdiv.,       { FRT, FRA, FRB });
  DE(fdivs,         0xEC000024, Form::kA,   fdivs,       { FRT, FRA, FRB });
  DE(fdivsdot,      0xEC000025, Form::kA,   fdivs.,      { FRT, FRA, FRB });
  DE(fmadd,         0xFC00003A, Form::kA,   fmadd,       { FRT,FRA,FRC,FRB });
  DE(fmadddot,      0xFC00003B, Form::kA,   fmadd.,      { FRT,FRA,FRC,FRB });
  DE(fmadds,        0xEC00003A, Form::kA,   fmadds,      { FRT,FRA,FRC,FRB });
  DE(fmaddsdot,     0xEC00003B, Form::kA,   fmadds.,     { FRT,FRA,FRC,FRB });
  DE(fmr,           0xFC000090, Form::kX,   fmr,         { FRT, FRB });
  DE(fmrdot,        0xFC000091, Form::kX,   fmr.,        { FRT, FRB });
  DE(fmrgew,        0xFC00078C, Form::kX,   fmrgew,      { FRT, FRA, FRB });
  DE(fmrgow,        0xFC00068C, Form::kX,   fmrgow,      { FRT, FRA, FRB });
  DE(fmsub,         0xFC000038, Form::kA,   fmsub,       { FRT,FRA,FRC,FRB });
  DE(fmsubdot,      0xFC000039, Form::kA,   fmsub.,      { FRT,FRA,FRC,FRB });
  DE(fmsubs,        0xEC000038, Form::kA,   fmsubs,      { FRT,FRA,FRC,FRB });
  DE(fmsubsdot,     0xEC000039, Form::kA,   fmsubs.,     { FRT,FRA,FRC,FRB });
  DE(fmul,          0xFC000032, Form::kA,   fmul,        { FRT, FRA, FRC });
  DE(fmuldot,       0xFC000033, Form::kA,   fmul.,       { FRT, FRA, FRC });
  DE(fmuls,         0xEC000032, Form::kA,   fmuls,       { FRT, FRA, FRC });
  DE(fmulsdot,      0xEC000033, Form::kA,   fmuls.,      { FRT, FRA, FRC });
  DE(fnabs,         0xFC000110, Form::kX,   fnabs,       { FRT, FRB });
  DE(fnabsdot,      0xFC000111, Form::kX,   fnabs.,      { FRT, FRB });
  DE(fneg,          0xFC000050, Form::kX,   fneg,        { FRT, FRB });
  DE(fnegdot,       0xFC000051, Form::kX,   fneg.,       { FRT, FRB });
  DE(fnmadd,        0xFC00003E, Form::kA,   fnmadd,      { FRT,FRA,FRC,FRB });
  DE(fnmadddot,     0xFC00003F, Form::kA,   fnmadd.,     { FRT,FRA,FRC,FRB });
  DE(fnmadds,       0xEC00003E, Form::kA,   fnmadds,     { FRT,FRA,FRC,FRB });
  DE(fnmaddsdot,    0xEC00003F, Form::kA,   fnmadds.,    { FRT,FRA,FRC,FRB });
  DE(fnmsub,        0xFC00003C, Form::kA,   fnmsub,      { FRT,FRA,FRC,FRB });
  DE(fnmsubdot,     0xFC00003D, Form::kA,   fnmsub.,     { FRT,FRA,FRC,FRB });
  DE(fnmsubs,       0xEC00003C, Form::kA,   fnmsubs,     { FRT,FRA,FRC,FRB });
  DE(fnmsubsdot,    0xEC00003D, Form::kA,   fnmsubs.,    { FRT,FRA,FRC,FRB });
  DE(fre,           0xFC000030, Form::kA,   fre,         { FRT, FRB, A_L });
  DE(fredot,        0xFC000031, Form::kA,   fre.,        { FRT, FRB, A_L });
  DE(fres,          0xEC000030, Form::kA,   fres,        { FRT, FRB, A_L });
  DE(fresdot,       0xEC000031, Form::kA,   fres.,       { FRT, FRB, A_L });
  DE(frim,          0xFC0003D0, Form::kX,   frim,        { FRT, FRB });
  DE(frimdot,       0xFC0003D1, Form::kX,   frim.,       { FRT, FRB });
  DE(frin,          0xFC000310, Form::kX,   frin,        { FRT, FRB });
  DE(frindot,       0xFC000311, Form::kX,   frin.,       { FRT, FRB });
  DE(frip,          0xFC000390, Form::kX,   frip,        { FRT, FRB });
  DE(fripdot,       0xFC000391, Form::kX,   frip.,       { FRT, FRB });
  DE(friz,          0xFC000350, Form::kX,   friz,        { FRT, FRB });
  DE(frizdot,       0xFC000351, Form::kX,   friz.,       { FRT, FRB });
  DE(frsp,          0xFC000018, Form::kX,   frsp,        { FRT, FRB });
  DE(frspdot,       0xFC000019, Form::kX,   frsp.,       { FRT, FRB });
  DE(frsqrte,       0xFC000034, Form::kA,   frsqrte,     { FRT, FRB, A_L });
  DE(frsqrtedot,    0xFC000035, Form::kA,   frsqrte.,    { FRT, FRB, A_L });
  DE(frsqrtes,      0xEC000034, Form::kA,   frsqrtes,    { FRT, FRB, A_L });
  DE(frsqrtesdot,   0xEC000035, Form::kA,   frsqrtes.,   { FRT, FRB, A_L });
  DE(fsel,          0xFC00002E, Form::kA,   fsel,        { FRT,FRA,FRC,FRB });
  DE(fseldot,       0xFC00002F, Form::kA,   fsel.,       { FRT,FRA,FRC,FRB });
  DE(fsqrt,         0xFC00002C, Form::kA,   fsqrt,       { FRT, FRB });
  DE(fsqrtdot,      0xFC00002D, Form::kA,   fsqrt.,      { FRT, FRB });
  DE(fsqrts,        0xEC00002C, Form::kA,   fsqrts,      { FRT, FRB });
  DE(fsqrtsdot,     0xEC00002D, Form::kA,   fsqrts.,     { FRT, FRB });
  DE(fsub,          0xFC000028, Form::kA,   fsub,        { FRT, FRA, FRB });
  DE(fsubdot,       0xFC000029, Form::kA,   fsub.,       { FRT, FRA, FRB });
  DE(fsubs,         0xEC000028, Form::kA,   fsubs,       { FRT, FRA, FRB });
  DE(fsubsdot,      0xEC000029, Form::kA,   fsubs.,      { FRT, FRA, FRB });
  DE(ftdiv,         0xFC000100, Form::kX,   ftdiv,       { BF, FRA, FRB});
  DE(ftsqrt,        0xFC000140, Form::kX,   ftsqrt,      { BF, FRB});
  DE(hrfid,         0x4C000224, Form::kXL,  hrfid,       { UN });
  DE(icbi,          0x7C0007AC, Form::kX,   icbi,        { RA, RB });
  DE(icbiep,        0x7C0007BE, Form::kX,   icbiep,      { RA, RB });
  DE(icblc,         0x7C0001CC, Form::kX,   icblc,       { CT, RA, RB });
  DE(icblqdot,      0x7C00018D, Form::kX,   icblq.,      { CT, RA, RB });
  DE(icbt,          0x7C00002C, Form::kX,   icbt,        { CT, RA, RB });
  DE(icbtls,        0x7C0003CC, Form::kX,   icbtls,      { CT, RA, RB });
  DE(ici,           0x7C00078C, Form::kX,   ici,         { CT});
  DE(icread,        0x7C0007CC, Form::kX,   icread,      { RA, RB });
  DE(isel,          0x7C00001E, Form::kA,   isel,        { RT, RA, RB, CRB });
  DE(isync,         0x4C00012C, Form::kXL,  isync,       { UN });
  DE(lbarx,         0x7C000068, Form::kX,   lbarx,       { RT, RA0, RB, EH });
  DE(lbdx,          0x7C000406, Form::kX,   lbdx,        { RT, RA, RB });
  DE(lbepx,         0x7C0000BE, Form::kX,   lbepx,       { RT, RA, RB });
  DE(lbz,           0x88000000, Form::kD,   lbz,         { RT, D, RA0 });
  DE(lbzcix,        0x7C0006AA, Form::kX,   lbzcix,      { RT, RA0, RB });
  DE(lbzu,          0x8C000000, Form::kD,   lbzu,        { RT, D, RAL });
  DE(lbzux,         0x7C0000EE, Form::kX,   lbzux,       { RT, RAL, RB });
  DE(lbzx,          0x7C0000AE, Form::kX,   lbzx,        { RT, RA0, RB });
  DE(ld,            0xE8000000, Form::kDS,  ld,          { RT, DS, RA0 });
  DE(ldarx,         0x7C0000A8, Form::kX,   ldarx,       { RT, RA0, RB, EH });
  DE(ldbrx,         0x7C000428, Form::kX,   ldbrx,       { RT, RA0, RB });
  DE(ldcix,         0x7C0006EA, Form::kX,   ldcix,       { RT, RA0, RB });
  DE(lddx,          0x7C0004C6, Form::kX,   lddx,        { RT, RA, RB });
  DE(ldepx,         0x7C00003A, Form::kX,   ldepx,       { RT, RA, RB });
  DE(ldu,           0xE8000001, Form::kDS,  ldu,         { RT, DS, RAL });
  DE(ldux,          0x7C00006A, Form::kX,   ldux,        { RT, RAL, RB });
  DE(ldx,           0x7C00002A, Form::kX,   ldx,         { RT, RA0, RB });
  DE(lfd,           0xC8000000, Form::kD,   lfd,         { FRT, D, RA0 });
  DE(lfddx,         0x7C000646, Form::kX,   lfddx,       { FRT, RA0, RB });
  DE(lfdepx,        0x7C0004BE, Form::kX,   lfdepx,      { FRT, RA0, RB });
  DE(lfdp,          0xE4000000, Form::kDS,  lfdp,        { FRT, D, RA0 });
  DE(lfdpx,         0x7C00062E, Form::kX,   lfdpx,       { FRT, RA, RB });
  DE(lfdu,          0xCC000000, Form::kD,   lfdu,        { FRT, D, RAS });
  DE(lfdux,         0x7C0004EE, Form::kX,   lfdux,       { FRT, RAS, RB });
  DE(lfdx,          0x7C0004AE, Form::kX,   lfdx,        { FRT, RA0, RB });
  DE(lfiwax,        0x7C0006AE, Form::kX,   lfiwax,      { FRT, RA0, RB });
  DE(lfiwzx,        0x7C0006EE, Form::kX,   lfiwzx,      { FRT, RA0, RB });
  DE(lfs,           0xC0000000, Form::kD,   lfs,         { FRT, D, RA0 });
  DE(lfsu,          0xC4000000, Form::kD,   lfsu,        { FRT, D, RAS });
  DE(lfsux,         0x7C00046E, Form::kX,   lfsux,       { FRT, RAS, RB });
  DE(lfsx,          0x7C00042E, Form::kX,   lfsx,        { FRT, RA0, RB });
  DE(lha,           0xA8000000, Form::kD,   lha,         { RT, D, RA0 });
  DE(lharx,         0x7C0000E8, Form::kX,   lharx,       { RT, RA0, RB, EH });
  DE(lhau,          0xAC000000, Form::kD,   lhau,        { RT, D, RAL });
  DE(lhaux,         0x7C0002EE, Form::kX,   lhaux,       { RT, RAL, RB });
  DE(lhax,          0x7C0002AE, Form::kX,   lhax,        { RT, RA0, RB });
  DE(lhbrx,         0x7C00062C, Form::kX,   lhbrx,       { RT, RA0, RB });
  DE(lhdx,          0x7C000446, Form::kX,   lhdx,        { RT, RA0, RB });
  DE(lhepx,         0x7C00023E, Form::kX,   lhepx,       { RT, RA0, RB });
  DE(lhz,           0xA0000000, Form::kD,   lhz,         { RT, D, RA0 });
  DE(lhzcix,        0x7C00066A, Form::kX,   lhzcix,      { RT, RA0, RB });
  DE(lhzu,          0xA4000000, Form::kD,   lhzu,        { RT, D, RAL });
  DE(lhzux,         0x7C00026E, Form::kX,   lhzux,       { RT, RAL, RB });
  DE(lhzx,          0x7C00022E, Form::kX,   lhzx,        { RT, RA0, RB });
  DE(lmw,           0xB8000000, Form::kD,   lmw,         { RT, D, RAM });
  DE(lq,            0xE0000000, Form::kDQ,  lq,          { RTQ, DQ, RAQ });
  DE(lqarx,         0x7C000228, Form::kX,   lqarx,       { RT, RA0, RB, EH });
  DE(lswi,          0x7C0004AA, Form::kX,   lswi,        { RT, RA0, NB });
  DE(lswx,          0x7C00042A, Form::kX,   lswx,        { RT, RA0, RB });
  DE(lvebx,         0x7C00000E, Form::kX,   lvebx,       { VD, RA, RB });
  DE(lvehx,         0x7C00004E, Form::kX,   lvehx,       { VD, RA, RB });
  DE(lvepx,         0x7C00024E, Form::kX,   lvepx,       { VD, RA, RB });
  DE(lvepxl,        0x7C00020E, Form::kX,   lvepxl,      { VD, RA, RB });
  DE(lvewx,         0x7C00008E, Form::kX,   lvewx,       { VD, RA, RB });
  DE(lvsl,          0x7C00000C, Form::kX,   lvsl,        { VD, RA, RB });
  DE(lvsr,          0x7C00004C, Form::kX,   lvsr,        { VD, RA, RB });
  DE(lvx,           0x7C0000CE, Form::kX,   lvx,         { VD, RA, RB });
  DE(lvxl,          0x7C0002CE, Form::kX,   lvxl,        { VD, RA, RB });
  DE(lwa,           0xE8000002, Form::kDS,  lwa,         { RT, DS, RA0 });
  DE(lwarx,         0x7C000028, Form::kX,   lwarx,       { RT, RA0, RB, EH });
  DE(lwaux,         0x7C0002EA, Form::kX,   lwaux,       { RT, RAL, RB });
  DE(lwax,          0x7C0002AA, Form::kX,   lwax,        { RT, RA0, RB });
  DE(lwbrx,         0x7C00042C, Form::kX,   lwbrx,       { RT, RA0, RB });
  DE(lwdx,          0x7C000486, Form::kX,   lwdx,        { RT, RA0, RB });
  DE(lwepx,         0x7C00003E, Form::kX,   lwepx,       { RT, RA0, RB });
  DE(lwz,           0x80000000, Form::kD,   lwz,         { RT, D, RA0 });
  DE(lwzcix,        0x7C00062A, Form::kX,   lwzcix,      { RT, RA0, RB });
  DE(lwzu,          0x84000000, Form::kD,   lwzu,        { RT, D, RAL });
  DE(lwzux,         0x7C00006E, Form::kX,   lwzux,       { RT, RAL, RB });
  DE(lwzx,          0x7C00002E, Form::kX,   lwzx,        { RT, RA0, RB });
  DE(lxsdx,         0x7C000498, Form::kXX1, lxsdx,       { XT, RA, RB });
  DE(lxsiwax,       0x7C000098, Form::kXX1, lxsiwax,     { XT, RA, RB });
  DE(lxsiwzx,       0x7C000018, Form::kXX1, lxsiwzx,     { XT, RA, RB });
  DE(lxsspx,        0x7C000418, Form::kXX1, lxsspx,      { XT, RA, RB });
  DE(lxvd2x,        0x7C000698, Form::kXX1, lxvd2x,      { XT, RA, RB });
  DE(lxvdsx,        0x7C000298, Form::kXX1, lxvdsx,      { XT, RA, RB });
  DE(lxvw4x,        0x7C000618, Form::kXX1, lxvw4x,      { XT, RA, RB });
  DE(macchw,        0x10000158, Form::kXO,  macchw,      { RT, RA, RB });
  DE(macchwdot,     0x10000159, Form::kXO,  macchw.,     { RT, RA, RB });
  DE(macchwo,       0x10000558, Form::kXO,  macchwo,     { RT, RA, RB });
  DE(macchwodot,    0x10000559, Form::kXO,  macchwo.,    { RT, RA, RB });
  DE(macchws,       0x100001D8, Form::kXO,  macchws,     { RT, RA, RB });
  DE(macchwsdot,    0x100001D9, Form::kXO,  macchws.,    { RT, RA, RB });
  DE(macchwso,      0x100005D8, Form::kXO,  macchwso,    { RT, RA, RB });
  DE(macchwsodot,   0x100005D9, Form::kXO,  macchwso.,   { RT, RA, RB });
  DE(macchwsu,      0x10000198, Form::kXO,  macchwsu,    { RT, RA, RB });
  DE(macchwsudot,   0x10000199, Form::kXO,  macchwsu.,   { RT, RA, RB });
  DE(macchwsuo,     0x10000598, Form::kXO,  macchwsuo,   { RT, RA, RB });
  DE(macchwsuodot,  0x10000599, Form::kXO,  macchwsuo.,  { RT, RA, RB });
  DE(macchwu,       0x10000118, Form::kXO,  macchwu,     { RT, RA, RB });
  DE(macchwudot,    0x10000119, Form::kXO,  macchwu.,    { RT, RA, RB });
  DE(macchwuo,      0x10000518, Form::kXO,  macchwuo,    { RT, RA, RB });
  DE(macchwuodot,   0x10000519, Form::kXO,  macchwuo.,   { RT, RA, RB });
  DE(machhw,        0x10000058, Form::kXO,  machhw,      { RT, RA, RB });
  DE(machhwdot,     0x10000059, Form::kXO,  machhw.,     { RT, RA, RB });
  DE(machhwo,       0x10000458, Form::kXO,  machhwo,     { RT, RA, RB });
  DE(machhwodot,    0x10000459, Form::kXO,  machhwo.,    { RT, RA, RB });
  DE(machhws,       0x100000D8, Form::kXO,  machhws,     { RT, RA, RB });
  DE(machhwsdot,    0x100000D9, Form::kXO,  machhws.,    { RT, RA, RB });
  DE(machhwso,      0x100004D8, Form::kXO,  machhwso,    { RT, RA, RB });
  DE(machhwsodot,   0x100004D9, Form::kXO,  machhwso.,   { RT, RA, RB });
  DE(machhwsu,      0x10000098, Form::kXO,  machhwsu,    { RT, RA, RB });
  DE(machhwsudot,   0x10000099, Form::kXO,  machhwsu.,   { RT, RA, RB });
  DE(machhwsuo,     0x10000498, Form::kXO,  machhwsuo,   { RT, RA, RB });
  DE(machhwsuodot,  0x10000499, Form::kXO,  machhwsuo.,  { RT, RA, RB });
  DE(machhwu,       0x10000018, Form::kXO,  machhwu,     { RT, RA, RB });
  DE(machhwudot,    0x10000019, Form::kXO,  machhwu.,    { RT, RA, RB });
  DE(machhwuo,      0x10000418, Form::kXO,  machhwuo,    { RT, RA, RB });
  DE(machhwuodot,   0x10000419, Form::kXO,  machhwuo.,   { RT, RA, RB });
  DE(maclhw,        0x10000358, Form::kXO,  maclhw,      { RT, RA, RB });
  DE(maclhwdot,     0x10000359, Form::kXO,  maclhw.,     { RT, RA, RB });
  DE(maclhwo,       0x10000758, Form::kXO,  maclhwo,     { RT, RA, RB });
  DE(maclhwodot,    0x10000759, Form::kXO,  maclhwo.,    { RT, RA, RB });
  DE(maclhws,       0x100003D8, Form::kXO,  maclhws,     { RT, RA, RB });
  DE(maclhwsdot,    0x100003D9, Form::kXO,  maclhws.,    { RT, RA, RB });
  DE(maclhwso,      0x100007D8, Form::kXO,  maclhwso,    { RT, RA, RB });
  DE(maclhwsodot,   0x100007D9, Form::kXO,  maclhwso.,   { RT, RA, RB });
  DE(maclhwsu,      0x10000398, Form::kXO,  maclhwsu,    { RT, RA, RB });
  DE(maclhwsudot,   0x10000399, Form::kXO,  maclhwsu.,   { RT, RA, RB });
  DE(maclhwsuo,     0x10000798, Form::kXO,  maclhwsuo,   { RT, RA, RB });
  DE(maclhwsuodot,  0x10000799, Form::kXO,  maclhwsuo.,  { RT, RA, RB });
  DE(maclhwu,       0x10000318, Form::kXO,  maclhwu,     { RT, RA, RB });
  DE(maclhwudot,    0x10000319, Form::kXO,  maclhwu.,    { RT, RA, RB });
  DE(maclhwuo,      0x10000718, Form::kXO,  maclhwuo,    { RT, RA, RB });
  DE(maclhwuodot,   0x10000719, Form::kXO,  maclhwuo.,   { RT, RA, RB });
  DE(mbar,          0x7C0006AC, Form::kX,   mbar,        { MO });
  DE(mcrf,          0x4C000000, Form::kXL,  mcrf,        { BF, BFA });
  DE(mcrfs,         0xFC000080, Form::kX,   mcrfs,       { BF, BFA });
  DE(mcrxr,         0x7C000400, Form::kX,   mcrxr,       { BF });
  DE(mfbhrbe,       0x7C00025C, Form::kXFX, mfbhrbe,     { RT, BHRBE});
  DE(mfcr,          0x7C000026, Form::kXFX, mfcr,        { RT });
  DE(mfdcr,         0x7C000286, Form::kXFX, mfdcr,       { RT, SPR });
  DE(mfdcrux,       0x7C000246, Form::kX,   mfdcrux,     { RT, RA });
  DE(mfdcrx,        0x7C000206, Form::kX,   mfdcrx,      { RS, RA });
  DE(mffs,          0xFC00048E, Form::kX,   mffs,        { FRT });
  DE(mffsdot,       0xFC00048D, Form::kX,   mffs.,       { FRT });
  DE(mfmsr,         0x7C0000A6, Form::kX,   mfmsr,       { RT });
  DE(mfocrf,        0x7C100026, Form::kXFX, mfocrf,      { RT, FXM });
  DE(mfpmr,         0x7C00029C, Form::kXFX, mfpmr,       { RT, PMR });
  DE(mfspr,         0x7C0002A6, Form::kXFX, mfspr,       { RT, SPR });
  DE(mfsr,          0x7C0004A6, Form::kX,   mfsr,        { RT, SR });
  DE(mfsrin,        0x7C000526, Form::kX,   mfsrin,      { RT, RB });
  DE(mftb,          0x7C0002E6, Form::kXFX, mftb,        { RT, TBR });
  DE(mfvscr,        0x10000604, Form::kVX,  mfvscr,      { VD });
  DE(mfvsrd,        0x7C000066, Form::kXX1, mfvsrd,      { RA, XS });
  DE(mfvsrwz,       0x7C0000E6, Form::kXX1, mfvsrwz,     { RA, XS });
  DE(msgclr,        0x7C0001DC, Form::kX,   msgclr,      { RB});
  DE(msgclrp,       0x7C00015C, Form::kX,   msgclrp,     { RB});
  DE(msgsnd,        0x7C00019C, Form::kX,   msgsnd,      { RB});
  DE(msgsndp,       0x7C00011C, Form::kX,   msgsndp,     { RB});
  DE(mtcrf,         0x7C000120, Form::kXFX, mtcrf,       { FXM, RS });
  DE(mtdcr,         0x7C000386, Form::kXFX, mtdcr,       { SPR, RS });
  DE(mtdcrux,       0x7C000346, Form::kX,   mtdcrux,     { RT, RA });
  DE(mtdcrx,        0x7C000306, Form::kX,   mtdcrx,      { RA, RS });
  DE(mtfsb0,        0xFC00008C, Form::kX,   mtfsb0,      { BT });
  DE(mtfsb0dot,     0xFC00008D, Form::kX,   mtfsb0.,     { BT });
  DE(mtfsb1,        0xFC00004C, Form::kX,   mtfsb1,      { BT });
  DE(mtfsb1dot,     0xFC00004D, Form::kX,   mtfsb1.,     { BT });
  DE(mtfsf,         0xFC00058E, Form::kXFL, mtfsf,      { FLM, FRB, XFL_L, W });
  DE(mtfsfdot,      0xFC00058F, Form::kXFL, mtfsf.,     { FLM, FRB, XFL_L, W });
  DE(mtfsfi,        0xFC00010C, Form::kX,   mtfsfi,      { BFF, U, W });
  DE(mtfsfidot,     0xFC00010D, Form::kX,   mtfsfi.,     { BFF, U, W });
  DE(mtmsr,         0x7C000124, Form::kX,   mtmsr,       { RS });
  DE(mtmsrd,        0x7C000164, Form::kX,   mtmsrd,      { RS, A_L });
  DE(mtocrf,        0x7C100120, Form::kXFX, mtocrf,      { FXM, RS });
  DE(mtpmr,         0x7C00039C, Form::kXFX, mtpmr,       { PMR, RS });
  DE(mtspr,         0x7C0003A6, Form::kXFX, mtspr,       { SPR, RS });
  DE(mtsr,          0x7C0001A4, Form::kX,   mtsr,        { SR, RS });
  DE(mtsrin,        0x7C0001E4, Form::kX,   mtsrin,      { RS, RB });
  DE(mtvscr,        0x10000644, Form::kVX,  mtvscr,      { VB });
  DE(mtvsrd,        0x7C000166, Form::kXX1, mtvsrd,      { XT, RA });
  DE(mtvsrwa,       0x7C0001A6, Form::kXX1, mtvsrwa,     { XT, RA });
  DE(mtvsrwz,       0x7C0001E6, Form::kXX1, mtvsrwz,     { XT, RA });
  DE(mulchw,        0x10000150, Form::kX,   mulchw,      { RT, RA, RB });
  DE(mulchwdot,     0x10000151, Form::kX,   mulchw.,     { RT, RA, RB });
  DE(mulchwu,       0x10000110, Form::kX,   mulchwu,     { RT, RA, RB });
  DE(mulchwudot,    0x10000111, Form::kX,   mulchwu.,    { RT, RA, RB });
  DE(mulhd,         0x7C000092, Form::kXO,  mulhd,       { RT, RA, RB });
  DE(mulhddot,      0x7C000093, Form::kXO,  mulhd.,      { RT, RA, RB });
  DE(mulhdu,        0x7C000012, Form::kXO,  mulhdu,      { RT, RA, RB });
  DE(mulhdudot,     0x7C000013, Form::kXO,  mulhdu.,     { RT, RA, RB });
  DE(mulhhw,        0x10000050, Form::kX,   mulhhw,      { RT, RA, RB });
  DE(mulhhwdot,     0x10000051, Form::kX,   mulhhw.,     { RT, RA, RB });
  DE(mulhhwu,       0x10000010, Form::kX,   mulhhwu,     { RT, RA, RB });
  DE(mulhhwudot,    0x10000011, Form::kX,   mulhhwu.,    { RT, RA, RB });
  DE(mulhw,         0x7C000096, Form::kXO,  mulhw,       { RT, RA, RB });
  DE(mulhwdot,      0x7C000097, Form::kXO,  mulhw.,      { RT, RA, RB });
  DE(mulhwu,        0x7C000016, Form::kXO,  mulhwu,      { RT, RA, RB });
  DE(mulhwudot,     0x7C000017, Form::kXO,  mulhwu.,     { RT, RA, RB });
  DE(mulld,         0x7C0001D2, Form::kXO,  mulld,       { RT, RA, RB });
  DE(mullddot,      0x7C0001D3, Form::kXO,  mulld.,      { RT, RA, RB });
  DE(mulldo,        0x7C0005D2, Form::kXO,  mulldo,      { RT, RA, RB });
  DE(mulldodot,     0x7C0005D3, Form::kXO,  mulldo.,     { RT, RA, RB });
  DE(mullhw,        0x10000350, Form::kX,   mullhw,      { RT, RA, RB });
  DE(mullhwdot,     0x10000351, Form::kX,   mullhw.,     { RT, RA, RB });
  DE(mullhwu,       0x10000310, Form::kX,   mullhwu,     { RT, RA, RB });
  DE(mullhwudot,    0x10000311, Form::kX,   mullhwu.,    { RT, RA, RB });
  DE(mulli,         0x1C000000, Form::kD,   mulli,       { RT, RA, SI });
  DE(mullw,         0x7C0001D6, Form::kXO,  mullw,       { RT, RA, RB });
  DE(mullwdot,      0x7C0001D7, Form::kXO,  mullw.,      { RT, RA, RB });
  DE(mullwo,        0x7C0005D6, Form::kXO,  mullwo,      { RT, RA, RB });
  DE(mullwodot,     0x7C0005D7, Form::kXO,  mullwo.,     { RT, RA, RB });
  DE(nand,          0x7C0003B8, Form::kX,   nand,        { RA, RS, RB });
  DE(nanddot,       0x7C0003B9, Form::kX,   nand.,       { RA, RS, RB });
  DE(nap,           0x4C000364, Form::kXL,  nap,         { UN });
  DE(neg,           0x7C0000D0, Form::kXO,  neg,         { RT, RA });
  DE(negdot,        0x7C0000D1, Form::kXO,  neg.,        { RT, RA });
  DE(nego,          0x7C0004D0, Form::kXO,  nego,        { RT, RA });
  DE(negodot,       0x7C0004D1, Form::kXO,  nego.,       { RT, RA });
  DE(nmacchw,       0x1000015C, Form::kXO,  nmacchw,     { RT, RA, RB });
  DE(nmacchwdot,    0x1000015D, Form::kXO,  nmacchw.,    { RT, RA, RB });
  DE(nmacchwo,      0x1000055C, Form::kXO,  nmacchwo,    { RT, RA, RB });
  DE(nmacchwodot,   0x1000055D, Form::kXO,  nmacchwo.,   { RT, RA, RB });
  DE(nmacchws,      0x100001DC, Form::kXO,  nmacchws,    { RT, RA, RB });
  DE(nmacchwsdot,   0x100001DD, Form::kXO,  nmacchws.,   { RT, RA, RB });
  DE(nmacchwso,     0x100005DC, Form::kXO,  nmacchwso,   { RT, RA, RB });
  DE(nmacchwsodot,  0x100005DD, Form::kXO,  nmacchwso.,  { RT, RA, RB });
  DE(nmachhw,       0x1000005C, Form::kXO,  nmachhw,     { RT, RA, RB });
  DE(nmachhwdot,    0x1000005D, Form::kXO,  nmachhw.,    { RT, RA, RB });
  DE(nmachhwo,      0x1000045C, Form::kXO,  nmachhwo,    { RT, RA, RB });
  DE(nmachhwodot,   0x1000045D, Form::kXO,  nmachhwo.,   { RT, RA, RB });
  DE(nmachhws,      0x100000DC, Form::kXO,  nmachhws,    { RT, RA, RB });
  DE(nmachhwsdot,   0x100000DD, Form::kXO,  nmachhws.,   { RT, RA, RB });
  DE(nmachhwso,     0x100004DC, Form::kXO,  nmachhwso,   { RT, RA, RB });
  DE(nmachhwsodot,  0x100004DD, Form::kXO,  nmachhwso.,  { RT, RA, RB });
  DE(nmaclhw,       0x1000035C, Form::kXO,  nmaclhw,     { RT, RA, RB });
  DE(nmaclhwdot,    0x1000035D, Form::kXO,  nmaclhw.,    { RT, RA, RB });
  DE(nmaclhwo,      0x1000075C, Form::kXO,  nmaclhwo,    { RT, RA, RB });
  DE(nmaclhwodot,   0x1000075D, Form::kXO,  nmaclhwo.,   { RT, RA, RB });
  DE(nmaclhws,      0x100003DC, Form::kXO,  nmaclhws,    { RT, RA, RB });
  DE(nmaclhwsdot,   0x100003DD, Form::kXO,  nmaclhws.,   { RT, RA, RB });
  DE(nmaclhwso,     0x100007DC, Form::kXO,  nmaclhwso,   { RT, RA, RB });
  DE(nmaclhwsodot,  0x100007DD, Form::kXO,  nmaclhwso.,  { RT, RA, RB });
  DE(nor,           0x7C0000F8, Form::kX,   nor,         { RA, RS, RB });
  DE(nordot,        0x7C0000F9, Form::kX,   nor.,        { RA, RS, RB });
  DE(or,            0x7C000378, Form::kX,   or,          { RA, RS, RB });
  DE(ordot,         0x7C000379, Form::kX,   or.,         { RA, RS, RB });
  DE(orc,           0x7C000338, Form::kX,   orc,         { RA, RS, RB });
  DE(orcdot,        0x7C000339, Form::kX,   orc.,        { RA, RS, RB });
  DE(ori,           0x60000000, Form::kD,   ori,         { RA, RS, UI });
  DE(oris,          0x64000000, Form::kD,   oris,        { RA, RS, UI });
  DE(popcntb,       0x7C0000F4, Form::kX,   popcntb,     { RA, RS });
  DE(popcntd,       0x7C0003F4, Form::kX,   popcntd,     { RA, RS });
  DE(popcntw,       0x7C0002F4, Form::kX,   popcntw,     { RA, RS });
  DE(prtyd,         0x7C000174, Form::kX,   prtyd,       { RA, RS });
  DE(prtyw,         0x7C000134, Form::kX,   prtyw,       { RA, RS });
  DE(rfci,          0x4C000066, Form::kXL,  rfci,        { UN });
  DE(rfdi,          0x4C00004E, Form::kX,   rfdi,        { UN });
  DE(rfebb,         0x4C000124, Form::kXL,  rfebb,       { S });
  DE(rfgi,          0x4C0000CC, Form::kXL,  rfgi,        { UN });
  DE(rfi,           0x4C000064, Form::kXL,  rfi,         { UN });
  DE(rfid,          0x4C000024, Form::kXL,  rfid,        { UN });
  DE(rfmci,         0x4C00004C, Form::kXL,  rfmci,       { UN });
  DE(rldcl,         0x78000010, Form::kMDS, rldcl,       { RA, RS, RB, MB6 });
  DE(rldcldot,      0x78000011, Form::kMDS, rldcl.,      { RA, RS, RB, MB6 });
  DE(rldcr,         0x78000012, Form::kMDS, rldcr,       { RA, RS, RB, ME6 });
  DE(rldcrdot,      0x78000013, Form::kMDS, rldcr.,      { RA, RS, RB, ME6 });
  DE(rldic,         0x78000008, Form::kMD,  rldic,       { RA, RS, SH6, MB6 });
  DE(rldicdot,      0x78000009, Form::kMD,  rldic.,      { RA, RS, SH6, MB6 });
  DE(rldicl,        0x78000000, Form::kMD,  rldicl,      { RA, RS, SH6, MB6 });
  DE(rldicldot,     0x78000001, Form::kMD,  rldicl.,     { RA, RS, SH6, MB6 });
  DE(rldicr,        0x78000004, Form::kMD,  rldicr,      { RA, RS, SH6, ME6 });
  DE(rldicrdot,     0x78000005, Form::kMD,  rldicr.,     { RA, RS, SH6, ME6 });
  DE(rldimi,        0x7800000C, Form::kMD,  rldimi,      { RA, RS, SH6, MB6 });
  DE(rldimidot,     0x7800000D, Form::kMD,  rldimi.,     { RA, RS, SH6, MB6 });
  DE(rlwimi,        0x50000000, Form::kM,   rlwimi,      { RA,RS,SH,MBE,ME });
  DE(rlwimidot,     0x50000001, Form::kM,   rlwimi.,     { RA,RS,SH,MBE,ME });
  DE(rlwinm,        0x54000000, Form::kM,   rlwinm,      { RA,RS,SH,MBE,ME });
  DE(rlwinmdot,     0x54000001, Form::kM,   rlwinm.,     { RA,RS,SH,MBE,ME });
  DE(rlwnm,         0x5C000000, Form::kM,   rlwnm,       { RA,RS,RB,MBE,ME });
  DE(rlwnmdot,      0x5C000001, Form::kM,   rlwnm.,      { RA,RS,RB,MBE,ME });
  DE(rvwinkle,      0x4C0003E4, Form::kXL,  rvwinkle,    { UN });
  DE(sc,            0x44000002, Form::kSC,  sc,          { LEV });
  DE(slbfeedot,     0x7C0007A7, Form::kX,   slbfee.,     { RT, RB });
  DE(slbia,         0x7C0003E4, Form::kX,   slbia,       { UN });
  DE(slbie,         0x7C000364, Form::kX,   slbie,       { RB });
  DE(slbmfee,       0x7C000726, Form::kX,   slbmfee,     { RT, RB });
  DE(slbmfev,       0x7C0006A6, Form::kX,   slbmfev,     { RT, RB });
  DE(slbmte,        0x7C000324, Form::kX,   slbmte,      { RS, RB });
  DE(sld,           0x7C000036, Form::kX,   sld,         { RA, RS, RB });
  DE(slddot,        0x7C000037, Form::kX,   sld.,        { RA, RS, RB });
  DE(sleep,         0x4C0003A4, Form::kXL,  sleep,       { UN });
  DE(slw,           0x7C000030, Form::kX,   slw,         { RA, RS, RB });
  DE(slwdot,        0x7C000031, Form::kX,   slw.,        { RA, RS, RB });
  DE(srad,          0x7C000634, Form::kX,   srad,        { RA, RS, RB });
  DE(sraddot,       0x7C000635, Form::kX,   srad.,       { RA, RS, RB });
  DE(sradi,         0x7C000674, Form::kXS,  sradi,       { RA, RS, SH6 });
  DE(sradidot,      0x7C000675, Form::kXS,  sradi.,      { RA, RS, SH6 });
  DE(sraw,          0x7C000630, Form::kX,   sraw,        { RA, RS, RB });
  DE(srawdot,       0x7C000631, Form::kX,   sraw.,       { RA, RS, RB });
  DE(srawi,         0x7C000670, Form::kX,   srawi,       { RA, RS, SH });
  DE(srawidot,      0x7C000671, Form::kX,   srawi.,      { RA, RS, SH });
  DE(srd,           0x7C000436, Form::kX,   srd,         { RA, RS, RB });
  DE(srddot,        0x7C000437, Form::kX,   srd.,        { RA, RS, RB });
  DE(srw,           0x7C000430, Form::kX,   srw,         { RA, RS, RB });
  DE(srwdot,        0x7C000431, Form::kX,   srw.,        { RA, RS, RB });
  DE(stb,           0x98000000, Form::kD,   stb,         { RS, D, RA0 });
  DE(stbcix,        0x7C0007AA, Form::kX,   stbcix,      { RS, RA0, RB });
  DE(stbcxdot,      0x7C00056D, Form::kX,   stbcx.,      { RS, RA0, RB });
  DE(stbdx,         0x7C000506, Form::kX,   stbdx,       { RS, RA0, RB });
  DE(stbepx,        0x7C0001BE, Form::kX,   stbepx,      { RS, RA0, RB });
  DE(stbu,          0x9C000000, Form::kD,   stbu,        { RS, D, RAS });
  DE(stbux,         0x7C0001EE, Form::kX,   stbux,       { RS, RAS, RB });
  DE(stbx,          0x7C0001AE, Form::kX,   stbx,        { RS, RA0, RB });
  DE(std,           0xF8000000, Form::kDS,  std,         { RS, DS, RA0 });
  DE(stdbrx,        0x7C000528, Form::kX,   stdbrx,      { RS, RA0, RB });
  DE(stdcix,        0x7C0007EA, Form::kX,   stdcix,      { RS, RA0, RB });
  DE(stdcxdot,      0x7C0001AD, Form::kX,   stdcx.,      { RS, RA0, RB });
  DE(stddx,         0x7C0005C6, Form::kX,   stddx,       { RS, RA0, RB });
  DE(stdepx,        0x7C00013A, Form::kX,   stdepx,      { RS, RA0, RB });
  DE(stdu,          0xF8000001, Form::kDS,  stdu,        { RS, DS, RAS });
  DE(stdux,         0x7C00016A, Form::kX,   stdux,       { RS, RAS, RB });
  DE(stdx,          0x7C00012A, Form::kX,   stdx,        { RS, RA0, RB });
  DE(stfd,          0xD8000000, Form::kD,   stfd,        { FRS, D, RA0 });
  DE(stfddx,        0x7C000746, Form::kX,   stfddx,      { FRS, RA, RB });
  DE(stfdepx,       0x7C0005BE, Form::kX,   stfdepx,     { FRS, RA, RB });
  DE(stfdp,         0xF4000000, Form::kDS,  stfdp,       { FRT, D, RA0 });
  DE(stfdpx,        0x7C00072E, Form::kX,   stfdpx,      { FRS, RA, RB });
  DE(stfdu,         0xDC000000, Form::kD,   stfdu,       { FRS, D, RAS });
  DE(stfdux,        0x7C0005EE, Form::kX,   stfdux,      { FRS, RAS, RB });
  DE(stfdx,         0x7C0005AE, Form::kX,   stfdx,       { FRS, RA0, RB });
  DE(stfiwx,        0x7C0007AE, Form::kX,   stfiwx,      { FRS, RA0, RB });
  DE(stfs,          0xD0000000, Form::kD,   stfs,        { FRS, D, RA0 });
  DE(stfsu,         0xD4000000, Form::kD,   stfsu,       { FRS, D, RAS });
  DE(stfsux,        0x7C00056E, Form::kX,   stfsux,      { FRS, RAS, RB });
  DE(stfsx,         0x7C00052E, Form::kX,   stfsx,       { FRS, RA0, RB });
  DE(sth,           0xB0000000, Form::kD,   sth,         { RS, D, RA0 });
  DE(sthbrx,        0x7C00072C, Form::kX,   sthbrx,      { RS, RA0, RB });
  DE(sthcix,        0x7C00076A, Form::kX,   sthcix,      { RS, RA0, RB });
  DE(sthcxdot,      0x7C0005AD, Form::kX,   sthcx.,      { RS, RA0, RB });
  DE(sthdx,         0x7C000546, Form::kX,   sthdx,       { RS, RA0, RB });
  DE(sthepx,        0x7C00033E, Form::kX,   sthepx,      { RS, RA0, RB });
  DE(sthu,          0xB4000000, Form::kD,   sthu,        { RS, D, RAS });
  DE(sthux,         0x7C00036E, Form::kX,   sthux,       { RS, RAS, RB });
  DE(sthx,          0x7C00032E, Form::kX,   sthx,        { RS, RA0, RB });
  DE(stmw,          0xBC000000, Form::kD,   stmw,        { RS, D, RA0 });
  DE(stq,           0xF8000002, Form::kDS,  stq,         { RSQ, DS, RA0 });
  DE(stqcxdot,      0x7C00016D, Form::kX,   stqcx.,      { RSQ, RA, RB });
  DE(stswi,         0x7C0005AA, Form::kX,   stswi,       { RT, RA0, NB });
  DE(stswx,         0x7C00052A, Form::kX,   stswx,       { RS, RA0, RB });
  DE(stvebx,        0x7C00010E, Form::kX,   stvebx,      { VS, RA, RB });
  DE(stvehx,        0x7C00014E, Form::kX,   stvehx,      { VS, RA, RB });
  DE(stvepx,        0x7C00064E, Form::kX,   stvepx,      { VS, RA, RB });
  DE(stvepxl,       0x7C00060E, Form::kX,   stvepxl,     { VS, RA, RB });
  DE(stvewx,        0x7C00018E, Form::kX,   stvewx,      { VS, RA, RB });
  DE(stvx,          0x7C0001CE, Form::kX,   stvx,        { VS, RA, RB });
  DE(stvxl,         0x7C0003CE, Form::kX,   stvxl,       { VS, RA, RB });
  DE(stw,           0x90000000, Form::kD,   stw,         { RS, D, RA0 });
  DE(stwbrx,        0x7C00052C, Form::kX,   stwbrx,      { RS, RA0, RB });
  DE(stwcix,        0x7C00072A, Form::kX,   stwcix,      { RS, RA0, RB });
  DE(stwcxdot,      0x7C00012D, Form::kX,   stwcx.,      { RS, RA0, RB });
  DE(stwdx,         0x7C000586, Form::kX,   stwdx,       { RS, RA0, RB });
  DE(stwepx,        0x7C00013E, Form::kX,   stwepx,      { RS, RA0, RB });
  DE(stwu,          0x94000000, Form::kD,   stwu,        { RS, D, RAS });
  DE(stwux,         0x7C00016E, Form::kX,   stwux,       { RS, RAS, RB });
  DE(stwx,          0x7C00012E, Form::kX,   stwx,        { RS, RA0, RB });
  DE(stxsdx,        0x7C000598, Form::kXX1, stxsdx,      { XS, RA, RB });
  DE(stxsiwx,       0x7C000118, Form::kXX1, stxsiwx,     { XS, RA, RB });
  DE(stxsspx,       0x7C000518, Form::kXX1, stxsspx,     { XS, RA, RB });
  DE(stxvd2x,       0x7C000798, Form::kXX1, stxvd2x,     { XS, RA, RB });
  DE(stxvw4x,       0x7C000718, Form::kXX1, stxvw4x,     { XS, RA, RB });
  DE(subf,          0x7C000050, Form::kXO,  subf,        { RT, RA, RB });
  DE(subfdot,       0x7C000051, Form::kXO,  subf.,       { RT, RA, RB });
  DE(subfc,         0x7C000010, Form::kXO,  subfc,       { RT, RA, RB });
  DE(subfcdot,      0x7C000011, Form::kXO,  subfc.,      { RT, RA, RB });
  DE(subfco,        0x7C000410, Form::kXO,  subfco,      { RT, RA, RB });
  DE(subfcodot,     0x7C000411, Form::kXO,  subfco.,     { RT, RA, RB });
  DE(subfe,         0x7C000110, Form::kXO,  subfe,       { RT, RA, RB });
  DE(subfedot,      0x7C000111, Form::kXO,  subfe.,      { RT, RA, RB });
  DE(subfeo,        0x7C000510, Form::kXO,  subfeo,      { RT, RA, RB });
  DE(subfeodot,     0x7C000511, Form::kXO,  subfeo.,     { RT, RA, RB });
  DE(subfic,        0x20000000, Form::kD,   subfic,      { RT, RA, SI });
  DE(subfme,        0x7C0001D0, Form::kXO,  subfme,      { RT, RA });
  DE(subfmedot,     0x7C0001D1, Form::kXO,  subfme.,     { RT, RA });
  DE(subfmeo,       0x7C0005D0, Form::kXO,  subfmeo,     { RT, RA });
  DE(subfmeodot,    0x7C0005D1, Form::kXO,  subfmeo.,    { RT, RA });
  DE(subfo,         0x7C000450, Form::kXO,  subfo,       { RT, RA, RB });
  DE(subfodot,      0x7C000451, Form::kXO,  subfo.,      { RT, RA, RB });
  DE(subfze,        0x7C000190, Form::kXO,  subfze,      { RT, RA });
  DE(subfzedot,     0x7C000191, Form::kXO,  subfze.,     { RT, RA });
  DE(subfzeo,       0x7C000590, Form::kXO,  subfzeo,     { RT, RA });
  DE(subfzeodot,    0x7C000591, Form::kXO,  subfzeo.,    { RT, RA });
  DE(sync,          0x7C0004AC, Form::kX,   sync,        { LS });
  DE(tabortdot,     0x7C00071D, Form::kX,   tabort.,     { RA});
  DE(tabortdcdot,   0x7C00065D, Form::kX,   tabortdc.,   { TO, RA, RB });
  DE(tabortdcidot,  0x7C0006DD, Form::kX,   tabortdci.,  { TO, RA, SI });
  DE(tabortwcdot,   0x7C00061D, Form::kX,   tabortwc.,   { TO, RA, RB });
  DE(tabortwcidot,  0x7C00069D, Form::kX,   tabortwci.,  { TO, RA, SI });
  DE(tbegindot,     0x7C00051D, Form::kX,   tbegin.,     { R });
  DE(tcheck,        0x7C00059C, Form::kX,   tcheck,      { BF });
  DE(td,            0x7C000088, Form::kX,   td,          { TO, RA, RB });
  DE(tdi,           0x08000000, Form::kD,   tdi,         { TO, RA, SI });
  DE(tenddot,       0x7C00055C, Form::kX,   tend.,       { A });
  DE(tlbia,         0x7C0002E4, Form::kX,   tlbia,       { UN });
  DE(tlbie,         0x7C000264, Form::kX,   tlbie,       { RB, L });
  DE(tlbiel,        0x7C000224, Form::kX,   tlbiel,      { RB, L });
  DE(tlbilx,        0x7C000024, Form::kX,   tlbilx,      { RA, RB });
  DE(tlbivax,       0x7C000624, Form::kX,   tlbivax,     { RA, RB });
  DE(tlbre,         0x7C000764, Form::kX,   tlbre,       { RSO, RAOPT, SHO });
  DE(tlbsrxdot,     0x7C0006A5, Form::kX,   tlbsrx.,     { RA, RB });
  DE(tlbsx,         0x7C000724, Form::kX,   tlbsx,       { RTO, RA, RB });
  DE(tlbsync,       0x7C00046C, Form::kX,   tlbsync,     { UN });
  DE(tlbwe,         0x7C0007A4, Form::kX,   tlbwe,       { RSO, RAOPT, SHO });
  DE(trechkptdot,   0x7C0007DD, Form::kX,   trechkpt.,   { UN });
  DE(treclaimdot,   0x7C00075D, Form::kX,   treclaim.,   { RA });
  DE(tsrdot,        0x7C0005DC, Form::kX,   tsr.,        { L });
  DE(tw,            0x7C000008, Form::kX,   tw,          { TO, RA, RB });
  DE(twi,           0x0C000000, Form::kD,   twi,         { TO, RA, SI });
  DE(vaddcuq,       0x10000140, Form::kVX,  vaddcuq,     { VD, VA, VB });
  DE(vaddcuw,       0x10000180, Form::kVX,  vaddcuw,     { VD, VA, VB });
  DE(vaddecuq,      0x1000003D, Form::kVA,  vaddecuq,    { VD, VA, VB, VC });
  DE(vaddeuqm,      0x1000003C, Form::kVA,  vaddeuqm,    { VD, VA, VB });
  DE(vaddfp,        0x1000000A, Form::kVX,  vaddfp,      { VD, VA, VB });
  DE(vaddsbs,       0x10000300, Form::kVX,  vaddsbs,     { VD, VA, VB });
  DE(vaddshs,       0x10000340, Form::kVX,  vaddshs,     { VD, VA, VB });
  DE(vaddsws,       0x10000380, Form::kVX,  vaddsws,     { VD, VA, VB });
  DE(vaddubm,       0x10000000, Form::kVX,  vaddubm,     { VD, VA, VB });
  DE(vaddubs,       0x10000200, Form::kVX,  vaddubs,     { VD, VA, VB });
  DE(vaddudm,       0x100000C0, Form::kVX,  vaddudm,     { VD, VA, VB });
  DE(vadduhm,       0x10000040, Form::kVX,  vadduhm,     { VD, VA, VB });
  DE(vadduhs,       0x10000240, Form::kVX,  vadduhs,     { VD, VA, VB });
  DE(vadduqm,       0x10000100, Form::kVX,  vadduqm,     { VD, VA, VB });
  DE(vadduwm,       0x10000080, Form::kVX,  vadduwm,     { VD, VA, VB });
  DE(vadduws,       0x10000280, Form::kVX,  vadduws,     { VD, VA, VB });
  DE(vand,          0x10000404, Form::kVX,  vand,        { VD, VA, VB });
  DE(vandc,         0x10000444, Form::kVX,  vandc,       { VD, VA, VB });
  DE(vavgsb,        0x10000502, Form::kVX,  vavgsb,      { VD, VA, VB });
  DE(vavgsh,        0x10000542, Form::kVX,  vavgsh,      { VD, VA, VB });
  DE(vavgsw,        0x10000582, Form::kVX,  vavgsw,      { VD, VA, VB });
  DE(vavgub,        0x10000402, Form::kVX,  vavgub,      { VD, VA, VB });
  DE(vavguh,        0x10000442, Form::kVX,  vavguh,      { VD, VA, VB });
  DE(vavguw,        0x10000482, Form::kVX,  vavguw,      { VD, VA, VB });
  DE(vbpermq,       0x1000054C, Form::kVX,  vbpermq,     { VD, VA, VB });
  DE(vcfsx,         0x1000034A, Form::kVX,  vcfsx,       { VD, VB, UIMM });
  DE(vcfux,         0x1000030A, Form::kVX,  vcfux,       { VD, VB, UIMM });
  DE(vcipher,       0x10000508, Form::kVX,  vcipher,     { VD, VA, VB });
  DE(vcipherlast,   0x10000509, Form::kVX,  vcipherlast, { VD, VA, VB });
  DE(vclzb,         0x10000702, Form::kVX,  vclzb,       { VD, VB });
  DE(vclzd,         0x100007C2, Form::kVX,  vclzd,       { VD, VB });
  DE(vclzh,         0x10000742, Form::kVX,  vclzh,       { VD, VB });
  DE(vclzw,         0x10000782, Form::kVX,  vclzw,       { VD, VB });
  DE(vcmpbfp,       0x100003C6, Form::kVC,  vcmpbfp,     { VD, VA, VB });
  DE(vcmpbfpdot,    0x100007C6, Form::kVC,  vcmpbfp.,    { VD, VA, VB });
  DE(vcmpeqfp,      0x100000C6, Form::kVC,  vcmpeqfp,    { VD, VA, VB });
  DE(vcmpeqfpdot,   0x100004C6, Form::kVC,  vcmpeqfp.,   { VD, VA, VB });
  DE(vcmpequb,      0x10000006, Form::kVC,  vcmpequb,    { VD, VA, VB });
  DE(vcmpequbdot,   0x10000406, Form::kVC,  vcmpequb.,   { VD, VA, VB });
  DE(vcmpequd,      0x100000C7, Form::kVC,  vcmpequd,    { VD, VA, VB });
  DE(vcmpequddot,   0x100004C7, Form::kVC,  vcmpequd.,   { VD, VA, VB });
  DE(vcmpequh,      0x10000046, Form::kVC,  vcmpequh,    { VD, VA, VB });
  DE(vcmpequhdot,   0x10000446, Form::kVC,  vcmpequh.,   { VD, VA, VB });
  DE(vcmpequw,      0x10000086, Form::kVC,  vcmpequw,    { VD, VA, VB });
  DE(vcmpequwdot,   0x10000486, Form::kVC,  vcmpequw.,   { VD, VA, VB });
  DE(vcmpgefp,      0x100001C6, Form::kVC,  vcmpgefp,    { VD, VA, VB });
  DE(vcmpgefpdot,   0x100005C6, Form::kVC,  vcmpgefp.,   { VD, VA, VB });
  DE(vcmpgtfp,      0x100002C6, Form::kVC,  vcmpgtfp,    { VD, VA, VB });
  DE(vcmpgtfpdot,   0x100006C6, Form::kVC,  vcmpgtfp.,   { VD, VA, VB });
  DE(vcmpgtsb,      0x10000306, Form::kVC,  vcmpgtsb,    { VD, VA, VB });
  DE(vcmpgtsbdot,   0x10000706, Form::kVC,  vcmpgtsb.,   { VD, VA, VB });
  DE(vcmpgtsd,      0x100003C7, Form::kVC,  vcmpgtsd,    { VD, VA, VB });
  DE(vcmpgtsddot,   0x100007C7, Form::kVC,  vcmpgtsd.,   { VD, VA, VB });
  DE(vcmpgtsh,      0x10000346, Form::kVC,  vcmpgtsh,    { VD, VA, VB });
  DE(vcmpgtshdot,   0x10000746, Form::kVC,  vcmpgtsh.,   { VD, VA, VB });
  DE(vcmpgtsw,      0x10000386, Form::kVC,  vcmpgtsw,    { VD, VA, VB });
  DE(vcmpgtswdot,   0x10000786, Form::kVC,  vcmpgtsw.,   { VD, VA, VB });
  DE(vcmpgtub,      0x10000206, Form::kVC,  vcmpgtub,    { VD, VA, VB });
  DE(vcmpgtubdot,   0x10000606, Form::kVC,  vcmpgtub.,   { VD, VA, VB });
  DE(vcmpgtud,      0x100002C7, Form::kVC,  vcmpgtud,    { VD, VA, VB });
  DE(vcmpgtuddot,   0x100006C7, Form::kVC,  vcmpgtud.,   { VD, VA, VB });
  DE(vcmpgtuh,      0x10000246, Form::kVC,  vcmpgtuh,    { VD, VA, VB });
  DE(vcmpgtuhdot,   0x10000646, Form::kVC,  vcmpgtuh.,   { VD, VA, VB });
  DE(vcmpgtuw,      0x10000286, Form::kVC,  vcmpgtuw,    { VD, VA, VB });
  DE(vcmpgtuwdot,   0x10000686, Form::kVC,  vcmpgtuw.,   { VD, VA, VB });
  DE(vctsxs,        0x100003CA, Form::kVX,  vctsxs,      { VD, VB, UIMM });
  DE(vctuxs,        0x1000038A, Form::kVX,  vctuxs,      { VD, VB, UIMM });
  DE(veqv,          0x10000684, Form::kVX,  veqv,        { VD, VA, VB });
  DE(vexptefp,      0x1000018A, Form::kVX,  vexptefp,    { VD, VB });
  DE(vgbbd,         0x1000050C, Form::kVX,  vgbbd,       { VD, VB });
  DE(vlogefp,       0x100001CA, Form::kVX,  vlogefp,     { VD, VB });
  DE(vmaddfp,       0x1000002E, Form::kVA,  vmaddfp,     { VD, VA, VC, VB });
  DE(vmaxfp,        0x1000040A, Form::kVX,  vmaxfp,      { VD, VA, VB });
  DE(vmaxsb,        0x10000102, Form::kVX,  vmaxsb,      { VD, VA, VB });
  DE(vmaxsd,        0x100001C2, Form::kVX,  vmaxsd,      { VD, VA, VB });
  DE(vmaxsh,        0x10000142, Form::kVX,  vmaxsh,      { VD, VA, VB });
  DE(vmaxsw,        0x10000182, Form::kVX,  vmaxsw,      { VD, VA, VB });
  DE(vmaxub,        0x10000002, Form::kVX,  vmaxub,      { VD, VA, VB });
  DE(vmaxud,        0x100000C2, Form::kVX,  vmaxud,      { VD, VA, VB });
  DE(vmaxuh,        0x10000042, Form::kVX,  vmaxuh,      { VD, VA, VB });
  DE(vmaxuw,        0x10000082, Form::kVX,  vmaxuw,      { VD, VA, VB });
  DE(vmhaddshs,     0x10000020, Form::kVA,  vmhaddshs,   { VD, VA, VB, VC });
  DE(vmhraddshs,    0x10000021, Form::kVA,  vmhraddshs,  { VD, VA, VB, VC });
  DE(vminfp,        0x1000044A, Form::kVX,  vminfp,      { VD, VA, VB });
  DE(vminsb,        0x10000302, Form::kVX,  vminsb,      { VD, VA, VB });
  DE(vminsd,        0x100003C2, Form::kX,   vminsd,      { VD, VA, VB });
  DE(vminsh,        0x10000342, Form::kVX,  vminsh,      { VD, VA, VB });
  DE(vminsw,        0x10000382, Form::kVX,  vminsw,      { VD, VA, VB });
  DE(vminub,        0x10000202, Form::kVX,  vminub,      { VD, VA, VB });
  DE(vminud,        0x100002C2, Form::kVX,  vminud,      { VD, VA, VB });
  DE(vminuh,        0x10000242, Form::kVX,  vminuh,      { VD, VA, VB });
  DE(vminuw,        0x10000282, Form::kVX,  vminuw,      { VD, VA, VB });
  DE(vmladduhm,     0x10000022, Form::kVA,  vmladduhm,   { VD, VA, VB, VC });
  DE(vmrgew,        0x1000078C, Form::kVX,  vmrgew,      { VD, VA, VB });
  DE(vmrghb,        0x1000000C, Form::kVX,  vmrghb,      { VD, VA, VB });
  DE(vmrghh,        0x1000004C, Form::kVX,  vmrghh,      { VD, VA, VB });
  DE(vmrghw,        0x1000008C, Form::kVX,  vmrghw,      { VD, VA, VB });
  DE(vmrglb,        0x1000010C, Form::kVX,  vmrglb,      { VD, VA, VB });
  DE(vmrglh,        0x1000014C, Form::kVX,  vmrglh,      { VD, VA, VB });
  DE(vmrglw,        0x1000018C, Form::kVX,  vmrglw,      { VD, VA, VB });
  DE(vmrgow,        0x1000068C, Form::kVX,  vmrgow,      { VD, VA, VB });
  DE(vmsummbm,      0x10000025, Form::kVA,  vmsummbm,    { VD, VA, VB, VC });
  DE(vmsumshm,      0x10000028, Form::kVA,  vmsumshm,    { VD, VA, VB, VC });
  DE(vmsumshs,      0x10000029, Form::kVA,  vmsumshs,    { VD, VA, VB, VC });
  DE(vmsumubm,      0x10000024, Form::kVA,  vmsumubm,    { VD, VA, VB, VC });
  DE(vmsumuhm,      0x10000026, Form::kVA,  vmsumuhm,    { VD, VA, VB, VC });
  DE(vmsumuhs,      0x10000027, Form::kVA,  vmsumuhs,    { VD, VA, VB, VC });
  DE(vmulesb,       0x10000308, Form::kVX,  vmulesb,     { VD, VA, VB });
  DE(vmulesh,       0x10000348, Form::kVX,  vmulesh,     { VD, VA, VB });
  DE(vmulesw,       0x10000388, Form::kVX,  vmulesw,     { VD, VA, VB });
  DE(vmuleub,       0x10000208, Form::kVX,  vmuleub,     { VD, VA, VB });
  DE(vmuleuh,       0x10000248, Form::kVX,  vmuleuh,     { VD, VA, VB });
  DE(vmuleuw,       0x10000288, Form::kVX,  vmuleuw,     { VD, VA, VB });
  DE(vmulosb,       0x10000108, Form::kVX,  vmulosb,     { VD, VA, VB });
  DE(vmulosh,       0x10000148, Form::kVX,  vmulosh,     { VD, VA, VB });
  DE(vmulosw,       0x10000188, Form::kVX,  vmulosw,     { VD, VA, VB });
  DE(vmuloub,       0x10000008, Form::kVX,  vmuloub,     { VD, VA, VB });
  DE(vmulouh,       0x10000048, Form::kVX,  vmulouh,     { VD, VA, VB });
  DE(vmulouw,       0x10000088, Form::kVX,  vmulouw,     { VD, VA, VB });
  DE(vmuluwm,       0x10000089, Form::kVX,  vmuluwm,     { VD, VA, VB });
  DE(vnand,         0x10000584, Form::kVX,  vnand,       { VD, VA, VB });
  DE(vncipher,      0x10000548, Form::kVX,  vncipher,    { VD, VA, VB });
  DE(vncipherlast,  0x10000549, Form::kVX,  vncipherlas, { VD, VA, VB });
  DE(vnmsubfp,      0x1000002F, Form::kVA,  vnmsubfp,    { VD, VA, VC, VB });
  DE(vnor,          0x10000504, Form::kVX,  vnor,        { VD, VA, VB });
  DE(vor,           0x10000484, Form::kVX,  vor,         { VD, VA, VB });
  DE(vorc,          0x10000544, Form::kVX,  vorc,        { VD, VA, VB });
  DE(vperm,         0x1000002B, Form::kVA,  vperm,       { VD, VA, VB, VC });
  DE(vpermxor,      0x1000002D, Form::kVA,  vpermxor,    { VD, VA, VB, VC });
  DE(vpkpx,         0x1000030E, Form::kVX,  vpkpx,       { VD, VA, VB });
  DE(vpksdss,       0x100005CE, Form::kVX,  vpksdss,     { VD, VA, VB });
  DE(vpksdus,       0x1000054E, Form::kVX,  vpksdus,     { VD, VA, VB });
  DE(vpkshss,       0x1000018E, Form::kVX,  vpkshss,     { VD, VA, VB });
  DE(vpkshus,       0x1000010E, Form::kVX,  vpkshus,     { VD, VA, VB });
  DE(vpkswss,       0x100001CE, Form::kVX,  vpkswss,     { VD, VA, VB });
  DE(vpkswus,       0x1000014E, Form::kVX,  vpkswus,     { VD, VA, VB });
  DE(vpkudum,       0x1000044E, Form::kVX,  vpkudum,     { VD, VA, VB });
  DE(vpkudus,       0x100004CE, Form::kVX,  vpkudus,     { VD, VA, VB });
  DE(vpkuhum,       0x1000000E, Form::kVX,  vpkuhum,     { VD, VA, VB });
  DE(vpkuhus,       0x1000008E, Form::kVX,  vpkuhus,     { VD, VA, VB });
  DE(vpkuwum,       0x1000004E, Form::kVX,  vpkuwum,     { VD, VA, VB });
  DE(vpkuwus,       0x100000CE, Form::kVX,  vpkuwus,     { VD, VA, VB });
  DE(vpmsumb,       0x10000408, Form::kVX,  vpmsumb,     { VD, VA, VB });
  DE(vpmsumd,       0x100004C8, Form::kVX,  vpmsumd,     { VD, VA, VB });
  DE(vpmsumh,       0x10000448, Form::kVX,  vpmsumh,     { VD, VA, VB });
  DE(vpmsumw,       0x10000488, Form::kVX,  vpmsumw,     { VD, VA, VB });
  DE(vpopcntb,      0x10000703, Form::kVX,  vpopcntb,    { VD, VB });
  DE(vpopcntd,      0x100007C3, Form::kVX,  vpopcntd,    { VD, VB });
  DE(vpopcnth,      0x10000743, Form::kVX,  vpopcnth,    { VD, VB });
  DE(vpopcntw,      0x10000783, Form::kVX,  vpopcntw,    { VD, VB });
  DE(vrefp,         0x1000010A, Form::kVX,  vrefp,       { VD, VB });
  DE(vrfim,         0x100002CA, Form::kVX,  vrfim,       { VD, VB });
  DE(vrfin,         0x1000020A, Form::kVX,  vrfin,       { VD, VB });
  DE(vrfip,         0x1000028A, Form::kVX,  vrfip,       { VD, VB });
  DE(vrfiz,         0x1000024A, Form::kVX,  vrfiz,       { VD, VB });
  DE(vrlb,          0x10000004, Form::kVX,  vrlb,        { VD, VA, VB });
  DE(vrld,          0x100000C4, Form::kVX,  vrld,        { VD, VA, VB });
  DE(vrlh,          0x10000044, Form::kVX,  vrlh,        { VD, VA, VB });
  DE(vrlw,          0x10000084, Form::kVX,  vrlw,        { VD, VA, VB });
  DE(vrsqrtefp,     0x1000014A, Form::kVX,  vrsqrtefp,   { VD, VB });
  DE(vsbox,         0x100005C8, Form::kVX,  vsbox,       { VD, VB });
  DE(vsel,          0x1000002A, Form::kVA,  vsel,        { VD, VA, VB, VC });
  DE(vshasigmad,    0x100006C2, Form::kVX,  vshasigmad,  { VD, VA, ST, SIX });
  DE(vshasigmaw,    0x10000682, Form::kVX,  vshasigmaw,  { VD, VA, ST, SIX });
  DE(vsl,           0x100001C4, Form::kVX,  vsl,         { VD, VA, VB });
  DE(vslb,          0x10000104, Form::kVX,  vslb,        { VD, VA, VB });
  DE(vsld,          0x100005C4, Form::kVX,  vsld,        { VD, VA, VB, SHB });
  DE(vsldoi,        0x1000002C, Form::kVA,  vsldoi,      { VD, VA, VB, SHB });
  DE(vslh,          0x10000144, Form::kVX,  vslh,        { VD, VA, VB });
  DE(vslo,          0x1000040C, Form::kVX,  vslo,        { VD, VA, VB });
  DE(vslw,          0x10000184, Form::kVX,  vslw,        { VD, VA, VB });
  DE(vspltb,        0x1000020C, Form::kVX,  vspltb,      { VD, VB, UIMM });
  DE(vsplth,        0x1000024C, Form::kVX,  vsplth,      { VD, VB, UIMM });
  DE(vspltisb,      0x1000030C, Form::kVX,  vspltisb,    { VD, SIMM });
  DE(vspltish,      0x1000034C, Form::kVX,  vspltish,    { VD, SIMM });
  DE(vspltisw,      0x1000038C, Form::kVX,  vspltisw,    { VD, SIMM });
  DE(vspltw,        0x1000028C, Form::kVX,  vspltw,      { VD, VB, UIMM });
  DE(vsr,           0x100002C4, Form::kVX,  vsr,         { VD, VA, VB });
  DE(vsrab,         0x10000304, Form::kVX,  vsrab,       { VD, VA, VB });
  DE(vsrad,         0x100003C4, Form::kVX,  vsrad,       { VD, VA, VB });
  DE(vsrah,         0x10000344, Form::kVX,  vsrah,       { VD, VA, VB });
  DE(vsraw,         0x10000384, Form::kVX,  vsraw,       { VD, VA, VB });
  DE(vsrb,          0x10000204, Form::kVX,  vsrb,        { VD, VA, VB });
  DE(vsrd,          0x100006C4, Form::kVX,  vsrd,        { VD, VA, VB });
  DE(vsrh,          0x10000244, Form::kVX,  vsrh,        { VD, VA, VB });
  DE(vsro,          0x1000044C, Form::kVX,  vsro,        { VD, VA, VB });
  DE(vsrw,          0x10000284, Form::kVX,  vsrw,        { VD, VA, VB });
  DE(vsubcuq,       0x10000540, Form::kVX,  vsubcuq,     { VD, VA, VB });
  DE(vsubcuw,       0x10000580, Form::kVX,  vsubcuw,     { VD, VA, VB });
  DE(vsubecuq,      0x1000003F, Form::kVA,  vsubecuq,    { VD, VA, VB, VC });
  DE(vsubeuqm,      0x1000003E, Form::kVA,  vsubeuqm,    { VD, VA, VB, VC });
  DE(vsubfp,        0x1000004A, Form::kVX,  vsubfp,      { VD, VA, VB });
  DE(vsubsbs,       0x10000700, Form::kVX,  vsubsbs,     { VD, VA, VB });
  DE(vsubshs,       0x10000740, Form::kVX,  vsubshs,     { VD, VA, VB });
  DE(vsubsws,       0x10000780, Form::kVX,  vsubsws,     { VD, VA, VB });
  DE(vsububm,       0x10000400, Form::kVX,  vsububm,     { VD, VA, VB });
  DE(vsububs,       0x10000600, Form::kVX,  vsububs,     { VD, VA, VB });
  DE(vsubudm,       0x100004C0, Form::kVX,  vsubudm,     { VD, VA, VB });
  DE(vsubuhm,       0x10000440, Form::kVX,  vsubuhm,     { VD, VA, VB });
  DE(vsubuhs,       0x10000640, Form::kVX,  vsubuhs,     { VD, VA, VB });
  DE(vsubuqm,       0x10000500, Form::kVX,  vsubuqm,     { VD, VA, VB });
  DE(vsubuwm,       0x10000480, Form::kVX,  vsubuwm,     { VD, VA, VB });
  DE(vsubuws,       0x10000680, Form::kVX,  vsubuws,     { VD, VA, VB });
  DE(vsum2sws,      0x10000688, Form::kVX,  vsum2sws,    { VD, VA, VB });
  DE(vsum4sbs,      0x10000708, Form::kVX,  vsum4sbs,    { VD, VA, VB });
  DE(vsum4shs,      0x10000648, Form::kVX,  vsum4shs,    { VD, VA, VB });
  DE(vsum4ubs,      0x10000608, Form::kVX,  vsum4ubs,    { VD, VA, VB });
  DE(vsumsws,       0x10000788, Form::kVX,  vsumsws,     { VD, VA, VB });
  DE(vupkhpx,       0x1000034E, Form::kVX,  vupkhpx,     { VD, VB });
  DE(vupkhsb,       0x1000020E, Form::kVX,  vupkhsb,     { VD, VB });
  DE(vupkhsh,       0x1000024E, Form::kVX,  vupkhsh,     { VD, VB });
  DE(vupkhsw,       0x1000064E, Form::kVX,  vupkhsw,     { VD, VB });
  DE(vupklpx,       0x100003CE, Form::kVX,  vupklpx,     { VD, VB });
  DE(vupklsb,       0x1000028E, Form::kVX,  vupklsb,     { VD, VB });
  DE(vupklsh,       0x100002CE, Form::kVX,  vupklsh,     { VD, VB });
  DE(vupklsw,       0x100006CE, Form::kVX,  vupklsw,     { VD, VB });
  DE(vxor,          0x100004C4, Form::kVX,  vxor,        { VD, VA, VB });
  DE(wait,          0x7C00007C, Form::kX,   wait,        { WC });
  DE(wrtee,         0x7C000106, Form::kX,   wrtee,       { RS });
  DE(wrteei,        0x7C000146, Form::kX,   wrteei,      { E });
  DE(xnop,          0x68000000, Form::kX,   xnop,        { UN });
  DE(xor,           0x7C000278, Form::kX,   xor,         { RA, RS, RB });
  DE(xordot,        0x7C000279, Form::kX,   xor.,        { RA, RS, RB });
  DE(xori,          0x68000000, Form::kD,   xori,        { RA, RS, UI });
  DE(xoris,         0x6C000000, Form::kD,   xoris,       { RA, RS, UI });
  DE(xsabsdp,       0xF0000564, Form::kXX2, xsabsdp,     { XT, XB });
  DE(xsadddp,       0xF0000100, Form::kXX3, xsadddp,     { XT, XA, XB });
  DE(xsaddsp,       0xF0000000, Form::kXX3, xsaddsp,     { XT, XA, XB });
  DE(xscmpodp,      0xF0000158, Form::kXX3, xscmpodp,    { BF, XA, XB });
  DE(xscmpudp,      0xF0000118, Form::kXX3, xscmpudp,    { BF, XA, XB });
  DE(xscpsgndp,     0xF0000580, Form::kXX3, xscpsgndp,   { XT, XA, XB });
  DE(xscvdpsp,      0xF0000424, Form::kXX2, xscvdpsp,    { XT, XB });
  DE(xscvdpspn,     0xF000042C, Form::kXX2, xscvdpspn,   { XT, XB });
  DE(xscvdpsxds,    0xF0000560, Form::kXX2, xscvdpsxds,  { XT, XB });
  DE(xscvdpsxws,    0xF0000160, Form::kXX2, xscvdpsxws,  { XT, XB });
  DE(xscvdpuxds,    0xF0000520, Form::kXX2, xscvdpuxds,  { XT, XB });
  DE(xscvdpuxws,    0xF0000120, Form::kXX2, xscvdpuxws,  { XT, XB });
  DE(xscvspdp,      0xF0000524, Form::kXX2, xscvspdp,    { XT, XB });
  DE(xscvspdpn,     0xF000052C, Form::kXX2, xscvspdpn,   { XT, XB });
  DE(xscvsxddp,     0xF00005E0, Form::kXX2, xscvsxddp,   { XT, XB });
  DE(xscvsxdsp,     0xF00004E0, Form::kXX2, xscvsxdsp,   { XT, XB });
  DE(xscvuxddp,     0xF00005A0, Form::kXX2, xscvuxddp,   { XT, XB });
  DE(xscvuxdsp,     0xF00004A0, Form::kXX2, xscvuxdsp,   { XT, XB });
  DE(xsdivdp,       0xF00001C0, Form::kXX3, xsdivdp,     { XT, XA, XB });
  DE(xsdivsp,       0xF00000C0, Form::kXX3, xsdivsp,     { XT, XA, XB });
  DE(xsmaddadp,     0xF0000108, Form::kXX3, xsmaddadp,   { XT, XA, XB });
  DE(xsmaddasp,     0xF0000008, Form::kXX3, xsmaddasp,   { XT, XA, XB });
  DE(xsmaddmdp,     0xF0000148, Form::kXX3, xsmaddmdp,   { XT, XA, XB });
  DE(xsmaddmsp,     0xF0000048, Form::kXX3, xsmaddmsp,   { XT, XA, XB });
  DE(xsmaxdp,       0xF0000500, Form::kXX3, xsmaxdp,     { XT, XA, XB });
  DE(xsmindp,       0xF0000540, Form::kXX3, xsmindp,     { XT, XA, XB });
  DE(xsmsubadp,     0xF0000188, Form::kXX3, xsmsubadp,   { XT, XA, XB });
  DE(xsmsubasp,     0xF0000088, Form::kXX3, xsmsubasp,   { XT, XA, XB });
  DE(xsmsubmdp,     0xF00001C8, Form::kXX3, xsmsubmdp,   { XT, XA, XB });
  DE(xsmsubmsp,     0xF00000C8, Form::kXX3, xsmsubmsp,   { XT, XA, XB });
  DE(xsmuldp,       0xF0000180, Form::kXX3, xsmuldp,     { XT, XA, XB });
  DE(xsmulsp,       0xF0000080, Form::kXX3, xsmulsp,     { XT, XA, XB });
  DE(xsnabsdp,      0xF00005A4, Form::kXX2, xsnabsdp,    { XT, XA, XB });
  DE(xsnegdp,       0xF00005E4, Form::kXX2, xsnegdp,     { XT, XA, XB });
  DE(xsnmaddadp,    0xF0000508, Form::kXX3, xsnmaddadp,  { XT, XA, XB });
  DE(xsnmaddasp,    0xF0000408, Form::kXX3, xsnmaddasp,  { XT, XA, XB });
  DE(xsnmaddmdp,    0xF0000548, Form::kXX3, xsnmaddmdp,  { XT, XA, XB });
  DE(xsnmaddmsp,    0xF0000448, Form::kXX3, xsnmaddmsp,  { XT, XA, XB });
  DE(xsnmsubadp,    0xF0000588, Form::kXX3, xsnmsubadp,  { XT, XA, XB });
  DE(xsnmsubasp,    0xF0000488, Form::kXX3, xsnmsubasp,  { XT, XA, XB });
  DE(xsnmsubmdp,    0xF00005C8, Form::kXX3, xsnmsubmdp,  { XT, XA, XB });
  DE(xsnmsubmsp,    0xF00004C8, Form::kXX3, xsnmsubmsp,  { XT, XA, XB });
  DE(xsrdpi,        0xF0000124, Form::kXX2, xsrdpi,      { XT, XB });
  DE(xsrdpic,       0xF00001AC, Form::kXX2, xsrdpic,     { XT, XB });
  DE(xsrdpim,       0xF00001E4, Form::kXX2, xsrdpim,     { XT, XB });
  DE(xsrdpip,       0xF00001A4, Form::kXX2, xsrdpip,     { XT, XB });
  DE(xsrdpiz,       0xF0000164, Form::kXX2, xsrdpiz,     { XT, XB });
  DE(xsredp,        0xF0000168, Form::kXX1, xsredp,      { XT, XB });
  DE(xsresp,        0xF0000068, Form::kXX2, xsresp,      { XT, XB });
  DE(xsrsp,         0xF0000464, Form::kXX2, xsrsp,       { XT, XB });
  DE(xsrsqrtedp,    0xF0000128, Form::kXX2, xsrsqrtedp,  { XT, XB });
  DE(xsrsqrtesp,    0xF0000028, Form::kXX2, xsrsqrtesp,  { XT, XB });
  DE(xssqrtdp,      0xF000012C, Form::kXX2, xssqrtdp,    { XT, XB });
  DE(xssqrtsp,      0xF000002C, Form::kXX2, xssqrtsp,    { XT, XB });
  DE(xssubdp,       0xF0000140, Form::kXX3, xssubdp,     { XT, XA, XB });
  DE(xssubsp,       0xF0000040, Form::kXX3, xssubsp,     { XT, XA, XB });
  DE(xstdivdp,      0xF00001E8, Form::kXX3, xstdivdp,    { BF, XA, XB });
  DE(xstsqrtdp,     0xF00001A8, Form::kXX2, xstsqrtdp,   { BF, XB });
  DE(xvabsdp,       0xF0000764, Form::kXX2, xvabsdp,     { XT, XB });
  DE(xvabssp,       0xF0000664, Form::kXX2, xvabssp,     { XT, XB });
  DE(xvadddp,       0xF0000300, Form::kXX3, xvadddp,     { XT, XB });
  DE(xvaddsp,       0xF0000200, Form::kXX3, xvaddsp,     { XT, XB });
  DE(xvcmpeqdp,     0xF0000318, Form::kXX3, xvcmpeqdp,   { XT, XA, XB });
  DE(xvcmpeqdpdot,  0xF0000718, Form::kXX3, xvcmpeqdp.,  { XT, XA, XB });
  DE(xvcmpeqsp,     0xF0000218, Form::kXX3, xvcmpeqsp,   { XT, XA, XB });
  DE(xvcmpeqspdot,  0xF0000618, Form::kXX3, xvcmpeqsp.,  { XT, XA, XB });
  DE(xvcmpgedp,     0xF0000398, Form::kXX3, xvcmpgedp,   { XT, XA, XB });
  DE(xvcmpgedpdot,  0xF0000798, Form::kXX3, xvcmpgedp.,  { XT, XA, XB });
  DE(xvcmpgesp,     0xF0000298, Form::kXX3, xvcmpgesp,   { XT, XA, XB });
  DE(xvcmpgespdot,  0xF0000698, Form::kXX3, xvcmpgesp.,  { XT, XA, XB });
  DE(xvcmpgtdp,     0xF0000358, Form::kXX3, xvcmpgtdp,   { XT, XA, XB });
  DE(xvcmpgtdpdot,  0xF0000758, Form::kXX3, xvcmpgtdp.,  { XT, XA, XB });
  DE(xvcmpgtsp,     0xF0000258, Form::kXX3, xvcmpgtsp,   { XT, XA, XB });
  DE(xvcmpgtspdot,  0xF0000658, Form::kXX3, xvcmpgtsp.,  { XT, XA, XB });
  DE(xvcpsgndp,     0xF0000780, Form::kXX3, xvcpsgndp,   { XT, XA, XB });
  DE(xvcpsgnsp,     0xF0000680, Form::kXX3, xvcpsgnsp,   { XT, XA, XB });
  DE(xvcvdpsp,      0xF0000624, Form::kXX2, xvcvdpsp,    { XT, XB });
  DE(xvcvdpsxds,    0xF0000760, Form::kXX2, xvcvdpsxds,  { XT, XB });
  DE(xvcvdpsxws,    0xF0000360, Form::kXX2, xvcvdpsxws,  { XT, XB });
  DE(xvcvdpuxds,    0xF0000720, Form::kXX2, xvcvdpuxds,  { XT, XB });
  DE(xvcvdpuxws,    0xF0000320, Form::kXX2, xvcvdpuxws,  { XT, XB });
  DE(xvcvspdp,      0xF0000724, Form::kXX2, xvcvspdp,    { XT, XB });
  DE(xvcvspsxds,    0xF0000660, Form::kXX2, xvcvspsxds,  { XT, XB });
  DE(xvcvspsxws,    0xF0000260, Form::kXX2, xvcvspsxws,  { XT, XB });
  DE(xvcvspuxds,    0xF0000620, Form::kXX2, xvcvspuxds,  { XT, XB });
  DE(xvcvspuxws,    0xF0000220, Form::kXX2, xvcvspuxws,  { XT, XB });
  DE(xvcvsxddp,     0xF00007E0, Form::kXX2, xvcvsxddp,   { XT, XB });
  DE(xvcvsxdsp,     0xF00006E0, Form::kXX2, xvcvsxdsp,   { XT, XB });
  DE(xvcvsxwdp,     0xF00003E0, Form::kXX2, xvcvsxwdp,   { XT, XB });
  DE(xvcvsxwsp,     0xF00002E0, Form::kXX2, xvcvsxwsp,   { XT, XB });
  DE(xvcvuxddp,     0xF00007A0, Form::kXX2, xvcvuxddp,   { XT, XB });
  DE(xvcvuxdsp,     0xF00006A0, Form::kXX2, xvcvuxdsp,   { XT, XB });
  DE(xvcvuxwdp,     0xF00003A0, Form::kXX2, xvcvuxwdp,   { XT, XB });
  DE(xvcvuxwsp,     0xF00002A0, Form::kXX2, xvcvuxwsp,   { XT, XB });
  DE(xvdivdp,       0xF00003C0, Form::kXX3, xvdivdp,     { XT, XA, XB });
  DE(xvdivsp,       0xF00002C0, Form::kXX3, xvdivsp,     { XT, XA, XB });
  DE(xvmaddadp,     0xF0000308, Form::kXX3, xvmaddadp,   { XT, XA, XB });
  DE(xvmaddasp,     0xF0000208, Form::kXX3, xvmaddasp,   { XT, XA, XB });
  DE(xvmaddmdp,     0xF0000348, Form::kXX3, xvmaddmdp,   { XT, XA, XB });
  DE(xvmaddmsp,     0xF0000248, Form::kXX3, xvmaddmsp,   { XT, XA, XB });
  DE(xvmaxdp,       0xF0000700, Form::kXX3, xvmaxdp,     { XT, XA, XB });
  DE(xvmaxsp,       0xF0000600, Form::kXX3, xvmaxsp,     { XT, XA, XB });
  DE(xvmindp,       0xF0000740, Form::kXX3, xvmindp,     { XT, XA, XB });
  DE(xvminsp,       0xF0000640, Form::kXX3, xvminsp,     { XT, XA, XB });
  DE(xvmsubadp,     0xF0000388, Form::kXX3, xvmsubadp,   { XT, XA, XB });
  DE(xvmsubasp,     0xF0000288, Form::kXX3, xvmsubasp,   { XT, XA, XB });
  DE(xvmsubmdp,     0xF00003C8, Form::kXX3, xvmsubmdp,   { XT, XA, XB });
  DE(xvmsubmsp,     0xF00002C8, Form::kXX3, xvmsubmsp,   { XT, XA, XB });
  DE(xvmuldp,       0xF0000380, Form::kXX3, xvmuldp,     { XT, XA, XB });
  DE(xvmulsp,       0xF0000280, Form::kXX3, xvmulsp,     { XT, XA, XB });
  DE(xvnabsdp,      0xF00007A4, Form::kXX2, xvnabsdp,    { XT, XB });
  DE(xvnabssp,      0xF00006A4, Form::kXX2, xvnabssp,    { XT, XB });
  DE(xvnegdp,       0xF00007E4, Form::kXX2, xvnegdp,     { XT, XB });
  DE(xvnegsp,       0xF00006E4, Form::kXX2, xvnegsp,     { XT, XB });
  DE(xvnmaddadp,    0xF0000708, Form::kXX3, xvnmaddadp,  { XT, XA, XB });
  DE(xvnmaddasp,    0xF0000608, Form::kXX3, xvnmaddasp,  { XT, XA, XB });
  DE(xvnmaddmdp,    0xF0000748, Form::kXX3, xvnmaddmdp,  { XT, XA, XB });
  DE(xvnmaddmsp,    0xF0000648, Form::kXX3, xvnmaddmsp,  { XT, XA, XB });
  DE(xvnmsubadp,    0xF0000788, Form::kXX3, xvnmsubadp,  { XT, XA, XB });
  DE(xvnmsubasp,    0xF0000688, Form::kXX3, xvnmsubasp,  { XT, XA, XB });
  DE(xvnmsubmdp,    0xF00007C8, Form::kXX3, xvnmsubmdp,  { XT, XA, XB });
  DE(xvnmsubmsp,    0xF00006C8, Form::kXX3, xvnmsubmsp,  { XT, XA, XB });
  DE(xvrdpi,        0xF0000324, Form::kXX2, xvrdpi,      { XT, XB });
  DE(xvrdpic,       0xF00003AC, Form::kXX2, xvrdpic,     { XT, XB });
  DE(xvrdpim,       0xF00003E4, Form::kXX2, xvrdpim,     { XT, XB });
  DE(xvrdpip,       0xF00003A4, Form::kXX2, xvrdpip,     { XT, XB });
  DE(xvrdpiz,       0xF0000364, Form::kXX2, xvrdpiz,     { XT, XB });
  DE(xvredp,        0xF0000368, Form::kXX2, xvredp,      { XT, XB });
  DE(xvresp,        0xF0000268, Form::kXX2, xvresp,      { XT, XB });
  DE(xvrspi,        0xF0000224, Form::kXX2, xvrspi,      { XT, XB });
  DE(xvrspic,       0xF00002AC, Form::kXX2, xvrspic,     { XT, XB });
  DE(xvrspim,       0xF00002E4, Form::kXX2, xvrspim,     { XT, XB });
  DE(xvrspip,       0xF00002A4, Form::kXX2, xvrspip,     { XT, XB });
  DE(xvrspiz,       0xF0000264, Form::kXX2, xvrspiz,     { XT, XB });
  DE(xvrsqrtedp,    0xF0000328, Form::kXX2, xvrsqrtedp,  { XT, XB });
  DE(xvrsqrtesp,    0xF0000228, Form::kXX2, xvrsqrtesp,  { XT, XB });
  DE(xvsqrtdp,      0xF000032C, Form::kXX2, xvsqrtdp,    { XT, XB });
  DE(xvsqrtsp,      0xF000022C, Form::kXX2, xvsqrtsp,    { XT, XB });
  DE(xvsubdp,       0xF0000340, Form::kXX3, xvsubdp,     { XT, XA, XB });
  DE(xvsubsp,       0xF0000240, Form::kXX3, xvsubsp,     { XT, XA, XB });
  DE(xvtdivdp,      0xF00003E8, Form::kXX3, xvtdivdp,    { BF, XA, XB });
  DE(xvtdivsp,      0xF00002E8, Form::kXX3, xvtdivsp,    { BF, XA, XB });
  DE(xvtsqrtdp,     0xF00003A8, Form::kXX2, xvtsqrtdp,   { BF, XB });
  DE(xvtsqrtsp,     0xF00002A8, Form::kXX2, xvtsqrtsp,   { BF, XB });
  DE(xxland,        0xF0000410, Form::kXX3, xxland,      { XT, XA, XB });
  DE(xxlandc,       0xF0000450, Form::kXX3, xxlandc,     { XT, XA, XB });
  DE(xxleqv,        0xF00005D0, Form::kXX3, xxleqv,      { XT, XA, XB });
  DE(xxlnand,       0xF0000590, Form::kXX3, xxlnand,     { XT, XA, XB });
  DE(xxlnor,        0xF0000510, Form::kXX3, xxlnor,      { XT, XA, XB });
  DE(xxlor,         0xF0000490, Form::kXX3, xxlor,       { XT, XA, XB });
  DE(xxlorc,        0xF0000550, Form::kXX3, xxlorc,      { XT, XA, XB });
  DE(xxlxor,        0xF00004D0, Form::kXX3, xxlxor,      { XT, XA, XB });
  DE(xxmrghw,       0xF0000090, Form::kXX3, xxmrghw,     { XT, XA, XB });
  DE(xxmrglw,       0xF0000190, Form::kXX3, xxmrglw,     { XT, XA, XB });
  DE(xxpermdi,      0xF0000050, Form::kXX3, xxpermdi,    { XT, XA, XB, DM });
  DE(xxsel,         0xF0000030, Form::kXX4, xxsel,       { XT, XA, XB, XC });
  DE(xxsldwi,       0xF0000010, Form::kXX3, xxsldwi,     { XT, XA, XB, SHW });
  DE(xxspltw,       0xF0000290, Form::kXX2, xxspltw,     { XT, XA, XB, UIMM });
  #undef DE
  #undef A
  #undef A_L
  #undef BA
  #undef BB
  #undef BD
  #undef BDA
  #undef BF
  #undef BFA
  #undef BFF
  #undef BH
  #undef BHRBE
  #undef BI
  #undef BO
  #undef BT
  #undef CRFD
  #undef CRB
  #undef CT
  #undef D
  #undef DCM
  #undef DGM
  #undef DM
  #undef DQ
  #undef DS
  #undef DUI
  #undef DUIS
  #undef E
  #undef EH
  #undef EVUIMM
  #undef EVUIMM_2
  #undef EVUIMM_4
  #undef EVUIMM_8
  #undef FLM
  #undef FRA
  #undef FRB
  #undef FRC
  #undef FRS
  #undef FRT
  #undef FXM
  #undef L
  #undef LEV
  #undef LI
  #undef LIA
  #undef LS
  #undef MB6
  #undef ME6
  #undef MBE
  #undef ME
  #undef MO
  #undef NB
  #undef OC
  #undef PMR
  #undef PS
  #undef R
  #undef RA
  #undef RA0
  #undef RAL
  #undef RAM
  #undef RAOPT
  #undef RAQ
  #undef RAS
  #undef RB
  #undef RMC
  #undef RS
  #undef RSO
  #undef RSQ
  #undef RT
  #undef RTO
  #undef RTQ
  #undef S
  #undef SH
  #undef SH16
  #undef SH6
  #undef SHB
  #undef SHO
  #undef SHW
  #undef SI
  #undef SIMM
  #undef SISIGNOPT
  #undef SIX
  #undef SP
  #undef SPR
  #undef SR
  #undef ST
  #undef TBR
  #undef TE
  #undef TH
  #undef TO
  #undef U
  #undef UI
  #undef UIMM
  #undef VA
  #undef VB
  #undef VC
  #undef VD
  #undef VS
  #undef W
  #undef WC
  #undef XA
  #undef XB
  #undef XC
  #undef XFL_L
  #undef XFL_L
  #undef XS
  #undef XT
  }

  DecoderInfo* GetInstruction(uint32_t opcode) {

    uint32_t index = (opcode % kDecoderSize);

    while (decoder_table[index] != nullptr &&
           decoder_table[index]->opcode() != opcode) {
      index = (index + 1) % kDecoderSize;
    }

    if (decoder_table[index] != nullptr) {
      return decoder_table[index];
    } else {
      DecoderInfo* invalid = new DecoderInfo(0, Form::kInvalid, "", { UN });
      #undef UN // Undef UN now
      return invalid;
    }
  }

  ~DecoderTable() {
    for(int i = 0; i < kDecoderSize; i++)
       if(decoder_table[i] != nullptr) {
          delete decoder_table[i];
       }
    delete[] decoder_table;
  }

private:

  void SetInstruction(DecoderInfo dinfo) {

    uint32_t index = (dinfo.opcode() % kDecoderSize);

    while (decoder_table[index] != nullptr &&
           decoder_table[index]->opcode() != dinfo.opcode()) {
        index = (index + 1) % kDecoderSize;
    }

    if (decoder_table[index] != nullptr && *decoder_table[index] != dinfo) {
        // in some architectures instructions can share the same opcode or
        // decoder info. Rhe meaning of the instruction will depends on
        // processor mode or execution mode
        decoder_table[index]->next(new DecoderInfo(dinfo));
    } else {
      delete decoder_table[index];
      decoder_table[index] = new DecoderInfo(dinfo);
    }
  }

  DecoderInfo **decoder_table;
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
static_assert(sizeof(XO_format) == sizeof(uint32_t), "XO_format_t size != 4");

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
static_assert(sizeof(X_format) == sizeof(uint32_t), "X_format_t size != 4");

typedef union D_format {
  struct {
    uint32_t D:16;
    uint32_t RA:5;
    uint32_t RT:5;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} D_form_t;
static_assert(sizeof(D_format) == sizeof(uint32_t), "D_format_t size != 4");

typedef union I_format {
  struct {
    uint32_t LK:1;
    uint32_t AA:1;
    uint32_t LI:24;
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} I_form_t;
static_assert(sizeof(I_format) == sizeof(uint32_t), "I_format_t size != 4");

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
static_assert(sizeof(B_format) == sizeof(uint32_t), "B_format_t size != 4");

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
static_assert(sizeof(SC_format) == sizeof(uint32_t), "SC_format_t size != 4");

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
static_assert(sizeof(DS_format) == sizeof(uint32_t), "DS_format_t size != 4");

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
static_assert(sizeof(DQ_format) == sizeof(uint32_t), "DQ_format_t size != 4");

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
static_assert(sizeof(XFX_format) == sizeof(uint32_t), "XFX_format_t size != 4");

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
static_assert(sizeof(XFL_format) == sizeof(uint32_t), "XFL_format_t size != 4");

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
static_assert(sizeof(XX1_format) == sizeof(uint32_t), "XX1_format_t size != 4");

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
static_assert(sizeof(XX2_format) == sizeof(uint32_t), "XX2_format_t size != 4");

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
static_assert(sizeof(XX3_format) == sizeof(uint32_t), "XX3_format_t size != 4");

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
static_assert(sizeof(XX4_format) == sizeof(uint32_t), "XX4_format_t size != 4");

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
static_assert(sizeof(XS_format) == sizeof(uint32_t), "XS_format_t size != 4");

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
static_assert(sizeof(A_format) == sizeof(uint32_t), "A_format_t size != 4");

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
static_assert(sizeof(M_format) == sizeof(uint32_t), "M_format_t size != 4");

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
static_assert(sizeof(MD_format) == sizeof(uint32_t), "MD_format_t size != 4");

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
static_assert(sizeof(MDS_format) == sizeof(uint32_t), "MDS_format_t size != 4");

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
static_assert(sizeof(VA_format) == sizeof(uint32_t), "VA_format_t size != 4");

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
static_assert(sizeof(VC_format) == sizeof(uint32_t), "VC_format_t size != 4");

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
static_assert(sizeof(VX_format) == sizeof(uint32_t), "VX_format_t size != 4");

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
static_assert(sizeof(EVX_format) == sizeof(uint32_t), "EVX_format_t size != 4");

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
static_assert(sizeof(EVS_format) == sizeof(uint32_t), "EVS_format_t size != 4");

typedef union Z22_format {
  struct {
    //TODO:
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} Z22_form_t;
static_assert(sizeof(Z22_format) == sizeof(uint32_t), "Z22_format_t size != 4");

typedef union Z23_format {
  struct {
    //TODO:
    uint32_t OP:6;
  };
  PPC64Instr instruction;
} Z23_form_t;
static_assert(sizeof(Z23_format) == sizeof(uint32_t), "Z23_format_t size != 4");

} // namespace ppc64_asm

#endif
