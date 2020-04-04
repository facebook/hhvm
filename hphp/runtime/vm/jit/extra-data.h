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

#pragma once

#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/iter.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/arena.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"

#include <folly/Conv.h>
#include <folly/Hash.h>
#include <folly/Optional.h>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>

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
 * Class pointer.
 *
 * Required to be non-null.
 */
struct ClassData : IRExtraData {
  explicit ClassData(const Class* cls)
    : cls(cls)
  {
    assertx(cls != nullptr);
  }

  std::string show() const {
    return folly::to<std::string>(cls->name()->data());
  }

  bool equals(const ClassData& o) const {
    return cls == o.cls;
  }

  size_t hash() const {
    return pointer_hash<Class>()(cls);
  }

  const Class* cls;
};

struct OptClassData : IRExtraData {
  explicit OptClassData(const Class* cls)
    : cls(cls)
  {}

  std::string show() const {
    return folly::to<std::string>(cls ? cls->name()->data() : "{null}");
  }

  bool equals(const OptClassData& o) const {
    return cls == o.cls;
  }

  size_t hash() const {
    return pointer_hash<Class>()(cls);
  }

  const Class* cls;
};

/*
 * Class pointer, suppress flag, is or as operation flag and range into the
 * stack (for type structures) needed for resolve type struct instruction
 *
 * Class pointer could be null.
 */
struct ResolveTypeStructData : IRExtraData {
  explicit ResolveTypeStructData(
    const Class* cls,
    bool suppress,
    IRSPRelOffset offset,
    uint32_t size,
    bool isOrAsOp
  )
    : cls(cls)
    , suppress(suppress)
    , offset(offset)
    , size(size)
    , isOrAsOp(isOrAsOp)
  {}

  std::string show() const {
    return folly::sformat("{},{},{},{},{}",
                          cls ? cls->name()->data() : "nullptr",
                          suppress ? "suppress" : "no-suppress",
                          offset.offset,
                          size,
                          isOrAsOp);
  }

  bool equals(const ResolveTypeStructData& o) const {
    return cls == o.cls && suppress == o.suppress &&
           offset == o.offset && size == o.size && isOrAsOp == o.isOrAsOp;
  }

  size_t hash() const {
    return (pointer_hash<Class>()(cls)
            + std::hash<int32_t>()(offset.offset)
            + std::hash<uint32_t>()(size))
           ^ ((int64_t)(suppress ? -1 : 0) << 32 | (isOrAsOp ? -1 : 0));
  }

  const Class* cls;
  bool suppress;
  IRSPRelOffset offset;
  uint32_t size;
  bool isOrAsOp;
};

/*
 * ExtendsClass.
 */
struct ExtendsClassData : IRExtraData {
  explicit ExtendsClassData(const Class* cls, bool strictLikely = false)
      : cls(cls), strictLikely(strictLikely)
  {
    assertx(cls != nullptr);
  }

  std::string show() const {
    return folly::sformat("{}{}",
                          cls->name(), strictLikely ? ":strictLikely" : "");
  }

  bool equals(const ExtendsClassData& o) const {
    return cls == o.cls && strictLikely == o.strictLikely;
  }

  size_t hash() const {
    return pointer_hash<Class>()(cls) ^ (strictLikely ? -1 : 0);
  }

  const Class* cls;
  bool strictLikely;
};

/*
 * InstanceOfIfaceVtable.
 */
struct InstanceOfIfaceVtableData : IRExtraData {
  InstanceOfIfaceVtableData(const Class* cls, bool canOptimize)
      : cls(cls), canOptimize(canOptimize)
  {
    assertx(cls != nullptr);
  }

  std::string show() const {
    return folly::sformat("{}{}",
                          cls->name(), canOptimize ? ":canOptimize" : "");
  }

  bool equals(const InstanceOfIfaceVtableData& o) const {
    return cls == o.cls && canOptimize == o.canOptimize;
  }

  size_t hash() const {
    return pointer_hash<Class>()(cls) ^ (canOptimize ? -1 : 0);
  }

  const Class* cls;
  bool canOptimize;
};

/*
 * Class with method name.
 */
struct ClsMethodData : IRExtraData {
  ClsMethodData(const StringData* cls, const StringData* method,
                const NamedEntity* ne, const Class* context)
    : clsName(cls)
    , methodName(method)
    , namedEntity(ne)
    , context(context)
  {}

  std::string show() const {
    return folly::sformat(
      "{}::{} ({})",
      clsName,
      methodName,
      context ? context->name()->data() : "{no context}");
  }

