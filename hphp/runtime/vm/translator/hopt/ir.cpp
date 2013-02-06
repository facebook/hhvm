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

#include "folly/Format.h"
#include "folly/Traits.h"

#include "util/disasm.h"
#include "util/trace.h"
#include "runtime/base/string_data.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/translator/hopt/irfactory.h"
#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/cse.h"
#include "runtime/vm/translator/hopt/simplifier.h"

namespace HPHP { namespace VM { namespace JIT {

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

enum OpcodeFlag : uint64_t {
  NoFlags          = 0x0000,
  HasDest          = 0x0001,
  CanCSE           = 0x0002,
  Essential        = 0x0004,
  MemEffects       = 0x0008,
  CallsNative      = 0x0010,
  ConsumesRC       = 0x0020,
  ProducesRC       = 0x0040,
  MayModifyRefs    = 0x0080,
  Rematerializable = 0x0100, // TODO: implies HasDest
  MayRaiseError    = 0x0200,
  Terminal         = 0x0400, // has no next instruction
  NaryDest         = 0x0800, // has 0 or more destinations
  HasExtra         = 0x1000,
};

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

#define ND        0
#define D(n)      HasDest
#define DofS(n)   HasDest
#define DUnbox(n) HasDest
#define DBox(n)   HasDest
#define DParam    HasDest
#define DLabel    NaryDest
#define DVector   HasDest
#define DPtrToParam HasDest

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

#undef ND
#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DParam
#undef DLabel

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

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_hash,   hash);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_equals, equals);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_clone,  clone);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_show,   show);

template<class T>
typename std::enable_if<
  has_hash<T,size_t () const>::value,
  size_t
>::type hashExtraImpl(T* t) { return t->hash(); }
size_t hashExtraImpl(IRExtraData*) {
  // This probably means an instruction was marked CanCSE but its
  // extra data had no hash function.
  always_assert(!"attempted to hash extra data that didn't "
    "provide a hash function");
}

template<class T>
typename std::enable_if<
  has_equals<T,bool (T const&) const>::value ||
  has_equals<T,bool (T)        const>::value,
  bool
>::type equalsExtraImpl(T* t, IRExtraData* o) {
  return t->equals(*static_cast<T*>(o));
}
bool equalsExtraImpl(IRExtraData*, IRExtraData*) {
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

MAKE_DISPATCHER(HashDispatcher, size_t, hashExtraImpl);
size_t hashExtra(Opcode opc, IRExtraData* data) {
  return dispatchExtra<size_t,HashDispatcher>(opc, data);
}

MAKE_DISPATCHER(EqualsDispatcher, bool, equalsExtraImpl);
bool equalsExtra(Opcode opc, IRExtraData* data, IRExtraData* other) {
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
  , m_asmAddr(nullptr)
  , m_label(inst->m_label)
  , m_parent(nullptr)
  , m_tca(nullptr)
  , m_extra(inst->m_extra ? cloneExtra(getOpcode(), inst->m_extra, arena)
                          : nullptr)
{
  std::copy(inst->m_srcs, inst->m_srcs + inst->m_numSrcs, m_srcs);
  if (naryDst()) {
    m_dsts = new (arena) SSATmp*[m_numDsts];
    memset(m_dsts, 0, m_numDsts * sizeof(*m_dsts));
  }
}

const char* opcodeName(Opcode opcode) { return OpInfo[opcode].name; }

static bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return OpInfo[opcode].flags & flags;
}

bool IRInstruction::hasExtra() const {
  return opcodeHasFlags(getOpcode(), HasExtra);
}

bool IRInstruction::hasDst() const {
  return opcodeHasFlags(getOpcode(), HasDest);
}

bool IRInstruction::naryDst() const {
  return opcodeHasFlags(getOpcode(), NaryDest);
}

bool IRInstruction::isNative() const {
  return opcodeHasFlags(getOpcode(), CallsNative);
}

bool IRInstruction::producesReference() const {
  return opcodeHasFlags(getOpcode(), ProducesRC);
}

bool IRInstruction::isRematerializable() const {
  return opcodeHasFlags(getOpcode(), Rematerializable);
}

bool IRInstruction::hasMemEffects() const {
  return opcodeHasFlags(getOpcode(), MemEffects) || mayReenterHelper();
}

bool IRInstruction::canCSE() const {
  auto canCSE = opcodeHasFlags(getOpcode(), CanCSE);
  // Make sure that instructions that are CSE'able can't produce a
  // reference count or consume reference counts.
  assert(!canCSE || !producesReference());
  assert(!canCSE || !consumesReferences());
  return canCSE && !mayReenterHelper();
}

bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(getOpcode(), ConsumesRC);
}

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }
  // Special case StMem, StMemNT, StProp, and StPropNT.
  // These instructions only consume the value operand.
  if ((m_op == StMem || m_op == StMemNT) && srcNo == 0) {
    // StMem[NT] <pointer>, <value>
    return false;
  }
  if ((m_op == StProp || m_op == StPropNT) && (srcNo == 0 || srcNo == 1)) {
    // StProp[NT] <base>, <offset>, <value>
    return false;
  }
  return true;
}

