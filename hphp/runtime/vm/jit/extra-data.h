/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <algorithm>

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/phys-loc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/ringbuffer.h"

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

struct LdBindAddrData : IRExtraData {
  explicit LdBindAddrData(SrcKey sk)
    : sk(sk)
  {}

  std::string show() const { return showShort(sk); }

  SrcKey sk;
};

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
    target->numCases   = numCases;
    target->defaultOff = defaultOff;
    target->cases      = new (arena) Elm[numCases];
    std::copy(cases, cases + numCases, const_cast<Elm*>(target->cases));
    return target;
  }

  int64_t     numCases;
  const Elm*  cases;
  Offset      defaultOff;
};

struct JmpSwitchData : IRExtraData {
  JmpSwitchData* clone(Arena& arena) const {
    JmpSwitchData* sd = new (arena) JmpSwitchData;
    sd->base       = base;
    sd->bounded    = bounded;
    sd->cases      = cases;
    sd->defaultOff = defaultOff;
    sd->targets    = new (arena) Offset[cases];
    std::copy(targets, targets + cases, const_cast<Offset*>(sd->targets));
    return sd;
  }

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

struct IterId : IRExtraData {
  explicit IterId(uint32_t id)
    : iterId(id)
  {}

  bool cseEquals(IterId o) const { return iterId == o.iterId; }
  size_t cseHash() const { return std::hash<uint32_t>()(iterId); }
  std::string show() const { return folly::to<std::string>(iterId); }

  uint32_t iterId;
};

struct IterData : IRExtraData {
  explicit IterData(uint32_t iter, uint32_t key, uint32_t val)
    : iterId(iter), keyId(key), valId(val)
  {}
  std::string show() const {
    return folly::format("{}::{}::{}", iterId, valId, valId).str();
  }

  uint32_t iterId;
  uint32_t keyId;
  uint32_t valId;
};

struct RDSHandleData : IRExtraData {
  explicit RDSHandleData(RDS::Handle handle)
    : handle(handle)
  {}

  std::string show() const {
    return folly::to<std::string>(handle);
  }

  RDS::Handle handle;
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
  ClsMethodData(const StringData* cls, const StringData* method,
                const NamedEntity* ne = nullptr)
    : clsName(cls)
    , methodName(method)
    , namedEntity(ne)
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
  const NamedEntity* namedEntity;
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

/*
 * Information for REQ_RETRANSLATE stubs.
 */
struct ReqRetranslateData : IRExtraData {
  TransFlags trflags;

  explicit ReqRetranslateData(TransFlags trflags)
    : trflags(trflags)
  {}

  std::string show() const {
    return folly::to<std::string>(trflags.packed);
  }
};

/*
 * Information for REQ_BIND_JMP stubs.
 */
struct ReqBindJmpData : IRExtraData {
  Offset offset;
  TransFlags trflags;

  explicit ReqBindJmpData(const Offset& offset,
                          TransFlags trflags = TransFlags{})
    : offset(offset)
    , trflags(trflags)
  {}

  std::string show() const {
    return folly::to<std::string>(offset, ',', trflags.packed);
  }
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
  TransFlags trflags;

