/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/vm/translator/hopt/ir.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>
#include <boost/algorithm/string.hpp>

#include "folly/Format.h"
#include "folly/Traits.h"

#include "util/disasm.h"
#include "util/trace.h"
#include "util/text_color.h"
#include "runtime/base/string_data.h"
#include "runtime/vm/runtime.h"
#include "runtime/base/stats.h"
#include "runtime/vm/translator/hopt/irfactory.h"
#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/cse.h"
#include "runtime/vm/translator/hopt/simplifier.h"

// Include last to localize effects to this file
#include "util/assert_throw.h"

namespace HPHP { namespace VM { namespace JIT {

using namespace HPHP::VM::Transl;

#define IRT(name, ...) const Type Type::name(Type::k##name);
IR_TYPES
#undef IRT

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
  IR_TYPES
# undef IRT

  // Concat all of the primitive types in the custom union type
  std::vector<std::string> types;
# define IRT(name, ...) if (name.subtypeOf(*this)) types.push_back(#name);
  IRT_PRIMITIVE
# undef IRT
  return folly::format("{{{}}}", folly::join('|', types)).str();
}

std::string Type::debugString(Type t) {
  return t.toString();
}

Type Type::fromString(const std::string& str) {
  static hphp_string_map<Type> types;
  static bool init = false;
  if (UNLIKELY(!init)) {
#   define IRT(name, ...) types[#name] = name;
    IR_TYPES
#   undef IRT
    init = true;
  }
  return mapGet(types, str, Type::None);
}

TRACE_SET_MOD(hhir);

namespace {

#define NF     0
#define C      CanCSE
#define E      Essential
#define N      CallsNative
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define Refs   MayModifyRefs
#define Rm     Rematerializable
#define Er     MayRaiseError
#define Mem    MemEffects
#define T      Terminal
#define P      Passthrough
#define K      KillsSources
#define StkFlags(f) HasStackVersion|(f)
#define VProp  VectorProp
#define VElem  VectorElem

#define ND        0
#define D(n)      HasDest
#define DofS(n)   HasDest
#define DUnbox(n) HasDest
#define DBox(n)   HasDest
#define DParam    HasDest
#define DMulti    NaryDest
#define DVector   HasDest
#define DStk(x)   ModifiesStack|(x)
#define DPtrToParam HasDest
#define DBuiltin  HasDest

struct {
  const char* name;
  uint64_t flags;
} OpInfo[] = {
#define O(name, dsts, srcs, flags)                    \
    { #name,                                          \
       (OpHasExtraData<name>::value ? HasExtra : 0) | \
       dsts | (flags)                                 \
    },
  IR_OPCODES
#undef O
  { 0 }
};

#undef NF
#undef C
#undef E
#undef PRc
#undef CRc
#undef Refs
#undef Rm
#undef Er
#undef Mem
#undef T
#undef P
#undef K
#undef StkFlags
#undef VProp
#undef VElem

#undef ND
#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DParam
#undef DMulti
#undef DVector
#undef DStk
#undef DPtrToParam
#undef DBuiltin

//////////////////////////////////////////////////////////////////////

/*
 * dispatchExtra translates from runtime values for the Opcode enum
 * into compile time types.  The goal is to call a `targetFunction'
 * that is overloaded on the extra data type structs.
 *
 * The purpose of the MAKE_DISPATCHER layer is to weed out Opcode
 * values that have no associated extra data.
 *
 * Basically this is doing dynamic dispatch without a vtable in
 * IRExtraData, instead using the Opcode tag from the associated
 * instruction to discriminate the runtime type.
 *
 * Note: functions made with this currently only make sense to call if
 * it's already known that the opcode has extra data.  If you call it
 * for one that doesn't, you'll get an abort.  Generally hasExtra()
 * should be checked first.
 */

#define MAKE_DISPATCHER(name, rettype, targetFunction)                \
  template<bool HasExtra, Opcode opc> struct name {                   \
    template<class... Args>                                           \
    static rettype go(IRExtraData* vp, Args&&...) { not_reached(); }  \
  };                                                                  \
  template<Opcode opc> struct name<true,opc> {                        \
    template<class... Args>                                           \
    static rettype go(IRExtraData* vp, Args&&... args) {              \
      return targetFunction(                                          \
        static_cast<typename IRExtraDataType<opc>::type*>(vp),        \
        std::forward<Args>(args)...                                   \
      );                                                              \
    }                                                                 \
  };

template<
  class RetType,
  template<bool, Opcode> class Dispatcher,
  class... Args
>
RetType dispatchExtra(Opcode opc, IRExtraData* data, Args&&... args) {
#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode:                                  \
    return Dispatcher<                          \
      OpHasExtraData<opcode>::value,            \
      opcode                                    \
    >::go(data, std::forward<Args>(args)...);
  switch (opc) { IR_OPCODES default: not_reached(); }
#undef O
  not_reached();
}

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_cseHash,   cseHash);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_cseEquals, cseEquals);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_clone,     clone);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_show,      show);

template<class T>
typename std::enable_if<
  has_cseHash<T,size_t () const>::value,
  size_t
>::type cseHashExtraImpl(T* t) { return t->cseHash(); }
size_t cseHashExtraImpl(IRExtraData*) {
  // This probably means an instruction was marked CanCSE but its
  // extra data had no hash function.
  always_assert(!"attempted to hash extra data that didn't "
    "provide a hash function");
}

template<class T>
typename std::enable_if<
  has_cseEquals<T,bool (T const&) const>::value ||
  has_cseEquals<T,bool (T)        const>::value,
  bool
