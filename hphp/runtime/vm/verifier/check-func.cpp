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

#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/runtime/vm/verifier/pretty.h"

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/rx.h"

#include <folly/Range.h>

#include <boost/dynamic_bitset.hpp>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <stdexcept>
#include <type_traits>

namespace HPHP {
namespace Verifier {

/**
 * State for one entry on the FPI stack which corresponds to one call-site
 * in progress.
 */
struct FpiState {
  Offset fpush;   // offset of fpush (can get num_params from this)
  int stkmin;     // stklen before FPush
  bool operator==(const FpiState& s) const {
    return fpush == s.fpush && stkmin == s.stkmin;
  }
  bool operator!=(const FpiState& s) const {
    return !(*this == s);
  }
};

/**
 * Facts about a Func's current frame at various program points
 */
struct State {
  FlavorDesc* stk{};  // Evaluation stack.
  FpiState* fpi{};    // FPI stack.
  bool* iters{};      // defined/not-defined state of each iter var.
  boost::dynamic_bitset<> clsRefSlots; // init state of class-ref slots
  boost::dynamic_bitset<> writtenByClsRefGetTSSlots; // cls-ref slots written
                                                     // by ClsRefGetTS
  int stklen{0};       // length of evaluation stack.
  int fpilen{0};       // length of FPI stack.
  bool mbr_live{false};    // liveness of member base register
  folly::Optional<MOpMode> mbr_mode; // mode of member base register
  boost::dynamic_bitset<> silences; // set of silenced local variables
  bool guaranteedThis; // whether $this is guaranteed to be non-null
  // immediately following BaseL or BaseH in a minstr sequence, when the base
  // was known to be Rx mutable
  bool mbrMustContainMutableLocalOrThis;
  bool afterDim; // has there been a Dim in the current minstr seqence
};

/**
 * Facts about a specific block.
 */
struct BlockInfo {
  State state_in;  // state at the start of the block
};

struct IterKindId {
  IterKind kind;
  Id id;
  int local;
};
using IterKindIdTable = std::vector<IterKindId>;

struct FuncChecker {
  FuncChecker(const FuncEmitter* func, ErrorMode mode);
  ~FuncChecker();
  bool checkOffsets();
  bool checkFlow();

 private:
  struct unknown_length : std::runtime_error {
    unknown_length()
      : std::runtime_error("Unknown instruction length")
    {}
  };

  bool checkEdge(Block* b, const State& cur, Block* t);
  bool checkBlock(State& cur, Block* b);
  bool checkSuccEdges(Block* b, State* cur);
  bool checkOffset(const char* name, Offset o, const char* regionName,
                   Offset base, Offset past, bool check_instrs = true);
  bool checkRegion(const char* name, Offset b, Offset p,
                   const char* regionName, Offset base, Offset past,
                   bool check_instrs = true);
  bool checkSection(bool main, const char* name, Offset base, Offset past);
  bool checkImmediates(const char* name, PC instr);
  bool checkImmVec(PC& pc, size_t elemSize);
#define ARGTYPE(name, type) bool checkImm##name(PC& pc, PC instr);
#define ARGTYPEVEC(name, type) ARGTYPE(name, type)
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
  bool checkOp(State*, PC, Op, Block*);
  template<typename Subop> bool checkImmOAImpl(PC& pc, PC instr);
  bool checkMemberKey(State* cur, PC, Op);
  bool checkInputs(State* cur, PC, Block* b);
  bool checkOutputs(State* cur, PC, Block* b);
  bool checkRxOp(State* cur, PC, Op);
  bool checkSig(PC pc, int len, const FlavorDesc* args, const FlavorDesc* sig);
  bool checkEHStack(const EHEnt&, Block* b);
  bool checkTerminal(State* cur, PC pc);
  bool checkFCall(State* cur, PC pc);
  bool checkIter(State* cur, PC pc);
  bool checkClsRefSlots(State* cur, PC pc);
  bool checkIterBreak(State* cur, PC pc);
  bool checkLocal(PC pc, int val);
  bool checkString(PC pc, Id id);
  bool checkExnEdge(State cur, Block* b);
  void reportStkUnderflow(Block*, const State& cur, PC);
  void reportStkOverflow(Block*, const State& cur, PC);
  void reportStkMismatch(Block* b, Block* target, const State& cur);
  void reportEscapeEdge(Block* b, Block* s);
  std::string stateToString(const State& cur) const;
  std::string sigToString(int len, const FlavorDesc* sig) const;
  std::string stkToString(int len, const FlavorDesc* args) const;
  std::string fpiToString(const FpiState&) const;
  std::string iterToString(const State& cur) const;
  std::string slotsToString(const boost::dynamic_bitset<>&) const;
  void copyState(State* to, const State* from);
  void initState(State* s);
  const FlavorDesc* sig(PC pc);
  Offset offset(PC pc) const { return pc - unit()->bc(); }
  PC at(Offset off) const { return unit()->bc() + off; }
  int maxStack() const { return m_func->maxStackCells; }
  int maxFpi() const { return m_func->fpitab.size(); }
  int numIters() const { return m_func->numIterators(); }
  int numLocals() const { return m_func->numLocals(); }
  int numParams() const { return m_func->params.size(); }
  int numClsRefSlots() const { return m_func->numClsRefSlots(); }
  const UnitEmitter* unit() const { return &m_func->ue(); }
  IterKindIdTable iterBreakIds(PC& pc) const;

 private:
  template<class... Args>
  void error(const char* fmt, Args&&... args) {
    verify_error(
      unit(),
      m_func,
      m_errmode == kThrow,
      fmt,
      std::forward<Args>(args)...
    );
  }
  template<class... Args>
  void ferror(Args&&... args) {
    verify_error(unit(), m_func, m_errmode == kThrow, "%s",
                 folly::sformat(std::forward<Args>(args)...).c_str());
  }

