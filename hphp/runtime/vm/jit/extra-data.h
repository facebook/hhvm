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

#ifndef incl_HPHP_VM_EXTRADATA_H_
#define incl_HPHP_VM_EXTRADATA_H_

#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/arena.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"

#include <folly/Conv.h>
#include <folly/Optional.h>

#include <algorithm>
#include <string>

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

//////////////////////////////////////////////////////////////////////

/*
 * Shared IRExtraData classes.
 *
 * These subtypes represent common parameters (e.g., statically known Func,
 * local variable ID, stack offset, etc.) that are useful for a variety of HHIR
 * instructions.
 *
 * These are kept separate from the one-off IRExtraDatas to make it easier to
 * find existing common parameters.
 */

/*
 * Offset in bytes from a base pointer---e.g., to a object property from an
 * ObjectData*.
 */
struct ByteOffsetData : IRExtraData {
  explicit ByteOffsetData(ptrdiff_t offset) : offsetBytes(offset) {}

  std::string show() const { return folly::to<std::string>(offsetBytes); }

  bool equals(ByteOffsetData o) const { return offsetBytes == o.offsetBytes; }
  size_t hash() const { return std::hash<ptrdiff_t>()(offsetBytes); }

  ptrdiff_t offsetBytes;
};

/*
 * Class pointer.
 *
 * Required to be non-null.
 */
struct ClassData : IRExtraData {
  explicit ClassData(const Class* cls)
    : cls(cls)
  {
    assert(cls != nullptr);
  }

  std::string show() const {
    return folly::to<std::string>(cls->name()->data());
  }

  bool equals(const ClassData& o) const {
    return cls == o.cls;
  }

  size_t hash() const {
    return hash_int64(reinterpret_cast<intptr_t>(cls));
  }

  const Class* cls;
};

/*
 * ExtendsClass.
 */
struct ExtendsClassData : IRExtraData {
  explicit ExtendsClassData(const Class* cls, bool strictLikely = false)
      : cls(cls), strictLikely(strictLikely)
  {
    assert(cls != nullptr);
  }

  std::string show() const {
    return folly::sformat("{}{}",
                          cls->name(), strictLikely ? ":strictLikely" : "");
  }

  bool equals(const ExtendsClassData& o) const {
    return cls == o.cls && strictLikely == o.strictLikely;
  }

  size_t hash() const {
    return pointer_hash<Class>()(cls);
  }

  const Class* cls;
  bool strictLikely;
};

/*
 * Class with method name.
 */
struct ClsMethodData : IRExtraData {
  ClsMethodData(const StringData* cls, const StringData* method,
                const NamedEntity* ne = nullptr)
    : clsName(cls)
    , methodName(method)
    , namedEntity(ne)
  {}

  std::string show() const {
    return folly::format("{}::{}", clsName, methodName).str();
  }

  bool equals(const ClsMethodData& o) const {
    // The strings are static so we can use pointer equality.
    return clsName == o.clsName && methodName == o.methodName;
  }
  size_t hash() const {
    return hash_int64_pair((intptr_t)clsName, (intptr_t)methodName);
  }

  const StringData* clsName;
  const StringData* methodName;
  const NamedEntity* namedEntity;
};

struct IfaceMethodData : IRExtraData {
  IfaceMethodData(Slot vtableIdx, Slot methodIdx)
    : vtableIdx(vtableIdx)
    , methodIdx(methodIdx)
  {}

  std::string show() const {
    return folly::sformat("{}, {}", vtableIdx, methodIdx);
  }

  bool equals(const IfaceMethodData& o) const {
    return vtableIdx == o.vtableIdx && methodIdx == o.methodIdx;
  }

  size_t hash() const {
    return hash_int64((int64_t)vtableIdx << 32 | methodIdx);
  }

  Slot vtableIdx;
  Slot methodIdx;
};

/*
 * Func with argument index.
 */
struct FuncArgData : IRExtraData {
  explicit FuncArgData(const Func* f, int64_t arg)
    : func(f)
    , argNum(arg)
  {}

  std::string show() const {
    return folly::format("{},{}", func->name(), argNum).str();
  }

  const Func* func;
  int64_t argNum;
};

/*
 * Local variable ID.
 */
struct LocalId : IRExtraData {
  explicit LocalId(uint32_t id) : locId(id) {}

  std::string show() const { return folly::to<std::string>(locId); }

  bool equals(LocalId o) const { return locId == o.locId; }
  size_t hash() const { return std::hash<uint32_t>()(locId); }

