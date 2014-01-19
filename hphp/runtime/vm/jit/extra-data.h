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

#ifndef incl_HPHP_VM_EXTRADATA_H_
#define incl_HPHP_VM_EXTRADATA_H_

#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-loc.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * Some IRInstructions with compile-time-only constants may carry
 * along extra data in the form of one of these structures.
 *
 * Note that this isn't really appropriate for compile-time constants
 * that are actually representing user values (we want them to be
 * visible to optimization passes, allocatable to registers, etc),
 * just compile-time metadata.
 *
 * These types must:
 *
 *   - Derive from IRExtraData (for overloading purposes)
 *   - Be arena-allocatable (no non-trivial destructors)
 *   - Either CopyConstructible, or implement a clone member
 *     function that takes an arena to clone to
 *
 * In addition, for extra data used with a cse-able instruction:
 *
 *   - Implement an cseEquals() member that indicates equality for CSE
 *     purposes.
 *   - Implement a cseHash() method.
 *
 * Finally, optionally they may implement a show() method for use in
 * debug printouts.
 */

/*
 * Traits that returns the type of the extra C++ data structure for a
 * given instruction, if it has one, along with some other information
 * about the type.
 */
template<Opcode op> struct OpHasExtraData { enum { value = 0 }; };
template<Opcode op> struct IRExtraDataType;

//////////////////////////////////////////////////////////////////////

struct IRExtraData {};

struct LdSSwitchData : IRExtraData {
  struct Elm {
    const StringData* str;
    Offset            dest;
  };

  explicit LdSSwitchData() = default;
  LdSSwitchData(const LdSSwitchData&) = delete;
  LdSSwitchData& operator=(const LdSSwitchData&) = delete;

  LdSSwitchData* clone(Arena& arena) const {
    LdSSwitchData* target = new (arena) LdSSwitchData;
    target->func       = func;
    target->numCases   = numCases;
    target->defaultOff = defaultOff;
    target->cases      = new (arena) Elm[numCases];
    std::copy(cases, cases + numCases, const_cast<Elm*>(target->cases));
    return target;
  }

  const Func* func;
  int64_t     numCases;
  const Elm*  cases;
  Offset      defaultOff;
};

struct JmpSwitchData : IRExtraData {
  JmpSwitchData* clone(Arena& arena) const {
    JmpSwitchData* sd = new (arena) JmpSwitchData;
    sd->func       = func;
    sd->base       = base;
    sd->bounded    = bounded;
    sd->cases      = cases;
    sd->defaultOff = defaultOff;
    sd->targets    = new (arena) Offset[cases];
    std::copy(targets, targets + cases, const_cast<Offset*>(sd->targets));
    return sd;
  }

  const Func* func;
  int64_t base;        // base of switch case
  bool    bounded;     // whether switch is bounded or not
  int32_t cases;       // number of cases
  Offset  defaultOff;  // offset of default case
  Offset* targets;     // offsets for all targets
};

struct LocalId : IRExtraData {
  explicit LocalId(uint32_t id)
    : locId(id)
  {}

  bool cseEquals(LocalId o) const { return locId == o.locId; }
  size_t cseHash() const { return std::hash<uint32_t>()(locId); }
  std::string show() const { return folly::to<std::string>(locId); }

  uint32_t locId;
};

struct LocalData : LocalId {
  explicit LocalData(uint32_t id, SSATmp* src)
    : LocalId(id)
    , valSrc(src)
  {}

  bool cseEquals(const LocalData& o) const {
    return LocalId::cseEquals(o) && valSrc == o.valSrc;
  }
  size_t cseHash() const {
    return hash_int64_pair(LocalId::cseHash(), int64_t(valSrc));
  }
  std::string show() const;

  SSATmp* valSrc;
};

struct IterId : IRExtraData {
  explicit IterId(uint32_t id)
    : iterId(id)
  {}

  bool cseEquals(IterId o) const { return iterId == o.iterId; }
  size_t cseHash() const { return std::hash<uint32_t>()(iterId); }
  std::string show() const { return folly::to<std::string>(iterId); }

  uint32_t iterId;
};

struct ClassData : IRExtraData {
  explicit ClassData(const Class* cls) : cls(cls) {}
  std::string show() const {
    return folly::to<std::string>(cls->name()->data());
  }
  const Class* cls;
};

struct FuncData : IRExtraData {
  explicit FuncData(const Func* func) : func(func) {}