  std::string show() const {
    return folly::to<std::string>(taken, ',', trflags.packed);
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

  bool isFromFPushCtor() const {
    ActRec ar;
    ar.m_numArgsAndFlags = numArgs;
    return ar.isFromFPushCtor();
  }

  std::string show() const {
    ActRec ar;
    ar.m_numArgsAndFlags = numArgs;
    return folly::to<std::string>(ar.numArgs(),
                                  ar.isFromFPushCtor() ? ",ctor" : "",
                                  ar.resumed() ? ",res" : "",
                                  ar.localsDecRefd() ? ",ldrd" : "",
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

struct ProfileStrData : IRExtraData {
  explicit ProfileStrData(const StringData* key)
    : key(key)
  {}

  std::string show() const { return key->data(); }

  const StringData* key;
};

/*
 * Translation IDs.
 */
struct TransIDData : IRExtraData {
  explicit TransIDData(TransID transId) : transId(transId) {}
  std::string show() const { return folly::to<std::string>(transId); }
  TransID transId;
};

/*
 * Information needed to generate a REQ_RETRANSLATE_OPT service request.
 */
struct ReqRetransOptData : IRExtraData {
  explicit ReqRetransOptData(TransID transId, Offset offset)
      : transId(transId)
      , offset(offset) {}
  std::string show() const {
    return folly::to<std::string>(transId, ", ", offset);
  }
  TransID transId;
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
 * DefInlineFP is present when we need to create a frame for inlining.  This
 * instruction also carries some metadata used by IRBuilder to track state
 * during an inlined call.
 */
struct DefInlineFPData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      target->fullName()->data(), "(),", retBCOff, ',', retSPOff,
      retTypePred < Type::Gen ? (',' + retTypePred.toString()) : ""
    );
  }

  const Func* target;
  Offset retBCOff;
  Offset retSPOff;
  Type retTypePred;
};

struct CallArrayData : IRExtraData {
  explicit CallArrayData(Offset pcOffset, Offset after, bool destroyLocals)
    : pc(pcOffset)
    , after(after)
    , destroyLocals(destroyLocals)
  {}

  std::string show() const {
    return folly::to<std::string>(pc, ",", after,
                                  destroyLocals ? ",destroyLocals" : "");
  }

  Offset pc;                    // XXX why isn't this available in the marker?
  Offset after;
  bool destroyLocals;
};

struct CallBuiltinData : IRExtraData {
  explicit CallBuiltinData(const Func* callee, bool destroyLocals)
    : callee{callee}
    , destroyLocals{destroyLocals}
  {}

  std::string show() const {
    return folly::to<std::string>(
      callee->fullName()->data(),
      destroyLocals ? ",destroyLocals" : ""
    );
  }

  const Func* callee;
  bool destroyLocals;
};

struct CallData : IRExtraData {
  explicit CallData(uint32_t numParams,
                    Offset after,
                    const Func* callee,
                    bool destroy,
                    TCA knownPrologue)
    : numParams(numParams)
    , after(after)
    , callee(callee)
    , destroyLocals(destroy)
    , knownPrologue(knownPrologue)
  {}

  std::string show() const {
    return folly::to<std::string>(
      numParams,
      ',',
      after,
      callee
        ? folly::format(",{}", callee->fullName()->data()).str()
        : std::string{},
      destroyLocals ? ",destroyLocals" : "",
      !!knownPrologue ? ",knownPrologue" : ""
    );
  }

  uint32_t numParams;
  Offset after;
  const Func* callee;  // nullptr if not statically known
  bool destroyLocals;
  TCA knownPrologue;   // nullptr if not statically known
};

struct RetCtrlData : IRExtraData {
  explicit RetCtrlData(bool suspendingResumed)
    : suspendingResumed(suspendingResumed)
  {}

  std::string show() const {
    return suspendingResumed ? "suspending resumed" : "";
  }

  // Indicates that the current generator frame is being suspended without
  // decrefing locals. Used by refcount optimizer.
  bool suspendingResumed;
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
  explicit LdObjMethodData(const StringData* method, bool fatal)
    : method(method)
    , fatal(fatal)
  {}

  std::string show() const {
    return folly::to<std::string>(method->data(), ',',
      fatal ? "fatal" : "warn");
  }

  const StringData* method;
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
 * StackOffset to adjust stack pointer by and boolean indicating whether or not
 * the stack pointer in src1 used for analysis spans a function call.
 *
 * Also contains a list of frame pointers and stack offsets for all enclosing
 * frames. This is used during optimizations to recalculate offsets from the
 * frame pointer when one or more enclosing frames have been elided.
 */
struct ReDefSPData : IRExtraData {
  explicit ReDefSPData(int32_t off, bool spans)
    : spOffset(off)
    , spansCall(spans)
  {}

  std::string show() const {
    return folly::format(
      "{}{}",
      spOffset,
      spansCall ? ",spansCall" : ""
    ).str();
  }

  int32_t spOffset;
  bool spansCall;
};

struct RBTraceData : IRExtraData {
  RBTraceData(Trace::RingBufferType t, SrcKey sk)
    : type(t)
    , sk(sk)
    , msg(nullptr)
  {}

  RBTraceData(Trace::RingBufferType t, const StringData* msg)
    : type(t)
    , sk()
    , msg(msg)
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

  PhysLoc* begin() const { return dests; }
  PhysLoc* end()   const { return dests + size; }

  PhysLoc* dests; // array of up to [cap] PhysLocs
  uint32_t size; // number of valid dests
  uint32_t cap; // available slots for more dests & srcs
};

struct ClassKindData : IRExtraData {
  explicit ClassKindData(ClassKind kind): kind(uint32_t(kind)) {}

  std::string show() const {
    switch (static_cast<ClassKind>(kind)) {
      case ClassKind::Class:     return "cls";
      case ClassKind::Interface: return "interface";
      case ClassKind::Trait:     return "trait";
    }
    not_reached();
  }

  uint32_t kind; // ... allows for direct usage in native_call
};

struct NewStructData : IRExtraData {
  uint32_t numKeys;
  StringData** keys;
  std::string show() const;
};

struct RawMemData : IRExtraData {
# define RAW_MEM_DATA_TYPES                     \
  RAW_TYPE(AsyncState)                          \
  RAW_TYPE(AsyncResumeAddr)                     \
  RAW_TYPE(AsyncResumeOffset)                   \
  RAW_TYPE(ContResumeAddr)                      \
  RAW_TYPE(ContResumeOffset)                    \
  RAW_TYPE(ContState)                           \
  RAW_TYPE(ContIndex)                           \
  RAW_TYPE(StrLen)                              \
  RAW_TYPE(FuncNumParams)                       \

  enum Type : uint8_t {
#   define RAW_TYPE(name) name,
    RAW_MEM_DATA_TYPES
#   undef RAW_TYPE
  };
# define RAW_TYPE(name) +1
  static constexpr size_t kNumTypes = RAW_MEM_DATA_TYPES;
# undef RAW_TYPE

  struct Info {
    const int offset;
    const int size;
    const JIT::Type type;
  };

  explicit RawMemData(Type t) : type(t) {}

  Type type;

  const Info& info() const;
  std::string show() const;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

X(LdBindAddr,                   LdBindAddrData);
X(JmpSwitchDest,                JmpSwitchData);
X(LdSSwitchDestFast,            LdSSwitchData);
X(LdSSwitchDestSlow,            LdSSwitchData);
X(GuardLoc,                     LocalId);
X(CheckLoc,                     LocalId);
X(AssertLoc,                    LocalId);
X(LdLocAddr,                    LocalId);
X(LdLoc,                        LocalId);
X(TrackLoc,                     LocalId);
X(LdGbl,                        LocalId);
X(DecRefLoc,                    LocalId);
X(StLoc,                        LocalId);
X(StGbl,                        LocalId);
X(StLocNT,                      LocalId);
X(IterFree,                     IterId);
X(MIterFree,                    IterId);
X(CIterFree,                    IterId);
X(DecodeCufIter,                IterId);
X(IterInit,                     IterData);
X(IterInitK,                    IterData);
X(IterNext,                     IterData);
X(IterNextK,                    IterData);
X(WIterInit,                    IterData);
X(WIterInitK,                   IterData);
X(WIterNext,                    IterData);
X(WIterNextK,                   IterData);
X(MIterInit,                    IterData);
X(MIterInitK,                   IterData);
X(MIterNext,                    IterData);
X(MIterNextK,                   IterData);
X(ConstructInstance,            ClassData);
X(CheckInitProps,               ClassData);
X(InitProps,                    ClassData);
X(CheckInitSProps,              ClassData);
X(InitSProps,                   ClassData);
X(NewInstanceRaw,               ClassData);
X(InitObjProps,                 ClassData);
X(LdCtx,                        FuncData);
X(CufIterSpillFrame,            FPushCufData);
X(SpillFrame,                   ActRecInfo);
X(GuardStk,                     StackOffset);
X(CheckStk,                     StackOffset);
X(CastStk,                      StackOffset);
X(CastStkIntToDbl,              StackOffset);
X(CoerceStk,                    StackOffset);
X(AssertStk,                    StackOffset);
X(ReDefSP,                      ReDefSPData);
X(DefSP,                        StackOffset);
X(LdStack,                      StackOffset);
X(LdStackAddr,                  StackOffset);
X(DecRefStack,                  StackOffset);
X(DefInlineFP,                  DefInlineFPData);
X(ReqRetranslate,               ReqRetranslateData);
X(ReqBindJmp,                   ReqBindJmpData);
X(ReqRetranslateOpt,            ReqRetransOptData);
X(CheckCold,                    TransIDData);
X(IncProfCounter,               TransIDData);
X(Call,                         CallData);
X(CallBuiltin,                  CallBuiltinData);
X(CallArray,                    CallArrayData);
X(RetCtrl,                      RetCtrlData);
X(FunctionSuspendHook,          RetCtrlData);
X(FunctionReturnHook,           RetCtrlData);
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
X(ReqBindJmpGtInt,              ReqBindJccData);
X(ReqBindJmpGteInt,             ReqBindJccData);
X(ReqBindJmpLtInt,              ReqBindJccData);
X(ReqBindJmpLteInt,             ReqBindJccData);
X(ReqBindJmpEqInt,              ReqBindJccData);
X(ReqBindJmpNeqInt,             ReqBindJccData);
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
X(SideExitJmpGtInt,             SideExitJccData);
X(SideExitJmpGteInt,            SideExitJccData);
X(SideExitJmpLtInt,             SideExitJccData);
X(SideExitJmpLteInt,            SideExitJccData);
X(SideExitJmpEqInt,             SideExitJccData);
X(SideExitJmpNeqInt,            SideExitJccData);
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
X(StClosureFunc,                FuncData);
X(StClosureArg,                 PropByteOffset);
X(RBTrace,                      RBTraceData);
X(Shuffle,                      ShuffleData);
X(OODeclExists,                 ClassKindData);
X(NewStructArray,               NewStructData);
X(LdRaw,                        RawMemData);
X(StRaw,                        RawMemData);
X(StAsyncArRaw,                 RawMemData);
X(LdContArRaw,                  RawMemData);
X(StContArRaw,                  RawMemData);
X(ProfileArray,                 RDSHandleData);
X(ProfileStr,                   ProfileStrData);

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