  bool equals(const ClsMethodData& o) const {
    // The strings are static so we can use pointer equality.
    return
      clsName == o.clsName &&
      methodName == o.methodName &&
      context == o.context;
  }
  size_t hash() const {
    return folly::hash::hash_combine(
      std::hash<const StringData*>()(clsName),
      std::hash<const StringData*>()(methodName),
      std::hash<const Class*>()(context)
    );
  }

  const StringData* clsName;
  const StringData* methodName;
  const NamedEntity* namedEntity;
  const Class* context;
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
 * Func
 */
struct FuncData : IRExtraData {
  explicit FuncData(const Func* f)
    : func(f)
  {}

  std::string show() const {
    return folly::format("{}", func->fullName()).str();
  }

  const Func* func;
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
 * Func with argument index and expected type.
 */
struct FuncArgTypeData : IRExtraData {
  explicit FuncArgTypeData(const Func* f, int64_t arg, DataType t)
    : func(f)
    , argNum(arg)
    , type(t)
  {}

  std::string show() const {
    return folly::format("{},{},{}", func->name(), argNum, type).str();
  }

  const Func* func;
  int64_t argNum;
  DataType type;
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

/*
 * Index into, e.g., an array.
 */
struct IndexData : IRExtraData {
  explicit IndexData(uint32_t index) : index(index) {}

  std::string show() const { return folly::format("{}", index).str(); }
  size_t hash() const { return std::hash<uint32_t>()(index); }

  bool equals(const IndexData& o) const {
    return index == o.index;
  }

  uint32_t index;
};

/*
 * An index and a key used to initialized a dict-ish array element.
 */
struct KeyedIndexData : IRExtraData {
  explicit KeyedIndexData(uint32_t index, const StringData* key)
    : index(index)
    , key(key)
    {}

  std::string show() const;

  uint32_t index;
  const StringData* key;
};

/*
 * Used to optimize array accesses. Does not change semantics, but changes how
 * we do the lookup - e.g. we scan small static arrays for static string keys.
 *
 * NOTE: Currently, we only use this hint in ArrayIdx and DictIdx. We may want
 * to use it for ArrayExists / ArrayGet / etc.
 */
struct SizeHintData : IRExtraData {
  enum SizeHint {
    Default,
    SmallStatic
  };

  SizeHintData() : hint(SizeHint::Default) {}
  explicit SizeHintData(SizeHint hint) : hint(hint) {}

  std::string show() const {
    switch (hint) {
      case SizeHint::Default:     return "Default";
      case SizeHint::SmallStatic: return "SmallStatic";
    }
    not_reached();
  }

  SizeHint hint;
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
 * Iter instruction data, used for both key-value and value-only iterators.
 * Check args.hasKey() to distinguish between the two.
 */
struct IterData : IRExtraData {
  explicit IterData(IterArgs args) : args(args) {}

  std::string show() const {
    return HPHP::show(args, [&](int32_t id) {
      return folly::to<std::string>(id);
    });
  }

  IterArgs args;
};

struct IterTypeData : IRExtraData {
  IterTypeData(uint32_t iterId, IterSpecialization type)
    : iterId{iterId}
    , type{type}
  {
    always_assert(type.specialized);
  }

  std::string show() const {
    return folly::format("{}::{}", iterId, HPHP::show(type)).str();
  }

  uint32_t iterId;
  IterSpecialization type;
};

struct IterOffsetData : IRExtraData {
  IterOffsetData(int16_t offset) : offset(offset) {}

  std::string show() const { return folly::to<std::string>(offset); }

  int16_t offset;
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
 * Array access profile.
 */
struct ArrayAccessProfileData : RDSHandleData {
  ArrayAccessProfileData(rds::Handle handle, bool cowCheck)
    : RDSHandleData(handle), cowCheck(cowCheck) {}

  std::string show() const {
    return folly::to<std::string>(handle, ",", cowCheck);
  }

  bool equals(ArrayAccessProfileData o) const {
    return handle == o.handle && cowCheck == o.cowCheck;
  }
  size_t hash() const {
    return folly::hash::hash_combine(std::hash<uint32_t>()(handle),
                                     std::hash<bool>()(cowCheck));
  }

  bool cowCheck;
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
 * DefInlineFP is present when we need to create a frame for inlining.  This
 * instruction also carries some metadata used by IRBuilder to track state
 * during an inlined call.
 */
struct DefInlineFPData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      target->fullName()->data(), "(),",
      callBCOff, ',',
      retSPOff.offset, ',',
      spOffset.offset
    );
  }

