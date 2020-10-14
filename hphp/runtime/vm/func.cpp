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
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/rx.h"
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

/*
 * FuncId high water mark and FuncId -> Func* table.
 * We can't start with 0 since that's used for special sentinel value
 * in TreadHashMap
 */
static std::atomic<FuncId> s_nextFuncId{1};
AtomicLowPtrVector<const Func> Func::s_funcVec{0, nullptr};
static InitFiniNode s_funcVecReinit([]{
  UnsafeReinitEmptyAtomicLowPtrVector(
    Func::s_funcVec, RuntimeOption::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_funcVec reinit");

const AtomicLowPtrVector<const Func>& Func::getFuncVec() {
  return s_funcVec;
}

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
  , m_serialized(false)
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
  , m_serialized(false)
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
  if (m_fullName != nullptr && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameStr(), &m_maybeIntercepted);
  }
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
  if (func->m_funcId != InvalidFuncId) {
    if (jit::mcgen::initialized() && RuntimeOption::EvalEnableReusableTC) {
      // Free TC-space associated with func
      jit::tc::reclaimFunction(func);
    }

    assertx(s_funcVec.get(func->m_funcId) == func);
    s_funcVec.set(func->m_funcId, nullptr);

    if (func->m_registeredInDataMap) {
      func->deregisterInDataMap();
    }
    func->m_funcId = InvalidFuncId;

    if (s_treadmill.load(std::memory_order_acquire)) {
      Treadmill::enqueue([func](){ destroy(func); });
      return;
    }
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

  if (m_funcId != InvalidFuncId) {
    assertx(s_funcVec.get(m_funcId) == this);
    s_funcVec.set(m_funcId, nullptr);
    if (m_registeredInDataMap) {
      deregisterInDataMap();
    }
    m_funcId = InvalidFuncId;
  }

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
  f->m_funcBody = nullptr;
  f->m_funcId = InvalidFuncId;
  if (name) f->m_name = name;
  f->m_u.setCls(cls);
  f->setFullName(numParams);

  if (f != this) {
    f->m_cachedFunc = rds::Link<LowPtr<Func>, rds::Mode::NonLocal>{};
    f->m_maybeIntercepted = -1;
    f->m_isPreFunc = false;
    f->m_registeredInDataMap = false;
  }

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
  // For methods, we defer setting the full name until m_cls is initialized
  m_maybeIntercepted = -1;
  if (!preClass()) {
    setNewFuncId();
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
      setNamedEntity(NamedEntity::get(m_name));
    }
  }
}

void Func::appendParam(bool ref, const Func::ParamInfo& info,
                       std::vector<ParamInfo>& pBuilder) {
  auto numParams = pBuilder.size();

  // When called by FuncEmitter, the least significant bit of m_paramCounts
  // are not yet being used as a variadic flag, so numParams() cannot be
  // used
  const int qword = numParams / kBitsPerQword;
  const int bit   = numParams % kBitsPerQword;
  assertx(!info.isVariadic() || (m_attrs & AttrVariadicParam));
  uint64_t* refBits = &m_inoutBitVal;
  // Grow args, if necessary.
  if (qword) {
    if (bit == 0) {
      shared()->m_inoutBitPtr = (uint64_t*)
        realloc(shared()->m_inoutBitPtr, qword * sizeof(uint64_t));
    }
    refBits = shared()->m_inoutBitPtr + qword - 1;
  }

  if (bit == 0) {
    *refBits = 0;
  }

  assertx(!(*refBits & (uint64_t(1) << bit)));
  *refBits |= uint64_t(ref) << bit;
  pBuilder.push_back(info);
}

/* This function is expected to be called after all calls to appendParam
 * are complete. After, m_paramCounts is initialized such that the least
 * significant bit of this->m_paramCounts indicates whether the last param
 * is (non)variadic; and the rest of the bits are the number of params.
 */
void Func::finishedEmittingParams(std::vector<ParamInfo>& fParams) {
  assertx(m_paramCounts == 0);
  assertx(fParams.size() || (!m_inoutBitVal && !shared()->m_inoutBitPtr));

  shared()->m_params = fParams;
  m_paramCounts = fParams.size() << 1;
  if (!(m_attrs & AttrVariadicParam)) {
    m_paramCounts |= 1;
  }
  assertx(numParams() == fParams.size());
}

void Func::registerInDataMap() {
  assertx(m_funcId != InvalidFuncId &&
          (!m_isPreFunc || m_cloned.flag.test_and_set()));
  assertx(!m_registeredInDataMap);
  assertx(mallocEnd());
  data_map::register_start(this);
  m_registeredInDataMap = true;
}

void Func::deregisterInDataMap() {
  assertx(m_registeredInDataMap);
  assertx(m_funcId != InvalidFuncId &&
          (!m_isPreFunc || m_cloned.flag.test_and_set()));
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

void Func::setNewFuncId() {
  assertx(m_funcId == InvalidFuncId);
  m_funcId = s_nextFuncId.fetch_add(1, std::memory_order_relaxed);

  s_funcVec.ensureSize(m_funcId + 1);
  assertx(s_funcVec.get(m_funcId) == nullptr);
  s_funcVec.set(m_funcId, this);
}

FuncId Func::nextFuncId() {
  return s_nextFuncId.load(std::memory_order_relaxed);
}

const Func* Func::fromFuncId(FuncId id) {
  assertx(id < s_nextFuncId);
  auto func = s_funcVec.get(id);
  func->validate();
  return func;
}

bool Func::isFuncIdValid(FuncId id) {
  if (id >= s_nextFuncId) return false;
  return s_funcVec.get(id) != nullptr;
}


///////////////////////////////////////////////////////////////////////////////
// Bytecode.

bool Func::isEntry(Offset offset) const {
  return offset == base() || isDVEntry(offset);
}

bool Func::isDVEntry(Offset offset) const {
  auto const nparams = numNonVariadicParams();
  for (int i = 0; i < nparams; i++) {
    const ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue() && pi.funcletOff == offset) return true;
  }
  return false;
}

int Func::getEntryNumParams(Offset offset) const {
  if (offset == base()) return numNonVariadicParams();
  return getDVEntryNumParams(offset);
}

int Func::getDVEntryNumParams(Offset offset) const {
  auto const nparams = numNonVariadicParams();
  for (int i = 0; i < nparams; i++) {
    const ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue() && pi.funcletOff == offset) return i;
  }
  return -1;
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
  return base();
}


