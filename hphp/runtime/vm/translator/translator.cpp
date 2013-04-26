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
#include "runtime/vm/translator/translator.h"

// Translator front-end: parse instruction stream into basic blocks, decode
// and normalize instructions. Propagate run-time type info to instructions
// to annotate their inputs and outputs with types.
#include <cinttypes>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <string>

#include "folly/Conv.h"

#include "util/trace.h"
#include "util/biased_coin.h"

#include "runtime/base/runtime_option.h"
#include "runtime/base/types.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/bytecode.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-deps.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/annotation.h"
#include "runtime/vm/type_profile.h"
#include "runtime/vm/runtime.h"

namespace HPHP {
namespace VM {
namespace Transl {

using namespace HPHP::VM;

TRACE_SET_MOD(trans)

static __thread BiasedCoin *dbgTranslateCoin;
Translator* transl;
Lease Translator::s_writeLease;

struct TraceletContext {
  TraceletContext() = delete;

  TraceletContext(Tracelet* t, const TypeMap& initialTypes)
    : m_t(t)
    , m_numJmps(0)
    , m_aliasTaint(false)
    , m_varEnvTaint(false)
  {
    for (auto& kv : initialTypes) {
      TRACE(1, "%s\n",
            Trace::prettyNode("InitialType", kv.first, kv.second).c_str());
      m_currentMap[kv.first] = t->newDynLocation(kv.first, kv.second);
    }
  }

  Tracelet*   m_t;
  ChangeMap   m_currentMap;
  DepMap      m_dependencies;
  DepMap      m_resolvedDeps; // dependencies resolved by static analysis
  LocationSet m_changeSet;
  LocationSet m_deletedSet;
  int         m_numJmps;
  bool        m_aliasTaint;
  bool        m_varEnvTaint;