  uint32_t locId;
};

struct ClsRefSlotData : IRExtraData {
  explicit ClsRefSlotData(uint32_t slot) : slot{slot} {}

  std::string show() const { return folly::to<std::string>(slot); }

  bool equals(ClsRefSlotData o) const { return slot == o.slot; }
  size_t hash() const { return std::hash<uint32_t>()(slot); }

  uint32_t slot;
};

/*
 * Index into, e.g., an array.
 */
struct IndexData : IRExtraData {
  explicit IndexData(uint32_t index) : index(index) {}

  std::string show() const { return folly::format("{}", index).str(); }

  uint32_t index;
};

/*
 * Iterator ID.
 */
struct IterId : IRExtraData {
  explicit IterId(uint32_t id)
    : iterId(id)
  {}

  std::string show() const { return folly::to<std::string>(iterId); }

  bool equals(IterId o) const { return iterId == o.iterId; }
  size_t hash() const { return std::hash<uint32_t>()(iterId); }

  uint32_t iterId;
};

/*
 * Iter instruction data.
 *
 * `iterId' is the iterator ID; `keyId' and `valId' are the IDs of the iterator
 * locals $key => $value.  For keyless iterators, we still use this class, with
 * `keyId` set to -1u.
 */
struct IterData : IRExtraData {
  explicit IterData(uint32_t iter, uint32_t key, uint32_t val)
    : iterId(iter)
    , keyId(key)
    , valId(val)
  {}

  std::string show() const {
    if (keyId == -1) return folly::format("{}::{}", iterId, valId).str();
    return folly::format("{}::{}::{}", iterId, keyId, valId).str();
  }

  uint32_t iterId;
  uint32_t keyId;
  uint32_t valId;
};

/*
 * RDS handle.
 */
struct RDSHandleData : IRExtraData {
  explicit RDSHandleData(rds::Handle handle) : handle(handle) {}

  std::string show() const { return folly::to<std::string>(handle); }

  bool equals(RDSHandleData o) const { return handle == o.handle; }
  size_t hash() const { return std::hash<uint32_t>()(handle); }

  rds::Handle handle;
};

/*
 * Translation ID.
 *
 * Used with profiling-related instructions.
 */
struct TransIDData : IRExtraData {
  explicit TransIDData(TransID transId) : transId(transId) {}

  std::string show() const { return folly::to<std::string>(transId); }

  TransID transId;
};

/*
 * FP-relative offset.
 */
struct FPRelOffsetData : IRExtraData {
  explicit FPRelOffsetData(FPRelOffset offset) : offset(offset) {}

  std::string show() const {
    return folly::to<std::string>("FPRelOff ", offset.offset);
  }

  bool equals(FPRelOffsetData o) const { return offset == o.offset; }
  size_t hash() const { return std::hash<int32_t>()(offset.offset); }

  FPRelOffset offset;
};

/*
 * Stack pointer offset.
 */
struct FPInvOffsetData : IRExtraData {
  explicit FPInvOffsetData(FPInvOffset offset) : offset(offset) {}

  std::string show() const {
    return folly::to<std::string>("FPInvOff ", offset.offset);
  }

  bool equals(FPInvOffsetData o) const { return offset == o.offset; }
  size_t hash() const { return std::hash<int32_t>()(offset.offset); }

  FPInvOffset offset;
};

/*
 * Stack offset.
 */
struct IRSPRelOffsetData : IRExtraData {
  explicit IRSPRelOffsetData(IRSPRelOffset offset) : offset(offset) {}

  std::string show() const {
    return folly::to<std::string>("IRSPOff ", offset.offset);
  }

  bool equals(IRSPRelOffsetData o) const { return offset == o.offset; }
  size_t hash() const { return std::hash<int32_t>()(offset.offset); }

  IRSPRelOffset offset;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * One-off IRExtraData classes.
 *
 * These are used for only one or two instructions and are in no particular
 * order.  Add new IRExtraData types here.
 */

struct IsAsyncData : IRExtraData {
  explicit IsAsyncData(bool isAsync) : isAsync(isAsync) {}

  std::string show() const { return folly::to<std::string>(isAsync); }
  bool equals(IsAsyncData d) const { return isAsync == d.isAsync; }
  size_t hash() const { return std::hash<int32_t>()(isAsync); }

  bool isAsync;
};

struct LdBindAddrData : IRExtraData {
  explicit LdBindAddrData(SrcKey sk, FPInvOffset bcSPOff)
    : sk(sk)
    , bcSPOff(bcSPOff)
  {}

