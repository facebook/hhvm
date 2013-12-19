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

#include <iomanip>
#include <list>
#include <cstdio>
#include <limits>

#include "hphp/runtime/vm/verifier/check.h"
#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/runtime/vm/verifier/pretty.h"

namespace HPHP {
namespace Verifier {

/**
 * State for one entry on the FPI stack which corresponds to one call-site
 * in progress.
 */
struct FpiState {
  Offset fpush;   // offset of fpush (can get num_params from this)
  int stkmin;     // stklen before FPush
  int next;       // next expected param number
  bool operator==(const FpiState& s) const {
    return fpush == s.fpush && stkmin == s.stkmin && next == s.next;
  }
  bool operator!=(const FpiState& s) const {
    return !(*this == s);
  }
};

/**
 * Facts about a Func's current frame at various program points
 */
struct State {
  FlavorDesc* stk; // Evaluation stack.
  FpiState* fpi;    // FPI stack.
  bool* iters;      // defined/not-defined state of each iter var.
  int stklen;       // length of evaluation stack.
  int fpilen;       // length of FPI stack.
};

/**
 * Facts about a specific block.
 */
struct BlockInfo {
  State state_in;  // state at the start of the block
};

class FuncChecker {
 public:
  FuncChecker(const Func* func, bool verbose);
  bool checkOffsets();
  bool checkFlow();

 private:
  bool checkEdge(Block* b, const State& cur, Block* t);
  bool checkSuccEdges(Block* b, State* cur);
  bool checkOffset(const char* name, Offset o, const char* regionName,
                   Offset base, Offset past, bool check_instrs = true);
  bool checkRegion(const char* name, Offset b, Offset p,
                   const char* regionName, Offset base, Offset past,
                   bool check_instrs = true);
  bool checkSection(bool main, const char* name, Offset base, Offset past);
  bool checkImmediates(const char* name, const Op* instr);
  bool checkInputs(State* cur, PC, Block* b);
  bool checkOutputs(State* cur, PC, Block* b);
  bool checkSig(PC pc, int len, const FlavorDesc* args, const FlavorDesc* sig);
  bool checkEHStack(const EHEnt&, Block* b);
  bool checkTerminal(State* cur, PC pc);
  bool checkFpi(State* cur, PC pc, Block* b);
  bool checkIter(State* cur, PC pc);
  bool checkLocal(PC pc, int val);
  bool checkString(PC pc, Id id);
  void reportStkUnderflow(Block*, const State& cur, PC);
  void reportStkOverflow(Block*, const State& cur, PC);
  void reportStkMismatch(Block* b, Block* target, const State& cur);
  void reportEscapeEdge(Block* b, Block* s);
  std::string stateToString(const State& cur);
  std::string sigToString(int len, const FlavorDesc* sig);
  std::string stkToString(int len, const FlavorDesc* args);
  std::string fpiToString(const FpiState&);
  std::string iterToString(const State& cur);
  void copyState(State* to, const State* from);
  void initState(State* s);
  const FlavorDesc* sig(PC pc);
  const FlavorDesc* vectorSig(PC pc, FlavorDesc rhs_flavor);
  Offset offset(PC pc) const { return pc - unit()->entry(); }
  PC at(Offset off) const { return unit()->at(off); }
  int maxStack() const { return m_func->maxStackCells(); }
  int maxFpi() const { return m_func->fpitab().size(); }
  int numIters() const { return m_func->numIterators(); }
  int numLocals() const { return m_func->numLocals(); }
  int numParams() const { return m_func->numParams(); }
  const Unit* unit() const { return m_func->unit(); }

 private:
  template<class... Args>
  void error(const char* fmt, Args&&... args) {
    verify_error(unit(), m_func, fmt, std::forward<Args>(args)...);
  }

