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

// Translator front-end: parse instruction stream into basic blocks, decode
// and normalize instructions. Propagate run-time type info to instructions
// to annotate their inputs and outputs with types.
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <string>

#include <runtime/base/runtime_option.h>
#include <runtime/base/types.h>
#include <runtime/base/tv_macros.h>
#include <runtime/ext/ext_continuation.h>
#include <util/trace.h>
#include <util/biased_coin.h>
#include <runtime/vm/hhbc.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/annotation.h>
#include <runtime/vm/type-profile.h>
#include <runtime/vm/runtime.h>

namespace HPHP {
namespace VM {
namespace Transl {

using namespace HPHP::VM;

TRACE_SET_MOD(trans)

static __thread BiasedCoin *dbgTranslateCoin;
Translator* transl;
Lease Translator::s_writeLease;

void InstrStream::append(NormalizedInstruction* ni) {
  if (last) {
    ASSERT(first);
    last->next = ni;
    ni->prev = last;
    ni->next = NULL;
    last = ni;
    return;
  }
  ASSERT(!first);
  first = ni;
  last = ni;
  ni->prev = NULL;
  ni->next = NULL;
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
  ni->prev = NULL;
  ni->next = NULL;
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
  ASSERT(end != m_liveEnd.end());
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
  const NormalizedInstruction* i = m_instrStream.first;
  if (i == NULL) {
    std::cerr << "<empty>\n";
    return;
  }

  std::cerr << i->unit()->filepath()->data() << ':'
            << i->unit()->getLineNumber(i->offset()) << std::endl;
  for (; i; i = i->next) {
    std::cerr << "  " << i->offset() << ": " << i->toString() << std::endl;
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
               filepath, (unsigned long long)m_funcId,
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

// liveType --
//   Return the live type of a location. If we've already plucked
//   out the cell ...
RuntimeType Translator::liveType(Location l, const Unit& u) {
  Cell *outer;
  switch(l.space) {
    case Location::Stack:
      // Stack accesses must be to addresses pushed before
      // translation time; if they are to addresses pushed after,
      // they should be hitting in the changemap.
      ASSERT(locPhysicalOffset(l) >= 0);
      // fallthru
    case Location::Local: {
      Cell *base;
      int offset = locPhysicalOffset(l);
      base    = l.space == Location::Stack ? vmsp() : vmfp();
      outer = &base[offset];
    } break;
    case Location::Iter: {
      const Iter *it = frame_iter(curFrame(), l.offset);
      TRACE(1, "Iter input: fp %p, iter %p, offset %lld\n", vmfp(),
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
  ASSERT(IS_REAL_TYPE(outer->m_type));
  return liveType(outer, l);
}

RuntimeType
Translator::liveType(const Cell* outer, const Location& l) {
  if (!outer) {
    // An undefined global; starts out as a variant null
    return RuntimeType(KindOfRef, KindOfNull);
  }
  DataType outerType = (DataType)outer->m_type;
  ASSERT(IS_REAL_TYPE(outerType));
  DataType valueType = outerType;
  const Cell* valCell = outer;
  if (outerType == KindOfRef) {
    // Variant. Pick up the inner type, too.
    valCell = outer->m_data.pref->tv();
    DataType innerType = valCell->m_type;
    ASSERT(IS_REAL_TYPE(innerType));
    valueType = innerType;
    ASSERT(innerType != KindOfRef);
    TRACE(2, "liveType Var -> %d\n", innerType);
    return RuntimeType(KindOfRef, innerType);
  }
  const Class *klass = NULL;
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
  // Use the current method's context class (ctx) as a constraint.
  // For instance methods, if $this is non-null, we are guaranteed
  // that $this is an instance of ctx or a class derived from
  // ctx. Zend allows this assumption to be violated but we have
  // deliberately chosen to diverge from them here.
  const Class *ctx = curFunc()->isMethod() ?
    arGetContextClass(curFrame()) : NULL;
  if (ctx) {
    ASSERT(!curFrame()->hasThis() ||
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
  ASSERT(offset >= 0);
  ASSERT(offset < ((ActRec*)frame)->m_func->numLocals());
  TRACE(2, "tvToLocation: %p -> L:%d\n", tv, offset);
  return Location(Location::Local, offset);
}

/* Opcode type-table. */
enum OutTypeConstraints {
  OutNull,
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
  StackTop2 = Stack1 | Stack2,
  StackTop3 = Stack1 | Stack2 | Stack3,
  StackCufSafe = StackIns1 | FStack
};

Operands
operator|(const Operands& l, const Operands& r) {
  return Operands(int(r) | int(l));
}

static int64 typeToMask(DataType t) {
  // KindOfInvalid == -1, so we have to add 2 to make sure t is
  // positive.
  static_assert(KindOfInvalid == -1,
                "assumption for KindOfInvalid value in typeToMask is wrong");
  ASSERT((t+2) > 0 && (t+2) <= 63);
  return (1 << (t+2));
}

struct InferenceRule {
  int64 mask;
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
  static const int64 name ## Mask = typeToMask(KindOf ## name);
TYPE_MASK(Invalid);
TYPE_MASK(Uninit);
TYPE_MASK(Null);
TYPE_MASK(Boolean);
static const int64 IntMask = typeToMask(KindOfInt64);
TYPE_MASK(Double);
static const int64 StringMask = typeToMask(KindOfString) |
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
  return inferType(BitOpRules, ins);
}

static uint32 m_w = 1;    /* must not be zero */
static uint32 m_z = 1;    /* must not be zero */

static uint32 get_random()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

/*
 * predictOutputs --
 *
 *   Provide a best guess for the output type of this instruction.
 */
static DataType
predictOutputs(NormalizedInstruction* ni) {
  if (RuntimeOption::EvalJitStressTypePredPercent &&
      RuntimeOption::EvalJitStressTypePredPercent > int(get_random() % 100)) {
    ni->outputPredicted = true;
    int dt;
    while (true) {
      dt = get_random() % (KindOfRef + 1);
      switch (dt) {
        case KindOfUninit:
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfString:
        case KindOfArray:
        case KindOfObject:
        case KindOfRef:
          break;
        default:
          continue;
      }
      break;
    }
    return DataType(dt);
  }

  if (ni->op() == OpClsCnsD) {
    const NamedEntityPair& cne =
      curFrame()->m_func->unit()->lookupNamedEntityPairId(ni->imm[1].u_SA);
    StringData* cnsName = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
    Class* cls = *cne.second->clsList();
    if (cls && (cls = cls->getCached())) {
      DataType dt = cls->clsCnsType(cnsName);
      if (dt != KindOfUninit) {
        ni->outputPredicted = true;
        TRACE(1, "clscnsd: %s:%s prediction type %d\n",
              cne.first->data(), cnsName->data(), dt);
        return dt;
      }
    }
  }

  std::pair<DataType, double> pred = std::make_pair(KindOfInvalid, 0.0);
  if (hasImmVector(ni->op()) && typeProfileCGetM) {
    const ImmVector& immVec = ni->immVec;
    StringData* name;
    MemberCode mc;
    if (immVec.decodeLastMember(curUnit(), name, mc)) {
      pred = predictType(TypeProfileKey(mc, name));
      TRACE(0, "prediction for %s named %s: %d, %f\n",
            mc == MET ? "elt" : "prop",
            name->data(),
            pred.first,
            pred.second);
    }
  }
  static const double kAccept = 1.00;
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
    return pred.first;
  }
  return KindOfInvalid;
}

/**
 * Returns the type of the value a SetOpL will store into the local.
 */
static RuntimeType setOpOutputType(NormalizedInstruction* ni,
                                   const vector<DynLocation*>& inputs) {
  ASSERT(inputs.size() == 2);
  const int kValIdx = 0;
  const int kLocIdx = 1;
  unsigned char op = ni->imm[1].u_OA;
  DynLocation locLocation(inputs[kLocIdx]->location,
                          inputs[kLocIdx]->rtt.unbox());
  ASSERT(inputs[kLocIdx]->location.isLocal());
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
      ASSERT(false);
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
  ASSERT(constraint != OutFInputL);

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
    case OutPred: return RuntimeType(predictOutputs(ni));

    case OutClassRef: {
      Op op = Op(ni->op());
      if ((op == OpAGetC && inputs[0]->isString())) {
        const StringData *sd = inputs[0]->rtt.valueString();
        if (sd) {
          Class *klass = Unit::lookupClass(sd);
          TRACE(3, "KindOfClass: derived class \"%s\" from string literal\n",
                klass ? klass->preClass()->name()->data() : "NULL");
          return RuntimeType(klass);
        }
      } else if (op == OpSelf) {
        return RuntimeType(curClass());
      } else if (op == OpParent) {
        Class* clss = curClass();
        if (clss != NULL)
          return RuntimeType(clss->parent());
      }
      return RuntimeType(KindOfClass);
    }

    case OutCns: {
      // If it's a system constant, burn in its type. Otherwise we have
      // to accept prediction; use the translation-time value, or fall back
      // to the targetcache if none exists.
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      ASSERT(sd);
      const TypedValue* tv = g_vmContext->getCns(sd, true, false);
      if (tv) {
        return RuntimeType(tv->m_type);
      }
      tv = g_vmContext->getCns(sd);
      if (tv) {
        ni->outputPredicted = true;
        TRACE(1, "CNS %s: guessing runtime type %d\n", sd->data(), tv->m_type);
        return RuntimeType(tv->m_type);
      }
      return RuntimeType(KindOfInvalid);
    }

    case OutStringImm: {
      ASSERT(ni->op() == OpString);
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      ASSERT(sd);
      return RuntimeType(sd);
    }

    case OutArrayImm: {
      ASSERT(ni->op() == OpArray);
      ArrayData *ad = curUnit()->lookupArrayId(ni->imm[0].u_AA);
      ASSERT(ad);
      return RuntimeType(ad);
    }

    case OutBooleanImm: {
      ASSERT(ni->op() == OpTrue || ni->op() == OpFalse);
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
      ASSERT(inputs.size() >= 1);
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
            ASSERT(inputs[idx]->rtt.isVariant() &&
                   !inputs[idx]->isLocal());
          } else {
            ASSERT(inputs[idx]->rtt.valueType() ==
                   inputs[idx]->rtt.outerType());
          }
        }
      }
      return inputs[idx]->rtt;
    }

    case OutCInputL: {
      ASSERT(inputs.size() >= 1);
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
      ASSERT(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      if (in->rtt.outerType() == KindOfRef) {
        return in->rtt.unbox();
      }
      return in->rtt;
    }

    case OutBitOp: {
      ASSERT(inputs.size() == 2 ||
             (inputs.size() == 1 && opcode == OpBitNot));
      if (inputs.size() == 2) {
        return bitOpType(inputs[0], inputs[1]);
      } else {
        return bitOpType(inputs[0], NULL);
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
  { OpRaise,       {StackTop2,        None,         OutNone,          -2 }},
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpZ,        {Stack1,           None,         OutNone,          -1 }},
  { OpJmpNZ,       {Stack1,           None,         OutNone,          -1 }},
  { OpSwitch,      {Stack1,           None,         OutNone,          -1 }},
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
  { OpBindM,       {MVector|Stack1,   Stack1,       OutSameAsInput,    0 }},
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
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallArray,  {FStack,           Stack1,       OutPred,
                                                   -(int)kNumActRecCells }},
  { OpCufSafeArray,{StackTop3|DontGuardAny,
                                      Stack1,       OutArray,         -2 }},
  { OpCufSafeReturn,{StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown,       -2 }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           None,         OutNull,          -1 }},
  { OpIterInitM,   {Stack1,           None,         OutNull,          -1 }},
  { OpIterFree,    {None,             None,         OutNone,           0 }},
  { OpIterValueC,  {Iter,             Stack1,       OutUnknown,        1 }},
  { OpIterValueV,  {Iter,             Stack1,       OutVUnknown,       1 }},
  { OpIterKey,     {Iter,             Stack1,       OutUnknown,        1 }},
  { OpIterNext,    {Iter,             None,         OutNull,           0 }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqMod,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqSrc,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpDefFunc,     {None,             None,         OutNone,           0 }},
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

  /*** 14. Continuation instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutObject,         1 }},
  { OpContEnter,   {None,             None,         OutNone,           0 }},
  { OpContExit,    {None,             None,         OutNone,           0 }},
  { OpUnpackCont,  {Local,            Stack1|Local, OutInt64,          1 }},
  { OpPackCont,    {Local|Stack1,     Local,        OutNone,          -1 }},
  { OpContRaised,  {Local,            None,         OutNone,           0 }},
  { OpContReceive, {Local,            Stack1,       OutUnknown,        1 }},
  { OpContDone,    {Local,            None,         OutNone,           0 }},
  { OpContNext,    {None,             None,         OutNone,           0 }},
  { OpContSend,    {Local,            None,         OutNone,           0 }},
  { OpContRaise,   {Local,            None,         OutNone,           0 }},
  { OpContValid,   {None,             Stack1,       OutBoolean,        1 }},
  { OpContCurrent, {None,             Stack1,       OutUnknown,        1 }},
  { OpContStopped, {None,             None,         OutNone,           0 }},
  { OpContHandle,  {Stack1,           None,         OutNone,          -1 }},
  { OpStrlen,      {Stack1,           Stack1,       OutInt64,          0 }},
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
  ASSERT(ni.immVec.isValid());
  return ni.immVec.numStackValues();
}

int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  Opcode op = ni.op();
  if (op == OpFCall) {
    int numArgs = ni.imm[0].u_IVA;
    return 1 - numArgs - kNumActRecCells;
  }
  if (op == OpNewTuple) {
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
  ASSERT(t.m_instrStream.last);
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
        ASSERT(!prev->deadLocs.size());
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
      prev->outStack = NULL;
      prev->prev->outStack = NULL;
      prev->prev->prev->outStack = NULL;
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
      const Class* cls = Unit::lookupClass(np.second);
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
        prev->outStack = NULL;
        SKTRACE(3, ni->source, "hoisting Pop instruction in analysis\n");
        for (unsigned i = 0; i < ni->deadLocs.size(); ++i) {
          prev->deadLocs.push_back(ni->deadLocs[i]);
        }
        t.m_instrStream.remove(ni);
        if ((prevOp == OpSetM || prevOp == OpSetOpM || prevOp == OpIncDecM) &&
            prev->prev && prev->prev->op() == OpCGetL &&
            prev->prev->inputs[0]->outerType() != KindOfUninit) {
          ASSERT(prev->prev->outStack);
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
          ASSERT(!ni->deadLocs.size());
          t.m_instrStream.remove(ni);
          continue;
      }
    }

    if (op == OpInstanceOfD && prevOp == OpCGetL &&
        (ni->m_txFlags & Supported)) {
      ASSERT(prev->outStack);
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
    NormalizedInstruction* pp = NULL;
    if (prevOp == OpString &&
        (ni->m_txFlags & Supported)) {
      switch (op) {
        case OpReqMod:
        case OpReqSrc:
        case OpReqDoc:
          /* Dont waste a register on the string */
          prev->outStack = NULL;
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
      ASSERT(!ni->outStack);
      ni->grouped = true;
      prev->outStack = NULL;
      pp = prev->prev;
    }

    if (pp && pp->op() == OpPopC &&
        pp->m_txFlags == Native) {
      NormalizedInstruction* ppp = prev->prev->prev;
      if (ppp && (ppp->m_txFlags & Supported)) {
        switch (ppp->op()) {
          case OpReqMod:
          case OpReqSrc:
          case OpReqDoc:
            /*
              We have a require+pop followed by a require or a scalar ret,
              where the pop doesnt have to do any work (the pop is Native).
              There is no need to inc/dec rbx between the two (since
              there will be no code between them)
            */
            ppp->outStack = NULL;
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
  while (ni != NULL) {
    if (ni->outStack == dl ||
        ni->outLocal == dl ||
        ni->outStack2 == dl ||
        ni->outStack3 == dl) {
      break;
    }
    ni = ni->prev;
  }
  return ni;
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
      case Unit::MetaInfo::GuardedThis:
        ni->guardedThis = true;
        break;
      case Unit::MetaInfo::ArrayCapacity:
        ni->imm[0].u_IVA = info.m_data;
        break;

      case Unit::MetaInfo::DataTypePredicted: {
        // If the original type was invalid or predicted, then use the
        // prediction in the meta-data.
        ASSERT((unsigned) arg < inputInfos.size());

        SKTRACE(1, ni->source, "MetaInfo DataTypePredicted for input %d; "
                "newType = %d\n", arg, DataType(info.m_data));
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, m_useHHIR, (DataType)info.m_data);
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
        ASSERT((unsigned)arg < inputInfos.size());
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
            NormalizedInstruction *src = NULL;
            if (mapContains(tas.m_changeSet, dl->location)) {
              src = findInputSrc(tas.m_t->m_instrStream.last, dl);
              if (src && src->outputPredicted) {
                src->outputPredicted = false;
              } else {
                src = NULL;
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
        ASSERT((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        ii.dontGuard = true;
        DynLocation* dl = tas.recordRead(ii, m_useHHIR, KindOfString);
        ASSERT(!dl->rtt.isString() || !dl->rtt.valueString() ||
               dl->rtt.valueString() == sd);
        SKTRACE(1, ni->source, "MetaInfo on input %d; old type = %s\n",
                arg, dl->pretty().c_str());
        dl->rtt = RuntimeType(sd);
        break;
      }

      case Unit::MetaInfo::Class: {
        ASSERT((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, m_useHHIR);
        if (dl->rtt.valueType() != KindOfObject) {
          continue;
        }

        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        const StringData* rttName =
          dl->rtt.valueClass() ? dl->rtt.valueClass()->name() : NULL;
        // The two classes might not be exactly the same, which is ok
        // as long as metaCls is more derived than rttCls.
        Class* metaCls = Unit::lookupClass(metaName);
        Class* rttCls = rttName ? Unit::lookupClass(rttName) : NULL;
        ASSERT(IMPLIES(metaCls && rttCls && metaCls != rttCls,
                       metaCls->classof(rttCls)));
        if (metaCls && metaCls != rttCls) {
          SKTRACE(1, ni->source, "replacing input %d with a MetaInfo-supplied "
                  "class of %s; old type = %s\n",
                  arg, metaName->data(), dl->pretty().c_str());
          if (dl->rtt.isVariant()) {
            dl->rtt = RuntimeType(KindOfRef, KindOfObject, metaCls);
          } else {
            dl->rtt = RuntimeType(KindOfObject, KindOfInvalid, metaCls);
          }
        }
        break;
      }

      case Unit::MetaInfo::MVecPropClass: {
        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        Class* metaCls = Unit::lookupClass(metaName);
        if (metaCls) {
          ni->immVecClasses[arg] = metaCls;
        }
        break;
      }

      case Unit::MetaInfo::NopOut:
        // NopOut should always be the first and only annotation
        // and was handled above.
        not_reached();
      case Unit::MetaInfo::None:
        break;
    }
  } while (metaHand.nextArg(info));

  return false;
}

static void addMVectorInputs(NormalizedInstruction& ni,
                             int& currentStackOffset,
                             std::vector<InputInfo>& inputs) {
  ASSERT(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  currentStackOffset -= ni.immVec.numStackValues();
  int localStackOffset = currentStackOffset;

  auto push_stack = [&] {
    ++stackCount;
    inputs.push_back(Location(Location::Stack, localStackOffset++));
  };
  auto push_local = [&] (int imm) {
    ++localCount;
    inputs.push_back(Location(Location::Local, imm));
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
      inputs.push_back(Location(Location::This));
    } else {
      ASSERT(lcode == LL || lcode == LGL || lcode == LNL);
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
      int64 imm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsLoc(mcode)) {
        push_local(imm);
      } else if (memberCodeImmIsString(mcode)) {
        inputs.push_back(Location(Location::Litstr, imm));
      } else {
        ASSERT(memberCodeImmIsInt(mcode));
        inputs.push_back(Location(Location::Litint, imm));
      }
    } else {
      push_stack();
    }
  }

  if (trailingClassRef) {
    push_stack();
  }

  ni.immVecClasses.resize(ni.immVecM.size());

  ASSERT(vec - ni.immVec.vec() == ni.immVec.size());
  ASSERT(stackCount == ni.immVec.numStackValues());

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
                           InputInfos& inputs) { // out
#ifdef USE_TRACE
  const SrcKey& sk = ni->source;
#endif
  ASSERT(inputs.empty());
  if (debug && !mapContains(instrInfo, ni->op())) {
    fprintf(stderr, "Translator does not understand "
      "instruction %s\n", opcodeToName(ni->op()));
    ASSERT(false);
  }
  const InstrInfo& info = instrInfo[ni->op()];
  Operands input = info.in;
  if (input & FuncdRef) {
    inputs.needsRefCheck = true;
  }
  if (input & Iter) {
    inputs.push_back(Location(Location::Iter, ni->imm[0].u_IVA));
  }
  if (input & FStack) {
    currentStackOffset -= ni->imm[0].u_IVA; // arguments consumed
    currentStackOffset -= kNumActRecCells; // ActRec is torn down as well
  }
  if (input & IgnoreInnerType) ni->ignoreInnerType = true;
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", currentStackOffset - 1);
    inputs.push_back(Location(Location::Stack, --currentStackOffset));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & DontBreakStack1) inputs.back().dontBreak = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.push_back(Location(Location::Stack, --currentStackOffset));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.push_back(Location(Location::Stack, --currentStackOffset));
      }
    }
  }
  if (input & StackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: stackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.push_back(Location(Location::Stack, --currentStackOffset));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
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
      case OpContRaised:
      case OpContDone:
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
    inputs.push_back(Location(Location::Local, loc));
    if (input & DontGuardLocal) inputs.back().dontGuard = true;
    if (input & DontBreakLocal) inputs.back().dontBreak = true;
  }
  if ((input & AllLocals) && freeLocalsInline()) {
    ni->ignoreInnerType = true;
    int n = curFunc()->numLocals();
    for (int i = 0; i < n; ++i) {
      inputs.push_back(Location(Location::Local, i));
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
    inputs.push_back(Location(Location::This));
  }
}

bool outputDependsOnInput(const Opcode instr) {
  switch (instrInfo[instr].type) {
    case OutNull:
    case OutString:
    case OutStringImm:
    case OutDouble:
    case OutBoolean:
    case OutBooleanImm:
    case OutInt64:
    case OutArray:
    case OutObject:
    case OutUnknown:
    case OutVUnknown:
    case OutClassRef:
    case OutNone:
      return false;
    default:
      break;
  }
  return true;
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
    ASSERT(inputs.size() >= 1);
    const DynLocation* in = inputs[inputs.size() - 1];
    DynLocation* outDynLoc = t.newDynLocation(in->location, in->rtt);
    outDynLoc->location = Location(Location::Stack, currentStackOffset++);
    bool isRef;
    if (typeInfo == OutVInputL) {
      isRef = true;
    } else {
      ASSERT(typeInfo == OutFInputL || typeInfo == OutFInputR);
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
    ASSERT(outDynLoc->location.isStack());
    ni->outStack = outDynLoc;

    if (isRef && in->rtt.outerType() != KindOfRef &&
        typeInfo != OutFInputR &&
        in->location.isLocal()) {
      // VGetH or FPassH boxing a local
      DynLocation* smashedLocal =
          t.newDynLocation(in->location, outDynLoc->rtt);
      ASSERT(smashedLocal->location.isLocal());
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
    ASSERT(opnd != None && opnd != Stack3);  // no instr produces 3 values
    ASSERT(opnd != FuncdRef);                // reffiness is immutable
    Location loc;
    switch (opnd) {
      // Pseudo-outputs that affect translator state
      case FStack: {
        currentStackOffset += kNumActRecCells;
        if (op == OpFPushFuncD) {
          const Unit& cu = *ni->unit();
          Id funcId = ni->imm[1].u_SA;
          const NamedEntityPair nep = cu.lookupNamedEntityPairId(funcId);
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
            op == OpBindN || op == OpUnsetN || op == OpUnpackCont ||
            op == OpPackCont) {
          varEnvTaint = true;
          continue;
        }
        ASSERT_NOT_IMPLEMENTED(op == OpSetOpL ||
                               op == OpSetM || op == OpSetOpM ||
                               op == OpIncDecL || op == OpIncDecG ||
                               op == OpUnsetG || op == OpBindG ||
                               op == OpSetG || op == OpSetOpG ||
                               op == OpVGetM ||
                               op == OpStaticLocInit || op == OpInitThisLoc ||
                               op == OpSetL || op == OpBindL ||
                               op == OpUnsetL);
        if (op == OpIncDecL) {
          ASSERT(ni->inputs.size() == 1);
          const RuntimeType &inRtt = ni->inputs[0]->rtt;
          RuntimeType rtt = IS_INT_TYPE(inRtt.valueType()) ? inRtt :
            RuntimeType(KindOfInvalid);
          DynLocation* incDecLoc =
            t.newDynLocation(ni->inputs[0]->location, rtt);
          ASSERT(incDecLoc->location.isLocal());
          ni->outLocal = incDecLoc;
          continue; // Doesn't mutate a loc's types for int. Carry on.
        }
        if (op == OpSetG || op == OpSetOpG ||
            op == OpUnsetG || op == OpBindG ||
            op == OpIncDecG) {
          continue;
        }
        if (op == OpUnsetL) {
          ASSERT(ni->inputs.size() == 1);
          DynLocation* inLoc = ni->inputs[0];
          ASSERT(inLoc->location.isLocal());
          RuntimeType newLhsRtt = RuntimeType(KindOfUninit);
          Location locLocation = inLoc->location;
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  locLocation.spaceName(), locLocation.offset,
                  newLhsRtt.valueType());
          DynLocation* unsetLoc = t.newDynLocation(locLocation, newLhsRtt);
          ASSERT(unsetLoc->location.isLocal());
          ni->outLocal = unsetLoc;
          continue;
        }
        if (op == OpStaticLocInit || op == OpInitThisLoc) {
          ni->outLocal = t.newDynLocation(Location(Location::Local,
                                                   ni->imm[0].u_OA),
                                          KindOfInvalid);
          continue;
        }
        if (op == OpSetM || op == OpSetOpM || op == OpVGetM) {
          // TODO(#1069330): This code assumes that the location is
          // LH. We need to figure out how to handle cases where the
          // location is LN or LG or LR. Also, this code is also
          // assuming that the first member is always E or W, which
          // promotes Null to Array. However, the first member could
          // be P, which promotes Null to Object.
          // XXX: analogous garbage needed for OpSetOpM.
          if (ni->immVec.locationCode() == LL) {
            const int kVecStart = (op == OpSetM || op == OpSetOpM)
              ? 1 : 0; // 0 is rhs for SetM/SetOpM
            DynLocation* inLoc = ni->inputs[kVecStart];
            ASSERT(inLoc->location.isLocal());
            Location locLoc = inLoc->location;
            if (inLoc->rtt.isString() ||
                inLoc->rtt.valueType() == KindOfBoolean) {
              // Strings and bools produce value-dependent results; "" and
              // false upgrade to an array successfully, while other values
              // fail and leave the lhs unmodified.
              DynLocation* baseLoc = t.newDynLocation(locLoc, KindOfInvalid);
              ASSERT(baseLoc->isLocal());
              ni->outLocal = baseLoc;
            } else if (inLoc->rtt.valueType() == KindOfUninit ||
                       inLoc->rtt.valueType() == KindOfNull) {
              RuntimeType newLhsRtt = inLoc->rtt.setValueType(KindOfArray);
              SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                      locLoc.spaceName(), locLoc.offset, newLhsRtt.valueType());
              DynLocation* baseLoc = t.newDynLocation(locLoc, newLhsRtt);
              ASSERT(baseLoc->location.isLocal());
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
          ASSERT(inLoc->location.isLocal());
          DynLocation* dl = t.newDynLocation();
          dl->location = inLoc->location;
          dl->rtt = setOpOutputType(ni, ni->inputs);
          if (inLoc->isVariant()) {
            dl->rtt = dl->rtt.box();
          }
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  inLoc->location.spaceName(), inLoc->location.offset,
                  dl->rtt.valueType());
          ASSERT(dl->location.isLocal());
          ni->outLocal = dl;
          continue;
        }
        ASSERT(ni->inputs.size() == 2);
        const int kValIdx  = 0;
        const int kLocIdx = 1;
        DynLocation* inLoc = ni->inputs[kLocIdx];
        DynLocation* inVal  = ni->inputs[kValIdx];
        Location locLocation = inLoc->location;
        // Variant RHS possible only when binding.
        ASSERT(inVal->rtt.isVagueValue() ||
               (op == OpBindL) ==
               (inVal->rtt.outerType() == KindOfRef));
        ASSERT(!inVal->location.isLocal());
        ASSERT(inLoc->location.isLocal());
        RuntimeType newLhsRtt = inVal->rtt.isVagueValue() || op == OpBindL ?
          inVal->rtt :
          inLoc->rtt.setValueType(inVal->rtt.outerType());
        if (inLoc->rtt.outerType() == KindOfRef) {
          ASSERT(newLhsRtt.outerType() == KindOfRef);
        } else {
          ASSERT(op == OpBindL ||
                 newLhsRtt.outerType() != KindOfRef);
        }
        SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                locLocation.spaceName(), locLocation.offset,
                inVal->rtt.valueType());
        DynLocation* outLhsLoc = t.newDynLocation(locLocation, newLhsRtt);
        ASSERT(outLhsLoc->location.isLocal());
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
        ASSERT(ni->inputs.size() >= 1);
        ni->outStack2 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[0]->rtt
        );
      } break;
      case StackIns2: {
        // Similar to StackIns1.
        loc = Location(Location::Stack, currentStackOffset++);

        // Move the top two locations up a slot.
        ASSERT(ni->inputs.size() >= 2);
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
    ASSERT(dl->location.isStack());
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
    ASSERT(iv.isValid());
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
      int64 lval;
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
    ((Translator::liveFrameIsPseudoMain() && isLocal()) ||
     isVariant());
}

DynLocation* TraceletContext::recordRead(const InputInfo& ii,
                                         bool useHHIR,
                                         DataType staticType) {
  DynLocation* dl;
  const Location& l = ii.loc;
  if (!mapGet(m_currentMap, l, &dl)) {
    // We should never try to read a location that has been deleted
    ASSERT(!mapContains(m_deletedSet, l));
    // If the given location was not in m_currentMap, then it shouldn't
    // be in m_changeSet either
    ASSERT(!mapContains(m_changeSet, l));
    if (ii.dontGuard && !l.isLiteral()) {
      ASSERT(!useHHIR || staticType != KindOfRef);
      dl = m_t->newDynLocation(l, RuntimeType(useHHIR ? staticType
                                                      : KindOfInvalid));
      if (useHHIR && staticType != KindOfInvalid) {
        m_resolvedDeps[l] = dl;
      }
    } else {
      RuntimeType rtt = Translator::liveType(l, *curUnit());
      ASSERT(rtt.isIter() || !rtt.isVagueValue());
      // Allocate a new DynLocation to represent this and store it in the
      // current map.
      dl = m_t->newDynLocation(l, rtt);

      if (!l.isLiteral()) {
        if (m_varEnvTaint && dl->isValue() && dl->isLocal()) {
          dl->rtt = RuntimeType(KindOfInvalid);
        } else if (m_aliasTaint && dl->canBeAliased()) {
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
      TRACE(1, "(%s, %lld) <- inner type invalidated\n",
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
      TRACE(1, "(%s, %lld) <- type invalidated\n",
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
  ASSERT(isValidOpcode(op));
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
                             int& currentStackOffset, Tracelet& t,
                             TraceletContext& tas) {
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

  if ((ni->op() != OpIterValueC && ni->op() != OpIterKey)
      || ni->m_txFlags == Interp) {
    return;
  }
  // examine the next two instructions to see if they are SetL and PopC
  SrcKey src = sk;
  const Unit* unit = ni->m_unit;
  src.advance(unit);
  PC setPC = unit->at(src.offset());
  Opcode setOp = *setPC;
  src.advance(unit);
  Opcode popOp = *unit->at(src.offset());
  if (setOp == OpSetL && popOp == OpPopC) {
    // set outLocal
    DynLocation *dl = ni->outStack;
    ni->outStack = NULL;
    int off = getImm(setPC, 0).u_IA;
    dl->location = Location(Location::Local, off);
    dl->rtt = RuntimeType(KindOfInvalid);
    ni->outLocal = dl;

    // skip SetL and PopC
    sk.advance(ni->m_unit);
    sk.advance(ni->m_unit);

    // record write
    tas.recordWrite(ni->outLocal, ni);
    // restore stack delta
    currentStackOffset--;
    t.m_stackChange--;
  }
}

/*
 * analyze --
 *
 *   Given a sequence of bytecodes, return our tracelet IR.
 */
void Translator::analyze(const SrcKey *csk, Tracelet& t) {
  t.m_sk = *csk;

  TRACE(3, "Translator::analyze %s:%d %s\n",
           curUnit()->filepath()->data(),
           curUnit()->getLineNumber(t.m_sk.offset()),
           curFunc()->fullName()->data());
  TraceletContext tas(&t);
  ImmStack immStack;
  stackFrameOffset = 0;
  int oldStackFrameOffset = 0;

  // numOpcodes counts the original number of opcodes in a tracelet
  // before the translator does any optimization
  t.m_numOpcodes = 0;
  Unit::MetaHandle metaHand;

  /**
   * The purposes of this analysis is to determine:
   *  1. Pre-conditions: What locations get read before they get written to:
   *     we will need typechecks for these and we will want to load them into
   *     registers. (m_dependencies)
   *  2. Post-conditions: the locations that have been written to and are
   *     still live at the end of the tracelet. We need to allocate registers
   *     of these and we need to spill them at the end of the tracelet.
   *     (m_changes)
   *  3. Determine the runtime types for each instruction's input locations
   *     and output locations.
   *
   * The analysis works by doing a single pass over the instructions. It
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
   * instruction executes into the 'outputs' field.
   */

  SrcKey sk = *csk; // copy for local use
  const Unit *unit = curUnit();
  for (;; sk.advance(unit)) {
  head:
    NormalizedInstruction* ni = t.newNormalizedInstruction();
    ni->source = sk;
    ni->stackOff = stackFrameOffset;
    ni->funcd = (t.m_arState.getCurrentState() == ActRecState::KNOWN) ?
      t.m_arState.getCurrentFunc() : NULL;
    ni->m_unit = unit;
    ni->preppedByRef = false;
    ni->breaksTracelet = false;
    ni->changesPC = opcodeChangesPC(ni->op());
    ni->manuallyAllocInputs = false;
    ni->fuseBranch = false;
    ni->outputPredicted = false;
    ni->outputPredictionStatic = false;

    ASSERT(!t.m_analysisFailed);
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
      InputInfos inputInfos;
      getInputs(t, ni, stackFrameOffset, inputInfos);
      bool noOp = applyInputMetaData(metaHand, ni, tas, inputInfos);
      if (noOp) {
        if (RuntimeOption::EvalJitUseIR) {
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
          if (!ni->ignoreInnerType) {
            if (rtt.isValue() && rtt.isVariant() &&
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
      assert(t.m_instrStream.last);
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
        ASSERT(fpi);
        Offset fpushOff = fpi->m_fpushOff;
        PC fpushPc = curUnit()->at(fpushOff);
        if (*fpushPc == OpFPushFunc) {
          doVarEnvTaint = true;
        } else if (*fpushPc == OpFPushFuncD) {
          StringData *funcName =
            curUnit()->lookupLitstrId(getImm(fpushPc, 1).u_SA);
          static const StringData* s_extract =
            StringData::GetStaticString("extract");
          static const StringData* s_hphp_unpack_continuation =
            StringData::GetStaticString("hphp_unpack_continuation");
          doVarEnvTaint = funcName->isame(s_extract) ||
            funcName->isame(s_hphp_unpack_continuation);
        }
      }
      t.m_arState.pop();
    }
    if (doVarEnvTaint) {
      tas.varEnvTaint();
    }

    DynLocation* outputs[] = { ni->outStack, ni->outLocal, ni->outStack2,
                               ni->outStack3 };
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
    ASSERT(getStackDelta(*ni) == (stackFrameOffset - oldStackFrameOffset));
    // If this instruction decreased the depth of the stack, mark the
    // appropriate stack locations as "dead"
    if (stackFrameOffset < oldStackFrameOffset) {
      for (int i = stackFrameOffset; i < oldStackFrameOffset; ++i) {
        Location loc(Location::Stack, i);
        tas.recordDelete(loc);
        ni->deadLocs.push_back(loc);
      }
    }

    t.m_stackChange += getStackDelta(*ni);

    t.m_instrStream.append(ni);
    ++t.m_numOpcodes;

    annotate(ni);
    analyzeInstr(t, *ni);

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
      ASSERT(dbgTranslateCoin);
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
      CT_ASSERT(OpLowInvalid >= 0);
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
    postAnalyze(ni, sk, stackFrameOffset, t, tas);
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
  if (!RuntimeOption::EvalJitUseIR) {
    analyzeSecondPass(t);
  }

  // Mark the last instruction appropriately
  ASSERT(t.m_instrStream.last);
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
}

Translator::Translator() :
    m_resumeHelper(NULL),
    m_useHHIR(false) {
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

uint64* Translator::getTransCounterAddr() {
  if (!isTransDBEnabled()) return NULL;

  TransID id = m_translations.size();

  // allocate a new chunk of counters if necessary
  if (id >= m_transCounters.size() * transCountersPerChunk) {
    uint32   size = sizeof(uint64) * transCountersPerChunk;
    uint64 *chunk = (uint64*)malloc(size);
    bzero(chunk, size);
    m_transCounters.push_back(chunk);
  }
  ASSERT(id / transCountersPerChunk < m_transCounters.size());
  return &(m_transCounters[id / transCountersPerChunk]
           [id % transCountersPerChunk]);
}


uint64 Translator::getTransCounter(TransID transId) const {
  if (!isTransDBEnabled()) return -1ul;
  ASSERT(transId < m_translations.size());

  uint64 counter;

  if (transId / transCountersPerChunk >= m_transCounters.size()) {
    counter = 0;
  } else {
    counter =  m_transCounters[transId / transCountersPerChunk]
                              [transId % transCountersPerChunk];
  }
  return counter;
}

void Translator::setTransCounter(TransID transId, uint64 value) {
  ASSERT(transId < m_translations.size());
  ASSERT(transId / transCountersPerChunk < m_transCounters.size());

  m_transCounters[transId / transCountersPerChunk]
                 [transId % transCountersPerChunk] = value;
}

static const char *transKindStr[] = {
  "Normal",
  "Anchor",
  "Prologue",
};

const char *getTransKindName(TransKind kind) {
  ASSERT(kind >= 0 && kind <= TransProlog);
  return transKindStr[kind];
}

string
TransRec::print(uint64 profCount) const {
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
           "  profCount = %llu\n"
           "  bcMapping = %lu\n",
           id, md5.toString().c_str(), src.m_funcId, src.offset(),
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
  r.m_topFunc = NULL;
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
  ASSERT(outRefDeps);
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
  ASSERT(r.m_topFunc);
  bool retval = r.m_topFunc->byRef(argNum);
  if (r.m_state == GUESSABLE) {
    ASSERT(r.m_entryArDelta != InvalidEntryArDelta);
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    outRefDeps->addDep(r.m_entryArDelta, argNum, retval);
  }
  return retval;
}

const Func*
ActRecState::getCurrentFunc() {
  if (m_arStack.empty()) return NULL;
  return m_arStack.back().m_topFunc;
}

ActRecState::State
ActRecState::getCurrentState() {
  if (m_arStack.empty()) return GUESSABLE;
  return m_arStack.back().m_state;
}

const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return NULL;
  bool privateOnly = false;
  if (!RuntimeOption::RepoAuthoritative ||
      !(cls->preClass()->attrs() & AttrUnique)) {
    Class* ctx = curFunc()->cls();
    if (!ctx || !ctx->classof(cls)) {
      return NULL;
    }
    if (!staticLookup) privateOnly = true;
  }

  const Func* func;
  MethodLookup::LookupResult res = staticLookup ?
    g_vmContext->lookupClsMethod(func, cls, name, 0, false) :
    g_vmContext->lookupObjMethod(func, cls, name, false);

  if (res == MethodLookup::MethodNotFound) return NULL;

  ASSERT(res == MethodLookup::MethodFoundWithThis ||
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
    return NULL;
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
      func = NULL;
    }
  } else if (!(func->attrs() & AttrPrivate)) {
    if (magicCall || func->attrs() & AttrStatic) {
      if (!(cls->preClass()->attrs() & AttrNoOverride)) {
        func = NULL;
      }
    } else if (!(func->attrs() & AttrNoOverride && !func->hasStaticLocals()) &&
               !(cls->preClass()->attrs() & AttrNoOverride)) {
      func = NULL;
    }
  }
  return func;
}

bool freeLocalsInline() {
  ActRec* ar = (ActRec*)vmfp();
  /*
   * If the current frame has a varEnv, we wont take the fast path
   * anyway. Use the presence of a varEnv at translation time as a
   * good guess.
   */
  return !ar->hasVarEnv() &&
    (curFunc()->numLocals() <= Translator::kFewLocals);
}

} } }
