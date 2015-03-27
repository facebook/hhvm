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

#include "hphp/runtime/ext/ext_generator.h"

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/util/arena.h"
#include "hphp/util/ringbuffer.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Some IRInstructions with compile-time-only constants may carry along extra
 * data in the form of one of these structures.
 *
 * Note that this isn't really appropriate for compile-time constants that are
 * actually representing user values (we want them to be visible to
 * optimization passes, allocatable to registers, etc), just compile-time
 * metadata.
 *
 * These types must:
 *
 *   - Derive from IRExtraData (for overloading purposes).
 *   - Be arena-allocatable (no non-trivial destructors).
 *   - Either CopyConstructible, or implement a clone member function that
 *     takes an arena to clone to.
 *
 * In addition, extra data belonging to IRInstructions that may be hashed in
 * IRInstrTables must:
 *
 *   - Implement an equals() member that indicates equality.
 *   - Implement a hash() method.
 *
 * Finally, optionally they may implement a show() method for use in debug
 * printouts.
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
    SrcKey            dest;
  };

  explicit LdSSwitchData() = default;
  LdSSwitchData(const LdSSwitchData&) = delete;
  LdSSwitchData& operator=(const LdSSwitchData&) = delete;

  LdSSwitchData* clone(Arena& arena) const {
    LdSSwitchData* target = new (arena) LdSSwitchData;
    target->numCases   = numCases;
    target->defaultSk  = defaultSk;
    target->cases      = new (arena) Elm[numCases];
    std::copy(cases, cases + numCases, const_cast<Elm*>(target->cases));
    return target;
  }

  int64_t     numCases;
  const Elm*  cases;
  SrcKey      defaultSk;
};

struct JmpSwitchData : IRExtraData {
  JmpSwitchData* clone(Arena& arena) const {
    JmpSwitchData* sd = new (arena) JmpSwitchData;
    sd->base       = base;
    sd->bounded    = bounded;
    sd->cases      = cases;
    sd->defaultSk  = defaultSk;
    sd->targets    = new (arena) SrcKey[cases];
    std::copy(targets, targets + cases, const_cast<SrcKey*>(sd->targets));
    return sd;
  }

  int64_t base;        // base of switch case
  bool    bounded;     // whether switch is bounded or not
  int32_t cases;       // number of cases
  SrcKey  defaultSk;   // srckey of default case
  SrcKey* targets;     // srckeys for all targets
};

struct LocalId : IRExtraData {
  explicit LocalId(uint32_t id)
    : locId(id)
  {}

  bool equals(LocalId o) const { return locId == o.locId; }
  size_t hash() const { return std::hash<uint32_t>()(locId); }
  std::string show() const { return folly::to<std::string>(locId); }

  uint32_t locId;
};

struct IterId : IRExtraData {
  explicit IterId(uint32_t id)
    : iterId(id)
  {}

  bool equals(IterId o) const { return iterId == o.iterId; }
  size_t hash() const { return std::hash<uint32_t>()(iterId); }
  std::string show() const { return folly::to<std::string>(iterId); }

  uint32_t iterId;
};

struct IterData : IRExtraData {
  explicit IterData(uint32_t iter, uint32_t key, uint32_t val)
    : iterId(iter), keyId(key), valId(val)
  {}
  std::string show() const {
    if (keyId == -1) return folly::format("{}::{}", iterId, valId).str();

    return folly::format("{}::{}::{}", iterId, keyId, valId).str();
  }

  uint32_t iterId;
  uint32_t keyId;
  uint32_t valId;
};

struct RDSHandleData : IRExtraData {
  explicit RDSHandleData(rds::Handle handle)
    : handle(handle)
  {}

  bool equals(RDSHandleData o) const { return handle == o.handle; }
  size_t hash() const { return std::hash<uint32_t>()(handle); }
  std::string show() const {
    return folly::to<std::string>(handle);
  }

  rds::Handle handle;
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

  bool equals(FuncData o) const { return func == o.func; }
  size_t hash() const { return std::hash<const Func*>()(func); }
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

  bool equals(const ClsMethodData& b) const {
    // Strings are static so we can use pointer equality
    return clsName == b.clsName && methodName == b.methodName;
  }
  size_t hash() const {
    return hash_int64_pair((uintptr_t)clsName, (uintptr_t)methodName);
  }