  std::string show() const { return showShort(sk); }

  SrcKey sk;
  FPInvOffset bcSPOff;
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
    target->bcSPOff    = bcSPOff;
    std::copy(cases, cases + numCases, const_cast<Elm*>(target->cases));
    return target;
  }

  std::string show() const {
    return folly::to<std::string>(bcSPOff.offset);
  }

  int64_t     numCases;
  const Elm*  cases;
  SrcKey      defaultSk;
  FPInvOffset bcSPOff;
};

struct ProfileSwitchData : IRExtraData {
  ProfileSwitchData(rds::Handle handle, int32_t cases, int64_t base)
    : handle(handle)
    , cases(cases)
    , base(base)
  {}

  std::string show() const {
    return folly::sformat("handle {}, {} cases, base {}", handle, cases, base);
  }

  rds::Handle handle;
  int32_t cases;
  int64_t base;
};

struct JmpSwitchData : IRExtraData {
  JmpSwitchData* clone(Arena& arena) const {
    JmpSwitchData* sd = new (arena) JmpSwitchData;
    sd->cases = cases;
    sd->targets = new (arena) SrcKey[cases];
    sd->spOffBCFromFP = spOffBCFromFP;
    sd->spOffBCFromIRSP = spOffBCFromIRSP;
    std::copy(targets, targets + cases, const_cast<SrcKey*>(sd->targets));
    return sd;
  }

  std::string show() const {
    return folly::sformat("{} cases", cases);
  }

  int32_t cases;       // number of cases
  SrcKey* targets;     // srckeys for all targets
  FPInvOffset spOffBCFromFP;
  IRSPRelOffset spOffBCFromIRSP;
};

struct LdTVAuxData : IRExtraData {
  explicit LdTVAuxData(int32_t v = -1) : valid(v) {}

  std::string show() const {
    return folly::sformat("{:x}", valid);
  }

  int32_t valid;
};

struct ReqBindJmpData : IRExtraData {
  explicit ReqBindJmpData(const SrcKey& target,
                          FPInvOffset invSPOff,
                          IRSPRelOffset irSPOff,
                          TransFlags trflags)
    : target(target)
    , invSPOff(invSPOff)
    , irSPOff(irSPOff)
    , trflags(trflags)
  {}

  std::string show() const {
    return folly::sformat(
      "{}, FPInv {}, IRSP {}, Flags {}",
      target.offset(), invSPOff.offset, irSPOff.offset, trflags.packed
    );
  }

  SrcKey target;
  FPInvOffset invSPOff;
  IRSPRelOffset irSPOff;
  TransFlags trflags;
};

struct ReqRetranslateData : IRExtraData {
  explicit ReqRetranslateData(IRSPRelOffset irSPOff,
                              TransFlags trflags)
    : irSPOff(irSPOff)
    , trflags{trflags}
  {}

  std::string show() const {
    return folly::to<std::string>(irSPOff.offset, ',', trflags.packed);
  }

  IRSPRelOffset irSPOff;
  TransFlags trflags;
};

/*
 * Compile-time metadata about an ActRec allocation.
 */
struct ActRecInfo : IRExtraData {
  IRSPRelOffset spOffset;
  int32_t numArgs;

  std::string show() const {
    return folly::to<std::string>(spOffset.offset, ',', numArgs);
  }
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
      retBCOff, ',',
      retSPOff.offset, ',',
      spOffset.offset
    );
  }

  const Func* target;
  SSATmp* ctx;  // Ctx, Cls or Nullptr.
  Offset retBCOff;
  FPInvOffset retSPOff;
  IRSPRelOffset spOffset; // offset from caller SP to bottom of callee's ActRec
  uint32_t numNonDefault;
};

struct SyncReturnBCData : IRExtraData {
  SyncReturnBCData(Offset bcOff, IRSPRelOffset spOff)
    : bcOffset(bcOff)
    , spOffset(spOff)
  {}
  std::string show() const {
    return folly::to<std::string>(bcOffset, ",", spOffset.offset);
  }

  Offset bcOffset;
  IRSPRelOffset spOffset;
};

struct CallArrayData : IRExtraData {
  explicit CallArrayData(IRSPRelOffset spOffset,
                         uint32_t numParams,
                         Offset pcOffset,
                         Offset after,
                         const Func* callee,
                         bool writeLocals,
                         bool readLocals)
    : spOffset(spOffset)
    , numParams(numParams)
    , pc(pcOffset)
    , after(after)
    , callee(callee)
    , writeLocals(writeLocals)
    , readLocals(readLocals)
  {}

