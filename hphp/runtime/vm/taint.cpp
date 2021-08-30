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
  State::get()->initialize();
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
  // TODO(T93491972): replace with assertions once we can run the integration tests.
  if (m_stack.empty()) {
    FTRACE(3, "taint: (WARNING) called `Stack::top()` on empty stack\n");
    return kNoSource;
  }
  return m_stack.back();
}

void Stack::pop(int n) {
  if (m_stack.size() < n) {
    FTRACE(
        3,
        "taint: (WARNING) called `Stack::pop({})` on stack of size {}\n",
        n,
        m_stack.size());
    n = m_stack.size();
  }

  for (int i = 0; i < n; i++) {
    m_stack.pop_back();
  }
}

void Stack::replaceTop(Source source) {
  if (m_stack.empty()) {
    FTRACE(3, "taint: (WARNING) called `Stack::replaceTop()` on empty stack\n");
    return;
  }
  m_stack.back() = source;
}

size_t Stack::size() const {
  return m_stack.size();
}

std::string Stack::show() const {
  std::stringstream stream("(-> top) ");
  for (int i = 0; i < m_stack.size(); i++) {
    stream << m_stack[i];
    if (i != m_stack.size() - 1) {
      stream << ", ";
    }
  }
  return stream.str();
}

void Stack::clear() {
  m_stack.clear();
}

void Heap::set(tv_lval to, Source source) {
  FTRACE(2, "taint: setting lval to `{}`\n", source);
  m_heap[to] = source;
}

Optional<Source> Heap::get(tv_lval from) {
  FTRACE(2, "taint: getting from lval\n");

  auto source = m_heap.find(from);
  if (source != m_heap.end()) {
    return source->second;
  } else {
    return std::nullopt;
  }
}

void Heap::clear() {
  m_heap.clear();
}

folly::Singleton<State, StateSingletonTag> kStateSingleton{};
/* static */ std::shared_ptr<State> State::get() {
  return kStateSingleton.try_get();
}

void State::initialize() {
  // Stack is initialized with 4 values before any operation happens.
  // We don't care about these values but mirroring simplifies
  // consistency checks.
  for (int i = 0; i < 1000; i++) {
    stack.push(kNoSource);
  }
}

void State::reset() {
  stack.clear();
  heap.clear();
  issues.clear();
  initialize();
}

namespace {

void iopPreamble(const std::string& name) {
  auto vm_stack_size = vmStack().count();
  FTRACE(1, "taint: iop{}\n", name);
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
        "taint: (WARNING) stacks out of sync (shadow stack size: {})\n",
        shadow_stack_size);
  }
  FTRACE(4, "taint: stack: {}\n", State::get()->stack.show());
}

void iopUnhandled(const std::string& name) {
  iopPreamble(name);
  FTRACE(1, "taint: (WARNING) unhandled opcode\n");
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
  iopUnhandled("Dup");
}

void iopCGetCUNop() {
  iopUnhandled("CGetCUNop");
}

void iopUGetCUNop() {
  iopUnhandled("UGetCUNop");
}

void iopNull() {
  iopPreamble("Null");
  State::get()->stack.push(kNoSource);
}

