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

#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/cti.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/source-location.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/functional.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

#include <algorithm>
#include <atomic>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

std::atomic<bool> Func::s_treadmill;

Mutex g_funcsMutex;

/*
 * FuncId high water mark and FuncId -> Func* table.
 * We can't start with 0 since that's used for special sentinel value
 * in TreadHashMap
 */
static std::atomic<FuncId::Int> s_nextFuncId{1};
#ifndef USE_LOWPTR
AtomicLowPtrVector<const Func> Func::s_funcVec{0, nullptr};
static InitFiniNode s_funcVecReinit([]{
  UnsafeReinitEmptyAtomicLowPtrVector(
    Func::s_funcVec, RuntimeOption::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_funcVec reinit");
#endif

static ServiceData::ExportedCounter* s_funcid_counter =
  ServiceData::createCounter("admin.func_ids");
static InitFiniNode s_funcidCounterInit([]{
  // see comment on s_nextFuncId for why we start at 1.
  s_funcid_counter->setValue(1);
}, InitFiniNode::When::PostRuntimeOptions, "func_id counter init");

namespace {
inline int numProloguesForNumParams(int numParams) {
  // The number of prologues is numParams + 2. The extra 2 are needed for
  // the following cases:
  //   - arguments passed > numParams
  //   - no arguments passed
  return numParams + 2;
}
}

///////////////////////////////////////////////////////////////////////////////
// Creation and destruction.

Func::Func(Unit& unit, const StringData* name, Attr attrs)
  : m_name(name)
  , m_isPreFunc(false)
  , m_hasPrivateAncestor(false)
  , m_shouldSampleJit(StructuredLog::coinflip(RuntimeOption::EvalJitSampleRate))
  , m_hasForeignThis(false)
  , m_registeredInDataMap(false)
  , m_unit(&unit)
  , m_shared(nullptr)
  , m_attrs(attrs)
{
}

Func::Func(
  Unit& unit, const StringData* name, Attr attrs,
  const StringData *methCallerCls, const StringData *methCallerMeth)
  : m_name(name)
  , m_methCallerMethName(to_low(methCallerMeth, kMethCallerBit))
  , m_u(methCallerCls)
  , m_isPreFunc(false)
  , m_hasPrivateAncestor(false)
  , m_shouldSampleJit(StructuredLog::coinflip(RuntimeOption::EvalJitSampleRate))
  , m_hasForeignThis(false)
  , m_registeredInDataMap(false)
  , m_unit(&unit)
  , m_shared(nullptr)
  , m_attrs(attrs)
{
  assertx(methCallerCls != nullptr);
  assertx(methCallerMeth != nullptr);
}

Func::~Func() {
  // Should've deregistered in Func::destroy() or Func::freeClone()
  assertx(!m_registeredInDataMap);
#ifndef NDEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void* Func::allocFuncMem(int numParams) {
  int numPrologues = numProloguesForNumParams(numParams);

  auto const funcSize =
    sizeof(Func) + numPrologues * sizeof(m_prologueTable[0])
    - sizeof(m_prologueTable);

  MemoryStats::LogAlloc(AllocKind::Func, funcSize);
  return lower_malloc(funcSize);
}

void Func::destroy(Func* func) {
  if (jit::mcgen::initialized() && RuntimeOption::EvalEnableReusableTC) {
    // Free TC-space associated with func
    jit::tc::reclaimFunction(func);
  }

  if (func->m_registeredInDataMap) {
    func->deregisterInDataMap();
  }

#ifndef USE_LOWPTR
  if (!func->m_funcId.isInvalid()) {
    assertx(s_funcVec.get(func->m_funcId.toInt()) == func);
    s_funcVec.set(func->m_funcId.toInt(), nullptr);
    func->m_funcId = {FuncId::Invalid};
  }
#endif

  if (s_treadmill.load(std::memory_order_acquire)) {
    Treadmill::enqueue([func](){
      func->~Func();
      lower_free(func);
    });
    return;
  }

  func->~Func();
  lower_free(func);
}

void Func::freeClone() {
  assertx(isPreFunc());
  assertx(m_cloned.flag.test_and_set());

  if (jit::mcgen::initialized() && RuntimeOption::EvalEnableReusableTC) {
    // Free TC-space associated with func
    jit::tc::reclaimFunction(this);
  }

  if (m_registeredInDataMap) {
    deregisterInDataMap();
  }

#ifndef USE_LOWPTR
  if (!m_funcId.isInvalid()) {
    assertx(s_funcVec.get(m_funcId.toInt()) == this);
    s_funcVec.set(m_funcId.toInt(), nullptr);
    m_funcId = {FuncId::Invalid};
  }
#endif

  m_cloned.flag.clear();
}

Func* Func::clone(Class* cls, const StringData* name) const {
  auto numParams = this->numParams();

  // If this is a PreFunc (i.e., a Func on a PreClass) that is not already
  // being used as a regular Func by a Class, and we aren't trying to change
  // its name (since the name is part of the template for later clones), we can
  // reuse this same Func as the clone.
  bool const can_reuse =
    m_isPreFunc && !name && !m_cloned.flag.test_and_set();

  Func* f = !can_reuse
    ? new (allocFuncMem(numParams)) Func(*this)
    : const_cast<Func*>(this);

  f->m_cloned.flag.test_and_set();
  f->initPrologues(numParams);
  f->m_funcEntry = nullptr;
  if (name) f->m_name = name;
  f->m_u.setCls(cls);
  f->setFullName(numParams);

  if (f != this) {
    f->m_isPreFunc = false;
    f->m_registeredInDataMap = false;
  }

#ifndef USE_LOWPTR
  f->m_funcId = {FuncId::Invalid};
#endif
  f->setNewFuncId();
  return f;
}

void Func::rescope(Class* ctx) {
  m_u.setCls(ctx);
  setFullName(numParams());
}

///////////////////////////////////////////////////////////////////////////////
// Initialization.

void Func::init(int numParams) {
#ifndef NDEBUG
  m_magic = kMagic;
#endif
  setNewFuncId();
  // For methods, we defer setting the full name until m_cls is initialized
  if (!preClass()) {
    setFullName(numParams);
  } else {
    m_fullName = nullptr;
  }
  if (isSpecial(m_name)) {
    /*
     * We dont want these compiler generated functions to
     * appear in backtraces.
     */
    m_attrs = m_attrs | AttrNoInjection;
  }
  assertx(m_name);
  initPrologues(numParams);
}

void Func::initPrologues(int numParams) {
  int numPrologues = numProloguesForNumParams(numParams);

  if (!jit::mcgen::initialized()) {
    for (int i = 0; i < numPrologues; i++) {
      m_prologueTable[i] = nullptr;
    }
    return;
  }

  auto const& stubs = jit::tc::ustubs();

  TRACE(4, "initPrologues func %p %d\n", this, numPrologues);
  for (int i = 0; i < numPrologues; i++) {
    m_prologueTable[i] = stubs.fcallHelperThunk;
  }
}

void Func::setFullName(int /*numParams*/) {
  assertx(m_name->isStatic());
  Class *clazz = cls();
  if (clazz) {
    m_fullName = (StringData*)kNeedsFullName;
  } else {
    m_fullName = m_name.get();

    // A scoped closure may not have a `cls', but we still need to preserve its
    // `methodSlot', which refers to its slot in its `baseCls' (which still
    // points to a subclass of Closure).
    if (!isMethod()) {
      setNamedFunc(NamedFunc::getOrCreate(m_name));
    }
  }
}

/* This function is expected to be called after all calls to appendParam
 * are complete. After, m_paramCounts is initialized such that the least
 * significant bit of this->m_paramCounts indicates whether the last param
 * is (non)variadic; and the rest of the bits are the number of params.
 */
void Func::finishedEmittingParams(std::vector<ParamInfo>& fParams) {
  assertx(m_paramCounts == 0);
  assertx(m_inoutBits == 0);

  // Initialize m_paramCounts.
  shared()->m_params = fParams;
  m_paramCounts = fParams.size() << 1;
  if (!(m_attrs & AttrVariadicParam)) {
    m_paramCounts |= 1;
  }
  assertx(numParams() == fParams.size());

  // Build m_inoutBits.
  for (auto i = 0u; i < fParams.size(); ++i) {
    if (LIKELY(!fParams[i].isInOut())) continue;
    m_inoutBits |= 1u << std::min(i, kInoutFastCheckBits);
  }
}

void Func::registerInDataMap() {
#ifndef USE_LOWPTR
  assertx(!m_funcId.isInvalid());
#endif
  assertx((!m_isPreFunc || m_cloned.flag.test_and_set()));
  assertx(!m_registeredInDataMap);
  assertx(mallocEnd());
  data_map::register_start(this);
  m_registeredInDataMap = true;
}

void Func::deregisterInDataMap() {
  assertx(m_registeredInDataMap);
  assertx((!m_isPreFunc || m_cloned.flag.test_and_set()));
#ifndef USE_LOWPTR
  assertx(!m_funcId.isInvalid());
#endif
  data_map::deregister(this);
  m_registeredInDataMap = false;
}

bool Func::isMemoizeImplName(const StringData* name) {
  return name->size() > 13 && !memcmp(name->data() + name->size() - 13,
                                      "$memoize_impl", 13);
}

const StringData* Func::genMemoizeImplName(const StringData* origName) {
  return makeStaticString(folly::sformat("{}$memoize_impl", origName->data()));
}

std::pair<const StringData*, const StringData*> Func::getMethCallerNames(
  const StringData* name) {
  assertx(name->size() > 11 && !memcmp(name->data(), "MethCaller$", 11));
  auto clsMethName = name->slice();
  clsMethName.uncheckedAdvance(11);
  auto const sep = folly::qfind(clsMethName, folly::StringPiece("$"));
  assertx(sep != std::string::npos);
  auto cls = clsMethName.uncheckedSubpiece(0, sep);
  auto meth = clsMethName.uncheckedSubpiece(sep + 1);
  return std::make_pair(makeStaticString(cls), makeStaticString(meth));
}

///////////////////////////////////////////////////////////////////////////////
// FuncId manipulation.

FuncId::Int Func::maxFuncIdNum() {
  return s_nextFuncId.load(std::memory_order_relaxed);
}

#ifdef USE_LOWPTR
void Func::setNewFuncId() {
  s_nextFuncId.fetch_add(1, std::memory_order_relaxed);
  s_funcid_counter->increment();
}

const Func* Func::fromFuncId(FuncId id) {
  auto const func = id.getFunc();
  func->validate();
  return func;
}

bool Func::isFuncIdValid(FuncId id) {
  return !id.isInvalid() && !id.isDummy();
}
#else
void Func::setNewFuncId() {
  assertx(m_funcId.isInvalid());
  m_funcId = {s_nextFuncId.fetch_add(1, std::memory_order_relaxed)};
  s_funcid_counter->increment();

  s_funcVec.ensureSize(m_funcId.toInt() + 1);
  assertx(s_funcVec.get(m_funcId.toInt()) == nullptr);
  s_funcVec.set(m_funcId.toInt(), this);
}

const Func* Func::fromFuncId(FuncId id) {
  assertx(id.toInt() < s_nextFuncId);
  auto const func = s_funcVec.get(id.toInt());
  func->validate();
  return func;
}

bool Func::isFuncIdValid(FuncId id) {
  if (id.toInt() >= s_nextFuncId) return false;
  return s_funcVec.get(id.toInt()) != nullptr;
}
#endif


///////////////////////////////////////////////////////////////////////////////
// Bytecode.

bool Func::isEntry(Offset offset) const {
  return offset == 0 || isDVEntry(offset);
}

bool Func::isDVEntry(Offset offset) const {
  auto const nparams = numNonVariadicParams();
  for (int i = 0; i < nparams; i++) {
    const ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue() && pi.funcletOff == offset) return true;
  }
  return false;
}

bool Func::hasNonTrivialDVFuncEntry() const {
  auto const nparams = numNonVariadicParams();
  for (auto i = numRequiredParams(); i < nparams; ++i) {
    if (!params()[i].hasTrivialDefaultValue()) return true;
  }
  return false;
}

Offset Func::getEntryForNumArgs(int numArgsPassed) const {
  assertx(numArgsPassed >= 0);
  auto const nparams = numNonVariadicParams();
  for (unsigned i = numArgsPassed; i < nparams; i++) {
    const Func::ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue()) {
      return pi.funcletOff;
    }
  }
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Parameters.

bool Func::isInOut(int32_t arg) const {
  assertx(arg >= 0);
  if (LIKELY(arg < kInoutFastCheckBits)) {
    return m_inoutBits & (1ull << arg);
  }

  if (arg >= numParams()) return false;
  return params()[arg].isInOut();
}

bool Func::isReadonly(int32_t arg) const {
  assertx(arg >= 0);
  if (arg >= numParams()) return false;
  return params()[arg].isReadonly();
}

uint32_t Func::numInOutParams() const {
  uint32_t count = folly::popcount(m_inoutBits);
  if (LIKELY(static_cast<int32_t>(m_inoutBits) >= 0)) {
    // The sign bit is not set, so only args 0..31 can possibly be inout.
    return count;
  }

  assertx(numParams() > kInoutFastCheckBits);
  for (auto i = numParams() - 1; i >= kInoutFastCheckBits; --i) {
    count += params()[i].isInOut() ? 1 : 0;
  }
  // Minus one for the sign bit.
  return count - 1;
}

uint32_t Func::numInOutParamsForArgs(int32_t numArgs) const {
  if (!takesInOutParams()) return 0;
  uint32_t i = 0;
  for (int p = 0; p < numArgs; ++p) i += isInOut(p);
  return i;
}

///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and stack.

Id Func::lookupVarId(const StringData* name) const {
  assertx(name != nullptr);
  return shared()->m_localNames.findIndex(name);
}

uint32_t Func::numClosureUseLocals() const {
  assertx(isClosureBody());
  auto const cls = implCls();
  return cls->numDeclProperties() - (cls->hasClosureCoeffectsProp() ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence.

bool Func::isImmutableFrom(const Class* cls) const {
  if (!RuntimeOption::RepoAuthoritative) return false;
  assertx(cls && cls->lookupMethod(name()) == this);
  if (attrs() & AttrNoOverride) {
    return true;
  }
  if (cls->preClass()->attrs() & AttrNoOverride) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// JIT data.

int Func::numPrologues() const {
  return numProloguesForNumParams(numParams());
}

void Func::resetPrologue(int numParams) {
  auto const& stubs = jit::tc::ustubs();
  m_prologueTable[numParams] = stubs.fcallHelperThunk;
}

void Func::resetFuncEntry() {
  m_funcEntry = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Reified Generics

namespace {
const ReifiedGenericsInfo k_defaultReifiedGenericsInfo{0, false, 0, {}};
} // namespace

const ReifiedGenericsInfo& Func::getReifiedGenericsInfo() const {
  if (!shared()->m_allFlags.m_hasReifiedGenerics) return k_defaultReifiedGenericsInfo;
  auto const ex = extShared();
  assertx(ex);
  return ex->m_reifiedGenericsInfo;
}

///////////////////////////////////////////////////////////////////////////////
// Pretty printer.

void Func::print_attrs(std::ostream& out, Attr attrs) {
  if (attrs & AttrStatic)    { out << " static"; }
  if (attrs & AttrPublic)    { out << " public"; }
  if (attrs & AttrProtected) { out << " protected"; }
  if (attrs & AttrPrivate)   { out << " private"; }
  if (attrs & AttrInternal)  { out << " internal"; }
  if (attrs & AttrAbstract)  { out << " abstract"; }
  if (attrs & AttrFinal)     { out << " final"; }
  if (attrs & AttrNoOverride){ out << " (no-override)"; }
  if (attrs & AttrInterceptable) { out << " (interceptable)"; }
  if (attrs & AttrPersistent) { out << " (persistent)"; }
  if (attrs & AttrBuiltin) { out << " (builtin)"; }
  if (attrs & AttrIsFoldable) { out << " (foldable)"; }
  if (attrs & AttrNoInjection) { out << " (no_injection)"; }
  if (attrs & AttrSupportsAsyncEagerReturn) { out << " (can_async_eager_ret)"; }
  if (attrs & AttrDynamicallyCallable) { out << " (dyn_callable)"; }
  if (attrs & AttrIsMethCaller) { out << " (is_meth_caller)"; }
  if (attrs & AttrNoRecording) { out << " (no_recording)"; }
}

void Func::prettyPrint(std::ostream& out, const PrintOpts& opts) const {
  if (opts.name) {
    if (preClass() != nullptr) {
      out << "Method";
      print_attrs(out, m_attrs);
      if (isPhpLeafFn()) out << " (leaf)";
      if (isMemoizeWrapper()) out << " (memoize_wrapper)";
      if (isMemoizeWrapperLSB()) out << " (memoize_wrapper_lsb)";
      if (cls() != nullptr) {
        out << ' ' << fullName()->data();
      } else {
        out << ' ' << preClass()->name()->data() << "::" << m_name->data();
      }
    } else {
      out << "Function";
      print_attrs(out, m_attrs);
      if (isPhpLeafFn()) out << " (leaf)";
      if (isMemoizeWrapper()) out << " (memoize_wrapper)";
      if (isMemoizeWrapperLSB()) out << " (memoize_wrapper_lsb)";
      out << ' ' << m_name->data();
    }

    out << std::endl;
  }

  if (opts.metadata) {
    const ParamInfoVec& params = shared()->m_params;
    for (uint32_t i = 0; i < params.size(); ++i) {
      auto const& param = params[i];
      out << " Param: " << localVarName(i)->data();
      if (param.typeConstraint.hasConstraint()) {
        out << " " << param.typeConstraint.displayName(cls(), true);
      }
      if (param.userType) {
        out << " (" << param.userType->data() << ")";
      }
      if (param.funcletOff != kInvalidOffset) {
        out << " DV" << " at " << param.funcletOff;
        if (param.phpCode) {
          out << " = " << param.phpCode->data();
        }
      }
      out << std::endl;
    }

    if (returnTypeConstraint().hasConstraint() ||
        (returnUserType() && !returnUserType()->empty())) {
      out << " Ret: ";
      if (returnTypeConstraint().hasConstraint()) {
        out << " " << returnTypeConstraint().displayName(cls(), true);
      }
      if (returnUserType() && !returnUserType()->empty()) {
        out << " (" << returnUserType()->data() << ")";
      }
      out << std::endl;
    }

    if (repoReturnType().tag() != RepoAuthType::Tag::Cell) {
      out << "repoReturnType: " << show(repoReturnType()) << '\n';
    }
    if (repoAwaitedReturnType().tag() != RepoAuthType::Tag::Cell) {
      out << "repoAwaitedReturnType: " << show(repoAwaitedReturnType()) << '\n';
    }
    out << "maxStackCells: " << maxStackCells() << '\n'
        << "numLocals: " << numLocals() << '\n'
        << "numIterators: " << numIterators() << '\n';

    const EHEntVec& ehtab = shared()->m_ehtab;
    size_t ehId = 0;
    for (auto it = ehtab.begin(); it != ehtab.end(); ++it, ++ehId) {
      out << " EH " << ehId << " Catch for " <<
        it->m_base << ":" << it->m_past;
      if (it->m_parentIndex != -1) {
        out << " outer EH " << it->m_parentIndex;
      }
      if (it->m_iterId != -1) {
        out << " iterId " << it->m_iterId;
      }
      out << " handle at " << it->m_handler;
      if (it->m_end != kInvalidOffset) {
        out << ":" << it->m_end;
      }
      if (it->m_parentIndex != -1) {
        out << " parentIndex " << it->m_parentIndex;
      }
      out << std::endl;
    }
  }

  if (opts.startOffset != kInvalidOffset) {
    auto startOffset = std::max(0, opts.startOffset);
    auto stopOffset = std::min(bclen(), opts.stopOffset);

    if (startOffset >= stopOffset) {
      return;
    }

    auto it = at(startOffset);
    auto const stop = at(stopOffset);
    int prevLineNum = -1;
    while (it < stop) {
      if (opts.showLines) {
        auto const lineNum =
          SourceLocation::getLineNumber(getOrLoadLineTable(), offsetOf(it));
        if (lineNum != prevLineNum) {
          out << "  // line " << lineNum << std::endl;
          prevLineNum = lineNum;
        }
      }

      out << std::string(opts.indentSize, ' ')
          << std::setw(4) << offsetOf(it) << ": "
          << instrToString(it, this)
          << std::endl;
      it += instrLen(it);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// SharedData.

Func::SharedData::SharedData(BCPtr bc, Offset bclen,
                             PreClass* preClass, int sn, int line1,
                             int line2, bool isPhpLeafFn)
  : m_bc(bc.isPtr() ? BCPtr::FromPtr(allocateBCRegion(bc.ptr(), bclen)) : bc)
  , m_preClass(preClass)
  , m_line1(line1)
  , m_originalFilename(nullptr)
  , m_cti_base(0)
  , m_numLocals(0)
  , m_numIterators(0)
{
  m_allFlags.m_isClosureBody = false;
  m_allFlags.m_isAsync = false;
  m_allFlags.m_isGenerator = false;
  m_allFlags.m_isPairGenerator = false;
  m_allFlags.m_isGenerated = false;
  m_allFlags.m_hasExtendedSharedData = false;
  m_allFlags.m_returnByValue = false;
  m_allFlags.m_isMemoizeWrapper = false;
  m_allFlags.m_isMemoizeWrapperLSB = false;
  m_allFlags.m_memoizeICType = Func::MemoizeICType::NoIC;
  m_allFlags.m_isPhpLeafFn = isPhpLeafFn;
  m_allFlags.m_hasReifiedGenerics = false;
  m_allFlags.m_hasParamsWithMultiUBs = false;
  m_allFlags.m_hasReturnWithMultiUBs = false;

  m_bclenSmall = std::min<uint32_t>(bclen, kSmallDeltaLimit);
  m_line2Delta = std::min<uint32_t>(line2 - line1, kSmallDeltaLimit);
  m_sn = std::min<uint32_t>(sn, kSmallDeltaLimit);
}

Func::SharedData::~SharedData() {
  if (auto bc = m_bc.copy(); bc.isPtr()) {
    freeBCRegion(bc.ptr(), bclen());
  }
  if (auto table = m_lineTable.copy(); table.isPtr()) {
    delete table.ptr();
  }
  Func::s_extendedLineInfo.erase(this);
  if (m_cti_base) free_cti(m_cti_base, m_cti_size);
}

void Func::SharedData::atomicRelease() {
  if (UNLIKELY(m_allFlags.m_hasExtendedSharedData)) {
    delete (ExtendedSharedData*)this;
  } else {
    delete this;
  }
}

Func::ExtendedSharedData::~ExtendedSharedData() {
}

///////////////////////////////////////////////////////////////////////////////

void logFunc(const Func* func, StructuredLogEntry& ent) {
  auto const attrs = attrs_to_vec(AttrContext::Func, func->attrs());
  std::set<folly::StringPiece> attrSet(attrs.begin(), attrs.end());

  if (func->isMemoizeWrapper()) attrSet.emplace("memoize_wrapper");
  if (func->isMemoizeWrapperLSB()) attrSet.emplace("memoize_wrapper_lsb");
  if (func->isMemoizeImpl()) attrSet.emplace("memoize_impl");
  if (func->isAsync()) attrSet.emplace("async");
  if (func->isGenerator()) attrSet.emplace("generator");
  if (func->isClosureBody()) attrSet.emplace("closure_body");
  if (func->isPairGenerator()) attrSet.emplace("pair_generator");
  if (func->hasVariadicCaptureParam()) attrSet.emplace("variadic_param");
  if (func->isPhpLeafFn()) attrSet.emplace("leaf_function");
  if (func->cls() && func->cls()->isPersistent()) attrSet.emplace("persistent");

  ent.setSet("func_attributes", attrSet);

  ent.setInt("num_params", func->numNonVariadicParams());
  ent.setInt("num_locals", func->numLocals());
  ent.setInt("num_iterators", func->numIterators());
  ent.setInt("frame_cells", func->numSlotsInFrame());
  ent.setInt("max_stack_cells", func->maxStackCells());
}

///////////////////////////////////////////////////////////////////////////////
// Lookup

const StaticString s_DebuggerMain("__DebuggerMain");

void Func::def(Func* func) {
  assertx(!func->isMethod());

  // Don't define the __debugger_main() function
  DEBUGGER_ATTACHED_ONLY(if (func->userAttributes().count(s_DebuggerMain.get())) { return; });

  auto const ne = func->getNamedFunc();

  Func* f = ne->getCachedFunc();
  if (f == nullptr) {
    std::unique_lock<Mutex> l(g_funcsMutex);
    f = ne->getCachedFunc();
    if (f == nullptr) {
      auto const persistent = func->isPersistent();
      assertx(!persistent || (RuntimeOption::RepoAuthoritative || func->unit()->isSystemLib()));

      if (!ne->m_cachedFunc.bound()) {
        ne->m_cachedFunc.bind(
          persistent ? rds::Mode::Persistent : rds::Mode::Normal,
          rds::LinkName{"Func", func->name()}
        );
      }
      ne->m_cachedFunc.initWith(func);
      l.unlock();

      DEBUGGER_ATTACHED_ONLY(phpDebuggerDefFuncHook(func));
    }
  }

  // If the function didn't exists before we are good
  if (f == nullptr) {
    return;
  }

  // Otherwise check if we match or show the right error message
  if (f == func) {
    assertx(!RO::RepoAuthoritative ||
            (f->isPersistent() && ne->m_cachedFunc.isPersistent()));
    return;
  }
  if (func->attrs() & AttrIsMethCaller) return;
  if (f->isBuiltin()) {
    assertx(!func->isBuiltin());
    raise_error(Strings::REDECLARE_BUILTIN, func->name()->data());
  }
  raise_error(Strings::FUNCTION_ALREADY_DEFINED, func->name()->data());
}

Func* Func::lookup(const NamedFunc* ne) {
  return ne->getCachedFunc();
}

Func* Func::lookup(const StringData* name) {
  const NamedFunc* ne = NamedFunc::getNoCreate(name);
  return ne ? ne->getCachedFunc() : nullptr;
}

Func* Func::lookupBuiltin(const StringData* name) {
  // Builtins are either persistent (the normal case), or defined at the
  // beginning of every request (if JitEnableRenameFunction or interception is
  // enabled). In either case, they're unique, so they should be present in the
  // NamedFunc.
  auto const ne = NamedFunc::getNoCreate(name);
  if (!ne) return nullptr;
  auto const f = ne->getCachedFunc();
  return (f && f->isPersistent() && f->isBuiltin()) ? f : nullptr;
}

Func* Func::load(const NamedFunc* ne, const StringData* name) {
  Func* func = ne->getCachedFunc();
  if (LIKELY(func != nullptr)) return func;
  if (AutoloadHandler::s_instance->autoloadFunc(
        const_cast<StringData*>(name))) {
    func = ne->getCachedFunc();
  }
  return func;
}

Func* Func::load(const StringData* name) {
  String normStr;
  auto ne = NamedFunc::getNoCreate(name, &normStr);
  if (!ne) return nullptr;

  // Try to fetch from cache
  Func* func_ = ne->getCachedFunc();
  if (LIKELY(func_ != nullptr)) return func_;

  // Normalize the namespace
  if (normStr) {
    name = normStr.get();
  }

  // Autoload the function
  return AutoloadHandler::s_instance->autoloadFunc(
    const_cast<StringData*>(name)
  ) ? ne->getCachedFunc() : nullptr;
}

namespace {
void handleModuleBoundaryViolation(const Func* callee, const Func* caller) {
  if (!callee || !caller) return;
  if (will_symbol_raise_module_boundary_violation(callee, caller)) {
    raiseModuleBoundaryViolation(nullptr, callee, caller->moduleName());
  }
  auto const& packageInfo = g_context->getPackageInfo();
  if (RO::EvalEnforceDeployment &&
      caller->moduleName() != callee->moduleName() &&
      !packageInfo.violatesDeploymentBoundary(caller->moduleName()) &&
      packageInfo.violatesDeploymentBoundary(*callee)) {
    raiseDeploymentBoundaryViolation(callee);
  }
}
} // namespace

Func* Func::resolve(const NamedFunc* ne, const StringData* name,
                    const Func* callerFunc) {
  Func* func = load(ne, name);
  handleModuleBoundaryViolation(func, callerFunc);
  return func;
}

Func* Func::resolve(const StringData* name, const Func* callerFunc) {
  Func* func = load(name);
  handleModuleBoundaryViolation(func, callerFunc);
  return func;
}

///////////////////////////////////////////////////////////////////////////////
// Code locations.

Func::ExtendedLineInfoCache Func::s_extendedLineInfo;

void Func::setLineTable(LineTable lineTable) {
  auto& table = shared()->m_lineTable;
  auto lock = table.lock_for_update();
  assertx(table.copy().isPtr() && !table.copy().ptr());
  lock.update(
    LineTablePtr::FromPtr(new LineTable{std::move(lineTable)})
  );
}

void Func::setLineTable(LineTablePtr::Token token) {
  assertx(RO::RepoAuthoritative);
  auto& table = shared()->m_lineTable;
  auto lock = table.lock_for_update();
  assertx(table.copy().isPtr() && !table.copy().ptr());
  lock.update(LineTablePtr::FromToken(token));
}

void Func::stashExtendedLineTable(SourceLocTable table) const {
  ExtendedLineInfoCache::accessor acc;
  if (s_extendedLineInfo.insert(acc, shared())) {
    acc->second.sourceLocTable = std::move(table);
  }
}

/*
 * Return the Unit's SourceLocTable, extracting it from the repo if
 * necessary.
 */
const SourceLocTable& Func::getLocTable() const {
  auto const sharedData = shared();
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, sharedData)) {
      return acc->second.sourceLocTable;
    }
  }
  static SourceLocTable empty;
  return empty;
}

/*
 * Return a copy of the Func's line to OffsetRangeVec table.
 */
LineToOffsetRangeVecMap Func::getLineToOffsetRangeVecMap() const {
  auto const sharedData = shared();
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, sharedData)) {
      if (!acc->second.lineToOffsetRange.empty()) {
        return acc->second.lineToOffsetRange;
      }
    }
  }

  LineToOffsetRangeVecMap map;
  auto const& srcLocTable = getLocTable();
  SourceLocation::generateLineToOffsetRangesMap(srcLocTable, map);

  ExtendedLineInfoCache::accessor acc;
  if (!s_extendedLineInfo.find(acc, sharedData)) {
    always_assert_flog(0, "ExtendedLineInfoCache was not found when it should "
      "have been");
  }
  if (acc->second.lineToOffsetRange.empty()) {
    acc->second.lineToOffsetRange = std::move(map);
  }
  return acc->second.lineToOffsetRange;
}

const LineTable* Func::getLineTable() const {
  auto const table = shared()->m_lineTable.copy();
  if (table.isPtr()) {
    assertx(table.ptr());
    return table.ptr();
  }
  return nullptr;
}

const LineTable& Func::getOrLoadLineTable() const {
  if (auto const table = getLineTable()) return *table;

  assertx(RO::RepoAuthoritative);

  auto& wrapper = shared()->m_lineTable;
  auto lock = wrapper.lock_for_update();

  auto const table = wrapper.copy();
  if (table.isPtr()) return *table.ptr();

  auto newTable = new LineTable{
    FuncEmitter::loadLineTableFromRepo(m_unit->sn(), table.token())
  };
  lock.update(LineTablePtr::FromPtr(newTable));
  return *newTable;
}

LineTable Func::getOrLoadLineTableCopy() const {
  auto const table = shared()->m_lineTable.copy();
  if (table.isPtr()) {
    assertx(table.ptr());
    return *table.ptr();
  }
  assertx(RO::RepoAuthoritative);
  return FuncEmitter::loadLineTableFromRepo(m_unit->sn(), table.token());
}

int Func::getLineNumber(Offset offset) const {
  auto const findLine = [&] {
    // lineMap is an atomically acquired bitwise copy of m_lineMap,
    // with no destructor
    auto lineMap(shared()->m_lineMap.get());
    if (lineMap->empty()) return INT_MIN;
    auto const it = std::upper_bound(
      lineMap->begin(), lineMap->end(),
      offset,
      [] (Offset info, const LineInfo& elm) {
        return info < elm.first.past;
      }
    );
    if (it != lineMap->end() && it->first.base <= offset) return it->second;
    return INT_MIN;
  };

  auto line = findLine();
  if (line != INT_MIN) return line;

  // Updating m_lineMap while coverage is enabled can cause the
  // treadmill to fill with an enormous number of resized maps.
  if (UNLIKELY(g_context && (m_unit->isCoverageEnabled() ||
                             RID().getCoverage()))) {
    return SourceLocation::getLineNumber(getOrLoadLineTable(), offset);
  }

  auto lock = shared()->m_lineMap.lock_for_update();
  line = findLine();
  if (line != INT_MIN) return line;

  auto const info = SourceLocation::getLineInfo(getOrLoadLineTable(), offset);
  auto copy = shared()->m_lineMap.copy();
  auto const it = std::upper_bound(
    copy.begin(), copy.end(),
    info,
    [&] (const LineInfo& a, const LineInfo& b) {
      return a.first.base < b.first.past;
    }
  );
  assertx(it == copy.end() ||
          (it->first.past > offset && it->first.base > offset));
  copy.insert(it, info);
  auto old = lock.update(std::move(copy));
  Treadmill::enqueue([old = std::move(old)] () mutable { old.clear(); });
  return info.second;
}

bool Func::getSourceLoc(Offset offset, SourceLoc& sLoc) const {
  auto const& sourceLocTable = getLocTable();
  return SourceLocation::getLoc(sourceLocTable, offset, sLoc);
}

bool Func::getOffsetRange(Offset offset, OffsetRange& range) const {
  auto line = getLineNumber(offset);
  if (line == -1) return false;

  auto map = getLineToOffsetRangeVecMap();
  auto it = map.find(line);
  if (it != map.end()) {
    for (auto o : it->second) {
      if (offset >= o.base && offset < o.past) {
        range = o;
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode

unsigned char*
allocateBCRegion(const unsigned char* bc, size_t bclen) {
  g_hhbc_size->addValue(bclen);
  auto arena = get_swappable_readonly_arena();
  auto mem = static_cast<unsigned char*>(
    arena ? arena->allocate(bclen) : malloc(bclen));
  std::copy(bc, bc + bclen, mem);
  return mem;
}

void freeBCRegion(const unsigned char* bc, size_t bclen) {
  // Can't free bytecode arena memory.
  if (RuntimeOption::RepoAuthoritative) return;

  if (debug) {
    // poison released bytecode
    memset(const_cast<unsigned char*>(bc), 0xff, bclen);
  }
  free(const_cast<unsigned char*>(bc));
  g_hhbc_size->addValue(-int64_t(bclen));
}

PC Func::loadBytecode() {
  assertx(RO::RepoAuthoritative);
  auto& wrapper = shared()->m_bc;
  auto lock = wrapper.lock_for_update();
  auto const bc = wrapper.copy();
  if (bc.isPtr()) return bc.ptr();
  auto const length = bclen();
  g_hhbc_size->addValue(length);
  auto arena = get_swappable_readonly_arena();
  auto mem = static_cast<unsigned char*>(
    arena ? arena->allocate(length) : malloc(length));
  RepoFile::readRawFromUnit(m_unit->sn(), bc.token(), mem, length);
  lock.update(BCPtr::FromPtr(mem));
  return mem;
}

///////////////////////////////////////////////////////////////////////////////
// Coverage

namespace {
RDS_LOCAL(uint32_t, tl_saved_coverage_index);
RDS_LOCAL_NO_CHECK(Array, tl_called_functions);
rds::Link<uint32_t, rds::Mode::Local> s_coverage_index;

using CoverageLinkMap = tbb::concurrent_hash_map<
  const StringData*,
  rds::Link<uint32_t, rds::Mode::Local>
>;

struct EmbeddedCoverageLinkMap {
  explicit operator bool() const { return inited; }

  CoverageLinkMap* operator->() {
    assertx(inited);
    return reinterpret_cast<CoverageLinkMap*>(&data);
  }
  CoverageLinkMap& operator*() { assertx(inited); return *operator->(); }

  void emplace(uint32_t size) {
    assertx(!inited);
    new (&data) CoverageLinkMap(size);
    inited = true;
  }

  void clear() {
    if (inited) {
      operator*().~CoverageLinkMap();
      inited = false;
    }
  }

private:
  typename std::aligned_storage<
    sizeof(CoverageLinkMap),
    alignof(CoverageLinkMap)
  >::type data;
  bool inited;
};

EmbeddedCoverageLinkMap s_covLinks;
static InitFiniNode s_covLinksReinit([]{
  if (RO::RepoAuthoritative || !RO::EvalEnableFuncCoverage) return;
  s_covLinks.emplace(RO::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_funcVec reinit");

InitFiniNode s_clear_called_functions([]{
  tl_called_functions.nullOut();
  if (s_coverage_index.bound()) *s_coverage_index = 0;
}, InitFiniNode::When::RequestFini, "tl_called_functions clear");
}

rds::Handle Func::GetCoverageIndex() {
  if (!s_coverage_index.bound()) {
    s_coverage_index.bind(rds::Mode::Local, rds::LinkID{"FuncCoverageIndex"});
  }
  return s_coverage_index.handle();
}

rds::Handle Func::getCoverageHandle() const {
  assertx(!RO::RepoAuthoritative && RO::EvalEnableFuncCoverage);
  assertx(!isNoInjection() && !isMethCaller());

  CoverageLinkMap::const_accessor cnsAcc;
  if (s_covLinks->find(cnsAcc, fullName())) {
    assertx(cnsAcc->second.bound());
    return cnsAcc->second.handle();
  }

  CoverageLinkMap::accessor acc;
  if (s_covLinks->insert(acc, fullName())) {
    assertx(!acc->second.bound());
    acc->second.bind(
      rds::Mode::Local, rds::LinkName{"FuncCoverageFlag", fullName()}
    );
  }
  assertx(acc->second.bound());
  return acc->second.handle();
}

void Func::EnableCoverage() {
  assertx(g_context);

  if (RO::RepoAuthoritative) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot enable function call coverage in repo authoritative mode"
    );
  }
  if (!RO::EvalEnableFuncCoverage) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot enable function call coverage (you must set "
      "Eval.EnableFuncCoverage = true)"
    );
  }
  if (!tl_called_functions.isNull()) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Function call coverage already enabled"
    );
  }

  GetCoverageIndex(); // bind the handle
  if (!*tl_saved_coverage_index) *tl_saved_coverage_index = 1;
  *s_coverage_index = (*tl_saved_coverage_index)++;
  tl_called_functions.emplace(Array::CreateDict());
}

rds::Handle Func::debuggerIntrSetHandle() const {
  assertx(shared()->m_funcHasDebuggerIntr.bound());
  return shared()->m_funcHasDebuggerIntr.handle();
}

rds::Link<bool, rds::Mode::Normal> Func::debuggerIntrSetLink() const {
  assertx(shared()->m_funcHasDebuggerIntr.bound());
  return shared()->m_funcHasDebuggerIntr;
}

void Func::ensureDebuggerIntrSetLinkBound() const {
  if (!shared()->m_funcHasDebuggerIntr.bound()) {
    shared()->m_funcHasDebuggerIntr.bind(
      rds::Mode::Normal,
      rds::LinkName{"FunctionDebuggerIntr", name()}
    );
  }
}

std::string show(PrologueID pid) {
  auto func = pid.func();
  return folly::sformat("{}(id 0x{:#x}) # of args: {}",
                        func->fullName()->data(),
                        pid.funcId().toInt(), pid.nargs());
}

Array Func::GetCoverage() {
  if (tl_called_functions.isNull()) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Function call coverage not enabled"
    );
  }

  auto const ret = std::move(*tl_called_functions.get());
  *s_coverage_index = 0;
  tl_called_functions.destroy();
  return ret;
}

void Func::recordCall() const {
  if (RO::RepoAuthoritative || !RO::EvalEnableFuncCoverage) return;
  if (tl_called_functions.isNull()) return;
  if (isNoInjection() || isMethCaller()) return;

  auto const path = unit()->isSystemLib()
    ? empty_string()
    : StrNR{unit()->filepath()}.asString();

  tl_called_functions->set(fullNameStr().asString(), std::move(path), true);
}

void Func::recordCallNoCheck() const {
  assertx(!RO::RepoAuthoritative && RO::EvalEnableFuncCoverage);
  assertx(!tl_called_functions.isNull());
  assertx(tl_called_functions->isDict());
  assertx(!isNoInjection() && !isMethCaller());
  assertx(!tl_called_functions->exists(fullNameStr().asString(), true));

  auto const path = unit()->isSystemLib()
    ? empty_string()
    : StrNR{unit()->filepath()}.asString();

  tl_called_functions->set(fullNameStr().asString(), std::move(path), true);
}

///////////////////////////////////////////////////////////////////////////////
}