  bool cseEquals(FuncData o) const { return func == o.func; }
  size_t cseHash() const { return std::hash<const Func*>()(func); }
  std::string show() const {
    return folly::to<std::string>(func->fullName()->data());
  }

  const Func* func;
};

struct ClsMethodData : IRExtraData {
  ClsMethodData(const StringData* cls, const StringData* method)
    : clsName(cls)
    , methodName(method)
  {}

  std::string show() const {
    return folly::format("{}::{}", *clsName, *methodName).str();
  }

  bool cseEquals(const ClsMethodData& b) const {
    // Strings are static so we can use pointer equality
    return clsName == b.clsName && methodName == b.methodName;
  }
  size_t cseHash() const {
    return hash_int64_pair((uintptr_t)clsName, (uintptr_t)methodName);
  }

  const StringData* clsName;
  const StringData* methodName;
};

struct FPushCufData : IRExtraData {
  FPushCufData(uint32_t a, int32_t id)
    : args(a), iterId(id)
  {}

  bool cseEquals(FPushCufData o) const {
    return iterId == o.iterId && args == o.args;
  }
  size_t cseHash() const {
    return std::hash<uint32_t>()(iterId) ^ std::hash<uint32_t>()(args);
  }
  std::string show() const {
    return folly::to<std::string>(iterId, ',', args);
  }

  uint32_t args;
  uint32_t iterId;
};

struct ConstData : IRExtraData {
  template<class T>
  explicit ConstData(T data)
    : m_dataBits(constToBits(data))
  {}

  template<class T>
  T as() const {
    T ret;
    std::memcpy(&ret, &m_dataBits, sizeof ret);
    return ret;
  }

  bool cseEquals(ConstData o) const { return m_dataBits == o.m_dataBits; }
  size_t cseHash() const { return std::hash<uintptr_t>()(m_dataBits); }

private:
  uintptr_t m_dataBits;
};

/*
 * Information for the REQ_BIND_JMPCC stubs we create when a tracelet
 * ends with conditional jumps.
 */
struct ReqBindJccData : IRExtraData {
  Offset taken;
  Offset notTaken;

  std::string show() const {
    return folly::to<std::string>(taken, ',', notTaken);
  }
};

/*
 * Information for REQ_BIND_SIDE_EXIT stubs created from a conditional jump.
 */
struct SideExitJccData : IRExtraData {
  Offset taken;

  std::string show() const {
    return folly::to<std::string>(taken);
  }
};

/*
 * Information for a conditional side exit based on a type check of a
 * local or stack cell.
 */
struct SideExitGuardData : IRExtraData {
  uint32_t checkedSlot;
  Offset taken;

  std::string show() const {
    return folly::to<std::string>(checkedSlot, ',', taken);
  }
};

/*
 * Compile-time metadata about an ActRec allocation.
 */
struct ActRecInfo : IRExtraData {
  const StringData* invName;  // may be nullptr
  int32_t numArgs;

  std::string show() const {
    auto numArgsAndCtorFlag = ActRec::decodeNumArgs(numArgs);
    return folly::to<std::string>(numArgsAndCtorFlag.first,
                                  numArgsAndCtorFlag.second ? ",ctor" : "",
                                  invName ? " M" : "");
  }
};

/*
 * Parameter index for type profiling.
 */
struct TypeProfileData : IRExtraData {
  explicit TypeProfileData(int32_t param) : param(param) {}
  int32_t param;
  std::string show() const {
    return folly::to<std::string>(param);
  }
};

/*
 * Stack offsets.
 */
struct StackOffset : IRExtraData {
  explicit StackOffset(int32_t offset) : offset(offset) {}

  std::string show() const { return folly::to<std::string>(offset); }

  bool cseEquals(StackOffset o) const { return offset == o.offset; }
  size_t cseHash() const { return std::hash<int32_t>()(offset); }

  int32_t offset;
};

/*
 * Bytecode offsets.
 */
struct BCOffset : IRExtraData {
  explicit BCOffset(Offset offset) : offset(offset) {}
  std::string show() const { return folly::to<std::string>(offset); }
  Offset offset;
};

/*
 * Translation IDs.
 */
struct TransIDData : IRExtraData {
  explicit TransIDData(JIT::TransID transId) : transId(transId) {}
  std::string show() const { return folly::to<std::string>(transId); }
  JIT::TransID transId;
};