 private:
  Arena m_arena;
  BlockInfo* m_info; // one per block
  const Func* const m_func;
  Graph* m_graph;
  Bits m_instrs;
  bool m_verbose;
  FlavorDesc* m_tmp_sig;
};

bool checkFunc(const Func* func, bool verbose) {
  if (verbose) {
    func->prettyPrint(std::cout);
    if (func->cls() || !func->preClass()) {
      printf("  FuncId %d\n", func->getFuncId());
    }
    printFPI(func);
  }
  FuncChecker v(func, verbose);
  return v.checkOffsets() &&
         v.checkFlow();
}

FuncChecker::FuncChecker(const Func* f, bool verbose)
: m_func(f)
, m_graph(0)
, m_instrs(m_arena, f->past() - f->base() + 1)
, m_verbose(verbose) {
}

// Needs to be a sorted map so we can divide funcs into contiguous sections.
typedef std::map<Offset,Offset> SectionMap;

/**
 * Return the start offset of the nearest enclosing section.  Caller must
 * ensure that off is at least within the entire func's bytecode region.
 */
Offset findSection(SectionMap& sections, Offset off) {
  assert(!sections.empty());
  SectionMap::iterator i = sections.upper_bound(off);
  --i;
  return i->first;
}

/**
 * Make sure all offsets are in-bounds.  Offset 0 is unit->m_bc,
 * for all functions.  Jump instructions use an Offset relative to
 * the start of the jump instruction.
 */
bool FuncChecker::checkOffsets() {
  bool ok = true;
  assert(unit()->bclen() >= 0);
  PC bc = unit()->entry();
  Offset base = m_func->base();
  Offset past = m_func->past();
  checkRegion("func", base, past, "unit", 0, unit()->bclen(), false);
  // find instruction boundaries and make sure no branches escape
  SectionMap sections;
  for (Range<FixedVector<EHEnt> > i(m_func->ehtab()); !i.empty(); ) {
    const EHEnt& eh = i.popFront();
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault funclet", eh.m_fault, "func bytecode", base,
                        past, false);
      sections[eh.m_fault] = 0;
    }
  }
  Offset funclets = !sections.empty() ? sections.begin()->first : past;
  sections[base] = funclets; // primary body
  // Get instruction boundaries and check branches within primary body
  // and each faultlet.
  for (Range<SectionMap> i(sections); !i.empty(); ) {
    Offset section_base = i.popFront().first;
    Offset section_past = i.empty() ? past : i.front().first;
    sections[section_base] = section_past;
    ok &= checkSection(section_base == base,
                       section_base == base ? "primary body" : "funclet body",
                       section_base, section_past);
  }
  // DV entry points must be in the primary function body
  for (Range<FixedVector<Func::ParamInfo> > p(m_func->params()); !p.empty(); ) {
    const Func::ParamInfo& param = p.popFront();
    if (param.hasDefaultValue()) {
      ok &= checkOffset("dv-entry", param.funcletOff(), "func body", base,
                        funclets);
    }
  }
  // Every FPI region must be contained within one section, either the
  // primary body or one fault funclet
  for (Range<FixedVector<FPIEnt> > i(m_func->fpitab()); !i.empty(); ) {
    const FPIEnt& fpi = i.popFront();
    Offset fpi_base = fpiBase(fpi, bc);
    Offset fpi_past = fpiPast(fpi, bc);
    if (checkRegion("fpi", fpi_base, fpi_past, "func", base, past)) {
      // FPI is within whole func, but we also need to check within the section
      Offset section_base = findSection(sections, fpi_base);
      Offset section_past = sections[section_base];
      ok &= checkRegion("fpi", fpi_base, fpi_past,
                        section_base == base ?  "func body" : "funclet",
                        section_base, section_past);
    } else {
      ok = false;
    }
  }
  // check EH regions and targets
  for (Range<FixedVector<EHEnt> > i(m_func->ehtab()); !i.empty(); ) {
    const EHEnt& eh = i.popFront();
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault", eh.m_fault, "funclets", funclets, past);
    }
  }
  return ok;
}

/**
 * Scan instructions in the given section to find valid instruction
 * boundaries, and check that branches a) land on valid boundaries,
 * b) do not escape the section.
 */
bool FuncChecker::checkSection(bool is_main, const char* name, Offset base,
                               Offset past) {
  bool ok = true;
  typedef std::list<PC> BranchList;
  BranchList branches;
  PC bc = unit()->entry();
  // Find instruction boundaries and branch instructions.
  for (InstrRange i(at(base), at(past)); !i.empty();) {
    if (!checkImmediates(name, (Op*)i.front())) return false;
    PC pc = i.popFront();
    m_instrs.set(offset(pc) - m_func->base());
    if (isSwitch(toOp(*pc)) ||
        instrJumpTarget((Op*)bc, offset(pc)) != InvalidAbsoluteOffset) {
      if (toOp(*pc) == OpSwitch && getImm((Op*)pc, 2).u_IVA != 0) {
        int64_t switchBase = getImm((Op*)pc, 1).u_I64A;
        int32_t len = getImmVector((Op*)pc).size();
        if (len <= 2) {
          error("Bounded switch must have a vector of length > 2 [%d:%d]\n",
                base, past);
        }
        if (switchBase > std::numeric_limits<int64_t>::max() - len + 2) {
          error("Overflow in Switch bounds [%d:%d]\n", base, past);
        }
      } else if (toOp(*pc) == OpSSwitch) {
        foreachSwitchString((Opcode*)pc, [&](Id& id) {
          ok &= checkString(pc, id);
        });
      }
      branches.push_back(pc);
    }
    if (i.empty()) {
      if (offset(pc + instrLen((Op*)pc)) != past) {
        error("Last instruction in %s at %d overflows [%d:%d]\n",
               name, offset(pc), base, past);
        ok = false;
      }
      if (!isTF(pc)) {
        error("Last instruction in %s is not teriminal %d:%s\n",
               name, offset(pc), instrToString((Op*)pc, unit()).c_str());
        ok = false;
      } else {
        if (isRet(pc) && !is_main) {
          error("Ret* may not appear in %s\n", name);
          ok = false;
        } else if (Op(*pc) == OpUnwind && is_main) {
          error("Unwind may not appear in %s\n", name);
          ok = false;
        }
      }
    }
  }
  // Check each branch target lands on a valid instruction boundary
  // within this region.
  for (Range<BranchList> i(branches); !i.empty();) {
    PC branch = i.popFront();
    if (isSwitch(*branch)) {
      foreachSwitchTarget((Op*)branch, [&](Offset& o) {
        // TODO(#2464197): dce breaks switch for verify
        if (offset(branch + o) != -1) {
          ok &= checkOffset("switch target", offset(branch + o),
                            name, base, past);
        }
      });
    } else {
      Offset target = instrJumpTarget((Op*)bc, offset(branch));
      ok &= checkOffset("branch target", target, name, base, past);
    }
  }
  return ok;
}