  const StringData* clsName;
  const StringData* methodName;
  const NamedEntity* namedEntity;
};

struct FPushCufData : IRExtraData {
  FPushCufData(IRSPOffset spOffset, uint32_t a, int32_t id)
    : spOffset(spOffset)
    , args(a)
    , iterId(id)
  {}

  bool equals(FPushCufData o) const {
    return iterId == o.iterId && args == o.args;
  }
  size_t hash() const {
    return std::hash<uint32_t>()(iterId) ^ std::hash<uint32_t>()(args);
  }
  std::string show() const {
    return folly::to<std::string>(spOffset.offset, ',', iterId, ',', args);
  }

  IRSPOffset spOffset;
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
  SrcKey dest;
  TransFlags trflags;

  explicit ReqBindJmpData(const SrcKey& dest,
                          TransFlags trflags = TransFlags{})
    : dest(dest)
    , trflags(trflags)
  {}

  std::string show() const {
    return folly::to<std::string>(dest.offset(), ',', trflags.packed);
  }
};

/*
 * Compile-time metadata about an ActRec allocation.
 */
struct ActRecInfo : IRExtraData {
  IRSPOffset spOffset;
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
    return folly::to<std::string>(spOffset.offset, ',',
                                  ar.numArgs(),
                                  ar.isFromFPushCtor() ? ",ctor" : "",
                                  ar.resumed() ? ",res" : "",
                                  ar.localsDecRefd() ? ",ldrd" : "",
                                  invName ? " M" : "");
  }
};

struct StackOffset : IRExtraData {
  explicit StackOffset(int32_t offset) : offset(offset) {}

  std::string show() const { return folly::to<std::string>(offset); }

  bool equals(StackOffset o) const { return offset == o.offset; }
  size_t hash() const { return std::hash<int32_t>()(offset); }

  int32_t offset;
};

/*
 * This particular ExtraData exists because we need both the BCSPOffset and
 * the IRSPOffset for a particular stack entry. The BCSPOffset is used in
 * visitGuards in region-tracelet.cpp at which point it would be difficult to
 * derive the BCSPOffset from the IRSPOffset. The IRSPOffset is used during
 * code gen. Thus, we pass both using this struct.
 */
struct RelOffsetData : IRExtraData {
  explicit RelOffsetData(BCSPOffset bcSpOffset, IRSPOffset irSpOffset)
    : bcSpOffset(bcSpOffset)
    , irSpOffset(irSpOffset)
  {
  }

  std::string show() const {
    return folly::to<std::string>(
      "BcSpOff ", bcSpOffset.offset, ", ",
      "IrSpOff ", irSpOffset.offset
    );
  }

  bool equals(RelOffsetData o) const {
    return bcSpOffset == o.bcSpOffset && irSpOffset == o.irSpOffset;
  }
  size_t hash() const {
    return hash_int64_pair(
      std::hash<int32_t>()(bcSpOffset.offset),
      std::hash<int32_t>()(irSpOffset.offset)
    );
  }

  BCSPOffset bcSpOffset;
  IRSPOffset irSpOffset;
};

struct IRSPOffsetData : IRExtraData {
  explicit IRSPOffsetData(IRSPOffset offset) : offset(offset) {}

  std::string show() const {
    return folly::to<std::string>("IrSpOff ", offset.offset);
  }

  bool equals(IRSPOffsetData o) const { return offset == o.offset; }
  size_t hash() const { return std::hash<int32_t>()(offset.offset); }

  IRSPOffset offset;
};

struct PropOffset : IRExtraData {
  explicit PropOffset(int32_t offset) : offsetBytes(offset) {}

  std::string show() const { return folly::to<std::string>(offsetBytes); }
  bool equals(PropOffset o) const { return offsetBytes == o.offsetBytes; }
  size_t hash() const { return std::hash<int32_t>()(offsetBytes); }

  int32_t offsetBytes;
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
  explicit ReqRetransOptData(TransID transId, SrcKey sk)
      : transId(transId), sk(sk) {}
  std::string show() const {
    return folly::to<std::string>(transId, ',', sk.offset());
  }
  TransID transId;
  SrcKey sk;
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
      target->fullName()->data(), "(),",
      fromFPushCtor ? "ctor," : "",
      retBCOff, ',',
      retSPOff.offset, ',',
      spOffset
    );
  }

  const Func* target;
  bool fromFPushCtor;
  SSATmp* ctx;       // Ctx, Cls or Nullptr.
  Offset retBCOff;
  FPAbsOffset retSPOff;
  int32_t spOffset;  // offset from caller SP to callee SP
};