  const Func* target;
  Offset callBCOff;
  FPInvOffset retSPOff;
  IRSPRelOffset spOffset; // offset from caller SP to bottom of callee's ActRec
  uint32_t numArgs;
  bool asyncEagerReturn;
  bool syncVmfp;
};

struct SyncReturnBCData : IRExtraData {
  SyncReturnBCData(Offset callBCOff, IRSPRelOffset spOff)
    : callBCOffset(callBCOff)
    , spOffset(spOff)
  {}
  std::string show() const {
    return folly::to<std::string>(callBCOffset, ",", spOffset.offset);
  }

  Offset callBCOffset;
  IRSPRelOffset spOffset;
};

struct CallBuiltinData : IRExtraData {
  explicit CallBuiltinData(IRSPRelOffset spOffset,
                           IRSPRelOffset retSpOffset,
                           const Func* callee,
                           int32_t numNonDefault)
    : spOffset(spOffset)
    , retSpOffset(retSpOffset)
    , callee{callee}
    , numNonDefault{numNonDefault}
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',',
      callee->fullName()->data()
    );
  }

  IRSPRelOffset spOffset; // offset from StkPtr to last passed arg
  IRSPRelOffset retSpOffset; // offset from StkPtr after a return
  const Func* callee;
  int32_t numNonDefault;
};

struct CallData : IRExtraData {
  explicit CallData(IRSPRelOffset spOffset,
                    uint32_t numArgs,
                    uint32_t numOut,
                    Offset callOffset,
                    uint32_t genericsBitmap,
                    bool hasGenerics,
                    bool hasUnpack,
                    bool dynamicCall,
                    bool asyncEagerReturn,
                    bool formingRegion,
                    bool skipNumArgsCheck)
    : spOffset(spOffset)
    , numArgs(numArgs)
    , numOut(numOut)
    , callOffset(callOffset)
    , genericsBitmap(genericsBitmap)
    , hasGenerics(hasGenerics)
    , hasUnpack(hasUnpack)
    , dynamicCall(dynamicCall)
    , asyncEagerReturn(asyncEagerReturn)
    , formingRegion(formingRegion)
    , skipNumArgsCheck(skipNumArgsCheck)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',', numArgs, ',', numOut, ',', callOffset,
      hasGenerics
        ? folly::sformat(",hasGenerics({})", genericsBitmap)
        : std::string{},
      hasUnpack ? ",unpack" : "",
      dynamicCall ? ",dynamicCall" : "",
      asyncEagerReturn ? ",asyncEagerReturn" : "",
      formingRegion ? ",formingRegion" : "",
      skipNumArgsCheck ? ",skipNumArgsCheck" : ""
    );
  }

  uint32_t numInputs() const {
    return numArgs + (hasUnpack ? 1 : 0) + (hasGenerics ? 1 : 0);
  }

  IRSPRelOffset spOffset; // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numArgs;
  uint32_t numOut;     // number of values returned via stack from the callee
  Offset callOffset;   // offset from func->base()
  uint32_t genericsBitmap;
  bool hasGenerics;
  bool hasUnpack;
  bool dynamicCall;
  bool asyncEagerReturn;
  bool formingRegion;
  bool skipNumArgsCheck;
};

struct CallUnpackData : IRExtraData {
  explicit CallUnpackData(IRSPRelOffset spOffset,
                          uint32_t numArgs,
                          uint32_t numOut,
                          Offset callOffset,
                          bool hasGenerics,
                          bool dynamicCall,
                          bool formingRegion)
    : spOffset(spOffset)
    , numArgs(numArgs)
    , numOut(numOut)
    , callOffset(callOffset)
    , hasGenerics(hasGenerics)
    , dynamicCall(dynamicCall)
    , formingRegion(formingRegion)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',', numArgs, ',', numOut, ',', callOffset,
      hasGenerics ? ",hasGenerics" : "",
      dynamicCall ? ",dynamicCall" : "",
      formingRegion ? ",formingRegion" : ""
    );
  }

  uint32_t numInputs() const {
    return numArgs + 1 + (hasGenerics ? 1 : 0);
  }

  IRSPRelOffset spOffset; // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numArgs;
  uint32_t numOut;
  Offset callOffset;  // offset from unit m_bc (unlike the one in CallData)
  bool hasGenerics;
  bool dynamicCall;
  bool formingRegion;
};

struct RetCtrlData : IRExtraData {
  explicit RetCtrlData(IRSPRelOffset offset, bool suspendingResumed,
                       AuxUnion aux)
    : offset(offset)
    , suspendingResumed(suspendingResumed)
    , aux(aux)
  {}

  std::string show() const {
    return folly::to<std::string>(
      offset.offset,
      suspendingResumed ? ",suspendingResumed" : ""
    );
  }

  // Adjustment we need to make to the stack pointer (for cross-tracelet ABI
  // purposes) before returning.
  IRSPRelOffset offset;

