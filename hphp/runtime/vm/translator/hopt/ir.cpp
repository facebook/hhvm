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

#include "hphp/runtime/vm/translator/hopt/ir.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>
#include <boost/algorithm/string.hpp>

#include "folly/Format.h"
#include "folly/Traits.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/string_data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/translator/hopt/cse.h"
#include "hphp/runtime/vm/translator/hopt/irinstruction.h"
#include "hphp/runtime/vm/translator/hopt/irfactory.h"
#include "hphp/runtime/vm/translator/hopt/linearscan.h"
#include "hphp/runtime/vm/translator/hopt/print.h"
#include "hphp/runtime/vm/translator/hopt/simplifier.h"
#include "hphp/runtime/vm/translator/hopt/trace.h"

// Include last to localize effects to this file
#include "hphp/util/assert_throw.h"

namespace HPHP {  namespace JIT {

using namespace HPHP::Transl;

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
#define DArith    HasDest
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
#undef DArith
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
std::string showExtraImpl(const IRExtraData*) { return "..."; }

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

} // namespace

std::string showExtra(Opcode opc, const IRExtraData* data) {
  return dispatchExtra<std::string,ShowDispatcher>(opc,
      const_cast<IRExtraData*>(data));
}

//////////////////////////////////////////////////////////////////////

IRInstruction::IRInstruction(Arena& arena, const IRInstruction* inst, Id id)
  : m_op(inst->m_op)
  , m_typeParam(inst->m_typeParam)
  , m_numSrcs(inst->m_numSrcs)
  , m_numDsts(inst->m_numDsts)
  , m_id(id)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dst(nullptr)
  , m_extra(inst->m_extra ? cloneExtra(op(), inst->m_extra, arena)
                          : nullptr)
{
  std::copy(inst->m_srcs, inst->m_srcs + inst->m_numSrcs, m_srcs);
  setTaken(inst->getTaken());
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

Trace* IRInstruction::getTrace() const {
  return getBlock()->getTrace();
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
  // count or consume reference counts. CheckType is special because it can
  // refine a maybeCounted type to a notCounted type, so it logically consumes
  // and produces a reference without doing any work.
  assert(!canCSE || !producesReference() || m_op == CheckType);
  assert(!canCSE || !consumesReferences() || m_op == CheckType);
  return canCSE && !mayReenterHelper();
}

bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(op(), ConsumesRC);
}

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }
  // CheckType consumes a reference if we're guarding from a maybeCounted type
  // to a notCounted type.
  if (m_op == CheckType) {
    assert(srcNo == 0);
    return getSrc(0)->type().maybeCounted() && getTypeParam().notCounted();
  }
  // SpillStack consumes inputs 2 and onward
  if (m_op == SpillStack) return srcNo >= 2;
  // Call consumes inputs 3 and onward
  if (m_op == Call) return srcNo >= 3;
  // StRetVal only consumes input 1
  if (m_op == StRetVal) return srcNo == 1;

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

/*
 * Returns true if the instruction loads into a SSATmp representing a
 * PHP value (a subtype of Gen).  Note that this function returns
 * false for instructions that load internal meta-data, such as Func*,
 * Class*, etc.
 */
bool IRInstruction::isLoad() const {
  switch (m_op) {
    case LdStack:
    case LdLoc:
    case LdMem:
    case LdProp:
    case LdRef:
    case LdThis:
    case LdStaticLocCached:
    case LookupCns:
    case LookupClsCns:
    case CGetProp:
    case VGetProp:
    case VGetPropStk:
    case ArrayGet:
    case CGetElem:
    case VGetElem:
    case VGetElemStk:
    case ArrayIdx:
      return true;

    default:
      return false;
  }
}

/*
 * Returns true if the instruction stores its source operand srcIdx to memory.
 */
bool IRInstruction::stores(uint32_t srcIdx) const {
  switch (m_op) {
    case StRetVal:
    case StLoc:
    case StLocNT:
    case StRef:
    case StRefNT:
    case SetNewElem:
    case SetNewElemStk:
    case BindNewElem:
    case BindNewElemStk:
      return srcIdx == 1;

    case StMem:
    case StMemNT:
    case StProp:
    case StPropNT:
      return srcIdx == 2;

    case SetElem:
    case SetElemStk:
    case BindElem:
    case BindElemStk:
      return srcIdx == 3;

    case SetProp:
    case SetPropStk:
    case BindProp:
    case BindPropStk:
      return srcIdx == 4;

    case SpillStack:
      return srcIdx >= 2 && srcIdx < getNumSrcs();

    case Call:
      return srcIdx >= 3 && srcIdx < getNumSrcs();

    case CallBuiltin:
      return srcIdx >= 1 && srcIdx < getNumSrcs();

    default:
      return false;
  }
}