struct CallArrayData : IRExtraData {
  explicit CallArrayData(IRSPOffset spOffset,
                         Offset pcOffset,
                         Offset after,
                         bool destroyLocals)
    : spOffset(spOffset)
    , pc(pcOffset)
    , after(after)
    , destroyLocals(destroyLocals)
  {}

  std::string show() const {
    return folly::to<std::string>(pc, ",", after,
                                  destroyLocals ? ",destroyLocals" : "");
  }

  IRSPOffset spOffset;    // offset from StkPtr to bottom of call's ActRec+args
  Offset pc;     // XXX why isn't this available in the marker?
  Offset after;  // offset from unit m_bc (unlike m_soff in ActRec)
  bool destroyLocals;
};

struct CallBuiltinData : IRExtraData {
  explicit CallBuiltinData(IRSPOffset spOffset,
                           const Func* callee,
                           bool destroyLocals)
    : spOffset(spOffset)
    , callee{callee}
    , destroyLocals{destroyLocals}
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',',
      callee->fullName()->data(),
      destroyLocals ? ",destroyLocals" : ""
    );
  }

  IRSPOffset spOffset;   // offset from StkPtr to last passed arg
  const Func* callee;
  bool destroyLocals;
};

struct CallData : IRExtraData {
  explicit CallData(IRSPOffset spOffset,
                    uint32_t numParams,
                    Offset after,
                    const Func* callee,
                    bool destroy)
    : spOffset(spOffset)
    , numParams(numParams)
    , after(after)
    , callee(callee)
    , destroyLocals(destroy)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',', numParams, ',', after,
      callee
        ? folly::format(",{}", callee->fullName()->data()).str()
        : std::string{},
      destroyLocals ? ",destroyLocals" : ""
    );
  }

  IRSPOffset spOffset;    // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numParams;
  Offset after;        // m_soff style: offset from func->base()
  const Func* callee;  // nullptr if not statically known
  bool destroyLocals;
};

struct RetCtrlData : IRExtraData {
  explicit RetCtrlData(IRSPOffset spOffset, bool suspendingResumed)
    : spOffset(spOffset)
    , suspendingResumed(suspendingResumed)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset,
      suspendingResumed ? ",suspendingResumed" : ""
    );
  }

  // Adjustment we need to make to the stack pointer (for cross-tracelet ABI
  // purposes) before returning.
  IRSPOffset spOffset;

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

  size_t hash() const { return name->hash(); }
  bool equals(const LdFuncCachedData& o) const {
    return name == o.name;
  }

  const StringData* name;
};

struct LdObjMethodData : IRExtraData {
  explicit LdObjMethodData(IRSPOffset offset,
                           const StringData* method,
                           bool fatal)
    : offset(offset)
    , method(method)
    , fatal(fatal)
  {}

  std::string show() const {
    return folly::to<std::string>(offset.offset, ',', method->data(), ',',
      fatal ? "fatal" : "warn");
  }

  IRSPOffset offset;
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

  size_t hash() const {
    return hash_int64_pair(name->hash(), fallback->hash());
  }
  bool equals(const LdFuncCachedUData& o) const {
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

  explicit InterpOneData(IRSPOffset spOffset)
    : spOffset(spOffset)
    , nChangedLocals(0)
    , changedLocals(nullptr)
    , smashesAllLocals(false)
  {}

  // Delta from the StkPtr src to the top of the stack.
  IRSPOffset spOffset;

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
    auto* id = new (arena) InterpOneData(spOffset);
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
    auto ret = folly::sformat(
      "{}: spOff:{}, bcOff:{}, popped:{}, pushed:{}",
      opcodeToName(opcode),
      spOffset.offset,
      bcOff,
      cellsPopped,
      cellsPushed
    );
    assert(!smashesAllLocals || !nChangedLocals);
    if (smashesAllLocals) ret += ", smashes all locals";
    if (nChangedLocals) {
      for (auto i = 0; i < nChangedLocals; ++i) {
        ret += folly::sformat(", Local {} -> {}",
                              changedLocals[i].id,
                              changedLocals[i].type);
      }
    }

    return ret;
  }
};