  // Indicates that the current resumable frame is being suspended without
  // decrefing locals.  Used by refcount optimizer.
  bool suspendingResumed;

  // TV aux value to attach to the function's return value.
  AuxUnion aux;
};

/*
 * Name of a record
 */
struct RecNameData : IRExtraData {
  explicit RecNameData(const StringData* name)
    : recName(name)
  {}

  std::string show() const {
    return folly::to<std::string>(recName->data());
  }

  size_t hash() const { return recName->hash(); }
  bool equals(const RecNameData& o) const {
    return recName == o.recName;
  }

  const StringData* recName;
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

struct FuncNameData : IRExtraData {
  FuncNameData(const StringData* name, const Class* context)
    : name(name)
    , context(context)
  {}

  std::string show() const {
    return folly::to<std::string>(
      name->data(), ",", context ? context->name()->data() : "{no context}");
  }

  size_t hash() const { return name->hash(); }
  bool equals(const FuncNameData& o) const {
    return name == o.name && context == o.context;
  }

  const StringData* name;
  const Class* context;
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

  explicit InterpOneData(IRSPRelOffset spOffset)
    : spOffset(spOffset)
    , nChangedLocals(0)
    , changedLocals(nullptr)
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
    assertx(!smashesAllLocals || !nChangedLocals);
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

struct CreateAAWHData : IRExtraData {
  explicit CreateAAWHData(uint32_t first, uint32_t count)
    : first(first)
    , count(count)
  {}

  std::string show() const {
    return folly::format("{},{}", first, count).str();
  }

  uint32_t first;
  uint32_t count;
};

struct CountWHNotDoneData : IRExtraData {
  explicit CountWHNotDoneData(uint32_t first, uint32_t count)
    : first(first)
    , count(count)
  {}

  std::string show() const {
    return folly::format("{},{}", first, count).str();
  }

  uint32_t first;
  uint32_t count;
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

struct MemoValueStaticData : IRExtraData {
  explicit MemoValueStaticData(const Func* func,
                               folly::Optional<bool> asyncEager,
                               bool loadAux)
    : func{func}
    , asyncEager{asyncEager}
    , loadAux{loadAux} {}
  std::string show() const {
    return folly::sformat(
      "{},{},{}",
      func->fullName()->toCppString(),
      asyncEager ? folly::to<std::string>(*asyncEager) : "-",
      loadAux
    );
  }
  const Func* func;
  folly::Optional<bool> asyncEager;
  bool loadAux;
};

struct MemoValueInstanceData : IRExtraData {
  explicit MemoValueInstanceData(Slot slot,
                                 const Func* func,
                                 folly::Optional<bool> asyncEager,
                                 bool loadAux)
    : slot{slot}
    , func{func}
    , asyncEager{asyncEager}
    , loadAux{loadAux} {}
  std::string show() const {
    return folly::sformat(
      "{},{},{},{}",
      slot,
      func->fullName(),
      asyncEager ? folly::to<std::string>(*asyncEager) : "-",
      loadAux
    );
  }
  Slot slot;
  const Func* func;
  folly::Optional<bool> asyncEager;
  bool loadAux;
};

struct MemoCacheStaticData : IRExtraData {
  MemoCacheStaticData(const Func* func,
                      LocalRange keys,
                      const bool* types,
                      folly::Optional<bool> asyncEager,
                      bool loadAux)
    : func{func}
    , keys{keys}
    , types{types}
    , asyncEager{asyncEager}
    , loadAux{loadAux} {}

  MemoCacheStaticData* clone(Arena& arena) const {
    auto p =
      new (arena) MemoCacheStaticData(func, keys, types, asyncEager, loadAux);
    auto tmp = new (arena) bool[keys.count];
    std::copy(types, types + keys.count, tmp);
    p->types = tmp;
    p->stackOffset = stackOffset;
    return p;
  }

  std::string show() const {
    std::string ret;
    if (stackOffset) {
      ret += folly::sformat(
        "{},IRSPOff {}", func->fullName(), stackOffset->offset
      );
    } else {
      ret += folly::sformat("{},{}", func->fullName(), HPHP::show(keys));
    }

    if (keys.count > 0) {
      ret += ",<";
      for (auto i = 0; i < keys.count; ++i) {
        if (i > 0) ret += ",";
        ret += folly::sformat("{}", types[i] ? "string" : "int");
      }
      ret += ">";
    }
    return ret;
  }

