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
#include "hphp/util/optional.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"

#include <folly/Conv.h>
#include <folly/Hash.h>
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
 *   - Implement an equals() member that indicates equality.
 *   - Implement a stableHash() method that is invariant across process
 *     restarts.
 *
 * In addition, extra data belonging to IRInstructions that may be hashed in
 * IRInstrTables must:
 *
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

  size_t stableHash() const {
    return cls->stableHash();
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

  size_t stableHash() const {
    return (cls ? cls->stableHash() : 0);
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      cls ? cls->stableHash() : 0,
      std::hash<int32_t>()(offset.offset),
      std::hash<uint32_t>()(size),
      std::hash<uint8_t>()(suppress << 1 | isOrAsOp)
    );
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

  size_t stableHash() const {
    return cls->stableHash() ^ (strictLikely ? -1 : 0);
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

  size_t stableHash() const {
    return cls->stableHash() ^ (canOptimize ? -1 : 0);
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      clsName->hashStatic(),
      methodName->hashStatic(),
      context ? context->stableHash() : 0
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

  size_t stableHash() const {
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

  bool equals(const FuncData& f) const {
    return func == f.func;
  }

  size_t stableHash() const {
    return func->stableHash();
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

  bool equals(const FuncArgData& o) const {
    return func == o.func && argNum == o.argNum;
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      func->stableHash(),
      std::hash<int64_t>()(argNum)
    );
  }

  const Func* func;
  int64_t argNum;
};

/*
 * Func with argument index and expected type.
 */
struct FuncArgTypeData : IRExtraData {
  explicit FuncArgTypeData(const Func* f, int64_t arg, const StringData* t)
    : func(f)
    , argNum(arg)
    , type(t)
  {}

  std::string show() const {
    return folly::format("{},{},{}", func->name(), argNum, type).str();
  }

  bool equals(const FuncArgTypeData& o) const {
    return func == o.func && argNum == o.argNum && type == o.type;
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      func->stableHash(),
      std::hash<int64_t>()(argNum),
      type->hashStatic()
    );
  }

  const Func* func;
  int64_t argNum;
  const StringData* type;
};

struct LdClsTypeCnsData : IRExtraData {
  explicit LdClsTypeCnsData(bool no_throw) : noThrow(no_throw) {}

  std::string show() const { return folly::to<std::string>(noThrow); }

  bool equals(LdClsTypeCnsData o) const { return noThrow == o.noThrow; }
  size_t hash() const { return noThrow ? 1 : 0; }
  size_t stableHash() const { return noThrow ? 1 : 0; }

  bool noThrow;
};

/*
 * Local variable ID.
 */
struct LocalId : IRExtraData {
  explicit LocalId(uint32_t id) : locId(id) {}

  std::string show() const { return folly::to<std::string>(locId); }

  bool equals(LocalId o) const { return locId == o.locId; }
  size_t hash() const { return std::hash<uint32_t>()(locId); }
  size_t stableHash() const { return std::hash<uint32_t>()(locId); }

  uint32_t locId;
};

/*
 * Index into, e.g., an array.
 */
struct IndexData : IRExtraData {
  explicit IndexData(uint32_t index) : index(index) {}

  std::string show() const { return folly::format("{}", index).str(); }
  size_t hash() const { return std::hash<uint32_t>()(index); }
  size_t stableHash() const { return std::hash<uint32_t>()(index); }

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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<uint32_t>()(index),
      key->hashStatic()
    );
  }

  bool equals(const KeyedIndexData& o) const {
    return index == o.index && key == o.key;
  }

  uint32_t index;
  const StringData* key;
};

/*
 * Used to optimize array accesses. Does not change semantics, but changes how
 * we do the lookup - e.g. we scan small static arrays for static string keys.
 *
 * NOTE: Currently, we only use this hint in DictIdx. We may want
 * to use it for ArrayExists / etc.
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

  size_t hash() const {
    return stableHash();
  }

  size_t stableHash() const {
    return std::hash<SizeHint>()(hint);
  }

  bool equals(const SizeHintData& o) const {
    return hint == o.hint;
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
  size_t stableHash() const { return std::hash<uint32_t>()(iterId); }

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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(args.iterId),
      std::hash<int32_t>()(args.keyId),
      std::hash<int32_t>()(args.valId),
      std::hash<IterArgs::Flags>()(args.flags)
    );
  }

  bool equals(const IterData& o) const {
    return args == o.args;
  }

  IterArgs args;
};

struct IterTypeData : IRExtraData {
  IterTypeData(uint32_t iterId, IterSpecialization type, ArrayLayout layout)
    : iterId{iterId}
    , type{type}
    , layout{layout}
  {
    always_assert(type.specialized);
  }

  std::string show() const {
    auto const type_str = HPHP::show(type);
    auto const layout_str = layout.describe();
    return folly::format("{}::{}::{}", iterId, type_str, layout_str).str();
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<uint32_t>()(iterId),
      std::hash<uint8_t>()(type.as_byte),
      std::hash<uint16_t>()(layout.toUint16())
    );
  }

  bool equals(const IterTypeData& o) const {
    return iterId == o.iterId && type.as_byte == o.type.as_byte &&
           layout == o.layout;
  }

  uint32_t iterId;
  IterSpecialization type;
  ArrayLayout layout;
};

struct IterOffsetData : IRExtraData {
  IterOffsetData(int16_t offset) : offset(offset) {}

  std::string show() const { return folly::to<std::string>(offset); }

  size_t stableHash() const { return std::hash<int16_t>()(offset); }

  bool equals(const IterOffsetData& o) const { return offset == o.offset; }

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

  size_t stableHash() const {
    auto const sym = rds::reverseLink(handle);
    if (!sym) return 0;
    return rds::symbol_stable_hash(*sym);
  }

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

  size_t stableHash() const {
    return folly::hash::hash_combine(RDSHandleData::stableHash(),
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

  size_t stableHash() const {
    return std::hash<TransID>()(transId);
  }

  bool equals(const TransIDData& o) const {
    return transId == o.transId;
  }

  TransID transId;
};

struct DefFPData : IRExtraData {
  explicit DefFPData(Optional<IRSPRelOffset> offset) : offset(offset) {}

  std::string show() const {
    if (!offset) return "IRSPOff unknown";
    return folly::to<std::string>("IRSPOff ", offset->offset);
  }

  bool equals(DefFPData o) const { return offset == o.offset; }
  size_t stableHash() const {
    return offset ? std::hash<int32_t>()(offset->offset) : 0;
  }

  // Frame position on the stack, if it lives there and the position is known.
  Optional<IRSPRelOffset> offset;
};

/*
 * Stack pointer offset.
 */