///////////////////////////////////////////////////////////////////////////////
// Parameters.

bool Func::takesInOutParams() const {
  if (m_inoutBitVal) {
    return true;
  }

  if (UNLIKELY(numParams() > kBitsPerQword)) {
    auto limit = argToQword(numParams() - 1);
    assertx(limit >= 0);
    for (int i = 0; i <= limit; ++i) {
      if (shared()->m_inoutBitPtr[i]) {
        return true;
      }
    }
  }
  return false;
}

bool Func::isInOut(int32_t arg) const {
  const uint64_t* ref = &m_inoutBitVal;
  assertx(arg >= 0);
  if (UNLIKELY(arg >= kBitsPerQword)) {
    if (arg >= numParams()) {
      return false;
    }
    ref = &shared()->m_inoutBitPtr[argToQword(arg)];
  }
  int bit = (uint32_t)arg % kBitsPerQword;
  return *ref & (1ull << bit);
}

uint32_t Func::numInOutParams() const {
  uint32_t count = folly::popcount(m_inoutBitVal);

  if (UNLIKELY(numParams() > kBitsPerQword)) {
    auto limit = argToQword(numParams() - 1);
    assertx(limit >= 0);
    for (int i = 0; i <= limit; ++i) {
      count += folly::popcount(shared()->m_inoutBitPtr[i]);
    }
  }
  return count;
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

void Func::resetFuncBody() {
  m_funcBody = nullptr;
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
  if (attrs & AttrAbstract)  { out << " abstract"; }
  if (attrs & AttrFinal)     { out << " final"; }
  if (attrs & AttrNoOverride){ out << " (nooverride)"; }
  if (attrs & AttrInterceptable) { out << " (interceptable)"; }
  if (attrs & AttrPersistent) { out << " (persistent)"; }
  if (attrs & AttrBuiltin) { out << " (builtin)"; }
  if (attrs & AttrIsFoldable) { out << " (foldable)"; }
  if (attrs & AttrNoInjection) { out << " (no_injection)"; }
  if (attrs & AttrSupportsAsyncEagerReturn) { out << " (can_async_eager_ret)"; }
  if (attrs & AttrDynamicallyCallable) { out << " (dyn_callable)"; }
  if (attrs & AttrIsMethCaller) { out << " (is_meth_caller)"; }
  if (attrs & AttrNoContext) { out << " (no_context)"; }
  auto rxAttrString = rxAttrsToAttrString(attrs);
  if (rxAttrString) out << " (" << rxAttrString << ")";
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

    out << " at " << base();
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
    auto startOffset = std::max(base(), opts.startOffset);
    auto stopOffset = std::min(past(), opts.stopOffset);

    if (startOffset >= stopOffset) {
      return;
    }

    const auto bc = unit()->entry();
    const auto* it = &bc[startOffset];
    int prevLineNum = -1;
    while (it < &bc[stopOffset]) {
      if (opts.showLines) {
        int lineNum = unit()->getLineNumber(offsetOf(it));
        if (lineNum != prevLineNum) {
          out << "  // line " << lineNum << std::endl;
          prevLineNum = lineNum;
        }
      }

      out << std::string(opts.indentSize, ' ');
      prettyPrintInstruction(out, offsetOf(it));
      it += instrLen(it);
    }
  }
}