 private:
  Arena m_arena;
  BlockInfo* m_info; // one per block
  const FuncEmitter* const m_func;
  Graph* m_graph;
  Bits m_instrs;
  ErrorMode m_errmode;
  FlavorDesc* m_tmp_sig;
  Id m_last_rpo_id; // rpo_id of the last block visited
};

const StaticString s_invoke("__invoke");

bool checkNativeFunc(const FuncEmitter* func, ErrorMode mode) {
  auto const funcname = func->name;
  auto const pc = func->pce();
  auto const clsname = pc ? pc->name() : nullptr;
  auto const& info = Native::getNativeFunction(func->ue().m_nativeFuncs,
                                               funcname, clsname,
                                               func->attrs & AttrStatic);

  if (!info.ptr) return true;

  if (func->isAsync) {
    verify_error(&func->ue(), func, mode == kThrow,
      "<<__Native>> function %s%s%s is declared async; <<__Native>> functions "
      "can return Awaitable<T>, but can not be declared async.\n",
      clsname ? clsname->data() : "",
      clsname ? "::" : "",
      funcname->data()
    );
    return false;
  }

  auto const& tc = func->retTypeConstraint;
  auto const message = Native::checkTypeFunc(info.sig, tc, func);

  if (message) {
    auto const tstr = info.sig.toString(clsname ? clsname->data() : nullptr,
                                        funcname->data());

    verify_error(&func->ue(), func, mode == kThrow,
      "<<__Native>> function %s%s%s does not match C++ function "
      "signature (%s): %s\n",
      clsname ? clsname->data() : "",
      clsname ? "::" : "",
      funcname->data(),
      tstr.c_str(),
      message
    );
    return false;
  }

  return true;
}

bool checkFunc(const FuncEmitter* func, ErrorMode mode) {
  if (mode == kVerbose) {
    pretty_print(func, std::cout);
    if (!func->pce()) {
      printf("  FuncId %d\n", func->id());
    }
    printFPI(func);
  }
  FuncChecker v(func, mode);
  return v.checkOffsets() &&
         v.checkFlow();
}

FuncChecker::FuncChecker(const FuncEmitter* f, ErrorMode mode)
: m_func(f)
, m_graph(0)
, m_instrs(m_arena, f->past - f->base + 1)
, m_errmode(mode) {
}

FuncChecker::~FuncChecker() {
  // if checkOffsets() is false, checkFlow() will never run so these will be
  // uninitialized
  if (!m_graph || !m_info) return;
  for (int i = 0; i < m_graph->block_count; i++) m_info[i].~BlockInfo();
}

// Needs to be a sorted map so we can divide funcs into contiguous sections.
using SectionMap = std::map<Offset,Offset>;

/**
 * Return the start offset of the nearest enclosing section.  Caller must
 * ensure that off is at least within the entire func's bytecode region.
 */
Offset findSection(SectionMap& sections, Offset off) {
  assertx(!sections.empty());
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
  assertx(unit()->bcPos() >= 0);
  PC bc = unit()->bc();
  Offset base = m_func->base;
  Offset past = m_func->past;
  checkRegion("func", base, past, "unit", 0, unit()->bcPos(), false);
  // find instruction boundaries and make sure no branches escape
  SectionMap sections;
  for (auto& eh : m_func->ehtab) {
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault funclet", eh.m_handler, "func bytecode", base,
                        past, false);
      sections[eh.m_handler] = 0;
    }
  }
  Offset funclets = !sections.empty() ? sections.begin()->first : past;
  sections[base] = funclets; // primary body
  // Get instruction boundaries and check branches within primary body
  // and each faultlet.
  for (auto i = sections.begin(), end = sections.end(); i != end;) {
    Offset section_base = i->first; ++i;
    Offset section_past = i == end ? past : i->first;
    sections[section_base] = section_past;
    ok &= checkSection(section_base == base,
                       section_base == base ? "primary body" : "funclet body",
                       section_base, section_past);
  }
  // DV entry points must be in the primary function body
  for (auto& param : m_func->params) {
    if (param.hasDefaultValue()) {
      ok &= checkOffset("dv-entry", param.funcletOff, "func body", base,
                        funclets);
    }
  }
  // Every FPI region must be contained within one section, either the
  // primary body or one fault funclet
  for (auto& fpi : m_func->fpitab) {
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
  for (auto& eh : m_func->ehtab) {
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault", eh.m_handler, "funclets", funclets, past);
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
  PC bc = unit()->bc();
  // Find instruction boundaries and branch instructions.
  for (InstrRange i(at(base), at(past)); !i.empty();) {
    auto pc = i.popFront();
    auto const op = peek_op(pc);
    if (!checkImmediates(name, pc)) {
      ferror("checkImmediates failed for {} @ {}\n",
             opcodeToName(op), offset(pc));
      return false;
    }
    m_instrs.set(offset(pc) - m_func->base);
    if (!instrJumpTargets(bc, offset(pc)).empty()) {
      if (op == OpSwitch && getImm(pc, 0).u_IVA == int(SwitchKind::Bounded)) {
        int64_t switchBase = getImm(pc, 1).u_I64A;
        int32_t len = getImmVector(pc).size();
        if (len <= 2) {
          error("Bounded switch must have a vector of length > 2 [%d:%d]\n",
                base, past);
        }
        if (switchBase > std::numeric_limits<int64_t>::max() - len + 2) {
          error("Overflow in Switch bounds [%d:%d]\n", base, past);
        }
      } else if (op == Op::SSwitch) {
        foreachSSwitchString(pc, [&](Id id) {
          ok &= checkString(pc, id);
        });
      }
      branches.push_back(pc);
    }
    if (i.empty()) {
      if (offset(pc + instrLen(pc)) != past) {
        error("Last instruction in %s at %d overflows [%d:%d]\n",
               name, offset(pc), base, past);
        ok = false;
      }
      if ((instrFlags(op) & TF) == 0) {
        error("Last instruction in %s is not terminal %d:%s\n",
               name, offset(pc), instrToString(pc, unit()).c_str());
        ok = false;
      } else {
        if (isRet(pc) && !is_main) {
          error("Ret* may not appear in %s\n", name);
          ok = false;
        } else if (op == Op::Unwind && is_main) {
          error("Unwind may not appear in %s\n", name);
          ok = false;
        }
      }
    }
  }
  // Check each branch target lands on a valid instruction boundary
  // within this region.
  for (auto const branch : branches) {
    auto const targets = instrJumpTargets(bc, offset(branch));
    for (auto const& target : targets) {
      ok &= checkOffset("branch target", target, name, base, past);
      if (peek_op(branch) == Op::JmpNS && target == offset(branch)) {
        error("JmpNS may not have zero offset in %s\n", name);
        ok = false;
      }
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

bool FuncChecker::checkLocal(PC pc, int k) {
  if (k < 0 || k >= numLocals()) {
    error("invalid local variable id %d at Offset %d\n",
           k, offset(pc));
    return false;
  }
  return true;
}

bool FuncChecker::checkString(PC /*pc*/, Id id) {
  if (isGlobalLitstrId(id)) {
    auto globalID = decodeGlobalLitstrId(id);
    return LitstrTable::get().contains(globalID);
  }
  return id < unit()->numLitstrs();
}

bool FuncChecker::checkImmVec(PC& pc, size_t elemSize) {
  auto const len = decode_iva(pc);
  if (len < 1) {
    error("invalid length of immediate vector %d at Offset %d\n",
          len, offset(pc));
    throw unknown_length{};
  }

  pc += len * elemSize;
  return true;
}

bool FuncChecker::checkImmBLA(PC& pc, PC const /*instr*/) {
  return checkImmVec(pc, sizeof(Offset));
}

bool FuncChecker::checkImmSLA(PC& pc, PC const /*instr*/) {
  return checkImmVec(pc, sizeof(Id) + sizeof(Offset));
}

bool FuncChecker::checkImmI32LA(PC& pc, PC const instr) {
  auto inst_copy = instr;
  decode_op(inst_copy);
  auto const nargs = decode_iva(inst_copy);
  auto const count = decode_iva(pc);
  for (int i = 0; i < count; ++i) {
    auto const num = decode_raw<uint32_t>(pc);
    if (nargs <= num) {
      error("invalid argument number (%i) in inout argument vector\n", i);
      return false;
    }
  }
  return true;
}

bool FuncChecker::checkImmILA(PC& pc, PC const /*instr*/) {
  auto const ids = iterBreakIds(pc);
  if (ids.size() < 1) {
    error("invalid length of iterator table %lu at Offset %d\n",
          ids.size(), offset(pc));
    return false;
  }
  auto ok = true;
  for (auto const& iter : ids) {
    if (iter.kind < KindOfIter || iter.kind > KindOfLIter) {
      error("invalid iterator kind %d in iter-vec at offset %d\n",
      iter.kind, offset(pc));
      ok = false;
    }
    if (iter.id < 0 || iter.id >= numIters()) {
      error("invalid iterator variable id %d at %d\n",
      iter.id, offset(pc));
      ok = false;
    }
    if (iter.kind == KindOfLIter) {
      if (iter.local < 0 || iter.local >= numLocals()) {
        error("invalid iterator local %d at %d\n", iter.local, offset(pc));
        ok = false;
      }
    } else if (iter.local != kInvalidId) {
      error("invalid iterator local for non-LIter at %d\n", offset(pc));
      ok = false;
    }
  }
  return ok;
}

bool FuncChecker::checkImmIVA(PC& pc, PC const instr) {
  auto const k = decode_iva(pc);
  switch (peek_op(instr)) {
    case Op::ConcatN:
      return k >= 2 && k <= kMaxConcatN;
    case Op::CombineAndResolveTypeStruct:
    case Op::RecordReifiedGeneric:
    case Op::ReifiedName:
      return k >= 1;
    default:
      return true;
  }
}

bool FuncChecker::checkImmI64A(PC& pc, PC const /*instr*/) {
  pc += sizeof(int64_t);
  return true;
}

bool FuncChecker::checkImmLA(PC& pc, PC const instr) {
  auto ok = true;
  auto const k = decode_iva(pc);
  ok &= checkLocal(pc, k);
  switch (peek_op(instr)) {
    case Op::VerifyParamType:
    case Op::VerifyParamTypeTS:
      if (k >= numParams()) {
        error("invalid parameter id %d at %d\n", k, offset(instr));
        ok = false;
      }
      break;
    default:
      break;
  }
  return ok;
}

bool FuncChecker::checkImmIA(PC& pc, PC const instr) {
  auto const k = decode_iva(pc);
  if (k >= numIters()) {
    error("invalid iterator variable id %d at %d\n", k, offset(instr));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmCAR(PC& pc, PC const instr) {
  auto const slot = decode_iva(pc);
  if (slot >= numClsRefSlots()) {
    error("invalid class-ref slot %d at %d\n", slot, offset(instr));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmCAW(PC& pc, PC const instr) {
  auto const slot = decode_iva(pc);
  if (slot >= numClsRefSlots()) {
    error("invalid class-ref slot %d at %d\n", slot, offset(instr));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmDA(PC& pc, PC const /*instr*/) {
  pc += sizeof(double);
  return true;
}

bool FuncChecker::checkImmSA(PC& pc, PC const /*instr*/) {
  auto const id = decodeId(&pc);
  return checkString(pc, id);
}

bool FuncChecker::checkImmAA(PC& pc, PC const /*instr*/) {
  auto const id = decodeId(&pc);
  if (id < 0 || id >= (Id)unit()->numArrays()) {
    error("invalid array id %d\n", id);
    return false;
  }
  return true;
}

bool FuncChecker::checkImmRATA(PC& pc, PC const /*instr*/) {
  // Nothing to check at the moment.
  pc += encodedRATSize(pc);
  return true;
}

bool FuncChecker::checkImmBA(PC& pc, PC const instr) {
  // we check branch offsets in checkSection(). ignore here.
  assertx(!instrJumpTargets(unit()->bc(), offset(instr)).empty());
  pc += sizeof(Offset);
  return true;
}

bool FuncChecker::checkImmVSA(PC& pc, PC const /*instr*/) {
  auto const len = decode_iva(pc);
  if (len < 1 || len > ArrayData::MaxElemsOnStack) {
    error("invalid length of immedate VSA vector %d at offset %d\n",
          len, offset(pc));
    throw unknown_length{};
  }

  auto ok = true;
  for (int i = 0; i < len; i++) {
    auto const id = decodeId(&pc);
    ok &= checkString(pc, id);
  }
  return ok;
}

template <typename Subop>
bool FuncChecker::checkImmOAImpl(PC& pc, PC const /*instr*/) {
  auto const subop = decode_oa<Subop>(pc);
  if (!subopValid(subop)) {
    ferror("Invalid subop {}\n", size_t(subop));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmKA(PC& pc, PC const /*instr*/) {
  static_assert(
      std::is_unsigned<typename std::underlying_type<MemberCode>::type>::value,
      "MemberCode is expected to be unsigned.");

  auto const mcode = decode_raw<MemberCode>(pc);
  if (mcode >= NumMemberCodes) {
    ferror("Invalid MemberCode {}\n", uint8_t{mcode});
    return false;
  }

  auto ok = true;
  switch (mcode) {
    case MW:
      break;
    case MEL: case MPL: {
      auto const loc = decode_iva(pc);
      ok &= checkLocal(pc, loc);
      break;
    }
    case MEC: case MPC:
      decode_iva(pc);
      break;
    case MEI:
      pc += sizeof(int64_t);
      break;
    case MET: case MPT: case MQT:
      auto const id = decode_raw<Id>(pc);
      ok &= checkString(pc, id);
      break;
  }

  return ok;
}

bool FuncChecker::checkImmLAR(PC& pc, PC const instr) {
  auto ok = true;
  auto const range = decodeLocalRange(pc);
  for (auto i = uint32_t{0}; i < range.count; ++i) {
    ok &= checkLocal(instr, range.first + i);
  }
  return ok;
}

bool FuncChecker::checkImmFCA(PC& pc, PC const instr) {
  auto fca = decodeFCallArgs(pc);
  if (fca.numRets == 0) {
    ferror("FCall at {} must return at least one value\n", offset(instr));
    return false;
  }
  return true;
}

IterKindIdTable FuncChecker::iterBreakIds(PC& pc) const {
  IterKindIdTable ret;
  auto const length = decode_iva(pc);
  for (int i = 0; i < length; ++i) {
    auto const kind = static_cast<IterKind>(decode_iva(pc));
    auto const id = static_cast<Id>(decode_iva(pc));
    auto const local = (kind == KindOfLIter)
      ? static_cast<int32_t>(decode_iva(pc))
      : kInvalidId;
    ret.push_back(IterKindId{kind, id, local});
  }
  return ret;
}

/**
 * Check instruction and its immediates. Returns false if we can't continue
 * because we don't know the length of this instruction or one of the
 * immediates was invalid.
 */
bool FuncChecker::checkImmediates(const char* name, PC const instr) {
  auto pc = instr;

  auto const op = decode_op(pc);
  if (!isValidOpcode(op)) {
    error("Invalid opcode %d in section %s at offset %d\n",
          size_t(op), name, offset(instr));
    return false;
  }
  bool ok = true;
  try {
    switch (op) {
#define NA
#define checkImmOA(ty) checkImmOAImpl<ty>
#define ONE(a) ok &= checkImm##a(pc, instr)
#define TWO(a, b) ONE(a); ok &= checkImm##b(pc, instr)
#define THREE(a, b, c) TWO(a, b); ok &= checkImm##c(pc, instr)
#define FOUR(a, b, c, d) THREE(a, b, c); ok &= checkImm##d(pc, instr)
#define FIVE(a, b, c, d, e) FOUR(a, b, c, d); ok &= checkImm##e(pc, instr)
#define O(name, imm, in, out, flags) case Op::name: imm; break;
      OPCODES
#undef NA
#undef checkOA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef O
    }
  } catch (const unknown_length&) {
    return false;
  }

  auto const expectPC = instr + instrLen(instr);
  if (pc != expectPC) {
    ferror("PC after decoding {} at {} is {} instead of expected {}\n",
           opcodeToName(op), instr, pc, expectPC);
    return false;
  }
  return ok;
}

static const char* stkflav(FlavorDesc f) {
  switch (f) {
  case NOV:  return "N";
  case CV:   return "C";
  case VV:   return "V";
  case UV:   return "U";
  case CUV:  return "C|U";
  case CVV:  return "C|V";
  case CVUV: return "C|V|U";
  }
  not_reached();
}

static bool checkArg(FlavorDesc expect, FlavorDesc check) {
  if (expect == check) return true;

  switch (expect) {
    case CVV:  return check == CV || check == VV;
    case CUV:  return check == CV || check == UV;
    case CVUV: return check == CV || check == VV || check == UV || check == CUV;
    default:   return false;
  }
}

bool FuncChecker::checkSig(PC pc, int len, const FlavorDesc* args,
                           const FlavorDesc* sig) {
  for (int i = 0; i < len; ++i) {
    if (!checkArg(sig[i], args[i])) {
      error("flavor mismatch at %d, got %s expected %s\n",
             offset(pc), stkToString(len, args).c_str(),
             sigToString(len, sig).c_str());
      return false;
    }
  }
  return true;
}

const FlavorDesc* FuncChecker::sig(PC pc) {
  static const FlavorDesc inputSigs[][kMaxHhbcImms] = {
  #define NOV { },
  #define CVUMANY { },
  #define FPUSH(nin, nobj) { },
  #define FCALL { },
  #define CMANY { },
  #define SMANY { },
  #define ONE(a) { a },
  #define TWO(a,b) { b, a },
  #define THREE(a,b,c) { c, b, a },
  #define FOUR(a,b,c,d) { d, c, b, a },
  #define FIVE(a,b,c,d,e) { e, d, c, b, a },
  #define MFINAL { },
  #define F_MFINAL { },
  #define C_MFINAL(n) { },
  #define V_MFINAL { },
  #define O(name, imm, pop, push, flags) pop
    OPCODES
  #undef O
  #undef MFINAL
  #undef F_MFINAL
  #undef C_MFINAL
  #undef V_MFINAL
  #undef CVUMANY
  #undef FPUSH
  #undef FCALL
  #undef CMANY
  #undef SMANY
  #undef FIVE
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef NOV
  };
  switch (peek_op(pc)) {
  case Op::QueryM:
  case Op::VGetM:
  case Op::IncDecM:
  case Op::UnsetM:
  case Op::SetM:
  case Op::SetOpM:
  case Op::SetRangeM:
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CV;
    }
    return m_tmp_sig;
  case Op::BindM:
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = i == n - 1 ? VV : CV;
    }
    return m_tmp_sig;
  case Op::FPushFunc:
  case Op::FPushFuncD:
  case Op::FPushFuncU:
  case Op::FPushCtor:
  case Op::FPushObjMethod:
  case Op::FPushObjMethodD:
  case Op::FPushClsMethod:
  case Op::FPushClsMethodS:
  case Op::FPushClsMethodSD:
  case Op::FPushClsMethodD: {  // IVA..., FPUSH, FPUSH
    auto const numPops = instrNumPops(pc);
    auto idx = 0;
    while (idx < numPops) m_tmp_sig[idx++] = CV;
    return m_tmp_sig;
  }
  case Op::FCall: {      // THREE(FCA,SA,SA), FCALL, FCALL
    auto const fca = getImm(pc, 0).u_FCA;
    assertx(fca.numRets != 0);
    auto idx = 0;
    for (int i = 0; i < fca.numRets - 1; ++i) m_tmp_sig[idx++] = UV;
    for (int i = 0; i < fca.numArgs; ++i) m_tmp_sig[idx++] = CVV;
    if (fca.hasUnpack()) m_tmp_sig[idx++] = CV;
    assertx(idx == instrNumPops(pc));
    return m_tmp_sig;
  }
  case Op::FCallBuiltin: //TWO(IVA, SA), CVUMANY,  ONE(CV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CVUV;
    }
    return m_tmp_sig;
  case Op::CreateCl:  // TWO(IVA,SA),  CVUMANY,   ONE(CV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CVUV;
    }
    return m_tmp_sig;
  case Op::NewPackedArray:  // ONE(IVA),     CMANY,   ONE(CV)
  case Op::NewStructArray:  // ONE(VSA),     SMANY,   ONE(CV)
  case Op::NewStructDArray: // ONE(VSA),     SMANY,   ONE(CV)
  case Op::NewStructDict:   // ONE(VSA),     SMANY,   ONE(CV)
  case Op::NewVecArray:     // ONE(IVA),     CMANY,   ONE(CV)
  case Op::NewKeysetArray:  // ONE(IVA),     CMANY,   ONE(CV)
  case Op::NewVArray:       // ONE(IVA),     CMANY,   ONE(CV)
  case Op::NewRecord:       // TWO(SA,VSA),  SMANY,   ONE(CV)
  case Op::ConcatN:         // ONE(IVA),     CMANY,   ONE(CV)
  case Op::RecordReifiedGeneric: // ONE(IVA),CMANY,   ONE(CV)
  case Op::ReifiedName:     // ONE(IVA),     CMANY,   ONE(CV)
  case Op::CombineAndResolveTypeStruct:
                            // ONE(IVA),     CMANY,   ONE(CV)
  case Op::RetM:            // ONE(IVA),     CMANY,   NA
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CV;
    }
    return m_tmp_sig;
  default:
    return &inputSigs[size_t(peek_op(pc))][0];
  }
}

bool FuncChecker::checkMemberKey(State* cur, PC pc, Op op) {
  MemberCode mcode;

  switch(op){
    case Op::IncDecM:
    case Op::SetOpM:
    case Op::QueryM:  //THREE(IVA, OA, KA)
      decode_op(pc);
      decode_iva(pc);
      decode_byte(pc);
      mcode = static_cast<MemberCode>(decode_byte(pc));
      break;
    case Op::BindM:
    case Op::UnsetM:
    case Op::SetM:
    case Op::VGetM:   //TWO(IVA, KA)
      decode_op(pc);
      decode_iva(pc);
      mcode = static_cast<MemberCode>(decode_byte(pc));
      break;
    case Op::SetRangeM:
      return true;

    default:
      always_assert(false);
      return false;
  }

  uint32_t iva = 0;
  switch (mcode) {
    case MET: case MPT: case MQT: {
      auto const id = decode_raw<Id>(pc);
      if (!checkString(pc, id)) {
        error("Member Key in op %s contains an invalid string id\n",
              opcodeToName(op));
        return false;
      }
      break;
    }

    case MEC: case MEL: case MPC: case MPL: iva = decode_iva(pc); break;
    case MEI:                               decode_raw<int64_t>(pc); break;
    case MW:                                break;
  }

  if ((mcode == MemberCode::MEC || mcode == MemberCode::MPC) &&
      iva + 1 > cur->stklen) {
    MemberKey key{mcode, iva};
    error("Member Key %s in op %s has stack offset greater than stack"
          " depth %d\n", show(key).c_str(), opcodeToName(op), cur->stklen);
    return false;
  }

  return true;
}

bool FuncChecker::checkInputs(State* cur, PC pc, Block* b) {
  StackTransInfo info = instrStackTransInfo(pc);
  auto fpiAdj = isFCallStar(peek_op(pc)) ? 2 : 1;
  int min = cur->fpilen > fpiAdj - 1
    ? cur->fpi[cur->fpilen - fpiAdj].stkmin
    : 0;
  if (info.numPops > 0 && cur->stklen - info.numPops < min) {
    reportStkUnderflow(b, *cur, pc);
    cur->stklen = 0;
    return false;
  }
  cur->stklen -= info.numPops;
  auto ok = checkSig(pc, info.numPops, &cur->stk[cur->stklen], sig(pc));
  auto const op = peek_op(pc);
  auto const need_live = isMemberDimOp(op) || isMemberFinalOp(op);
  auto const live_ok = need_live || isTypeAssert(op);
  if ((need_live && !cur->mbr_live) || (cur->mbr_live && !live_ok)) {
    error("Member base register %s coming into %s\n",
          cur->mbr_live ? "live" : "dead", opcodeToName(op));
    ok = false;
  }

  if (cur->mbr_live && isMemberOp(op)) {
    folly::Optional<MOpMode> op_mode;
    auto new_pc = pc;
    if (op == Op::QueryM) {
      decode_op(new_pc);
      decode_iva(new_pc);
      op_mode = getQueryMOpMode(decode_oa<QueryMOp>(new_pc));
    } else if (isMemberFinalOp(op)) {
      op_mode = finalMemberOpMode(op);
    } else if (op == Op::Dim) {
      decode_op(new_pc);
      op_mode = decode_oa<MOpMode>(new_pc);
    }

    if (op_mode == MOpMode::InOut) {
      if (op_mode == MOpMode::InOut && op != Op::Dim && op != Op::QueryM) {
        error("Member instruction %s is incompatible with mode InOut\n",
              opcodeToName(op));
        ok = false;
      } else {
        auto const c = static_cast<MemberCode>(decode_byte(new_pc));
        if (!mcodeIsElem(c)) {
          error("Member instruction with mode InOut and incompatible member "
                "code %s, must be Elem\n", memberCodeString(c));
          ok = false;
        }
      }
    }

    if(cur->mbr_mode && cur->mbr_mode != op_mode) {
      if (cur->mbr_mode != MOpMode::Warn ||
        (op_mode != MOpMode::InOut && op_mode != MOpMode::None)) {
        error("Member base register mode at %s is %s when it should be %s\n",
              opcodeToName(op),
              op_mode ? subopToName(op_mode.value()) : "Unknown",
              subopToName(cur->mbr_mode.value()));
        ok = false;
      }
    }

    cur->mbr_mode = op_mode;
  }

  if (cur->fpilen > 0 && isJmp(op)) {
    auto offset = getImm(pc, 0).u_BA;
    if (offset < 0) {
      error("FPI contains backwards jump at %s\n",
            opcodeToName(op));
      ok = false;
    }
  }

  return ok;
}

bool FuncChecker::checkTerminal(State* cur, PC pc) {
  if (isRet(pc) || peek_op(pc) == Op::Unwind) {
    if (cur->stklen != 0) {
      error("stack depth must equal 0 after Ret* and Unwind; got %d\n",
             cur->stklen);
      return false;
    }
    if (cur->clsRefSlots.any() || cur->writtenByClsRefGetTSSlots.any()) {
      ferror("all class-ref slots must be uninitialized after Ret* and Unwind; "
             "got [{}][{}]\n",
             slotsToString(cur->clsRefSlots),
             slotsToString(cur->writtenByClsRefGetTSSlots));
      return false;
    }
  }
  return true;
}

bool FuncChecker::checkFCall(State* cur, PC pc) {
  assertx(isFCallStar(peek_op(pc)));

  if (cur->fpilen <= 0) {
    error("%s", "cannot access empty FPI stack\n");
    return false;
  }
  bool ok = true;
  FpiState& fpi = cur->fpi[cur->fpilen - 1];
  --cur->fpilen;
  auto const fca = getImm(pc, 0).u_FCA;
  int call_params = fca.numArgs + (fca.hasUnpack() ? 1 : 0);
  int push_params = getImmIva(at(fpi.fpush));
  if (call_params != push_params) {
    error("FCall* param_count (%d) doesn't match FPush* (%d)\n",
           call_params, push_params);
    ok = false;
  }
  if (cur->stklen != fpi.stkmin - fca.numRets + 1) {
    error("wrong # of params were passed; got %d expected %d\n",
          push_params + cur->stklen + fca.numRets - 1 - fpi.stkmin,
          push_params);
    ok = false;
  }

  return ok;
}

// Check that the initialization state of the iterator referenced by the current
// iterator instruction is valid. For *IterInit, it must not already be
// initialized; for *IterNext and *IterFree, it must be initialized.
bool FuncChecker::checkIter(State* cur, PC const pc) {
  assertx(isIter(pc));
  int id = getImmIva(pc);
  bool ok = true;
  auto op = peek_op(pc);
  if (op == Op::IterInit || op == Op::IterInitK ||
      op == Op::LIterInit || op == Op::LIterInitK) {
    if (cur->iters[id]) {
      error(
        "IterInit* <%d> trying to double-initialize\n", id);
      ok = false;
    }
  } else {
    if (!cur->iters[id]) {
      error("Cannot access un-initialized iter %d\n", id);
      ok = false;
    }
    if (op == Op::IterFree || op == Op::LIterFree) {
      cur->iters[id] = false;
    }
  }
  return ok;
}

std::array<int32_t, 4> getReadClsRefSlots(PC pc) {
  std::array<int32_t, 4> ret = { -1, -1, -1, -1 };
  size_t index = 0;

  auto const op = peek_op(pc);
  auto const numImmeds = numImmediates(op);
  for (int i = 0; i < numImmeds; ++i) {
    if (immType(op, i) == ArgType::CAR) {
      assertx(index < ret.size());
      ret[index++] = getImm(pc, i).u_CAR;
    }
  }
  return ret;
}

std::array<int32_t, 4> getWrittenClsRefSlots(PC pc) {
  std::array<int32_t, 4> ret = { -1, -1, -1, -1 };
  size_t index = 0;

  auto const op = peek_op(pc);
  auto const numImmeds = numImmediates(op);
  for (int i = 0; i < numImmeds; ++i) {
    if (immType(op, i) == ArgType::CAW) {
      assertx(index < ret.size());
      ret[index++] = getImm(pc, i).u_CAW;
    }
  }
  return ret;
}

/* Returns a set of the immediates to op that are a local id */
std::set<int> localImmediates(Op op) {
  std::set<int> imms;
  switch (op) {
#define NA
#define ONE(a) a(0)
#define TWO(a, b) ONE(a) b(1)
#define THREE(a, b, c) TWO(a, b) c(2)
#define FOUR(a, b, c, d) THREE(a, b, c) d(3)
#define FIVE(a, b, c, d, e) FOUR(a, b, c, d) e(4)
#define LA(n) imms.insert(n);
#define MA(n)
#define BLA(n)
#define SLA(n)
#define ILA(n)
#define I32LA(n)
#define IVA(n)
#define I64A(n)
#define IA(n)
#define CAR(n)
#define CAW(n)
#define DA(n)
#define SA(n)
#define AA(n)
#define RATA(n)
#define BA(n)
#define EMPTY_OA(n)
#define OA(op) EMPTY_OA
#define VSA(n)
#define KA(n)
#define LAR(n)
#define FCA(n)
#define O(name, imm, in, out, flags) case Op::name: imm; break;
    OPCODES
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef LA
#undef MA
#undef BLA
#undef SLA
#undef ILA
#undef I32LA
#undef IVA
#undef I64A
#undef IA
#undef CAR
#undef CAW
#undef DA
#undef SA
#undef AA
#undef RATA
#undef BA
#undef EMPTY_OA
#undef OA
#undef VSA
#undef KA
#undef LAR
#undef FCA
#undef O
  }
  return imms;
}

bool FuncChecker::checkClsRefSlots(State* cur, PC const pc) {
  bool ok = true;

  auto const op = peek_op(pc);

  for (auto const read : getReadClsRefSlots(pc)) {
    if (read < 0) continue;
    if (!cur->clsRefSlots[read]) {
      ferror("{} trying to read from uninitialized class-ref slot {} at {}\n",
             opcodeToName(op), read, offset(pc));
      ok = false;
    }
    cur->clsRefSlots[read] = false;
    cur->writtenByClsRefGetTSSlots[read] = false;
  }
  for (auto const write : getWrittenClsRefSlots(pc)) {
    if (write < 0) continue;
    if (cur->clsRefSlots[write]) {
      ferror(
        "{} trying to write to already initialized class-ref slot {} at {}\n",
        opcodeToName(op), write, offset(pc)
      );
      ok = false;
    }
    cur->clsRefSlots[write] = true;
    cur->writtenByClsRefGetTSSlots[write] = op == Op::ClsRefGetTS;
  }

  return ok;
}

bool FuncChecker::checkOp(State* cur, PC pc, Op op, Block* b) {
  switch (op) {
    case Op::BreakTraceHint:
      if (cur->mbr_live) {
        // Special case for unreachable code. hhbbc generates
        // BreakTraceHint; String; Fatal
        auto const str = pc + instrLen(pc);
        if (peek_op(str) != Op::String) break;
        auto const fatal = str + instrLen(str);
        if (peek_op(fatal) != Op::Fatal) break;
        cur->mbr_live = false;
      }
      break;
    case Op::InitProp:
    case Op::CheckProp: {
        auto fname = m_func->name->toCppString();
        if (fname.compare("86pinit") != 0 &&
            fname.compare("86sinit") != 0 &&
            fname.compare("86linit") != 0) {
          ferror("{} cannot appear in {} function\n", opcodeToName(op), fname);
          return false;
        }
        if (!LitstrTable::canRead()) {
          // Unfortunately in order to check if the property name itself is
          // valid we need to be able to read from the Litstr table, which we
          // cannot do while verifying an optimized repo in hhbbc.
          if (!checkString(pc, getImm(pc, 0).u_SA)) return false;
          break;
        }
        auto const prop = m_func->ue().lookupLitstr(getImm(pc, 0).u_SA);
        if (!m_func->pce() || !m_func->pce()->hasProp(prop)){
             ferror("{} references non-existent property {}\n",
                    opcodeToName(op), prop);
             return false;
        }
        break;
    }
    case Op::This:
    case Op::CheckThis: {
      cur->guaranteedThis = true;
      break;
    }
    case Op::BareThis: {
      auto new_pc = pc;
      decode_op(new_pc);
      if (decode_oa<BareThisOp>(new_pc) != BareThisOp::NeverNull) break;
    }
    case Op::BaseH: {
      // In HHBBC we can track the $this across loads into locals and pushes
      // onto the stack. If the types of those equivalent locations are refined
      // we may end up knowing that $this is never null. The verifier doesn't
      // currently support this type of sophisticated tracking and it's doubtful
      // there would be much value in adding it.
      if (!RuntimeOption::RepoAuthoritative && !cur->guaranteedThis) {
        ferror("{} required that $this be guaranteed to be non-null\n",
        opcodeToName(op));
        return false;
      }
      break;
    }
    case Op::DefCls:
    case Op::DefClsNop: {
      auto const id = getImm(pc, 0).u_IVA;
      if (id >= unit()->numPreClasses()) {
        ferror("{} references nonexistent class ({})\n", opcodeToName(op), id);
        return false;
      }
      break;
    }
    case Op::DefRecord: {
      auto const id = getImm(pc, 0).u_IVA;
      if (id >= unit()->numRecords()) {
        ferror("{} references nonexistent record ({})\n", opcodeToName(op), id);
        return false;
      }
      break;
    }
    case Op::CreateCl: {
      auto const id = getImm(pc, 1).u_IVA;
      if (id >= unit()->numPreClasses()) {
        ferror("CreateCl must reference a closure defined in the same "
               "unit\n");
        return false;
      }
      auto const preCls = unit()->pce(id);
      if (preCls->parentName()->toCppString() != std::string("Closure")) {
        ferror("CreateCl references non-closure class {} ({})\n",
               preCls->name(), id);
        return false;
      }
      auto const numBound = getImm(pc, 0).u_IVA;
      auto const invoke = preCls->lookupMethod(s_invoke.get());
      if (invoke &&
          numBound != preCls->numProperties() - invoke->staticVars.size()) {
        ferror("CreateCl bound Closure {} with {} params instead of {}\n",
               preCls->name(), numBound, preCls->numProperties());
        return false;
      }
      break;
    }
    case Op::DefTypeAlias: {
      auto id = getImm(pc, 0).u_IVA;
      if (id >= unit()->typeAliases().size()) {
        ferror("{} references nonexistent type alias ({})\n",
                opcodeToName(op), id);
        return false;
      }
      break;
    }
    #define O(name) \
    case Op::name: { \
      auto const id = getImm(pc, 0).u_AA; \
      if (id < 0 || id >= unit()->numArrays() || \
          unit()->lookupArray(id)->toDataType() != KindOf##name) { \
        ferror("{} references array data that is not a {}\n", \
                #name, #name); \
        return false; \
      } \
      break; \
    }
    O(Keyset)
    O(Array)
    O(Dict)
    O(Vec)
    #undef O
    case Op::GetMemoKeyL:
    case Op::MemoGet:
    case Op::MemoGetEager:
    case Op::MemoSet:
    case Op::MemoSetEager:
      if (!m_func->isMemoizeWrapper) {
        ferror("{} can only appear within memoize wrappers\n",
               opcodeToName(op));
        return false;
      }
      if (op == Op::MemoGetEager || op == Op::MemoSetEager) {
        if (!m_func->isAsync || m_func->isGenerator) {
          ferror("{} can only appear within async functions\n",
                 opcodeToName(op));
          return false;
        }
      }
      break;
    case Op::NewCol:
    case Op::ColFromArray: {
      auto new_pc = pc;
      decode_op(new_pc);
      auto colType = decode_oa<CollectionType>(new_pc);
      if (colType == CollectionType::Pair) {
        ferror("Immediate of {} must not be a pair\n", opcodeToName(op));
        return false;
      }
      break;
    }
    case Op::Catch: {
      auto handler = Func::findEHbyHandler(m_func->ehtab, offset(pc));
      if (!handler || handler->m_type != EHEnt::Type::Catch ||
          offset(pc) != handler->m_handler) {
        ferror("{} must be the first instruction in a Catch handler\n",
               opcodeToName(op));
        return false;
      }
      break;
    }
    case Op::AssertRATL:
    case Op::AssertRATStk: {
      if (pc == b->last){
        ferror("{} cannot appear at the end of a block\n", opcodeToName(op));
        return false;
      }
      if (op == Op::AssertRATL) break;
    }
    case Op::BaseGC:
    case Op::BaseSC:
    case Op::BaseC: {
      auto const stackIdx = getImm(pc, 0).u_IVA;
      if (stackIdx >= cur->stklen) {
        ferror("{} indexes ({}) past end of stack ({})\n", opcodeToName(op),
               stackIdx, cur->stklen);
        return false;
      }
      break;
    }
    case Op::ContAssignDelegate:
    case Op::ContEnterDelegate:
    case Op::YieldFromDelegate:
    case Op::ContUnsetDelegate:
      if (m_func->isAsync) {
        ferror("{} may only appear in a non-async generator\n",
               opcodeToName(op));
        return false;
      }
      // fallthrough
    case Op::CreateCont:
    case Op::YieldK:
    case Op::Yield:
      if (!m_func->isGenerator) {
        ferror("{} may only appear in a generator\n", opcodeToName(op));
        return false;
      }
      break;
    case Op::ContEnter: // Only in non-static generator methods
    case Op::ContRaise: {
      auto cls = m_func->pce();
      if ((m_func->attrs & AttrStatic) || !cls ||
          (cls->name()->toCppString() != std::string("Generator") &&
           cls->name()->toCppString() != std::string("HH\\AsyncGenerator"))) {
        ferror("{} may only appear in non-static methods of the "
               "[Async]Generator class\n", opcodeToName(op));
        return false;
      }
      break;
    }

    case Op::AwaitAll: {
      auto const& range = getImm(pc, 0).u_LAR;
      if (range.count == 0) {
        ferror("{} must have a non-empty local range\n", opcodeToName(op));
        return false;
      }
      // Fall-through
    }
    case Op::Await: {
      if (!m_func->isAsync) {
        ferror("{} may only appear in an async function\n", opcodeToName(op));
        return false;
      }
      if (cur->fpilen != 0) {
        ferror("{} may not appear in the middle of an FPI region\n",
               opcodeToName(op));
        return false;
      }
      if (cur->stklen != instrNumPops(pc)) {
        ferror("{} may not be used with non-empty stack\n", opcodeToName(op));
        return false;
      }
      break;
    }
    case Op::Silence: {
      auto new_pc = pc;
      decode_op(new_pc);
      auto const local = decode_iva(new_pc);
      if (local < m_func->numNamedLocals()) {
        ferror("Cannot store error reporting value in named local\n");
        return false;
      }

      auto const silence = decode_oa<SilenceOp>(new_pc);
      if (local + 1 > cur->silences.size()) cur->silences.resize(local + 1);
      if (silence == SilenceOp::End && !cur->silences[local]) {
        ferror("Silence ended on local variable {} with no start\n", local);
        return false;
      }

      cur->silences[local] = silence == SilenceOp::Start ? 1 : 0;
      break;
    }
    case Op::Fatal:
      // fatals skip all fault handlers, and the silence state is reset
      // by the runtime.
      cur->silences.clear();
      break;

    case Op::NewPackedArray:
    case Op::NewVArray:
    case Op::NewVecArray:
    case Op::NewKeysetArray: {
      auto new_pc = pc;
      decode_op(new_pc);
      auto const elems = decode_iva(new_pc);
      static_assert(std::is_unsigned<decltype(elems)>::value,
                    "IVA should be unsigned");
      if (elems == 0 || elems > ArrayData::MaxElemsOnStack) {
        ferror("{} has an illegal element count\n", opcodeToName(op));
        return false;
      }
      break;
    }

    case Op::NewObj: {
      auto new_pc = pc;
      decode_op(new_pc);
      auto const slot = decode_iva(new_pc);
      auto const has_generic_op = decode_oa<HasGenericsOp>(new_pc);
      if (has_generic_op == HasGenericsOp::MaybeGenerics &&
          !cur->writtenByClsRefGetTSSlots[slot]) {
        ferror("NewObj uses MaybeGenerics flavor but a cls-ref slot that "
               "was not written by ClsRefGetTS\n");
        return false;
      }
      break;
    }

    case Op::RetCSuspended:
      if (!m_func->isAsync || m_func->isGenerator) {
        ferror("{} can only appear within async functions\n",
               opcodeToName(op));
        return false;
      }

    default:
      break;
  }

  if (op != Op::Silence && !isTypeAssert(op)) {
    for (int imm : localImmediates(op)) {
      auto local = getImm(pc, imm).u_LA;
      if (cur->silences.size() > local && cur->silences[local]) {
        ferror("{} at PC {} affected a local variable ({}) which was reserved "
               "for storing the error reporting level\n",
               opcodeToName(op), offset(pc), local);
        return false;
      }
    }
  }

  return true;
}

// Check that each of the iterators provided as arguments to IterBreak
// are currently initialized.
bool FuncChecker::checkIterBreak(State* cur, PC pc) {
  pc += encoded_op_size(Op::IterBreak); // skip opcode
  decode_raw<Offset>(pc); // skip target offset
  for (auto const& iter : iterBreakIds(pc)) {
    if (!cur->iters[iter.id]) {
      error("Cannot access un-initialized iter %d\n", iter.id);
      return false;
    }
    // IterBreak has no fall-through path, so don't change iter.id's current
    // state; instead it will be done in checkSuccEdges.
  }
  return true;
}

bool FuncChecker::checkOutputs(State* cur, PC pc, Block* b) {
  static const FlavorDesc outputSigs[][kMaxHhbcImms] = {
  #define NOV { },
  #define FPUSH { },
  #define FCALL { },
  #define ONE(a) { a },
  #define TWO(a,b) { a, b },
  #define THREE(a,b,c) { a, b, c },
  #define FOUR(a,b,c,d) { a, b, c, d },
  #define FIVE(a,b,c,d,e) { a, b, c, d, e },
  #define INS_1(a) { a },
  #define O(name, imm, pop, push, flags) push
    OPCODES
  #undef O
  #undef INS_1
  #undef FIVE
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef FPUSH
  #undef FCALL
  #undef NOV
  };
  bool ok = true;
  auto const op = peek_op(pc);
  StackTransInfo info = instrStackTransInfo(pc);
  if (info.kind == StackTransInfo::Kind::InsertMid) {
    int index = cur->stklen - info.pos - 1;
    if (index < 0) {
      reportStkUnderflow(b, *cur, pc);
      return false;
    }
    memmove(&cur->stk[index + 1], &cur->stk[index],
           (info.pos + 1) * sizeof(*cur->stk));
    cur->stk[index] = outputSigs[size_t(op)][0];
    cur->stklen++;
  } else {
    int pushes = info.numPushes;
    if (cur->stklen + pushes > maxStack()) reportStkOverflow(b, *cur, pc);
    FlavorDesc *outs = &cur->stk[cur->stklen];
    cur->stklen += pushes;
    if (op == Op::BaseSC) {
      if (pushes == 1) outs[0] = outs[1];
    } else if (op == Op::FCall) {
      for (int i = 0; i < pushes; ++i) {
        outs[i] = CV;
      }
    } else {
      for (int i = 0; i < pushes; ++i) {
        outs[i] = outputSigs[size_t(op)][i];
      }
    }
    if (isFPush(op)) {
      if (cur->fpilen >= maxFpi()) {
        error("%s", "more FPush* instructions than FPI regions\n");
        return false;
      }
      FpiState& fpi = cur->fpi[cur->fpilen];
      cur->fpilen++;
      fpi.fpush = offset(pc);
      fpi.stkmin = cur->stklen;
    }
  }

  if (isMemberBaseOp(op)) {
    cur->mbr_live = true;
    if (op == Op::BaseGC || op == Op::BaseGL || op == Op::BaseL)  {
      auto new_pc = pc;
      decode_op(new_pc);
      decode_iva(new_pc);
      cur->mbr_mode = decode_oa<MOpMode>(new_pc);
    }
  } else if (isMemberFinalOp(op)) {
    cur->mbr_live = false;
    cur->mbr_mode.clear();
  }

  if (cur->fpilen > 0 && (op == Op::RetC || op == Op::RetCSuspended ||
                          op == Op::RetM || op == Op::Unwind)) {
    error("%s instruction encountered inside of FPI region\n",
          opcodeToName(op));
    ok = false;
  }

  return ok;
}

bool FuncChecker::checkRxOp(State* cur, PC pc, Op op) {
  switch (op) {
    // nops
    case Op::Nop:
    case Op::EntryNop:
    case Op::BreakTraceHint:
      return true;

    // flavor safety
    case Op::CGetCUNop:
    case Op::UGetCUNop:
      return true;

    // literals
    case Op::Null:
    case Op::NullUninit:
    case Op::True:
    case Op::False:
    case Op::Int:
    case Op::Double:
    case Op::String:
    case Op::Array:
    case Op::Dict:
    case Op::Keyset:
    case Op::Vec:
    case Op::NewArray:
    case Op::NewMixedArray:
    case Op::NewDictArray:
    case Op::NewLikeArrayL:
    case Op::NewPackedArray:
    case Op::NewRecord:
    case Op::NewStructArray:
    case Op::NewStructDArray:
    case Op::NewStructDict:
    case Op::NewVecArray:
    case Op::NewKeysetArray:
    case Op::NewVArray:
    case Op::NewDArray:
    case Op::AddElemC:
    case Op::AddNewElemC:
    case Op::NewCol:
    case Op::NewPair:
    case Op::ColFromArray:
      return true;

    // constants
    case Op::CnsE:
    case Op::ClsCns:
    case Op::ClsCnsD:
    case Op::File:
    case Op::Dir:
    case Op::Method:
      return true;

    // basic operations
    case Op::Concat:
    case Op::ConcatN:
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::AddO:
    case Op::SubO:
    case Op::MulO:
    case Op::Div:
    case Op::Mod:
    case Op::Pow:
    case Op::Xor:
    case Op::Not:
    case Op::Same:
    case Op::NSame:
    case Op::Eq:
    case Op::Neq:
    case Op::Lt:
    case Op::Lte:
    case Op::Gt:
    case Op::Gte:
    case Op::Cmp:
    case Op::BitAnd:
    case Op::BitOr:
    case Op::BitXor:
    case Op::BitNot:
    case Op::Shl:
    case Op::Shr:
      return true;

    // type transformations
    case Op::CastBool:
    case Op::CastInt:
    case Op::CastDouble:
    case Op::CastString:
    case Op::CastArray:
    case Op::CastObject:
    case Op::CastDict:
    case Op::CastKeyset:
    case Op::CastVec:
    case Op::CastVArray:
    case Op::CastDArray:
    case Op::DblAsBits:

    // type tests and assertions
    case Op::IsTypeC:
    case Op::IsTypeL:
    case Op::InstanceOf:
    case Op::InstanceOfD:
    case Op::IsLateBoundCls:
    case Op::IsTypeStructC:
    case Op::AsTypeStructC:
    case Op::CombineAndResolveTypeStruct:
    case Op::RecordReifiedGeneric:
    case Op::ReifiedName:
    case Op::CheckReifiedGenericMismatch:
    case Op::VerifyOutType:
    case Op::VerifyParamType:
    case Op::VerifyParamTypeTS:
    case Op::VerifyRetTypeC:
    case Op::VerifyRetTypeTS:
    case Op::VerifyRetNonNullC:
    case Op::AssertRATL:
    case Op::AssertRATStk:
      return true;

    // movs and mov-like things
    case Op::Dup:
    case Op::Select:
    case Op::PopC:
    case Op::PopU:
    case Op::PopU2:
    case Op::PopL:
    case Op::CGetL:
    case Op::CGetQuietL:
    case Op::CUGetL:
    case Op::CGetL2:
    case Op::PushL:
    case Op::IssetL:
    case Op::EmptyL:
    case Op::SetL:
    case Op::SetOpL:
    case Op::IncDecL:
    case Op::UnsetL:
    case Op::AKExists:
    case Op::Idx:
    case Op::ArrayIdx:
      return true;

    // this
    case Op::This:
    case Op::BareThis:
    case Op::CheckThis:
    case Op::InitThisLoc:
      return true;

    // classref and related
    case Op::Self:
    case Op::Parent:
    case Op::LateBoundCls:
    case Op::ClsRefGetC:
    case Op::ClsRefGetTS:
    case Op::DiscardClsRef:
    case Op::ClsRefName:
    case Op::OODeclExists:
      return true;

    // control flow
    case Op::Jmp:
    case Op::JmpNS:
    case Op::JmpZ:
    case Op::JmpNZ:
    case Op::Switch:
    case Op::SSwitch:
    case Op::RetC:
    case Op::RetCSuspended:
    case Op::RetM:
    case Op::Fatal:
    case Op::Throw:
    case Op::Catch:
    case Op::Unwind:
    case Op::ChainFaults:
      return true;

    // iteration (safe variants)
    case Op::IterInit:
    case Op::LIterInit:
    case Op::IterInitK:
    case Op::LIterInitK:
    case Op::IterNext:
    case Op::LIterNext:
    case Op::IterNextK:
    case Op::LIterNextK:
    case Op::IterFree:
    case Op::LIterFree:
    case Op::IterBreak:
      return true;

    // function calling
    case Op::FPushFunc:
    case Op::FPushFuncD:
    case Op::FPushFuncU:
    case Op::FPushObjMethod:
    case Op::FPushObjMethodD:
    case Op::FPushClsMethod:
    case Op::FPushClsMethodD:
    case Op::FPushClsMethodS:
    case Op::FPushClsMethodSD:
    case Op::FPushCtor:
    case Op::FCall:
    case Op::FCallBuiltin:
    case Op::NativeImpl:
    case Op::NewObj:
    case Op::NewObjD:
    case Op::NewObjS:
    case Op::ResolveFunc:
    case Op::ResolveObjMethod:
    case Op::ResolveClsMethod:
      return true;

    // closures and generators
    case Op::CreateCl:
    case Op::CreateCont:
    case Op::ContEnter:
    case Op::ContRaise:
    case Op::Yield:
    case Op::YieldK:
    case Op::ContCheck:
    case Op::ContValid:
    case Op::ContKey:
    case Op::ContCurrent:
    case Op::ContGetReturn:
      return true;

    // memoization wrappers
    case Op::GetMemoKeyL:
    case Op::MemoGet:
    case Op::MemoGetEager:
    case Op::MemoSet:
    case Op::MemoSetEager:
      return true;

    // awaitable special ops
    case Op::WHResult:
    case Op::Await:
    case Op::AwaitAll:
      return true;

    // safe member base operations
    case Op::BaseL:
    case Op::BaseH:
      // TODO(alexeyt): check mutability
      cur->mbrMustContainMutableLocalOrThis = true;
      cur->afterDim = false;
      return true;

    // more safe member base operations
    case Op::BaseC:
      cur->mbrMustContainMutableLocalOrThis = false;
      cur->afterDim = false;
      return true;

    // Dim is safe as long as we're not doing a write. Doing writes using [] is
    // safe. Doing writes using -> is only safe if the base refers to a mutable
    // local or mutable $this.
    case Op::Dim: {
      auto const mbr_contains_mutable = cur->mbrMustContainMutableLocalOrThis;
      cur->mbrMustContainMutableLocalOrThis = false;
      cur->afterDim = true;
      decode_op(pc);
      auto const mode = decode_oa<MOpMode>(pc);
      if (mode != MOpMode::Define && mode != MOpMode::Unset) return true;
      switch (decode_raw<MemberCode>(pc)) {
        case MEC:
        case MEL:
        case MET:
        case MEI:
        case MW:
          return true;
        case MPC:
        case MPL:
        case MPT:
        case MQT:
          if (mbr_contains_mutable) return true;
          break;
      }
      ferror("Dim through object for write is only allowed on locals and "
             "$this in Rx functions\n");
      return RuntimeOption::EvalRxVerifyBody < 2;
    }

    // member final read: QueryM is always safe
    case Op::QueryM:
      return true;

    // member final write: SetRangeM only operates on strings, always safe
    case Op::SetRangeM:
      return true;

    // member final write: prop write must not follow Dim, base must be mutable
    case Op::SetM:
    case Op::UnsetM:
    case Op::IncDecM:
    case Op::SetOpM:
    {
      decode_op(pc);
      decode_iva(pc);
      switch(op) {
        case Op::SetM:
        case Op::UnsetM:
          break;
        case Op::IncDecM:
          decode_oa<IncDecOp>(pc);
          break;
        case Op::SetOpM:
          decode_oa<SetOpOp>(pc);
          break;
        default:
          not_reached();
      }
      switch (decode_raw<MemberCode>(pc)) {
        case MEC:
        case MEL:
        case MET:
        case MEI:
        case MW:
          return true;
        case MPC:
        case MPL:
        case MPT:
        case MQT:
          break;
      }
      if (cur->afterDim) {
        ferror("writes to embedded objects are forbidden in Rx functions: {}\n",
               opcodeToName(op));
        return RuntimeOption::EvalRxVerifyBody < 2;
      }
      if (!cur->mbrMustContainMutableLocalOrThis) {
        ferror("property writes are only allowed on mutable values in Rx "
               "functions: {}\n", opcodeToName(op));
        return RuntimeOption::EvalRxVerifyBody < 2;
      }
      return true;
    }

    // undesirable: func_num_args()
    case Op::FuncNumArgs:
      ferror("func_num_args() is forbidden in Rx functions\n");
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: operations definitely involving boxes
    case Op::PopV:
    case Op::Box:
    case Op::Unbox:
    case Op::AddElemV:
    case Op::AddNewElemV:
    case Op::VGetL:
    case Op::VGetG:
    case Op::VGetS:
    case Op::BindL:
    case Op::BindG:
    case Op::BindS:
    case Op::VGetM:
    case Op::BindM:
      ferror("references are forbidden in Rx functions: {}\n",
             opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: globals
    case Op::BaseGC:
    case Op::BaseGL:
      cur->mbrMustContainMutableLocalOrThis = false;
      cur->afterDim = false;
      // fallthrough
    case Op::CGetG:
    case Op::CGetQuietG:
    case Op::IssetG:
    case Op::EmptyG:
    case Op::SetG:
    case Op::SetOpG:
    case Op::IncDecG:
    case Op::UnsetG:
      ferror("globals are forbidden in Rx functions: {}\n", opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: class statics
    case Op::BaseSC:
      cur->mbrMustContainMutableLocalOrThis = false;
      cur->afterDim = false;
      // fallthrough
    case Op::CGetS:
    case Op::IssetS:
    case Op::EmptyS:
    case Op::SetS:
    case Op::SetOpS:
    case Op::IncDecS:
      ferror("statics are forbidden in Rx functions: {}\n", opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: static locals
    case Op::StaticLocCheck:
    case Op::StaticLocDef:
    case Op::StaticLocInit:
      ferror("static locals are forbidden in Rx functions: {}\n",
             opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: defines and includes
    case Op::DefCls:
    case Op::DefClsNop:
    case Op::DefRecord:
    case Op::AliasCls:
    case Op::DefCns:
    case Op::DefTypeAlias:
    case Op::Incl:
    case Op::InclOnce:
    case Op::Req:
    case Op::ReqOnce:
    case Op::ReqDoc:
      ferror("defines/includes are forbidden in Rx functions: {}\n",
             opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: generator delegation (yield from)
    case Op::ContAssignDelegate:
    case Op::ContEnterDelegate:
    case Op::YieldFromDelegate:
    case Op::ContUnsetDelegate:
      ferror("`yield from` is forbidden in Rx functions: {}\n",
             opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;

    // unsafe: misc
    case Op::CheckProp:
    case Op::InitProp:
    case Op::Clone: // only unsafe because of __clone, we may revisit this
    case Op::Exit:
    case Op::Eval:
    case Op::Print:
    case Op::Silence:
      ferror("{} cannot appear in Rx function\n", opcodeToName(op));
      return RuntimeOption::EvalRxVerifyBody < 2;
  }
  not_reached();
}

std::string FuncChecker::stkToString(int len, const FlavorDesc* args) const {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(args[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::sigToString(int len, const FlavorDesc* sig) const {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(sig[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::iterToString(const State& cur) const {
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

std::string
FuncChecker::slotsToString(const boost::dynamic_bitset<>& slots) const {
  std::ostringstream oss;
  oss << slots;
  return oss.str();
}

std::string FuncChecker::stateToString(const State& cur) const {
  return folly::sformat(
    "{}{}[{}][{}]",
    iterToString(cur),
    stkToString(cur.stklen, cur.stk),
    slotsToString(cur.clsRefSlots),
    slotsToString(cur.writtenByClsRefGetTSSlots)
  );
}

std::string FuncChecker::fpiToString(const FpiState& fpi) const {
  std::stringstream out;
  out << '(' << fpi.fpush << ':' << fpi.stkmin << ')';
  return out.str();
}

void FuncChecker::initState(State* s) {
  s->stk = new (m_arena) FlavorDesc[maxStack()];
  s->fpi = new (m_arena) FpiState[maxFpi()];
  s->iters = new (m_arena) bool[numIters()];
  for (int i = 0, n = numIters(); i < n; ++i) s->iters[i] = false;
  s->clsRefSlots.resize(numClsRefSlots());
  s->writtenByClsRefGetTSSlots.resize(numClsRefSlots());
  s->stklen = 0;
  s->fpilen = 0;
  s->mbr_live = false;
  s->mbr_mode.clear();
  s->silences.clear();
  s->guaranteedThis =
    !m_func->isClosureBody && (m_func->attrs & AttrRequiresThis);
  s->mbrMustContainMutableLocalOrThis = false;
  s->afterDim = false;
}

void FuncChecker::copyState(State* to, const State* from) {
  assertx(from->stk);
  if (!to->stk) initState(to);
  memcpy(to->stk, from->stk, from->stklen * sizeof(*to->stk));
  memcpy(to->fpi, from->fpi, from->fpilen * sizeof(*to->fpi));
  memcpy(to->iters, from->iters, numIters() * sizeof(*to->iters));
  to->clsRefSlots = from->clsRefSlots;
  to->writtenByClsRefGetTSSlots = from->writtenByClsRefGetTSSlots;
  to->stklen = from->stklen;
  to->fpilen = from->fpilen;
  to->mbr_live = from->mbr_live;
  to->mbr_mode = from->mbr_mode;
  to->silences = from->silences;
  to->guaranteedThis = from->guaranteedThis;
  to->mbrMustContainMutableLocalOrThis = from->mbrMustContainMutableLocalOrThis;
  to->afterDim = from->afterDim;
}

bool FuncChecker::checkExnEdge(State cur, Block* b) {
  // Reachable catch blocks and fault funclets have an empty stack and
  // non-initialized class-ref slots. Checking an edge to the fault
  // handler right before every instruction is unnecessary since
  // not every instruction can throw; there is room for improvement
  // here if we want to note in the bytecode table which instructions
  // can actually throw to the fault handler.
  int save_stklen = cur.stklen;
  int save_fpilen = cur.fpilen;
  auto save_slots = cur.clsRefSlots;
  auto save_slots_clsrefgetts = cur.writtenByClsRefGetTSSlots;
  cur.stklen = 0;
  cur.fpilen = 0;
  cur.clsRefSlots.reset();
  cur.writtenByClsRefGetTSSlots.reset();
  auto const ok = checkEdge(b, cur, b->exn);
  cur.stklen = save_stklen;
  cur.fpilen = save_fpilen;
  cur.clsRefSlots = std::move(save_slots);
  cur.writtenByClsRefGetTSSlots = std::move(save_slots_clsrefgetts);
  return ok;
}

bool FuncChecker::checkBlock(State& cur, Block* b) {
  bool ok = true;
  auto const verify_rx = (RuntimeOption::EvalRxVerifyBody > 0) &&
    funcAttrIsAnyRx(m_func->attrs) && !m_func->isRxDisabled;
  if (m_errmode == kVerbose) {
    std::cout << blockToString(b, m_graph, unit()) << std::endl;
  }
  for (InstrRange i = blockInstrs(b); !i.empty(); ) {
    PC pc = i.popFront();
    if (m_errmode == kVerbose) {
      std::cout << "  " << std::setw(5) << offset(pc) << ":" <<
                   stateToString(cur) << " " <<
                   instrToString(pc, unit()) << std::endl;
    }
    auto const op = peek_op(pc);

    if (b->exn) ok &= checkExnEdge(cur, b);
    if (isMemberFinalOp(op)) ok &= checkMemberKey(&cur, pc, op);
    ok &= checkOp(&cur, pc, op, b);
    ok &= checkInputs(&cur, pc, b);
    auto const flags = instrFlags(op);
    if (flags & TF) ok &= checkTerminal(&cur, pc);
    if (op == Op::FCall) ok &= checkFCall(&cur, pc);
    if (isIter(pc)) ok &= checkIter(&cur, pc);
    if (op == Op::IterBreak) ok &= checkIterBreak(&cur, pc);
    ok &= checkClsRefSlots(&cur, pc);
    ok &= checkOutputs(&cur, pc, b);
    if (verify_rx) ok &= checkRxOp(&cur, pc, op);
  }
  ok &= checkSuccEdges(b, &cur);
  return ok;
}

/**
 * Verify stack depth along all control flow paths
 */
bool FuncChecker::checkFlow() {
  bool ok = true;
  GraphBuilder builder(m_arena, m_func);
  m_graph = builder.build();
  m_info = new (m_arena) BlockInfo[m_graph->block_count];
  m_tmp_sig = new (m_arena) FlavorDesc[maxStack()];
  sortRpo(m_graph);
  State cur;
  initState(&cur);
  // initialize state at all entry points
  for (BlockPtrRange i = entryBlocks(m_graph); !i.empty();) {
    ok &= checkEdge(0, cur, i.popFront());
  }
  for (Block* b = m_graph->first_rpo; b; b = b->next_rpo) {
    m_last_rpo_id = b->rpo_id;
    copyState(&cur, &m_info[b->id].state_in);
    ok &= checkBlock(cur, b);
  }
  // Make sure eval stack is correct at start of each try region
  for (auto& handler : m_func->ehtab) {
    if (handler.m_type == EHEnt::Type::Catch) {
      ok &= checkEHStack(handler, builder.at(handler.m_base));
    }
  }

  return ok;
}

bool FuncChecker::checkSuccEdges(Block* b, State* cur) {
  bool ok = true;
  int succs = numSuccBlocks(b);

  if (isIter(b->last) && succs == 2) {
    // IterInit* and IterNext*, Both implicitly free their iterator variable
    // on the loop-exit path.  Compute the iterator state on the "taken" path;
    // the fall-through path has the opposite state.
    int id = getImmIva(b->last);
    auto const last_op = peek_op(b->last);
    bool taken_state =
      (last_op == OpIterNext || last_op == OpIterNextK ||
       last_op == OpLIterNext || last_op == OpLIterNextK);
    bool save = cur->iters[id];
    cur->iters[id] = taken_state;
    if (m_errmode == kVerbose) {
      std::cout << "        " << stateToString(*cur) <<
        " -> B" << b->succs[1]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[1]);
    cur->iters[id] = !taken_state;
    if (m_errmode == kVerbose) {
      std::cout << "        " << stateToString(*cur) <<
                   " -> B" << b->succs[0]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[0]);
    cur->iters[id] = save;
  } else if (Op(*b->last) == Op::IterBreak) {
    auto pc = b->last + encoded_op_size(Op::IterBreak);
    decode_raw<Offset>(pc);
    for (auto const& iter : iterBreakIds(pc)) {
      cur->iters[iter.id] = false;
    }
    ok &= checkEdge(b, *cur, b->succs[0]);
  } else if (peek_op(b->last) == OpMemoGet && numSuccBlocks(b) == 2) {
    ok &= checkEdge(b, *cur, b->succs[0]);
    --cur->stklen;
    ok &= checkEdge(b, *cur, b->succs[1]);
  } else if (peek_op(b->last) == OpMemoGetEager && numSuccBlocks(b) == 3) {
    ok &= checkEdge(b, *cur, b->succs[0]);
    --cur->stklen;
    ok &= checkEdge(b, *cur, b->succs[1]);
    ++cur->stklen;
    ok &= checkEdge(b, *cur, b->succs[2]);
  } else {
    // Other branch instructions send the same state to all successors.
    if (m_errmode == kVerbose) {
      std::cout << "        " << stateToString(*cur) << std::endl;
    }
    for (BlockPtrRange i = succBlocks(b); !i.empty(); ) {
      ok &= checkEdge(b, *cur, i.popFront());
    }
  }

  for (int i = 0; i < succs; i++) {
    auto const t = b->succs[i];
    if (t && offset(t->start) == m_func->base) {
      boost::dynamic_bitset<> visited(m_graph->block_count);
      if (Block::reachable(t, b, visited)) {
        error("%s: Control flow cycles from the entry-block "
              "to itself are not allowed\n", opcodeToName(peek_op(b->last)));
        ok = false;
      }
    }
  }

  if (cur->mbr_live) {
    // MBR must not be live across control flow edges.
    error("Member base register live at end of B%d\n", b->id);
    ok = false;
  }

  if (succs == 0 && cur->silences.find_first() != cur->silences.npos &&
      !b->exn) {
    error("Error reporting was silenced at end of terminal block B%d\n", b->id);
    return false;
  }

  return ok;
}

bool FuncChecker::checkEHStack(const EHEnt& /*handler*/, Block* b) {
  const State& state = m_info[b->id].state_in;
  if (!state.stk) return true; // ignore unreachable block
  if (state.fpilen != 0) {
    error("EH region starts with non-empty FPI stack at B%d\n", b->id);
    return false;
  }
  return true;
}

/**
 * Check the edge b->t, given the current state at the end of b.
 * If this is the first edge ->t we've seen, copy the state to t.
 * Otherwise, unify the state and require the nonunifiable parts
 * to exactly match.
 */
bool FuncChecker::checkEdge(Block* b, const State& cur, Block *t) {
  State& state = m_info[t->id].state_in;
  bool stateChange = false;
  if (!state.stk) {
    copyState(&state, &cur);
    return true;
  }

  // An empty bitset should be considered equivalent to a bitset of all 0s
  if (cur.silences.size() != state.silences.size()) {
    state.silences.resize(cur.silences.size());
  }

  // Check silence state
  if (cur.silences != state.silences) {
    std::string current, target;
    boost::to_string(cur.silences, current);
    boost::to_string(state.silences, target);
    error("Silencer state mismatch on edge B%d->B%d: B%d had state %s, "
          "B%d had state %s\n", b->id, t->id, b->id, current.c_str(),
          t->id, target.c_str());
    return false;
  }

  // Conservatively propagate guarantees about $this
  if (state.guaranteedThis && !cur.guaranteedThis) {
    stateChange = true;
    state.guaranteedThis = false;
  }

  // Check stack.
  if (state.stklen != cur.stklen) {
    reportStkMismatch(b, t, cur);
    return false;
  }
  for (int i = 0, n = cur.stklen; i < n; i++) {
    if (state.stk[i] != cur.stk[i]) {
      // Allow C and V to unify into C|V
      if ((state.stk[i] == CV || state.stk[i] == VV || state.stk[i] == CVV) &&
          (cur.stk[i] == CV || cur.stk[i] == VV || cur.stk[i] == CVV)) {
        stateChange = true;
        state.stk[i] = CVV;
      } else {
        error("mismatch on edge B%d->B%d, current %s target %s\n",
               b->id, t->id, stkToString(n, cur.stk).c_str(),
               stkToString(n, state.stk).c_str());
        return false;
      }
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

  // Check class-ref slot state.
  if (state.clsRefSlots != cur.clsRefSlots ||
      state.writtenByClsRefGetTSSlots != cur.writtenByClsRefGetTSSlots) {
    ferror(
      "mismatched class-ref state on edge B{}->B{}, "
      "current [{}][{}] target [{}][{}]\n",
      b->id, t->id,
      slotsToString(cur.clsRefSlots),
      slotsToString(cur.writtenByClsRefGetTSSlots),
      slotsToString(state.clsRefSlots),
      slotsToString(state.writtenByClsRefGetTSSlots)
    );
    return false;
  }

  // t's state has changed, but we've already visited it, so we need to visit
  // it again. This is guaranteed to terminate because we only allow monotonic
  // state changes
  if (m_last_rpo_id > t->rpo_id && stateChange) {
    State tmp;
    copyState(&tmp, &state);
    return checkBlock(tmp, t);
  }

  return true;
}

void FuncChecker::reportStkUnderflow(Block*, const State& cur, PC pc) {
  int min = cur.fpilen > 0 ? cur.fpi[cur.fpilen - 1].stkmin : 0;
  error("Rule2: Stack underflow at PC %d, min depth %d\n",
         offset(pc), min);
}

void FuncChecker::reportStkOverflow(Block*, const State& /*cur*/, PC pc) {
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
  assertx(past >= base);
  if (off < base || off >= past) {
    error("Offset %s %d is outside region %s %d:%d\n",
           name, off, regionName, base, past);
    return false;
  }
  if (check_instrs && !m_instrs.get(off - m_func->base)) {
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
  assertx(past >= base);
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
             (!m_instrs.get(b - m_func->base) ||
              (p < past && !m_instrs.get(p - m_func->base)))) {
    error("region %s %d:%d boundaries are inbetween instructions\n",
           name, b, p);
    return false;
  }
  return true;
}

}} // HPHP::Verifier