struct DefStackData : IRExtraData {
  explicit DefStackData(SBInvOffset irSPOff, SBInvOffset bcSPOff)
    : irSPOff(irSPOff)
    , bcSPOff(bcSPOff)
  {}

  std::string show() const {
    return folly::sformat("irSPOff={}, bcSPOff={}",
                          irSPOff.offset, bcSPOff.offset);
  }

  bool equals(DefStackData o) const {
    return irSPOff == o.irSPOff && bcSPOff == o.bcSPOff;
  }
  size_t hash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(irSPOff.offset),
      std::hash<int32_t>()(bcSPOff.offset)
    );
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(irSPOff.offset),
      std::hash<int32_t>()(bcSPOff.offset)
    );
  }

  SBInvOffset irSPOff;  // offset from stack base to vmsp()
  SBInvOffset bcSPOff;  // offset from stack base to top of the stack
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

  size_t stableHash() const { return std::hash<int32_t>()(offset.offset); }

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

  size_t stableHash() const { return std::hash<int32_t>()(isAsync); }

  bool isAsync;
};

struct LdBindAddrData : IRExtraData {
  explicit LdBindAddrData(SrcKey sk, SBInvOffset bcSPOff)
    : sk(sk)
    , bcSPOff(bcSPOff)
  {}

  std::string show() const { return showShort(sk); }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      SrcKey::StableHasher()(sk),
      std::hash<int32_t>()(bcSPOff.offset)
    );
  }

  bool equals(const LdBindAddrData& o) const {
    return sk == o.sk && bcSPOff == o.bcSPOff;
  }

  SrcKey sk;
  SBInvOffset bcSPOff;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int64_t>()(numCases),
      SrcKey::StableHasher()(defaultSk),
      std::hash<int32_t>()(bcSPOff.offset)
    );
  }

  bool equals(const LdSSwitchData& o) const {
    if (numCases != o.numCases) return false;
    if (defaultSk != o.defaultSk) return false;
    if (bcSPOff != o.bcSPOff) return false;
    for (int64_t i = 0; i < numCases; i++) {
      if (cases[i].dest != o.cases[i].dest) return false;
      if (cases[i].str != o.cases[i].str) return false;
    }
    return true;
  }

  int64_t     numCases;
  const Elm*  cases;
  SrcKey      defaultSk;
  SBInvOffset bcSPOff;
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

  size_t stableHash() const {
    auto const sym = rds::reverseLink(handle);
    return folly::hash::hash_combine(
      sym ? rds::symbol_stable_hash(*sym) : 0,
      std::hash<int32_t>()(cases),
      std::hash<int64_t>()(base)
    );
  }

  bool equals(const ProfileSwitchData& o) const {
    return handle == o.handle && cases == o.cases && base == o.base;
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
    sd->spOffBCFromStackBase = spOffBCFromStackBase;
    sd->spOffBCFromIRSP = spOffBCFromIRSP;
    std::copy(targets, targets + cases, const_cast<SrcKey*>(sd->targets));
    return sd;
  }

  std::string show() const {
    return folly::sformat("{} cases", cases);
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(cases),
      std::hash<int32_t>()(spOffBCFromStackBase.offset),
      std::hash<int32_t>()(spOffBCFromIRSP.offset)
    );
  }

  bool equals(const JmpSwitchData& o) const {
    if (cases != o.cases) return false;
    if (spOffBCFromStackBase != o.spOffBCFromStackBase) return false;
    if (spOffBCFromIRSP != o.spOffBCFromIRSP) return false;
    for (int64_t i = 0; i < cases; i++) {
      if (targets[i] != o.targets[i]) return false;
    }
    return true;
  }

  int32_t cases;       // number of cases
  SrcKey* targets;     // srckeys for all targets
  SBInvOffset spOffBCFromStackBase;
  IRSPRelOffset spOffBCFromIRSP;
};

struct LdTVAuxData : IRExtraData {
  explicit LdTVAuxData(int32_t v = -1) : valid(v) {}

  std::string show() const {
    return folly::sformat("{:x}", valid);
  }

  size_t stableHash() const {
    return std::hash<int32_t>()(valid);
  }

  bool equals(const LdTVAuxData& o) const {
    return valid == o.valid;
  }

  int32_t valid;
};

struct ReqBindJmpData : IRExtraData {
  explicit ReqBindJmpData(const SrcKey& target,
                          SBInvOffset invSPOff,
                          IRSPRelOffset irSPOff)
    : target(target)
    , invSPOff(invSPOff)
    , irSPOff(irSPOff)
  {}

  std::string show() const {
    return folly::sformat(
      "{}, SBInv {}, IRSP {}",
      target.offset(), invSPOff.offset, irSPOff.offset
    );
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      SrcKey::StableHasher()(target),
      std::hash<int32_t>()(invSPOff.offset),
      std::hash<int32_t>()(irSPOff.offset)
    );
  }

  bool equals(const ReqBindJmpData& o) const {
    return target == o.target && invSPOff == o.invSPOff && irSPOff == o.irSPOff;
  }

  SrcKey target;
  SBInvOffset invSPOff;
  IRSPRelOffset irSPOff;
};

/*
 * InlineCall is present when we need to create a frame for inlining.  This
 * instruction also carries some metadata used by IRBuilder to track state
 * during an inlined call.
 */
struct InlineCallData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset
    );
  }

  size_t stableHash() const {
    // For now we are ignoring `syncVmpc`, but this could be fixed by storing
    // an SK rather than a raw PC.
    return std::hash<int32_t>()(spOffset.offset);
  }

  bool equals(const InlineCallData& o) const {
    return spOffset == o.spOffset && syncVmpc == o.syncVmpc;
  }

  IRSPRelOffset spOffset; // offset from caller SP to bottom of callee's ActRec
  PC syncVmpc{nullptr};
  SrcKey returnSk;
  SBInvOffset returnSPOff;
};