Id decodeId(PC* ppc) {
  Id id = *(Id*)*ppc;
  *ppc += sizeof(Id);
  return id;
}

Offset decodeOffset(PC* ppc) {
  Offset offset = *(Offset*)*ppc;
  *ppc += sizeof(Offset);
  return offset;
}

/**
 * Range over the members of an immediate member vector.
 */
class ImmVecRange {
 public:
  explicit ImmVecRange(const Op* instr) : v(getImmVector(instr)),
      vecp(v.vec() + 1), // skip location code
      loc(v.locationCode()),
      loc_local(numLocationCodeImms(loc) ? decodeVariableSizeImm(&vecp) : -1) {
  }

  bool empty() const {
    return vecp >= v.vec() + v.size();
  }

  MemberCode frontMember() const {
    assert(!empty());
    return MemberCode(*vecp);
  }

  int frontLocal() const {
    PC p = vecp + 1;
    const MemberCode mc = frontMember();
    return (mc == MEL || mc == MPL) ? decodeMemberCodeImm(&p, mc) : -1;
  }

  Id frontString() const {
    PC p = vecp + 1;
    const MemberCode mc = frontMember();
    return (mc == MET || mc == MPT) ? Id(decodeMemberCodeImm(&p, mc)) : -1;
  }

  void popFront() {
    assert(!empty());
    vecp++;
    const MemberCode mc = MemberCode(vecp[-1]);
    if (memberCodeHasImm(mc)) {
      decodeMemberCodeImm(&vecp, mc);
    }
  }

  int size() const {
    return v.size();
  }

 private:
  ImmVector v;
  PC vecp;
 public:
  const LocationCode loc;
  const int loc_local;    // local variable id or -1
};

bool FuncChecker::checkLocal(PC pc, int k) {
  if (k < 0 || k >= numLocals()) {
    error("invalid local variable id %d at Offset %d\n",
           k, offset(pc));
    return false;
  }
  return true;
}

bool FuncChecker::checkString(PC pc, Id id) {
  return unit()->checkStringId(id);
}

/**
 * Check instruction and its immediates, return false if we can't continue
 * because we don't know the length of this instruction
 */