bool IRInstruction::mayModifyRefs() const {
  Opcode opc = getOpcode();
  // DecRefNZ does not have side effects other than decrementing the ref
  // count. Therefore, its MayModifyRefs should be false.
  if (opc == DecRef) {
    auto type = getSrc(0)->getType();
    if (isControlFlowInstruction() || type.isString()) {
      // If the decref has a target label, then it exits if the destructor
      // has to be called, so it does not have any side effects on the main
      // trace.
      return false;
    }
    if (type.isBoxed()) {
      Type innerType = type.innerType();
      return innerType == Type::Obj || innerType == Type::Arr;
    }
  }
  return opcodeHasFlags(opc, MayModifyRefs) || mayReenterHelper();
}

bool IRInstruction::mayRaiseError() const {
  return opcodeHasFlags(getOpcode(), MayRaiseError) || mayReenterHelper();
}

bool IRInstruction::isEssential() const {
  Opcode opc = getOpcode();
  if (opc == DecRefNZ) {
    // If the source of a DecRefNZ is not an IncRef, mark it as essential
    // because we won't remove its source as well as itself.
    // If the ref count optimization is turned off, mark all DecRefNZ as
    // essential.
    if (!RuntimeOption::EvalHHIREnableRefCountOpt ||
        getSrc(0)->getInstruction()->getOpcode() != IncRef) {
      return true;
    }
  }
  return isControlFlowInstruction() ||
         opcodeHasFlags(opc, Essential) ||
         mayReenterHelper();
}

bool IRInstruction::isTerminal() const {
  return opcodeHasFlags(getOpcode(), Terminal);
}

bool IRInstruction::mayReenterHelper() const {
  if (isCmpOp(getOpcode())) {
    return cmpOpTypesMayReenter(getOpcode(),
                                getSrc(0)->getType(),
                                getSrc(1)->getType());
  }
  // Not necessarily actually false; this is just a helper for other
  // bits.
  return false;
}

SSARange IRInstruction::getDsts() {
  return hasDst() ? SSARange(&m_dst, m_numDsts) :
         SSARange(m_dsts, m_numDsts);
}

folly::Range<SSATmp* const*> IRInstruction::getDsts() const {
  return hasDst() ? Range<SSATmp* const*>(&m_dst, m_numDsts) :
         Range<SSATmp* const*>(const_cast<SSATmp* const*>(m_dsts), m_numDsts);
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
  if (tmp->getType().notCounted()) {
    return false;
  }
  IRInstruction* inst = tmp->getInstruction();
  Opcode opc = inst->getOpcode();
  if (opc == DefConst || opc == LdConst || opc == LdClsCns) {
    return false;
  }
  return true;
}

void IRInstruction::convertToNop() {
  IRInstruction nop(Nop);
  // copy all but m_iid and m_parent
  m_op = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs = nop.m_numSrcs;
  m_id = nop.m_id;
  m_srcs = nop.m_srcs;
  m_liveOutRegs = nop.m_liveOutRegs;
  m_dst = nop.m_dst;
  m_asmAddr = nop.m_asmAddr;
  m_label = nop.m_label;
  m_tca = nop.m_tca;
  m_extra = nullptr;
}

IRInstruction* IRInstruction::clone(IRFactory* factory) const {
  return factory->cloneInstruction(this);
}

IRInstruction* LabelInstruction::clone(IRFactory* factory) const {
  return factory->cloneInstruction(this);
}

SSATmp* IRInstruction::getSrc(uint32 i) const {
  if (i >= getNumSrcs()) return nullptr;
  return m_srcs[i];
}