struct StFrameMetaData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      callBCOff, ',',
      asyncEagerReturn
    );
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<Offset>()(callBCOff),
      std::hash<bool>()(asyncEagerReturn)
    );
  }

  bool equals(const StFrameMetaData& o) const {
    return callBCOff == o.callBCOff && asyncEagerReturn == o.asyncEagerReturn;
  }


  Offset callBCOff;
  bool isInlined;
  bool asyncEagerReturn;
};

struct CallBuiltinData : IRExtraData {
  explicit CallBuiltinData(IRSPRelOffset spOffset,
                           Optional<IRSPRelOffset> retSpOffset,
                           const Func* callee)
    : spOffset(spOffset)
    , retSpOffset(retSpOffset)
    , callee{callee}
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',',
      retSpOffset ? folly::to<std::string>(retSpOffset->offset) : "*", ',',
      callee->fullName()->data()
    );
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(spOffset.offset),
      retSpOffset ? std::hash<int32_t>()(retSpOffset->offset) : 0,
      callee->stableHash()
    );
  }

  bool equals(const CallBuiltinData& o) const {
    return spOffset == o.spOffset && retSpOffset == o.retSpOffset &&
      callee == o.callee;
  }

  IRSPRelOffset spOffset; // offset from StkPtr to last passed arg
  Optional<IRSPRelOffset> retSpOffset; // offset from StkPtr after a return
  const Func* callee;
};

struct CallData : IRExtraData {
  explicit CallData(IRSPRelOffset spOffset,
                    uint32_t numArgs,
                    uint32_t numOut,
                    Offset callOffset,
                    uint16_t genericsBitmap,
                    bool hasGenerics,
                    bool hasUnpack,
                    bool skipRepack,
                    bool dynamicCall,
                    bool asyncEagerReturn,
                    bool formingRegion)
    : spOffset(spOffset)
    , numArgs(numArgs)
    , numOut(numOut)
    , callOffset(callOffset)
    , genericsBitmap(genericsBitmap)
    , hasGenerics(hasGenerics)
    , hasUnpack(hasUnpack)
    , skipRepack(skipRepack)
    , dynamicCall(dynamicCall)
    , asyncEagerReturn(asyncEagerReturn)
    , formingRegion(formingRegion)
  {}

  std::string show() const {
    return folly::to<std::string>(
      spOffset.offset, ',', numArgs, ',', numOut, ',', callOffset,
      hasGenerics
        ? folly::sformat(",hasGenerics({})", genericsBitmap)
        : std::string{},
      hasUnpack ? ",unpack" : "",
      skipRepack ? ",skipRepack" : "",
      dynamicCall ? ",dynamicCall" : "",
      asyncEagerReturn ? ",asyncEagerReturn" : "",
      formingRegion ? ",formingRegion" : ""
    );
  }

  uint32_t numInputs() const {
    return numArgs + (hasUnpack ? 1 : 0) + (hasGenerics ? 1 : 0);
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(spOffset.offset),
      std::hash<uint32_t>()(numArgs),
      std::hash<uint32_t>()(numOut),
      std::hash<Offset>()(callOffset),
      std::hash<uint16_t>()(genericsBitmap),
      std::hash<uint8_t>()(
        hasGenerics << 5 |
        hasUnpack << 4 |
        skipRepack << 3 |
        dynamicCall << 2 |
        asyncEagerReturn << 1 |
        formingRegion
      )
    );
  }

  bool equals(const CallData& o) const {
    return spOffset == o.spOffset &&
           numArgs == o.numArgs &&
           numOut == o.numOut &&
           callOffset == o.callOffset &&
           genericsBitmap == o.genericsBitmap &&
           hasGenerics == o.hasGenerics &&
           hasUnpack == o.hasUnpack &&
           skipRepack == o.skipRepack &&
           dynamicCall == o.dynamicCall &&
           asyncEagerReturn == o.asyncEagerReturn &&
           formingRegion == o.formingRegion;
  }


  IRSPRelOffset spOffset; // offset from StkPtr to bottom of call's ActRec+args
  uint32_t numArgs;
  uint32_t numOut;     // number of values returned via stack from the callee
  Offset callOffset;   // offset from func->base()
  uint16_t genericsBitmap;
  bool hasGenerics;
  bool hasUnpack;
  bool skipRepack;
  bool dynamicCall;
  bool asyncEagerReturn;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<bool>()(suspendingResumed),
      std::hash<uint32_t>()(aux.u_raw)
    );
  }

  bool equals(const RetCtrlData& o) const {
    return offset == o.offset && suspendingResumed == o.suspendingResumed &&
           aux.u_raw == o.aux.u_raw;
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

  size_t stableHash() const { return recName->hash(); }
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      clsName->hashStatic(),
      cnsName->hashStatic()
    );
  }
  bool equals(const ClsCnsName& o) const {
    return clsName == o.clsName &&
           cnsName == o.cnsName;
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
  size_t stableHash() const {
    return folly::hash::hash_combine(
      cnsName->hashStatic(),
      std::hash<Slot>()(slot)
    );
  }
  bool equals(const LdSubClsCnsData& o) const {
    return cnsName == o.cnsName && slot == o.slot;
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

  size_t stableHash() const {
    auto const sym = rds::reverseLink(handle);
    return folly::hash::hash_combine(
      cnsName->hashStatic(),
      sym ? rds::symbol_stable_hash(*sym) : 0
    );
  }

  bool equals(const ProfileSubClsCnsData& o) const {
    return cnsName == o.cnsName && handle == o.handle;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      name->hash(),
      context ? context->stableHash() : 0
    );
  }

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

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      std::hash<int32_t>()(spOffset.offset),
      std::hash<Offset>()(bcOff),
      std::hash<int64_t>()(cellsPopped),
      std::hash<int64_t>()(cellsPushed),
      std::hash<Op>()(opcode),
      std::hash<uint32_t>()(nChangedLocals),
      std::hash<bool>()(smashesAllLocals)
    );
    for (uint32_t i = 0; i < nChangedLocals; i++) {
      hash = folly::hash::hash_combine(
        hash,
        changedLocals[i].id,
        changedLocals[i].type.stableHash()
      );
    }
    return hash;
  }

  bool equals(const InterpOneData& o) const {
    if (spOffset == o.spOffset &&
        bcOff == o.bcOff &&
        cellsPopped == o.cellsPopped &&
        cellsPushed == o.cellsPushed &&
        opcode == o.opcode &&
        nChangedLocals == o.nChangedLocals &&
        smashesAllLocals == o.smashesAllLocals) {
      for (uint32_t i = 0; i < nChangedLocals; i++) {
        if (changedLocals[i].id != o.changedLocals[i].id) return false;
        if (changedLocals[i].type != o.changedLocals[i].type) return false;
      }
      return true;
    }
    return false;
  }

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

  size_t stableHash() const {
    return std::hash<Trace::RingBufferType>()(type) ^
           SrcKey::StableHasher()(sk);
  }

  bool equals(const RBEntryData& o) const {
    return type == o.type && sk == o.sk;
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

  size_t stableHash() const {
    return std::hash<Trace::RingBufferType>()(type) ^ msg->hash();
  }

  bool equals(const RBMsgData& o) const {
    return type == o.type && msg->equal(o.msg);
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

  size_t stableHash() const {
    return std::hash<uint32_t>()(kind);
  }

  bool equals(const ClassKindData& o) const {
    return kind == o.kind;
  }

  uint32_t kind; // ... allows for direct usage in native_call
};

struct NewStructData : IRExtraData {
  std::string show() const;

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<uint32_t>()(numKeys)
    );

    for (uint32_t i = 0; i < numKeys; i++) {
      hash = folly::hash::hash_combine(
        hash,
        keys[i]->hashStatic()
      );
    }
    return hash;
  }

  bool equals(const NewStructData& o) const {
    if (offset != o.offset) return false;
    if (numKeys != o.numKeys) return false;
    for (uint32_t i = 0; i < numKeys; i++) {
      if (keys[i] != o.keys[i]) return false;
    }
    return true;
  }

  IRSPRelOffset offset;
  uint32_t numKeys;
  StringData** keys;
};