>::type cseEqualsExtraImpl(T* t, IRExtraData* o) {
  return t->cseEquals(*static_cast<T*>(o));
}
bool cseEqualsExtraImpl(IRExtraData*, IRExtraData*) {
  // This probably means an instruction was marked CanCSE but its
  // extra data had no equals function.
  always_assert(!"attempted to compare extra data that didn't "
                 "provide an equals function");
}

// Clone using a data-specific clone function.
template<class T>
typename std::enable_if<
  has_clone<T,T* (Arena&) const>::value,
  T*
>::type cloneExtraImpl(T* t, Arena& arena) {
  return t->clone(arena);
}

// Use the copy constructor if no clone() function was supplied.
template<class T>
typename std::enable_if<
  !has_clone<T,T* (Arena&) const>::value,
  T*
>::type cloneExtraImpl(T* t, Arena& arena) {
  return new (arena) T(*t);
}

template<class T>
typename std::enable_if<
  has_show<T,std::string () const>::value,
  std::string
>::type showExtraImpl(T* t) { return t->show(); }
std::string showExtraImpl(IRExtraData*) { return "..."; }

MAKE_DISPATCHER(HashDispatcher, size_t, cseHashExtraImpl);
size_t cseHashExtra(Opcode opc, IRExtraData* data) {
  return dispatchExtra<size_t,HashDispatcher>(opc, data);
}

MAKE_DISPATCHER(EqualsDispatcher, bool, cseEqualsExtraImpl);
bool cseEqualsExtra(Opcode opc, IRExtraData* data, IRExtraData* other) {
  return dispatchExtra<bool,EqualsDispatcher>(opc, data, other);
}

MAKE_DISPATCHER(CloneDispatcher, IRExtraData*, cloneExtraImpl);
IRExtraData* cloneExtra(Opcode opc, IRExtraData* data, Arena& a) {
  return dispatchExtra<IRExtraData*,CloneDispatcher>(opc, data, a);
}

MAKE_DISPATCHER(ShowDispatcher, std::string, showExtraImpl);
std::string showExtra(Opcode opc, IRExtraData* data) {
  return dispatchExtra<std::string,ShowDispatcher>(opc, data);
}

//////////////////////////////////////////////////////////////////////

// Helper for pretty-printing punctuation.
std::string punc(const char* str) {
  return folly::format("{}{}{}",
    color(ANSI_COLOR_DARK_GRAY), str, color(ANSI_COLOR_END)).str();
}

//////////////////////////////////////////////////////////////////////

}

IRInstruction::IRInstruction(Arena& arena, const IRInstruction* inst, IId iid)
  : m_op(inst->m_op)
  , m_typeParam(inst->m_typeParam)
  , m_numSrcs(inst->m_numSrcs)
  , m_numDsts(inst->m_numDsts)
  , m_iid(iid)
  , m_id(0)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dst(nullptr)
  , m_taken(nullptr)
  , m_block(nullptr)
  , m_tca(nullptr)
  , m_extra(inst->m_extra ? cloneExtra(op(), inst->m_extra, arena)
                          : nullptr)
{
  std::copy(inst->m_srcs, inst->m_srcs + inst->m_numSrcs, m_srcs);
  setTaken(inst->m_taken);
}

const char* opcodeName(Opcode opcode) { return OpInfo[opcode].name; }

bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return OpInfo[opcode].flags & flags;
}

Opcode getStackModifyingOpcode(Opcode opc) {
  assert(opcodeHasFlags(opc, HasStackVersion));
  opc = Opcode(opc + 1);
  assert(opcodeHasFlags(opc, ModifiesStack));
  return opc;
}

bool IRInstruction::hasExtra() const {
  return opcodeHasFlags(op(), HasExtra) && m_extra;
}

// Instructions with ModifiesStack are always naryDst regardless of
// the inner dest.

bool IRInstruction::hasDst() const {
  return opcodeHasFlags(op(), HasDest) &&
    !opcodeHasFlags(op(), ModifiesStack);
}

bool IRInstruction::naryDst() const {
  return opcodeHasFlags(op(), NaryDest | ModifiesStack);
}

bool IRInstruction::isNative() const {
  return opcodeHasFlags(op(), CallsNative);
}

bool IRInstruction::producesReference() const {
  return opcodeHasFlags(op(), ProducesRC);
}

bool IRInstruction::isRematerializable() const {
  return opcodeHasFlags(op(), Rematerializable);
}

bool IRInstruction::hasMemEffects() const {
  return opcodeHasFlags(op(), MemEffects) || mayReenterHelper();
}

bool IRInstruction::canCSE() const {
  auto canCSE = opcodeHasFlags(op(), CanCSE);
  // Make sure that instructions that are CSE'able can't produce a reference
  // count or consume reference counts. GuardType is special because it can
  // refine a maybeCounted type to a notCounted type, so it logically consumes
  // and produces a reference without doing any work.
  assert(!canCSE || !producesReference() || m_op == GuardType);
  assert(!canCSE || !consumesReferences() || m_op == GuardType);
  return canCSE && !mayReenterHelper();
}

bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(op(), ConsumesRC);
}

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }
  // GuardType consumes a reference if we're guarding from a maybeCounted type
  // to a notCounted type.
  if (m_op == GuardType) {
    assert(srcNo == 0);
    return getSrc(0)->type().maybeCounted() && getTypeParam().notCounted();
  }
  // SpillStack consumes inputs 2 and onward
  if (m_op == SpillStack) return srcNo >= 2;
  // Call consumes inputs 3 and onward
  if (m_op == Call) return srcNo >= 3;
  // RetVal only consumes input 1
  if (m_op == RetVal) return srcNo == 1;

  if (m_op == StLoc || m_op == StLocNT) {
    // StLoc[NT] <stkptr>, <value>
    return srcNo == 1;
  }
  if (m_op == StProp || m_op == StPropNT || m_op == StMem || m_op == StMemNT) {
    // StProp[NT]|StMem[NT] <base>, <offset>, <value>
    return srcNo == 2;
  }
  if (m_op == ArraySet || m_op == ArraySetRef) {
    // Only consumes the reference to its input array
    return srcNo == 1;
  }
  return true;
}

bool IRInstruction::mayModifyRefs() const {
  Opcode opc = op();
  // DecRefNZ does not have side effects other than decrementing the ref
  // count. Therefore, its MayModifyRefs should be false.
  if (opc == DecRef) {
    auto type = getSrc(0)->type();
    if (isControlFlowInstruction()) {
      // If the decref has a target label, then it exits if the destructor
      // has to be called, so it does not have any side effects on the main
      // trace.
      return false;
    }
    if (!type.canRunDtor()) {
      return false;
    }
  }
  return opcodeHasFlags(opc, MayModifyRefs) || mayReenterHelper();
}

bool IRInstruction::mayRaiseError() const {
  return opcodeHasFlags(op(), MayRaiseError) || mayReenterHelper();
}

bool IRInstruction::isEssential() const {
  Opcode opc = op();
  if (opc == DecRefNZ) {
    // If the source of a DecRefNZ is not an IncRef, mark it as essential
    // because we won't remove its source as well as itself.
    // If the ref count optimization is turned off, mark all DecRefNZ as
    // essential.
    if (!RuntimeOption::EvalHHIREnableRefCountOpt ||
        getSrc(0)->inst()->op() != IncRef) {
      return true;
    }
  }
  return isControlFlowInstruction() ||
         opcodeHasFlags(opc, Essential) ||
         mayReenterHelper();
}

bool IRInstruction::isTerminal() const {
  return opcodeHasFlags(op(), Terminal);
}

bool IRInstruction::isPassthrough() const {
  return opcodeHasFlags(op(), Passthrough);
}

SSATmp* IRInstruction::getPassthroughValue() const {
  assert(isPassthrough());
  assert(m_op == IncRef || m_op == GuardType || m_op == Mov);
  return getSrc(0);
}

bool IRInstruction::killsSources() const {
  return opcodeHasFlags(op(), KillsSources);
}

bool IRInstruction::killsSource(int idx) const {
  if (!killsSources()) return false;
  switch (m_op) {
    case DecRef:
    case ConvObjToArr:
    case ConvCellToArr:
    case ConvCellToBool:
    case ConvObjToDbl:
    case ConvStrToDbl:
    case ConvCellToDbl:
    case ConvObjToInt:
    case ConvCellToInt:
    case ConvCellToObj:
    case ConvObjToStr:
    case ConvCellToStr:
      assert(idx == 0);
      return true;
    case ArraySet:
    case ArraySetRef:
      return idx == 1;
    case DecRefKillThis:
      return idx == 0;
    default:
      not_reached();
      break;
  }
}

bool IRInstruction::modifiesStack() const {
  return opcodeHasFlags(op(), ModifiesStack);
}

SSATmp* IRInstruction::modifiedStkPtr() const {
  assert(modifiesStack());
  assert(VectorEffects::supported(this));
  SSATmp* sp = getDst(hasMainDst() ? 1 : 0);
  assert(sp->isA(Type::StkPtr));
  return sp;
}

bool IRInstruction::hasMainDst() const {
  return opcodeHasFlags(op(), HasDest);
}

bool IRInstruction::mayReenterHelper() const {
  if (isCmpOp(op())) {
    return cmpOpTypesMayReenter(op(),
                                getSrc(0)->type(),
                                getSrc(1)->type());
  }
  // Not necessarily actually false; this is just a helper for other
  // bits.
  return false;
}

SSATmp* IRInstruction::getDst(unsigned i) const {
  if (i == 0 && m_numDsts == 0) return nullptr;
  assert(i < m_numDsts);
  assert(naryDst() || i == 0);
  return hasDst() ? getDst() : &m_dst[i];
}

DstRange IRInstruction::getDsts() {
  return Range<SSATmp*>(m_dst, m_numDsts);
}

Range<const SSATmp*> IRInstruction::getDsts() const {
  return Range<const SSATmp*>(m_dst, m_numDsts);
}

Opcode negateQueryOp(Opcode opc) {
  assert(isQueryOp(opc));

  switch (opc) {
  case OpGt:                return OpLte;
  case OpGte:               return OpLt;
  case OpLt:                return OpGte;
  case OpLte:               return OpGt;
  case OpEq:                return OpNeq;
  case OpNeq:               return OpEq;
  case OpSame:              return OpNSame;
  case OpNSame:             return OpSame;
  case InstanceOf:          return NInstanceOf;
  case NInstanceOf:         return InstanceOf;
  case InstanceOfBitmask:   return NInstanceOfBitmask;
  case NInstanceOfBitmask:  return InstanceOfBitmask;
  case IsType:              return IsNType;
  case IsNType:             return IsType;
  default:                  always_assert(0);
  }
}

Opcode queryCommuteTable[] = {
  OpLt,         // OpGt
  OpLte,        // OpGte
  OpGt,         // OpLt
  OpGte,        // OpLte
  OpEq,         // OpEq
  OpNeq,        // OpNeq
  OpSame,       // OpSame
  OpNSame       // OpNSame
};