void IRInstruction::setSrc(uint32 i, SSATmp* newSrc) {
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

bool IRInstruction::equals(IRInstruction* inst) const {
  if (m_op != inst->m_op ||
      m_typeParam != inst->m_typeParam ||
      m_numSrcs != inst->m_numSrcs) {
    return false;
  }
  for (uint32 i = 0; i < getNumSrcs(); i++) {
    if (getSrc(i) != inst->getSrc(i)) {
      return false;
    }
  }
  if (hasExtra() && !equalsExtra(getOpcode(), m_extra, inst->m_extra)) {
    return false;
  }
  // TODO: check label for ControlFlowInstructions?
  return true;
}

size_t IRInstruction::hash() const {
  size_t srcHash = 0;
  for (unsigned i = 0; i < getNumSrcs(); ++i) {
    srcHash = CSEHash::hashCombine(srcHash, getSrc(i));
  }
  if (hasExtra()) {
    srcHash = CSEHash::hashCombine(srcHash, hashExtra(getOpcode(), m_extra));
  }
  return CSEHash::hashCombine(srcHash, m_op, m_typeParam);
}

void IRInstruction::printOpcode(std::ostream& os) const {
  os << opcodeName(m_op);

  if (m_typeParam == Type::None && !hasExtra()) {
    return;
  }
  os << '<';
  if (m_typeParam != Type::None) {
    os << m_typeParam.toString();
    if (hasExtra()) os << ',';
  }
  if (hasExtra()) {
    os << showExtra(getOpcode(), m_extra);
  }
  os << '>';
}

void IRInstruction::printDst(std::ostream& ostream) const {
  if (m_dst) {
    m_dst->print(ostream, true);
    ostream << " = ";
  }
}

void IRInstruction::printSrc(std::ostream& ostream, uint32 i) const {
  SSATmp* src = getSrc(i);
  if (src != nullptr) {
    if (m_id != 0 && !src->isConst() && src->getLastUseId() == m_id) {
      ostream << "~";
    }
    src->print(ostream);
  } else {
    ostream << "!!!NULL @ " << i;
  }
}

void IRInstruction::printSrcs(std::ostream& ostream) const {
  bool first = true;
  if (getOpcode() == IncStat) {
    ostream << " " << Stats::g_counterNames[getSrc(0)->getValInt()] <<
               ", " << getSrc(1)->getValInt();
    return;
  }
  for (uint32 i = 0; i < m_numSrcs; i++) {
    if (!first) {
      ostream << ", ";
    } else {
      ostream << " ";
      first = false;
    }
    printSrc(ostream, i);
  }
}

void IRInstruction::print(std::ostream& ostream) const {
  if (getOpcode() == Marker) {
    auto* marker = getExtra<Marker>();
    ostream << "--- bc" << marker->bcOff
            << ", spOff: " << marker->stackOff;
    return;
  }

  if (!isTransient()) {
    ostream << folly::format("({:02d}) ", getIId());
  }
  printDst(ostream);

  bool isStMem = m_op == StMem || m_op == StMemNT || m_op == StRaw;
  bool isLdMem = m_op == LdRaw;
  if (isStMem || isLdMem) {
    ostream << opcodeName(m_op) << " [";
    printSrc(ostream, 0);
    SSATmp* offset = getSrc(1);
    if (m_op == StRaw) {
      RawMemSlot& s =
        RawMemSlot::Get(RawMemSlot::Kind(offset->getValInt()));
      int64 offset = s.getOffset() + getSrc(3)->getValInt();
      if (offset) {
        ostream << " + " << offset;
      }
    } else if ((isStMem || isLdMem) &&
        (!offset->isConst() || offset->getValInt() != 0)) {
      ostream << " + ";
      printSrc(ostream, 1);
    }
    Type type = isStMem ? getSrc(2)->getType() : m_typeParam;
    ostream << "]:" << type.toString();
    if (!isLdMem) {
      assert(getNumSrcs() > 1);
      ostream << ", ";
      printSrc(ostream, isStMem ? 2 : 1);
    }
  } else {
    printOpcode(ostream);
    printSrcs(ostream);
  }

  if (m_label) {
    ostream << " -> ";
    m_label->printLabel(ostream);
  }
  if (m_tca) {
    ostream << ", ";
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
  } else if (t == Type::Arr) {
    auto arr = inst->getExtra<DefConst>()->as<const ArrayData*>();
    if (arr->empty()) {
      os << "array()";
    } else {
      os << "Array(" << arr << ")";
    }
  } else if (t.isNull()) {
    os << t.toString();
  } else if (t == Type::Func) {
    auto func = c->as<const Func*>();
    os << "Func(" << (func ? func->fullName()->data() : "0") << ")";
  } else if (t == Type::Cls) {
    auto cls = c->as<const Class*>();
    os << "Cls(" << (cls ? cls->name()->data() : "0") << ")";
  } else if (t == Type::NamedEntity) {
    auto ne = c->as<const NamedEntity*>();
    os << "NamedEntity(" << ne << ")";
  } else if (t == Type::TCA) {
    void* vp = c->as<TCA>();
    os << folly::format("TCA: 0x{}", vp);
  } else if (t == Type::None) {
    os << "None:" << c->as<int64_t>();
  } else {
    not_reached();
  }
}

bool LabelInstruction::equals(IRInstruction* inst) const {
  assert(0);
  return false;
}

size_t LabelInstruction::hash() const {
  assert(0);
  return 0;
}

// Thread chain of patch locations using the 4 byte space in each jmp/jcc
void LabelInstruction::prependPatchAddr(TCA patchAddr) {
  assert(getOpcode() == DefLabel);
  ssize_t diff = getPatchAddr() ? ((TCA)patchAddr - (TCA)getPatchAddr()) : 0;
  assert(deltaFits(diff, sz::dword));
  *(int*)(patchAddr) = (int)diff;
  m_patchAddr = patchAddr;
}

void* LabelInstruction::getPatchAddr() {
  return m_patchAddr;
}

void LabelInstruction::printLabel(std::ostream& ostream) const {
  ostream << "L" << m_labelId;
}

void LabelInstruction::print(std::ostream& ostream) const {
  assert(getOpcode() == DefLabel);
  printLabel(ostream);
  if (unsigned n = getNumDsts()) {
    auto dsts = getDsts();
    ostream << '(';
    for (unsigned i = 0; i < n; ++i) {
      ostream << (i > 0 ? ", " : "");
      dsts[i]->print(ostream, true);
    }
    ostream << ')';
  }
  ostream << ":";
}

int SSATmp::numNeededRegs() const {
  auto type = getType();
  if (type.subtypeOf(Type::None | Type::Null | Type::ActRec)) {
    // These don't need a register because their values are static or unused.
    return 0;
  }
  if (type.subtypeOf(Type::Ctx) || type.isPtr()) {
    // Ctx and PtrTo* may be statically unknown but always need just 1 register.
    return 1;
  }
  if (!type.isStaticallyKnown() || type.subtypeOf(Type::FuncCtx)) {
    // These need 2 registers, 1 for the value and 1 for the type, or
    // 1 for the Func* and 1 for the {Obj|Cctx}
    return 2;
  }

  // By default, all other types only need 1 register.
  assert(type.isStaticallyKnown());
  return 1;
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


bool SSATmp::getValBool() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Bool));
  return m_inst->getExtra<ConstData>()->as<bool>();
}