struct ArrayLayoutData : IRExtraData {
  explicit ArrayLayoutData(ArrayLayout layout) : layout(layout) {}

  std::string show() const { return layout.describe(); }

  size_t stableHash() const { return layout.toUint16(); }

  bool equals(const ArrayLayoutData& o) const { return layout == o.layout; }

  ArrayLayout layout;
};

struct NewBespokeStructData : IRExtraData {
  NewBespokeStructData(ArrayLayout layout, IRSPRelOffset offset,
                       uint32_t numSlots, Slot* slots)
    : layout(layout), offset(offset), numSlots(numSlots), slots(slots) {}

  std::string show() const;

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      std::hash<uint16_t>()(layout.toUint16()),
      std::hash<int32_t>()(offset.offset),
      std::hash<uint32_t>()(numSlots)
    );
    for (auto i = 0; i < numSlots; i++) {
      hash = folly::hash::hash_combine(hash, slots[i]);
    }
    return hash;
  }

  bool equals(const NewBespokeStructData& o) const {
    if (layout != o.layout) return false;
    if (offset != o.offset) return false;
    if (numSlots != o.numSlots) return false;
    for (auto i = 0; i < numSlots; i++) {
      if (slots[i] != o.slots[i]) return false;
    }
    return true;
  }

  ArrayLayout layout;
  IRSPRelOffset offset;
  uint32_t numSlots;
  Slot* slots;
};

struct InitStructPositionsData : IRExtraData {
  InitStructPositionsData(ArrayLayout layout, uint32_t numSlots, Slot* slots)
    : layout(layout), numSlots(numSlots), slots(slots) {}

  std::string show() const;

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      std::hash<uint16_t>()(layout.toUint16()),
      std::hash<uint32_t>()(numSlots)
    );
    for (auto i = 0; i < numSlots; i++) {
      hash = folly::hash::hash_combine(hash, slots[i]);
    }
    return hash;
  }

  bool equals(const InitStructPositionsData& o) const {
    if (layout != o.layout) return false;
    if (numSlots != o.numSlots) return false;
    for (auto i = 0; i < numSlots; i++) {
      if (slots[i] != o.slots[i]) return false;
    }
    return true;
  }

  ArrayLayout layout;
  uint32_t numSlots;
  Slot* slots;
};

struct PackedArrayData : IRExtraData {
  explicit PackedArrayData(uint32_t size) : size(size) {}
  std::string show() const { return folly::format("{}", size).str(); }

  size_t stableHash() const {
    return std::hash<uint32_t>()(size);
  }

  bool equals(const PackedArrayData& o) const {
    return size == o.size;
  }

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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<uint32_t>()(size)
    );
  }

  bool equals(const InitPackedArrayLoopData& o) const {
    return offset == o.offset && size == o.size;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<uint32_t>()(first),
      std::hash<uint32_t>()(count)
    );
  }

  bool equals(const CreateAAWHData& o) const {
    return first == o.first && count == o.count;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<uint32_t>()(first),
      std::hash<uint32_t>()(count)
    );
  }

  bool equals(const CountWHNotDoneData& o) const {
    return first == o.first && count == o.count;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<uint32_t>()(size)
    );
  }

  bool equals(const NewKeysetArrayData& o) const {
    return offset == o.offset && size == o.size;
  }

  IRSPRelOffset offset;
  uint32_t size;
};

struct MemoValueStaticData : IRExtraData {
  explicit MemoValueStaticData(const Func* func,
                               Optional<bool> asyncEager,
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      func->stableHash(),
      std::hash<Optional<bool>>()(asyncEager),
      std::hash<bool>()(loadAux)
    );
  }

  bool equals(const MemoValueStaticData& o) const {
    return func == o.func && asyncEager == o.asyncEager && loadAux == o.loadAux;
  }

  const Func* func;
  Optional<bool> asyncEager;
  bool loadAux;
};