bool FuncChecker::checkImmediates(const char* name, const Op* instr) {
  if (!isValidOpcode(*instr)) {
    error("Invalid opcode %d in section %s at offset %d\n",
           uint8_t(*instr), name, offset(PC(instr)));
    return false;
  }
  bool ok = true;
  PC pc = (PC)instr + 1;
  for (int i = 0, n = numImmediates(*instr); i < n;
       pc += immSize(instr, i), i++) {
    switch (immType(*instr, i)) {
    default: assert(false && "Unexpected immType");
    case MA: { // member vector
      ImmVecRange vr(instr);
      if (vr.size() < 2) {
        // vector must at least have a LocationCode and 1+ MemberCodes
        error("invalid vector size %d at %d\n",
               vr.size(), offset((PC)instr));
        return false;
      } else {
        if (vr.loc < 0 || vr.loc >= NumLocationCodes) {
          error("invalid location code %d in vector at %d\n",
                 (int)vr.loc, offset((PC)instr));
          ok = false;
        }
        if (vr.loc_local != -1) checkLocal(pc, vr.loc_local);
        for (; !vr.empty(); vr.popFront()) {
          MemberCode member = vr.frontMember();
          if (member < 0 || member >= NumMemberCodes) {
            error("invalid member code %d in vector at %d\n",
                   (int) member, offset((PC)instr));
            ok = false;
          }
          if (vr.frontLocal() != -1) {
            ok &= checkLocal(pc, vr.frontLocal());
          } else if (vr.frontString() != -1) {
            ok &= checkString(pc, vr.frontString());
          }
        }
      }
      break;
    }
    case LA: { // local argument (id of local variable)
      PC ha_pc = pc;
      int32_t k = decodeVariableSizeImm(&ha_pc);
      ok &= checkLocal(pc, k);
      break;
    }
    case IA: { // iterator argument (id of iterator variable)
      PC ia_pc = pc;
      int32_t k = decodeVariableSizeImm(&ia_pc);
      if (k >= numIters()) {
        error("invalid iterator variable id %d at %d\n",
               k, offset((PC)instr));
        ok = false;
      }
      break;
    }
    case IVA: { // variable size int
      PC iva_pc = pc;
      int32_t k = decodeVariableSizeImm(&iva_pc);
      switch (*instr) {
      case OpStaticLoc:
      case OpStaticLocInit:
        ok &= checkLocal(pc, k);
        break;
      case OpVerifyParamType:
        if (k >= numParams()) {
          error("invalid parameter id %d at %d\n",
                 k, offset((PC)instr));
          ok = false;
        }
      default:
        break;
      }
      break;
    }
    case ILA:
    case BLA:
    case SLA: { // vec of offsets for Switch/SSwitch/IterBreak
      int len = *(int*)pc;
      if (len < 1) {
        error("invalid length of immediate vector %d at Offset %d\n",
               len, offset(pc));
        return false;
      }
      break;
    }
    case I64A: // 64-bit int
    case DA: // double:
      // nothing to check
      break;
    case SA: { // litstr id
      PC sa_pc = pc;
      Id id = decodeId(&sa_pc);
      ok &= checkString(pc, id);
      break;
    }
    case VSA: { // vector of litstr ids
      auto len = *(uint32_t*)pc;
      if (len < 1 || len > HphpArray::MaxMakeSize) {
        error("invalid length of immedate VSA vector %d at offset %d\n",
              len, offset(pc));
        return false;
      }
      for (int i = 0; i < len; i++) {
        PC sa_pc = pc + (i + 1) * sizeof(int);
        Id id = decodeId(&sa_pc);
        ok &= checkString(pc, id);
      }
      break;
    }
    case AA: { // static array id
      PC aa_pc = pc;
      Id id = decodeId(&aa_pc);
      if (id < 0 || id >= (Id)unit()->numArrays()) {
        error("invalid array id %d\n", id);
        ok = false;
      }
      break;
    }
    case BA: // bytecode address
      // we check branch offsets in checkSection(). ignore here.
      assert(instrJumpTarget((Op*)unit()->entry(), offset((PC)instr)) !=
             InvalidAbsoluteOffset);
      break;
    case OA: { // secondary opcode
      assert(int(*pc) >= 0); // guaranteed because PC is unsigned char*
      int op = int(*pc);
      switch (*instr) {
      default: assert(false && "Unexpected opcode with immType OA");
      case Op::AssertTL: case Op::AssertTStk:
      case Op::PredictTL: case Op::PredictTStk:
#define ASSERTT_OP(x)  if (op == static_cast<uint8_t>(AssertTOp::x)) break;
        ASSERTT_OPS
#undef ASSERTT_OP
        error("invalid operation for AssertT*: %d\n", op);
        ok = false;
        break;
      case Op::IsTypeC: case Op::IsTypeL:
#define ISTYPE_OP(x)  if (op == static_cast<uint8_t>(IsTypeOp::x)) break;
        ISTYPE_OPS
#undef ISTYPE_OP
        error("invalid operation for IsType*: %d\n", op);
        ok = false;
        break;
      case OpIncDecL: case OpIncDecN: case OpIncDecG: case OpIncDecS:
      case OpIncDecM:
#define INCDEC_OP(x)   if (op == static_cast<uint8_t>(IncDecOp::x)) break;
        INCDEC_OPS
#undef INCDEC_OP
        error("invalid operation for IncDec*: %d\n", op);
        ok = false;
        break;
      case OpSetOpL: case OpSetOpN: case OpSetOpG: case OpSetOpS:
      case OpSetOpM:
#define SETOP_OP(x, y) if (op == static_cast<uint8_t>(SetOpOp::x)) break;
        SETOP_OPS
#undef SETOP_OP
        error("invalid operation for SetOp*: %d\n", op);
        ok = false;
        break;
      case OpBareThis:
        if (op > 1) ok = false;
        break;
      case OpFatal:
#define FATAL_OP(x)   if (op == static_cast<uint8_t>(FatalOp::x)) break;
        FATAL_OPS
#undef FATAL_OP
        error("invalid error kind for Fatal: %d\n", op);
        ok = false;
        break;
      }
      break;
    }}
  }
  return ok;
}

static char stkflav(FlavorDesc f) {
  static const char flavs[] = { 'N', 'C', 'V', 'A', 'R', 'F' };
  return f > NOV && f <= FV ? flavs[f] : '?';
}

bool FuncChecker::checkSig(PC pc, int len, const FlavorDesc* args,
                           const FlavorDesc* sig) {
  for (int i = 0; i < len; ++i) {
    if (args[i] != (FlavorDesc)sig[i] &&
        !((FlavorDesc)sig[i] == CVV && (args[i] == CV || args[i] == VV)) &&
        !((FlavorDesc)sig[i] == CVUV && (args[i] == CV || args[i] == VV ||
                                         args[i] == UV))) {
      error("flavor mismatch at %d, got %s expected %s\n",
             offset(pc), stkToString(len, args).c_str(),
             sigToString(len, sig).c_str());
      return false;
    }
  }
  return true;
}

/**
 * format of vector in memory:
 *   int32 size;
 *   int32 numstk;
 *   uint8 locationCode
 *   [IVA imm for location]
 *   [uint8 MemberCode; [IVA imm for member]]
 */