  std::string show() const {
    return folly::to<std::string>(
      pc, ",",
      after,
      callee
        ? folly::sformat(",{}", callee->fullName())
        : std::string{},
      writeLocals ? ",writeLocals" : "",
      readLocals ? ",readLocals" : "");
  }

  IRSPRelOffset spOffset; // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numParams;
  Offset pc;     // XXX why isn't this available in the marker?
  Offset after;  // offset from unit m_bc (unlike m_soff in ActRec)
  const Func* callee; // nullptr if not statically known
  bool writeLocals;
  bool readLocals;
};

struct CallBuiltinData : IRExtraData {
  explicit CallBuiltinData(IRSPRelOffset spOffset,
                           const Func* callee,
                           int32_t numNonDefault,
                           bool writeLocals,
                           bool readLocals,
                           bool needsFrame)
    : spOffset(spOffset)
    , callee{callee}
    , numNonDefault{numNonDefault}
    , writeLocals{writeLocals}
    , readLocals{readLocals}
    , needsCallerFrame{needsFrame}
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',',
      callee->fullName()->data(),
      writeLocals ? ",writeLocals" : "",
      readLocals ? ",readLocals" : "",
      needsCallerFrame ? ",needsCallerFrame" : ""
    );
  }

  IRSPRelOffset spOffset; // offset from StkPtr to last passed arg
  const Func* callee;
  int32_t numNonDefault;
  bool writeLocals;
  bool readLocals;
  bool needsCallerFrame;
};

struct CallData : IRExtraData {
  explicit CallData(IRSPRelOffset spOffset,
                    uint32_t numParams,
                    Offset after,
                    const Func* callee,
                    bool writeLocals,
                    bool readLocals,
                    bool needsFrame,
                    bool fcallAwait)
    : spOffset(spOffset)
    , numParams(numParams)
    , after(after)
    , callee(callee)
    , writeLocals(writeLocals)
    , readLocals(readLocals)
    , needsCallerFrame(needsFrame)
    , fcallAwait(fcallAwait)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',', numParams, ',', after,
      callee
        ? folly::format(",{}", callee->fullName()).str()
        : std::string{},
      writeLocals ? ",writeLocals" : "",
      readLocals ? ",readLocals" : "",
      needsCallerFrame ? ",needsCallerFrame" : "",
      fcallAwait ? ",fcallAwait" : ""
    );
  }

  IRSPRelOffset spOffset; // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numParams;
  Offset after;        // m_soff style: offset from func->base()
  const Func* callee;  // nullptr if not statically known
  bool writeLocals;
  bool readLocals;
  bool needsCallerFrame;
  bool fcallAwait;
};

struct RetCtrlData : IRExtraData {
  explicit RetCtrlData(IRSPRelOffset spOffset, bool suspendingResumed,
                       folly::Optional<AuxUnion> aux = folly::none)
    : spOffset(spOffset)
    , suspendingResumed(suspendingResumed)
    , aux(aux)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset,
      suspendingResumed ? ",suspendingResumed" : ""
    );
  }

  // Adjustment we need to make to the stack pointer (for cross-tracelet ABI
  // purposes) before returning.
  IRSPRelOffset spOffset;

  // Indicates that the current resumable frame is being suspended without
  // decrefing locals.  Used by refcount optimizer.
  bool suspendingResumed;

  // Optional TV aux value to attach to the function's return value.
  folly::Optional<AuxUnion> aux;
};

/*
 * Name of a class constant in a known class
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
 * Name of a class constant in an unknown class.
 */
struct LdSubClsCnsData : IRExtraData {
  explicit LdSubClsCnsData(const StringData* cns, Slot s)
    : cnsName(cns)
    , slot(s)
  {}

  std::string show() const {
    return folly::sformat("<cls>::{}({})", cnsName, slot);
  }

  const StringData* cnsName;
  Slot slot;
};

/*
 * Name and handle of profiled class constant
 */
struct ProfileSubClsCnsData : IRExtraData {
  explicit ProfileSubClsCnsData(const StringData* cns, rds::Handle h)
    : cnsName(cns)
    , handle(h)
  {}

  std::string show() const {
    return folly::to<std::string>("<cls>::", cnsName->data());
  }

  const StringData* cnsName;
  rds::Handle handle;
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