  RuntimeType currentType(const Location& l) const;
  DynLocation* recordRead(const InputInfo& l, bool useHHIR,
                          DataType staticType = KindOfInvalid);
  void recordWrite(DynLocation* dl, NormalizedInstruction* source);
  void recordDelete(const Location& l);
  void recordJmp();
  void aliasTaint();
  void varEnvTaint();
};

void InstrStream::append(NormalizedInstruction* ni) {
  if (last) {
    assert(first);
    last->next = ni;
    ni->prev = last;
    ni->next = nullptr;
    last = ni;
    return;
  }
  assert(!first);
  first = ni;
  last = ni;
  ni->prev = nullptr;
  ni->next = nullptr;
}

void InstrStream::remove(NormalizedInstruction* ni) {
  if (ni->prev) {
    ni->prev->next = ni->next;
  } else {
    first = ni->next;
  }
  if (ni->next) {
    ni->next->prev = ni->prev;
  } else {
    last = ni->prev;
  }
  ni->prev = nullptr;
  ni->next = nullptr;
}

void Tracelet::constructLiveRanges() {
  // Helper function.
  auto considerLoc = [this](DynLocation* dloc,
                            const NormalizedInstruction* ni,
                            bool output) {
    if (!dloc) return;
    Location loc = dloc->location;
    m_liveEnd[loc] = ni->sequenceNum;
    if (output) m_liveDirtyEnd[loc] = ni->sequenceNum;
  };
  // We assign each instruction a sequence number. We do this here, rather
  // than when creating the instruction, to allow splicing and removing
  // instructions
  int sequenceNum = 0;
  for (auto ni = m_instrStream.first; ni; ni = ni->next) {
    ni->sequenceNum = sequenceNum++;
    considerLoc(ni->outLocal, ni, true);
    considerLoc(ni->outStack3, ni, true);
    considerLoc(ni->outStack2, ni, true);
    considerLoc(ni->outStack, ni, true);
    for (auto inp : ni->inputs) {
      considerLoc(inp, ni, false);
    }
  }
}

bool Tracelet::isLiveAfterInstr(Location l,
                                const NormalizedInstruction& ni) const {
  const auto end = m_liveEnd.find(l);
  assert(end != m_liveEnd.end());
  return ni.sequenceNum < end->second;
}

bool Tracelet::isWrittenAfterInstr(Location l,
                                   const NormalizedInstruction& ni) const {
  const auto end = m_liveDirtyEnd.find(l);
  if (end == m_liveDirtyEnd.end()) return false;
  return ni.sequenceNum < end->second;
}

NormalizedInstruction* Tracelet::newNormalizedInstruction() {
  NormalizedInstruction* ni = new NormalizedInstruction();
  m_instrs.push_back(ni);
  return ni;
}

DynLocation* Tracelet::newDynLocation(Location l, DataType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation(Location l, RuntimeType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation() {
  DynLocation* dl = new DynLocation();
  m_dynlocs.push_back(dl);
  return dl;
}

void Tracelet::print() const {
  print(std::cerr);
}

void Tracelet::print(std::ostream& out) const {
  const NormalizedInstruction* i = m_instrStream.first;
  if (i == nullptr) {
    out << "<empty>\n";
    return;
  }

  out << i->unit()->filepath()->data() << ':'
            << i->unit()->getLineNumber(i->offset()) << std::endl;
  for (; i; i = i->next) {
    out << "  " << i->offset() << ": " << i->toString() << std::endl;
  }
}

void
SrcKey::trace(const char *fmt, ...) const {
  if (!Trace::enabled) {
    return;
  }
  // We don't want to print string literals, so don't pass the unit
  string s = instrToString(curUnit()->at(m_offset));
  const char *filepath = "*anonFile*";
  if (curUnit()->filepath()->data() &&
      strlen(curUnit()->filepath()->data()) > 0)
    filepath = curUnit()->filepath()->data();
  Trace::trace("%s:%llx %6d: %20s ",
               filepath, (unsigned long long)getFuncId(),
               m_offset, s.c_str());
  va_list a;
  va_start(a, fmt);
  Trace::vtrace(fmt, a);
  va_end(a);
}

void
SrcKey::print(int ninstrs) const {
  const Unit* u = curUnit();
  Opcode* op = (Opcode*)u->at(m_offset);
  std::cerr << u->filepath()->data() << ':' << u->getLineNumber(m_offset)
            << std::endl;
  for (int i = 0;
       i < ninstrs && (uintptr_t)op < ((uintptr_t)u->entry() + u->bclen());
       op += instrLen(op), ++i) {
    std::cerr << "  " << u->offsetOf(op) << ": " << instrToString(op, u)
              << std::endl;
  }
}

std::string
SrcKey::pretty() const {
  std::ostringstream result;
  const char* filepath = tl_regState == REGSTATE_CLEAN ?
    curUnit()->filepath()->data() : "unknown";
  result << filepath << ':' << getFuncId() << ':' << m_offset;
  return result.str();
}

// advance --
//
//  Move over the current instruction pointer.
void
Translator::advance(const Opcode** instrs) {
  (*instrs) += instrLen(*instrs);
}

/*
 * locPhysicalOffset --
 *
 *   Return offset, in cells, of this location from its base
 *   pointer. It needs a function descriptor to see how many locals
 *   to skip for iterators; if the current frame pointer is not the context
 *   you're looking for, be sure to pass in a non-default f.
 */
int
Translator::locPhysicalOffset(Location l, const Func* f) {
  f = f ? f : curFunc();
  assert_not_implemented(l.space == Location::Stack ||
                         l.space == Location::Local ||
                         l.space == Location::Iter);
  int localsToSkip = l.space == Location::Iter ? f->numLocals() : 0;
  int iterInflator = l.space == Location::Iter ? kNumIterCells : 1;
  return -((l.offset + 1) * iterInflator + localsToSkip);
}

RuntimeType Translator::liveType(Location l, const Unit& u) {
  Cell *outer;
  switch (l.space) {
    case Location::Stack:
      // Stack accesses must be to addresses pushed before
      // translation time; if they are to addresses pushed after,
      // they should be hitting in the changemap.
      assert(locPhysicalOffset(l) >= 0);
      // fallthru
    case Location::Local: {
      Cell *base;
      int offset = locPhysicalOffset(l);
      base    = l.space == Location::Stack ? vmsp() : vmfp();
      outer = &base[offset];
    } break;
    case Location::Iter: {
      const Iter *it = frame_iter(curFrame(), l.offset);
      TRACE(1, "Iter input: fp %p, iter %p, offset %" PRId64 "\n", vmfp(),
            it, l.offset);
      return RuntimeType(it);
    } break;
    case Location::Litstr: {
      return RuntimeType(u.lookupLitstrId(l.offset));
    } break;
    case Location::Litint: {
      return RuntimeType(l.offset);
    } break;
    case Location::This: {
      return outThisObjectType();
    } break;
    default: {
      not_reached();
    }
  }
  assert(IS_REAL_TYPE(outer->m_type));
  return liveType(outer, l);
}

RuntimeType
Translator::liveType(const Cell* outer, const Location& l) {
  always_assert(analysisDepth() == 0);

  if (!outer) {
    // An undefined global; starts out as a variant null
    return RuntimeType(KindOfRef, KindOfNull);
  }
  DataType outerType = (DataType)outer->m_type;
  assert(IS_REAL_TYPE(outerType));
  DataType valueType = outerType;
  const Cell* valCell = outer;
  if (outerType == KindOfRef) {
    // Variant. Pick up the inner type, too.
    valCell = outer->m_data.pref->tv();
    DataType innerType = valCell->m_type;
    assert(IS_REAL_TYPE(innerType));
    valueType = innerType;
    assert(innerType != KindOfRef);
    TRACE(2, "liveType Var -> %d\n", innerType);
    return RuntimeType(KindOfRef, innerType);
  }
  const Class *klass = nullptr;
  if (valueType == KindOfObject) {
    // TODO: Infer the class, too.
    if (false) {
      klass = valCell->m_data.pobj->getVMClass();
    }
  }
  TRACE(2, "liveType %d\n", outerType);
  RuntimeType retval = RuntimeType(outerType, KindOfInvalid, klass);
  return retval;
}

RuntimeType Translator::outThisObjectType() {
  /*
   * Use the current method's context class (ctx) as a constraint.
   * For instance methods, if $this is non-null, we are guaranteed
   * that $this is an instance of ctx or a class derived from
   * ctx. Zend allows this assumption to be violated but we have
   * deliberately chosen to diverge from them here.
   *
   * Note that if analysisDepth() != 0 we'll have !hasThis() here,
   * because our fake ActRec has no $this, but we'll still return the
   * correct object type because arGetContextClass() looks at
   * ar->m_func's class for methods.
   */
  const Class *ctx = curFunc()->isMethod() ?
    arGetContextClass(curFrame()) : nullptr;
  if (ctx) {
    assert(!curFrame()->hasThis() ||
           curFrame()->getThis()->getVMClass()->classof(ctx));
    TRACE(2, "OutThisObject: derived from Class \"%s\"\n",
          ctx->name()->data());
    return RuntimeType(KindOfObject, KindOfInvalid, ctx);
  }
  return RuntimeType(KindOfObject, KindOfInvalid);
}

bool Translator::liveFrameIsPseudoMain() {
  ActRec* ar = (ActRec*)vmfp();
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

Location
Translator::tvToLocation(const TypedValue* tv, const TypedValue* frame) {
  const Cell *arg0 = frame + locPhysicalOffset(Location(Location::Local, 0));
  // Physical stack offsets grow downwards from the frame pointer. See
  // locPhysicalOffset.
  int offset = -(tv - arg0);
  assert(offset >= 0);
  assert(offset < ((ActRec*)frame)->m_func->numLocals());
  TRACE(2, "tvToLocation: %p -> L:%d\n", tv, offset);
  return Location(Location::Local, offset);
}

/* Opcode type-table. */
enum OutTypeConstraints {
  OutNull,
  OutNullUninit,
  OutString,
  OutStringImm,         // String w/ precisely known immediate.
  OutDouble,
  OutBoolean,
  OutBooleanImm,
  OutInt64,
  OutArray,
  OutArrayImm,
  OutObject,
  OutThisObject,        // Object from current environment
  OutFDesc,             // Blows away the current function desc

  OutUnknown,           // Not known at tracelet compile-time
  OutPred,              // Unknown, but give prediction a whirl.
  OutCns,               // Constant; may be known at compile-time
  OutVUnknown,          // type is V(unknown)

  OutSameAsInput,       // type is the same as the first stack inpute
  OutCInput,            // type is C(input)
  OutVInput,            // type is V(input)
  OutCInputL,           // type is C(type) of local input
  OutVInputL,           // type is V(type) of local input
  OutFInputL,           // type is V(type) of local input if current param is
                        //   by ref, else type is C(type) of local input
  OutFInputR,           // Like FInputL, but for R's on the stack.

  OutArith,             // For Add, Sub, Mul
  OutBitOp,             // For BitAnd, BitOr, BitXor
  OutSetOp,             // For SetOpL
  OutIncDec,            // For IncDecL
  OutClassRef,          // KindOfClass
  OutNone
};

/*
 * Input codes indicate what an instruction reads, and some other
 * things about their behavior.  The order these show up in the inputs
 * vector is given in getInputs(), and is relevant in a few cases
 * (e.g. instructions taking both stack inputs and MVectors).
 */
enum Operands {
  None            = 0,
  Stack3          = 1 << 0,
  Stack2          = 1 << 1,
  Stack1          = 1 << 2,
  StackIns1       = 1 << 3,  // Insert an element under top of stack
  StackIns2       = 1 << 4,  // Insert an element under top 2 of stack
  FuncdRef        = 1 << 5,  // Input to FPass*
  FStack          = 1 << 6,  // output of FPushFuncD and friends
  Local           = 1 << 7,  // Writes to a local
  MVector         = 1 << 8,  // Member-vector input
  Iter            = 1 << 9,  // Iterator in imm[0]
  AllLocals       = 1 << 10, // All locals (used by RetC)
  DontGuardLocal  = 1 << 11, // Dont force a guard on behalf of the local input
  DontGuardStack1 = 1 << 12, // Dont force a guard on behalf of stack1 input
  DontBreakLocal  = 1 << 13, // Dont break a tracelet on behalf of the local
  DontBreakStack1 = 1 << 14, // Dont break a tracelet on behalf of stack1 input
  IgnoreInnerType = 1 << 15, // Instruction doesnt care about the inner types
  DontGuardAny    = 1 << 16, // Dont force a guard for any input
  This            = 1 << 17, // Input to CheckThis
  StackN          = 1 << 18, // pop N cells from stack; n = imm[0].u_IVA
  BStackN         = 1 << 19, // consume N cells from stack for builtin call;
                             // n = imm[0].u_IVA
  StackTop2 = Stack1 | Stack2,
  StackTop3 = Stack1 | Stack2 | Stack3,
  StackCufSafe = StackIns1 | FStack
};

Operands
operator|(const Operands& l, const Operands& r) {
  return Operands(int(r) | int(l));
}

static int64_t typeToMask(DataType t) {
  return (t == KindOfInvalid) ? 1 : (1 << (1 + getDataTypeIndex(t)));
}

struct InferenceRule {
  int64_t mask;
  DataType result;
};

static DataType inferType(const InferenceRule* rules,
                          const vector<DynLocation*>& inputs) {
  int inputMask = 0;
  // We generate the inputMask by ORing together the mask for each input's
  // type.
  for (unsigned int i = 0; i < inputs.size(); ++i) {
    DataType inType = inputs[i]->rtt.valueType();
    inputMask |= typeToMask(inType);
  }
  // This loop checks each rule in order, looking for the first rule that
  // applies. Note that we assume there's a "catch-all" at the end.
  for (unsigned int i = 0; ; ++i) {
    if (rules[i].mask == 0 || (rules[i].mask & inputMask) != 0) {
      return rules[i].result;
    }
  }
  // We return KindOfInvalid by default if none of the rules applied.
  return KindOfInvalid;
}

/*
 * Inference rules used for OutArith. These are applied in order
 * row-by-row.
 */

#define TYPE_MASK(name) \
  static const int64_t name ## Mask = typeToMask(KindOf ## name);
TYPE_MASK(Invalid);
TYPE_MASK(Uninit);
TYPE_MASK(Null);
TYPE_MASK(Boolean);
static const int64_t IntMask = typeToMask(KindOfInt64);
TYPE_MASK(Double);
static const int64_t StringMask = typeToMask(KindOfString) |
                                typeToMask(KindOfStaticString);
TYPE_MASK(Array);
TYPE_MASK(Object);

static const InferenceRule ArithRules[] = {
  { DoubleMask, KindOfDouble },
  { ArrayMask, KindOfArray },
  // If one of the inputs is known to be a String or if one of the input
  // types is unknown, the output type is Unknown
  { StringMask | InvalidMask, KindOfInvalid },
  // Default to Int64
  { 0, KindOfInt64 },
};

static const int NumArithRules = sizeof(ArithRules) / sizeof(InferenceRule);

/**
 * Returns the type of the output of a bitwise operator on the two
 * DynLocs. The only case that doesn't result in KindOfInt64 is String
 * op String.
 */
static const InferenceRule BitOpRules[] = {
  { UninitMask | NullMask | BooleanMask |
    IntMask | DoubleMask | ArrayMask | ObjectMask,
    KindOfInt64 },
  { StringMask, KindOfString },
  { 0, KindOfInvalid },
};

static RuntimeType bitOpType(DynLocation* a, DynLocation* b) {
  vector<DynLocation*> ins;
  ins.push_back(a);
  if (b) ins.push_back(b);
  return RuntimeType(inferType(BitOpRules, ins));
}

static uint32_t m_w = 1;    /* must not be zero */
static uint32_t m_z = 1;    /* must not be zero */

static uint32_t get_random()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

static const int kTooPolyPred = 2;
static const int kTooPolyRet = 6;

static std::pair<DataType,double>
predictMVec(const NormalizedInstruction* ni) {
  auto info = getFinalPropertyOffset(*ni,
                                     curFunc()->cls(),
                                     getMInstrInfo(ni->mInstrOp()));
  if (info.offset != -1 && info.hphpcType != KindOfInvalid) {
    FTRACE(1, "prediction for CGetM prop: {}, hphpc\n",
           int(info.hphpcType));
    return std::make_pair(info.hphpcType, 1.0);
  }

  auto& immVec = ni->immVec;
  StringData* name;
  MemberCode mc;
  if (immVec.decodeLastMember(curUnit(), name, mc)) {
    auto pred = predictType(TypeProfileKey(mc, name));
    TRACE(1, "prediction for CGetM %s named %s: %d, %f\n",
          mc == MET ? "elt" : "prop",
          name->data(),
          pred.first,
          pred.second);
    return pred;
  }

  return std::make_pair(KindOfInvalid, 0.0);
}

/*
 * predictOutputs --
 *
 *   Provide a best guess for the output type of this instruction.
 */
static DataType
predictOutputs(const Tracelet& t,
               NormalizedInstruction* ni) {
  if (RuntimeOption::EvalJitStressTypePredPercent &&
      RuntimeOption::EvalJitStressTypePredPercent > int(get_random() % 100)) {
    ni->outputPredicted = true;
    int dt;
    while (true) {
      dt = get_random() % (KindOfRef + 1);
      switch (dt) {
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfString:
        case KindOfArray:
        case KindOfObject:
          break;
        // KindOfRef and KindOfUninit can't happen for lots of predicted
        // types.
        case KindOfRef:
        case KindOfUninit:
        default:
          continue;
      }
      break;
    }
    return DataType(dt);
  }

  if (ni->op() == OpMod) {
    // x % 0 returns boolean false, so we don't know for certain, but it's
    // probably an int.
    return KindOfInt64;
  }

  if (ni->op() == OpDiv) {
    // Integers can produce integers if there's no residue, but $i / $j in
    // general produces a double. $i / 0 produces boolean false, so we have
    // actually check the result.
    return KindOfDouble;
  }

  if (ni->op() == OpClsCnsD) {
    const NamedEntityPair& cne =
      curFrame()->m_func->unit()->lookupNamedEntityPairId(ni->imm[1].u_SA);
    StringData* cnsName = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
    Class* cls = cne.second->getCachedClass();
    if (cls) {
      DataType dt = cls->clsCnsType(cnsName);
      if (dt != KindOfUninit) {
        ni->outputPredicted = true;
        TRACE(1, "clscnsd: %s:%s prediction type %d\n",
              cne.first->data(), cnsName->data(), dt);
        return dt;
      }
    }
  }

  static const double kAccept = 1.0;
  std::pair<DataType, double> pred = std::make_pair(KindOfInvalid, 0.0);
  // Type predictions grow tracelets, and can have a side effect of making
  // them combinatorially explode if they bring in precondtions that vary a
  // lot. Get more conservative as evidence mounts that this is a
  // polymorphic tracelet.
  if (tx64->numTranslations(t.m_sk) >= kTooPolyPred) return KindOfInvalid;
  if (hasImmVector(ni->op())) {
    pred = predictMVec(ni);
  }
  if (debug && pred.second < kAccept) {
    if (const StringData* invName = fcallToFuncName(ni)) {
      pred = predictType(TypeProfileKey(TypeProfileKey::MethodName, invName));
      TRACE(1, "prediction for methods named %s: %d, %f\n",
            invName->data(),
            pred.first,
            pred.second);
    }
  }
  if (pred.second >= kAccept) {
    ni->outputPredicted = true;
    TRACE(1, "accepting prediction of type %d\n", pred.first);
    assert(pred.first != KindOfUninit);
    return pred.first;
  }
  return KindOfInvalid;
}

/**
 * Returns the type of the value a SetOpL will store into the local.
 */
static RuntimeType setOpOutputType(NormalizedInstruction* ni,
                                   const vector<DynLocation*>& inputs) {
  assert(inputs.size() == 2);
  const int kValIdx = 0;
  const int kLocIdx = 1;
  unsigned char op = ni->imm[1].u_OA;
  DynLocation locLocation(inputs[kLocIdx]->location,
                          inputs[kLocIdx]->rtt.unbox());
  assert(inputs[kLocIdx]->location.isLocal());
  switch (op) {
    case SetOpPlusEqual:
    case SetOpMinusEqual:
    case SetOpMulEqual: {
      // Same as OutArith, except we have to fiddle with inputs a bit.
      vector<DynLocation*> arithInputs;
      arithInputs.push_back(&locLocation);
      arithInputs.push_back(inputs[kValIdx]);
      return RuntimeType(inferType(ArithRules, arithInputs));
    }
    case SetOpConcatEqual: return RuntimeType(KindOfString);
    case SetOpDivEqual:
    case SetOpModEqual:    return RuntimeType(KindOfInvalid);
    case SetOpAndEqual:
    case SetOpOrEqual:
    case SetOpXorEqual:    return bitOpType(&locLocation, inputs[kValIdx]);
    case SetOpSlEqual:
    case SetOpSrEqual:     return RuntimeType(KindOfInt64);
    default:
      assert(false);
  }
  NOT_REACHED();
  return RuntimeType(KindOfInvalid);
}

static RuntimeType
getDynLocType(const vector<DynLocation*>& inputs,
              const Tracelet& t,
              Opcode opcode,
              NormalizedInstruction* ni,
              Operands op,
              OutTypeConstraints constraint,
              DynLocation* outDynLoc) {
  assert(constraint != OutFInputL);

  switch (constraint) {
#define CS(OutXLike, KindOfX) \
    case OutXLike:            \
      return RuntimeType(KindOfX);
    CS(OutInt64,       KindOfInt64);
    CS(OutBoolean,     KindOfBoolean);
    CS(OutDouble,      KindOfDouble);
    CS(OutString,      KindOfString);
    CS(OutNull,        KindOfNull);
    CS(OutUnknown,     KindOfInvalid); // Subtle interaction with BB-breaking.
    CS(OutFDesc,       KindOfInvalid); // Unclear if OutFDesc has a purpose.
    CS(OutArray,       KindOfArray);
    CS(OutObject,      KindOfObject);
#undef CS
    case OutPred: return RuntimeType(predictOutputs(t, ni));

    case OutClassRef: {
      Op op = Op(ni->op());
      if ((op == OpAGetC && inputs[0]->isString())) {
        const StringData *sd = inputs[0]->rtt.valueString();
        if (sd) {
          Class *klass = Unit::lookupUniqueClass(sd);
          TRACE(3, "KindOfClass: derived class \"%s\" from string literal\n",
                klass ? klass->preClass()->name()->data() : "NULL");
          return RuntimeType(klass);
        }
      } else if (op == OpSelf) {
        return RuntimeType(curClass());
      } else if (op == OpParent) {
        Class* clss = curClass();
        if (clss != nullptr)
          return RuntimeType(clss->parent());
      }
      return RuntimeType(KindOfClass);
    }

    case OutCns: {
      // If it's a system constant, burn in its type. Otherwise we have
      // to accept prediction; use the translation-time value, or fall back
      // to the targetcache if none exists.
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      assert(sd);
      const TypedValue* tv = Unit::lookupPersistentCns(sd);
      if (tv) {
        return RuntimeType(tv->m_type);
      }
      tv = Unit::lookupCns(sd);
      if (tv) {
        ni->outputPredicted = true;
        TRACE(1, "CNS %s: guessing runtime type %d\n", sd->data(), tv->m_type);
        return RuntimeType(tv->m_type);
      }
      return RuntimeType(KindOfInvalid);
    }

    case OutNullUninit: {
      assert(ni->op() == OpNullUninit);
      return RuntimeType(KindOfUninit);
    }

    case OutStringImm: {
      assert(ni->op() == OpString);
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      assert(sd);
      return RuntimeType(sd);
    }

    case OutArrayImm: {
      assert(ni->op() == OpArray);
      ArrayData *ad = curUnit()->lookupArrayId(ni->imm[0].u_AA);
      assert(ad);
      return RuntimeType(ad);
    }

    case OutBooleanImm: {
      assert(ni->op() == OpTrue || ni->op() == OpFalse);
      return RuntimeType(ni->op() == OpTrue);
    }

    case OutThisObject: {
      return Translator::outThisObjectType();
    }

    case OutVUnknown: {
      return RuntimeType(KindOfRef, KindOfInvalid);
    }

    case OutArith: {
      return RuntimeType(inferType(ArithRules, inputs));
    }

    case OutSameAsInput: {
      /*
       * Relies closely on the order that inputs are pushed in
       * getInputs().  (Pushing top of stack first for multi-stack
       * consumers, stack elements before M-vectors and locals, etc.)
       */
      assert(inputs.size() >= 1);
      Opcode op = ni->op();
      ASSERT_NOT_IMPLEMENTED(
        // Sets and binds that take multiple arguments have the rhs
        // pushed first.  In the case of the M-vector versions, the
        // rhs comes before the M-vector elements.
        op == OpSetL  || op == OpSetN  || op == OpSetG  || op == OpSetS  ||
        op == OpBindL || op == OpBindG || op == OpBindS || op == OpBindN ||
        op == OpSetM  || op == OpBindM ||
        // Dup takes a single element.
        op == OpDup
      );

      if (op == OpSetM || op == OpBindM) {
        /*
         * these return null for "invalid" inputs
         * if we cant prove everything's ok, we will
         * have to insert a side exit
         */
        bool ok = inputs.size() <= 3;
        if (ok) {
          switch (inputs[1]->rtt.valueType()) {
            case KindOfObject:
              ok = mcodeMaybePropName(ni->immVecM[0]);
              break;
            case KindOfArray:
              ok = mcodeMaybeArrayKey(ni->immVecM[0]);
              break;
            case KindOfNull:
            case KindOfUninit:
              break;
            default:
              ok = false;
          }
        }
        if (ok) {
          for (int i = inputs.size(); --i >= 2; ) {
            switch (inputs[i]->rtt.valueType()) {
              case KindOfObject:
              case KindOfArray:
                ok = false;
                break;
              default:
                continue;
            }
            break;
          }
        }
        if (!ok) {
          ni->outputPredicted = true;
        }
      }

      const int idx = 0; // all currently supported cases.

      if (debug) {
        if (!inputs[idx]->rtt.isVagueValue()) {
          if (op == OpBindG || op == OpBindN || op == OpBindS ||
              op == OpBindM || op == OpBindL) {
            assert(inputs[idx]->rtt.isRef() && !inputs[idx]->isLocal());
          } else {
            assert(inputs[idx]->rtt.valueType() ==
                   inputs[idx]->rtt.outerType());
          }
        }
      }
      return inputs[idx]->rtt;
    }

    case OutCInputL: {
      assert(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      RuntimeType retval;
      if (in->rtt.outerType() == KindOfUninit) {
        // Locals can be KindOfUninit, so we need to convert
        // this to KindOfNull
        retval = RuntimeType(KindOfNull);
      } else {
        retval = in->rtt.unbox();
      }
      TRACE(2, "Input (%d, %d) -> (%d, %d)\n",
            in->rtt.outerType(), in->rtt.innerType(),
            retval.outerType(), retval.innerType());
      return retval;
    }

    case OutIncDec: {
      const RuntimeType &inRtt = ni->inputs[0]->rtt;
      // TODO: instead of KindOfInvalid this should track the actual
      // type we will get from interping a non-int IncDec.
      return RuntimeType(IS_INT_TYPE(inRtt.valueType()) ?
                         KindOfInt64 : KindOfInvalid);
    }

    case OutCInput: {
      assert(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      if (in->rtt.outerType() == KindOfRef) {
        return in->rtt.unbox();
      }
      return in->rtt;
    }

    case OutBitOp: {
      assert(inputs.size() == 2 ||
             (inputs.size() == 1 && opcode == OpBitNot));
      if (inputs.size() == 2) {
        return bitOpType(inputs[0], inputs[1]);
      } else {
        return bitOpType(inputs[0], nullptr);
      }
    }

    case OutSetOp: {
      return setOpOutputType(ni, inputs);
    }

    case OutNone:
    default:
      return RuntimeType(KindOfInvalid);
  }
}

/*
 * NB: this opcode structure is sparse; it cannot just be indexed by
 * opcode.
 */
struct InstrInfo {
  Operands           in;
  Operands           out;
  OutTypeConstraints type;       // How are outputs related to inputs?
  int                stackDelta; // Impact on stack: # cells *pushed*
};

static const struct {
  Opcode    op;
  InstrInfo info;
} instrInfoSparse [] = {

  // Op             Inputs            Outputs       OutputTypes    Stack delta
  // --             ------            -------       -----------    -----------

  /*** 1. Basic instructions ***/

  { OpNop,         {None,             None,         OutNone,           0 }},
  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone,          -1 }},
  { OpPopV,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpPopR,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpDup,         {Stack1,           StackTop2,    OutSameAsInput,    1 }},
  { OpBox,         {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnbox,       {Stack1,           Stack1,       OutCInput,         0 }},
  { OpBoxR,        {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput,         0 }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull,           1 }},
  { OpNullUninit,  {None,             Stack1,       OutNullUninit,     1 }},
  { OpTrue,        {None,             Stack1,       OutBooleanImm,     1 }},
  { OpFalse,       {None,             Stack1,       OutBooleanImm,     1 }},
  { OpInt,         {None,             Stack1,       OutInt64,          1 }},
  { OpDouble,      {None,             Stack1,       OutDouble,         1 }},
  { OpString,      {None,             Stack1,       OutStringImm,      1 }},
  { OpArray,       {None,             Stack1,       OutArrayImm,       1 }},
  { OpNewArray,    {None,             Stack1,       OutArray,          1 }},
  { OpNewTuple,    {StackN,           Stack1,       OutArray,          0 }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpNewCol,      {None,             Stack1,       OutObject,         1 }},
  { OpColAddElemC, {StackTop3,        Stack1,       OutObject,        -2 }},
  { OpColAddNewElemC, {StackTop2,     Stack1,       OutObject,        -1 }},
  { OpCns,         {None,             Stack1,       OutCns,            1 }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpClsCnsD,     {None,             Stack1,       OutPred,           1 }},
  { OpFile,        {None,             Stack1,       OutString,         1 }},
  { OpDir,         {None,             Stack1,       OutString,         1 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  /* Logical ops */
  { OpXor,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNot,         {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpSame,        {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNSame,       {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEq,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNeq,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  /* Bitwise ops */
  { OpBitAnd,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitOr,       {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitXor,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitNot,      {Stack1,           Stack1,       OutBitOp,          0 }},
  { OpShl,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  { OpShr,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  /* Cast instructions */
  { OpCastBool,    {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpCastInt,     {Stack1,           Stack1,       OutInt64,          0 }},
  { OpCastDouble,  {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCastString,  {Stack1,           Stack1,       OutString,         0 }},
  { OpCastArray,   {Stack1,           Stack1,       OutArray,          0 }},
  { OpCastObject,  {Stack1,           Stack1,       OutObject,         0 }},
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64,          0 }},
  { OpClone,       {Stack1,           Stack1,       OutObject,         0 }},
  { OpExit,        {Stack1,           None,         OutNone,          -1 }},
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpZ,        {Stack1,           None,         OutNone,          -1 }},
  { OpJmpNZ,       {Stack1,           None,         OutNone,          -1 }},
  { OpSwitch,      {Stack1,           None,         OutNone,          -1 }},
  { OpSSwitch,     {Stack1,           None,         OutNone,          -1 }},
  /*
   * RetC and RetV are special. Their manipulation of the runtime stack are
   * outside the boundaries of the tracelet abstraction; since they always end
   * a basic block, they behave more like "glue" between BBs than the
   * instructions in the body of a BB.
   *
   * RetC and RetV consume a value from the stack, and this value's type needs
   * to be known at compile-time.
   */
  { OpRetC,        {AllLocals,        None,         OutNone,           0 }},
  { OpRetV,        {AllLocals,        None,         OutNone,           0 }},
  { OpThrow,       {Stack1,           None,         OutNone,          -1 }},
  { OpUnwind,      {None,             None,         OutNone,           0 }},

  /*** 5. Get instructions ***/

  { OpCGetL,       {Local,            Stack1,       OutCInputL,        1 }},
  { OpCGetL2,      {Stack1|Local,     StackIns1,    OutCInputL,        1 }},
  { OpCGetL3,      {StackTop2|Local,  StackIns2,    OutCInputL,        1 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutPred,           1 }},
  { OpVGetL,       {Local,            Stack1,       OutVInputL,        1 }},
  { OpVGetN,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  // TODO: In pseudo-main, the VGetG instruction invalidates what we know
  // about the types of the locals because it could cause any one of the
  // local variables to become "boxed". We need to add logic to tracelet
  // analysis to deal with this properly.
  { OpVGetG,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  { OpVGetS,       {StackTop2,        Stack1,       OutVUnknown,      -1 }},
  { OpVGetM,       {MVector,          Stack1|Local, OutVUnknown,       1 }},
  { OpAGetC,       {Stack1,           Stack1,       OutClassRef,       0 }},
  { OpAGetL,       {Local,            Stack1,       OutClassRef,       1 }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpIssetN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpEmptyL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpEmptyN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEmptyM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpIsNullC,     {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsBoolC,     {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsIntC,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsDoubleC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsStringC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsArrayC,    {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsObjectC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsNullL,     {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsBoolL,     {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsIntL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsDoubleL,   {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsStringL,   {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsArrayL,    {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsObjectL,   {Local,            Stack1,       OutBoolean,        1 }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput,    0 }},
  { OpSetN,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetG,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutSameAsInput,    0 }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp,          0 }},
  { OpSetOpN,      {StackTop2,        Stack1|Local, OutUnknown,       -1 }},
  { OpSetOpG,      {StackTop2,        Stack1|Local, OutUnknown,       -1 }},
  { OpSetOpS,      {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpSetOpM,      {MVector|Stack1,   Stack1|Local, OutUnknown,        0 }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec,         1 }},
  { OpIncDecN,     {Stack1,           Stack1|Local, OutUnknown,        0 }},
  { OpIncDecG,     {Stack1,           Stack1|Local, OutUnknown,        0 }},
  { OpIncDecS,     {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpIncDecM,     {MVector,          Stack1,       OutUnknown,        1 }},
  { OpBindL,       {Stack1|Local|
                    IgnoreInnerType,  Stack1|Local, OutSameAsInput,    0 }},
  { OpBindN,       {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpBindG,       {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpBindS,       {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpBindM,       {MVector|Stack1,   Stack1|Local, OutSameAsInput,    0 }},
  { OpUnsetL,      {Local,            Local,        OutNone,           0 }},
  { OpUnsetN,      {Stack1,           Local,        OutNone,          -1 }},
  { OpUnsetG,      {Stack1,           Local,        OutNone,          -1 }},
  { OpUnsetM,      {MVector,          None,         OutNone,           0 }},

  /*** 8. Call instructions ***/

  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushFuncD,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushObjMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushObjMethodD,
                   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushClsMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodF,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushCtor,   {Stack1,           Stack1|FStack,OutObject,
                                                         kNumActRecCells }},
  { OpFPushCtorD,  {None,             Stack1|FStack,OutObject,
                                                     kNumActRecCells + 1 }},
  { OpFPushCuf,    {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufF,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufSafe,{StackTop2|DontGuardAny,
                                      StackCufSafe, OutFDesc,
                                                         kNumActRecCells }},
  { OpFPassC,      {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassCW,     {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassCE,     {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassV,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassL,      {Local|FuncdRef,   Stack1,       OutFInputL,        1 }},
  { OpFPassN,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassS,      {StackTop2|FuncdRef,
                                      Stack1,       OutUnknown,       -1 }},
  { OpFPassM,      {MVector|FuncdRef, Stack1,       OutUnknown,        1 }},
  { OpBPassC,      {None,             None,         OutNull,           0 }},
  { OpBPassV,      {None,             None,         OutNull,           0 }},
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallArray,  {FStack,           Stack1,       OutPred,
                                                   -(int)kNumActRecCells }},
  // TODO: output type is known
  { OpFCallBuiltin,{BStackN,          Stack1,       OutPred,          0 }},
  { OpCufSafeArray,{StackTop3|DontGuardAny,
                                      Stack1,       OutArray,         -2 }},
  { OpCufSafeReturn,{StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown,       -2 }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInit,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterInitK,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInitK,  {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterNext,    {None,             Local,        OutUnknown,        0 }},
  { OpMIterNext,   {None,             Local,        OutUnknown,        0 }},
  { OpIterNextK,   {None,             Local,        OutUnknown,        0 }},
  { OpMIterNextK,  {None,             Local,        OutUnknown,        0 }},
  { OpIterFree,    {None,             None,         OutNone,           0 }},
  { OpMIterFree,   {None,             None,         OutNone,           0 }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpDefFunc,     {None,             None,         OutNone,           0 }},
  { OpDefTypedef,  {None,             None,         OutNone,           0 }},
  { OpDefCls,      {None,             None,         OutNone,           0 }},
  { OpDefCns,      {Stack1,           Stack1,       OutBoolean,        0 }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject,     1 }},
  { OpBareThis,    {None,             Stack1,       OutUnknown,        1 }},
  { OpCheckThis,   {This,             None,         OutNone,           0 }},
  { OpInitThisLoc,
                   {None,             Local,        OutUnknown,        0 }},
  { OpStaticLoc,
                   {None,             Stack1,       OutBoolean,        1 }},
  { OpStaticLocInit,
                   {Stack1,           Local,        OutVUnknown,      -1 }},
  { OpCatch,       {None,             Stack1,       OutObject,         1 }},
  { OpVerifyParamType,
                   {Local,            None,         OutNone,           0 }},
  { OpClassExists, {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpInterfaceExists,
                   {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpTraitExists, {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpSelf,        {None,             Stack1,       OutClassRef,       1 }},
  { OpParent,      {None,             Stack1,       OutClassRef,       1 }},
  { OpLateBoundCls,{None,             Stack1,       OutClassRef,       1 }},
  { OpNativeImpl,  {None,             None,         OutNone,           0 }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject,         1 }},

  /*** 14. Continuation instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutObject,         1 }},
  { OpContEnter,   {None,             None,         OutNone,           0 }},
  { OpContExit,    {None,             None,         OutNone,           0 }},
  { OpUnpackCont,  {Local,            Stack1,       OutInt64,          1 }},
  { OpPackCont,    {Local|Stack1,     None,         OutNone,          -1 }},
  { OpContReceive, {Local,            Stack1,       OutUnknown,        1 }},
  { OpContRetC,    {Local|Stack1,     None,         OutNone,          -1 }},
  { OpContNext,    {None,             None,         OutNone,           0 }},
  { OpContSend,    {Local,            None,         OutNone,           0 }},
  { OpContRaise,   {Local,            None,         OutNone,           0 }},
  { OpContValid,   {None,             Stack1,       OutBoolean,        1 }},
  { OpContCurrent, {None,             Stack1,       OutUnknown,        1 }},
  { OpContStopped, {None,             None,         OutNone,           0 }},
  { OpContHandle,  {Stack1,           None,         OutNone,          -1 }},
  { OpStrlen,      {Stack1,           Stack1,       OutInt64,          0 }},
  { OpIncStat,     {None,             None,         OutNone,           0 }},
};

static hphp_hash_map<Opcode, InstrInfo> instrInfo;
static bool instrInfoInited;
static void initInstrInfo() {
  if (!instrInfoInited) {
    for (size_t i = 0; i < sizeof(instrInfoSparse) / sizeof(instrInfoSparse[0]);
         i++) {
      instrInfo[instrInfoSparse[i].op] = instrInfoSparse[i].info;
    }

    instrInfoInited = true;
  }
}

static int numHiddenStackInputs(const NormalizedInstruction& ni) {
  assert(ni.immVec.isValid());
  return ni.immVec.numStackValues();
}

int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  Opcode op = ni.op();
  switch (op) {
    case OpFCall: {
      int numArgs = ni.imm[0].u_IVA;
      return 1 - numArgs - kNumActRecCells;
    }

    case OpFCallBuiltin:
    case OpNewTuple:
    case OpCreateCl:
      return 1 - ni.imm[0].u_IVA;
  }
  const InstrInfo& info = instrInfo[op];
  if (info.in & MVector) {
    hiddenStackInputs = numHiddenStackInputs(ni);
    SKTRACE(2, ni.source, "Has %d hidden stack inputs\n", hiddenStackInputs);
  }
  int delta = instrInfo[op].stackDelta - hiddenStackInputs;
  return delta;
}

/*
 * analyzeSecondPass --
 *
 *   Whole-tracelet analysis pass, after we've set up the dataflow
 *   graph. Modifies the instruction stream, and further annotates
 *   individual instructions.
 */
void Translator::analyzeSecondPass(Tracelet& t) {
  assert(t.m_instrStream.last);
  NormalizedInstruction* next;
  for (NormalizedInstruction* ni = t.m_instrStream.first; ni; ni = next) {
    const Opcode op = ni->op();
    next = ni->next;

    if (op == OpNop) {
      t.m_instrStream.remove(ni);
      continue;
    }

    if (op == OpCGetL) {
      /*
       * If the local isn't more broadly useful in this tracelet, don't bother
       * allocating space for it.
       */
      const Location& local = ni->inputs[0]->location;

      bool seen = false;
      for (NormalizedInstruction* cur = ni->next; cur; cur = cur->next) {
        if ((ni->m_txFlags & Native) != Native) break;
        for (unsigned dli = 0; dli < cur->inputs.size(); ++dli) {
          Location& loc = cur->inputs[dli]->location;
          if (loc == local) {
            SKTRACE(1, ni->source, "CGetL: loading input\n");
            seen = true;
            break;
          }
        }
      }

      ni->manuallyAllocInputs = !seen && !ni->inputs[0]->rtt.isUninit();
      SKTRACE(1, ni->source, "CGetL: manuallyAllocInputs: %d\n",
              ni->manuallyAllocInputs);
      continue;
    }

    NormalizedInstruction* prev = ni->prev;
    if (!prev || !(prev->m_txFlags & Supported)) continue;
    const Opcode prevOp = prev->op();

    if (!ni->next &&
        (op == OpJmpZ || op == OpJmpNZ)) {
      if (prevOp == OpNot && (ni->m_txFlags & Supported)) {
        ni->invertCond = !ni->invertCond;
        ni->inputs[0] = prev->inputs[0];
        assert(!prev->deadLocs.size());
        t.m_instrStream.remove(prev);
        next = ni;
        continue;
      }
      if (prevOp == OpGt || prevOp == OpLt ||
          prevOp == OpGte || prevOp == OpLte ||
          prevOp == OpEq || prevOp == OpNeq ||
          prevOp == OpIssetL || prevOp == OpAKExists ||
          isTypePred(prevOp) || prevOp == OpInstanceOfD ||
          prev->fuseBranch) {
        prev->breaksTracelet = true;
        prev->changesPC = true; // Dont generate generic glue.
        // Leave prev->next linked. The translator will end up needing it. The
        // breaksTracelet annotation here will prevent us from really translating the
        // Jmp*.
        continue;
      }
    }

    if (op == OpFPushClsMethodF && ni->directCall &&
        prevOp == OpAGetC &&
        prev->prev && prev->prev->op() == OpString &&
        prev->prev->prev && prev->prev->prev->op() == OpString) {
      /*
       * We have a fully determined OpFPushClsMethodF. We dont
       * need to put the class and method name strings, or the
       * Class* into registers.
       */
      prev->outStack = nullptr;
      prev->prev->outStack = nullptr;
      prev->prev->prev->outStack = nullptr;
    }

    if (RuntimeOption::RepoAuthoritative &&
        prevOp == OpFPushCtorD &&
        !prev->noCtor &&
        prev->imm[0].u_IVA == 0 &&
        op == OpFCall && (ni->m_txFlags & Supported) &&
        ni->next && (ni->next->m_txFlags & Supported) &&
        ni->next->op() == OpPopR) {
      /* new obj with a ctor that takes no args */
      const NamedEntityPair& np =
        curUnit()->lookupNamedEntityPairId(prev->imm[1].u_SA);
      const Class* cls = Unit::lookupUniqueClass(np.second);
      if (cls && (cls->attrs() & AttrUnique) &&
          Func::isSpecial(cls->getCtor()->name())) {
        /* its the generated 86ctor, so no need to call it */
        next = next->next;
        t.m_instrStream.remove(ni->next);
        t.m_instrStream.remove(ni);
        prev->noCtor = 1;
        SKTRACE(1, prev->source, "FPushCtorD: killing ctor for %s in %s\n",
                np.first->data(), curFunc()->fullName()->data());
        continue;
      }
    }

    /*
     * If this is a Pop instruction and the previous instruction pushed a
     * single return value cell on the stack, we can roll the pop into the
     * previous instruction.
     *
     * TODO: SetG/SetS?
     */
    const bool isPop = op == OpPopC || op == OpPopV;
    const bool isOptimizable = prevOp == OpSetL ||
      prevOp == OpBindL ||
      prevOp == OpIncDecL ||
      prevOp == OpPrint ||
      prevOp == OpSetM ||
      prevOp == OpSetOpM ||
      prevOp == OpIncDecM;

    if (isPop && isOptimizable) {
      // If one of these instructions already has a null outStack, we
      // already hoisted a pop into it.
      const bool alreadyHoisted = !prev->outStack;

      if (!alreadyHoisted) {
        prev->outStack = nullptr;
        SKTRACE(3, ni->source, "hoisting Pop instruction in analysis\n");
        for (unsigned i = 0; i < ni->deadLocs.size(); ++i) {
          prev->deadLocs.push_back(ni->deadLocs[i]);
        }
        t.m_instrStream.remove(ni);
        if ((prevOp == OpSetM || prevOp == OpSetOpM || prevOp == OpIncDecM) &&
            prev->prev && prev->prev->op() == OpCGetL &&
            prev->prev->inputs[0]->outerType() != KindOfUninit) {
          assert(prev->prev->outStack);
          prev->prev->outStack = 0;
          prev->prev->manuallyAllocInputs = true;
          prev->prev->ignoreInnerType = true;
          prev->inputs[0] = prev->prev->inputs[0];
          prev->grouped = true;
        }
        continue;
      }
    }

    /*
     * A Not instruction following an Is* instruction can
     * be folded.
     */
    if (op == OpNot) {
      switch (prevOp) {
        case OpAKExists:
        case OpIssetL:
        case OpIsNullL:   case OpIsNullC:
        case OpIsBoolL:   case OpIsBoolC:
        case OpIsIntL:    case OpIsIntC:
        case OpIsDoubleL: case OpIsDoubleC:
        case OpIsStringL: case OpIsStringC:
        case OpIsArrayL:  case OpIsArrayC:
        case OpIsObjectL: case OpIsObjectC:
          prev->invertCond = !prev->invertCond;
          prev->outStack = ni->outStack;
          SKTRACE(3, ni->source, "folding Not instruction in analysis\n");
          assert(!ni->deadLocs.size());
          t.m_instrStream.remove(ni);
          continue;
      }
    }

    if (op == OpInstanceOfD && prevOp == OpCGetL &&
        (ni->m_txFlags & Supported)) {
      assert(prev->outStack);
      ni->inputs[0] = prev->inputs[0];
      /*
        the CGetL becomes a no-op (other
        than checking for UninitNull), but
        we mark the InstanceOfD as grouped to
        avoid breaking the tracelet between the
        two.
      */
      prev->ignoreInnerType = true;
      prev->outStack = 0;
      prev->manuallyAllocInputs = true;
      ni->grouped = true;
    }

    if ((op == OpInstanceOfD || op == OpIsNullC) &&
        (ni->m_txFlags & Supported) &&
        (prevOp == OpThis || prevOp == OpBareThis)) {
      prev->outStack = 0;
      ni->grouped = true;
      ni->manuallyAllocInputs = true;
    }

    /*
     * TODO: #1181258 this should mostly be subsumed by the IR.
     * Remove this once the IR is seen to be handling it.
     */
    NormalizedInstruction* pp = nullptr;
    if (prevOp == OpString &&
        (ni->m_txFlags & Supported)) {
      switch (op) {
        case OpReqDoc:
          /* Dont waste a register on the string */
          prev->outStack = nullptr;
          pp = prev->prev;
      }
    }

    if (op == OpRetC && (ni->m_txFlags & Supported) &&
        (prevOp == OpString ||
         prevOp == OpInt ||
         prevOp == OpNull ||
         prevOp == OpTrue ||
         prevOp == OpFalse ||
         prevOp == OpDouble ||
         prevOp == OpArray ||
         prevOp == OpThis ||
         prevOp == OpBareThis)) {
      assert(!ni->outStack);
      ni->grouped = true;
      prev->outStack = nullptr;
      pp = prev->prev;
    }

    if (pp && pp->op() == OpPopC &&
        pp->m_txFlags == Native) {
      NormalizedInstruction* ppp = prev->prev->prev;
      if (ppp && (ppp->m_txFlags & Supported)) {
        switch (ppp->op()) {
          case OpReqDoc:
            /*
              We have a require+pop followed by a require or a scalar ret,
              where the pop doesnt have to do any work (the pop is Native).
              There is no need to inc/dec rbx between the two (since
              there will be no code between them)
            */
            ppp->outStack = nullptr;
            ni->skipSync = true;
            break;

          default:
            // do nothing
            break;
        }
      }
    }
  }
}

static NormalizedInstruction* findInputSrc(NormalizedInstruction* ni,
                                           DynLocation* dl) {
  while (ni != nullptr) {
    if (ni->outStack == dl ||
        ni->outLocal == dl ||
        ni->outLocal2 == dl ||
        ni->outStack2 == dl ||
        ni->outStack3 == dl) {
      break;
    }
    ni = ni->prev;
  }
  return ni;
}

/*
 * For MetaData information that affects whether we want to even put a
 * value in the ni->inputs, we need to look at it before we call
 * getInputs(), so this is separate from applyInputMetaData.
 *
 * We also check GuardedThis here, since RetC is short-circuited in
 * applyInputMetaData.
 */
void Translator::preInputApplyMetaData(Unit::MetaHandle metaHand,
                                       NormalizedInstruction* ni) {
  if (!metaHand.findMeta(ni->unit(), ni->offset())) return;

  Unit::MetaInfo info;
  while (metaHand.nextArg(info)) {
    switch (info.m_kind) {
    case Unit::MetaInfo::NonRefCounted:
      ni->nonRefCountedLocals.resize(curFunc()->numLocals());
      ni->nonRefCountedLocals[info.m_data] = 1;
      break;
    case Unit::MetaInfo::GuardedThis:
      ni->guardedThis = true;
      break;
    default:
      break;
    }
  }
}

bool Translator::applyInputMetaData(Unit::MetaHandle& metaHand,
                                    NormalizedInstruction* ni,
                                    TraceletContext& tas,
                                    InputInfos &inputInfos) {
  if (!metaHand.findMeta(ni->unit(), ni->offset())) return false;

  Unit::MetaInfo info;
  if (!metaHand.nextArg(info)) return false;
  if (info.m_kind == Unit::MetaInfo::NopOut) {
    ni->noOp = true;
    return true;
  }

  /*
   * We need to adjust the indexes in MetaInfo::m_arg if this
   * instruction takes other stack arguments than those related to the
   * MVector.  (For example, the rhs of an assignment.)
   */
  const InstrInfo& iInfo = instrInfo[ni->op()];
  if (iInfo.in & AllLocals) {
    /*
     * RetC/RetV dont care about their stack input, but it may have
     * been annotated. Skip it (because RetC/RetV pretend they dont
     * have a stack input).
     */
    return false;
  }
  if (iInfo.in == FuncdRef) {
    /*
     * FPassC* pretend to have no inputs
     */
    return false;
  }
  const int base = !(iInfo.in & MVector) ? 0 :
                   !(iInfo.in & Stack1) ? 0 :
                   !(iInfo.in & Stack2) ? 1 :
                   !(iInfo.in & Stack3) ? 2 : 3;

  do {
    SKTRACE(3, ni->source, "considering MetaInfo of kind %d\n", info.m_kind);

    int arg = info.m_arg & Unit::MetaInfo::VectorArg ?
      base + (info.m_arg & ~Unit::MetaInfo::VectorArg) : info.m_arg;

    switch (info.m_kind) {
      case Unit::MetaInfo::NoSurprise:
        ni->noSurprise = true;
        break;
      case Unit::MetaInfo::GuardedCls:
        ni->guardedCls = true;
        break;
      case Unit::MetaInfo::ArrayCapacity:
        ni->imm[0].u_IVA = info.m_data;
        break;
      case Unit::MetaInfo::DataTypePredicted: {
        // If the original type was invalid or predicted, then use the
        // prediction in the meta-data.
        assert((unsigned) arg < inputInfos.size());

        SKTRACE(1, ni->source, "MetaInfo DataTypePredicted for input %d; "
                "newType = %d\n", arg, DataType(info.m_data));
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, false, KindOfInvalid);
        NormalizedInstruction* src = findInputSrc(tas.m_t->m_instrStream.last,
                                                  dl);
        if (src) {
          // Update the rtt and mark src's output as predicted if either:
          //  a) we don't have type information yet (ie, it's KindOfInvalid), or
          //  b) src's output was predicted. This is assuming that the
          //     front-end's prediction is more accurate.
          if (dl->rtt.outerType() == KindOfInvalid || src->outputPredicted) {
            SKTRACE(1, ni->source, "MetaInfo DataTypePredicted for input %d; "
                    "replacing oldType = %d with newType = %d\n", arg,
                    dl->rtt.outerType(), DataType(info.m_data));
            dl->rtt = RuntimeType((DataType)info.m_data);
            src->outputPredicted = true;
            src->outputPredictionStatic = true;
          }
        }
        break;
      }
      case Unit::MetaInfo::DataTypeInferred: {
        assert((unsigned)arg < inputInfos.size());
        SKTRACE(1, ni->source, "MetaInfo DataTypeInferred for input %d; "
                   "newType = %d\n", arg, DataType(info.m_data));
        InputInfo& ii = inputInfos[arg];
        ii.dontGuard = true;
        DynLocation* dl = tas.recordRead(ii, m_useHHIR, (DataType)info.m_data);
        if (dl->rtt.outerType() != info.m_data &&
            (!dl->isString() || info.m_data != KindOfString)) {
          if (dl->rtt.outerType() != KindOfInvalid) {
            // Either static analysis is wrong, or
            // this was mis-predicted by the type
            // profiler, or this code is unreachable,
            // and there's an earlier bytecode in the tracelet
            // thats going to fatal
            NormalizedInstruction *src = nullptr;
            if (mapContains(tas.m_changeSet, dl->location)) {
              src = findInputSrc(tas.m_t->m_instrStream.last, dl);
              if (src && src->outputPredicted) {
                src->outputPredicted = false;
              } else {
                src = nullptr;
              }
            }
            if (!src) {
              // Not a type-profiler mis-predict
              if (tas.m_t->m_instrStream.first) {
                // We're not the first instruction, so punt
                // If this bytecode /is/ reachable, we'll
                // get here again, and that time, we will
                // be the first instruction
                punt();
              }
              not_reached();
            }
          }
          dl->rtt = RuntimeType((DataType)info.m_data);
          ni->markInputInferred(arg);
        } else {
          /*
           * Static inference confirmed the expected type
           * but if the expected type was provided by the type
           * profiler we want to clear outputPredicted to
           * avoid unneeded guards
           */
          if (mapContains(tas.m_changeSet, dl->location)) {
            NormalizedInstruction *src =
              findInputSrc(tas.m_t->m_instrStream.last, dl);
            if (src->outputPredicted) {
              src->outputPredicted = false;
              ni->markInputInferred(arg);
            }
          }
        }
        break;
      }

      case Unit::MetaInfo::String: {
        const StringData* sd = ni->unit()->lookupLitstrId(info.m_data);
        assert((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        ii.dontGuard = true;
        DynLocation* dl = tas.recordRead(ii, m_useHHIR, KindOfString);
        assert(!dl->rtt.isString() || !dl->rtt.valueString() ||
               dl->rtt.valueString() == sd);
        SKTRACE(1, ni->source, "MetaInfo on input %d; old type = %s\n",
                arg, dl->pretty().c_str());
        dl->rtt = RuntimeType(sd);
        break;
      }

      case Unit::MetaInfo::Class: {
        assert((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, m_useHHIR);
        if (dl->rtt.valueType() != KindOfObject) {
          continue;
        }

        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        const StringData* rttName =
          dl->rtt.valueClass() ? dl->rtt.valueClass()->name() : nullptr;
        // The two classes might not be exactly the same, which is ok
        // as long as metaCls is more derived than rttCls.
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        Class* rttCls = rttName ? Unit::lookupUniqueClass(rttName) : nullptr;
        if (metaCls && rttCls && metaCls != rttCls &&
            !metaCls->classof(rttCls)) {
          // Runtime type is more derived
          metaCls = 0;
        }
        if (metaCls && metaCls != rttCls) {
          SKTRACE(1, ni->source, "replacing input %d with a MetaInfo-supplied "
                  "class of %s; old type = %s\n",
                  arg, metaName->data(), dl->pretty().c_str());
          if (dl->rtt.isRef()) {
            dl->rtt = RuntimeType(KindOfRef, KindOfObject, metaCls);
          } else {
            dl->rtt = RuntimeType(KindOfObject, KindOfInvalid, metaCls);
          }
        }
        break;
      }

      case Unit::MetaInfo::MVecPropClass: {
        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        if (metaCls) {
          ni->immVecClasses[arg] = metaCls;
        }
        break;
      }

      case Unit::MetaInfo::NopOut:
        // NopOut should always be the first and only annotation
        // and was handled above.
        not_reached();

      case Unit::MetaInfo::GuardedThis:
      case Unit::MetaInfo::NonRefCounted:
        // fallthrough; these are handled in preInputApplyMetaData.
      case Unit::MetaInfo::None:
        break;
    }
  } while (metaHand.nextArg(info));

  return false;
}

static void addMVectorInputs(NormalizedInstruction& ni,
                             int& currentStackOffset,
                             std::vector<InputInfo>& inputs) {
  assert(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  currentStackOffset -= ni.immVec.numStackValues();
  int localStackOffset = currentStackOffset;

  auto push_stack = [&] {
    ++stackCount;
    inputs.emplace_back(Location(Location::Stack, localStackOffset++));
  };
  auto push_local = [&] (int imm) {
    ++localCount;
    inputs.emplace_back(Location(Location::Local, imm));
  };

  /*
   * Note that we have to push as we go so that the arguments come in
   * the order expected for the M-vector.
   *
   * Indexes into these argument lists must also be in the same order
   * as the information in Unit::MetaInfo, because the analysis phase
   * may replace some of them with literals.
   */

  /*
   * Also note: if we eventually have immediates that are not local
   * ids (i.e. string ids), this analysis step is going to have to be
   * a bit wiser.
   */
  const uint8_t* vec = ni.immVec.vec();
  const LocationCode lcode = LocationCode(*vec++);

  const bool trailingClassRef = lcode == LSL || lcode == LSC;

  switch (numLocationCodeStackVals(lcode)) {
  case 0: {
    if (lcode == LH) {
      inputs.emplace_back(Location(Location::This));
    } else {
      assert(lcode == LL || lcode == LGL || lcode == LNL);
      int numImms = numLocationCodeImms(lcode);
      for (int i = 0; i < numImms; ++i) {
        push_local(decodeVariableSizeImm(&vec));
      }
    }
  } break;
  case 1:
    if (lcode == LSL) {
      // We'll get the trailing stack value after pushing all the
      // member vector elements.
      push_local(decodeVariableSizeImm(&vec));
    } else {
      push_stack();
    }
    break;
  case 2:
    push_stack();
    if (!trailingClassRef) {
      // This one is actually at the back.
      push_stack();
    }
    break;
  default: not_reached();
  }

  // Now push all the members in the correct order.
  while (vec - ni.immVec.vec() < ni.immVec.size()) {
    const MemberCode mcode = MemberCode(*vec++);
    ni.immVecM.push_back(mcode);

    if (mcode == MW) {
      // No stack and no locals.
    } else if (memberCodeHasImm(mcode)) {
      int64_t imm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsLoc(mcode)) {
        push_local(imm);
      } else if (memberCodeImmIsString(mcode)) {
        inputs.emplace_back(Location(Location::Litstr, imm));
      } else {
        assert(memberCodeImmIsInt(mcode));
        inputs.emplace_back(Location(Location::Litint, imm));
      }
    } else {
      push_stack();
    }
    inputs.back().dontGuardInner = true;
  }

  if (trailingClassRef) {
    push_stack();
  }

  ni.immVecClasses.resize(ni.immVecM.size());

  assert(vec - ni.immVec.vec() == ni.immVec.size());
  assert(stackCount == ni.immVec.numStackValues());

  SKTRACE(2, ni.source, "M-vector using %d hidden stack "
                        "inputs, %d locals\n", stackCount, localCount);
}

/*
 * getInputs --
 *   Returns locations for this instruction's inputs.
 *
 * Throws:
 *   TranslationFailedExc:
 *     Unimplemented functionality, probably an opcode.
 *
 *   UnknownInputExc:
 *     Consumed a datum whose type or value could not be constrained at
 *     translation time, because the tracelet has already modified it.
 *     Truncate the tracelet at the preceding instruction, which must
 *     exists because *something* modified something in it.
 */
void Translator::getInputs(Tracelet& t,
                           NormalizedInstruction* ni,
                           int& currentStackOffset,
                           InputInfos& inputs,
                           const TraceletContext& tas) {
#ifdef USE_TRACE
  const SrcKey& sk = ni->source;
#endif
  assert(inputs.empty());
  if (debug && !mapContains(instrInfo, ni->op())) {
    fprintf(stderr, "Translator does not understand "
      "instruction %s\n", opcodeToName(ni->op()));
    assert(false);
  }
  const InstrInfo& info = instrInfo[ni->op()];
  Operands input = info.in;
  if (input & FuncdRef) {
    inputs.needsRefCheck = true;
  }
  if (input & Iter) {
    inputs.emplace_back(Location(Location::Iter, ni->imm[0].u_IVA));
  }
  if (input & FStack) {
    currentStackOffset -= ni->imm[0].u_IVA; // arguments consumed
    currentStackOffset -= kNumActRecCells; // ActRec is torn down as well
  }
  if (input & IgnoreInnerType) ni->ignoreInnerType = true;
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", currentStackOffset - 1);
    inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & DontBreakStack1) inputs.back().dontBreak = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      }
    }
  }
  if (input & StackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: stackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (input & BStackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    }
  }
  if (input & MVector) {
    addMVectorInputs(*ni, currentStackOffset, inputs);
  }
  if (input & Local) {
    // Many of the continuation instructions read local 0. All other
    // instructions that take a Local have its index at their first
    // immediate.
    int loc;
    switch (ni->op()) {
      case OpUnpackCont:
      case OpPackCont:
      case OpContReceive:
      case OpContRetC:
      case OpContSend:
      case OpContRaise:
        loc = 0;
        break;

      case OpFPassL:
        loc = ni->imm[1].u_IVA;
        break;

      default:
        loc = ni->imm[0].u_IVA;
        break;
    }
    SKTRACE(1, sk, "getInputs: local %d\n", loc);
    inputs.emplace_back(Location(Location::Local, loc));
    if (input & DontGuardLocal) inputs.back().dontGuard = true;
    if (input & DontBreakLocal) inputs.back().dontBreak = true;
  }

  const bool wantInlineReturn = [&] {
    const int localCount = curFunc()->numLocals();
    // Inline return causes us to guard this tracelet more precisely. If
    // we're already chaining to get here, just do a generic return in the
    // hopes of avoiding further specialization. The localCount constraint
    // is an unfortunate consequence of the current generic machinery not
    // working for 0 locals.
    if (tx64->numTranslations(t.m_sk) >= kTooPolyRet && localCount > 0) {
      return false;
    }
    ni->nonRefCountedLocals.resize(localCount);
    int numRefCounted = 0;
    for (int i = 0; i < localCount; ++i) {
      auto curType = tas.currentType(Location(Location::Local, i));
      if (ni->nonRefCountedLocals[i]) {
        assert(!curType.isRefCounted() && "Static analysis was wrong");
      }
      numRefCounted += curType.isRefCounted();
    }
    return numRefCounted <= kMaxInlineReturnDecRefs;
  }();

  if ((input & AllLocals) && wantInlineReturn) {
    ni->inlineReturn = true;
    ni->ignoreInnerType = true;
    int n = curFunc()->numLocals();
    for (int i = 0; i < n; ++i) {
      if (!ni->nonRefCountedLocals[i]) {
        inputs.emplace_back(Location(Location::Local, i));
      }
    }
  }

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", currentStackOffset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if (inputs.size() &&
      ((input & DontGuardAny) || dontGuardAnyInputs(ni->op()))) {
    for (int i = inputs.size(); i--; ) {
      inputs[i].dontGuard = true;
    }
  }
  if (input & This) {
    inputs.emplace_back(Location(Location::This));
  }
}

bool outputDependsOnInput(const Opcode instr) {
  switch (instrInfo[instr].type) {
    case OutNull:
    case OutNullUninit:
    case OutString:
    case OutStringImm:
    case OutDouble:
    case OutBoolean:
    case OutBooleanImm:
    case OutInt64:
    case OutArray:
    case OutArrayImm:
    case OutObject:
    case OutThisObject:
    case OutUnknown:
    case OutVUnknown:
    case OutClassRef:
    case OutPred:
    case OutCns:
    case OutNone:
      return false;
    case OutFDesc:
    case OutSameAsInput:
    case OutCInput:
    case OutVInput:
    case OutCInputL:
    case OutVInputL:
    case OutFInputL:
    case OutFInputR:
    case OutArith:
    case OutBitOp:
    case OutSetOp:
    case OutIncDec:
      return true;
  }
  not_reached();
}

/*
 * getOutputs --
 *   Builds a vector describing this instruction's outputs. Also
 *   records any write to a value that *might* alias a local.
 *
 * Throws:
 *   TranslationFailedExc:
 *     Unimplemented functionality, probably an opcode.
 */
void Translator::getOutputs(/*inout*/ Tracelet& t,
                            /*inout*/ NormalizedInstruction* ni,
                            /*inout*/ int& currentStackOffset,
                            /*out*/   bool& varEnvTaint) {
  varEnvTaint = false;

  const vector<DynLocation*>& inputs = ni->inputs;
  Op op = ni->op();

  initInstrInfo();
  assert_not_implemented(instrInfo.find(op) != instrInfo.end());
  const Operands outLocs = instrInfo[op].out;
  const OutTypeConstraints typeInfo = instrInfo[op].type;

  SKTRACE(1, ni->source, "output flavor %d\n", typeInfo);
  if (typeInfo == OutFInputL || typeInfo == OutFInputR ||
      typeInfo == OutVInputL) {
    // Variable number of outputs. If we box the loc we're reading,
    // we need to write out its boxed-ness.
    assert(inputs.size() >= 1);
    const DynLocation* in = inputs[inputs.size() - 1];
    DynLocation* outDynLoc = t.newDynLocation(in->location, in->rtt);
    outDynLoc->location = Location(Location::Stack, currentStackOffset++);
    bool isRef;
    if (typeInfo == OutVInputL) {
      isRef = true;
    } else {
      assert(typeInfo == OutFInputL || typeInfo == OutFInputR);
      isRef = ni->preppedByRef;
    }
    if (isRef) {
      // Locals can be KindOfUninit, so we need to convert
      // this to KindOfNull
      if (in->rtt.outerType() == KindOfUninit) {
        outDynLoc->rtt = RuntimeType(KindOfRef, KindOfNull);
      } else {
        outDynLoc->rtt = in->rtt.box();
      }
      SKTRACE(1, ni->source, "boxed type: %d -> %d\n",
              outDynLoc->rtt.outerType(), outDynLoc->rtt.innerType());
    } else {
      if (outDynLoc->rtt.outerType() == KindOfUninit) {
        outDynLoc->rtt = RuntimeType(KindOfNull);
      } else {
        outDynLoc->rtt = outDynLoc->rtt.unbox();
      }
      SKTRACE(1, ni->source, "unboxed type: %d\n",
              outDynLoc->rtt.outerType());
    }
    assert(outDynLoc->location.isStack());
    ni->outStack = outDynLoc;

    if (isRef && in->rtt.outerType() != KindOfRef &&
        typeInfo != OutFInputR &&
        in->location.isLocal()) {
      // VGetH or FPassH boxing a local
      DynLocation* smashedLocal =
          t.newDynLocation(in->location, outDynLoc->rtt);
      assert(smashedLocal->location.isLocal());
      ni->outLocal = smashedLocal;
    }
    // Other things that might be getting boxed here include globals
    // and array values; since we don't attempt to track these things'
    // types in symbolic execution anyway, we can ignore them.
    return;
  }

  int opnd = None;
  for (int outLocsCopy = (int)outLocs;
       outLocsCopy != (int)None;
       outLocsCopy &= ~opnd) {
    opnd = 1 << (ffs(outLocsCopy) - 1);
    assert(opnd != None && opnd != Stack3);  // no instr produces 3 values
    assert(opnd != FuncdRef);                // reffiness is immutable
    Location loc;
    switch (opnd) {
      // Pseudo-outputs that affect translator state
      case FStack: {
        currentStackOffset += kNumActRecCells;
        if (op == OpFPushFuncD) {
          const Unit& cu = *ni->unit();
          Id funcId = ni->imm[1].u_SA;
          const NamedEntityPair &nep = cu.lookupNamedEntityPairId(funcId);
          const Func* f = Unit::lookupFunc(nep.second, nep.first);
          if (f && f->isNameBindingImmutable(&cu)) {
            t.m_arState.pushFuncD(f);
          } else {
            t.m_arState.pushDynFunc();
          }
        } else {
          // Non-deterministic in some way
          t.m_arState.pushDynFunc();
        }
      } continue; // no instr-associated output

      case Local: {
        if (op == OpSetN || op == OpSetOpN || op == OpIncDecN ||
            op == OpBindN || op == OpUnsetN) {
          varEnvTaint = true;
          continue;
        }
        ASSERT_NOT_IMPLEMENTED(op == OpSetOpL ||
                               op == OpSetM || op == OpSetOpM ||
                               op == OpBindM ||
                               op == OpIncDecL || op == OpIncDecG ||
                               op == OpUnsetG || op == OpBindG ||
                               op == OpSetG || op == OpSetOpG ||
                               op == OpVGetM ||
                               op == OpStaticLocInit || op == OpInitThisLoc ||
                               op == OpSetL || op == OpBindL ||
                               op == OpUnsetL ||
                               op == OpIterInit || op == OpIterInitK ||
                               op == OpMIterInit || op == OpMIterInitK ||
                               op == OpIterNext || op == OpIterNextK ||
                               op == OpMIterNext || op == OpMIterNextK);
        if (op == OpIncDecL) {
          assert(ni->inputs.size() == 1);
          const RuntimeType &inRtt = ni->inputs[0]->rtt;
          RuntimeType rtt = IS_INT_TYPE(inRtt.valueType()) ? inRtt :
            RuntimeType(KindOfInvalid);
          DynLocation* incDecLoc =
            t.newDynLocation(ni->inputs[0]->location, rtt);
          assert(incDecLoc->location.isLocal());
          ni->outLocal = incDecLoc;
          continue; // Doesn't mutate a loc's types for int. Carry on.
        }
        if (op == OpSetG || op == OpSetOpG ||
            op == OpUnsetG || op == OpBindG ||
            op == OpIncDecG) {
          continue;
        }
        if (op == OpUnsetL) {
          assert(ni->inputs.size() == 1);
          DynLocation* inLoc = ni->inputs[0];
          assert(inLoc->location.isLocal());
          RuntimeType newLhsRtt = RuntimeType(KindOfUninit);
          Location locLocation = inLoc->location;
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  locLocation.spaceName(), locLocation.offset,
                  newLhsRtt.valueType());
          DynLocation* unsetLoc = t.newDynLocation(locLocation, newLhsRtt);
          assert(unsetLoc->location.isLocal());
          ni->outLocal = unsetLoc;
          continue;
        }
        if (op == OpStaticLocInit || op == OpInitThisLoc) {
          ni->outLocal = t.newDynLocation(Location(Location::Local,
                                                   ni->imm[0].u_OA),
                                          KindOfInvalid);
          continue;
        }
        if (op == OpSetM || op == OpSetOpM || op == OpVGetM || op == OpBindM) {
          // TODO(#1069330): This code assumes that the location is
          // LH. We need to figure out how to handle cases where the
          // location is LN or LG or LR.
          // XXX: analogous garbage needed for OpSetOpM.
          if (ni->immVec.locationCode() == LL) {
            const int kVecStart = (op == OpSetM ||
                                   op == OpSetOpM ||
                                   op == OpBindM) ?
              1 : 0; // 0 is rhs for SetM/SetOpM
            DynLocation* inLoc = ni->inputs[kVecStart];
            assert(inLoc->location.isLocal());
            Location locLoc = inLoc->location;
            if (inLoc->rtt.isString() ||
                inLoc->rtt.valueType() == KindOfBoolean) {
              // Strings and bools produce value-dependent results; "" and
              // false upgrade to an array successfully, while other values
              // fail and leave the lhs unmodified.
              DynLocation* baseLoc = t.newDynLocation(locLoc, KindOfInvalid);
              assert(baseLoc->isLocal());
              ni->outLocal = baseLoc;
            } else if (inLoc->rtt.valueType() == KindOfUninit ||
                       inLoc->rtt.valueType() == KindOfNull) {
              RuntimeType newLhsRtt = inLoc->rtt.setValueType(
                mcodeMaybePropName(ni->immVecM[0]) ?
                KindOfObject : KindOfArray);
              SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                      locLoc.spaceName(), locLoc.offset, newLhsRtt.valueType());
              DynLocation* baseLoc = t.newDynLocation(locLoc, newLhsRtt);
              assert(baseLoc->location.isLocal());
              ni->outLocal = baseLoc;
            }
            // Note (if we start translating pseudo-mains):
            //
            // A SetM in pseudo-main might alias a local whose type we're
            // remembering:
            //
            //   $GLOBALS['a'] = 123; // $a :: Int
            //
            // and more deviously:
            //
            //   $loc['b'][17] = $GLOBALS; $x = 'b'; $y = 17;
            //   $loc[$x][$y]['a'] = 123; // $a :: Int
          }
          continue;
        }
        if (op == OpSetOpL) {
          const int kLocIdx = 1;
          DynLocation* inLoc = ni->inputs[kLocIdx];
          assert(inLoc->location.isLocal());
          DynLocation* dl = t.newDynLocation();
          dl->location = inLoc->location;
          dl->rtt = setOpOutputType(ni, ni->inputs);
          if (inLoc->isRef()) {
            dl->rtt = dl->rtt.box();
          }
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  inLoc->location.spaceName(), inLoc->location.offset,
                  dl->rtt.valueType());
          assert(dl->location.isLocal());
          ni->outLocal = dl;
          continue;
        }
        if (op >= OpIterInit && op <= OpMIterNextK) {
          assert(op == OpIterInit || op == OpIterInitK ||
                 op == OpMIterInit || op == OpMIterInitK ||
                 op == OpIterNext || op == OpIterNextK ||
                 op == OpMIterNext || op == OpMIterNextK);
          const int kValImmIdx = 2;
          const int kKeyImmIdx = 3;
          DynLocation* outVal = t.newDynLocation();
          int off = ni->imm[kValImmIdx].u_IVA;
          outVal->location = Location(Location::Local, off);
          if (op == OpMIterInit || op == OpMIterInitK ||
              op == OpMIterNext || op == OpMIterNextK) {
            outVal->rtt = RuntimeType(KindOfRef, KindOfInvalid);
          } else {
            outVal->rtt = RuntimeType(KindOfInvalid);
          }
          ni->outLocal = outVal;
          if (op == OpIterInitK || op == OpIterNextK) {
            DynLocation* outKey = t.newDynLocation();
            int keyOff = getImm(ni->pc(), kKeyImmIdx).u_IVA;
            outKey->location = Location(Location::Local, keyOff);
            outKey->rtt = RuntimeType(KindOfInvalid);
            ni->outLocal2 = outKey;
          } else if (op == OpMIterInitK || op == OpMIterNextK) {
            DynLocation* outKey = t.newDynLocation();
            int keyOff = getImm(ni->pc(), kKeyImmIdx).u_IVA;
            outKey->location = Location(Location::Local, keyOff);
            outKey->rtt = RuntimeType(KindOfRef, KindOfInvalid);
            ni->outLocal2 = outKey;
          }
          continue;
        }
        assert(ni->inputs.size() == 2);
        const int kValIdx  = 0;
        const int kLocIdx = 1;
        DynLocation* inLoc = ni->inputs[kLocIdx];
        DynLocation* inVal  = ni->inputs[kValIdx];
        Location locLocation = inLoc->location;
        // Variant RHS possible only when binding.
        assert(inVal->rtt.isVagueValue() ||
               (op == OpBindL) ==
               (inVal->rtt.outerType() == KindOfRef));
        assert(!inVal->location.isLocal());
        assert(inLoc->location.isLocal());
        RuntimeType newLhsRtt = inVal->rtt.isVagueValue() || op == OpBindL ?
          inVal->rtt :
          inLoc->rtt.setValueType(inVal->rtt.outerType());
        if (inLoc->rtt.outerType() == KindOfRef) {
          assert(newLhsRtt.outerType() == KindOfRef);
        } else {
          assert(op == OpBindL ||
                 newLhsRtt.outerType() != KindOfRef);
        }
        SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                locLocation.spaceName(), locLocation.offset,
                inVal->rtt.valueType());
        DynLocation* outLhsLoc = t.newDynLocation(locLocation, newLhsRtt);
        assert(outLhsLoc->location.isLocal());
        ni->outLocal = outLhsLoc;
      } continue; // already pushed an output for the local

      case Stack1:
      case Stack2:
        loc = Location(Location::Stack, currentStackOffset++);
        break;
      case StackIns1: {
        // First stack output is where the inserted element will go.
        // The output code for the instruction will affect what we
        // think about this location.
        loc = Location(Location::Stack, currentStackOffset++);

        // The existing top is just being moved up a notch.  This one
        // always functions as if it were OutSameAsInput.
        assert(ni->inputs.size() >= 1);
        ni->outStack2 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[0]->rtt
        );
      } break;
      case StackIns2: {
        // Similar to StackIns1.
        loc = Location(Location::Stack, currentStackOffset++);

        // Move the top two locations up a slot.
        assert(ni->inputs.size() >= 2);
        ni->outStack2 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[1]->rtt
        );
        ni->outStack3 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[0]->rtt
        );
      } break;
      default:
        not_reached();
    }
    DynLocation* dl = t.newDynLocation();
    dl->location = loc;
    dl->rtt = getDynLocType(inputs, t, op, ni, (Operands)opnd,
                            typeInfo, dl);
    SKTRACE(2, ni->source, "recording output t(%d->%d) #(%s, %d)\n",
            dl->rtt.outerType(), dl->rtt.innerType(),
            dl->location.spaceName(), dl->location.offset);
    assert(dl->location.isStack());
    ni->outStack = dl;
  }
}

void
Translator::findImmable(ImmStack &stack,
                        NormalizedInstruction* ni) {
  switch (ni->op()) {
  case OpInt:
    stack.pushInt(getImm(ni->pc(), 0).u_I64A);
    return;

  case OpString:
    stack.pushLitstr(getImm(ni->pc(), 0).u_SA);
    return;

  // For binary ops we assume that only one of the two is an immediate
  //   because if both were immediates then hopefully the second pass
  //   optimized away this instruction. However, if a binary op has two
  //   immediates, we won't generate incorrect code: instead it will
  //   merely be suboptimal.

  // we can only handle an immediate if it's the second immediate
  case OpAdd:
  case OpSub:
    if (stack.isInt(0)) {
      SKTRACE(1, ni->source, "marking for immediate elision\n");
      ni->hasConstImm = true;
      ni->constImm.u_I64A = stack.get(0).i64a;
      // We don't currently remove the OpInt instruction that produced
      // this integer. We should update the translator to correctly support
      // removing instructions from the tracelet.
    }
    break;

  case OpFPassM:
  case OpCGetM:
  case OpSetM:
  case OpIssetM: {
    // If this is "<VecInstr>M <... EC>"
    const ImmVector& iv = ni->immVec;
    assert(iv.isValid());
    MemberCode mc;
    StringData* str;
    int64_t strId;
    if (iv.size() > 1 &&
        iv.decodeLastMember(curUnit(), str, mc, &strId) &&
        mc == MET) {
      /*
       * If the operand takes a literal string that's not strictly an
       * integer, we can call into array-access helper functions that
       * don't bother with the integer check.
       */
      int64_t lval;
      if (LIKELY(!str->isStrictlyInteger(lval))) {
        ni->hasConstImm = true;
        ni->constImm.u_SA = strId;
      }
    }
  } break;

  default: ;
  }

  stack.processOpcode(ni->pc());
}

void
Translator::requestResetHighLevelTranslator() {
  if (dbgTranslateCoin) {
    dbgTranslateCoin->reset();
  }
}

bool DynLocation::canBeAliased() const {
  return isValue() &&
    ((Translator::liveFrameIsPseudoMain() && isLocal()) || isRef());
}

// Test the type of a location without recording it as a read yet.
RuntimeType TraceletContext::currentType(const Location& l) const {
  DynLocation* dl;
  if (!mapGet(m_currentMap, l, &dl)) {
    assert(!mapContains(m_deletedSet, l));
    assert(!mapContains(m_changeSet, l));
    return tx64->liveType(l, *curUnit());
  }
  return dl->rtt;
}

DynLocation* TraceletContext::recordRead(const InputInfo& ii,
                                         bool useHHIR,
                                         DataType staticType) {
  DynLocation* dl;
  const Location& l = ii.loc;
  if (!mapGet(m_currentMap, l, &dl)) {
    // We should never try to read a location that has been deleted
    assert(!mapContains(m_deletedSet, l));
    // If the given location was not in m_currentMap, then it shouldn't
    // be in m_changeSet either
    assert(!mapContains(m_changeSet, l));
    if (ii.dontGuard && !l.isLiteral()) {
      assert(!useHHIR || staticType != KindOfRef);
      dl = m_t->newDynLocation(l, RuntimeType(staticType));
      if (useHHIR && staticType != KindOfInvalid) {
        m_resolvedDeps[l] = dl;
      }
    } else {
      RuntimeType rtt = tx64->liveType(l, *curUnit());
      assert(rtt.isIter() || !rtt.isVagueValue());
      // Allocate a new DynLocation to represent this and store it in the
      // current map.
      dl = m_t->newDynLocation(l, rtt);

      if (!l.isLiteral()) {
        if (m_varEnvTaint && dl->isValue() && dl->isLocal()) {
          dl->rtt = RuntimeType(KindOfInvalid);
        } else if ((m_aliasTaint && dl->canBeAliased()) ||
                   (rtt.isValue() && rtt.isRef() && ii.dontGuardInner)) {
          dl->rtt = rtt.setValueType(KindOfInvalid);
        }
        // Record that we depend on the live type of the specified location
        // as well (and remember what the live type was)
        m_dependencies[l] = dl;
      }
    }
    m_currentMap[l] = dl;
  }
  TRACE(2, "recordRead: %s : %s\n", l.pretty().c_str(),
        dl->rtt.pretty().c_str());
  return dl;
}

void TraceletContext::recordWrite(DynLocation* dl,
                                  NormalizedInstruction* source) {
  TRACE(2, "recordWrite: %s : %s\n", dl->location.pretty().c_str(),
                                     dl->rtt.pretty().c_str());
  dl->source = source;
  m_currentMap[dl->location] = dl;
  m_changeSet.insert(dl->location);
  m_deletedSet.erase(dl->location);
}

void TraceletContext::recordDelete(const Location& l) {
  // We should not be trying to delete the rtt of location that is
  // not in m_currentMap
  TRACE(2, "recordDelete: %s\n", l.pretty().c_str());
  m_currentMap.erase(l);
  m_changeSet.erase(l);
  m_deletedSet.insert(l);
}

void TraceletContext::aliasTaint() {
  m_aliasTaint = true;
  for (ChangeMap::iterator it = m_currentMap.begin();
       it != m_currentMap.end(); ++it) {
    DynLocation* dl = it->second;
    if (dl->canBeAliased()) {
      TRACE(1, "(%s, %" PRId64 ") <- inner type invalidated\n",
            it->first.spaceName(), it->first.offset);
      RuntimeType newRtt = dl->rtt.setValueType(KindOfInvalid);
      it->second = m_t->newDynLocation(dl->location, newRtt);
    }
  }
}

void TraceletContext::varEnvTaint() {
  m_varEnvTaint = true;
  for (ChangeMap::iterator it = m_currentMap.begin();
       it != m_currentMap.end(); ++it) {
    DynLocation* dl = it->second;
    if (dl->isValue() && dl->isLocal()) {
      TRACE(1, "(%s, %" PRId64 ") <- type invalidated\n",
            it->first.spaceName(), it->first.offset);
      it->second = m_t->newDynLocation(dl->location,
                                       RuntimeType(KindOfInvalid));
    }
  }
}

void TraceletContext::recordJmp() {
  m_numJmps++;
}

/*
 *   Helpers for recovering context of this instruction.
 */
Op NormalizedInstruction::op() const {
  uchar op = *pc();
  assert(isValidOpcode(op));
  return (Op)op;
}

Op NormalizedInstruction::mInstrOp() const {
  Op opcode = op();
#define MII(instr, a, b, i, v, d) case Op##instr##M: return opcode;
  switch (opcode) {
    MINSTRS
  case OpFPassM:
    return preppedByRef ? OpVGetM : OpCGetM;
  default:
    not_reached();
  }
#undef MII
}

PC NormalizedInstruction::pc() const {
  return unit()->at(source.offset());
}

const Unit* NormalizedInstruction::unit() const {
  return m_unit;
}

Offset NormalizedInstruction::offset() const {
  return source.offset();
}

std::string NormalizedInstruction::toString() const {
  return instrToString(pc(), unit());
}

void Translator::postAnalyze(NormalizedInstruction* ni, SrcKey& sk,
                             Tracelet& t, TraceletContext& tas) {
  if (ni->op() == OpBareThis &&
      ni->outStack->rtt.isVagueValue()) {
    SrcKey src = sk;
    const Unit* unit = ni->m_unit;
    src.advance(unit);
    Opcode next = *unit->at(src.offset());
    if (next == OpInstanceOfD || next == OpIsNullC) {
      ni->outStack->rtt = RuntimeType(KindOfObject);
    }
    return;
  }
}

NormalizedInstruction::OutputUse
NormalizedInstruction::getOutputUsage(DynLocation* output,
                                      bool ignorePops /* =false */) const {
  for (NormalizedInstruction* succ = next;
       succ; succ = succ->next) {
    if (ignorePops &&
        (succ->op() == OpPopC ||
         succ->op() == OpPopV ||
         succ->op() == OpPopR)) {
      continue;
    }
    for (size_t i = 0; i < succ->inputs.size(); ++i) {
      if (succ->inputs[i] == output) {
        if (succ->inputWasInferred(i)) {
          return OutputInferred;
        }
        if (Translator::Get()->dontGuardAnyInputs(succ->op())) {
          /* the consumer doesnt care about its inputs
             but we may still have inferred something about
             its outputs that a later instruction may depend on
          */
          if (!outputDependsOnInput(succ->op()) ||
              !(succ->outStack && !succ->outStack->rtt.isVagueValue() &&
                succ->getOutputUsage(succ->outStack) != OutputUsed) ||
              !(succ->outLocal && !succ->outLocal->rtt.isVagueValue() &&
                succ->getOutputUsage(succ->outLocal)) != OutputUsed) {
            return OutputDoesntCare;
          }
        }
        return OutputUsed;
      }
    }
  }
  return OutputUnused;
}

GuardType::GuardType(DataType outer, DataType inner)
  : outerType(outer), innerType(inner) {
}

GuardType::GuardType(const RuntimeType& rtt) {
  assert(rtt.isValue());
  outerType = rtt.outerType();
  innerType = rtt.innerType();
}

GuardType::GuardType(const GuardType& other) {
  *this = other;
}


const DataType GuardType::getOuterType() const {
  return outerType;
}

const DataType GuardType::getInnerType() const {
  return innerType;
}

bool GuardType::isSpecific() const {
  return outerType > KindOfInvalid;
}

bool GuardType::isRelaxed() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfUncounted:
    case KindOfUncountedInit:
      return true;
    default:
      return false;
  }
}

bool GuardType::isGeneric() const {
  return outerType == KindOfAny;
}

bool GuardType::isCounted() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfRef:
      return true;
    default:
      return false;
  }
}

bool GuardType::isMoreRefinedThan(const GuardType& other) const {
  return getCategory() > other.getCategory();
}

DataTypeCategory GuardType::getCategory() const {
  switch (outerType) {
    case KindOfAny:           return DataTypeGeneric;
    case KindOfUncounted:     return DataTypeCountness;
    case KindOfUncountedInit: return DataTypeCountnessInit;
    default:                  return DataTypeSpecific;
  }
}

bool GuardType::mayBeUninit() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfUncounted:
    case KindOfUninit:
      return true;
    default:
      return false;
  }
}

GuardType GuardType::getCountness() const {
  // Note that translations need to be able to handle KindOfString and
  // KindOfStaticString interchangeably.  This implies that KindOfStaticString
  // needs to be treated as KindOfString, i.e. as possibly counted.
  assert(isSpecific());
  switch (outerType) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:  return GuardType(KindOfUncounted);
    default:            return *this;
  }
}

GuardType GuardType::getCountnessInit() const {
  assert(isSpecific());
  switch (outerType) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:  return GuardType(KindOfUncountedInit);
    default:            return *this;
  }
}


DataTypeCategory
Translator::getOperandConstraintCategory(NormalizedInstruction* instr,
                                         size_t opndIdx) {
  static const NormalizedInstruction::OutputUse kOutputUsed =
    NormalizedInstruction::OutputUsed;
  switch (instr->op()) {
    case OpSetL : {
      assert(opndIdx < 2);
      if (opndIdx == 0) { // stack value
        auto stackValUsage = instr->getOutputUsage(instr->outStack, true);
        if ((instr->outStack && (!m_useHHIR || stackValUsage == kOutputUsed)) ||
            (instr->getOutputUsage(instr->outLocal) == kOutputUsed)) {
          return DataTypeSpecific;
        }
        return DataTypeGeneric;
      } else { // old local value
        return DataTypeCountness;
      }
    }
    case OpCGetL : {
      if (!instr->outStack || (instr->getOutputUsage(instr->outStack, true) ==
                               NormalizedInstruction::OutputUsed)) {
        return DataTypeSpecific;
      }
      return DataTypeCountnessInit;
    }
    case OpRetC :
    case OpRetV : {
      return DataTypeCountness;
    }
    default: return DataTypeSpecific;
  }
}


GuardType Translator::getOperandConstraintType(NormalizedInstruction* instr,
                                               size_t                 opndIdx,
                                               const GuardType&       specType) {
  DataTypeCategory dtCategory = getOperandConstraintCategory(instr, opndIdx);
  switch (dtCategory) {
    case DataTypeGeneric:       return GuardType(KindOfAny);
    case DataTypeCountness:     return specType.getCountness();
    case DataTypeCountnessInit: return specType.getCountnessInit();
    case DataTypeSpecific:
    default:                    return specType;
  }
}

void Translator::constrainOperandType(GuardType&             relxType,
                                      NormalizedInstruction* instr,
                                      size_t                 opndIdx,
                                      const GuardType&       specType) {
  if (relxType.isSpecific()) return; // Can't constrain any further

  switch (instr->op()) {
    case OpRetC  :
    case OpRetV  :
    case OpCGetL :
    case OpSetL  :
    {
      GuardType consType = getOperandConstraintType(instr, opndIdx, specType);
      if (consType.isMoreRefinedThan(relxType)) {
        relxType = consType;
      }
      return;
    }
    default: {
      relxType = specType;
      return;
    }
  }
}

void Translator::reanalizeConsumers(Tracelet& tclet, DynLocation* depDynLoc) {
  for (auto& instr : tclet.m_instrs) {
    for (size_t i = 0; i < instr.inputs.size(); i++) {
      if (instr.inputs[i] == depDynLoc) {
        analyzeInstr(tclet, instr);
      }
    }
  }
}


/**
 * This method looks at all the uses of the tracelet dependencies in the
 * instruction stream and tries to relax the type associated with each location.
 */
void Translator::relaxDeps(Tracelet& tclet, TraceletContext& tctxt) {
  DynLocTypeMap locRelxTypeMap;
  DynLocTypeMap locSpecTypeMap;

  // Initialize type maps.  Relaxed types start off very relaxed, and then
  // they may get more specific depending on how the instructions use them.
  DepMap& deps = tctxt.m_dependencies;
  for (auto depIt = deps.begin(); depIt != deps.end(); depIt++) {
    DynLocation*       loc = depIt->second;
    const RuntimeType& rtt = depIt->second->rtt;
    if (rtt.isValue() && !rtt.isVagueValue() && !loc->location.isThis()) {
      locSpecTypeMap[loc] = GuardType(rtt);
      locRelxTypeMap[loc] = GuardType(KindOfAny);
    }
  }

  // Process the instruction stream, constraining the relaxed types along the way
  for (NormalizedInstruction* instr = tclet.m_instrStream.first; instr;
       instr = instr->next) {
    for (size_t i = 0; i < instr->inputs.size(); i++) {
      DynLocation* loc = instr->inputs[i];
      auto it = locRelxTypeMap.find(loc);
      if (it != locRelxTypeMap.end()) {
        GuardType& relxType = it->second;
        constrainOperandType(relxType, instr, i, locSpecTypeMap[loc]);
      }
    }
  }

  // For each dependency, if we found a more relaxed type for it, use such type.
  for (auto mapIt = locRelxTypeMap.begin(); mapIt != locRelxTypeMap.end();
       mapIt++) {
    DynLocation* loc = mapIt->first;
    const GuardType& relxType = mapIt->second;
    if (relxType.isRelaxed()) {
      TRACE(1, "relaxDeps: Loc: %s   oldType: %s   =>   newType: %s\n",
            loc->location.pretty().c_str(),
            deps[loc->location]->rtt.pretty().c_str(),
            RuntimeType(relxType.getOuterType(),
                        relxType.getInnerType()).pretty().c_str());
      assert(deps[loc->location] == loc);
      assert(relxType.getOuterType() != KindOfInvalid);
      deps[loc->location]->rtt = RuntimeType(relxType.getOuterType(),
                                             relxType.getInnerType());
      reanalizeConsumers(tclet, loc);
    }
  }
}

static bool checkTaintFuncs(StringData* name) {
  static const StringData* s_extract =
    StringData::GetStaticString("extract");
  return name->isame(s_extract);
}

static const NormalizedInstruction*
findFPushForCall(const FPIEnt* fpi,
                 const NormalizedInstruction* fcall) {
  for (auto* ni = fcall->prev; ni; ni = ni->prev) {
    if (ni->source.offset() == fpi->m_fpushOff) {
      return ni;
    }
  }
  return nullptr;
}

/*
 * Check whether the a given FCall should be analyzed for possible
 * inlining or not.
 */
static bool shouldAnalyzeCallee(const NormalizedInstruction* fcall) {
  auto const numArgs = fcall->imm[0].u_IVA;
  auto const target  = fcall->funcd;
  auto const fpi     = curFunc()->findFPI(fcall->source.m_offset);
  auto const pushOp  = curUnit()->getOpcode(fpi->m_fpushOff);

  if (!RuntimeOption::RepoAuthoritative) return false;

  // Note: the IR assumes that $this is available in all inlined object
  // methods, which will need to be updated when we support
  // OpFPushClsMethod here.
  if (pushOp != OpFPushFuncD && pushOp != OpFPushObjMethodD) {
    FTRACE(1, "analyzeCallee: push op ({}) was not supported\n",
           opcodeToName(pushOp));
    return false;
  }

  if (!target) {
    FTRACE(1, "analyzeCallee: target func not known\n");
    return false;
  }
  if (target->isBuiltin()) {
    FTRACE(1, "analyzeCallee: target func is a builtin\n");
    return false;
  }

  constexpr int kMaxSubtraceAnalysisDepth = 2;
  if (tx64->analysisDepth() + 1 >= kMaxSubtraceAnalysisDepth) {
    FTRACE(1, "analyzeCallee: max inlining depth reached\n");
    return false;
  }

  if (numArgs != target->numParams()) {
    FTRACE(1, "analyzeCallee: param count mismatch {} != {}\n",
           numArgs, target->numParams());
    return false;
  }
  if (numArgs != 0) {
    FTRACE(1, "analyzeCallee: currently ignoring calls with parameters\n");
    return false;
  }

  if (!findFPushForCall(fpi, fcall)) {
    FTRACE(1, "analyzeCallee: push instruction was in a different "
              "tracelet\n");
    return false;
  }

  return true;
}

void Translator::analyzeCallee(TraceletContext& tas,
                               Tracelet& parent,
                               NormalizedInstruction* fcall) {
  always_assert(m_useHHIR);
  if (!shouldAnalyzeCallee(fcall)) return;

  auto const numArgs = fcall->imm[0].u_IVA;
  auto const target  = fcall->funcd;

  /*
   * Prepare a map for all the known information about the argument
   * types.
   *
   * Also, fill out KindOfUninit for any remaining locals.  The point
   * here is that the subtrace can't call liveType for a local or
   * stack location (since our ActRec is fake), so we need them all in
   * the TraceletContext.
   *
   * If any of the argument types are unknown (including inner-types
   * of KindOfRefs), we don't really try to analyze the callee.  It
   * might be possible to do this but we'll need to modify the
   * analyzer to support unknown input types before there are any
   * NormalizedInstructions in the Tracelet.
   */
  TypeMap initialMap;
  LocationSet callerArgLocs;
  for (int i = 0; i < numArgs; ++i) {
    auto callerLoc = Location(Location::Stack, fcall->stackOff - i - 1);
    auto calleeLoc = Location(Location::Local, numArgs - i - 1);
    auto type      = tas.currentType(callerLoc);

    callerArgLocs.insert(callerLoc);

    if (type.isVagueValue()) {
      FTRACE(1, "analyzeCallee: {} has unknown type\n", callerLoc.pretty());
      return;
    }
    if (type.isValue() && type.isRef() &&
        type.innerType() == KindOfInvalid) {
      FTRACE(1, "analyzeCallee: {} has unknown inner-refdata type\n",
             callerLoc.pretty());
      return;
    }

    FTRACE(2, "mapping arg{} locs {} -> {} :: {}\n",
              numArgs - i - 1,
              callerLoc.pretty(),
              calleeLoc.pretty(),
              type.pretty());
    initialMap[calleeLoc] = type;
  }
  for (int i = numArgs; i < target->numLocals(); ++i) {
    initialMap[Location(Location::Local, i)] = RuntimeType(KindOfUninit);
  }

  /*
   * When reentering analyze to generate a Tracelet for a callee,
   * currently we handle this by creating a fake ActRec on the stack.
   *
   * This is mostly a compromise to deal with existing code during the
   * analysis phase which pretty liberally inspects live VM state.
   */
  ActRec fakeAR;
  fakeAR.m_savedRbp = reinterpret_cast<uintptr_t>(curFrame());
  fakeAR.m_savedRip = 0xbaabaa;  // should never be inspected
  fakeAR.m_func = fcall->funcd;
  fakeAR.m_soff = 0xb00b00;      // should never be inspected
  fakeAR.m_numArgsAndCtorFlag = numArgs;
  fakeAR.m_varEnv = nullptr;

  /*
   * Even when inlining an object method, we can leave the m_this as
   * null.  See outThisObjectType().
   */
  fakeAR.m_this = nullptr;

  FTRACE(1, "analyzing sub trace =================================\n");
  auto const oldFP = vmfp();
  auto const oldSP = vmsp();
  auto const oldPC = vmpc();
  auto const oldAnalyzeCalleeDepth = m_analysisDepth++;
  vmpc() = nullptr; // should never be used
  vmsp() = nullptr; // should never be used
  vmfp() = reinterpret_cast<Cell*>(&fakeAR);
  auto restoreFrame = [&]{
    vmfp() = oldFP;
    vmsp() = oldSP;
    vmpc() = oldPC;
    m_analysisDepth = oldAnalyzeCalleeDepth;
  };
  SCOPE_EXIT {
    // It's ok to restoreFrame() twice---we have it in this scope
    // handler to ensure it still happens if we exit via an exception.
    restoreFrame();
    FTRACE(1, "finished sub trace ===================================\n");
  };

  auto subTrace = analyze(SrcKey(target, target->base()), initialMap);

  /*
   * Verify the target trace actually ended with a return, or we have
   * no business doing anything based on it right now.
   */
  if (!subTrace->m_instrStream.last ||
      (subTrace->m_instrStream.last->op() != OpRetC &&
       subTrace->m_instrStream.last->op() != OpRetV)) {
    FTRACE(1, "analyzeCallee: callee did not end in a return\n");
    return;
  }

  /*
   * Disabled for now:
   *
   * Propagate the return type to our caller.  If the return type is
   * not vague, it will hold if we can inline the trace.
   *
   * This isn't really a sensible thing to do if we aren't also going
   * to inline the callee, however, because the return type may only
   * be what it is due to other output predictions (CGetMs or FCall)
   * inside the callee.  This means we would need to check the return
   * value in the caller still as if it were a predicted return type.
   */
  Location retVal(Location::Stack, 0);
  auto it = subTrace->m_changes.find(retVal);
  assert(it != subTrace->m_changes.end());
  FTRACE(1, "subtrace return: {}\n", it->second->pretty());
  if (false) {
    if (!it->second->rtt.isVagueValue() && !it->second->rtt.isRef()) {
      FTRACE(1, "changing callee's return type from {} to {}\n",
                fcall->outStack->rtt.pretty(),
                it->second->pretty());

      fcall->outputPredicted = true;
      fcall->outputPredictionStatic = false;
      fcall->outStack = parent.newDynLocation(fcall->outStack->location,
                                              it->second->rtt);
      tas.recordWrite(fcall->outStack, fcall);
    }
  }

  /*
   * In order for relaxDeps not to relax guards on things we may
   * potentially have depended on here, we need to ensure that the
   * call instruction depends on all the inputs we've used.
   *
   * What we probably want to do is modify getOutputUsage to be aware
   * of callee-uses of parameters.
   *
   * For now this assert is just protecting the known breakage if we
   * start doing analyzeCallee on things with parameters.
   */
  restoreFrame();
  assert(callerArgLocs.empty());

  FTRACE(1, "analyzeCallee: inline candidate\n");
  fcall->calleeTrace = std::move(subTrace);
}

/*
 * analyze --
 *
 *   Given a sequence of bytecodes, return our tracelet IR.
 *
 * The purposes of this analysis is to determine:
 *
 *  1. Pre-conditions: What locations get read before they get written to:
 *     we will need typechecks for these and we will want to load them into
 *     registers. (m_dependencies)
 *
 *  2. Post-conditions: the locations that have been written to and are
 *     still live at the end of the tracelet. We need to allocate registers
 *     of these and we need to spill them at the end of the tracelet.
 *     (m_changes)
 *
 *  3. Determine the runtime types for each instruction's input locations
 *     and output locations.
 *
 * The main analysis works by doing a single pass over the instructions. It
 * effectively simulates the execution of each instruction, updating its
 * knowledge about types as it goes.
 *
 * The TraceletContext class is used to keep track of the current state of
 * the world. Initially it is empty, and when the inputs for the first
 * instruction are analyzed we call recordRead(). The recordRead() function
 * in turn inspects the live types of the inputs and adds them to the type
 * map. This serves two purposes: (1) it figures out what typechecks this
 * tracelet needs; and (2) it guarantees that the code we generate will
 * satisfy the live types that are about to be passed in.
 *
 * Over time the TraceletContext's type map will change. However, we need to
 * record what the types _were_ right before and right after a given
 * instruction executes. This is where the NormalizedInstruction class comes
 * in. We store the RuntimeTypes from the TraceletContext right before an
 * instruction executes into the NormalizedInstruction's 'inputs' field, and
 * we store the RuntimeTypes from the TraceletContext right after the
 * instruction executes into the various output fields.
 */
std::unique_ptr<Tracelet> Translator::analyze(SrcKey sk,
                                              const TypeMap& initialTypes) {
  std::unique_ptr<Tracelet> retval(new Tracelet());
  auto& t = *retval;
  t.m_sk = sk;

  DEBUG_ONLY const char* file = curUnit()->filepath()->data();
  DEBUG_ONLY const int lineNum = curUnit()->getLineNumber(t.m_sk.offset());
  DEBUG_ONLY const char* funcName = curFunc()->fullName()->data();

  TRACE(1, "Translator::analyze %s:%d %s\n", file, lineNum, funcName);
  TraceletContext tas(&t, initialTypes);
  ImmStack immStack;
  int stackFrameOffset = 0;
  int oldStackFrameOffset = 0;

  // numOpcodes counts the original number of opcodes in a tracelet
  // before the translator does any optimization
  t.m_numOpcodes = 0;
  Unit::MetaHandle metaHand;

  const Unit *unit = curUnit();
  for (;; sk.advance(unit)) {
  head:
    NormalizedInstruction* ni = t.newNormalizedInstruction();
    ni->source = sk;
    ni->stackOff = stackFrameOffset;
    ni->funcd = (t.m_arState.getCurrentState() == ActRecState::KNOWN) ?
      t.m_arState.getCurrentFunc() : nullptr;
    ni->m_unit = unit;
    ni->preppedByRef = false;
    ni->breaksTracelet = false;
    ni->changesPC = opcodeChangesPC(ni->op());
    ni->manuallyAllocInputs = false;
    ni->fuseBranch = false;
    ni->outputPredicted = false;
    ni->outputPredictionStatic = false;

    assert(!t.m_analysisFailed);
    oldStackFrameOffset = stackFrameOffset;
    for (int i = 0; i < numImmediates(ni->op()); i++) {
      ni->imm[i] = getImm(ni->pc(), i);
    }
    if (hasImmVector(*ni->pc())) {
      ni->immVec = getImmVector(ni->pc());
    }
    if (ni->op() == OpFCallArray) {
      ni->imm[0].u_IVA = 1;
    }

    // Use the basic block analyzer to follow the flow of immediate values.
    findImmable(immStack, ni);

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // Translation could fail entirely (because of an unknown opcode), or
    // encounter an input that cannot be computed.
    try {
      preInputApplyMetaData(metaHand, ni);
      InputInfos inputInfos;
      getInputs(t, ni, stackFrameOffset, inputInfos, tas);
      bool noOp = applyInputMetaData(metaHand, ni, tas, inputInfos);
      if (noOp) {
        if (m_useHHIR) {
          t.m_instrStream.append(ni);
          ++t.m_numOpcodes;
        }
        stackFrameOffset = oldStackFrameOffset;
        continue;
      }
      if (inputInfos.needsRefCheck) {
        // Drive the arState machine; if it is going to throw an input
        // exception, do so here.
        int argNum = ni->imm[0].u_IVA;
        // instrSpToArDelta() returns the delta relative to the sp at the
        // beginning of the instruction, but getReffiness() wants the delta
        // relative to the sp at the beginning of the tracelet, so we adjust
        // by subtracting ni->stackOff
        int entryArDelta = instrSpToArDelta(ni->pc()) - ni->stackOff;
        ni->preppedByRef = t.m_arState.getReffiness(argNum,
                                                    entryArDelta,
                                                    &t.m_refDeps);
        SKTRACE(1, sk, "passing arg%d by %s\n", argNum,
                ni->preppedByRef ? "reference" : "value");
      }

      for (unsigned int i = 0; i < inputInfos.size(); i++) {
        SKTRACE(2, sk, "typing input %d\n", i);
        const InputInfo& ii = inputInfos[i];
        DynLocation* dl = tas.recordRead(ii, m_useHHIR);
        const RuntimeType& rtt = dl->rtt;
        // Some instructions are able to handle an input with an unknown type
        if (!ii.dontBreak && !ii.dontGuard) {
          if (rtt.isVagueValue()) {
            // Consumed a "poisoned" output: e.g., result of an array
            // deref.
            throwUnknownInput();
          }
          if (!ni->ignoreInnerType && !ii.dontGuardInner) {
            if (rtt.isValue() && rtt.isRef() &&
                rtt.innerType() == KindOfInvalid) {
              throwUnknownInput();
            }
          }
        }
        ni->inputs.push_back(dl);
      }
    } catch (TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator fail: %s:%d\n", tfe.m_file, tfe.m_line);
      if (!t.m_numOpcodes) {
        t.m_analysisFailed = true;
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
      }
      goto breakBB;
    } catch (UnknownInputExc& uie) {
      // Subtle: if this instruction consumes an unknown runtime type,
      // break the BB on the *previous* instruction. We know that a
      // previous instruction exists, because the KindOfInvalid must
      // have come from somewhere.
      always_assert(t.m_instrStream.last);
      SKTRACE(2, sk, "Consumed unknown input (%s:%d); breaking BB at "
        "predecessor\n", uie.m_file, uie.m_line);
      goto breakBB;
    }

    SKTRACE(2, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    bool doVarEnvTaint; // initialized by reference.

    try {
      getOutputs(t, ni, stackFrameOffset, doVarEnvTaint);
    } catch (TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator getOutputs fail: %s:%d\n",
              tfe.m_file, tfe.m_line);
      if (!t.m_numOpcodes) {
        t.m_analysisFailed = true;
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
      }
      goto breakBB;
    }

    if (isFCallStar(ni->op())) {
      if (!doVarEnvTaint) {
        const FPIEnt *fpi = curFunc()->findFPI(ni->source.m_offset);
        assert(fpi);
        Offset fpushOff = fpi->m_fpushOff;
        PC fpushPc = curUnit()->at(fpushOff);
        if (*fpushPc == OpFPushFunc) {
          doVarEnvTaint = true;
        } else if (*fpushPc == OpFPushFuncD) {
          StringData *funcName =
            curUnit()->lookupLitstrId(getImm(fpushPc, 1).u_SA);
          doVarEnvTaint = checkTaintFuncs(funcName);
        }
      }
      t.m_arState.pop();
    }
    if (ni->op() == OpFCallBuiltin && !doVarEnvTaint) {
      StringData* funcName = curUnit()->lookupLitstrId(ni->imm[2].u_SA);
      doVarEnvTaint = checkTaintFuncs(funcName);
    }
    if (doVarEnvTaint) {
      tas.varEnvTaint();
    }

    DynLocation* outputs[] = { ni->outStack, ni->outLocal, ni->outLocal2,
                               ni->outStack2, ni->outStack3 };
    for (size_t i = 0; i < sizeof(outputs) / sizeof(*outputs); ++i) {
      if (outputs[i]) {
        DynLocation* o = outputs[i];
        SKTRACE(2, sk, "inserting output t(%d->%d) #(%s, %d)\n",
                o->rtt.outerType(), o->rtt.innerType(),
                o->location.spaceName(), o->location.offset);
        tas.recordWrite(o, ni);
      }
    }

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // This assert failing means that your instruction has an
    // inconsistent row in the InstrInfo table; the stackDelta doesn't
    // agree with the inputs and outputs.
    assert(getStackDelta(*ni) == (stackFrameOffset - oldStackFrameOffset));
    // If this instruction decreased the depth of the stack, mark the
    // appropriate stack locations as "dead".  But we need to leave
    // them in the TraceletContext until after analyzeCallee (if this
    // is an FCall).
    if (stackFrameOffset < oldStackFrameOffset) {
      for (int i = stackFrameOffset; i < oldStackFrameOffset; ++i) {
        ni->deadLocs.push_back(Location(Location::Stack, i));
      }
    }

    t.m_stackChange += getStackDelta(*ni);

    t.m_instrStream.append(ni);
    ++t.m_numOpcodes;

    /*
     * The annotation step attempts to track Func*'s associated with
     * given FCalls when the FPush is in a different tracelet.
     *
     * When we're analyzing a callee, we can't do this because we may
     * have class information in some of our RuntimeTypes that is only
     * true because of who the caller was.  (Normally it is only there
     * if it came from static analysis.)
     */
    if (analysisDepth() == 0) {
      annotate(ni);
    }

    analyzeInstr(t, *ni);
    if (m_useHHIR && ni->op() == OpFCall) {
      analyzeCallee(tas, t, ni);
    }

    for (auto& l : ni->deadLocs) {
      tas.recordDelete(l);
    }

    if (debug) {
      // The interpreter has lots of nice sanity assertions in debug mode
      // that the translator doesn't exercise. As a cross-check on the
      // translator's correctness, this is a debug-only facility for
      // sending a random selection of instructions through the
      // interpreter.
      //
      // Set stress_txInterpPct to a value between 1 and 100. If you want
      // to reproduce a failing case, look for the seed in the log and set
      // stress_txInterpSeed accordingly.
      if (!dbgTranslateCoin) {
        dbgTranslateCoin = new BiasedCoin(Trace::stress_txInterpPct,
                                          Trace::stress_txInterpSeed);
        TRACE(1, "BiasedCoin(stress_txInterpPct,stress_txInterpSeed): "
              "pct %f, seed %d\n",
              dbgTranslateCoin->getPercent(), dbgTranslateCoin->getSeed());
      }
      assert(dbgTranslateCoin);
      if (dbgTranslateCoin->flip()) {
        SKTRACE(3, ni->source, "stress interp\n");
        ni->m_txFlags = Interp;
      }

      if ((ni->op()) > Trace::moduleLevel(Trace::tmp0) &&
          (ni->op()) < Trace::moduleLevel(Trace::tmp1)) {
        ni->m_txFlags = Interp;
      }
    }

    Op txOpBisectLowOp  = (Op)moduleLevel(Trace::txOpBisectLow),
       txOpBisectHighOp = (Op)moduleLevel(Trace::txOpBisectHigh);
    if (txOpBisectLowOp  > OpLowInvalid &&
        txOpBisectHighOp > OpLowInvalid &&
        txOpBisectHighOp < OpHighInvalid) {
      // If the user specified an operation bisection interval [Low, High]
      // that is strictly included in (OpLowInvalid, OpHighInvalid), then
      // only support the operations in that interval. Since the default
      // value of moduleLevel is 0 and OpLowInvalid is also 0, this ensures
      // that bisection is disabled by default.
      static_assert(OpLowInvalid >= 0,
                    "OpLowInvalid must be nonnegative");
      if (ni->op() < txOpBisectLowOp ||
          ni->op() > txOpBisectHighOp)
        ni->m_txFlags = Interp;
    }

    // Check if we need to break the tracelet.
    //
    // If we've gotten this far, it mostly boils down to control-flow
    // instructions. However, we'll trace through a few unconditional jmps.
    if (ni->op() == OpJmp &&
        ni->imm[0].u_IA > 0 &&
        tas.m_numJmps < MaxJmpsTracedThrough) {
      // Continue tracing through jumps. To prevent pathologies, only trace
      // through a finite number of forward jumps.
      SKTRACE(1, sk, "greedily continuing through %dth jmp + %d\n",
              tas.m_numJmps, ni->imm[0].u_IA);
      tas.recordJmp();
      sk = SrcKey(curFunc(), sk.m_offset + ni->imm[0].u_IA);
      goto head; // don't advance sk
    } else if (opcodeBreaksBB(ni->op()) ||
        (ni->m_txFlags == Interp && opcodeChangesPC(ni->op()))) {
      SKTRACE(1, sk, "BB broken\n");
      sk.advance(unit);
      goto breakBB;
    }
    postAnalyze(ni, sk, t, tas);
  }
breakBB:
  NormalizedInstruction* ni = t.m_instrStream.last;
  while (ni) {
    switch (ni->op()) {
      // We dont want to end a tracelet with a literal;
      // it will cause the literal to be pushed on the
      // stack, and the next tracelet will have to guard
      // on the type.
      case OpNull:
      case OpNullUninit:
      case OpTrue:
      case OpFalse:
      case OpInt:
      case OpDouble:
      case OpString:
      case OpArray:
      // Similarly, This, Self and Parent will lose
      // type information thats only useful in the
      // following tracelet.
      case OpThis:
      case OpSelf:
      case OpParent:
        ni = ni->prev;
        continue;
      default:
        break;
    }
    break;
  }
  if (ni) {
    while (ni != t.m_instrStream.last) {
      t.m_stackChange -= getStackDelta(*t.m_instrStream.last);
      sk = t.m_instrStream.last->source;
      t.m_instrStream.remove(t.m_instrStream.last);
      --t.m_numOpcodes;
    }
  }

  // Peephole optimizations may leave the bytecode stream in a state that is
  // inconsistent and troubles HHIR emission, so don't do it if HHIR is in use
  if (!m_useHHIR) {
    analyzeSecondPass(t);
  }

  relaxDeps(t, tas);

  // Mark the last instruction appropriately
  assert(t.m_instrStream.last);
  t.m_instrStream.last->breaksTracelet = true;
  t.m_nextSk = sk;
  // Populate t.m_changes, t.intermediates, t.m_dependencies
  t.m_dependencies = tas.m_dependencies;
  t.m_resolvedDeps = tas.m_resolvedDeps;
  t.m_changes.clear();
  LocationSet::iterator it = tas.m_changeSet.begin();
  for (; it != tas.m_changeSet.end(); ++it) {
    t.m_changes[*it] = tas.m_currentMap[*it];
  }

  t.constructLiveRanges();

  TRACE(1, "Tracelet done: stack delta %d\n", t.m_stackChange);
  return retval;
}

Translator::Translator()
  : m_resumeHelper(nullptr)
  , m_useHHIR(false)
  , m_createdTime(Timer::GetCurrentTimeMicros())
  , m_analysisDepth(0)
{
  initInstrInfo();
}

Translator::~Translator() {
}

Translator*
Translator::Get() {
  return TranslatorX64::Get();
}

bool
Translator::isSrcKeyInBL(const Unit* unit, const SrcKey& sk) {
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLSrcKey.find(sk) != m_dbgBLSrcKey.end()) {
    return true;
  }
  for (PC pc = unit->at(sk.m_offset); !opcodeBreaksBB(*pc);
       pc += instrLen(pc)) {
    if (m_dbgBLPC.checkPC(pc)) {
      m_dbgBLSrcKey.insert(sk);
      return true;
    }
  }
  return false;
}

void
Translator::clearDbgBL() {
  Lock l(m_dbgBlacklistLock);
  m_dbgBLSrcKey.clear();
  m_dbgBLPC.clear();
}

bool
Translator::addDbgBLPC(PC pc) {
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLPC.checkPC(pc)) {
    // already there
    return false;
  }
  m_dbgBLPC.addPC(pc);
  return true;
}

uint64_t* Translator::getTransCounterAddr() {
  if (!isTransDBEnabled()) return nullptr;

  TransID id = m_translations.size();

  // allocate a new chunk of counters if necessary
  if (id >= m_transCounters.size() * transCountersPerChunk) {
    uint32_t   size = sizeof(uint64_t) * transCountersPerChunk;
    auto *chunk = (uint64_t*)malloc(size);
    bzero(chunk, size);
    m_transCounters.push_back(chunk);
  }
  assert(id / transCountersPerChunk < m_transCounters.size());
  return &(m_transCounters[id / transCountersPerChunk]
           [id % transCountersPerChunk]);
}


uint64_t Translator::getTransCounter(TransID transId) const {
  if (!isTransDBEnabled()) return -1ul;
  assert(transId < m_translations.size());

  uint64_t counter;

  if (transId / transCountersPerChunk >= m_transCounters.size()) {
    counter = 0;
  } else {
    counter =  m_transCounters[transId / transCountersPerChunk]
                              [transId % transCountersPerChunk];
  }
  return counter;
}

void Translator::setTransCounter(TransID transId, uint64_t value) {
  assert(transId < m_translations.size());
  assert(transId / transCountersPerChunk < m_transCounters.size());

  m_transCounters[transId / transCountersPerChunk]
                 [transId % transCountersPerChunk] = value;
}

static const char *transKindStr[] = {
  "Normal_Tx64",
  "Normal_HHIR",
  "Anchor",
  "Prologue",
};

const char *getTransKindName(TransKind kind) {
  assert(kind >= 0 && kind <= TransProlog);
  return transKindStr[kind];
}

string
TransRec::print(uint64_t profCount) const {
  const size_t kBufSize = 1000;
  static char formatBuf[kBufSize];

  snprintf(formatBuf, kBufSize,
           "Translation %d {\n"
           "  src.md5 = %s\n"
           "  src.funcId = %u\n"
           "  src.startOffset = 0x%x\n"
           "  src.stopOffset = 0x%x\n"
           "  kind = %u (%s)\n"
           "  aStart = %p\n"
           "  aLen = 0x%x\n"
           "  stubStart = %p\n"
           "  stubLen = 0x%x\n"
           "  profCount = %" PRIu64 "\n"
           "  bcMapping = %lu\n",
           id, md5.toString().c_str(), src.getFuncId(), src.offset(),
           bcStopOffset, kind, getTransKindName(kind), aStart, aLen,
           astubsStart, astubsLen, profCount, bcMapping.size());

  string ret(formatBuf);

  for (size_t i = 0; i < bcMapping.size(); i++) {
    snprintf(formatBuf, kBufSize, "    0x%x %p %p\n",
             bcMapping[i].bcStart,
             bcMapping[i].aStart,
             bcMapping[i].astubsStart);

    ret += string(formatBuf);
  }

  ret += "}\n\n";
  return ret;
}

void
ActRecState::pushFuncD(const Func* func) {
  TRACE(2, "ActRecState: pushStatic func %p(%s)\n", func, func->name()->data());
  Record r;
  r.m_state = KNOWN;
  r.m_topFunc = func;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pushDynFunc() {
  TRACE(2, "ActRecState: pushDynFunc\n");
  Record r;
  r.m_state = UNKNOWABLE;
  r.m_topFunc = nullptr;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pop() {
  if (!m_arStack.empty()) {
    m_arStack.pop_back();
  }
}

/**
 * getReffiness() returns true if the parameter specified by argNum is pass
 * by reference, otherwise it returns false. This function may also throw an
 * UnknownInputException if the reffiness cannot be determined.
 *
 * Note that the 'entryArDelta' parameter specifies the delta between sp at
 * the beginning of the tracelet and ar.
 */
bool
ActRecState::getReffiness(int argNum, int entryArDelta, RefDeps* outRefDeps) {
  assert(outRefDeps);
  TRACE(2, "ActRecState: getting reffiness for arg %d\n", argNum);
  if (m_arStack.empty()) {
    // The ActRec in question was pushed before the beginning of the
    // tracelet, so we can make a guess about parameter reffiness and
    // record our assumptions about parameter reffiness as tracelet
    // guards.
    const ActRec* ar = arFromSpOffset((ActRec*)vmsp(), entryArDelta);
    Record r;
    r.m_state = GUESSABLE;
    r.m_entryArDelta = entryArDelta;
    r.m_topFunc = ar->m_func;
    m_arStack.push_back(r);
  }
  Record& r = m_arStack.back();
  if (r.m_state == UNKNOWABLE) {
    TRACE(2, "ActRecState: unknowable, throwing in the towel\n");
    throwUnknownInput();
    not_reached();
  }
  assert(r.m_topFunc);
  bool retval = r.m_topFunc->byRef(argNum);
  if (r.m_state == GUESSABLE) {
    assert(r.m_entryArDelta != InvalidEntryArDelta);
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    outRefDeps->addDep(r.m_entryArDelta, argNum, retval);
  }
  return retval;
}

const Func*
ActRecState::getCurrentFunc() {
  if (m_arStack.empty()) return nullptr;
  return m_arStack.back().m_topFunc;
}

ActRecState::State
ActRecState::getCurrentState() {
  if (m_arStack.empty()) return GUESSABLE;
  return m_arStack.back().m_state;
}

const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  bool privateOnly = false;
  if (!RuntimeOption::RepoAuthoritative ||
      !(cls->preClass()->attrs() & AttrUnique)) {
    Class* ctx = curFunc()->cls();
    if (!ctx || !ctx->classof(cls)) {
      return nullptr;
    }
    if (!staticLookup) privateOnly = true;
  }

  const Func* func;
  MethodLookup::LookupResult res = staticLookup ?
    g_vmContext->lookupClsMethod(func, cls, name, 0, false) :
    g_vmContext->lookupObjMethod(func, cls, name, false);

  if (res == MethodLookup::MethodNotFound) return nullptr;

  assert(res == MethodLookup::MethodFoundWithThis ||
         res == MethodLookup::MethodFoundNoThis ||
         (staticLookup ?
          res == MethodLookup::MagicCallStaticFound :
          res == MethodLookup::MagicCallFound));

  magicCall =
    res == MethodLookup::MagicCallStaticFound ||
    res == MethodLookup::MagicCallFound;

  if ((privateOnly && (!(func->attrs() & AttrPrivate) || magicCall)) ||
      func->isAbstract() ||
      func->attrs() & AttrDynamicInvoke) {
    return nullptr;
  }

  if (staticLookup) {
    if (magicCall) {
      /*
       *  i) We cant tell if a magic call would go to __call or __callStatic
       *       - Could deal with this by checking for the existence of __call
       *
       * ii) hphp semantics is that in the case of an object call, we look
       *     for __call in the scope of the object (this is incompatible
       *     with zend) which means we would have to know that there is no
       *     __call higher up in the tree
       *       - Could deal with this by checking for AttrNoOverride on the
       *         class
       */
      func = nullptr;
    }
  } else if (!(func->attrs() & AttrPrivate)) {
    if (magicCall || func->attrs() & AttrStatic) {
      if (!(cls->preClass()->attrs() & AttrNoOverride)) {
        func = nullptr;
      }
    } else if (!(func->attrs() & AttrNoOverride && !func->hasStaticLocals()) &&
               !(cls->preClass()->attrs() & AttrNoOverride)) {
      func = nullptr;
    }
  }
  return func;
}

std::string traceletShape(const Tracelet& trace) {
  std::string ret;

  for (auto ni = trace.m_instrStream.first; ni; ni = ni->next) {
    using folly::toAppend;

    toAppend(opcodeToName(ni->op()), &ret);
    if (ni->immVec.isValid()) {
      toAppend(
        "<",
        locationCodeString(ni->immVec.locationCode()),
        &ret);
      for (auto& mc : ni->immVecM) {
        toAppend(" ", memberCodeString(mc), &ret);
      }
      toAppend(">", &ret);
    }
    toAppend(" ", &ret);
  }

  return ret;
}

} } }