struct MemoValueInstanceData : IRExtraData {
  explicit MemoValueInstanceData(Slot slot,
                                 const Func* func,
                                 Optional<bool> asyncEager,
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<Slot>()(slot),
      func->stableHash(),
      std::hash<Optional<bool>>()(asyncEager),
      std::hash<bool>()(loadAux)
    );
  }

  bool equals(const MemoValueInstanceData& o) const {
    return slot == o.slot && func == o.func &&
           asyncEager == o.asyncEager && loadAux == o.loadAux;
  }

  Slot slot;
  const Func* func;
  Optional<bool> asyncEager;
  bool loadAux;
};

struct MemoCacheStaticData : IRExtraData {
  MemoCacheStaticData(const Func* func,
                      LocalRange keys,
                      const bool* types,
                      Optional<bool> asyncEager,
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
    return p;
  }

  std::string show() const {
    std::string ret;
    ret += folly::sformat("{},{}", func->fullName(), HPHP::show(keys));
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

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      func->stableHash(),
      std::hash<Optional<bool>>()(asyncEager),
      std::hash<uint32_t>()(keys.first),
      std::hash<uint32_t>()(keys.count),
      std::hash<bool>()(loadAux)
    );
    for (auto i = 0; i < keys.count; i++) {
      hash = folly::hash::hash_combine(
        hash,
        std::hash<bool>()(types[i])
      );
    }
    return hash;
  }

  bool equals(const MemoCacheStaticData& o) const {
    if (func == o.func && asyncEager == o.asyncEager && loadAux == o.loadAux &&
        keys.first == o.keys.first && keys.count == o.keys.count) {
      for (auto i = 0; i < keys.count; i++) {
        if (types[i] != o.types[i]) return false;
      }
      return true;
    }
    return false;
  }

  const Func* func;
  LocalRange keys;
  const bool* types;
  Optional<bool> asyncEager;
  bool loadAux;
};

struct MemoCacheInstanceData : IRExtraData {
  MemoCacheInstanceData(Slot slot,
                        LocalRange keys,
                        const bool* types,
                        const Func* func,
                        bool shared,
                        Optional<bool> asyncEager,
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
    return p;
  }

  std::string show() const {
    return folly::sformat(
      "{},{},{}<{}>,{}",
      slot,
      func->fullName(),
      HPHP::show(keys),
      [&]{
        using namespace folly::gen;
        return range<uint32_t>(0, keys.count)
          | map([this] (uint32_t i) { return types[i] ? "string" : "int"; })
          | unsplit<std::string>(",");
      }(),
      shared ? "shared" : "non-shared"
    );
  }

  size_t stableHash() const {
    auto hash = folly::hash::hash_combine(
      std::hash<Slot>()(slot),
      func->stableHash(),
      std::hash<Optional<bool>>()(asyncEager),
      std::hash<uint32_t>()(keys.first),
      std::hash<uint32_t>()(keys.count),
      std::hash<bool>()(loadAux),
      std::hash<bool>()(shared)
    );
    for (auto i = 0; i < keys.count; i++) {
      hash = folly::hash::hash_combine(
        hash,
        std::hash<bool>()(types[i])
      );
    }
    return hash;
  }

  bool equals(const MemoCacheInstanceData& o) const {
    if (slot == o.slot && func == o.func && asyncEager == o.asyncEager &&
        loadAux == o.loadAux && keys.first == o.keys.first &&
        keys.count == o.keys.count && shared == o.shared) {
      for (auto i = 0; i < keys.count; i++) {
        if (types[i] != o.types[i]) return false;
      }
      return true;
    }
    return false;
  }

  Slot slot;
  LocalRange keys;
  const bool* types;
  const Func* func;
  bool shared;
  Optional<bool> asyncEager;
  bool loadAux;
};

struct MOpModeData : IRExtraData {
  explicit MOpModeData(MOpMode mode) : mode{mode} {}

  std::string show() const { return subopToName(mode); }

  size_t stableHash() const {
    return std::hash<MOpMode>()(mode);
  }

  bool equals(const MOpModeData& o) const {
    return mode == o.mode;
  }

  MOpMode mode;
};

struct PropData : IRExtraData {
  explicit PropData(MOpMode mode, ReadOnlyOp op) : mode{mode}, op(op) {}

  std::string show() const {
    return fmt::format("{} {}", subopToName(mode), subopToName(op));
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<MOpMode>()(mode),
      std::hash<ReadOnlyOp>()(op)
    );
  }

  bool equals(const PropData& o) const {
    return mode == o.mode && op == o.op;
  }

  MOpMode mode;
  ReadOnlyOp op;
};

struct ReadOnlyData : IRExtraData {
  explicit ReadOnlyData(ReadOnlyOp op) : op(op) {}
  std::string show() const { return subopToName(op); }

  size_t stableHash() const {
    return std::hash<ReadOnlyOp>()(op);
  }

  bool equals(const ReadOnlyData& o) const {
    return op == o.op;
  }

  ReadOnlyOp op;
};

struct SetOpData : IRExtraData {
  explicit SetOpData(SetOpOp op) : op(op) {}
  std::string show() const { return subopToName(op); }

  size_t stableHash() const {
    return std::hash<SetOpOp>()(op);
  }

  bool equals(const SetOpData& o) const {
    return op == o.op;
  }

  SetOpOp op;
};

struct DecRefData : IRExtraData {
  explicit DecRefData(int locId = -1) : locId(locId) {}
  std::string show() const {
    return locId != -1 ? folly::to<std::string>("Loc", locId) : "-";
  }

  size_t stableHash() const {
    return std::hash<int>()(locId);
  }

  bool equals(const DecRefData& o) const {
    return locId == o.locId;
  }

  int locId; // If a known local, this has its id; -1 otherwise.
};

struct IncDecData : IRExtraData {
  explicit IncDecData(IncDecOp op) : op(op) {}
  std::string show() const { return subopToName(op); }

  size_t stableHash() const {
    return std::hash<IncDecOp>()(op);
  }

  bool equals(const IncDecData& o) const {
    return op == o.op;
  }

  IncDecOp op;
};

