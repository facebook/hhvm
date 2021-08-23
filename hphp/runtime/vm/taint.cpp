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

#include <folly/Singleton.h>
#include <folly/json.h>

#include <boost/filesystem/string_file.hpp>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/taint.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/trace.h"

namespace HPHP {
namespace taint {

TRACE_SET_MOD(taint);

namespace {

struct ConfigurationSingletonTag {};
struct StateSingletonTag {};

} // namespace

InitFiniNode s_configurationInitialization([]() {
  Configuration::get()->read(RO::EvalTaintConfigurationPath);
}, InitFiniNode::When::ProcessInit);

void Configuration::read(const std::string& path) {
  sources.clear();
  sinks.clear();

  try {
    std::string contents;
    boost::filesystem::load_string_file(path, contents);
    auto parsed = folly::parseJson(contents);
    for (const auto source : parsed["sources"]) {
      sources.insert(source.asString());
    }
    for (const auto sink : parsed["sinks"]) {
      sinks.insert(sink.asString());
    }
  } catch (std::exception& exception) {
    // Swallow because we don't use it in tests.
    std::cerr << "taint: warning, unable to read configuration ("
      << exception.what() << ")" << std::endl;
  }
}

folly::Singleton<Configuration, ConfigurationSingletonTag> kConfigurationSingleton{};
/* static */ std::shared_ptr<Configuration> Configuration::get() {
  return kConfigurationSingleton.try_get();
}

void Stack::push(Source source) {
  m_stack.push_back(source);
}

Source Stack::top() const {
  assertx(!m_stack.empty());
  return m_stack.back();
}

void Stack::pop(int n) {
  assertx(m_stack.size() >= n);
  for (int i = 0; i < n; i++) {
    m_stack.pop_back();
  }
}

void Stack::replaceTop(Source source) {
  assertx(!m_stack.empty());
  m_stack.back() = source;
}

size_t Stack::size() const {
  return m_stack.size();
}

void Stack::clear() {
  m_stack.clear();
}

folly::Singleton<State, StateSingletonTag> kStateSingleton{};
/* static */ std::shared_ptr<State> State::get() {
  return kStateSingleton.try_get();
}

namespace {

void iopPreamble() {
  auto vm_stack_size = vmStack().count();
  FTRACE(
      3,
      "taint: stack -> size: {}, pushes: {}, pops: {}\n",
      vm_stack_size,
      instrNumPushes(vmpc()),
      instrNumPops(vmpc()));
  auto shadow_stack_size = State::get()->stack.size();
  if (vm_stack_size != shadow_stack_size) {
    FTRACE(
        3,
        "taint: stacks out of sync (shadow stack size: {})\n",
        shadow_stack_size);
  }
}

void iopUnhandled(const std::string& name) {
  FTRACE(1, "taint: unhandled opcode `{}`\n", name);
  iopPreamble();
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
  iopUnhandled("PopC");
}

void iopPopU() {
  iopUnhandled("PopU");
}

void iopPopU2() {
  iopUnhandled("PopU2");
}

void iopPopL() {
  iopUnhandled("PopL");
}

void iopDup() {
  iopUnhandled("Dup");
}

void iopCGetCUNop() {
  iopUnhandled("CGetCUNop");
}

void iopUGetCUNop() {
  iopUnhandled("UGetCUNop");
}

void iopNull() {
  iopUnhandled("Null");
}

void iopNullUninit() {
  iopUnhandled("NullUninit");
}

void iopTrue() {
  iopUnhandled("True");
}

void iopFalse() {
  iopUnhandled("False");
}

void iopFuncCred() {
  iopUnhandled("FuncCred");
}

void iopInt() {
  iopUnhandled("Int");
}

void iopDouble() {
  iopUnhandled("Double");
}

void iopString() {
  iopUnhandled("String");
}

void iopDict() {
  iopUnhandled("Dict");
}

void iopKeyset() {
  iopUnhandled("Keyset");
}

void iopVec() {
  iopUnhandled("Vec");
}

void iopNewDictArray() {
  iopUnhandled("NewDictArray");
}

void iopNewStructDict() {
  iopUnhandled("NewStructDict");
}

void iopNewVec() {
  iopUnhandled("NewVec");
}

void iopNewKeysetArray() {
  iopUnhandled("NewKeysetArray");
}

void iopNewRecord() {
  iopUnhandled("NewRecord");
}

void iopAddElemC() {
  iopUnhandled("AddElemC");
}

void iopAddNewElemC() {
  iopUnhandled("AddNewElemC");
}

void iopNewCol() {
  iopUnhandled("NewCol");
}

void iopNewPair() {
  iopUnhandled("NewPair");
}

void iopColFromArray() {
  iopUnhandled("ColFromArray");
}

void iopCnsE() {
  iopUnhandled("CnsE");
}

void iopClsCns() {
  iopUnhandled("ClsCns");
}

void iopClsCnsD() {
  iopUnhandled("ClsCnsD");
}

void iopClsCnsL() {
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

void iopConcatN() {
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

void iopInstanceOfD() {
  iopUnhandled("InstanceOfD");
}

void iopIsLateBoundCls() {
  iopUnhandled("IsLateBoundCls");
}

void iopIsTypeStructC() {
  iopUnhandled("IsTypeStructC");
}

void iopThrowAsTypeStructException() {
  iopUnhandled("ThrowAsTypeStructException");
}

void iopCombineAndResolveTypeStruct() {
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

void iopFatal() {
  iopUnhandled("Fatal");
}

void iopJmp() {
  iopUnhandled("Jmp");
}

void iopJmpNS() {
  iopUnhandled("JmpNS");
}

void iopJmpZ() {
  iopUnhandled("JmpZ");
}

void iopJmpNZ() {
  iopUnhandled("JmpNZ");
}

void iopSwitch() {
  iopUnhandled("Switch");
}

void iopSSwitch() {
  iopUnhandled("SSwitch");
}

void iopRetC() {
  FTRACE(1, "taint: RetC\n");
  iopPreamble();

  std::string name = vmfp()->func()->fullName()->data();
  auto& sources = Configuration::get()->sources;
  if (sources.find(name) != sources.end()) {
    FTRACE(1, "taint: {}: function returns `TestSource`\n", pcOff());
    State::get()->stack.replaceTop(kTestSource);
  }
}

void iopRetM() {
  iopUnhandled("RetM");
}

void iopRetCSuspended() {
  iopUnhandled("RetCSuspended");
}

void iopThrow() {
  iopUnhandled("Throw");
}

void iopCGetL() {
  iopUnhandled("CGetL");
}

void iopCGetQuietL() {
  iopUnhandled("CGetQuietL");
}

void iopCUGetL() {
  iopUnhandled("CUGetL");
}

void iopCGetL2() {
  iopUnhandled("CGetL2");
}

void iopPushL() {
  iopUnhandled("PushL");
}

void iopCGetG() {
  iopUnhandled("CGetG");
}

void iopCGetS() {
  iopUnhandled("CGetS");
}

void iopClassGetC() {
  iopUnhandled("ClassGetC");
}

void iopClassGetTS() {
  iopUnhandled("ClassGetTS");
}

void iopGetMemoKeyL() {
  iopUnhandled("GetMemoKeyL");
}

void iopAKExists() {
  iopUnhandled("AKExists");
}

void iopIssetL() {
  iopUnhandled("IssetL");
}

void iopIssetG() {
  iopUnhandled("IssetG");
}

void iopIssetS() {
  iopUnhandled("IssetS");
}

void iopIsUnsetL() {
  iopUnhandled("IsUnsetL");
}

void iopIsTypeC() {
  iopUnhandled("IsTypeC");
}

void iopIsTypeL() {
  iopUnhandled("IsTypeL");
}

void iopAssertRATL() {
  iopUnhandled("AssertRATL");
}

void iopAssertRATStk() {
  iopUnhandled("AssertRATStk");
}

void iopSetL() {
  iopUnhandled("SetL");
}

void iopSetG() {
  iopUnhandled("SetG");
}

void iopSetS() {
  iopUnhandled("SetS");
}

void iopSetOpL() {
  iopUnhandled("SetOpL");
}

void iopSetOpG() {
  iopUnhandled("SetOpG");
}

void iopSetOpS() {
  iopUnhandled("SetOpS");
}

void iopIncDecL() {
  iopUnhandled("IncDecL");
}

void iopIncDecG() {
  iopUnhandled("IncDecG");
}

void iopIncDecS() {
  iopUnhandled("IncDecS");
}

void iopUnsetL() {
  iopUnhandled("UnsetL");
}

void iopUnsetG() {
  iopUnhandled("UnsetG");
}

void iopResolveFunc() {
  iopUnhandled("ResolveFunc");
}

void iopResolveMethCaller() {
  iopUnhandled("ResolveMethCaller");
}

void iopResolveRFunc() {
  iopUnhandled("ResolveRFunc");
}

void iopResolveClsMethod() {
  iopUnhandled("ResolveClsMethod");
}

void iopResolveClsMethodD() {
  iopUnhandled("ResolveClsMethodD");
}

void iopResolveClsMethodS() {
  iopUnhandled("ResolveClsMethodS");
}

void iopResolveRClsMethod() {
  iopUnhandled("ResolveRClsMethod");
}

void iopResolveRClsMethodD() {
  iopUnhandled("ResolveRClsMethodD");
}

void iopResolveRClsMethodS() {
  iopUnhandled("ResolveRClsMethodS");
}

void iopResolveClass() {
  iopUnhandled("ResolveClass");
}

void iopLazyClass() {
  iopUnhandled("LazyClass");
}

void iopNewObj() {
  iopUnhandled("NewObj");
}

void iopNewObjR() {
  iopUnhandled("NewObjR");
}

void iopNewObjD() {
  iopUnhandled("NewObjD");
}

void iopNewObjRD() {
  iopUnhandled("NewObjRD");
}

void iopNewObjS() {
  iopUnhandled("NewObjS");
}

void iopLockObj() {
  iopUnhandled("LockObj");
}

void iopFCallClsMethod() {
  iopUnhandled("FCallClsMethod");
}

void iopFCallClsMethodD() {
  iopUnhandled("FCallClsMethodD");
}

void iopFCallClsMethodS() {
  iopUnhandled("FCallClsMethodS");
}

void iopFCallClsMethodSD() {
  iopUnhandled("FCallClsMethodSD");
}

void iopFCallCtor() {
  iopUnhandled("FCallCtor");
}

void iopFCallFunc() {
  iopUnhandled("FCallFunc");
}

void iopFCallFuncD() {
  FTRACE(1, "taint: FCallFuncD\n");
  iopPreamble();

  std::string name = vmfp()->func()->fullName()->data();
  auto& sinks = Configuration::get()->sinks;
  if (sinks.find(name) != sinks.end() &&
      State::get()->stack.top() == kTestSource) {
    FTRACE(1, "taint: {}: tainted value flows into sink\n", pcOff());
    State::get()->issues.push_back({kTestSource, name});
  }
}

void iopFCallObjMethod() {
  iopUnhandled("FCallObjMethod");
}

void iopFCallObjMethodD() {
  iopUnhandled("FCallObjMethodD");
}

void iopIterInit() {
  iopUnhandled("IterInit");
}

void iopLIterInit() {
  iopUnhandled("LIterInit");
}

void iopIterNext() {
  iopUnhandled("IterNext");
}

void iopLIterNext() {
  iopUnhandled("LIterNext");
}

void iopIterFree() {
  iopUnhandled("IterFree");
}

void iopLIterFree() {
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

void iopBareThis() {
  iopUnhandled("BareThis");
}

void iopCheckThis() {
  iopUnhandled("CheckThis");
}

void iopChainFaults() {
  iopUnhandled("ChainFaults");
}

void iopOODeclExists() {
  iopUnhandled("OODeclExists");
}

void iopVerifyOutType() {
  iopUnhandled("VerifyOutType");
}

void iopVerifyParamType() {
  iopUnhandled("VerifyParamType");
}

void iopVerifyParamTypeTS() {
  iopUnhandled("VerifyParamTypeTS");
}

void iopVerifyRetTypeC() {
  iopUnhandled("VerifyRetTypeC");
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

void iopNativeImpl() {
  iopUnhandled("NativeImpl");
}

void iopCreateCl() {
  iopUnhandled("CreateCl");
}

void iopCreateCont() {
  iopUnhandled("CreateCont");
}

void iopContEnter() {
  iopUnhandled("ContEnter");
}

void iopContRaise() {
  iopUnhandled("ContRaise");
}

void iopYield() {
  iopUnhandled("Yield");
}

void iopYieldK() {
  iopUnhandled("YieldK");
}

void iopContCheck() {
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

void iopAwait() {
  iopUnhandled("Await");
}

void iopAwaitAll() {
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

void iopCheckProp() {
  iopUnhandled("CheckProp");
}

void iopInitProp() {
  iopUnhandled("InitProp");
}

void iopSilence() {
  iopUnhandled("Silence");
}

void iopThrowNonExhaustiveSwitch() {
  iopUnhandled("ThrowNonExhaustiveSwitch");
}

void iopRaiseClassStringConversionWarning() {
  iopUnhandled("RaiseClassStringConversionWarning");
}

void iopBaseGC() {
  iopUnhandled("BaseGC");
}

void iopBaseGL() {
  iopUnhandled("BaseGL");
}

void iopBaseSC() {
  iopUnhandled("BaseSC");
}

void iopBaseL() {
  iopUnhandled("BaseL");
}

void iopBaseC() {
  iopUnhandled("BaseC");
}

void iopBaseH() {
  iopUnhandled("BaseH");
}

void iopDim() {
  iopUnhandled("Dim");
}

void iopQueryM() {
  iopUnhandled("QueryM");
}

void iopSetM() {
  iopUnhandled("SetM");
}

void iopSetRangeM() {
  iopUnhandled("SetRangeM");
}

void iopIncDecM() {
  iopUnhandled("IncDecM");
}

void iopSetOpM() {
  iopUnhandled("SetOpM");
}

void iopUnsetM() {
  iopUnhandled("UnsetM");
}

void iopMemoGet() {
  iopUnhandled("MemoGet");
}

void iopMemoGetEager() {
  iopUnhandled("MemoGetEager");
}

void iopMemoSet() {
  iopUnhandled("MemoSet");
}

void iopMemoSetEager() {
  iopUnhandled("MemoSetEager");
}

} // namespace taint
} // namespace HPHP

#endif
