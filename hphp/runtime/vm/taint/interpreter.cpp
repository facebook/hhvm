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

#ifdef HHVM_TAINT

#include <sstream>

#include <folly/Singleton.h>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/taint/configuration.h"
#include "hphp/runtime/vm/taint/interpreter.h"
#include "hphp/runtime/vm/taint/state.h"

#include "hphp/util/text-color.h"
#include "hphp/util/trace.h"

namespace folly {

template <>
class FormatValue<HPHP::taint::Value> {
 public:
  explicit FormatValue(const HPHP::taint::Value& value) : m_value(value) {}
  template <class FormatCallback>
  void format(FormatArg& arg, FormatCallback& cb) const {
    auto value = m_value ? "S" : "_";
    FormatValue<std::string>(value).format(arg, cb);
  }

 private:
  const HPHP::taint::Value& m_value;
};

template <>
class FormatValue<HPHP::tv_lval> {
 public:
  explicit FormatValue(const HPHP::tv_lval& value) : m_value(value) {}
  template <class FormatCallback>
  void format(FormatArg& arg, FormatCallback& cb) const {
    if (!m_value.is_set()) {
      FormatValue<std::string>("<nullptr>").format(arg, cb);
      return;
    }
    auto string = folly::sformat("<{}>", uintptr_t(&m_value.val()));
    format_value::formatString(string, arg, cb);
  }

 private:
  const HPHP::tv_lval& m_value;
};

template <>
class FormatValue<HPHP::MemberKey> {
 public:
  explicit FormatValue(const HPHP::MemberKey& memberKey) : m_value(memberKey) {}
  template <class FormatCallback>
  void format(FormatArg& arg, FormatCallback& cb) const {
    auto string = folly::sformat("{}", HPHP::show(m_value));
    format_value::formatString(string, arg, cb);
  }

 private:
  const HPHP::MemberKey& m_value;
};

} // namespace folly