/*
 * Information needed to generate a REQ_RETRANSLATE_OPT service request.
 */
struct ReqRetransOptData : IRExtraData {
  explicit ReqRetransOptData(JIT::TransID transId, Offset offset)
      : transId(transId)
      , offset(offset) {}
  std::string show() const {
    return folly::to<std::string>(transId, ", ", offset);
  }
  JIT::TransID transId;
  Offset offset;
};

/*
 * Offset to a TypedValue from some base pointer, in bytes.  (E.g. to
 * a object property slot.)
 */
struct PropByteOffset : IRExtraData {
  explicit PropByteOffset(size_t offsetBytes) : offsetBytes(offsetBytes) {}
  std::string show() const { return folly::to<std::string>(offsetBytes); }
  size_t offsetBytes;
};

/*
 * DefInlineFP is present when we need to create a frame for inlining.
 * This instruction also carries some metadata used by tracebuilder to
 * track state during an inlined call.
 */
struct DefInlineFPData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      target->fullName()->data(), "(),", retBCOff, ',', retSPOff
    );
  }

  const Func* target;
  Offset retBCOff;
  Offset retSPOff;
};

/*
 * FCallArray offsets
 */
struct CallArrayData : IRExtraData {
  explicit CallArrayData(Offset pcOffset, Offset aft, bool destroyLocals)
    : pc(pcOffset)
    , after(aft)
    , destroyLocals(destroyLocals)
  {}

  std::string show() const {
    return folly::to<std::string>(pc, ",", after,
                                  destroyLocals ? " destroy locals" : "");
  }

  Offset pc, after;
  bool destroyLocals;
};

struct CallData : IRExtraData {
  explicit CallData(bool destroy)
    : destroyLocals(destroy)
  {}

  std::string show() const {
    return destroyLocals ? "destroy locals" : "";
  }

  bool destroyLocals;
};

struct InGeneratorData : IRExtraData {
  explicit InGeneratorData(bool inGenerator)
    : inGenerator(inGenerator)
  {}

  std::string show() const {
    return inGenerator ? "in generator" : "";
  }

  bool inGenerator;
};

/*
 * Name of a class constant.
 */
struct ClsCnsName : IRExtraData {
  explicit ClsCnsName(const StringData* cls, const StringData* cns)
    : clsName(cls)
    , cnsName(cns)
  {}

  std::string show() const {
    return folly::to<std::string>(clsName->data(), "::", cnsName->data());
  }

  const StringData* clsName;
  const StringData* cnsName;
};

/*
 * The name of a static local in a function.
 */
struct StaticLocName : IRExtraData {
  StaticLocName(const Func* func, const StringData* name)
    : func(func)
    , name(name)
  {}

  std::string show() const {
    return folly::to<std::string>(
      func->fullName()->data(), "$", name->data()
    );
  }

  const Func* func;
  const StringData* name;
};

struct LdFuncCachedData : IRExtraData {
  explicit LdFuncCachedData(const StringData* name)
    : name(name)
  {}

  std::string show() const {
    return folly::to<std::string>(name->data());
  }

  size_t cseHash() const { return name->hash(); }
  bool cseEquals(const LdFuncCachedData& o) const {
    return name == o.name;
  }

  const StringData* name;
};

struct LdObjMethodData : IRExtraData {
  explicit LdObjMethodData(bool fatal)
    : fatal(fatal)
  {}

  std::string show() const {
    return folly::to<std::string>(fatal ? "fatal" : "warn");
  }

  bool fatal;
};

struct LdFuncCachedUData : IRExtraData {
  explicit LdFuncCachedUData(const StringData* name,
                             const StringData* fallback)
    : name(name)
    , fallback(fallback)
  {}

  std::string show() const {
    return folly::to<std::string>(name->data(), ',', fallback->data());
  }

  size_t cseHash() const {
    return hash_int64_pair(name->hash(), fallback->hash());
  }
  bool cseEquals(const LdFuncCachedUData& o) const {
    return name == o.name && fallback == o.fallback;
  }

  const StringData* name;
  const StringData* fallback;
};

/*
 * The name of a class, and the expected Class* at runtime.
 */
struct CheckDefinedClsData : IRExtraData {
  CheckDefinedClsData(const StringData* clsName, const Class* cls)
    : clsName(clsName)
    , cls(cls)
  {}