struct CoerceStkData : IRExtraData {
  explicit CoerceStkData(IRSPOffset off, const Func* f, int64_t arg_num)
    : offset(off), callee(f), argNum(arg_num) {}

  std::string show() const {
    return folly::format(
      "{},{},{}",
      offset.offset,
      callee->name()->data(),
      argNum
    ).str();
  }

  IRSPOffset offset;
  const Func* callee;
  int32_t argNum;
};

struct CoerceData : IRExtraData {
  explicit CoerceData(const Func* f, int64_t arg_num)
    : callee(f), argNum(arg_num) {}

  std::string show() const {
    return folly::format(
      "{},{}",
      callee->name()->data(),
      argNum
    ).str();
  }

  const Func* callee;
  int64_t argNum;
};

struct RBEntryData : IRExtraData {
  RBEntryData(Trace::RingBufferType t, SrcKey sk)
    : type(t)
    , sk(sk)
  {}

  std::string show() const {
    return folly::sformat("{}: {}", ringbufferName(type), showShort(sk));
  }

  Trace::RingBufferType type;
  SrcKey sk;
};

struct RBMsgData : IRExtraData {
  RBMsgData(Trace::RingBufferType t, const StringData* msg)
    : type(t)
    , msg(msg)
  {
    assert(msg->isStatic());
  }

  std::string show() const {
    return folly::sformat("{}: {}", ringbufferName(type), msg->data());
  }

  Trace::RingBufferType type;
  const StringData* msg;
};

struct ClassKindData : IRExtraData {
  explicit ClassKindData(ClassKind kind): kind(uint32_t(kind)) {}

  std::string show() const {
    switch (static_cast<ClassKind>(kind)) {
      case ClassKind::Class:     return "cls";
      case ClassKind::Interface: return "interface";
      case ClassKind::Trait:     return "trait";
      case ClassKind::Enum:      return "enum";
    }
    not_reached();
  }

  uint32_t kind; // ... allows for direct usage in native_call
};

struct NewStructData : IRExtraData {
  std::string show() const;
  IRSPOffset offset;
  uint32_t numKeys;
  StringData** keys;
};

struct PackedArrayData : IRExtraData {
  explicit PackedArrayData(uint32_t size) : size(size) {}
  std::string show() const { return folly::format("{}", size).str(); }
  uint32_t size;
};

struct InitPackedArrayLoopData : IRExtraData {
  explicit InitPackedArrayLoopData(IRSPOffset offset, uint32_t size)
    : offset(offset)
    , size(size)
  {}

  std::string show() const {
    return folly::format("{},{}", offset.offset, size).str();
  }

  IRSPOffset offset;
  uint32_t size;
};

struct IndexData : IRExtraData {
  explicit IndexData(uint32_t index) : index(index) {}
  uint32_t index;
  std::string show() const { return folly::format("{}", index).str(); }
};

struct ClsNeqData : IRExtraData {
  explicit ClsNeqData(Class* testClass) : testClass(testClass) {}

  std::string show() const {
    return testClass->name()->data();
  }

  bool equals(ClsNeqData o) const { return testClass == o.testClass; }
  size_t hash() const { return std::hash<Class*>()(testClass); }

  Class* testClass; // class we're checking equality with
};

struct MInstrAttrData : IRExtraData {
  explicit MInstrAttrData(MInstrAttr mia) : mia(mia) {}
  std::string show() const {
    using U = std::underlying_type<MInstrAttr>::type;
    return folly::to<std::string>(static_cast<U>(mia));
  }
  MInstrAttr mia;
};

struct SetOpData : IRExtraData {
  explicit SetOpData(SetOpOp op) : op(op) {}
  std::string show() const { return subopToName(op); }
  SetOpOp op;
};

struct IncDecData : IRExtraData {
  explicit IncDecData(IncDecOp op) : op(op) {}
  std::string show() const { return subopToName(op); }
  IncDecOp op;
};

struct ResumeOffset : IRExtraData {
  explicit ResumeOffset(Offset off) : off(off) {}
  std::string show() const { return folly::to<std::string>(off); }
  Offset off;
};

struct GeneratorState : IRExtraData {
  explicit GeneratorState(BaseGenerator::State state) : state(state) {}
  std::string show() const {
    using U = std::underlying_type<BaseGenerator::State>::type;
    return folly::to<std::string>(static_cast<U>(state));
  }
  BaseGenerator::State state;
};