// Objects compared with strings may involve calling a user-defined
// __toString function.
bool cmpOpTypesMayReenter(Opcode op, Type t0, Type t1) {
  if (op == OpNSame || op == OpSame) return false;
  assert(t0 != Type::Gen && t1 != Type::Gen);
  return (t0 == Type::Cell || t1 == Type::Cell) ||
    ((t0 == Type::Obj || t1 == Type::Obj) &&
     (t0.isString() || t1.isString()));
}

TraceExitType::ExitType getExitType(Opcode opc) {
  assert(opc >= ExitTrace && opc <= ExitGuardFailure);
  return (TraceExitType::ExitType)(opc - ExitTrace);
}

Opcode getExitOpcode(TraceExitType::ExitType type) {
  return (Opcode)(ExitTrace + type);
}

bool isRefCounted(SSATmp* tmp) {
  if (tmp->type().notCounted()) {
    return false;
  }
  IRInstruction* inst = tmp->inst();
  Opcode opc = inst->op();
  if (opc == DefConst || opc == LdConst || opc == LdClsCns) {
    return false;
  }
  return true;
}

void IRInstruction::convertToNop() {
  IRInstruction nop(Nop);
  // copy all but m_iid, m_block, m_taken, m_listNode
  m_op = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs = nop.m_numSrcs;
  m_id = nop.m_id;
  m_srcs = nop.m_srcs;
  m_liveRegs = nop.m_liveRegs;
  m_numDsts = nop.m_numDsts;
  m_dst = nop.m_dst;
  m_taken = nullptr;
  m_tca = nop.m_tca;
  m_extra = nullptr;
}

void IRInstruction::convertToJmp() {
  assert(isControlFlowInstruction());
  assert(getBlock()->back() == this);
  m_op = Jmp_;
  m_typeParam = Type::None;
  m_numSrcs = 0;
  m_numDsts = 0;
  m_srcs = nullptr;
  m_dst = nullptr;
  getBlock()->setNext(nullptr);
}

void IRInstruction::convertToMov() {
  assert(!isControlFlowInstruction());
  m_op = Mov;
  m_typeParam = Type::None;
  assert(m_numSrcs == 1);
  assert(m_numDsts == 1);
}

IRInstruction* IRInstruction::clone(IRFactory* factory) const {
  return factory->cloneInstruction(this);
}

SSATmp* IRInstruction::getSrc(uint32_t i) const {
  if (i >= getNumSrcs()) return nullptr;
  return m_srcs[i];
}

void IRInstruction::setSrc(uint32_t i, SSATmp* newSrc) {
  assert(i < getNumSrcs());
  m_srcs[i] = newSrc;
}

void IRInstruction::appendSrc(Arena& arena, SSATmp* newSrc) {
  auto newSrcs = new (arena) SSATmp*[getNumSrcs() + 1];
  std::copy(m_srcs, m_srcs + getNumSrcs(), newSrcs);
  newSrcs[getNumSrcs()] = newSrc;
  ++m_numSrcs;
  m_srcs = newSrcs;
}

void IRInstruction::setTaken(Block* target) {
  if (m_op == Jmp_ && m_extra) {
    if (m_taken) m_taken->removeEdge(this);
    if (target) target->addEdge(this);
  }
  m_taken = target;
}

void Block::addEdge(IRInstruction* jmp) {
  assert(!jmp->isTransient());
  EdgeData* node = jmp->getExtra<Jmp_>();
  node->jmp = jmp;
  node->next = m_preds;
  m_preds = node;
}

void Block::removeEdge(IRInstruction* jmp) {
  assert(!jmp->isTransient());
  EdgeData* node = jmp->getExtra<Jmp_>();
  assert(node->jmp == jmp);
  EdgeData** ptr = &m_preds;
  while (*ptr != node) ptr = &(*ptr)->next;
  *ptr = node->next;
  assert((node->next = nullptr, true));
}

bool IRInstruction::cseEquals(IRInstruction* inst) const {
  assert(canCSE());

  if (m_op != inst->m_op ||
      m_typeParam != inst->m_typeParam ||
      m_numSrcs != inst->m_numSrcs) {
    return false;
  }
  for (uint32_t i = 0; i < getNumSrcs(); i++) {
    if (getSrc(i) != inst->getSrc(i)) {
      return false;
    }
  }
  if (hasExtra() && !cseEqualsExtra(op(), m_extra, inst->m_extra)) {
    return false;
  }
  /*
   * Don't CSE on m_taken---it's ok to use the destination of some
   * earlier guarded load even though the instruction we may have
   * generated here would've exited to a different trace.
   *
   * For example, we use this to cse LdThis regardless of its label.
   */
  return true;
}

size_t IRInstruction::cseHash() const {
  assert(canCSE());

  size_t srcHash = 0;
  for (unsigned i = 0; i < getNumSrcs(); ++i) {
    srcHash = CSEHash::hashCombine(srcHash, getSrc(i));
  }
  if (hasExtra()) {
    srcHash = CSEHash::hashCombine(srcHash,
      cseHashExtra(op(), m_extra));
  }
  return CSEHash::hashCombine(srcHash, m_op, m_typeParam);
}