void iopNullUninit() {
  iopPreamble("NullUninit");
  State::get()->stack.push(kNoSource);
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

void iopInt(int64_t /* imm */) {
  iopPreamble("Int");
  State::get()->stack.push(kNoSource);
}

void iopDouble(double /* imm */) {
  iopUnhandled("Double");
}

void iopString(const StringData* /* s */) {
  iopUnhandled("String");
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

void iopNewRecord(const StringData* /* s */, imm_array<int32_t> /* ids */) {
  iopUnhandled("NewRecord");
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

void iopSwitch(PC /* origpc */, PC& /* pc */, SwitchKind /* kind */, int64_t /* base */,
               imm_array<Offset> /* jmptab */) {
  iopUnhandled("Switch");
}

void iopSSwitch(PC /* origpc */, PC& /* pc */, imm_array<StrVecItem> /* jmptab */) {
  iopUnhandled("SSwitch");
}

void iopRetC(PC& /* pc */) {
  iopPreamble("RetC");

  auto func = vmfp()->func();
  State::get()->stack.pop(2 + func->params().size());

  std::string name = func->fullName()->data();
  auto& sources = Configuration::get()->sources;
  if (sources.find(name) != sources.end()) {
    FTRACE(1, "taint: {}: function returns `TestSource`\n", pcOff());
    State::get()->stack.replaceTop(kTestSource);
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
  if (!value) {
    value = kNoSource;
  }
  state->stack.push(*value);
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
  state->heap.set(to, state->stack.top());
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

void iopResolveClsMethodD(Id /* classId */,
                          const StringData* /* methName */) {
  iopUnhandled("ResolveClsMethodD");
}

void iopResolveClsMethodS(SpecialClsRef /* ref */,
                          const StringData* /* methName */) {
  iopUnhandled("ResolveClsMethodS");
}

void iopResolveRClsMethod(const StringData* /* methName */) {
  iopUnhandled("ResolveRClsMethod");
}

void iopResolveRClsMethodD(Id /* classId */,
                           const StringData* /* methName */) {
  iopUnhandled("ResolveRClsMethodD");
}

void iopResolveRClsMethodS(SpecialClsRef /* ref */,
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
  iopUnhandled("NewObjD");
}

void iopNewObjRD(Id /* id */) {
  iopUnhandled("NewObjRD");
}

void iopNewObjS(SpecialClsRef /* ref */) {
  iopUnhandled("NewObjS");
}

void iopLockObj() {
  iopUnhandled("LockObj");
}

TCA iopFCallClsMethod(bool /* retToJit */,
                      PC /* origpc */,
                      PC& /* pc */,
                      const FCallArgs& /* fca */,
                      const StringData*,
                      IsLogAsDynamicCallOp /* op */) {
  iopUnhandled("FCallClsMethod");
  return nullptr;
}

TCA iopFCallClsMethodD(bool /* retToJit */,
                       PC /* origpc */,
                       PC& /* pc */,
                       const FCallArgs& /* fca */,
                       const StringData*,
                       Id /* classId */,
                       const StringData* /* methName */) {
  iopUnhandled("FCallClsMethodD");
  return nullptr;
}

TCA iopFCallClsMethodS(bool /* retToJit */,
                       PC /* origpc */,
                       PC& /* pc */,
                       const FCallArgs& /* fca */,
                       const StringData*,
                       SpecialClsRef /* ref */) {
  iopUnhandled("FCallClsMethodS");
  return nullptr;
}

TCA iopFCallClsMethodSD(bool /* retToJit */,
                        PC /* origpc */,
                        PC& /* pc */,
                        const FCallArgs& /* fca */,
                        const StringData*,
                        SpecialClsRef /* ref */,
                        const StringData* /* methName */) {
  iopUnhandled("FCallClsMethodSD");
  return nullptr;
}

TCA iopFCallCtor(bool /* retToJit */,
                 PC /* origpc */,
                 PC& /* pc */,
                 const FCallArgs& /* fca */,
                 const StringData*) {
  iopUnhandled("FCallCtor");
  return nullptr;
}

TCA iopFCallFunc(bool /* retToJit */,
                 PC /* origpc */,
                 PC& /* pc */,
                 const FCallArgs& /* fca */) {
  iopUnhandled("FCallFunc");
  return nullptr;
}

TCA iopFCallFuncD(bool /* retToJit */,
                  PC /* origpc */,
                  PC& /* pc */,
                  const FCallArgs& /* fca */,
                  Id id) {
  iopPreamble("FCallFuncD");

  auto const nep = vmfp()->unit()->lookupNamedEntityPairId(id);
  auto const func = Func::load(nep.second, nep.first);
  auto name = func->fullName()->data();

  auto& sinks = Configuration::get()->sinks;
  if (sinks.find(name) != sinks.end() &&
      State::get()->stack.top() == kTestSource) {
    FTRACE(1, "taint: {}: tainted value flows into sink\n", pcOff());
    std::cerr << "tainted value flows into sink" << std::endl;
    State::get()->issues.push_back({kTestSource, name});
  }

  return nullptr;
}

TCA iopFCallObjMethod(bool /* retToJit */,
                      PC /* origpc */,
                      PC& /* pc */,
                      const FCallArgs& /* fca */,
                      const StringData*,
                      ObjMethodOp /* op */) {
  iopUnhandled("FCallObjMethod");
  return nullptr;
}

TCA iopFCallObjMethodD(bool /* retToJit */,
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

void iopLIterInit(PC& /* pc */,
                  const IterArgs& /* ita */,
                  TypedValue* /* base */,
                  PC /* targetpc */) {
  iopUnhandled("LIterInit");
}

void iopIterNext(PC& /* pc */, const IterArgs& /* ita */, PC /* targetpc */) {
  iopUnhandled("IterNext");
}

void iopLIterNext(PC& /* pc */,
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

void iopVerifyParamType(local_var /* param */) {
  iopUnhandled("VerifyParamType");
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

void iopBaseSC(uint32_t /* keyIdx */,
               uint32_t /* clsIdx */,
               MOpMode /* mode */,
               ReadonlyOp /* op */) {
  iopUnhandled("BaseSC");
}

void iopBaseL(named_local_var /* loc */,
              MOpMode /* mode */,
              ReadonlyOp /* op */) {
  iopUnhandled("BaseL");
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

void iopQueryM(uint32_t /* nDiscard */,
               QueryMOp /* subop */,
               MemberKey /* mk */) {
  iopUnhandled("QueryM");
}

void iopSetM(uint32_t /* nDiscard */, MemberKey /* mk */) {
  iopUnhandled("SetM");
}

void iopSetRangeM(uint32_t /* nDiscard */,
                  uint32_t /* size */,
                  SetRangeOp /* op */) {
  iopUnhandled("SetRangeM");
}

void iopIncDecM(uint32_t /* nDiscard */,
                IncDecOp /* subop */,
                MemberKey /* mk */) {
  iopUnhandled("IncDecM");
}

void iopSetOpM(uint32_t /* nDiscard */,
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

void iopMemoGetEager(PC& /* pc */,
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