int64 SSATmp::getValInt() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::Int));
  return m_inst->getExtra<ConstData>()->as<int64_t>();
}

int64 SSATmp::getValRawInt() const {
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
  assert(m_inst->getTypeParam().equals(Type::Arr));
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

TCA SSATmp::getValTCA() const {
  assert(isConst());
  assert(m_inst->getTypeParam().equals(Type::TCA));
  return m_inst->getExtra<ConstData>()->as<TCA>();
}

void SSATmp::setTCA(TCA tca) {
  getInstruction()->setTCA(tca);
}
TCA SSATmp::getTCA() const {
  return getInstruction()->getTCA();
}

void SSATmp::print(std::ostream& os, bool printLastUse) const {
  if (m_inst->getOpcode() == DefConst) {
    printConst(os, m_inst);
    return;
  }
  os << "t" << m_id;
  if (printLastUse && m_lastUseId != 0) {
    os << "@" << m_lastUseId << "#" << m_useCount;
  }
  if (m_isSpilled || numAllocatedRegs() > 0) {
    os << '(';
    if (!m_isSpilled) {
      for (int i = 0, sz = numAllocatedRegs(); i < sz; ++i) {
        if (i != 0) os << ", ";
        os << reg::regname(Reg64(int(m_regs[i])));
      }
    } else {
      for (int i = 0, sz = numNeededRegs(); i < sz; ++i) {
        if (i != 0) os << ", ";
        os << m_spillInfo[i];
      }
    }
    os << ')';
  }
  os << ":" << getType().toString();
}

void SSATmp::print() const {
  print(std::cerr);
  std::cerr << std::endl;
}

void Trace::print(std::ostream& os, bool printAsm,
                  bool isExit /* = false */) const {
  Disasm disasm(14, RuntimeOption::EvalDumpIR > 5);

  auto it = begin(m_instructionList);
  while (it != end(m_instructionList)) {
    auto* inst = *it;
    ++it;
    if (inst->getOpcode() == Marker) {
      auto* marker = inst->getExtra<Marker>();
      if (isExit) {
        // Don't print bytecode, but print the label.
        os << std::string(6, ' ');
        inst->print(os);
        os << '\n';
        continue;
      }
      uint32 bcOffset = marker->bcOff;
      if (const auto* func = marker->func) {
        func->unit()->prettyPrint(
          os, Unit::PrintOpts()
                .range(bcOffset, bcOffset+1)
                .noLineNumbers());
        continue;
      }
    }
    if (inst->getOpcode() == DefLabel) {
      os << std::string(6, ' ');
    } else {
      os << std::string(8, ' ');
    }
    inst->print(os);
    os << '\n';
    if (!printAsm) {
      continue;
    }
    uint8* asmAddr = (uint8*)inst->getAsmAddr();
    if (asmAddr == nullptr) {
      continue;
    }
    // Find the next instruction that has an non-NULL asm address.
    auto nextHasAsmAddr = it;
    while (nextHasAsmAddr != m_instructionList.end() &&
           (*nextHasAsmAddr)->getAsmAddr() == nullptr) {
      ++nextHasAsmAddr;
    }
    uint8* endAsm;
    if (nextHasAsmAddr != m_instructionList.end()) {
      endAsm = (uint8*)(*nextHasAsmAddr)->getAsmAddr();
    } else {
      endAsm = m_lastAsmAddress;
    }
    if (asmAddr != endAsm) {
      // print out the assembly
      os << '\n';
      disasm.disasm(os, asmAddr, endAsm);
      os << '\n';
    }
  }

  bool firstExitTracePrinted = false;
  for (auto* exitTrace : m_exitTraces) {
    if (!firstExitTracePrinted) {
      firstExitTracePrinted = true;
      // print out any extra code in astubs
      if (m_firstAstubsAddress < exitTrace->m_firstAsmAddress) {
        os << std::string(8, ' ') << "AStubs:\n";
        disasm.disasm(os, m_firstAstubsAddress,
                      exitTrace->m_firstAsmAddress);
        os << '\n';
      }

    }
    os << "\n      -------  Exit Trace  -------\n";
    exitTrace->print(os, printAsm, true);
  }
}

void Trace::print() const {
  print(std::cout, true /* printAsm */);
}

void resetIdsAux(Trace* trace) {
  for (IRInstruction* inst : trace->getInstructionList()) {
    inst->setId(0);
    for (SSATmp* dst : inst->getDsts()) {
      dst->setLastUseId(0);
      dst->setUseCount(0);
      dst->setSpillSlot(-1);
    }
  }
}

void resetIds(Trace* trace) {
  resetIdsAux(trace);
  for (Trace* exit : trace->getExitTraces()) {
    resetIdsAux(exit);
  }
}

int32_t spillValueCells(IRInstruction* spillStack) {
  int32_t numSrcs = spillStack->getNumSrcs();
  int32_t ret = 0;
  for (int i = 2; i < numSrcs; ++i) {
    if (spillStack->getSrc(i)->getType() == Type::ActRec) {
      ret += kNumActRecCells;
      i += kSpillStackActRecExtraArgs;
    } else {
      ++ret;
    }
  }
  return ret;
}

BlockList numberInstructions(Trace* trace) {
  resetIds(trace);
  BlockList blocks = buildCfg(trace);
  uint32_t nextId = 1;
  for (Block& block : blocks) {
    for (IRInstruction* inst : block) {
      for (SSATmp* dst : inst->getDsts()) {
        // Initialize this value for register spilling.
        dst->setSpillSlot(-1);
      }
      if (inst->getOpcode() == Marker) {
        continue; // don't number markers
      }
      uint32_t id = nextId++;
      inst->setId(id);
      for (SSATmp* tmp : inst->getSrcs()) {
        tmp->setLastUseId(id);
        tmp->incUseCount();
      }
    }
  }
  return blocks;
}

/**
 * TopoSort encapsulates a depth-first search which identifies basic
 * blocks and populates a list of blocks in reverse-postorder.
 */
struct TopoSort {
  typedef IRInstruction::Iterator Iter;

  TopoSort(BlockList& blocks) : m_blocks(blocks), m_next_id(0) {
    blocks.clear();
  }

  void visit(Iter start, Iter end) {
    assert(start != end);
    if (m_visited.find(*start) != m_visited.end()) return;
    // first time in this block; remember the first instruction.
    m_visited[*start] = nullptr;
    Iter i = start;
    while (i != end && !isBlockEnd(*i)) i++; // skip to end
    IRInstruction* inst = *i; // last instruction in block
    ++i;
    if (!inst->isTerminal()) {
      // visit fall-through path with remainder of list
      visit(i, end);
    }
    if (inst->isControlFlowInstruction()) {
      // visit target of branch with remainder of the target trace's list
      LabelInstruction* target = inst->getLabel();
      Trace* targetTrace = target->getParent();
      auto& traceList = targetTrace->getInstructionList();
      Iter t = targetTrace == inst->getParent() ?
               find(i, end, target) : // search forward in current trace
               find(traceList.begin(), traceList.end(), target);
      visit(t, traceList.end());
    }
    // [start, i) is the basic block we just processed; prepend it.
    m_blocks.push_front(Block(m_next_id++, start, i));
    m_visited[*start] = &m_blocks.front();
  }

  static bool isBlockEnd(IRInstruction* inst) {
    return inst->isControlFlowInstruction() || inst->isTerminal();
  }

  hphp_hash_map<IRInstruction*, Block*> m_visited;
  BlockList& m_blocks;
  unsigned m_next_id; // next postorder id to assign
};

BlockList buildCfg(Trace* trace) {
  assert(trace->isMain());
  auto& traceList = trace->getInstructionList();
  BlockList blocks;
  TopoSort sorter(blocks);
  sorter.visit(traceList.begin(), traceList.end());
  // link successors
  boost::dynamic_bitset<> has_pred(blocks.size());
  auto processSucc = [&] (IRInstruction* succInst) {
    Block* s = sorter.m_visited[succInst];
    s->m_isJoin = has_pred[s->postId()];
    has_pred[s->postId()] = 1;
    return s;
  };
  for (Block& block : blocks) {
    assert(block.postId() < blocks.size());
    IRInstruction* last = block.lastInst();
    if (!last->isTerminal()) {
      block.m_succ[0] = processSucc(*block.end());
    }
    if (last->isControlFlowInstruction()) {
      block.m_succ[1] = processSucc(last->getLabel());
    }
  }
  return blocks;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of postorder ids, indexed by postorder id.
 */
std::vector<int> findDominators(BlockList& blocks) {
  // compute predecessors of each block
  int num_blocks = blocks.size();
  std::forward_list<int> preds[num_blocks];
  for (Block& block : blocks) {
    for (Block* succ : block.succ()) {
      preds[succ->postId()].push_front(block.postId());
    }
  }
  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  vector<int> idom(num_blocks, -1);
  auto start = blocks.begin();
  int start_id = (*start).postId();
  idom[start_id] = start_id;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      int b = (*it).postId();
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

/*
 * Build the CFG, then the dominator tree, then use it to validate SSA.
 * 1. Each src must be defined by some other instruction, and each dst must
 *    be defined by the current instruction.
 * 2. Each src must be defined earlier in the same block or in a dominator.
 * 3. Each dst must not be previously defined.
 * 4. Treat tmps defined by DefConst as always defined.
 */
bool checkCfg(Trace* trace, const IRFactory& factory) {
  BlockList blocks = buildCfg(trace);
  vector<int> idom = findDominators(blocks);
  // build dominator-children lists
  std::forward_list<Block*> children[blocks.size()];
  for (Block& block : blocks) {
    int idom_id = idom[block.postId()];
    if (idom_id != -1) children[idom_id].push_front(&block);
  }
  // visit dom tree in preorder, checking all tmps
  boost::dynamic_bitset<> defined0(factory.numTmps());
  forPreorderDoms(&blocks.front(), children, defined0,
                  [] (Block* block, boost::dynamic_bitset<>& defined) {
    for (IRInstruction* inst : *block) {
      for (SSATmp* UNUSED src : inst->getSrcs()) {
        assert(src->getInstruction() != inst);
        assert(src->getInstruction()->getOpcode() == DefConst ||
               defined[src->getId()]);
      }
      for (SSATmp* dst : inst->getDsts()) {
        assert(dst->getInstruction() == inst &&
               inst->getOpcode() != DefConst);
        assert(!defined[dst->getId()]);
        defined[dst->getId()] = 1;
      }
    }
  });
  return true;
}

}}}