  const Func* func;
  LocalRange keys;
  const bool* types;
  folly::Optional<bool> asyncEager;
  bool loadAux;
  // Should only be present if the frame is given by a StkPtr
  folly::Optional<IRSPRelOffset> stackOffset;
};

struct MemoCacheInstanceData : IRExtraData {
  MemoCacheInstanceData(Slot slot,
                        LocalRange keys,
                        const bool* types,
                        const Func* func,
                        bool shared,
                        folly::Optional<bool> asyncEager,
                        bool loadAux)
    : slot{slot}
    , keys{keys}
    , types{types}
    , func{func}
    , shared{shared}
    , asyncEager{asyncEager}
    , loadAux{loadAux} {}

  MemoCacheInstanceData* clone(Arena& arena) const {
    auto p = new (arena) MemoCacheInstanceData(
      slot, keys, types, func, shared, asyncEager, loadAux
    );
    auto tmp = new (arena) bool[keys.count];
    std::copy(types, types + keys.count, tmp);
    p->types = tmp;
    p->stackOffset = stackOffset;
    return p;
  }

  std::string show() const {
    return folly::sformat(
      "{},{},{},<{}>,{}",
      slot,
      func->fullName(),
      stackOffset
        ? folly::sformat("IRSPOff {}", stackOffset->offset)
        : HPHP::show(keys),
      [&]{
        using namespace folly::gen;
        return range<uint32_t>(0, keys.count)
          | map([this] (uint32_t i) { return types[i] ? "string" : "int"; })
          | unsplit<std::string>(",");
      }(),
      shared ? "shared" : "non-shared"
    );
  }

  Slot slot;
  LocalRange keys;
  const bool* types;
  const Func* func;
  bool shared;
  folly::Optional<bool> asyncEager;
  bool loadAux;
  // Should only be present if the frame is given by a StkPtr
  folly::Optional<IRSPRelOffset> stackOffset;
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

struct SuspendOffset : IRExtraData {
  explicit SuspendOffset(Offset off) : off(off) {}
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
  explicit ContEnterData(IRSPRelOffset spOffset, Offset callBCOffset,
                         bool isAsync)
    : spOffset(spOffset)
    , callBCOffset(callBCOffset)
    , isAsync(isAsync)
  {}

  std::string show() const {
    return folly::to<std::string>(spOffset.offset, ',', callBCOffset,
                                  isAsync ? ",async" : "");
  }

  IRSPRelOffset spOffset;
  Offset callBCOffset;
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

struct CheckInOutsData : IRExtraData {
  CheckInOutsData(unsigned firstBit, uint64_t mask, uint64_t vals)
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

struct ProfileCallTargetData : IRExtraData {
  explicit ProfileCallTargetData(rds::Handle handle)
    : handle(handle)
  {}

  std::string show() const {
    return folly::to<std::string>(handle);
  }

  rds::Handle handle;
};

struct BeginInliningData : IRExtraData {
  BeginInliningData(IRSPRelOffset offset, const Func* func, int cost)
    : offset(offset)
    , func(func)
    , cost(cost)
  {}

  std::string show() const {
    return folly::to<std::string>("IRSPOff ", offset.offset,
                                  " FUNC ", func->fullName()->data());
  }

  IRSPRelOffset offset;
  const Func* func;
  int cost;
};

struct ParamData : IRExtraData {
  explicit ParamData(int32_t paramId) : paramId(paramId) {}

  std::string show() const {
    return folly::to<std::string>(paramId);
  }

  int32_t paramId;
};

struct ParamWithTCData : IRExtraData {
  explicit ParamWithTCData(int32_t paramId, const TypeConstraint* tc)
    : paramId(paramId)
    , tc(tc) {}

  std::string show() const {
    return folly::to<std::string>(paramId, ":", tc->displayName());
  }

  int32_t paramId;
  const TypeConstraint* tc;
};

struct RaiseHackArrTypehintNoticeData : IRExtraData {
  explicit RaiseHackArrTypehintNoticeData(const TypeConstraint& tc) : tc{tc} {}

  std::string show() const { return tc.displayName(); }

  TypeConstraint tc;
};

struct RaiseHackArrParamNoticeData : RaiseHackArrTypehintNoticeData {
  RaiseHackArrParamNoticeData(const TypeConstraint& tc,
                              int32_t id, bool isReturn)
    : RaiseHackArrTypehintNoticeData{tc}
    , id{id}
    , isReturn{isReturn} {}

  std::string show() const {
    auto const typeStr = RaiseHackArrTypehintNoticeData::show();
    return folly::to<std::string>(
      typeStr, ",",
      id, ",",
      isReturn ? "true" : "false"
    );
  }

  int32_t id;
  bool isReturn;
};

struct RaiseClsMethPropConvertNoticeData : IRExtraData {
  RaiseClsMethPropConvertNoticeData(const TypeConstraint* tc, bool isSProp)
    : tc(tc)
    , isSProp(isSProp)
  {}