void Func::prettyPrintInstruction(std::ostream& out, Offset offset) const {
  const auto bc = unit()->entry();
  const auto* it = &bc[offset];
  out << std::setw(4) << (it - bc) << ": "
    << instrToString(it, this)
    << std::endl;
}


///////////////////////////////////////////////////////////////////////////////
// SharedData.

Func::SharedData::SharedData(PreClass* preClass, Offset base, Offset past,
                             int line1, int line2, bool isPhpLeafFn,
                             const StringData* docComment)
  : m_base(base)
  , m_preClass(preClass)
  , m_numLocals(0)
  , m_numIterators(0)
  , m_line1(line1)
  , m_docComment(docComment)
  , m_inoutBitPtr(0)
  , m_originalFilename(nullptr)
  , m_cti_base(0)
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
  m_allFlags.m_isPhpLeafFn = isPhpLeafFn;
  m_allFlags.m_hasReifiedGenerics = false;
  m_allFlags.m_isRxDisabled = false;
  m_allFlags.m_hasParamsWithMultiUBs = false;
  m_allFlags.m_hasReturnWithMultiUBs = false;
  m_allFlags.m_hasCoeffectRules = false;

  m_pastDelta = std::min<uint32_t>(past - base, kSmallDeltaLimit);
  m_line2Delta = std::min<uint32_t>(line2 - line1, kSmallDeltaLimit);
}

Func::SharedData::~SharedData() {
  free(m_inoutBitPtr);
  if (m_cti_base) free_cti(m_cti_base, m_cti_size);
}

void Func::SharedData::atomicRelease() {
  if (UNLIKELY(m_allFlags.m_hasExtendedSharedData)) {
    delete (ExtendedSharedData*)this;
  } else {
    delete this;
  }
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

void Func::def(Func* func, bool debugger) {
  assertx(!func->isMethod());
  auto const handle = func->funcHandle();

  if (UNLIKELY(debugger)) {
    // Don't define the __debugger_main() function
    if (func->userAttributes().count(s_DebuggerMain.get())) return;
  }

  if (rds::isPersistentHandle(handle)) {
    auto& funcAddr = rds::handleToRef<LowPtr<Func>,
                                      rds::Mode::Persistent>(handle);
    auto const oldFunc = funcAddr.get();
    if (oldFunc == func) return;
    if (UNLIKELY(oldFunc != nullptr)) {
      assertx(oldFunc->isBuiltin() && !func->isBuiltin());
      raise_error(Strings::REDECLARE_BUILTIN, func->name()->data());
    }
    funcAddr = func;
  } else {
    assertx(rds::isNormalHandle(handle));
    auto& funcAddr = rds::handleToRef<LowPtr<Func>, rds::Mode::Normal>(handle);
    if (!rds::isHandleInit(handle, rds::NormalTag{})) {
      rds::initHandle(handle);
    } else {
      if (funcAddr.get() == func) return;
      if (func->attrs() & AttrIsMethCaller) {
        // emit the duplicated meth_caller directly
        return;
      }
      raise_error(Strings::FUNCTION_ALREADY_DEFINED, func->name()->data());
    }
    funcAddr = func;
  }

  if (func->isUnique()) func->getNamedEntity()->setUniqueFunc(func);

  if (UNLIKELY(debugger)) phpDebuggerDefFuncHook(func);
}

Func* Func::lookup(const NamedEntity* ne) {
  return ne->getCachedFunc();
}

Func* Func::lookup(const StringData* name) {
  const NamedEntity* ne = NamedEntity::get(name);
  return ne->getCachedFunc();
}

Func* Func::lookupBuiltin(const StringData* name) {
  // Builtins are either persistent (the normal case), or defined at the
  // beginning of every request (if JitEnableRenameFunction or interception is
  // enabled). In either case, they're unique, so they should be present in the
  // NamedEntity.
  auto const ne = NamedEntity::get(name);
  auto const f = ne->uniqueFunc();
  return (f && f->isBuiltin()) ? f : nullptr;
}

Func* Func::load(const NamedEntity* ne, const StringData* name) {
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
  auto ne = NamedEntity::get(name, true, &normStr);

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

void Func::bind(Func *func) {
  assertx(!func->isMethod());
  auto const ne = func->getNamedEntity();

  auto const persistent =
    (RuntimeOption::RepoAuthoritative || !SystemLib::s_inited) &&
    (func->attrs() & AttrPersistent);

  auto const init_val = LowPtr<Func>(func);

  ne->m_cachedFunc.bind(
    persistent ? rds::Mode::Persistent : rds::Mode::Normal,
    rds::LinkName{"Func", func->name()},
    &init_val
  );
  if (func->isUnique() && func == ne->getCachedFunc()) {
    // we need to check that we actually were responsible for the bind here
    // before we set the uniqueFunc on `ne`.  this seems strange, but it's
    // because meth_caller funcs are unique but can have the same name.
    ne->setUniqueFunc(func);
  }
  func->setFuncHandle(ne->m_cachedFunc);
}

///////////////////////////////////////////////////////////////////////////////
}