struct SuspendOffset : IRExtraData {
  explicit SuspendOffset(Offset off) : off(off) {}
  std::string show() const { return folly::to<std::string>(off); }

  size_t stableHash() const {
    return std::hash<Offset>()(off);
  }

  bool equals(const SuspendOffset& o) const {
    return off == o.off;
  }

  Offset off;
};

struct GeneratorState : IRExtraData {
  explicit GeneratorState(BaseGenerator::State state) : state(state) {}
  std::string show() const {
    using U = std::underlying_type<BaseGenerator::State>::type;
    return folly::to<std::string>(static_cast<U>(state));
  }

  size_t stableHash() const {
    return std::hash<BaseGenerator::State>()(state);
  }

  bool equals(const GeneratorState& o) const {
    return state == o.state;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(spOffset.offset),
      std::hash<Offset>()(callBCOffset),
      std::hash<bool>()(isAsync)
    );
  }

  bool equals(const ContEnterData& o) const {
    return spOffset == o.spOffset && callBCOffset == o.callBCOffset &&
           isAsync && o.isAsync;
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

  size_t stableHash() const {
    return std::hash<CollectionType>()(type);
  }

  bool equals(const NewColData& o) const {
    return type == o.type;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<uint32_t>()(start),
      std::hash<uint32_t>()(end)
    );
  }

  bool equals(const LocalIdRange& o) const {
    return start == o.start && end == o.end;
  }

  uint32_t start, end;
};

struct StackRange : IRExtraData {
  StackRange(IRSPRelOffset start, uint32_t count)
    : start(start)
    , count(count)
  {}

  std::string show() const {
    return folly::sformat("[{}, {})", start.offset, start.offset + count);
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(std::hash<int32_t>()(start.offset),
                                     std::hash<int32_t>()(count));
  }

  bool equals(const StackRange& o) const {
    return start == o.start && count == o.count;
  }

  IRSPRelOffset start;
  uint32_t count;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      func->stableHash(),
      std::hash<uint32_t>()(argc)
    );
  }

  bool equals(const FuncEntryData& o) const {
    return func == o.func && argc == o.argc;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int>()(firstBit),
      std::hash<uint64_t>()(mask),
      std::hash<uint64_t>()(vals)
    );
  }

  bool equals(const CheckInOutsData& o) const {
    return firstBit == o.firstBit && mask == o.mask && vals == o.vals;
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

  size_t stableHash() const {
    auto const sym = rds::reverseLink(handle);
    if (!sym) return 0;
    return rds::symbol_stable_hash(*sym);
  }

  bool equals(const ProfileCallTargetData& o) const {
    return handle == o.handle;
  }

  rds::Handle handle;
};

struct BeginInliningData : IRExtraData {
  BeginInliningData(IRSPRelOffset offset, const Func* func, int cost, int na)
    : spOffset(offset)
    , func(func)
    , cost(cost)
    , numArgs(na)
  {}

  std::string show() const {
    return folly::to<std::string>("IRSPOff ", spOffset.offset,
                                  " FUNC ", func->fullName()->data());
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(spOffset.offset),
      func->stableHash(),
      std::hash<int>()(cost),
      std::hash<int>()(numArgs)
    );
  }

  bool equals(const BeginInliningData& o) const {
    return spOffset == o.spOffset && func == o.func && cost == o.cost &&
           numArgs == o.numArgs;
  }

  IRSPRelOffset spOffset;
  const Func* func;
  int cost;
  int numArgs;
};

struct ParamData : IRExtraData {
  explicit ParamData(int32_t paramId) : paramId(paramId) {}

  std::string show() const {
    return folly::to<std::string>(paramId);
  }

  size_t stableHash() const { return std::hash<int32_t>()(paramId); }
  bool equals(const ParamData& o) const {
    return paramId == o.paramId;
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

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(paramId),
      std::hash<std::string>()(tc->fullName())  // Not great but hey its easy.
    );
  }

  bool equals(const ParamWithTCData& o) const {
    return paramId == o.paramId &&
           *tc == *o.tc;
  }

  int32_t paramId;
  const TypeConstraint* tc;
};

struct TypeConstraintData : IRExtraData {
  explicit TypeConstraintData(const TypeConstraint* tc)
    : tc(tc) {}

  std::string show() const { return tc->displayName(); }

  size_t stableHash() const {
    // Not great but easy.
    return std::hash<std::string>()(tc->fullName());
  }

  bool equals(const TypeConstraintData& o) const {
    return *tc == *o.tc;
  }

  const TypeConstraint* tc;
};

struct RaiseClsMethPropConvertNoticeData : IRExtraData {
  RaiseClsMethPropConvertNoticeData(const TypeConstraint* tc, bool isSProp)
    : tc(tc)
    , isSProp(isSProp)
  {}

  std::string show() const {
    return folly::to<std::string>(tc->displayName(), ",", isSProp);
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<bool>()(isSProp),
      std::hash<std::string>()(tc->fullName())  // Not great but hey its easy.
    );
  }

  bool equals(const RaiseClsMethPropConvertNoticeData& o) const {
    return isSProp == o.isSProp &&
           *tc == *o.tc;
  }

  union { const TypeConstraint* tc; int64_t tcIntVal; };
  bool isSProp;
};

struct ArrayGetExceptionData : IRExtraData {
  explicit ArrayGetExceptionData(bool isInOut) : isInOut(isInOut) {}

  std::string show() const {
    return isInOut ? "inout" : "none";
  }

  size_t stableHash() const {
    return std::hash<bool>()(isInOut);
  }

  bool equals(const ArrayGetExceptionData& o) const {
    return isInOut == o.isInOut;
  }

  bool isInOut;
};

struct AssertReason : IRExtraData {
  explicit AssertReason(Reason r) : reason{r.file, r.line} {}

  std::string show() const {
    return jit::show(reason);
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<const char*>()(reason.file),
      std::hash<unsigned>()(reason.line)
    );
  }

  bool equals(const AssertReason& o) const {
    return reason == o.reason;
  }

  Reason reason;
};

#define ASSERT_REASON AssertReason{Reason{__FILE__, __LINE__}}