SSATmp* IRInstruction::getPassthroughValue() const {
  assert(isPassthrough());
  assert(m_op == IncRef || m_op == CheckType || m_op == Mov);
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

const StringData* findClassName(SSATmp* cls) {
  assert(cls->isA(Type::Cls));

  if (cls->isConst()) {
    return cls->getValClass()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->getSrc(0);
    assert(clsName->isA(Type::Str));
    if (clsName->isConst()) {
      return clsName->getValStr();
    }
  }
  return nullptr;
}

bool isQueryOp(Opcode opc) {
  switch (opc) {
  case OpGt:
  case OpGte:
  case OpLt:
  case OpLte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case IsType:
  case IsNType:
    return true;
  default:
    return false;
  }
}

bool isCmpOp(Opcode opc) {
  switch (opc) {
  case OpGt:
  case OpGte:
  case OpLt:
  case OpLte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
    return true;
  default:
    return false;
  }
}

bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpSame:
  case JmpNSame:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
  case JmpIsType:
  case JmpIsNType:
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

Opcode queryToJmpOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case OpGt:               return JmpGt;
  case OpGte:              return JmpGte;
  case OpLt:               return JmpLt;
  case OpLte:              return JmpLte;
  case OpEq:               return JmpEq;
  case OpNeq:              return JmpNeq;
  case OpSame:             return JmpSame;
  case OpNSame:            return JmpNSame;
  case InstanceOfBitmask:  return JmpInstanceOfBitmask;
  case NInstanceOfBitmask: return JmpNInstanceOfBitmask;
  case IsType:             return JmpIsType;
  case IsNType:            return JmpIsNType;
  default:                 always_assert(0);
  }
}

Opcode queryJmpToQueryOp(Opcode opc) {
  assert(isQueryJmpOp(opc));
  switch (opc) {
  case JmpGt:                 return OpGt;
  case JmpGte:                return OpGte;
  case JmpLt:                 return OpLt;
  case JmpLte:                return OpLte;
  case JmpEq:                 return OpEq;
  case JmpNeq:                return OpNeq;
  case JmpSame:               return OpSame;
  case JmpNSame:              return OpNSame;
  case JmpInstanceOfBitmask:  return InstanceOfBitmask;
  case JmpNInstanceOfBitmask: return NInstanceOfBitmask;
  case JmpIsType:             return IsType;
  case JmpIsNType:            return IsNType;
  default:                    always_assert(0);
  }
}

Opcode jmpToReqBindJmp(Opcode opc) {
  switch (opc) {
  case JmpGt:                 return ReqBindJmpGt;
  case JmpGte:                return ReqBindJmpGte;
  case JmpLt:                 return ReqBindJmpLt;
  case JmpLte:                return ReqBindJmpLte;
  case JmpEq:                 return ReqBindJmpEq;
  case JmpNeq:                return ReqBindJmpNeq;
  case JmpSame:               return ReqBindJmpSame;
  case JmpNSame:              return ReqBindJmpNSame;
  case JmpInstanceOfBitmask:  return ReqBindJmpInstanceOfBitmask;
  case JmpNInstanceOfBitmask: return ReqBindJmpNInstanceOfBitmask;
  case JmpZero:               return ReqBindJmpZero;
  case JmpNZero:              return ReqBindJmpNZero;
  default:                    always_assert(0);
  }
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
  case InstanceOfBitmask:   return NInstanceOfBitmask;
  case NInstanceOfBitmask:  return InstanceOfBitmask;
  case IsType:              return IsNType;
  case IsNType:             return IsType;
  default:                  always_assert(0);
  }
}

Opcode commuteQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case OpGt:    return OpLt;
  case OpGte:   return OpLte;
  case OpLt:    return OpGt;
  case OpLte:   return OpGte;
  case OpEq:    return OpEq;
  case OpNeq:   return OpNeq;
  case OpSame:  return OpSame;
  case OpNSame: return OpNSame;
  default:      always_assert(0);
  }
}

