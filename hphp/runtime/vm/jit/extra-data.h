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

#include "hphp/runtime/vm/jit/ir.h"

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

struct IterId : IRExtraData {
  explicit IterId(uint32_t id)
    : iterId(id)
  {}

  bool cseEquals(IterId o) const { return iterId == o.iterId; }
  size_t cseHash() const { return std::hash<uint32_t>()(iterId); }
  std::string show() const { return folly::to<std::string>(iterId); }

  uint32_t iterId;
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
 * DefInlineFP is present when we need to create a frame for inlining.
 * This instruction also carries some metadata used by tracebuilder to
 * track state during an inlined call.
 */
struct DefInlineFPData : IRExtraData {
  std::string show() const {
    return folly::to<std::string>(
      target->fullName()->data(), "(),", retSPOff, ',', retBCOff
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
  explicit CallArrayData(Offset pcOffset, Offset aft)
    : pc(pcOffset), after(aft) {}

  std::string show() const { return folly::to<std::string>(pc, ",", after); }

  Offset pc, after;
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
};

/*
 * Information for creating continuation objects.
 * CreateCont{Func,Meth}.
 */
struct CreateContData : IRExtraData {
  CreateContData(const Func* origFunc, const Func* genFunc)
    : origFunc(origFunc)
    , genFunc(genFunc)
  {}

  std::string show() const {
    return folly::to<std::string>(origFunc->fullName()->data(), "()");
  }

  const Func* origFunc;
  const Func* genFunc;
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
X(RaiseUninitLoc,               LocalId);
X(GuardLoc,                     LocalId);
X(CheckLoc,                     LocalId);
X(AssertLoc,                    LocalId);
X(OverrideLoc,                  LocalId);
X(LdLocAddr,                    LocalId);
X(DecRefLoc,                    LocalId);
X(LdLoc,                        LocalId);
X(StLoc,                        LocalId);
X(StLocNT,                      LocalId);
X(GuardIter,                    IterId);
X(IterFree,                     IterId);
X(MIterFree,                    IterId);
X(CIterFree,                    IterId);
X(DecodeCufIter,                IterId);
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
X(ReDefSP,                      StackOffset);
X(ReDefGeneratorSP,             StackOffset);
X(DefSP,                        StackOffset);
X(LdStack,                      StackOffset);
X(LdStackAddr,                  StackOffset);
X(DecRefStack,                  StackOffset);
X(DefInlineFP,                  DefInlineFPData);
X(ReqBindJmp,                   BCOffset);
X(ReqBindJmpNoIR,               BCOffset);
X(ReqRetranslateNoIR,           BCOffset);
X(CallArray,                    CallArrayData);
X(LdClsCns,                     ClsCnsName);
X(LookupClsCns,                 ClsCnsName);
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
X(SideExitGuardLoc,             SideExitGuardData);
X(SideExitGuardStk,             SideExitGuardData);
X(CheckDefinedClsEq,            CheckDefinedClsData);
X(InterpOne,                    InterpOneData);
X(CreateContFunc,               CreateContData);
X(CreateContMeth,               CreateContData);

#undef X

//////////////////////////////////////////////////////////////////////

template<bool hasExtra, Opcode opc, class T> struct AssertExtraTypes {
  static void doassert() {
    assert(!"called extra on an opcode without extra data");
  }
};

template<Opcode opc, class T> struct AssertExtraTypes<true,opc,T> {
  static void doassert() {
    typedef typename IRExtraDataType<opc>::type ExtraType;
    if (!std::is_same<ExtraType,T>::value) {
      assert(!"extra<T> was called with an extra data "
              "type that doesn't match the opcode type");
    }
  }
};

// Asserts that Opcode opc has extradata and it is of type T.
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

std::string showExtra(Opcode opc, const IRExtraData* data);

//////////////////////////////////////////////////////////////////////

}}

#endif