  bool equals(const StaticLocName& o) const {
    return func == o.func && name == o.name;
  }
  size_t hash() const {
    return hash_int64_pair((intptr_t)func, (intptr_t)name);
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
  explicit LdObjMethodData(IRSPRelOffset offset,
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

  IRSPRelOffset offset;
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
    return hash_int64_pair((uint32_t)name->hash(), (uint32_t)fallback->hash());
  }
  bool equals(const LdFuncCachedUData& o) const {
    return name == o.name && fallback == o.fallback;
  }

  const StringData* name;
  const StringData* fallback;
};

/*
 * Offset and stack deltas for InterpOne.
 */
struct InterpOneData : IRExtraData {
  struct LocalType {
    explicit LocalType(uint32_t id = 0, Type type = TBottom)
      : id(id)
      , type(type)
    {}

    uint32_t id;
    Type type;
  };

  struct ClsRefSlot {
    explicit ClsRefSlot(uint32_t id = 0, bool write = false)
      : id{id}
      , write{write}
    {}

    uint32_t id;
    bool write;
  };

  explicit InterpOneData(IRSPRelOffset spOffset)
    : spOffset(spOffset)
    , nChangedLocals(0)
    , changedLocals(nullptr)
    , nChangedClsRefSlots(0)
    , changedClsRefSlots(nullptr)
    , smashesAllLocals(false)
  {}

  // Offset of the BC stack top relative to the current IR stack pointer.
  IRSPRelOffset spOffset;

  // Offset of the instruction to interpret, in the Unit indicated by the
  // current Marker.
  Offset bcOff;

  // The number of eval stack cells consumed and produced by the instruction,
  // respectively.  Includes ActRecs.
  int64_t cellsPopped;
  int64_t cellsPushed;

  // Opcode, in case we need to fix the stack differently.  Some bytecode
  // instructions modify things below the top of the stack.
  Op opcode;

  uint32_t nChangedLocals;
  LocalType* changedLocals;

  uint32_t nChangedClsRefSlots;
  ClsRefSlot* changedClsRefSlots;

  bool smashesAllLocals;

  InterpOneData* clone(Arena& arena) const {
    auto* id = new (arena) InterpOneData(spOffset);
    id->bcOff = bcOff;
    id->cellsPopped = cellsPopped;
    id->cellsPushed = cellsPushed;
    id->opcode = opcode;
    id->nChangedLocals = nChangedLocals;
    id->changedLocals = new (arena) LocalType[nChangedLocals];
    id->nChangedClsRefSlots = nChangedClsRefSlots;
    id->changedClsRefSlots = new (arena) ClsRefSlot[nChangedClsRefSlots];
    id->smashesAllLocals = smashesAllLocals;
    std::copy(changedLocals, changedLocals + nChangedLocals, id->changedLocals);
    std::copy(changedClsRefSlots, changedClsRefSlots + nChangedClsRefSlots,
              id->changedClsRefSlots);
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
    assertx(!smashesAllLocals || !nChangedLocals);
    if (smashesAllLocals) ret += ", smashes all locals";
    if (nChangedLocals) {
      for (auto i = 0; i < nChangedLocals; ++i) {
        ret += folly::sformat(", Local {} -> {}",
                              changedLocals[i].id,
                              changedLocals[i].type);
      }
    }
    if (nChangedClsRefSlots) {
      for (auto i = 0; i < nChangedClsRefSlots; ++i) {
        ret += folly::sformat(", Slot {}{}",
                              changedClsRefSlots[i].id,
                              changedClsRefSlots[i].write ? "W" : "R");
      }
    }

    return ret;
  }
};

struct CoerceStkData : IRExtraData {
  explicit CoerceStkData(IRSPRelOffset off, const Func* f, int64_t arg_num)
    : offset(off), callee(f), argNum(arg_num) {}

  std::string show() const {
    return folly::sformat(
      "IRSP {},{},{}",
      offset.offset,
      callee->name(),
      argNum
    );
  }

  IRSPRelOffset offset;
  const Func* callee;
  int32_t argNum;
};

struct CoerceMemData : IRExtraData {
  explicit CoerceMemData(const Func* f, int64_t arg_num)
    : callee(f), argNum(arg_num) {}

  std::string show() const {
    return folly::sformat("{},{}", callee->name(), argNum);
  }