const FlavorDesc* FuncChecker::vectorSig(PC pc, FlavorDesc rhs_flavor) {
  ImmVecRange vr((Op*)pc);
  int n = 0;
  if (vr.loc_local != -1 ||
      vr.loc == LH ||
      vr.loc == LGL ||
      vr.loc == LNL ||
      vr.loc == LSL) {
    /* nothing on stack for loc */
  } else if (vr.loc == LR) {
    m_tmp_sig[n++] = RV;
  } else {
    m_tmp_sig[n++] = CV;
  }
  for (; !vr.empty(); vr.popFront()) {
    MemberCode member = vr.frontMember();
    if (member == MEC || member == MPC) {
      m_tmp_sig[n++] = CV;
    }
  }
  if (vr.loc == LSC || vr.loc == LSL) m_tmp_sig[n++] = AV; // extra classref
  if (rhs_flavor != NOV) m_tmp_sig[n++] = rhs_flavor; // extra rhs value for Set
  assert(n == instrNumPops((Op*)pc));
  return m_tmp_sig;
}

const FlavorDesc* FuncChecker::sig(PC pc) {
  static const FlavorDesc inputSigs[][3] = {
  #define NOV { },
  #define FMANY { },
  #define CVMANY { },
  #define CVUMANY { },
  #define CMANY { },
  #define SMANY { },
  #define ONE(a) { a },
  #define TWO(a,b) { b, a },
  #define THREE(a,b,c) { c, b, a },
  #define FOUR(a,b,c,d) { d, c, b, a },
  #define MMANY { },
  #define C_MMANY { },
  #define V_MMANY { },
  #define R_MMANY { },
  #define O(name, imm, pop, push, flags) pop
    OPCODES
  #undef O
  #undef C_MMANY
  #undef V_MMANY
  #undef R_MMANY
  #undef MMANY
  #undef FMANY
  #undef CVMANY
  #undef CVUMANY
  #undef CMANY
  #undef SMANY
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef NOV
  };
  switch (toOp(*pc)) {
  case OpCGetM:     // ONE(LA),      MMANY, ONE(CV)
  case OpVGetM:     // ONE(LA),      MMANY, ONE(VV)
  case OpIssetM:    // ONE(LA),      MMANY, ONE(CV)
  case OpEmptyM:    // ONE(LA),      MMANY, ONE(CV)
  case OpUnsetM:    // ONE(LA),      MMANY, NOV
  case OpFPassM:    // TWO(IVA,LA),  MMANY, ONE(FV)
  case OpIncDecM:   // TWO(OA,LA),   MMANY, ONE(CV)
    return vectorSig(pc, NOV);
  case OpBindM:     // ONE(LA),    V_MMANY, ONE(VV)
    return vectorSig(pc, VV);
  case OpSetM:      // ONE(LA),    C_MMANY, ONE(CV)
  case OpSetOpM:    // TWO(OA,LA), C_MMANY, ONE(CV)
    return vectorSig(pc, CV);
  case OpSetWithRefLM://TWO(MA, HA), MMANY, NOV
    return vectorSig(pc, NOV);
  case OpSetWithRefRM://ONE(MA),   R_MMANY, NOV
    return vectorSig(pc, RV);
  case OpFCall:     // ONE(IVA),     FMANY,   ONE(RV)
  case OpFCallArray:// NA,           ONE(FV), ONE(RV)
    for (int i = 0, n = instrNumPops((Op*)pc); i < n; ++i) {
      m_tmp_sig[i] = FV;
    }
    return m_tmp_sig;
  case OpFCallBuiltin: //TWO(IVA, SA), CVUMANY,  ONE(RV)
    for (int i = 0, n = instrNumPops((Op*)pc); i < n; ++i) {
      m_tmp_sig[i] = CVUV;
    }
    return m_tmp_sig;
  case OpCreateCl:  // TWO(IVA,SA),  CVMANY,   ONE(CV)
    for (int i = 0, n = instrNumPops((Op*)pc); i < n; ++i) {
      m_tmp_sig[i] = CVV;
    }
    return m_tmp_sig;
  case OpNewPackedArray:  // ONE(IVA),     CMANY,   ONE(CV)
  case OpNewStructArray:  // ONE(VSA),     SMANY,   ONE(CV)
    for (int i = 0, n = instrNumPops((Op*)pc); i < n; ++i) {
      m_tmp_sig[i] = CV;
    }
    return m_tmp_sig;
  default:
    return &inputSigs[uint8_t(toOp(*pc))][0];
  }
}

bool FuncChecker::checkInputs(State* cur, PC pc, Block* b) {
  StackTransInfo info = instrStackTransInfo((Op*)pc);
  int min = cur->fpilen > 0 ? cur->fpi[cur->fpilen - 1].stkmin : 0;
  if (info.numPops > 0 && cur->stklen - info.numPops < min) {
    reportStkUnderflow(b, *cur, pc);
    cur->stklen = 0;
    return false;
  }
  cur->stklen -= info.numPops;
  return checkSig(pc, info.numPops, &cur->stk[cur->stklen], sig(pc));
}