struct EndCatchData : IRExtraData {
  enum class CatchMode { UnwindOnly, CallCatch, SideExit, LocalsDecRefd };
  enum class FrameMode { Phplogue, Stublogue };
  enum class Teardown  { NA, None, Full, OnlyThis };

  explicit EndCatchData(IRSPRelOffset offset, CatchMode mode,
                        FrameMode stublogue, Teardown teardown)
    : offset{offset}
    , mode{mode}
    , stublogue{stublogue}
    , teardown{teardown}
    {}

  std::string show() const {
    return folly::to<std::string>(
      "IRSPOff ", offset.offset, ",",
      mode == CatchMode::UnwindOnly ? "UnwindOnly" :
        mode == CatchMode::CallCatch ? "CallCatch" :
          mode == CatchMode::SideExit ? "SideExit" : "LocalsDecRefd", ",",
      stublogue == FrameMode::Stublogue ? "Stublogue" : "Phplogue", ",",
      teardown == Teardown::NA ? "NA" :
        teardown == Teardown::None ? "None" :
          teardown == Teardown::Full ? "Full" : "OnlyThis");
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<CatchMode>()(mode),
      std::hash<FrameMode>()(stublogue),
      std::hash<Teardown>()(teardown)
    );
  }

  bool equals(const EndCatchData& o) const {
    return offset == o.offset && mode == o.mode && stublogue == o.stublogue &&
           teardown == o.teardown;
  }

  IRSPRelOffset offset;
  CatchMode mode;
  FrameMode stublogue;
  Teardown teardown;
};

struct EnterTCUnwindData : IRExtraData {
  explicit EnterTCUnwindData(IRSPRelOffset offset, bool teardown)
    : offset{offset}, teardown{teardown} {}

  std::string show() const {
    return folly::to<std::string>(
      "IRSPOff ", offset.offset, ",",
      teardown ? "" : "no-", "teardown"
    );
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<int32_t>()(offset.offset),
      std::hash<bool>()(teardown)
    );
  }

  bool equals(const EnterTCUnwindData& o) const {
    return offset == o.offset && teardown == o.teardown;
  }

  IRSPRelOffset offset;
  bool teardown;
};

/*
 * Func/Class/Prop attributes
 */
struct AttrData : IRExtraData {
  explicit AttrData(Attr attr) : attr(static_cast<int32_t>(attr)) {}

  std::string show() const {
    return folly::format("{}", attr).str();
  }

  size_t stableHash() const {
    return std::hash<int32_t>()(attr);
  }

  size_t hash() const {
    return std::hash<int32_t>()(attr);
  }

  bool equals(const AttrData& o) const {
    return attr == o.attr;
  }

  int32_t attr;
};

struct MethCallerData : IRExtraData {
  explicit MethCallerData(bool isCls) : isCls(isCls) {}
  std::string show() const {
    return folly::format("{}", isCls).str();
  }

  size_t stableHash() const {
    return std::hash<bool>()(isCls);
  }

  bool equals(const MethCallerData& o) const {
    return isCls == o.isCls;
  }

  bool isCls;
};

struct LoggingProfileData : IRExtraData {
  LoggingProfileData(bespoke::LoggingProfile* profile, bool isStatic)
    : profile(profile)
    , isStatic(isStatic)
  {}

  std::string show() const {
    // profile->source is already printed in the instruction's marker.
    return folly::sformat("{}", reinterpret_cast<void*>(profile));
  }

  size_t stableHash() const;

  bool equals(const LoggingProfileData& o) const {
    return profile == o.profile;
  }

  bespoke::LoggingProfile* profile;
  bool isStatic; // Whether the output is guaranteed to be static
};

struct SinkProfileData : IRExtraData {
  explicit SinkProfileData(bespoke::SinkProfile* profile)
    : profile(profile)
  {}

  std::string show() const {
    // profile->sink is already printed in the instruction's marker.
    return folly::sformat("{}", reinterpret_cast<void*>(profile));
  }

  size_t stableHash() const;

  bool equals(const SinkProfileData& o) const {
    return profile == o.profile;
  }

  bespoke::SinkProfile* profile;
};

struct BespokeGetData : IRExtraData {
  enum class KeyState { Present, Unknown };

  explicit BespokeGetData(KeyState state) : state(state) {}

  std::string show() const {
    switch (state) {
      case KeyState::Present: return "Present";
      case KeyState::Unknown: return "Unknown";
    }
    always_assert(false);
  }

  size_t hash() const {
    return stableHash();
  }

  size_t stableHash() const {
    return std::hash<KeyState>()(state);
  }

  bool equals(const BespokeGetData& o) const {
    return state == o.state;
  }

  KeyState state;
};

struct ConvNoticeData : IRExtraData {
  explicit ConvNoticeData(ConvNoticeLevel l = ConvNoticeLevel::None,
                          const StringData* r = nullptr,
                          bool noticeWithinNum_ = true)
                          : level(l), reason(r), noticeWithinNum(noticeWithinNum_)  {}
  std::string show() const {
    assertx(level == ConvNoticeLevel::None || (reason != nullptr && reason->isStatic()));
    const auto reason_str = reason ? folly::format(" for {}", reason).str() : "";
    const auto num_str = !noticeWithinNum ? " with no intra-num notices": "";
    return folly::format("{}{}{}", convOpToName(level), reason_str, num_str).str();
  }

  size_t stableHash() const {
    return folly::hash::hash_combine(
      std::hash<ConvNoticeLevel>()(level),
      std::hash<const StringData*>()(reason),
      std::hash<bool>()(noticeWithinNum)
    );
  }

  bool equals(const ConvNoticeData& o) const {
    // can use pointer equality bc reason is always a StaticString
    return level == o.level && reason == o.reason && noticeWithinNum == o.noticeWithinNum;
  }

  ConvNoticeLevel level;
  union { const StringData* reason; int64_t reasonIntVal; };
  // whether to trigger notices for conversions between int and double
  bool noticeWithinNum = true;
};