  std::string show() const {
    return folly::to<std::string>(clsName->data());
  }

  const StringData* clsName;
  const Class* cls;
};

/*
 * Offset and stack deltas for InterpOne.
 */
struct InterpOneData : IRExtraData {
  struct LocalType {
    explicit LocalType(uint32_t id = 0, Type type = Type::Bottom)
      : id(id)
      , type(type)
    {}

    uint32_t id;
    Type type;
  };

  InterpOneData()
    : nChangedLocals(0)
    , changedLocals(nullptr)
    , smashesAllLocals(false)
  {}

  // Offset of the instruction to interpret, in the Unit indicated by
  // the current Marker.
  Offset bcOff;

  // The number of stack cells consumed and produced by the
  // instruction, respectively. Includes ActRecs.
  int64_t cellsPopped;
  int64_t cellsPushed;

  // Opcode, in case we need to fix the stack differently. Some byte-
  // code instructions modify things below the top of the stack.
  Op opcode;

  uint32_t nChangedLocals;
  LocalType* changedLocals;

  bool smashesAllLocals;

  InterpOneData* clone(Arena& arena) const {
    auto* id = new (arena) InterpOneData;
    id->bcOff = bcOff;
    id->cellsPopped = cellsPopped;
    id->cellsPushed = cellsPushed;
    id->opcode = opcode;
    id->nChangedLocals = nChangedLocals;
    id->changedLocals = new (arena) LocalType[nChangedLocals];
    id->smashesAllLocals = smashesAllLocals;
    std::copy(changedLocals, changedLocals + nChangedLocals, id->changedLocals);
    return id;
  }

  std::string show() const {
    std::string ret = folly::format("{}: bcOff:{}, popped:{}, pushed:{}",
                                    opcodeToName(opcode), bcOff, cellsPopped,
                                    cellsPushed).str();

    assert(!smashesAllLocals || !nChangedLocals);
    if (smashesAllLocals) ret += ", smashes all locals";
    if (nChangedLocals) {
      for (auto i = 0; i < nChangedLocals; ++i) {
        ret += folly::format(", Local {} -> {}",
                             changedLocals[i].id, changedLocals[i].type).str();
      }
    }

    return ret;
  }
};

/*
 * Information for creating continuation objects.
 * Create{Cont,AFWH}{Func,Meth}.
 */
struct CreateContData : IRExtraData {
  explicit CreateContData(const Func* genFunc) : genFunc(genFunc) {}

  std::string show() const {
    auto name = genFunc->getGeneratorOrigFunc()->fullName()->data();
    return folly::to<std::string>(name, "()");
  }

  const Func* genFunc;
};

/*
 * Important during offset to determine if crossing inline function will also
 * cross function call boundary.
 */
struct ReDefGeneratorSPData : IRExtraData {
  explicit ReDefGeneratorSPData(bool spans) : spansCall(spans) {}

  std::string show() const {
    return folly::to<std::string>(spansCall);
  }

  bool spansCall;
};

/*
 * StackOffset to adjust stack pointer by and boolean indicating whether or not
 * the stack pointer in src1 used for analysis spans a function call.
 *
 * Also contains a list of frame pointers and stack offsets for all enclosing
 * frames. This is used during optimizations to recalculate offsets from the
 * frame pointer when one or more enclosing frames have been elided.
 */
struct ReDefSPData : IRExtraData {
  struct Frame {
    explicit Frame(const SSATmp* fp = nullptr, int32_t spOff = 0)
      : fp(fp)
      , spOff(spOff)
    {}

    const SSATmp* fp;
    int32_t spOff;
  };

  explicit ReDefSPData(uint32_t nFrames, Frame* frames, int32_t off, bool spans)
    : frames(frames)
    , nFrames(nFrames)
    , spOffset(off)
    , spansCall(spans)
  {}

  ReDefSPData* clone(Arena& arena) const {
    auto* r = new (arena) ReDefSPData(nFrames, frames, spOffset, spansCall);
    r->frames = new (arena) Frame[nFrames];
    std::copy(frames, frames + nFrames, r->frames);
    return r;
  }

  std::string show() const {
    return folly::format("spOff:{}{}", spOffset,
                         spansCall ? ", spans call" : "").str();
  }

  Frame* frames;
  uint32_t nFrames;

  int32_t spOffset;
  bool spansCall;
};