bool FuncChecker::checkTerminal(State* cur, PC pc) {
  if (isRet(pc) || toOp(*pc) == OpUnwind) {
    if (cur->stklen != 0) {
      error("stack depth must equal 0 after Ret* and Unwind; got %d\n",
             cur->stklen);
      return false;
    }
  }
  return true;
}

bool FuncChecker::checkFpi(State* cur, PC pc, Block* b) {
  if (cur->fpilen <= 0) {
    error("%s", "cannot access empty FPI stack\n");
    return false;
  }
  bool ok = true;
  FpiState& fpi = cur->fpi[cur->fpilen - 1];
  if (isFCallStar(toOp(*pc))) {
    --cur->fpilen;
    int call_params = toOp(*pc) == OpFCall ? getImmIva(pc) : 1;
    int push_params = getImmIva(at(fpi.fpush));
    if (call_params != push_params) {
      error("FCall* param_count (%d) doesn't match FPush* (%d)\n",
             call_params, push_params);
      ok = false;
    }
    if (fpi.next != push_params) {
      error("wrong # of params were passed; got %d expected %d\n",
             fpi.next, push_params);
      ok = false;
    }
    if (cur->stklen != fpi.stkmin) {
      error("%s", "FCall didn't consume the proper param count\n");
      ok = false;
    }
  } else {
    // FPass*
    int param_id = getImmIva(pc);
    int push_params = getImmIva(at(fpi.fpush));
    if (param_id >= push_params) {
      error("param_id %d out of range [0:%d)\n", param_id,
             push_params);
      return false;
    }
    if (param_id != fpi.next) {
      error("FPass* out of order; got id %d expected %d\n",
             param_id, fpi.next);
      ok = false;
    }
    // we have already popped FPush's input, but not pushed the output,
    // so this check doesn't count the F result of this FPush, but does
    // count the previous FPush*s.
    if (cur->stklen != fpi.stkmin + param_id) {
      error("Stack depth incorrect after FPush; got %d expected %d\n",
             cur->stklen, fpi.stkmin + param_id);
      ok = false;
    }
    fpi.next++;
  }
  return ok;
}

bool FuncChecker::checkIter(State* cur, PC const pc) {
  assert(isIter(pc));
  int id = getImmIva(pc);
  bool ok = true;
  auto op = toOp(*pc);
  if (op == OpIterInit || op == OpIterInitK ||
      op == OpWIterInit || op == OpWIterInitK ||
      op == OpMIterInit || op == OpMIterInitK ||
      op == OpDecodeCufIter) {
    if (cur->iters[id]) {
      error(
        "IterInit* or MIterInit* <%d> trying to double-initialize\n", id);
      ok = false;
    }
  } else {
    if (!cur->iters[id]) {
      // TODO(#2608280): we produce incorrect bytecode for iterators still
      //error("Cannot access un-initialized iter %d\n", id);
      //ok = false;
    }
    if (op == OpIterFree ||
        op == OpMIterFree ||
        op == OpCIterFree) {
      cur->iters[id] = false;
    }
  }
  return ok;
}

bool FuncChecker::checkOutputs(State* cur, PC pc, Block* b) {
  static const FlavorDesc outputSigs[][3] = {
  #define NOV { },
  #define FMANY { },
  #define CMANY { },
  #define SMANY { },
  #define ONE(a) { a },
  #define TWO(a,b) { a, b },
  #define THREE(a,b,c) { a, b, c },
  #define FOUR(a,b,c,d) { a, b, c, d },
  #define INS_1(a) { a },
  #define INS_2(a) { a },
  #define MMANY() { },
  #define C_MMANY() { },
  #define V_MMANY() { },
  #define R_MMANY() { },
  #define O(name, imm, pop, push, flags) push
    OPCODES
  #undef O
  #undef C_MMANY
  #undef V_MMANY
  #undef R_MMANY
  #undef MMANY
  #undef FMANY
  #undef CMANY
  #undef SMANY
  #undef INS_1
  #undef INS_2
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef NOV
  };
  bool ok = true;
  StackTransInfo info = instrStackTransInfo((Op*)pc);
  if (info.kind == StackTransInfo::Kind::InsertMid) {
    int index = cur->stklen - info.pos - 1;
    if (index < 0) {
      reportStkUnderflow(b, *cur, pc);
      return false;
    }
    memmove(&cur->stk[index + 1], &cur->stk[index],
           (info.pos + 1) * sizeof(*cur->stk));
    cur->stk[index] = outputSigs[uint8_t(toOp(*pc))][0];
    cur->stklen++;
  } else {
    int pushes = info.numPushes;
    if (cur->stklen + pushes > maxStack()) reportStkOverflow(b, *cur, pc);
    FlavorDesc *outs = &cur->stk[cur->stklen];
    cur->stklen += pushes;
    for (int i = 0; i < pushes; ++i)
      outs[i] = outputSigs[uint8_t(toOp(*pc))][i];
    if (isFPush(toOp(*pc))) {
      if (cur->fpilen >= maxFpi()) {
        error("%s", "more FPush* instructions than FPI regions\n");
        return false;
      }
      FpiState& fpi = cur->fpi[cur->fpilen];
      cur->fpilen++;
      fpi.fpush = offset(pc);
      fpi.next = 0;
      fpi.stkmin = cur->stklen;
    }
  }
  return ok;
}