struct ContEnterData : IRExtraData {
  explicit ContEnterData(IRSPOffset spOffset, Offset returnBCOffset)
    : spOffset(spOffset)
    , returnBCOffset(returnBCOffset)
  {}

  std::string show() const {
    return folly::to<std::string>(spOffset.offset, ',', returnBCOffset);
  }

  IRSPOffset spOffset;
  Offset returnBCOffset;
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
X(HintLocInner,                 LocalId);
X(CheckLoc,                     LocalId);
X(AssertLoc,                    LocalId);
X(LdLocAddr,                    LocalId);
X(LdLoc,                        LocalId);
X(LdLocPseudoMain,              LocalId);
X(StLoc,                        LocalId);
X(StLocPseudoMain,              LocalId);
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
X(CufIterSpillFrame,            FPushCufData);
X(SpillFrame,                   ActRecInfo);
X(CheckStk,                     RelOffsetData);
X(HintStkInner,                 RelOffsetData);
X(CastStk,                      IRSPOffsetData);
X(StStk,                        IRSPOffsetData);
X(CoerceStk,                    CoerceStkData);
X(CoerceCellToInt,              CoerceData);
X(CoerceCellToDbl,              CoerceData);
X(CoerceCellToBool,             CoerceData);
X(CoerceStrToInt,               CoerceData);
X(CoerceStrToDbl,               CoerceData);
X(AssertStk,                    IRSPOffsetData);
X(ReDefSP,                      StackOffset);
X(DefSP,                        StackOffset);
X(ResetSP,                      StackOffset);
X(LdStk,                        IRSPOffsetData);
X(LdStkAddr,                    IRSPOffsetData);
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
X(LdArrFuncCtx,                 IRSPOffsetData);
X(LdArrFPushCuf,                IRSPOffsetData);
X(LdStrFPushCuf,                IRSPOffsetData);
X(LookupClsCns,                 ClsCnsName);
X(LookupClsMethod,              IRSPOffsetData);
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
X(InterpOne,                    InterpOneData);
X(InterpOneCF,                  InterpOneData);
X(StClosureFunc,                FuncData);
X(StClosureArg,                 PropByteOffset);
X(RBTraceEntry,                 RBEntryData);
X(RBTraceMsg,                   RBMsgData);
X(OODeclExists,                 ClassKindData);
X(NewStructArray,               NewStructData);
X(AllocPackedArray,             PackedArrayData);
X(InitPackedArrayLoop,          InitPackedArrayLoopData);
X(InitPackedArray,              IndexData);
X(ProfilePackedArray,           RDSHandleData);
X(ProfileStructArray,           RDSHandleData);
X(LdRDSAddr,                    RDSHandleData);
X(ClsNeq,                       ClsNeqData);
X(BaseG,                        MInstrAttrData);
X(PropX,                        MInstrAttrData);
X(PropDX,                       MInstrAttrData);
X(ElemX,                        MInstrAttrData);
X(ElemDX,                       MInstrAttrData);
X(ElemUX,                       MInstrAttrData);
X(SetOpProp,                    SetOpData);
X(IncDecProp,                   IncDecData);
X(SetOpElem,                    SetOpData);
X(IncDecElem,                   IncDecData);
X(StAsyncArResume,              ResumeOffset);
X(StContArResume,               ResumeOffset);
X(StContArState,                GeneratorState);
X(ContEnter,                    ContEnterData);
X(LdARFuncPtr,                  StackOffset);
X(EndCatch,                     IRSPOffsetData);
X(AdjustSP,                     IRSPOffsetData);
X(DbgTrashStk,                  IRSPOffsetData);
X(DbgTrashFrame,                IRSPOffsetData);
X(LdPropAddr,                   PropOffset);

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
      fprintf(stderr, "opcode = %s\n", opcodeName(opc));   \
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

size_t hashExtra(Opcode opc, const IRExtraData* data);
bool equalsExtra(Opcode opc, const IRExtraData* a, const IRExtraData* b);
IRExtraData* cloneExtra(Opcode opc, IRExtraData* data, Arena& a);
std::string showExtra(Opcode opc, const IRExtraData* data);

//////////////////////////////////////////////////////////////////////

}}

#endif