  std::string show() const {
    return folly::to<std::string>(tc->displayName(), ",", isSProp);
  }

  union { const TypeConstraint* tc; int64_t tcIntVal; };
  bool isSProp;
};

struct ArrayGetExceptionData : IRExtraData {
  explicit ArrayGetExceptionData(bool isInOut) : isInOut(isInOut) {}

  std::string show() const {
    return isInOut ? "inout" : "none";
  }

  bool isInOut;
};

struct AssertReason : IRExtraData {
  explicit AssertReason(Reason r) : reason{r.file, r.line} {}

  std::string show() const {
    return jit::show(reason);
  }

  Reason reason;
};

#define ASSERT_REASON AssertReason{Reason{__FILE__, __LINE__}}

struct EndCatchData : IRSPRelOffsetData {
  enum class CatchMode { UnwindOnly, CallCatch, SideExit, LocalsDecRefd };
  enum class FrameMode { Phplogue, Stublogue };
  enum class Teardown  { NA, None, Full, OnlyThis };

  explicit EndCatchData(IRSPRelOffset offset, CatchMode mode,
                        FrameMode stublogue, Teardown teardown)
    : IRSPRelOffsetData{offset}
    , mode{mode}
    , stublogue{stublogue}
    , teardown{teardown}
    {}

  std::string show() const {
    return folly::to<std::string>(
      IRSPRelOffsetData::show(), ",",
      mode == CatchMode::UnwindOnly ? "UnwindOnly" :
        mode == CatchMode::CallCatch ? "CallCatch" :
          mode == CatchMode::SideExit ? "SideExit" : "LocalsDecRefd", ",",
      stublogue == FrameMode::Stublogue ? "Stublogue" : "Phplogue", ",",
      teardown == Teardown::NA ? "NA" :
        teardown == Teardown::None ? "None" :
          teardown == Teardown::Full ? "Full" : "OnlyThis");
  }

  CatchMode mode;
  FrameMode stublogue;
  Teardown teardown;
};

struct EnterTCUnwindData : IRExtraData {
  explicit EnterTCUnwindData(bool teardown) : teardown{teardown} {}

  std::string show() const {
    return folly::to<std::string>(teardown ? "" : "no-", "teardown");
  }

  bool teardown;
};

/*
 * Func attributes
 */
struct AttrData : IRExtraData {
  explicit AttrData(int32_t attr) : attr(attr) {}

  std::string show() const {
    return folly::format("{}", attr).str();
  }