struct BadComparisonData : IRExtraData {
  explicit BadComparisonData(bool eqOp) : eq(eqOp)  {}
  std::string show() const { return eq ? "For Eq" : "For Cmp"; }
  size_t stableHash() const { return std::hash<bool>()(eq); }
  bool equals(const BadComparisonData& o) const { return eq == o.eq; }
  bool eq;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

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
X(LdClsInitElem,                IndexData);
X(StClsInitElem,                IndexData);
X(StLoc,                        LocalId);
X(StLocRange,                   LocalIdRange);
X(AdvanceDictPtrIter,           IterOffsetData);
X(StFrameFunc,                  FuncData);
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
X(StStkRange,                   StackRange);
X(StOutValue,                   IndexData);
X(LdOutAddr,                    IndexData);
X(AssertStk,                    IRSPRelOffsetData);
X(DefFP,                        DefFPData);
X(DefFrameRelSP,                DefStackData);
X(DefRegSP,                     DefStackData);
X(LdStk,                        IRSPRelOffsetData);
X(LdStkAddr,                    IRSPRelOffsetData);
X(InlineCall,                   InlineCallData);
X(StFrameMeta,                  StFrameMetaData);
X(BeginInlining,                BeginInliningData);
X(ReqBindJmp,                   ReqBindJmpData);
X(ReqInterpBBNoTranslate,       ReqBindJmpData);
X(ReqRetranslate,               IRSPRelOffsetData);
X(ReqRetranslateOpt,            IRSPRelOffsetData);
X(CheckCold,                    TransIDData);
X(IncProfCounter,               TransIDData);
X(LogArrayReach,                SinkProfileData);
X(NewLoggingArray,              LoggingProfileData);
X(BespokeGet,                   BespokeGetData);
X(DefFuncEntryFP,               FuncData);
X(Call,                         CallData);
X(CallBuiltin,                  CallBuiltinData);
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
X(InitSubClsCns,                LdSubClsCnsData);
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
X(RaiseCoeffectsCallViolation,  FuncData);
X(RaiseCoeffectsFunParamTypeViolation, ParamData);
X(ThrowParamInOutMismatch,      ParamData);
X(ThrowParamInOutMismatchRange, CheckInOutsData);
X(ThrowParameterWrongType,      FuncArgTypeData);
X(CheckClsReifiedGenericMismatch,
                                ClassData);
X(IsFunReifiedGenericsMatched,  FuncData);
X(IsTypeStruct,                 RDSHandleData);
X(InterpOne,                    InterpOneData);
X(InterpOneCF,                  InterpOneData);
X(StClosureArg,                 IndexData);
X(RBTraceEntry,                 RBEntryData);
X(RBTraceMsg,                   RBMsgData);
X(OODeclExists,                 ClassKindData);
X(NewStructDict,                NewStructData);
X(NewRecord,                    NewStructData);
X(AllocStructDict,              NewStructData);
X(AllocBespokeStructDict,       ArrayLayoutData);
X(InitStructPositions,          InitStructPositionsData);
X(NewBespokeStructDict,         NewBespokeStructData);
X(AllocVec,                     PackedArrayData);
X(NewKeysetArray,               NewKeysetArrayData);
X(InitVecElemLoop,              InitPackedArrayLoopData);
X(InitVecElem,                  IndexData);
X(InitDictElem,                 KeyedIndexData);
X(InitStructElem,               KeyedIndexData);
X(CreateAAWH,                   CreateAAWHData);
X(CountWHNotDone,               CountWHNotDoneData);
X(CheckDictOffset,              IndexData);
X(CheckKeysetOffset,            IndexData);
X(ProfileDictAccess,            ArrayAccessProfileData);
X(ProfileKeysetAccess,          ArrayAccessProfileData);
X(ProfileType,                  RDSHandleData);
X(ProfileCall,                  ProfileCallTargetData);
X(ProfileMethod,                ProfileCallTargetData);
X(ProfileIsTypeStruct,          RDSHandleData);
X(LdRDSAddr,                    RDSHandleData);
X(CheckRDSInitialized,          RDSHandleData);
X(MarkRDSInitialized,           RDSHandleData);
X(MarkRDSAccess,                RDSHandleData);
X(LdInitRDSAddr,                RDSHandleData);
X(BaseG,                        MOpModeData);
X(PropX,                        PropData);
X(PropQ,                        ReadOnlyData);
X(PropDX,                       PropData);
X(ElemX,                        MOpModeData);
X(ElemDX,                       MOpModeData);
X(ElemUX,                       MOpModeData);
X(CGetProp,                     PropData);
X(CGetPropQ,                    ReadOnlyData);
X(CGetElem,                     MOpModeData);
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
X(SetProp,                      ReadOnlyData);
X(OutlineSetOp,                 SetOpData);
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
X(VerifyPropCls,                TypeConstraintData);
X(VerifyPropRecDesc,            TypeConstraintData);
X(VerifyPropFail,               TypeConstraintData);
X(VerifyPropFailHard,           TypeConstraintData);
X(VerifyProp,                   TypeConstraintData);
X(VerifyPropCoerce,             TypeConstraintData);
X(EndCatch,                     EndCatchData);
X(EnterTCUnwind,                EnterTCUnwindData);
X(FuncHasAttr,                  AttrData);
X(ClassHasAttr,                 AttrData);
X(LdMethCallerName,             MethCallerData);
X(LdRecDescCached,              RecNameData);
X(LdRecDescCachedSafe,          RecNameData);
X(LdUnitPerRequestFilepath,     RDSHandleData);
X(LdClsTypeCns,                 LdClsTypeCnsData);
X(ConvTVToStr,                  ConvNoticeData);
X(ConvTVToInt,                  ConvNoticeData);
X(ConvObjToInt,                 ConvNoticeData);
X(CheckFuncNeedsCoverage,       FuncData);
X(RecordFuncCall,               FuncData);
X(RaiseBadComparisonViolation,  BadComparisonData);

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
size_t stableHashExtra(Opcode opc, const IRExtraData* data);
bool equalsExtra(Opcode opc, const IRExtraData* a, const IRExtraData* b);
IRExtraData* cloneExtra(Opcode opc, IRExtraData* data, Arena& a);
std::string showExtra(Opcode opc, const IRExtraData* data);

//////////////////////////////////////////////////////////////////////

}}