  const Func* callee;
  int32_t argNum;
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
    assertx(msg->isStatic());
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
  IRSPRelOffset offset;
  uint32_t numKeys;
  StringData** keys;
};

struct PackedArrayData : IRExtraData {
  explicit PackedArrayData(uint32_t size) : size(size) {}
  std::string show() const { return folly::format("{}", size).str(); }
  uint32_t size;
};

struct InitPackedArrayLoopData : IRExtraData {
  explicit InitPackedArrayLoopData(IRSPRelOffset offset, uint32_t size)
    : offset(offset)
    , size(size)
  {}

  std::string show() const {
    return folly::format("{},{}", offset.offset, size).str();
  }

  IRSPRelOffset offset;
  uint32_t size;
};

struct NewKeysetArrayData : IRExtraData {
  explicit NewKeysetArrayData(IRSPRelOffset offset, uint32_t size)
    : offset(offset)
    , size(size)
  {}

  std::string show() const {
    return folly::format("{},{}", offset.offset, size).str();
  }

  IRSPRelOffset offset;
  uint32_t size;
};

struct MemoData : IRExtraData {
  explicit MemoData(LocalRange locals)
    : locals(locals) {}

  std::string show() const { return HPHP::show(locals); }

  LocalRange locals;
};

struct MOpModeData : IRExtraData {
  explicit MOpModeData(MOpMode mode) : mode{mode} {}

  std::string show() const { return subopToName(mode); }

  MOpMode mode;
};

struct SetOpData : IRExtraData {
  explicit SetOpData(SetOpOp op) : op(op) {}
  std::string show() const { return subopToName(op); }
  SetOpOp op;
};

struct DecRefData : IRExtraData {
  explicit DecRefData(int locId = -1) : locId(locId) {}
  std::string show() const {
    return locId != -1 ? folly::to<std::string>("Loc", locId) : "-";
  }
  int locId; // If a known local, this has its id; -1 otherwise.
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
  explicit ContEnterData(IRSPRelOffset spOffset, Offset returnBCOffset,
                         bool isAsync)
    : spOffset(spOffset)
    , returnBCOffset(returnBCOffset)
    , isAsync(isAsync)
  {}

  std::string show() const {
    return folly::to<std::string>(spOffset.offset, ',', returnBCOffset,
                                  isAsync ? ",async" : "");
  }

  IRSPRelOffset spOffset;
  Offset returnBCOffset;
  bool isAsync;
};

struct NewColData : IRExtraData {
  explicit NewColData(CollectionType itype)
    : type(itype)
  {}

  std::string show() const {
    return collections::typeToString(type)->toCppString();
  }

  CollectionType type;
};

struct LocalIdRange : IRExtraData {
  LocalIdRange(uint32_t start, uint32_t end)
    : start(start)
    , end(end)
  {}

  std::string show() const {
    return folly::format("[{}, {})", start, end).str();
  }

  uint32_t start, end;
};

struct FuncEntryData : IRExtraData {
  FuncEntryData(const Func* func, uint32_t argc)
    : func(func)
    , argc(argc)
  {}

  std::string show() const {
    return folly::format(
      "{}({} args)",
      func->fullName(),
      argc
    ).str();
  }

  const Func* func;
  uint32_t argc;
};

struct CheckRefsData : IRExtraData {
  CheckRefsData(unsigned firstBit, uint64_t mask, uint64_t vals)
    : firstBit(safe_cast<int>(firstBit))
    , mask(mask)
    , vals(vals)
  {}

  std::string show() const {
    return folly::format("{},{},{}", firstBit, mask, vals).str();
  }

  int firstBit;
  uint64_t mask;
  uint64_t vals;
};

struct LookupClsMethodData : IRExtraData {
  explicit LookupClsMethodData(IRSPRelOffset offset, bool forward) :
    calleeAROffset(offset), forward(forward) {}

  std::string show() const {
    return folly::to<std::string>("IRSPOff ", calleeAROffset.offset,
                                  forward ? " forwarded" : "");
  }

  // offset from caller SP to bottom of callee's ActRec
  IRSPRelOffset calleeAROffset;
  bool forward;
};


struct ProfileMethodData : IRExtraData {
  ProfileMethodData(IRSPRelOffset bcSPOff, rds::Handle handle)
    : bcSPOff(bcSPOff)
    , handle(handle)
  {}

  std::string show() const { return folly::to<std::string>(handle); }

  IRSPRelOffset bcSPOff;
  rds::Handle handle;
};

struct FuncGuardData : IRExtraData {
  FuncGuardData(const Func* func, TCA* prologueAddrPtr)
    : func(func)
    , prologueAddrPtr(prologueAddrPtr)
  {}