struct RBTraceData : IRExtraData {
  RBTraceData(Trace::RingBufferType t, SrcKey sk_)
    : type(t)
    , sk(sk_)
    , msg(nullptr)
  {}

  RBTraceData(Trace::RingBufferType t, const StringData* msg_)
    : type(t)
    , sk()
    , msg(msg_)
  {
    assert(msg->isStatic());
  }

  std::string show() const {
    auto const data = msg ? msg->data() : showShort(sk);
    return folly::format("{}: {}", ringbufferName(type), data).str();
  }

  Trace::RingBufferType type;
  SrcKey sk;
  const StringData* msg;
};

/*
 * ShuffleData holds an array of destination locations for a Shuffle,
 * one per source, as well as a capacity field so we can track the
 * available space to add more srcs and dsts without reallocating.
 */
struct ShuffleData : IRExtraData {
  ShuffleData(PhysLoc* dests, uint32_t size, uint32_t cap)
    : dests(dests), size(size), cap(cap)
  {}

  std::string show() const;

  PhysLoc* dests; // array of up to [cap] PhysLocs
  uint32_t size; // number of valid dests
  uint32_t cap; // available slots for more dests & srcs
};

struct ClassKindData : IRExtraData {
  explicit ClassKindData(ClassKind kind) : kind(kind) {}

  std::string show() const {
    switch (kind) {
    case ClassKind::Class:     return "cls";
    case ClassKind::Interface: return "interface";
    case ClassKind::Trait:     return "trait";
    }
    not_reached();
  }

  ClassKind kind;
};