void IRInstruction::printOpcode(std::ostream& os) const {
  os << color(ANSI_COLOR_CYAN)
     << opcodeName(m_op)
     << color(ANSI_COLOR_END)
     ;

  if (m_typeParam == Type::None && !hasExtra()) {
    return;
  }
  os << color(ANSI_COLOR_LIGHT_BLUE) << '<' << color(ANSI_COLOR_END);
  if (m_typeParam != Type::None) {
    os << color(ANSI_COLOR_GREEN)
       << m_typeParam.toString()
       << color(ANSI_COLOR_END)
       ;
    if (hasExtra()) {
      os << punc(",");
    }
  }
  if (hasExtra()) {
    os << color(ANSI_COLOR_GREEN)
       << showExtra(op(), m_extra)
       << color(ANSI_COLOR_END);
  }
  os << color(ANSI_COLOR_LIGHT_BLUE)
     << '>'
     << color(ANSI_COLOR_END);
}

void IRInstruction::printDst(std::ostream& os) const {
  if (getNumDsts() == 0) return;

  const char* sep = "";
  for (const SSATmp& dst : getDsts()) {
    os << punc(sep);
    dst.print(os, true);
    sep = ", ";
  }
  os << punc(" = ");
}

void IRInstruction::printSrc(std::ostream& ostream, uint32_t i) const {
  SSATmp* src = getSrc(i);
  if (src != nullptr) {
    if (m_id != 0 && !src->isConst() && src->getLastUseId() == m_id) {
      ostream << "~";
    }
    src->print(ostream);
  } else {
    ostream << color(ANSI_COLOR_RED)
            << "!!!NULL @ " << i
            << color(ANSI_COLOR_END)
            ;
  }
}

void IRInstruction::printSrcs(std::ostream& os) const {
  bool first = true;
  if (op() == IncStat) {
    os << " " << Stats::g_counterNames[getSrc(0)->getValInt()]
       << ", " << getSrc(1)->getValInt();
    return;
  }
  for (uint32_t i = 0; i < m_numSrcs; i++) {
    if (!first) {
      os << punc(", ");
    } else {
      os << " ";
      first = false;
    }
    printSrc(os, i);
  }
}

void IRInstruction::print(std::ostream& ostream) const {
  if (op() == Marker) {
    auto* marker = getExtra<Marker>();
    ostream << color(ANSI_COLOR_BLUE)
            << folly::format("--- bc {}, spOff {} ({})",
                             marker->bcOff,
                             marker->stackOff,
                             marker->func->fullName()->data())
            << color(ANSI_COLOR_END);
    return;
  }

  if (!isTransient()) {
    ostream << color(ANSI_COLOR_YELLOW);
    if (!m_id) ostream << folly::format("({:02d}) ", getIId());
    else ostream << folly::format("({:02d}@{:02d}) ", getIId(), m_id);
    ostream << color(ANSI_COLOR_END);
  }
  printDst(ostream);
  printOpcode(ostream);
  printSrcs(ostream);

  if (m_taken) {
    ostream << punc(" -> ");
    m_taken->printLabel(ostream);
  }

  if (m_tca) {
    ostream << punc(", ");
    if (m_tca == kIRDirectJccJmpActive) {
      ostream << "JccJmp_Exit ";
    }
    else
    if (m_tca == kIRDirectJccActive) {
      ostream << "Jcc_Exit ";
    }
    else
    if (m_tca == kIRDirectGuardActive) {
      ostream << "Guard_Exit ";
    }
    else {
      ostream << (void*)m_tca;
    }
  }
}

void IRInstruction::print() const {
  print(std::cerr);
  std::cerr << std::endl;
}

std::string IRInstruction::toString() const {
  std::ostringstream str;
  print(str);
  return str.str();
}

static void printConst(std::ostream& os, IRInstruction* inst) {
  os << color(ANSI_COLOR_LIGHT_BLUE);
  SCOPE_EXIT { os << color(ANSI_COLOR_END); };

  auto t = inst->getTypeParam();
  auto c = inst->getExtra<DefConst>();
  if (t == Type::Int) {
    os << c->as<int64_t>();
  } else if (t == Type::Dbl) {
    os << c->as<double>();
  } else if (t == Type::Bool) {
    os << (c->as<bool>() ? "true" : "false");
  } else if (t.isString()) {
    auto str = c->as<const StringData*>();
    os << "\""
       << Util::escapeStringForCPP(str->data(), str->size())
       << "\"";
  } else if (t.isArray()) {
    auto arr = inst->getExtra<DefConst>()->as<const ArrayData*>();
    if (arr->empty()) {
      os << "array()";
    } else {
      os << "Array(" << arr << ")";
    }
  } else if (t.isNull()) {
    os << t.toString();
  } else if (t.subtypeOf(Type::Func)) {
    auto func = c->as<const Func*>();
    os << "Func(" << (func ? func->fullName()->data() : "0") << ")";
  } else if (t.subtypeOf(Type::Cls)) {
    auto cls = c->as<const Class*>();
    os << "Cls(" << (cls ? cls->name()->data() : "0") << ")";
  } else if (t.subtypeOf(Type::NamedEntity)) {
    auto ne = c->as<const NamedEntity*>();
    os << "NamedEntity(" << ne << ")";
  } else if (t.subtypeOf(Type::TCA)) {
    TCA tca = c->as<TCA>();
    char* nameRaw = Util::getNativeFunctionName(tca);
    SCOPE_EXIT { free(nameRaw); };
    std::string name(nameRaw);
    boost::algorithm::trim(name);
    os << folly::format("TCA: {}({})", tca, name);
  } else if (t.subtypeOf(Type::None)) {
    os << "None:" << c->as<int64_t>();
  } else if (t.isPtr()) {
    os << folly::format("{}({:#x})", t.toString(), c->as<uint64_t>());
  } else if (t.subtypeOf(Type::CacheHandle)) {
    os << folly::format("CacheHandle({:#x})", c->as<int64_t>());
  } else {
    not_reached();
  }
}