namespace HPHP {
namespace taint {

TRACE_SET_MOD(taint);

namespace {

std::string quote(const std::string& string) {
  return folly::sformat("`{}`", string);
}

std::string yellow(const std::string& string) {
  return folly::sformat("{}{}{}", ANSI_COLOR_YELLOW, string, ANSI_COLOR_END);
}

std::string gray(const std::string& string) {
  return folly::sformat("{}{}{}", ANSI_COLOR_GRAY, string, ANSI_COLOR_END);
}

void iopPreamble(folly::StringPiece name) {
  auto vm_stack_size = vmStack().count();

  FTRACE(1, "taint: {}\n", gray(folly::sformat("iop{}", name)));
  FTRACE(4, "taint: stack: {}\n", State::instance->stack.show());

  auto& stack = State::instance->stack;
  auto shadow_stack_size = stack.size();
  if (vm_stack_size != shadow_stack_size) {
    FTRACE(
        3,
        "taint: (WARNING) stacks out of sync "
        "(stack size: {}, shadow stack size: {}). Adjusting...\n",
        vm_stack_size,
        shadow_stack_size);
    for (int i = shadow_stack_size; i < vm_stack_size; i++) {
      stack.pushFront(nullptr);
    }
    for (int i = shadow_stack_size; i > vm_stack_size; i--) {
      stack.popFront();
    }
  }
}

void iopConstant(folly::StringPiece name) {
  iopPreamble(name);
  State::instance->stack.push(nullptr);
}

void iopUnhandled(folly::StringPiece name) {
  iopPreamble(name);
  FTRACE(1, "taint: (WARNING) unhandled opcode\n");
}

// Marker for opcodes that do not affect taint. For example
// they may pop one element and push one back that keeps the same taint.
void iopDoesNotAffectTaint(folly::StringPiece name) {
  iopPreamble(name);
}

const Func* callee() {
  auto sfp = vmfp()->sfp();
  if (sfp != nullptr) {
    return sfp->func();
  }
  return nullptr;
}

} // namespace

/**
 * Opcode implementations are below.
 *
 * This attempts to model the spec at
 * https://github.com/facebook/hhvm/blob/master/hphp/doc/bytecode.specification
 *
 * Most opcodes are still unhandled for now and marked as such using
 * a call to `iopUnhandled`.
 *
 * We are currently going through all the opcodes and implementing them,
 * following the following convention:
 *
 * 1) If an iop needs special handling to track taint, it will be implemented as
 * such. 2) If an iop does not need any handling, it will just have a call to
 * `iopPreamble` and (hopefully) comment explaining why it's OK to ignore. 3) If
 * hasn't been looked at yet, it will be marked with `iopUnhandled`.
 *
 * Rough indications of current status:
 *
 * ~some basic opcodes are implemented, enough to trace values through some
 * function calls
 *
 * The following sections are completely implemented:
 *
 * 1) Basic instructions
 * 2) Literal and constant instructions
 * 5) Get instructions (except CGetG for globals and CGetS for class properties)
 * 6) Isset and type querying instructions
 */

void iopNop() {
  iopPreamble("Nop");
}

void iopEntryNop() {
  iopPreamble("EntryNop");
}

void iopBreakTraceHint() {
  iopDoesNotAffectTaint("BreakTraceHint");
}

void iopPopC() {
  iopPreamble("PopC");
  State::instance->stack.pop();
}

void iopPopU() {
  iopPreamble("PopU");
  State::instance->stack.pop();
}

void iopPopU2() {
  iopPreamble("PopU2");
  auto& stack = State::instance->stack;
  auto saved = stack.top();
  stack.pop();
  stack.push(saved);
}

void iopPopL(tv_lval to) {
  iopPreamble("PopL");

  // PopL behaves like a SetL PopC pair
  auto state = State::instance;
  auto value = state->stack.top();

  FTRACE(2, "taint: setting {} to `{}`\n", to, value);

  state->heap_locals.set(std::move(to), value);
  state->stack.pop();
}

void iopDup() {
  iopPreamble("Dup");
  auto& stack = State::instance->stack;
  stack.push(stack.top());
}

void iopCGetCUNop() {
  iopPreamble("CGetCUNop");
  // This is a no op
}

void iopUGetCUNop() {
  iopPreamble("UGetCUNop");
  // This is a no op
}

void iopNull() {
  iopConstant("Null");
}

void iopNullUninit() {
  iopConstant("NullUninit");
}

void iopTrue() {
  iopConstant("True");
}

void iopFalse() {
  iopConstant("False");
}

void iopFuncCred() {
  iopPreamble("FuncCred");
  State::instance->stack.push(nullptr);
}

void iopInt(int64_t /* imm */) {
  iopConstant("Int");
}

void iopDouble(double /* imm */) {
  iopConstant("Double");
}

void iopString(const StringData* /* s */) {
  iopConstant("String");
}

namespace {
void iopNewEmptyCollection(folly::StringPiece name) {
  iopPreamble(name);
  auto& mstate = vmMInstrState();
  auto to = mstate.base;
  State::instance->heap_collections.set(to, nullptr);
}

void newCollectionSizeN(uint32_t n) {
  auto state = State::instance;
  auto& stack = state->stack;

  // Check all arguments to see if they're tainted.
  // One tainted element taints result.
  Value value = nullptr;
  for (int i = 0; i < n; i++) {
    auto argument = stack.peek(i);
    if (argument) {
      value = argument;
      break;
    }
  }

  stack.pop(n);
  stack.push(value);

  auto& mstate = vmMInstrState();
  auto to = mstate.base;
  state->heap_collections.set(to, value);
}
} // namespace

void iopDict(const ArrayData* /* a */) {
  iopNewEmptyCollection("Dict");
}

void iopKeyset(const ArrayData* /* a */) {
  iopNewEmptyCollection("Keyset");
}

void iopVec(const ArrayData* /* a */) {
  iopNewEmptyCollection("Vec");
}

void iopNewDictArray(uint32_t /* capacity */) {
  iopNewEmptyCollection("NewDictArray");
}

void iopNewStructDict(imm_array<int32_t> ids) {
  iopPreamble("NewStructDict");

  // We taint the whole struct if one value is tainted. This could eventually
  // be a join operation.
  newCollectionSizeN(ids.size);

  FTRACE(2, "taint: new struct is `{}`\n", State::instance->stack.top());
}

void iopNewVec(uint32_t n) {
  iopPreamble("NewVec");
  newCollectionSizeN(n);
}

void iopNewKeysetArray(uint32_t n) {
  iopPreamble("NewKeysetArray");
  newCollectionSizeN(n);
}

void iopAddElemC() {
  iopPreamble("AddElemC");

  // Update the taint on the collection to be that of the new value
  // if it's tainted. This should eventually be a join.
  auto& stack = State::instance->stack;
  auto value = stack.top();
  stack.pop(2);
  if (value == nullptr) {
    value = stack.top();
  }
  stack.replaceTop(value);
}

void iopAddNewElemC() {
  iopPreamble("AddNewElemC");
  // Update the taint on the collection to be that of the new value
  // if it's tainted. This should eventually be a join.
  auto& stack = State::instance->stack;
  auto value = stack.top();
  stack.pop();
  if (value == nullptr) {
    value = stack.top();
  }
  stack.replaceTop(value);
}

void iopNewCol(CollectionType /* cType */) {
  iopPreamble("NewCol");
  State::instance->stack.push(nullptr);
}

void iopNewPair() {
  iopPreamble("NewPair");
  newCollectionSizeN(2);
}

void iopColFromArray(CollectionType /* cType */) {
  iopDoesNotAffectTaint("ColFromArray");
}

void iopCnsE(const StringData* /* s */) {
  iopConstant("CnsE");
}

void iopClsCns(const StringData* /* clsCnsName */) {
  iopDoesNotAffectTaint("ClsCns");
}

void iopClsCnsD(const StringData* /* clsCnsName */, Id /* classId */) {
  iopConstant("ClsCnsD");
}

void iopClsCnsL(tv_lval /* local */) {
  iopDoesNotAffectTaint("ClsCnsL");
}

void iopClassName() {
  iopPreamble("ClassName");
  State::instance->stack.push(nullptr);
}

void iopLazyClassFromClass() {
  iopDoesNotAffectTaint("LazyClassFromClass");
}

void iopFile() {
  iopConstant("File");
}

void iopDir() {
  iopConstant("Dir");
}

void iopMethod() {
  iopConstant("Method");
}

void iopConcat() {
  iopUnhandled("Concat");
}

void iopConcatN(uint32_t /* n */) {
  iopUnhandled("ConcatN");
}

void iopAdd() {
  iopUnhandled("Add");
}

void iopSub() {
  iopUnhandled("Sub");
}

void iopMul() {
  iopUnhandled("Mul");
}

void iopAddO() {
  iopUnhandled("AddO");
}

void iopSubO() {
  iopUnhandled("SubO");
}

void iopMulO() {
  iopUnhandled("MulO");
}

void iopDiv() {
  iopUnhandled("Div");
}

void iopMod() {
  iopUnhandled("Mod");
}

void iopPow() {
  iopUnhandled("Pow");
}

void iopNot() {
  iopUnhandled("Not");
}

void iopSame() {
  iopUnhandled("Same");
}

void iopNSame() {
  iopUnhandled("NSame");
}

void iopEq() {
  iopUnhandled("Eq");
}

void iopNeq() {
  iopUnhandled("Neq");
}

void iopLt() {
  iopUnhandled("Lt");
}

void iopLte() {
  iopUnhandled("Lte");
}

void iopGt() {
  iopUnhandled("Gt");
}

void iopGte() {
  iopUnhandled("Gte");
}

void iopCmp() {
  iopUnhandled("Cmp");
}

void iopBitAnd() {
  iopUnhandled("BitAnd");
}

void iopBitOr() {
  iopUnhandled("BitOr");
}

void iopBitXor() {
  iopUnhandled("BitXor");
}

void iopBitNot() {
  iopUnhandled("BitNot");
}

void iopShl() {
  iopUnhandled("Shl");
}

void iopShr() {
  iopUnhandled("Shr");
}

void iopCastBool() {
  iopUnhandled("CastBool");
}

void iopCastInt() {
  iopUnhandled("CastInt");
}

void iopCastDouble() {
  iopUnhandled("CastDouble");
}

void iopCastString() {
  iopUnhandled("CastString");
}

void iopCastDict() {
  iopUnhandled("CastDict");
}

void iopCastKeyset() {
  iopUnhandled("CastKeyset");
}

void iopCastVec() {
  iopUnhandled("CastVec");
}

void iopDblAsBits() {
  iopUnhandled("DblAsBits");
}

void iopInstanceOf() {
  iopUnhandled("InstanceOf");
}

void iopInstanceOfD(Id /* id */) {
  iopUnhandled("InstanceOfD");
}

void iopIsLateBoundCls() {
  iopUnhandled("IsLateBoundCls");
}

void iopIsTypeStructC(TypeStructResolveOp /* op */) {
  iopUnhandled("IsTypeStructC");
}

void iopThrowAsTypeStructException() {
  iopUnhandled("ThrowAsTypeStructException");
}

void iopCombineAndResolveTypeStruct(uint32_t /* n */) {
  iopUnhandled("CombineAndResolveTypeStruct");
}

void iopSelect() {
  iopUnhandled("Select");
}

void iopPrint() {
  iopUnhandled("Print");
}

void iopClone() {
  iopUnhandled("Clone");
}

void iopExit() {
  iopUnhandled("Exit");
}

void iopFatal(FatalOp /* kind_char */) {
  iopDoesNotAffectTaint("Fatal");
}

void iopJmp(PC& /* pc */, PC /* targetpc */) {
  iopUnhandled("Jmp");
}

void iopJmpNS(PC& /* pc */, PC /* targetpc */) {
  iopUnhandled("JmpNS");
}

void iopJmpZ(PC& /* pc */, PC /* targetpc */) {
  iopUnhandled("JmpZ");
}

void iopJmpNZ(PC& /* pc */, PC /* targetpc */) {
  iopUnhandled("JmpNZ");
}

void iopSwitch(
    PC /* origpc */,
    PC& /* pc */,
    SwitchKind /* kind */,
    int64_t /* base */,
    imm_array<Offset> /* jmptab */) {
  iopUnhandled("Switch");
}

void iopSSwitch(
    PC /* origpc */,
    PC& /* pc */,
    imm_array<StrVecItem> /* jmptab */) {
  iopUnhandled("SSwitch");
}

void iopRetC(PC& /* pc */) {
  iopPreamble("RetC");

  auto state = State::instance;
  auto& stack = state->stack;
  auto saved = stack.top();

  auto func = vmfp()->func();
  stack.pop(2 + func->params().size());

  FTRACE(1, "taint: leaving {}\n", yellow(quote(func->fullName()->data())));

  const auto sources = state->sources(func);
  FTRACE(3, "taint: {} sources\n", sources.size());

  // Check if this is the origin of a source.
  if (!sources.empty()) {
    FTRACE(1, "taint: function returns source\n");
    auto path = Path::origin(state->arena.get(), Hop{func, callee()});
    stack.replaceTop(path);
  }

  // Return value overrides top of stack.
  // TODO(T93549800): we may want to keep a set of traces.
  if (saved) {
    FTRACE(1, "taint: function returns source\n");
    stack.replaceTop(saved);
  }
}

void iopRetM(PC& /* pc */, uint32_t /* numRet */) {
  iopUnhandled("RetM");
}

void iopRetCSuspended(PC& /* pc */) {
  iopUnhandled("RetCSuspended");
}

void iopThrow(PC& /* pc */) {
  iopUnhandled("Throw");
}

void iopCGetL(named_local_var fr) {
  iopPreamble("CGetL");

  auto state = State::instance;
  auto value = state->heap_locals.get(fr.lval);

  FTRACE(
      2, "taint: getting {} (name: {}, value: {})\n", fr.lval, fr.name, value);

  state->stack.push(value);
}

void iopCGetQuietL(tv_lval fr) {
  iopPreamble("CGetQuietL");

  auto state = State::instance;
  auto value = state->heap_locals.get(fr);

  FTRACE(
      2, "taint: getting {} (value: {})\n", fr, value);

  state->stack.push(value);
}

void iopCUGetL(tv_lval fr) {
  iopPreamble("CUGetL");

  auto state = State::instance;
  auto value = state->heap_locals.get(fr);

  FTRACE(
      2, "taint: getting {} (value: {})\n", fr, value);

  state->stack.push(value);
}

void iopCGetL2(named_local_var fr) {
  iopPreamble("CGetL2");

  auto state = State::instance;

  auto top = state->stack.top();
  state->stack.pop();
  auto value = state->heap_locals.get(fr.lval);

  FTRACE(
      2, "taint: getting {} (name: {}, value: {})\n", fr.lval, fr.name, value);

  state->stack.push(value);
  state->stack.push(top);
}

void iopPushL(tv_lval locVal) {
  iopPreamble("PushL");

  auto state = State::instance;
  auto value = state->heap_locals.get(locVal);

  FTRACE(
      2, "taint: getting {} (value: {})\n", locVal, value);

  state->stack.push(value);
  state->heap_locals.set(locVal, nullptr);
}

void iopCGetG() {
  iopUnhandled("CGetG");
}

void iopCGetS(ReadonlyOp /* op */) {
  iopUnhandled("CGetS");
}

void iopClassGetC() {
  iopDoesNotAffectTaint("ClassGetC");
}

void iopClassGetTS() {
  iopPreamble("ClassGetTS");
  State::instance->stack.push(nullptr);
}

void iopGetMemoKeyL(named_local_var /* loc */) {
  iopUnhandled("GetMemoKeyL");
}

void iopAKExists() {
  iopUnhandled("AKExists");
}

void iopIssetL(tv_lval /* val */) {
  iopPreamble("IssetL");
  State::instance->stack.push(nullptr);
}

void iopIssetG() {
  iopPreamble("IssetG");
  State::instance->stack.push(nullptr);
}

void iopIssetS() {
  iopPreamble("IssetS");

  auto state = State::instance;
  auto& stack = state->stack;
  stack.pop(2);
  stack.push(nullptr);
}

void iopIsUnsetL(tv_lval /* val */) {
  iopPreamble("IsUnsetL");
  State::instance->stack.push(nullptr);
}

void iopIsTypeC(IsTypeOp /* op */) {
  iopPreamble("IsTypeC");

  auto state = State::instance;
  auto& stack = state->stack;
  stack.pop();
  stack.push(nullptr);
}

void iopIsTypeL(named_local_var /* loc */, IsTypeOp /* op */) {
  iopPreamble("IsTypeL");

  State::instance->stack.push(nullptr);
}

void iopAssertRATL(local_var /* loc */, RepoAuthType /* rat */) {
  iopDoesNotAffectTaint("AssertRATL");
}

void iopAssertRATStk(uint32_t /* stkSlot */, RepoAuthType /* rat */) {
  iopDoesNotAffectTaint("AssertRATStk");
}

void iopSetL(tv_lval to) {
  iopPreamble("SetL");

  auto state = State::instance;
  auto value = state->stack.top();

  FTRACE(2, "taint: setting {} to `{}`\n", to, value);

  state->heap_locals.set(std::move(to), value);
}

void iopSetG() {
  iopUnhandled("SetG");
}

void iopSetS(ReadonlyOp /* op */) {
  iopUnhandled("SetS");
}

void iopSetOpL(tv_lval to, SetOpOp /* op */) {
  iopPreamble("SetOpL");

  // Pick whichever value is tainted.
  // This should eventually be a join.
  auto state = State::instance;
  auto value = state->heap_locals.get(to);
  if (value == nullptr) {
    value = state->stack.top();
  }
  state->stack.pop();
  state->stack.push(value);
}

void iopSetOpG(SetOpOp /* op */) {
  iopUnhandled("SetOpG");
}

void iopSetOpS(SetOpOp /* op */) {
  iopUnhandled("SetOpS");
}

void iopIncDecL(named_local_var fr, IncDecOp /* op */) {
  iopPreamble("IncDecL");

  auto state = State::instance;
  auto value = state->heap_locals.get(fr.lval);
  state->stack.push(value);
}

void iopIncDecG(IncDecOp /* op */) {
  iopUnhandled("IncDecG");
}

void iopIncDecS(IncDecOp /* op */) {
  iopUnhandled("IncDecS");
}

void iopUnsetL(tv_lval loc) {
  iopPreamble("UnsetL");
  State::instance->heap_locals.set(loc, nullptr);
}

void iopUnsetG() {
  iopUnhandled("UnsetG");
}

void iopResolveFunc(Id /* id */) {
  iopUnhandled("ResolveFunc");
}

void iopResolveMethCaller(Id /* id */) {
  iopUnhandled("ResolveMethCaller");
}

void iopResolveRFunc(Id /* id */) {
  iopUnhandled("ResolveRFunc");
}

void iopResolveClsMethod(const StringData* /* methName */) {
  iopUnhandled("ResolveClsMethod");
}

void iopResolveClsMethodD(Id /* classId */, const StringData* /* methName */) {
  iopUnhandled("ResolveClsMethodD");
}

void iopResolveClsMethodS(
    SpecialClsRef /* ref */,
    const StringData* /* methName */) {
  iopUnhandled("ResolveClsMethodS");
}

void iopResolveRClsMethod(const StringData* /* methName */) {
  iopUnhandled("ResolveRClsMethod");
}

void iopResolveRClsMethodD(Id /* classId */, const StringData* /* methName */) {
  iopUnhandled("ResolveRClsMethodD");
}

void iopResolveRClsMethodS(
    SpecialClsRef /* ref */,
    const StringData* /* methName */) {
  iopUnhandled("ResolveRClsMethodS");
}

void iopResolveClass(Id /* id */) {
  iopUnhandled("ResolveClass");
}

void iopLazyClass(Id /* id */) {
  iopConstant("LazyClass");
}

void iopNewObj() {
  iopUnhandled("NewObj");
}

void iopNewObjR() {
  iopUnhandled("NewObjR");
}

void iopNewObjD(Id /* id */) {
  iopPreamble("NewObjD");
  State::instance->stack.push(nullptr);
}

void iopNewObjRD(Id /* id */) {
  iopUnhandled("NewObjRD");
}

void iopNewObjS(SpecialClsRef /* ref */) {
  iopUnhandled("NewObjS");
}

void iopLockObj() {
  iopPreamble("LockObj");
}

void iopFCall(const Func* func, const FCallArgs& fca) {
  if (func == nullptr) {
    return;
  }

  FTRACE(
      1,
      "taint: entering {} ({} arguments)\n",
      yellow(func->fullName()->data()),
      fca.numArgs);

  auto state = State::instance;
  const auto sinks = state->sinks(func);
  FTRACE(3, "taint: {} sinks\n", sinks.size());

  for (const auto& sink : sinks) {
    Value value = nullptr;
    if (sink.index) {
      value = state->stack.peek(fca.numArgs - 1 - *sink.index);
      if (!value) {
        continue;
      }
    } else {
      // Pick the first tainted argument.
      // This should eventually be a join.
      for (int i = 0; i < fca.numArgs; i++) {
        value = state->stack.peek(fca.numArgs - 1 - i);
        if (value) {
          break;
        }
      }
      if (!value) {
        continue;
      }
    }

    FTRACE(1, "taint: tainted value flows into sink\n");
    auto path = value->to(state->arena.get(), Hop{vmfp()->func(), func});
    state->paths.push_back(path);
  }
}

TCA iopFCallClsMethod(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData*,
    IsLogAsDynamicCallOp /* op */) {
  iopUnhandled("FCallClsMethod");
  return nullptr;
}

TCA iopFCallClsMethodD(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& fca,
    const StringData*,
    Id classId,
    const StringData* methName) {
  iopPreamble("FCallClsMethodD");

  auto ar = vmfp();
  auto ctx = ar == nullptr ? nullptr : ar->func()->cls();

  auto const nep = ar->func()->unit()->lookupNamedEntityPairId(classId);
  auto cls = Class::load(nep.second, nep.first);
  if (cls == nullptr) {
    FTRACE(2, "taint: unable to load class for method `{}`\n", methName);
    return nullptr;
  }

  const Func* func;
  auto const res = lookupClsMethod(
      func, cls, methName, nullptr, ctx, MethodLookupErrorOptions::None);
  if (res == LookupResult::MethodNotFound) {
    FTRACE(2, "taint: unable to load method `{}`\n", methName);
    return nullptr;
  }

  iopFCall(func, fca);

  return nullptr;
}

TCA iopFCallClsMethodS(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData*,
    SpecialClsRef /* ref */) {
  iopUnhandled("FCallClsMethodS");
  return nullptr;
}

TCA iopFCallClsMethodSD(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData*,
    SpecialClsRef /* ref */,
    const StringData* /* methName */) {
  iopUnhandled("FCallClsMethodSD");
  return nullptr;
}

TCA iopFCallCtor(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& fca,
    const StringData*) {
  iopPreamble("FCallCtor");

  auto const obj =
      vmStack().indC(fca.numInputs() + (kNumActRecCells - 1))->m_data.pobj;

  const Func* func;
  auto ar = vmfp();
  auto ctx = ar == nullptr ? nullptr : ar->func()->cls();
  lookupCtorMethod(
      func, obj->getVMClass(), ctx, MethodLookupErrorOptions::RaiseOnNotFound);

  iopFCall(func, fca);

  return nullptr;
}

TCA iopFCallFunc(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */) {
  iopUnhandled("FCallFunc");
  return nullptr;
}

TCA iopFCallFuncD(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& fca,
    Id id) {
  iopPreamble("FCallFuncD");

  auto const nep = vmfp()->unit()->lookupNamedEntityPairId(id);
  auto const func = Func::load(nep.second, nep.first);

  iopFCall(func, fca);

  return nullptr;
}

TCA iopFCallObjMethod(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData* /* name */,
    ObjMethodOp /* op */) {
  iopUnhandled("FCallObjMethod");
  return nullptr;
}

TCA iopFCallObjMethodD(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& fca,
    const StringData*,
    ObjMethodOp /* op */,
    const StringData* name) {
  iopPreamble("FCallObjMethodD");

  auto const obj = vmStack().indC(fca.numInputs() + (kNumActRecCells - 1));
  if (!isObjectType(obj->m_type)) {
    return nullptr;
  }

  auto ar = vmfp();
  auto ctx = ar == nullptr ? nullptr : ar->func()->cls();

  const Func* func;
  lookupObjMethod(
      func,
      obj->m_data.pobj->getVMClass(),
      name,
      ctx,
      MethodLookupErrorOptions::RaiseOnNotFound);

  iopFCall(func, fca);

  return nullptr;
}

void iopIterInit(PC& /* pc */, const IterArgs& /* ita */, PC /* targetpc */) {
  iopUnhandled("IterInit");
}

void iopLIterInit(
    PC& /* pc */,
    const IterArgs& /* ita */,
    TypedValue* /* base */,
    PC /* targetpc */) {
  iopUnhandled("LIterInit");
}

void iopIterNext(PC& /* pc */, const IterArgs& /* ita */, PC /* targetpc */) {
  iopUnhandled("IterNext");
}

void iopLIterNext(
    PC& /* pc */,
    const IterArgs& /* ita */,
    TypedValue* /* base */,
    PC /* targetpc */) {
  iopUnhandled("LIterNext");
}

void iopIterFree(Iter* /* it */) {
  iopUnhandled("IterFree");
}

void iopLIterFree(Iter* /* it */, tv_lval) {
  iopUnhandled("LIterFree");
}

void iopIncl() {
  iopUnhandled("Incl");
}

void iopInclOnce() {
  iopUnhandled("InclOnce");
}

void iopReq() {
  iopUnhandled("Req");
}

void iopReqOnce() {
  iopUnhandled("ReqOnce");
}

void iopReqDoc() {
  iopUnhandled("ReqDoc");
}

void iopEval() {
  iopUnhandled("Eval");
}

void iopThis() {
  iopPreamble("This");
  // We don't care about tainting `this` (for now) since we taint properties directly
  State::instance->stack.push(nullptr);
}

void iopBareThis(BareThisOp /* bto */) {
  iopPreamble("BareThis");
  // We don't care about tainting `this` (for now) since we taint properties directly
  State::instance->stack.push(nullptr);
}

void iopCheckThis() {
  iopDoesNotAffectTaint("CheckThis");
}

void iopChainFaults() {
  iopUnhandled("ChainFaults");
}

void iopOODeclExists(OODeclExistsOp /* subop */) {
  iopUnhandled("OODeclExists");
}

void iopVerifyOutType(uint32_t /* paramId */) {
  iopUnhandled("VerifyOutType");
}

void iopVerifyParamType(local_var param) {
  iopPreamble("VerifyParamType");

  auto state = State::instance;
  auto func = vmfp()->func();

  // Taint propagation.
  auto index = func->numParams() - (param.index + 1);
  auto value = state->stack.peek(index);
  if (value) {
    FTRACE(
        2,
        "taint: setting parameter {} (index: {}) to `{}`\n",
        param.lval,
        param.index,
        value);
    auto path = value->to(state->arena.get(), Hop{callee(), func});
    state->heap_locals.set(param.lval, path);
  }

  // Taint generation.
  auto sources = state->sources(func);
  for (auto& source : sources) {
    if (source.index && *source.index == param.index) {
      FTRACE(
          2,
          "taint: parameter {} (index: {}) is tainted\n",
          param.lval,
          param.index);
      // TODO: What hop should we put here?
      auto path = Path::origin(state->arena.get(), Hop());
      state->heap_locals.set(param.lval, path);
      break;
    }
  }
}

void iopVerifyParamTypeTS(local_var /* param */) {
  iopUnhandled("VerifyParamTypeTS");
}

void iopVerifyRetTypeC() {
  iopPreamble("VerifyRetTypeC");
}

void iopVerifyRetTypeTS() {
  iopUnhandled("VerifyRetTypeTS");
}

void iopVerifyRetNonNullC() {
  iopUnhandled("VerifyRetNonNullC");
}

void iopSelfCls() {
  iopUnhandled("SelfCls");
}

void iopParentCls() {
  iopUnhandled("ParentCls");
}

void iopLateBoundCls() {
  iopUnhandled("LateBoundCls");
}

void iopRecordReifiedGeneric() {
  iopUnhandled("RecordReifiedGeneric");
}

void iopCheckReifiedGenericMismatch() {
  iopUnhandled("CheckReifiedGenericMismatch");
}

void iopNativeImpl(PC& /* pc */) {
  iopUnhandled("NativeImpl");
}

void iopCreateCl(uint32_t /* numArgs */, uint32_t /* clsIx */) {
  iopUnhandled("CreateCl");
}

void iopCreateCont(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("CreateCont");
}

void iopContEnter(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("ContEnter");
}

void iopContRaise(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("ContRaise");
}

void iopYield(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("Yield");
}

void iopYieldK(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("YieldK");
}

void iopContCheck(ContCheckOp /* subop */) {
  iopUnhandled("ContCheck");
}

void iopContValid() {
  iopUnhandled("ContValid");
}

void iopContKey() {
  iopUnhandled("ContKey");
}

void iopContCurrent() {
  iopUnhandled("ContCurrent");
}

void iopContGetReturn() {
  iopUnhandled("ContGetReturn");
}

void iopWHResult() {
  iopUnhandled("WHResult");
}

void iopAwait(PC /* origpc */, PC& /* pc */) {
  iopUnhandled("Await");
}

void iopAwaitAll(PC /* origpc */, PC& /* pc */, LocalRange /* locals */) {
  iopUnhandled("AwaitAll");
}

void iopIdx() {
  iopUnhandled("Idx");
}

void iopArrayIdx() {
  iopUnhandled("ArrayIdx");
}

void iopArrayMarkLegacy() {
  iopUnhandled("ArrayMarkLegacy");
}

void iopArrayUnmarkLegacy() {
  iopUnhandled("ArrayUnmarkLegacy");
}

void iopCheckProp(const StringData* /* propName */) {
  iopPreamble("CheckProp");
  State::instance->stack.push(nullptr);
}

void iopSetImplicitContextByValue() {
  iopUnhandled("SetImplicitContextByValue");
}

void iopInitProp(const StringData* /* propName */, InitPropOp /* propOp */) {
  iopPreamble("InitProp");
  State::instance->stack.pop();
}

void iopSilence(tv_lval /* loc */, SilenceOp /* subop */) {
  iopUnhandled("Silence");
}

void iopThrowNonExhaustiveSwitch() {
  iopUnhandled("ThrowNonExhaustiveSwitch");
}

void iopRaiseClassStringConversionWarning() {
  iopUnhandled("RaiseClassStringConversionWarning");
}

void iopBaseGC(uint32_t /* idx */, MOpMode /* mode */) {
  iopUnhandled("BaseGC");
}

void iopBaseGL(tv_lval /* loc */, MOpMode /* mode */) {
  iopUnhandled("BaseGL");
}

void iopBaseSC(
    uint32_t /* keyIdx */,
    uint32_t /* clsIdx */,
    MOpMode /* mode */,
    ReadonlyOp /* op */) {
  iopUnhandled("BaseSC");
}

void iopBaseL(
    named_local_var /* loc */,
    MOpMode /* mode */,
    ReadonlyOp /* op */) {
  iopPreamble("BaseL");
}

void iopBaseC(uint32_t /* idx */, MOpMode) {
  iopUnhandled("BaseC");
}

void iopBaseH() {
  iopUnhandled("BaseH");
}

void iopDim(MOpMode /* mode */, MemberKey /* mk */) {
  iopUnhandled("Dim");
}

namespace {

// Resolves a member key to the actual value being read as a TypedValue
// to resolve the property access
TypedValue memberKeyToObjectKey(MemberKey memberKey) {
  switch (memberKey.mcode) {
    case MW:
      return TypedValue{};
    case MEL:
    case MPL: {
      auto const local = frame_local(vmfp(), memberKey.local.id);
      if (type(local) == KindOfUninit) {
        return make_tv<KindOfNull>();
      }
      return tvClassToString(*local);
    }
    case MEC:
    case MPC:
      return tvClassToString(*vmStack().indTV(memberKey.iva));
    case MEI:
      return make_tv<KindOfInt64>(memberKey.int64);
    case MET:
    case MPT:
    case MQT:
      return make_tv<KindOfPersistentString>(memberKey.litstr);
  }
  not_reached();
}

// Similar to the above but just returns whether either the key or the
// value in this operation is tainted or not. Ignores joins,
// just propagates the first taint it finds for now.
Value memberKeyToElementTaint(MemberKey memberKey) {
  auto state = State::instance;
  switch (memberKey.mcode) {
    case MW:
      // New elements add the value on the top of the stack
      return state->stack.top();
    case MEL:
    case MPL: {
      // Check if the value is tainted
      auto taint = state->stack.top();
      // If not tainted, fall back to the taint for the key.
      if (taint == nullptr) {
        auto const local = frame_local(vmfp(), memberKey.local.id);
        taint = state->heap_locals.get(local);
      }
      return taint;
    }
    case MEC:
    case MPC: {
      // Check if the value is tainted
      auto taint = state->stack.top();
      // Pull from the referenced stack element (which is the key)
      if (taint == nullptr) {
        taint = state->stack.peek(memberKey.iva);
      }
      return taint;
    }
    case MEI: {
      // Check if the value is tainted
      auto taint = state->stack.top();
      // Pull from the referenced stack element (which is the key)
      if (taint == nullptr) {
        taint = state->stack.peek(memberKey.int64);
      }
      return taint;
    }
    case MET:
    case MPT:
    case MQT:
      // Accesses using, so fall back to the taint on the value
      // We assume immediates are not tainted.
      return state->stack.top();
  }
  not_reached();
}

} // namespace

void iopQueryM(uint32_t nDiscard, QueryMOp op, MemberKey memberKey) {
  iopPreamble("QueryM");

  auto state = State::instance;
  Value value = nullptr;
  switch (op) {
    case QueryMOp::InOut:
    case QueryMOp::CGet:
    case QueryMOp::CGetQuiet: {
      if (mcodeIsProp(memberKey.mcode)) {
        // Handle objects as just a map from property to taint.
        // Ignore custom getters and any exceptions thrown if the
        // base is not an object (silently return null)
        auto key = memberKeyToObjectKey(memberKey);
        if (isStringType(key.m_type)) {
          StringData* str = key.m_data.pstr;
          auto property = str->slice();
          auto& mstate = vmMInstrState();
          ObjectData* from = val(*mstate.base).pobj;
          value = state->heap_objects.get(from, property);
          FTRACE(
              2,
              "taint: getting member {} (from: {}, name: {}), value: `{}`\n",
              memberKey,
              from,
              property,
              value);
        } else {
          // This should never happen... just swallow.
        }
      } else if (mcodeIsElem(memberKey.mcode)) {
        auto& mstate = vmMInstrState();
        auto from = mstate.base;
        value = state->heap_collections.get(from);
        FTRACE(
            2,
            "taint: getting member {} (from: {}), value: `{}`\n",
            memberKey,
            from,
            value);
      } else {
        // The interpreter swallows this, and would warn on this too.
        FTRACE(2, "taint: Cannot use [] for reading");
      }
      break;
    }
    case QueryMOp::Isset: {
      // Don't care about taint here, fall through
      break;
    }
  }

  for (size_t i = 0; i < nDiscard; i++) {
    state->stack.pop();
  }
  state->stack.push(value);
}

void iopSetM(uint32_t nDiscard, MemberKey memberKey) {
  iopPreamble("SetM");

  auto state = State::instance;
  auto value = state->stack.top();
  if (mcodeIsProp(memberKey.mcode)) {
    // Handle objects as just a map from property to taint.
    // Ignore custom setters and any exceptions thrown if the
    // base is not an object (silently return null) or if
    // the SetM fails. The stack might get out of sync in that case
    auto key = memberKeyToObjectKey(memberKey);
    if (isStringType(key.m_type)) {
      StringData* str = key.m_data.pstr;
      auto property = str->slice();
      auto& mstate = vmMInstrState();
      ObjectData* to = val(*mstate.base).pobj;
      state->heap_objects.set(to, property, value);
      FTRACE(
          2,
          "taint: setting member {} (to: {}, name: {}), value: `{}`\n",
          memberKey,
          to,
          property,
          value);
    } else {
      // This should never happen... just swallow.
    }
  } else if (mcodeIsElem(memberKey.mcode) || memberKey.mcode == MW) {
    // We're either modifying a collection or adding an element to something which we
    // assume is a collection. Find the right base, which should be set already.
    auto& mstate = vmMInstrState();
    auto to = mstate.base;
    // TODO: Should we overwrite the taint on the value that gets pushed
    //       back on the stack? We ignore it for now
    auto taint = memberKeyToElementTaint(memberKey);
    if (taint == nullptr) {
      // See if the collection is already tainted, if the value is not
      // This should eventually be a join (or we should track taints precisely).
      taint = state->heap_collections.get(to);
    }
    state->heap_collections.set(to, taint);
    FTRACE(
        2,
        "taint: setting member {} (to: {}), value: `{}`\n",
        memberKey,
        to,
        taint);
  } else {
    // This should never happen...
  }

  for (size_t i = 0; i < nDiscard + 1; i++) {
    state->stack.pop();
  }
  state->stack.push(value);
}

void iopSetRangeM(
    uint32_t /* nDiscard */,
    uint32_t /* size */,
    SetRangeOp /* op */) {
  iopUnhandled("SetRangeM");
}

void iopIncDecM(
    uint32_t /* nDiscard */,
    IncDecOp /* subop */,
    MemberKey /* mk */) {
  iopUnhandled("IncDecM");
}

void iopSetOpM(
    uint32_t /* nDiscard */,
    SetOpOp /* subop */,
    MemberKey /* mk */) {
  iopUnhandled("SetOpM");
}

void iopUnsetM(uint32_t /* nDiscard */, MemberKey /* mk */) {
  iopUnhandled("UnsetM");
}

void iopMemoGet(PC& /* pc */, PC /* notfound */, LocalRange /* keys */) {
  iopUnhandled("MemoGet");
}

void iopMemoGetEager(
    PC& /* pc */,
    PC /* notfound */,
    PC /* suspended */,
    LocalRange /* keys */) {
  iopUnhandled("MemoGetEager");
}

void iopMemoSet(LocalRange /* keys */) {
  iopUnhandled("MemoSet");
}

void iopMemoSetEager(LocalRange /* keys */) {
  iopUnhandled("MemoSetEager");
}

} // namespace taint
} // namespace HPHP

#endif