  std::string show() const {
    return folly::sformat("{}=>{}",
      func->fullName(), prologueAddrPtr
    );
  }

  const Func* func;
  TCA* prologueAddrPtr;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

X(LdBindAddr,                   LdBindAddrData);
X(ProfileSwitchDest,            ProfileSwitchData);
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
X(StLocRange,                   LocalIdRange);
X(LdClsRef,                     ClsRefSlotData);
X(StClsRef,                     ClsRefSlotData);
X(KillClsRef,                   ClsRefSlotData);
X(IterFree,                     IterId);
X(MIterFree,                    IterId);
X(DecodeCufIter,                IterId);
X(StCufIterFunc,                IterId);
X(StCufIterCtx,                 IterId);
X(StCufIterInvName,             IterId);
X(LdCufIterFunc,                IterId);
X(LdCufIterCtx,                 IterId);
X(LdCufIterInvName,             IterId);
X(KillCufIter,                  IterId);
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
X(InstanceOfIfaceVtable,        ClassData);
X(ExtendsClass,                 ExtendsClassData);
X(SpillFrame,                   ActRecInfo);
X(CheckStk,                     IRSPRelOffsetData);
X(HintStkInner,                 IRSPRelOffsetData);
X(StStk,                        IRSPRelOffsetData);
X(CastStk,                      IRSPRelOffsetData);
X(CoerceStk,                    CoerceStkData);
X(CoerceMem,                    CoerceMemData);
X(CoerceCellToInt,              FuncArgData);
X(CoerceCellToDbl,              FuncArgData);
X(CoerceCellToBool,             FuncArgData);
X(CoerceStrToInt,               FuncArgData);
X(CoerceStrToDbl,               FuncArgData);
X(AssertStk,                    IRSPRelOffsetData);
X(DefSP,                        FPInvOffsetData);
X(LdStk,                        IRSPRelOffsetData);
X(LdStkAddr,                    IRSPRelOffsetData);
X(DefInlineFP,                  DefInlineFPData);
X(BeginInlining,                IRSPRelOffsetData);
X(SyncReturnBC,                 SyncReturnBCData);
X(InlineReturn,                 FPRelOffsetData);
X(InlineReturnNoFrame,          FPRelOffsetData);
X(ReqRetranslate,               ReqRetranslateData);
X(ReqBindJmp,                   ReqBindJmpData);
X(ReqRetranslateOpt,            IRSPRelOffsetData);
X(CheckCold,                    TransIDData);
X(IncProfCounter,               TransIDData);
X(Call,                         CallData);
X(CallBuiltin,                  CallBuiltinData);
X(CallArray,                    CallArrayData);
X(RetCtrl,                      RetCtrlData);
X(AsyncRetCtrl,                 RetCtrlData);
X(AsyncRetFast,                 RetCtrlData);
X(AsyncSwitchFast,              RetCtrlData);
X(LdArrFuncCtx,                 IRSPRelOffsetData);
X(LdArrFPushCuf,                IRSPRelOffsetData);
X(LdStrFPushCuf,                IRSPRelOffsetData);
X(LdFunc,                       IRSPRelOffsetData);
X(LookupClsMethod,              LookupClsMethodData);
X(LookupClsMethodCache,         ClsMethodData);
X(LdClsMethodCacheFunc,         ClsMethodData);
X(LdClsMethodCacheCls,          ClsMethodData);
X(LdClsMethodFCacheFunc,        ClsMethodData);
X(LookupClsMethodFCache,        ClsMethodData);
X(LdIfaceMethod,                IfaceMethodData);
X(LdClosureStaticLoc,           StaticLocName);
X(LdStaticLoc,                  StaticLocName);
X(CheckStaticLoc,               StaticLocName);
X(InitStaticLoc,                StaticLocName);
X(LdClsCns,                     ClsCnsName);
X(InitClsCns,                   ClsCnsName);
X(LdSubClsCns,                  LdSubClsCnsData);
X(CheckSubClsCns,               LdSubClsCnsData);
X(ProfileSubClsCns,             ProfileSubClsCnsData);
X(LdFuncCached,                 LdFuncCachedData);
X(LdFuncCachedSafe,             LdFuncCachedData);
X(LdFuncCachedU,                LdFuncCachedUData);
X(LdObjMethod,                  LdObjMethodData);
X(RaiseMissingArg,              FuncArgData);
X(InterpOne,                    InterpOneData);
X(InterpOneCF,                  InterpOneData);
X(StClosureArg,                 ByteOffsetData);
X(RBTraceEntry,                 RBEntryData);
X(RBTraceMsg,                   RBMsgData);
X(OODeclExists,                 ClassKindData);
X(NewStructArray,               NewStructData);
X(AllocPackedArray,             PackedArrayData);
X(AllocVecArray,                PackedArrayData);
X(NewKeysetArray,               NewKeysetArrayData);
X(InitPackedLayoutArrayLoop,    InitPackedArrayLoopData);
X(InitPackedLayoutArray,        IndexData);
X(CheckMixedArrayOffset,        IndexData);
X(CheckDictOffset,              IndexData);
X(CheckKeysetOffset,            IndexData);
X(ElemMixedArrayK,              IndexData);
X(MixedArrayGetK,               IndexData);
X(DictGetK,                     IndexData);
X(KeysetGetK,                   IndexData);
X(ElemDictK,                    IndexData);
X(ElemKeysetK,                  IndexData);
X(ProfileArrayKind,             RDSHandleData);
X(ProfileMixedArrayOffset,      RDSHandleData);
X(ProfileDictOffset,            RDSHandleData);
X(ProfileKeysetOffset,          RDSHandleData);
X(ProfileType,                  RDSHandleData);
X(ProfileMethod,                ProfileMethodData);
X(LdRDSAddr,                    RDSHandleData);
X(BaseG,                        MOpModeData);
X(PropX,                        MOpModeData);
X(PropDX,                       MOpModeData);
X(ElemX,                        MOpModeData);
X(ElemDX,                       MOpModeData);
X(ElemUX,                       MOpModeData);
X(CGetProp,                     MOpModeData);
X(CGetElem,                     MOpModeData);
X(MemoGet,                      MemoData);
X(MemoSet,                      MemoData);
X(SetOpProp,                    SetOpData);
X(SetOpCell,                    SetOpData);
X(IncDecProp,                   IncDecData);
X(SetOpElem,                    SetOpData);
X(IncDecElem,                   IncDecData);
X(StAsyncArResume,              ResumeOffset);
X(StContArResume,               ResumeOffset);
X(StContArState,                GeneratorState);
X(ContEnter,                    ContEnterData);
X(DbgAssertARFunc,              IRSPRelOffsetData);
X(LdARFuncPtr,                  IRSPRelOffsetData);
X(LdARCtx,                      IRSPRelOffsetData);
X(EndCatch,                     IRSPRelOffsetData);
X(EagerSyncVMRegs,              IRSPRelOffsetData);
X(JmpSSwitchDest,               IRSPRelOffsetData);
X(DbgTrashStk,                  IRSPRelOffsetData);
X(DbgTrashFrame,                IRSPRelOffsetData);
X(DbgTraceCall,                 IRSPRelOffsetData);
X(LdPropAddr,                   ByteOffsetData);
X(NewCol,                       NewColData);
X(NewColFromArray,              NewColData);
X(InitExtraArgs,                FuncEntryData);
X(CheckSurpriseFlagsEnter,      FuncEntryData);
X(CheckSurpriseAndStack,        FuncEntryData);
X(ContPreNext,                  IsAsyncData);
X(ContStartedCheck,             IsAsyncData);
X(ContValid,                    IsAsyncData);
X(LdContResumeAddr,             IsAsyncData);
X(LdContActRec,                 IsAsyncData);
X(DecRef,                       DecRefData);
X(LdTVAux,                      LdTVAuxData);
X(CheckRefs,                    CheckRefsData);
X(FuncGuard,                    FuncGuardData);

#undef X

//////////////////////////////////////////////////////////////////////

template<bool hasExtra, Opcode opc, class T> struct AssertExtraTypes {
  static void doassertx() {
    assertx(!"called extra on an opcode without extra data");
  }
  static void doassert_same() {
    assertx(!"called extra on an opcode without extra data");
  }
};

template<Opcode opc, class T> struct AssertExtraTypes<true,opc,T> {
  typedef typename IRExtraDataType<opc>::type ExtraType;

  static void doassertx() {
    if (!std::is_base_of<T,ExtraType>::value) {
      assertx(!"extra<T> was called with an extra data "
              "type that doesn't match the opcode type");
    }
  }
  static void doassert_same() {
    if (!std::is_same<T,ExtraType>::value) {
      fprintf(stderr, "opcode = %s\n", opcodeName(opc));   \
      assertx(!"extra<T> was called with an extra data type that "
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
    >::doassertx();                              \
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