void Block::printLabel(std::ostream& os) const {
  os << color(ANSI_COLOR_MAGENTA);
  os << "L" << m_id;
  if (getHint() == Unlikely) {
    os << "<Unlikely>";
  }
  os << color(ANSI_COLOR_END);
}

int SSATmp::numNeededRegs() const {
  auto t = type();
  if (t.subtypeOfAny(Type::None, Type::Null, Type::ActRec, Type::RetAddr)) {
    // These don't need a register because their values are static or unused.
    //
    // RetAddr doesn't take any register because currently we only target x86,
    // which takes the return address from the stack.  This knowledge should be
    // moved to a machine-specific section once we target other architectures.
    return 0;
  }
  if (t.subtypeOf(Type::Ctx) || t.isPtr()) {
    // Ctx and PtrTo* may be statically unknown but always need just 1 register.
    return 1;
  }
  if (t.subtypeOf(Type::FuncCtx)) {
    // 2 registers regardless of union status: 1 for the Func* and 1
    // for the {Obj|Cctx}, differentiated by the low bit.
    return 2;
  }
  if (!t.isUnion()) {
    // Not a union type and not a special case: 1 register.
    assert(IMPLIES(t.subtypeOf(Type::Gen), t.isKnownDataType()));
    return 1;
  }

  assert(t.subtypeOf(Type::Gen));
  return t.needsReg() ? 2 : 1;
}

int SSATmp::numAllocatedRegs() const {
  // If an SSATmp is spilled, it must've actually had a full set of
  // registers allocated to it.
  if (m_isSpilled) return numNeededRegs();

  // Return the number of register slots that actually have an
  // allocated register.  We may not have allocated a full
  // numNeededRegs() worth of registers in some cases (if the value
  // of this tmp wasn't used, etc).
  int i = 0;
  while (i < kMaxNumRegs && m_regs[i] != InvalidReg) {
    ++i;
  }
  return i;
}

RegSet SSATmp::getRegs() const {
  RegSet regs;
  for (int i = 0, n = numAllocatedRegs(); i < n; ++i) {
    if (hasReg(i)) regs.add(getReg(i));
  }
  return regs;
}

bool SSATmp::getValBool() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Bool));
  return m_inst->getExtra<ConstData>()->as<bool>();
}

int64_t SSATmp::getValInt() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Int));
  return m_inst->getExtra<ConstData>()->as<int64_t>();
}

int64_t SSATmp::getValRawInt() const {
  assert(isConst());
  return m_inst->getExtra<ConstData>()->as<int64_t>();
}

double SSATmp::getValDbl() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Dbl));
  return m_inst->getExtra<ConstData>()->as<double>();
}

const StringData* SSATmp::getValStr() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::StaticStr));
  return m_inst->getExtra<ConstData>()->as<const StringData*>();
}

const ArrayData* SSATmp::getValArr() const {
  assert(isConst());
  // TODO: Task #2124292, Reintroduce StaticArr
  assert(m_inst->getTypeParam().subtypeOf(Type::Arr));
  return m_inst->getExtra<ConstData>()->as<const ArrayData*>();
}

const Func* SSATmp::getValFunc() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Func));
  return m_inst->getExtra<ConstData>()->as<const Func*>();
}

const Class* SSATmp::getValClass() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Cls));
  return m_inst->getExtra<ConstData>()->as<const Class*>();
}

const NamedEntity* SSATmp::getValNamedEntity() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::NamedEntity));
  return m_inst->getExtra<ConstData>()->as<const NamedEntity*>();
}

uintptr_t SSATmp::getValBits() const {
  assert(isConst());
  return m_inst->getExtra<ConstData>()->as<uintptr_t>();
}

Variant SSATmp::getValVariant() const {
  switch (m_inst->getTypeParam().toDataType()) {
  case KindOfUninit:
  case KindOfNull:
    return uninit_null();
  case KindOfBoolean:
    return m_inst->getExtra<ConstData>()->as<bool>();
  case KindOfInt64:
    return m_inst->getExtra<ConstData>()->as<int64_t>();
  case KindOfDouble:
    return m_inst->getExtra<ConstData>()->as<double>();
  case KindOfString:
  case KindOfStaticString:
    return (litstr)m_inst->getExtra<ConstData>()
        ->as<const StringData*>()->data();
  case KindOfArray:
    return Array(ArrayData::GetScalarArray(m_inst->getExtra<ConstData>()
      ->as<ArrayData*>()));
  case KindOfObject:
    return m_inst->getExtra<ConstData>()->as<const Object*>();
  default:
    assert(false);
    return uninit_null();
  }
}

TCA SSATmp::getValTCA() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::TCA));
  return m_inst->getExtra<ConstData>()->as<TCA>();
}

std::string ExitData::show() const {
  return folly::to<std::string>(toSmash->getIId());
}

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out);
  return out.str();
}