std::string FuncChecker::stkToString(int len, const FlavorDesc* args) {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(args[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::sigToString(int len, const FlavorDesc* sig) {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(sig[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::iterToString(const State& cur) {
  int n = numIters();
  if (!n) return "";
  std::stringstream out;
  out << '[';
  for (int i = 0; i < n; ++i) {
    out << (cur.iters[i] ? '1' : '0');
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::stateToString(const State& cur) {
  return iterToString(cur) + stkToString(cur.stklen, cur.stk);
}

std::string FuncChecker::fpiToString(const FpiState& fpi) {
  std::stringstream out;
  out << '(' << fpi.fpush << ':' << fpi.stkmin << ',' << fpi.next << ')';
  return out.str();
}

void FuncChecker::initState(State* s) {
  s->stk = new (m_arena) FlavorDesc[maxStack()];
  s->fpi = new (m_arena) FpiState[maxFpi()];
  s->iters = new (m_arena) bool[numIters()];
  for (int i = 0, n = numIters(); i < n; ++i) s->iters[i] = false;
  s->stklen = 0;
  s->fpilen = 0;
}

void FuncChecker::copyState(State* to, const State* from) {
  assert(from->stk);
  if (!to->stk) initState(to);
  memcpy(to->stk, from->stk, from->stklen * sizeof(*to->stk));
  memcpy(to->fpi, from->fpi, from->fpilen * sizeof(*to->fpi));
  memcpy(to->iters, from->iters, numIters() * sizeof(*to->iters));
  to->stklen = from->stklen;
  to->fpilen = from->fpilen;
}

/**
 * Verify stack depth along all control flow paths
 */
bool FuncChecker::checkFlow() {
  bool ok = true;
  GraphBuilder builder(m_arena, m_func);
  m_graph = builder.build();
  m_info = new (m_arena) BlockInfo[m_graph->block_count];
  memset(m_info, 0, sizeof(BlockInfo) * m_graph->block_count);
  m_tmp_sig = new (m_arena) FlavorDesc[maxStack()];
  sortRpo(m_graph);
  State cur;
  initState(&cur);
  // initialize state at all entry points
  for (BlockPtrRange i = entryBlocks(m_graph); !i.empty();) {
    ok &= checkEdge(0, cur, i.popFront());
  }
  for (Block* b = m_graph->first_rpo; b; b = b->next_rpo) {
    copyState(&cur, &m_info[b->id].state_in);
    if (m_verbose) {
      std::cout << blockToString(b, m_graph, unit()) << std::endl;
    }
    for (InstrRange i = blockInstrs(b); !i.empty(); ) {
      PC pc = i.popFront();
      if (m_verbose) {
        std::cout << "  " << std::setw(5) << offset(pc) << ":" <<
                     stateToString(cur) << " " <<
                     instrToString((Op*)pc, unit()) << std::endl;
      }
      ok &= checkInputs(&cur, pc, b);
      if (isTF(pc)) ok &= checkTerminal(&cur, pc);
      if (isFF(pc)) ok &= checkFpi(&cur, pc, b);
      if (isIter(pc)) ok &= checkIter(&cur, pc);
      ok &= checkOutputs(&cur, pc, b);
    }
    ok &= checkSuccEdges(b, &cur);
  }
  // Make sure eval stack is correct at start of each try region
  for (Range<FixedVector<EHEnt> > i(m_func->ehtab()); !i.empty(); ) {
    const EHEnt& handler = i.popFront();
    if (handler.m_type == EHEnt::Type::Catch) {
      ok &= checkEHStack(handler, builder.at(handler.m_base));
    }
  }
  return ok;
}

bool FuncChecker::checkSuccEdges(Block* b, State* cur) {
  bool ok = true;
  // Reachable catch blocks and fault funclets have an empty stack.
  if (m_graph->exn_cap > 0) {
    int save_stklen = cur->stklen;
    int save_fpilen = cur->fpilen;
    cur->stklen = 0;
    cur->fpilen = 0;
    for (BlockPtrRange i = exnBlocks(m_graph, b); !i.empty(); ) {
      ok &= checkEdge(b, *cur, i.popFront());
    }
    cur->stklen = save_stklen;
    cur->fpilen = save_fpilen;
  }
  if (isIter(b->last) && numSuccBlocks(b) == 2) {
    // IterInit* and IterNext*, Both implicitly free their iterator variable
    // on the loop-exit path.  Compute the iterator state on the "taken" path;
    // the fall-through path has the opposite state.
    int id = getImmIva(b->last);
    bool taken_state =
      (Op(*b->last) == OpIterNext || Op(*b->last) == OpIterNextK ||
       Op(*b->last) == OpMIterNext || Op(*b->last) == OpMIterNextK ||
       Op(*b->last) == OpWIterNext || Op(*b->last) == OpWIterNextK);
    bool save = cur->iters[id];
    cur->iters[id] = taken_state;
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) <<
        " -> B" << b->succs[1]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[1]);
    cur->iters[id] = !taken_state;
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) <<
                   " -> B" << b->succs[0]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[0]);
    cur->iters[id] = save;
  } else {
    // Other branch instructions send the same state to all successors.
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) << std::endl;
    }
    for (BlockPtrRange i = succBlocks(b); !i.empty(); ) {
      ok &= checkEdge(b, *cur, i.popFront());
    }
  }
  return ok;
}

bool FuncChecker::checkEHStack(const EHEnt& handler, Block* b) {
  const State& state = m_info[b->id].state_in;
  if (!state.stk) return true; // ignore unreachable block
  if (state.fpilen != 0) {
    error("EH region starts with non-empty FPI stack at B%d\n",
           b->id);
    return false;
  }
  return true;
}

/**
 * Check the edge b->t, given the current state at the end of b.
 * If this is the first edge ->t we've seen, copy the state to t.
 * Otherwise, require the state exactly match.
 */
bool FuncChecker::checkEdge(Block* b, const State& cur, Block *t) {
  bool ok = true;
  State& state = m_info[t->id].state_in;
  if (!state.stk) {
    copyState(&state, &cur);
    return false;
  }
  // Check stack.
  if (state.stklen != cur.stklen) {
    reportStkMismatch(b, t, cur);
    return false;
  }
  for (int i = 0, n = cur.stklen; i < n; i++) {
    if (state.stk[i] != cur.stk[i]) {
      error("mismatch on edge B%d->B%d, current %s target %s\n",
             b->id, t->id, stkToString(n, cur.stk).c_str(),
             stkToString(n, state.stk).c_str());
      return false;
    }
  }
  // Check FPI stack.
  if (state.fpilen != cur.fpilen) {
    error("FPI stack depth mismatch on edge B%d->B%d, "
           "current %d target %d\n", b->id, t->id, cur.fpilen, state.fpilen);
    return false;
  }
  for (int i = 0, n = cur.fpilen; i < n; i++) {
    if (state.fpi[i] != cur.fpi[i]) {
      error("FPI mismatch on edge B%d->B%d, current %s target %s\n",
             b->id, t->id, fpiToString(cur.fpi[i]).c_str(),
             fpiToString(state.fpi[i]).c_str());
      return false;
    }
  }
  // Check iterator variable state.
  if (false /* TODO(#1097182): Iterator verification disabled */) {
    for (int i = 0, n = numIters(); i < n; i++) {
      if (state.iters[i] != cur.iters[i]) {
        error("mismatched iterator state on edge B%d->B%d, "
               "current %s target %s\n", b->id, t->id,
               iterToString(cur).c_str(), iterToString(state).c_str());
        return false;
      }
    }
  }
  return ok;
}

void FuncChecker::reportStkUnderflow(Block*, const State& cur, PC pc) {
  int min = cur.fpilen > 0 ? cur.fpi[cur.fpilen - 1].stkmin : 0;
  error("Rule2: Stack underflow at PC %d, min depth %d\n",
         offset(pc), min);
}

void FuncChecker::reportStkOverflow(Block*, const State& cur, PC pc) {
  error("Rule2: Stack overflow at PC %d\n", offset(pc));
}

void FuncChecker::reportStkMismatch(Block* b, Block* t, const State& cur) {
  const State& st = m_info[t->id].state_in;
  error("Rule1: Stack mismatch on edge B%d->B%d; depth %d->%d\n",
          b->id, t->id, cur.stklen, st.stklen);
}

void FuncChecker::reportEscapeEdge(Block* b, Block* s) {
  error("Edge from B%d to offset %d escapes function\n",
         b->id, offset(s->start));
}

/**
 * Check that the offset is within the region and it lands on an exact
 * instruction start.
 */
bool FuncChecker::checkOffset(const char* name, Offset off,
                              const char* regionName, Offset base,
                              Offset past, bool check_instrs) {
  assert(past >= base);
  if (off < base || off >= past) {
    error("Offset %s %d is outside region %s %d:%d\n",
           name, off, regionName, base, past);
    return false;
  }
  if (check_instrs && !m_instrs.get(off - m_func->base())) {
    error("label %s %d is not on a valid instruction boundary\n",
           name, off);
    return false;
  }
  return true;
}

/**
 * Check that the given inner region is valid, within the outer region, and
 * the inner region boundaries are exact instructions.
 */
bool FuncChecker::checkRegion(const char* name, Offset b, Offset p,
                               const char* regionName, Offset base,
                               Offset past, bool check_instrs) {
  assert(past >= base);
  if (p < b) {
    error("region %s %d:%d has negative length\n",
           name, b, p);
    return false;
  }
  if (b < base || p > past) {
    error("region %s %d:%d is not inside region %s %d:%d\n",
           name, b, p, regionName, base, past);
    return false;
  } else if (check_instrs &&
             (!m_instrs.get(b - m_func->base()) ||
              !m_instrs.get(p - m_func->base()))) {
    error("region %s %d:%d boundaries are inbetween instructions\n",
           name, b, p);
    return false;
  }
  return true;
}

}} // HPHP::Verifier