  int32_t attr;
};

struct MethCallerData : IRExtraData {
  explicit MethCallerData(bool isCls) : isCls(isCls) {}
  std::string show() const {
    return folly::format("{}", isCls).str();
  }
  bool isCls;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

X(ArrayIdx,                     SizeHintData);
X(DictIdx,                      SizeHintData);
X(LdBindAddr,                   LdBindAddrData);
X(ProfileSwitchDest,            ProfileSwitchData);
X(JmpSwitchDest,                JmpSwitchData);
X(LdSSwitchDestFast,            LdSSwitchData);
X(LdSSwitchDestSlow,            LdSSwitchData);
X(CheckLoc,                     LocalId);
X(AssertLoc,                    LocalId);
X(LdLocAddr,                    LocalId);
X(LdLoc,                        LocalId);
X(LdLocPseudoMain,              LocalId);
X(LdClsInitElem,                IndexData);
X(StClsInitElem,                IndexData);
X(StLoc,                        LocalId);
X(StLocPseudoMain,              LocalId);
X(StLocRange,                   LocalIdRange);
X(AdvanceMixedPtrIter,          IterOffsetData);
X(AdvancePackedPtrIter,         IterOffsetData);
X(CheckIter,                    IterTypeData);
X(StIterBase,                   IterId);
X(StIterType,                   IterTypeData);
X(StIterPos,                    IterId);
X(StIterEnd,                    IterId);
X(LdIterBase,                   IterId);
X(LdIterPos,                    IterId);
X(LdIterEnd,                    IterId);
X(KillIter,                     IterId);
X(IterFree,                     IterId);
X(IterInit,                     IterData);
X(IterInitK,                    IterData);
X(IterNext,                     IterData);
X(IterNextK,                    IterData);
X(LIterInit,                    IterData);
X(LIterInitK,                   IterData);
X(LIterNext,                    IterData);
X(LIterNextK,                   IterData);
X(ConstructInstance,            ClassData);
X(ConstructClosure,             ClassData);
X(InitProps,                    ClassData);
X(InitSProps,                   ClassData);
X(NewInstanceRaw,               ClassData);
X(InitObjProps,                 ClassData);
X(InitObjMemoSlots,             ClassData);
X(InstanceOfIfaceVtable,        InstanceOfIfaceVtableData);
X(ResolveTypeStruct,            ResolveTypeStructData);
X(ExtendsClass,                 ExtendsClassData);
X(CheckStk,                     IRSPRelOffsetData);
X(StStk,                        IRSPRelOffsetData);
X(StOutValue,                   IndexData);
X(LdOutAddr,                    IndexData);
X(AssertStk,                    IRSPRelOffsetData);
X(DefFrameRelSP,                FPInvOffsetData);
X(DefRegSP,                     FPInvOffsetData);
X(LdStk,                        IRSPRelOffsetData);
X(LdStkAddr,                    IRSPRelOffsetData);
X(DefInlineFP,                  DefInlineFPData);
X(BeginInlining,                BeginInliningData);
X(SyncReturnBC,                 SyncReturnBCData);
X(InlineReturn,                 FPRelOffsetData);
X(InlineSuspend,                FPRelOffsetData);
X(InlineReturnNoFrame,          FPRelOffsetData);
X(ReqRetranslate,               ReqRetranslateData);
X(ReqBindJmp,                   ReqBindJmpData);
X(ReqRetranslateOpt,            IRSPRelOffsetData);
X(CheckCold,                    TransIDData);
X(IncProfCounter,               TransIDData);
X(DefFuncEntryFP,               FuncData);
X(Call,                         CallData);
X(CallBuiltin,                  CallBuiltinData);
X(CallUnpack,                   CallUnpackData);
X(RetCtrl,                      RetCtrlData);
X(AsyncFuncRet,                 IRSPRelOffsetData);
X(AsyncFuncRetSlow,             IRSPRelOffsetData);
X(AsyncSwitchFast,              IRSPRelOffsetData);
X(LookupClsMethodCache,         ClsMethodData);
X(LdClsMethodCacheFunc,         ClsMethodData);
X(LdClsMethodCacheCls,          ClsMethodData);
X(LdClsMethodFCacheFunc,        ClsMethodData);
X(LookupClsMethodFCache,        ClsMethodData);
X(LdIfaceMethod,                IfaceMethodData);
X(LdClsCns,                     ClsCnsName);
X(InitClsCns,                   ClsCnsName);
X(LdSubClsCns,                  LdSubClsCnsData);
X(LdSubClsCnsClsName,           LdSubClsCnsData);
X(CheckSubClsCns,               LdSubClsCnsData);
X(ProfileSubClsCns,             ProfileSubClsCnsData);
X(LdFuncCached,                 FuncNameData);
X(LookupFuncCached,             FuncNameData);
X(LdObjMethodS,                 FuncNameData);
X(LdObjMethodD,                 OptClassData);
X(ThrowMissingArg,              FuncArgData);
X(RaiseClsMethPropConvertNotice,RaiseClsMethPropConvertNoticeData);
X(RaiseTooManyArg,              FuncData);
X(ThrowParamInOutMismatch,      ParamData);
X(ThrowParamInOutMismatchRange, CheckInOutsData);
X(ThrowArrayIndexException,     ArrayGetExceptionData);
X(ThrowArrayKeyException,       ArrayGetExceptionData);
X(ThrowParameterWrongType,      FuncArgTypeData);
X(CheckClsReifiedGenericMismatch,
                                ClassData);
X(CheckFunReifiedGenericMismatch,
                                FuncData);
X(IsFunReifiedGenericsMatched,  FuncData);
X(InterpOne,                    InterpOneData);
X(InterpOneCF,                  InterpOneData);
X(StClosureArg,                 IndexData);
X(RBTraceEntry,                 RBEntryData);
X(RBTraceMsg,                   RBMsgData);
X(OODeclExists,                 ClassKindData);
X(NewStructArray,               NewStructData);
X(NewStructDArray,              NewStructData);
X(NewStructDict,                NewStructData);
X(NewRecord,                    NewStructData);
X(NewRecordArray,               NewStructData);
X(AllocStructArray,             NewStructData);
X(AllocStructDArray,            NewStructData);
X(AllocStructDict,              NewStructData);
X(AllocPackedArray,             PackedArrayData);
X(AllocVArray,                  PackedArrayData);
X(AllocVecArray,                PackedArrayData);
X(NewKeysetArray,               NewKeysetArrayData);
X(InitPackedLayoutArrayLoop,    InitPackedArrayLoopData);
X(InitPackedLayoutArray,        IndexData);
X(InitMixedLayoutArray,         KeyedIndexData);
X(CreateAAWH,                   CreateAAWHData);
X(CountWHNotDone,               CountWHNotDoneData);
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
X(ProfileMixedArrayAccess,      ArrayAccessProfileData);
X(ProfileDictAccess,            ArrayAccessProfileData);
X(ProfileKeysetAccess,          ArrayAccessProfileData);
X(ProfileType,                  RDSHandleData);
X(ProfileCall,                  ProfileCallTargetData);
X(ProfileMethod,                ProfileCallTargetData);
X(LdRDSAddr,                    RDSHandleData);
X(CheckRDSInitialized,          RDSHandleData);
X(MarkRDSInitialized,           RDSHandleData);
X(LdInitRDSAddr,                RDSHandleData);
X(BaseG,                        MOpModeData);
X(PropX,                        MOpModeData);
X(PropDX,                       MOpModeData);
X(ElemX,                        MOpModeData);
X(ElemDX,                       MOpModeData);
X(ElemUX,                       MOpModeData);
X(ElemArrayX,                   MOpModeData);
X(ElemDictX,                    MOpModeData);
X(ElemKeysetX,                  MOpModeData);
X(CGetProp,                     MOpModeData);
X(CGetElem,                     MOpModeData);
X(ArrayGet,                     MOpModeData);
X(MemoGetStaticValue,           MemoValueStaticData);
X(MemoSetStaticValue,           MemoValueStaticData);
X(MemoGetStaticCache,           MemoCacheStaticData);
X(MemoSetStaticCache,           MemoCacheStaticData);
X(MemoGetLSBValue,              MemoValueStaticData);
X(MemoSetLSBValue,              MemoValueStaticData);
X(MemoGetLSBCache,              MemoCacheStaticData);
X(MemoSetLSBCache,              MemoCacheStaticData);
X(MemoGetInstanceValue,         MemoValueInstanceData);
X(MemoSetInstanceValue,         MemoValueInstanceData);
X(MemoGetInstanceCache,         MemoCacheInstanceData);
X(MemoSetInstanceCache,         MemoCacheInstanceData);
X(SetOpProp,                    SetOpData);
X(SetOpTV,                      SetOpData);
X(SetOpTVVerify,                SetOpData);
X(IncDecProp,                   IncDecData);
X(SetOpElem,                    SetOpData);
X(IncDecElem,                   IncDecData);
X(StArResumeAddr,               SuspendOffset);
X(StContArState,                GeneratorState);
X(ContEnter,                    ContEnterData);
X(EagerSyncVMRegs,              IRSPRelOffsetData);
X(JmpSSwitchDest,               IRSPRelOffsetData);
X(DbgTrashStk,                  IRSPRelOffsetData);
X(DbgTrashFrame,                IRSPRelOffsetData);
X(DbgTraceCall,                 IRSPRelOffsetData);
X(LdPropAddr,                   IndexData);
X(LdInitPropAddr,               IndexData);
X(NewCol,                       NewColData);
X(NewColFromArray,              NewColData);
X(CheckSurpriseFlagsEnter,      FuncEntryData);
X(CheckSurpriseAndStack,        FuncEntryData);
X(ContPreNext,                  IsAsyncData);
X(ContStartedCheck,             IsAsyncData);
X(ContValid,                    IsAsyncData);
X(LdContResumeAddr,             IsAsyncData);
X(LdContActRec,                 IsAsyncData);
X(DecRef,                       DecRefData);
X(DecRefNZ,                     DecRefData);
X(ProfileDecRef,                DecRefData);
X(LdTVAux,                      LdTVAuxData);
X(CheckInOuts,                  CheckInOutsData);
X(RaiseHackArrParamNotice,      RaiseHackArrParamNoticeData);
X(RaiseHackArrPropNotice,       RaiseHackArrTypehintNoticeData);
X(DbgAssertRefCount,            AssertReason);
X(Unreachable,                  AssertReason);
X(EndBlock,                     AssertReason);
X(VerifyRetCallable,            ParamData);
X(VerifyRetCls,                 ParamData);
X(VerifyRetRecDesc,             ParamData);
X(VerifyParamFail,              ParamWithTCData);
X(VerifyParamFailHard,          ParamWithTCData);
X(VerifyRetFail,                ParamWithTCData);
X(VerifyRetFailHard,            ParamWithTCData);
X(VerifyReifiedLocalType,       ParamData);
X(EndCatch,                     EndCatchData);
X(EnterTCUnwind,                EnterTCUnwindData);
X(FuncHasAttr,                  AttrData);
X(LdMethCallerName,             MethCallerData);
X(LdRecDescCached,              RecNameData);
X(LdRecDescCachedSafe,          RecNameData);

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