void SSATmp::print(std::ostream& os, bool printLastUse) const {
  if (m_inst->op() == DefConst) {
    printConst(os, m_inst);
    return;
  }
  os << color(ANSI_COLOR_WHITE);
  os << "t" << m_id;
  os << color(ANSI_COLOR_END);
  if (printLastUse && m_lastUseId != 0) {
    os << color(ANSI_COLOR_GRAY)
       << "@" << m_lastUseId << "#" << m_useCount
       << color(ANSI_COLOR_END);
  }
  if (m_isSpilled || numAllocatedRegs() > 0) {
    os << color(ANSI_COLOR_BROWN) << '(';
    if (!m_isSpilled) {
      for (int i = 0, sz = numAllocatedRegs(); i < sz; ++i) {
        if (i != 0) os << ",";
        os << reg::regname(Reg64(int(m_regs[i])));
      }
    } else {
      for (int i = 0, sz = numNeededRegs(); i < sz; ++i) {
        if (i != 0) os << ",";
        os << m_spillInfo[i];
      }
    }
    os << ')' << color(ANSI_COLOR_END);
  }
  os << punc(":")
     << color(ANSI_COLOR_GREEN)
     << type().toString()
     << color(ANSI_COLOR_END)
     ;
}

void SSATmp::print() const {
  print(std::cerr);
  std::cerr << std::endl;
}

std::string Trace::toString() const {
  std::ostringstream out;
  print(out, nullptr);
  return out.str();
}

void Trace::print() const {
  print(std::cout, nullptr);
}

void Trace::print(std::ostream& os, const AsmInfo* asmInfo) const {
  static const int kIndent = 4;
  Disasm disasm(Disasm::Options().indent(kIndent + 4)
                                 .printEncoding(dumpIREnabled(6))
                                 .color(color(ANSI_COLOR_BROWN)));

  // Print unlikely blocks at the end
  BlockList blocks, unlikely;
  for (Block* block : m_blocks) {
    if (block->getHint() == Block::Unlikely) {
      unlikely.push_back(block);
    } else {
      blocks.push_back(block);
    }
  }
  blocks.splice(blocks.end(), unlikely);

  for (Block* block : blocks) {
    TcaRange blockRange = asmInfo ? asmInfo->asmRanges[block] :
                          TcaRange(nullptr, nullptr);
    for (auto it = block->begin(); it != block->end();) {
      auto& inst = *it; ++it;

      if (inst.op() == Marker) {
        os << std::string(kIndent, ' ');
        inst.print(os);
        os << '\n';

        // Don't print bytecode in a non-main trace.
        if (!isMain()) continue;

        auto* marker = inst.getExtra<Marker>();
        uint32_t bcOffset = marker->bcOff;
        if (const auto* func = marker->func) {
          std::ostringstream uStr;
          func->unit()->prettyPrint(
            uStr, Unit::PrintOpts()
                  .range(bcOffset, bcOffset+1)
                  .noLineNumbers()
                  .indent(0));
          std::vector<std::string> vec;
          folly::split('\n', uStr.str(), vec);
          for (auto& s : vec) {
            os << color(ANSI_COLOR_BLUE) << s << color(ANSI_COLOR_END) << '\n';
          }
          continue;
        }
      }

      if (inst.op() == DefLabel) {
        os << std::string(kIndent - 2, ' ');
        inst.getBlock()->printLabel(os);
        os << punc(":") << "\n";
        // print phi pseudo-instructions
        for (unsigned i = 0, n = inst.getNumDsts(); i < n; ++i) {
          os << std::string(kIndent +
                            folly::format("({}) ", inst.getIId()).str().size(),
                            ' ');
          inst.getDst(i)->print(os, false);
          os << punc(" = ") << color(ANSI_COLOR_CYAN) << "phi "
             << color(ANSI_COLOR_END);
          bool first = true;
          inst.getBlock()->forEachSrc(i, [&](IRInstruction* jmp, SSATmp*) {
            if (!first) os << punc(", ");
            first = false;
            jmp->printSrc(os, i);
            os << punc("@");
            jmp->getBlock()->printLabel(os);
          });
          os << '\n';
        }
      }

      os << std::string(kIndent, ' ');
      inst.print(os);
      os << '\n';

      if (asmInfo) {
        TcaRange instRange = asmInfo->instRanges[inst];
        if (!instRange.empty()) {
          disasm.disasm(os, instRange.begin(), instRange.end());
          os << '\n';
          assert(instRange.end() >= blockRange.start() &&
                 instRange.end() <= blockRange.end());
          blockRange = TcaRange(instRange.end(), blockRange.end());
        }
      }
    }

    if (asmInfo) {
      // print code associated with this block that isn't tied to any
      // instruction.  This includes code after the last isntruction (e.g.
      // jmp to next block), and AStubs code.
      if (!blockRange.empty()) {
        os << std::string(kIndent, ' ') << punc("A:") << "\n";
        disasm.disasm(os, blockRange.start(), blockRange.end());
      }
      auto astubRange = asmInfo->astubRanges[block];
      if (!astubRange.empty()) {
        os << std::string(kIndent, ' ') << punc("AStubs:") << "\n";
        disasm.disasm(os, astubRange.start(), astubRange.end());
      }
      if (!blockRange.empty() || !astubRange.empty()) {
        os << '\n';
      }
    }
  }

  for (auto* exitTrace : m_exitTraces) {
    os << "\n" << color(ANSI_COLOR_GREEN)
       << "    -------  Exit Trace  -------"
       << color(ANSI_COLOR_END) << '\n';
    exitTrace->print(os, asmInfo);
  }
}

int32_t spillValueCells(IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->getNumSrcs();
  return numSrcs - 2;
}

/**
 * Return a list of blocks in reverse postorder
 */