struct NewStructData : IRExtraData {
  uint32_t numKeys;
  StringData** keys;
  std::string show() const;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

X(JmpSwitchDest,                JmpSwitchData);
X(LdSSwitchDestFast,            LdSSwitchData);
X(LdSSwitchDestSlow,            LdSSwitchData);
X(GuardLoc,                     LocalId);
X(CheckLoc,                     LocalId);
X(AssertLoc,                    LocalId);
X(OverrideLocVal,               LocalId);
X(LdLocAddr,                    LocalData);
X(LdLoc,                        LocalData);
X(DecRefLoc,                    LocalId);
X(StLoc,                        LocalId);
X(StLocNT,                      LocalId);
X(IterFree,                     IterId);
X(MIterFree,                    IterId);
X(CIterFree,                    IterId);
X(DecodeCufIter,                IterId);
X(AllocObjFast,                 ClassData);
X(LdCtx,                        FuncData);
X(CufIterSpillFrame,            FPushCufData);
X(DefConst,                     ConstData);
X(LdConst,                      ConstData);
X(SpillFrame,                   ActRecInfo);
X(GuardStk,                     StackOffset);
X(CheckStk,                     StackOffset);
X(CastStk,                      StackOffset);
X(CoerceStk,                    StackOffset);
X(AssertStk,                    StackOffset);
X(AssertStkVal,                 StackOffset);
X(ReDefSP,                      ReDefSPData);
X(ReDefGeneratorSP,             ReDefGeneratorSPData);
X(DefSP,                        StackOffset);
X(DefInlineSP,                  StackOffset);
X(LdStack,                      StackOffset);
X(LdStackAddr,                  StackOffset);
X(DecRefStack,                  StackOffset);
X(DefInlineFP,                  DefInlineFPData);
X(ReqBindJmp,                   BCOffset);
X(ReqRetranslateOpt,            ReqRetransOptData);
X(CheckCold,                    TransIDData);
X(IncProfCounter,               TransIDData);
X(Call,                         CallData);
X(CallBuiltin,                  CallData);
X(CallArray,                    CallArrayData);
X(RetCtrl,                      InGeneratorData);
X(FunctionExitSurpriseHook,     InGeneratorData);
X(LdClsCns,                     ClsCnsName);
X(LookupClsCns,                 ClsCnsName);
X(LookupClsMethodCache,         ClsMethodData);
X(LdClsMethodCacheFunc,         ClsMethodData);
X(LdClsMethodCacheCls,          ClsMethodData);
X(LdClsMethodFCacheFunc,        ClsMethodData);
X(LookupClsMethodFCache,        ClsMethodData);
X(GetCtxFwdCallDyn,             ClsMethodData);
X(LdStaticLocCached,            StaticLocName);
X(LdFuncCached,                 LdFuncCachedData);
X(LdFuncCachedSafe,             LdFuncCachedData);
X(LdFuncCachedU,                LdFuncCachedUData);
X(LdObjMethod,                  LdObjMethodData);
X(ReqBindJmpGt,                 ReqBindJccData);
X(ReqBindJmpGte,                ReqBindJccData);
X(ReqBindJmpLt,                 ReqBindJccData);
X(ReqBindJmpLte,                ReqBindJccData);
X(ReqBindJmpEq,                 ReqBindJccData);
X(ReqBindJmpNeq,                ReqBindJccData);
X(ReqBindJmpSame,               ReqBindJccData);
X(ReqBindJmpNSame,              ReqBindJccData);
X(ReqBindJmpInstanceOfBitmask,  ReqBindJccData);
X(ReqBindJmpNInstanceOfBitmask, ReqBindJccData);
X(ReqBindJmpZero,               ReqBindJccData);
X(ReqBindJmpNZero,              ReqBindJccData);
X(SideExitJmpGt,                SideExitJccData);
X(SideExitJmpGte,               SideExitJccData);
X(SideExitJmpLt,                SideExitJccData);
X(SideExitJmpLte,               SideExitJccData);
X(SideExitJmpEq,                SideExitJccData);
X(SideExitJmpNeq,               SideExitJccData);
X(SideExitJmpSame,              SideExitJccData);
X(SideExitJmpNSame,             SideExitJccData);
X(SideExitJmpInstanceOfBitmask, SideExitJccData);
X(SideExitJmpNInstanceOfBitmask,SideExitJccData);
X(SideExitJmpZero,              SideExitJccData);
X(SideExitJmpNZero,             SideExitJccData);
X(SideExitGuardLoc,             SideExitGuardData);
X(SideExitGuardStk,             SideExitGuardData);
X(CheckDefinedClsEq,            CheckDefinedClsData);
X(InterpOne,                    InterpOneData);
X(TypeProfileFunc,              TypeProfileData);
X(InterpOneCF,                  InterpOneData);
X(CreateContFunc,               CreateContData);
X(CreateContMeth,               CreateContData);
X(CreateAFWHFunc,               CreateContData);
X(CreateAFWHMeth,               CreateContData);
X(StClosureFunc,                FuncData);
X(StClosureArg,                 PropByteOffset);
X(RBTrace,                      RBTraceData);
X(Shuffle,                      ShuffleData);
X(ThingExists,                  ClassKindData);
X(NewStructArray,               NewStructData);

#undef X

//////////////////////////////////////////////////////////////////////

template<bool hasExtra, Opcode opc, class T> struct AssertExtraTypes {
  static void doassert() {
    assert(!"called extra on an opcode without extra data");
  }
  static void doassert_same() {
    assert(!"called extra on an opcode without extra data");
  }
};

template<Opcode opc, class T> struct AssertExtraTypes<true,opc,T> {
  typedef typename IRExtraDataType<opc>::type ExtraType;

  static void doassert() {
    if (!std::is_base_of<T,ExtraType>::value) {
      assert(!"extra<T> was called with an extra data "
              "type that doesn't match the opcode type");
    }
  }
  static void doassert_same() {
    if (!std::is_same<T,ExtraType>::value) {
      assert(!"extra<T> was called with an extra data type that "
             "doesn't exactly match the opcode type");
    }
  }
};

// Asserts that Opcode opc has extradata and it is of type T, or a
// type derived from T.
template<class T> void assert_opcode_extra(Opcode opc) {
#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode:                                  \
    AssertExtraTypes<                           \
      OpHasExtraData<opcode>::value,opcode,T    \
    >::doassert();                              \
    break;
  switch (opc) { IR_OPCODES default: not_reached(); }
#undef O
}

template<class T> void assert_opcode_extra_same(Opcode opc) {
#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode:                                  \
    AssertExtraTypes<                           \
      OpHasExtraData<opcode>::value,opcode,T    \
    >::doassert_same();                         \
    break;
  switch (opc) { IR_OPCODES default: not_reached(); }
#undef O
}

size_t cseHashExtra(Opcode opc, IRExtraData* data);
bool cseEqualsExtra(Opcode opc, IRExtraData* a, IRExtraData* b);
IRExtraData* cloneExtra(Opcode opc, IRExtraData* data, Arena& a);
std::string showExtra(Opcode opc, const IRExtraData* data);

//////////////////////////////////////////////////////////////////////

}}

#endif
