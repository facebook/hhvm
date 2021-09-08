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

#include "hphp/util/trace.h"
#include "hphp/util/text-color.h"

namespace folly {

template<> class FormatValue<HPHP::taint::Value> {
 public:
  explicit FormatValue(const HPHP::taint::Value& value): m_value(value) {}
  template<class FormatCallback>
  void format(FormatArg& arg, FormatCallback& cb) const {
    auto value = m_value ? "S" : "_";
    FormatValue<std::string>(value).format(arg, cb);
  }
 private:
  const HPHP::taint::Value& m_value;
};

template<> class FormatValue<HPHP::tv_lval> {
 public:
  explicit FormatValue(const HPHP::tv_lval& value): m_value(value) {}
  template<class FormatCallback>
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

template<> class FormatValue<HPHP::MemberKey> {
 public:
  explicit FormatValue(const HPHP::MemberKey& memberKey): m_value(memberKey) {}
  template<class FormatCallback>
  void format(FormatArg& arg, FormatCallback& cb) const {
    auto string = folly::sformat("{}", HPHP::show(m_value));
    format_value::formatString(string, arg, cb);
  }
 private:
  const HPHP::MemberKey& m_value;
};

} // folly


namespace HPHP {
namespace taint {

TRACE_SET_MOD(taint);

namespace {

void iopPreamble(const std::string& name) {
  auto vm_stack_size = vmStack().count();

  FTRACE(1, "taint: iop{}\n", name);
  FTRACE(4, "taint: stack: {}\n", State::get()->stack.show());

  auto& stack = State::get()->stack;
  auto shadow_stack_size = stack.size();
  if (vm_stack_size != shadow_stack_size) {
    FTRACE(
        3,
        "taint: (WARNING) stacks out of sync "
        "(stack size: {}, shadow stack size: {}). Adjusting...\n",
        vm_stack_size,
        shadow_stack_size);
    for (int i = shadow_stack_size; i <= vm_stack_size; i++) {
      stack.push(std::nullopt);
    }
    for (int i = shadow_stack_size; i > vm_stack_size; i--) {
      stack.pop();
    }
  }
}

void iopConstant(const std::string& name) {
  iopPreamble(name);
  State::get()->stack.push(std::nullopt);
}

void iopUnhandled(const std::string& name) {
  iopPreamble(name);
  FTRACE(1, "taint: (WARNING) unhandled opcode\n");
}

std::string yellow(const std::string& string) {
  return folly::sformat(
      "{}`{}`{}",
      ANSI_COLOR_YELLOW,
      string,
      ANSI_COLOR_END);
}

} // namespace

void iopNop() {
  iopUnhandled("Nop");
}

void iopEntryNop() {
  iopUnhandled("EntryNop");
}

void iopBreakTraceHint() {
  iopUnhandled("BreakTraceHint");
}

void iopPopC() {
  iopPreamble("PopC");
  State::get()->stack.pop();
}

void iopPopU() {
  iopUnhandled("PopU");
}

void iopPopU2() {
  iopUnhandled("PopU2");
}

void iopPopL(tv_lval /* to */) {
  iopUnhandled("PopL");
}

void iopDup() {
  iopPreamble("Dup");
  State::get()->stack.push(std::nullopt);
}

void iopCGetCUNop() {
  iopUnhandled("CGetCUNop");
}

void iopUGetCUNop() {
  iopUnhandled("UGetCUNop");
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
  iopUnhandled("FuncCred");
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

void iopDict(const ArrayData* /* a */) {
  iopUnhandled("Dict");
}

void iopKeyset(const ArrayData* /* a */) {
  iopUnhandled("Keyset");
}

void iopVec(const ArrayData* /* a */) {
  iopUnhandled("Vec");
}

void iopNewDictArray(uint32_t /* capacity */) {
  iopUnhandled("NewDictArray");
}

void iopNewStructDict(imm_array<int32_t> /* ids */) {
  iopUnhandled("NewStructDict");
}

void iopNewVec(uint32_t /* n */) {
  iopUnhandled("NewVec");
}

void iopNewKeysetArray(uint32_t /* n */) {
  iopUnhandled("NewKeysetArray");
}

void iopAddElemC() {
  iopUnhandled("AddElemC");
}

void iopAddNewElemC() {
  iopUnhandled("AddNewElemC");
}

void iopNewCol(CollectionType /* cType */) {
  iopUnhandled("NewCol");
}

void iopNewPair() {
  iopUnhandled("NewPair");
}

void iopColFromArray(CollectionType /* cType */) {
  iopUnhandled("ColFromArray");
}

void iopCnsE(const StringData* /* s */) {
  iopUnhandled("CnsE");
}

void iopClsCns(const StringData* /* clsCnsName */) {
  iopUnhandled("ClsCns");
}

void iopClsCnsD(const StringData* /* clsCnsName */, Id /* classId */) {
  iopUnhandled("ClsCnsD");
}

void iopClsCnsL(tv_lval /* local */) {
  iopUnhandled("ClsCnsL");
}

void iopClassName() {
  iopUnhandled("ClassName");
}

void iopLazyClassFromClass() {
  iopUnhandled("LazyClassFromClass");
}

void iopFile() {
  iopUnhandled("File");
}

void iopDir() {
  iopUnhandled("Dir");
}

void iopMethod() {
  iopUnhandled("Method");
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
  iopUnhandled("Fatal");
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

  auto& stack = State::get()->stack;
  auto saved = stack.top();

  auto func = vmfp()->func();
  stack.pop(2 + func->params().size());

  std::string name = func->fullName()->data();
  FTRACE(1, "taint: leaving {}\n", yellow(name));

  // Check if this function is the origin of a source.
  auto& sources = Configuration::get()->sources;
  if (sources.find(name) != sources.end()) {
    FTRACE(1, "taint: function returns source\n");
    Path path;
    path.hops.push_back(func);
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

  auto state = State::get();
  auto value = state->heap.get(fr.lval);

  FTRACE(2, "taint: getting {} (name: {}, value: {})\n", fr.lval, fr.name, value);

  state->stack.push(value);
}

void iopCGetQuietL(tv_lval /* fr */) {
  iopUnhandled("CGetQuietL");
}

void iopCUGetL(tv_lval /* fr */) {
  iopUnhandled("CUGetL");
}

void iopCGetL2(named_local_var /* fr */) {
  iopUnhandled("CGetL2");
}

void iopPushL(tv_lval /* locVal */) {
  iopUnhandled("PushL");
}

void iopCGetG() {
  iopUnhandled("CGetG");
}

void iopCGetS(ReadonlyOp /* op */) {
  iopUnhandled("CGetS");
}

void iopClassGetC() {
  iopUnhandled("ClassGetC");
}

void iopClassGetTS() {
  iopUnhandled("ClassGetTS");
}

void iopGetMemoKeyL(named_local_var /* loc */) {
  iopUnhandled("GetMemoKeyL");
}

void iopAKExists() {
  iopUnhandled("AKExists");
}

void iopIssetL(tv_lval /* val */) {
  iopUnhandled("IssetL");
}

void iopIssetG() {
  iopUnhandled("IssetG");
}

void iopIssetS() {
  iopUnhandled("IssetS");
}

void iopIsUnsetL(tv_lval /* val */) {
  iopUnhandled("IsUnsetL");
}

void iopIsTypeC(IsTypeOp /* op */) {
  iopUnhandled("IsTypeC");
}

void iopIsTypeL(named_local_var /* loc */, IsTypeOp /* op */) {
  iopUnhandled("IsTypeL");
}

void iopAssertRATL(local_var /* loc */, RepoAuthType /* rat */) {
  iopUnhandled("AssertRATL");
}

void iopAssertRATStk(uint32_t /* stkSlot */, RepoAuthType /* rat */) {
  iopUnhandled("AssertRATStk");
}

void iopSetL(tv_lval to) {
  iopPreamble("SetL");

  auto state = State::get();
  auto value = state->stack.top();

  FTRACE(2, "taint: setting {} to `{}`\n", to, value);

  state->heap.set(to, value);
}

void iopSetG() {
  iopUnhandled("SetG");
}

void iopSetS(ReadonlyOp /* op */) {
  iopUnhandled("SetS");
}

void iopSetOpL(tv_lval /* to */, SetOpOp /* op */) {
  iopUnhandled("SetOpL");
}

void iopSetOpG(SetOpOp /* op */) {
  iopUnhandled("SetOpG");
}

void iopSetOpS(SetOpOp /* op */) {
  iopUnhandled("SetOpS");
}

void iopIncDecL(named_local_var /* fr */, IncDecOp /* op */) {
  iopUnhandled("IncDecL");
}

void iopIncDecG(IncDecOp /* op */) {
  iopUnhandled("IncDecG");
}

void iopIncDecS(IncDecOp /* op */) {
  iopUnhandled("IncDecS");
}

void iopUnsetL(tv_lval /* loc */) {
  iopUnhandled("UnsetL");
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
  iopUnhandled("LazyClass");
}

void iopNewObj() {
  iopUnhandled("NewObj");
}

void iopNewObjR() {
  iopUnhandled("NewObjR");
}

void iopNewObjD(Id /* id */) {
  iopPreamble("NewObjD");
  State::get()->stack.push(std::nullopt);
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
    const FCallArgs& /* fca */,
    const StringData*,
    Id /* classId */,
    const StringData* /* methName */) {
  iopUnhandled("FCallClsMethodD");
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

  auto const obj = vmStack()
      .indC(fca.numInputs() + (kNumActRecCells - 1))
      ->m_data.pobj;

  const Func* func;
  auto ar = vmfp();
  auto ctx = ar == nullptr ? nullptr : ar->func()->cls();
  lookupCtorMethod(
      func,
      obj->getVMClass(),
      ctx,
      MethodLookupErrorOptions::RaiseOnNotFound);
  auto name = func->fullName()->data();

  FTRACE(1, "taint: entering {}\n", yellow(name));

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
  auto name = func->fullName()->data();

  FTRACE(1, "taint: entering {}\n", yellow(name));

  const auto& sinks = Configuration::get()->sinks(name);
  for (const auto& sink : sinks) {
    auto value = State::get()->stack.peek(fca.numArgs - 1 - sink.index);
    if (!value) { continue; }

    FTRACE(1, "taint: tainted value flows into sink\n");
    value->hops.push_back(func);
    value->dump();
  }

  return nullptr;
}

TCA iopFCallObjMethod(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData*,
    ObjMethodOp /* op */) {
  iopUnhandled("FCallObjMethod");
  return nullptr;
}

TCA iopFCallObjMethodD(
    bool /* retToJit */,
    PC /* origpc */,
    PC& /* pc */,
    const FCallArgs& /* fca */,
    const StringData*,
    ObjMethodOp /* op */,
    const StringData* /* methName */) {
  iopUnhandled("FCallObjMethodD");
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
  iopUnhandled("This");
}

void iopBareThis(BareThisOp /* bto */) {
  iopUnhandled("BareThis");
}

void iopCheckThis() {
  iopUnhandled("CheckThis");
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

  auto state = State::get();
  auto func = vmfp()->func();

  auto index = func->numParams() - (param.index + 1);
  auto value = state->stack.peek(index);
  if (value) {
    FTRACE(
        2,
        "taint: setting parameter {} (index: {}) to `{}`\n",
        param.lval,
        param.index,
        value);
    value->hops.push_back(func);
    state->heap.set(param.lval, value);
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

void iopSelf() {
  iopUnhandled("Self");
}

void iopParent() {
  iopUnhandled("Parent");
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
  iopUnhandled("CheckProp");
}

void iopInitProp(const StringData* /* propName */, InitPropOp /* propOp */) {
  iopUnhandled("InitProp");
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

TypedValue typedValue(MemberKey memberKey) {
  switch (memberKey.mcode) {
    case MW:
      return TypedValue{};
    case MEL: case MPL: {
      auto const local = frame_local(vmfp(), memberKey.local.id);
      if (type(local) == KindOfUninit) {
        return make_tv<KindOfNull>();
      }
      return tvClassToString(*local);
    }
    case MEC: case MPC:
      return tvClassToString(*vmStack().indTV(memberKey.iva));
    case MEI:
      return make_tv<KindOfInt64>(memberKey.int64);
    case MET: case MPT: case MQT:
      return make_tv<KindOfPersistentString>(memberKey.litstr);
  }
  not_reached();
}

tv_lval resolveMemberKey(const MemberKey& memberKey) {
  auto& instructionState = vmMInstrState();
  auto key = typedValue(memberKey);
  return Prop<MOpMode::None>(
      instructionState.tvTempBase,
      arGetContextClass(vmfp()),
      instructionState.base,
      key,
      ReadonlyOp::Readonly);
}

}  // namespace

void iopQueryM(
    uint32_t /* nDiscard */,
    QueryMOp op,
    MemberKey memberKey) {
  iopPreamble("QueryM");

  switch(op) {
    case QueryMOp::CGet: {
      auto state = State::get();
      auto from = resolveMemberKey(memberKey);
      auto value = state->heap.get(from);
      FTRACE(2, "taint: getting member {}, value: `{}`\n", memberKey, value);
      state->stack.push(value);
      break;
    }
    default:
      FTRACE(1, "taint: (WARNING) unsuppoted query operation\n");
      return;
  }
}

void iopSetM(uint32_t /* nDiscard */, MemberKey memberKey) {
  iopPreamble("SetM");

  auto state = State::get();
  auto value = state->stack.top();
  auto to = resolveMemberKey(memberKey);

  FTRACE(
      2,
      "taint: setting member {} (resolved: {}) to `{}`\n",
      memberKey,
      to,
      value);
  state->heap.set(to, value);
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