BlockList sortCfg(Trace* trace, const IRFactory& factory) {
  assert(trace->isMain());
  BlockList blocks;
  unsigned next_id = 0;
  postorderWalk([&](Block* block) {
      block->setPostId(next_id++);
      blocks.push_front(block);
    }, factory.numBlocks(), trace->front());
  return blocks;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of postorder ids, indexed by postorder id.
 */
IdomVector findDominators(const BlockList& blocks) {
  // compute predecessors of each block
  int num_blocks = blocks.size();
  std::forward_list<int> preds[num_blocks];
  for (Block* block : blocks) {
    if (Block* succ = block->getNext()) {
      preds[succ->postId()].push_front(block->postId());
    }
    if (Block* succ = block->getTaken()) {
      preds[succ->postId()].push_front(block->postId());
    }
  }
  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  IdomVector idom(num_blocks, -1);
  auto start = blocks.begin();
  int start_id = (*start)->postId();
  idom[start_id] = start_id;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      int b = (*it)->postId();
      // new_idom = any already-processed predecessor
      auto pred_it = preds[b].begin();
      int new_idom = *pred_it;
      while (idom[new_idom] == -1) new_idom = *(++pred_it);
      // for all other already-processed predecessors p of b
      for (int p : preds[b]) {
        if (p != new_idom && idom[p] != -1) {
          // find earliest common predecessor of p and new_idom
          // (higher postIds are earlier in flow and in dom-tree).
          int b1 = p, b2 = new_idom;
          do {
            while (b1 < b2) b1 = idom[b1];
            while (b2 < b1) b2 = idom[b2];
          } while (b1 != b2);
          new_idom = b1;
        }
      }
      if (idom[b] != new_idom) {
        idom[b] = new_idom;
        changed = true;
      }
    }
  }
  idom[start_id] = -1; // start has no idom.
  return idom;
}

bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms) {
  int p1 = b1->postId();
  int p2 = b2->postId();
  for (int i = p2; i != -1; i = idoms[i]) {
    if (i == p1) return true;
  }
  return false;
}

/*
 * Check one block for being well formed.  It must:
 * 1. have exactly one DefLabel as the first instruction
 * 2. if any instruction is isBlockEnd(), it must be last
 * 3. if the last instruction isTerminal(), block->next must be null.
 */
bool checkBlock(Block* b) {
  assert(!b->empty());
  assert(b->front()->op() == DefLabel);
  if (b->back()->isTerminal()) assert(!b->getNext());
  if (b->getTaken()) {
    // only Jmp_ can branch to a join block expecting values.
    assert(b->back()->op() == Jmp_ ||
           b->getTaken()->front()->getNumDsts() == 0);
  }
  if (b->getNext()) {
    // cannot fall-through to join block expecting values
    assert(b->getNext()->front()->getNumDsts() == 0);
  }
  auto i = b->begin(), e = b->end();
  ++i;
  if (b->back()->isBlockEnd()) --e;
  for (UNUSED IRInstruction& inst : folly::makeRange(i, e)) {
    assert(inst.op() != DefLabel);
    assert(!inst.isBlockEnd());
  }
  return true;
}

/*
 * Build the CFG, then the dominator tree, then use it to validate SSA.
 * 1. Each src must be defined by some other instruction, and each dst must
 *    be defined by the current instruction.
 * 2. Each src must be defined earlier in the same block or in a dominator.
 * 3. Each dst must not be previously defined.
 * 4. Treat tmps defined by DefConst as always defined.
 */
bool checkCfg(Trace* trace, const IRFactory& factory) {
  forEachTraceBlock(trace, [&](Block* b) {
    checkBlock(b);
  });
  BlockList blocks = sortCfg(trace, factory);
  IdomVector idom = findDominators(blocks);
  // build dominator-children lists
  std::forward_list<Block*> children[blocks.size()];
  for (Block* block : blocks) {
    int idom_id = idom[block->postId()];
    if (idom_id != -1) children[idom_id].push_front(block);
  }

  // visit dom tree in preorder, checking all tmps
  boost::dynamic_bitset<> defined0(factory.numTmps());
  forPreorderDoms(blocks.front(), children, defined0,
                  [] (Block* block, boost::dynamic_bitset<>& defined) {
    for (IRInstruction& inst : *block) {
      for (SSATmp* UNUSED src : inst.getSrcs()) {
        assert(src->inst() != &inst);
        assert_log(src->inst()->op() == DefConst ||
                   defined[src->getId()],
                   [&]{ return folly::format(
                       "src '{}' in '{}' came from '{}', which is not a "
                       "DefConst and is not defined at this use site",
                       src->toString(), inst.toString(),
                       src->inst()->toString()).str();
                   });
      }
      for (SSATmp& dst : inst.getDsts()) {
        assert(dst.inst() == &inst &&
               inst.op() != DefConst);
        assert(!defined[dst.getId()]);
        defined[dst.getId()] = 1;
      }
    }
  });
  return true;
}

bool hasInternalFlow(Trace* trace) {
  for (Block* block : trace->getBlocks()) {
    if (Block* taken = block->getTaken()) {
      if (taken->getTrace() == trace) return true;
    }
  }
  return false;
}

void dumpTraceImpl(const Trace* trace, std::ostream& out,
                   const AsmInfo* asmInfo) {
  trace->print(out, asmInfo);
}

// Suggested captions: "before jiffy removal", "after goat saturation",
// etc.
void dumpTrace(int level, const Trace* trace, const char* caption,
               AsmInfo* ai) {
  if (dumpIREnabled(level)) {
    std::ostringstream str;
    auto bannerFmt = "{:-^40}\n";
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, caption)
        << color(ANSI_COLOR_END)
        ;
    dumpTraceImpl(trace, str, ai);
    trace->print(str);
    str << color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN)
        << folly::format(bannerFmt, "")
        << color(ANSI_COLOR_END)
        ;
    HPHP::Trace::traceRelease("%s", str.str().c_str());
  }
}

}}}