// Objects compared with strings may involve calling a user-defined
// __toString function.
bool cmpOpTypesMayReenter(Opcode op, Type t0, Type t1) {
  if (op == OpNSame || op == OpSame) return false;
  assert(t0 != Type::Gen && t1 != Type::Gen);
  return (t0 == Type::Cell || t1 == Type::Cell) ||
    ((t0 == Type::Obj || t1 == Type::Obj) &&
     (t0.isString() || t1.isString()));
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
  // copy all but m_id, m_taken, m_listNode
  m_op = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs = nop.m_numSrcs;
  m_srcs = nop.m_srcs;
  m_numDsts = nop.m_numDsts;
  m_dst = nop.m_dst;
  setTaken(nullptr);
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

void IRInstruction::become(IRFactory* factory, IRInstruction* other) {
  assert(other->isTransient() || m_numDsts == other->m_numDsts);
  auto& arena = factory->arena();

  // Copy all but m_id, m_taken.from, m_listNode, and don't clone
  // dests---the whole point of become() is things still point to us.
  m_op = other->m_op;
  m_typeParam = other->m_typeParam;
  m_numSrcs = other->m_numSrcs;
  m_extra = other->m_extra ? cloneExtra(m_op, other->m_extra, arena) : nullptr;
  m_srcs = new (arena) SSATmp*[m_numSrcs];
  std::copy(other->m_srcs, other->m_srcs + m_numSrcs, m_srcs);
  setTaken(other->getTaken());
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

bool Block::isMainExit() const {
  return isMain() && isExit();
}

bool Block::isMain() const {
  return m_trace->isMain();
}

bool Block::isExit() const {
  return !getTaken() && !getNext();
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

std::string IRInstruction::toString() const {
  std::ostringstream str;
  print(str, this);
  return str.str();
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

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

std::string Trace::toString() const {
  std::ostringstream out;
  print(out, this, nullptr);
  return out.str();
}

int32_t spillValueCells(IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->getNumSrcs();
  return numSrcs - 2;
}

bool isConvIntOrPtrToBool(IRInstruction* instr) {
  switch (instr->op()) {
    case ConvIntToBool:
      return true;
    case ConvCellToBool:
      return instr->getSrc(0)->type().subtypeOfAny(
        Type::Func, Type::Cls, Type::FuncCls, Type::VarEnv, Type::TCA);
    default:
      return false;
  }
}

BlockList sortCfg(Trace* trace, const IRFactory& factory) {
  assert(trace->isMain());
  BlockList blocks;
  unsigned next_id = 0;
  postorderWalk(
    [&](Block* block) {
      block->setPostId(next_id++);
      blocks.push_front(block);
    },
    factory.numBlocks(),
    trace->front()
  );
  assert(blocks.size() <= factory.numBlocks());
  assert(next_id <= factory.numBlocks());
  return blocks;
}

bool isRPOSorted(const BlockList& blocks) {
  int id = 0;
  for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
    if ((*it)->postId() != id++) return false;
  }
  return true;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of postorder ids, indexed by postorder id.
 */
IdomVector findDominators(const BlockList& blocks) {
  assert(isRPOSorted(blocks));

  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  auto const num_blocks = blocks.size();
  IdomVector idom(num_blocks, -1);
  auto start = blocks.begin();
  int start_id = (*start)->postId();
  idom[start_id] = start_id;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      Block* block = *it;
      int b = block->postId();
      // new_idom = any already-processed predecessor
      auto edge_it = block->preds().begin();
      int new_idom = edge_it->from()->postId();
      while (idom[new_idom] == -1) new_idom = (++edge_it)->from()->postId();
      // for all other already-processed predecessors p of b
      for (auto& edge : block->preds()) {
        auto p = edge.from()->postId();
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

DomChildren findDomChildren(const BlockList& blocks) {
  IdomVector idom = findDominators(blocks);
  DomChildren children(blocks.size(), BlockPtrList());
  for (Block* block : blocks) {
    int idom_id = idom[block->postId()];
    if (idom_id != -1) children[idom_id].push_front(block);
  }
  return children;
}

bool hasInternalFlow(Trace* trace) {
  for (Block* block : trace->getBlocks()) {
    if (Block* taken = block->getTaken()) {
      if (taken->getTrace() == trace) return true;
    }
  }
  return false;
}

}}

