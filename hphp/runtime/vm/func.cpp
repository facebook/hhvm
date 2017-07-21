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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/functional.h"
#include "hphp/util/debug.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

#include <algorithm>
#include <atomic>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_set>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

const StringData*     Func::s___call       = makeStaticString("__call");
const StringData*     Func::s___callStatic = makeStaticString("__callStatic");
std::atomic<bool>     Func::s_treadmill;

/*
 * FuncId high water mark and FuncId -> Func* table.
 */
static std::atomic<FuncId> s_nextFuncId{0};
static AtomicVector<const Func*> s_funcVec{0, nullptr};
static InitFiniNode s_funcVecReinit([]{
  UnsafeReinitEmptyAtomicVector(
    s_funcVec, RuntimeOption::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_funcVec reinit");

const AtomicVector<const Func*>& Func::getFuncVec() {
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
  , m_unit(&unit)
  , m_attrs(attrs)
{
  m_isPreFunc = false;
  m_hasPrivateAncestor = false;
  m_shared = nullptr;
  m_shouldSampleJit = StructuredLog::coinflip(
      RuntimeOption::EvalJitSampleRate
  );

 assertx(IMPLIES(accessesCallerFrame(), isBuiltin() && !isMethod()));
}

Func::~Func() {
  if (m_fullName != nullptr && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameStr(), &m_maybeIntercepted);
  }
  if (jit::mcgen::initialized() && !RuntimeOption::EvalEnableReusableTC) {
    // If Reusable TC is enabled then the prologue may have already been smashed
    // and the memory may now be in use by another function.
    jit::clobberFuncGuards(this);
  }
#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void* Func::allocFuncMem(int numParams) {
  int numPrologues = numProloguesForNumParams(numParams);

  auto const funcSize =
    sizeof(Func) + numPrologues * sizeof(m_prologueTable[0])
    - sizeof(m_prologueTable);

  return low_malloc_data(funcSize);
}

void Func::destroy(Func* func) {
  if (func->m_funcId != InvalidFuncId) {
    if (jit::mcgen::initialized() && RuntimeOption::EvalEnableReusableTC) {
      // Free TC-space associated with func
      jit::tc::reclaimFunction(func);
    }

    DEBUG_ONLY auto oldVal = s_funcVec.exchange(func->m_funcId, nullptr);
    assert(oldVal == func);
    func->m_funcId = InvalidFuncId;

    if (RuntimeOption::EvalEnableReverseDataMap) {
      // We register Funcs to data_map in Func::init() and Func::clone(), both
      // of which are accompanied by calls to Func::setNewFuncId().
      data_map::deregister(func);
    }

    if (s_treadmill.load(std::memory_order_acquire)) {
      Treadmill::enqueue([func](){ destroy(func); });
      return;
    }
  }
  func->~Func();
  low_free_data(func);
}

void Func::freeClone() {
  assert(isPreFunc());
  assert(m_cloned.flag.test_and_set());

  if (jit::mcgen::initialized() && RuntimeOption::EvalEnableReusableTC) {
    // Free TC-space associated with func
    jit::tc::reclaimFunction(this);
  } else {
    jit::clobberFuncGuards(this);
  }

  if (m_funcId != InvalidFuncId) {
    DEBUG_ONLY auto oldVal = s_funcVec.exchange(m_funcId, nullptr);
    assert(oldVal == this);
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
  f->m_funcId = InvalidFuncId;
  if (name) f->m_name = name;
  f->m_cls = cls;
  f->setFullName(numParams);

  if (RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(f);
  }

  if (f != this) {
    f->m_cachedFunc = rds::Link<LowPtr<Func>>{rds::kInvalidHandle};
    f->m_maybeIntercepted = -1;
    f->m_isPreFunc = false;
  }

  return f;
}

void Func::rescope(Class* ctx, Attr attrs) {
  m_cls = ctx;
  if (attrs != AttrNone) {
    m_attrs = attrs;
    assertx(IMPLIES(accessesCallerFrame(), isBuiltin() && !isMethod()));
  }
  setFullName(numParams());
}

void Func::rename(const StringData* name) {
  m_name = name;
  setFullName(numParams());
  // load the renamed function
  Unit::loadFunc(this);
}


///////////////////////////////////////////////////////////////////////////////
// Initialization.

void Func::init(int numParams) {
  // For methods, we defer setting the full name until m_cls is initialized
  m_maybeIntercepted = -1;
  if (!preClass()) {
    setNewFuncId();
    setFullName(numParams);

    if (RuntimeOption::EvalEnableReverseDataMap) {
      data_map::register_start(this);
    }
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
#ifdef DEBUG
  m_magic = kMagic;
#endif
  assert(m_name);
  initPrologues(numParams);
}

void Func::initPrologues(int numParams) {
  int numPrologues = numProloguesForNumParams(numParams);

  if (!jit::mcgen::initialized()) {
    m_funcBody = nullptr;
    for (int i = 0; i < numPrologues; i++) {
      m_prologueTable[i] = nullptr;
    }
    return;
  }

  auto const& stubs = jit::tc::ustubs();

  m_funcBody = stubs.funcBodyHelperThunk;

  TRACE(2, "initPrologues func %p %d\n", this, numPrologues);
  for (int i = 0; i < numPrologues; i++) {
    m_prologueTable[i] = stubs.fcallHelperThunk;
  }
}

void Func::setFullName(int /*numParams*/) {
  assert(m_name->isStatic());
  if (m_cls) {
    m_fullName = makeStaticString(
      std::string(m_cls->name()->data()) + "::" + m_name->data());
  } else {
    m_fullName = m_name;

    // A scoped closure may not have a `cls', but we still need to preserve its
    // `methodSlot', which refers to its slot in its `baseCls' (which still
    // points to a subclass of Closure).
    if (!isMethod()) {
      setNamedEntity(NamedEntity::get(m_name));
    }
  }

  if (!RuntimeOption::RepoAuthoritative &&
      RuntimeOption::DynamicInvokeFunctions.count(m_fullName->data())) {
    m_attrs = Attr(m_attrs | AttrInterceptable);
  }
}

void Func::appendParam(bool ref, const Func::ParamInfo& info,
                       std::vector<ParamInfo>& pBuilder) {
  auto numParams = pBuilder.size();

  // When called by FuncEmitter, the least significant bit of m_paramCounts
  // are not yet being used as a variadic flag, so numParams() cannot be
  // used
  int qword = numParams / kBitsPerQword;
  int bit   = numParams % kBitsPerQword;
  assert(!info.isVariadic() || (m_attrs & AttrVariadicParam));
  uint64_t* refBits = &m_refBitVal;
  // Grow args, if necessary.
  if (qword) {
    if (bit == 0) {
      shared()->m_refBitPtr = (uint64_t*)
        realloc(shared()->m_refBitPtr, qword * sizeof(uint64_t));
    }
    refBits = shared()->m_refBitPtr + qword - 1;
  }

  if (bit == 0) {
    // The new word is either zerod or set to 1, depending on whether
    // we are one of the special builtins that takes variadic
    // reference arguments.  This is for use in the translator.
    *refBits = (m_attrs & AttrVariadicByRef) ? -1ull : 0;
  }

  assert(!(*refBits & (uint64_t(1) << bit)) == !(m_attrs & AttrVariadicByRef));
  *refBits &= ~(1ull << bit);
  *refBits |= uint64_t(ref) << bit;
  pBuilder.push_back(info);
}

/* This function is expected to be called after all calls to appendParam
 * are complete. After, m_paramCounts is initialized such that the least
 * significant bit of this->m_paramCounts indicates whether the last param
 * is (non)variadic; and the rest of the bits are the number of params.
 */
void Func::finishedEmittingParams(std::vector<ParamInfo>& fParams) {
  assert(m_paramCounts == 0);
  if (!fParams.size()) {
    assert(!m_refBitVal && !shared()->m_refBitPtr);
    m_refBitVal = attrs() & AttrVariadicByRef ? -1uLL : 0uLL;
  }

  shared()->m_params = fParams;
  m_paramCounts = fParams.size() << 1;
  if (!(m_attrs & AttrVariadicParam)) {
    m_paramCounts |= 1;
  }
  assert(numParams() == fParams.size());
}

bool Func::isMemoizeImplName(const StringData* name) {
  return name->size() > 13 && !memcmp(name->data() + name->size() - 13,
                                      "$memoize_impl", 13);
}

const StringData* Func::genMemoizeImplName(const StringData* origName) {
  return makeStaticString(folly::sformat("{}$memoize_impl", origName->data()));
}

///////////////////////////////////////////////////////////////////////////////
// FuncId manipulation.

void Func::setNewFuncId() {
  assert(m_funcId == InvalidFuncId);
  m_funcId = s_nextFuncId.fetch_add(1, std::memory_order_relaxed);

  s_funcVec.ensureSize(m_funcId + 1);
  DEBUG_ONLY auto oldVal = s_funcVec.exchange(m_funcId, this);
  assert(oldVal == nullptr);
}

FuncId Func::nextFuncId() {
  return s_nextFuncId.load(std::memory_order_relaxed);
}

const Func* Func::fromFuncId(FuncId id) {
  assert(id < s_nextFuncId);
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

DVFuncletsVec Func::getDVFunclets() const {
  DVFuncletsVec dvs;
  int nParams = numParams();
  for (int i = 0; i < nParams; ++i) {
    const ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue()) {
      dvs.push_back(std::make_pair(i, pi.funcletOff));
    }
  }
  return dvs;
}

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
  assert(numArgsPassed >= 0);
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

bool Func::anyByRef() const {
  if (m_refBitVal || (m_attrs & AttrVariadicByRef)) {
    return true;
  }

  if (UNLIKELY(numParams() >= kBitsPerQword)) {
    auto limit = ((uint32_t) numParams() / kBitsPerQword);
    for (int i = 0; i < limit; ++i) {
      if (shared()->m_refBitPtr[i]) {
        return true;
      }
    }
  }
  return false;
}

bool Func::byRef(int32_t arg) const {
  const uint64_t* ref = &m_refBitVal;
  assert(arg >= 0);
  if (UNLIKELY(arg >= kBitsPerQword)) {
    // Super special case. A handful of builtins are varargs functions where the
    // (not formally declared) varargs are pass-by-reference. psychedelic-kitten
    if (arg >= numParams()) {
      return m_attrs & AttrVariadicByRef;
    }
    ref = &shared()->m_refBitPtr[(uint32_t)arg / kBitsPerQword - 1];
  }
  int bit = (uint32_t)arg % kBitsPerQword;
  return *ref & (1ull << bit);
}

const StaticString s_extract("extract");
const StaticString s_current("current");
const StaticString s_key("key");
const StaticString s_array_multisort("array_multisort");

bool Func::mustBeRef(int32_t arg) const {
  if (!byRef(arg)) return false;
  if (arg == 0) {
    if (UNLIKELY(m_attrs & AttrBuiltin)) {
      // This hacks mustBeRef() to return false for the first parameter of
      // extract(), current(), key(), and array_multisort(). These functions
      // try to take their first parameter by reference but they also allow
      // expressions that cannot be taken by reference (ex. an array literal).
      // TODO Task #4442937: Come up with a cleaner way to do this.
      if (cls()) return true;
      auto const name = displayName();
      if (name == s_extract.get()) return false;
      if (name == s_current.get()) return false;
      if (name == s_key.get()) return false;
      if (name == s_array_multisort.get()) return false;
    }
  }

  // We force mustBeRef() to return false for array_multisort(). It tries to
  // pass all variadic arguments by reference, but it also allow expressions
  // that cannot be taken by reference (ex. SORT_REGULAR flag).
  return
    arg < numParams() ||
    !(m_attrs & AttrVariadicByRef) ||
    !(name() == s_array_multisort.get() && !cls());
}


///////////////////////////////////////////////////////////////////////////////
// Locals, iterators, and stack.

Id Func::lookupVarId(const StringData* name) const {
  assert(name != nullptr);
  return shared()->m_localNames.findIndex(name);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence.

bool Func::isImmutableFrom(const Class* cls) const {
  if (!RuntimeOption::RepoAuthoritative) return false;
  assert(cls && cls->lookupMethod(name()) == this);
  if (attrs() & AttrNoOverride) {
    // Even if the func isn't overridden, we clone it into
    // any derived classes if it has static locals
    if (!hasStaticLocals()) return true;
  }
  if (cls->preClass()->attrs() & AttrNoOverride) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Unit table entries.

const FPIEnt* Func::findFPI(const FPIEnt* b, const FPIEnt* e, Offset o) {
  /*
   * We consider the "FCall" instruction part of the FPI region, but
   * the corresponding push is not considered part of it.  (This
   * means all offsets in the FPI region will have the partial
   * ActRec on the stack.)
   */

  auto it =
    std::lower_bound(b, e, o, [](const FPIEnt& f, Offset o) {
      return f.m_fpushOff < o;
    });

  // Didn't find any candidates.
  if (it == b) {
    return nullptr;
  }

  const FPIEnt* fe = --it;

  // Iterate through parents until we find a valid region.
  while (true) {
    if (fe->m_fpiEndOff >= o) {
      return fe;
    }

    if (fe->m_parentIndex < 0) {
      return nullptr;
    }

    fe = &b[fe->m_parentIndex];
  }
}

const FPIEnt* Func::findPrecedingFPI(Offset o) const {
  assert(o >= base() && o < past());
  const FPIEntVec& fpitab = shared()->m_fpitab;
  assert(fpitab.size());
  const FPIEnt* fe = 0;
  for (unsigned i = 0; i < fpitab.size(); i++) {
    const FPIEnt* cur = &fpitab[i];
    if (o > cur->m_fpiEndOff &&
        (!fe || fe->m_fpiEndOff < cur->m_fpiEndOff)) {
      fe = cur;
    }
  }
  assert(fe);
  return fe;
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


///////////////////////////////////////////////////////////////////////////////
// Pretty printer.

static void print_attrs(std::ostream& out, Attr attrs) {
  if (attrs & AttrStatic)    { out << " static"; }
  if (attrs & AttrPublic)    { out << " public"; }
  if (attrs & AttrProtected) { out << " protected"; }
  if (attrs & AttrPrivate)   { out << " private"; }
  if (attrs & AttrAbstract)  { out << " abstract"; }
  if (attrs & AttrFinal)     { out << " final"; }
  if (attrs & AttrPhpLeafFn) { out << " (leaf)"; }
  if (attrs & AttrHot)       { out << " (hot)"; }
  if (attrs & AttrNoOverride){ out << " (nooverride)"; }
  if (attrs & AttrInterceptable) { out << " (interceptable)"; }
  if (attrs & AttrPersistent) { out << " (persistent)"; }
  if (attrs & AttrMayUseVV) { out << " (mayusevv)"; }
  if (attrs & AttrRequiresThis) { out << " (requiresthis)"; }
  if (attrs & AttrBuiltin) { out << " (builtin)"; }
  if (attrs & AttrReadsCallerFrame) { out << " (reads_caller_frame)"; }
  if (attrs & AttrWritesCallerFrame) { out << " (writes_caller_frame)"; }
  if (attrs & AttrSkipFrame) { out << " (skip_frame)"; }
  if (attrs & AttrIsFoldable) { out << " (foldable)"; }
  if (attrs & AttrNoInjection) { out << " (no_injection)"; }
}

void Func::prettyPrint(std::ostream& out, const PrintOpts& opts) const {
  if (isPseudoMain()) {
    out << "Pseudo-main";
  } else if (preClass() != nullptr) {
    out << "Method";
    print_attrs(out, m_attrs);
    if (isMemoizeWrapper()) out << " (memoize_wrapper)";
    if (cls() != nullptr) {
      out << ' ' << fullName()->data();
    } else {
      out << ' ' << preClass()->name()->data() << "::" << m_name->data();
    }
  } else {
    out << "Function";
    print_attrs(out, m_attrs);
    if (isMemoizeWrapper()) out << " (memoize_wrapper)";
    out << ' ' << m_name->data();
  }

  out << " at " << base();
  out << std::endl;

  if (!opts.metadata) return;

  const ParamInfoVec& params = shared()->m_params;
  for (uint32_t i = 0; i < params.size(); ++i) {
    auto const& param = params[i];
    out << " Param: " << localVarName(i)->data();
    if (param.typeConstraint.hasConstraint()) {
      out << " " << param.typeConstraint.displayName(this, true);
    }
    if (param.userType) {
      out << " (" << param.userType->data() << ")";
    }
    if (param.funcletOff != InvalidAbsoluteOffset) {
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
      out << " " << returnTypeConstraint().displayName(this, true);
    }
    if (returnUserType() && !returnUserType()->empty()) {
      out << " (" << returnUserType()->data() << ")";
    }
    out << std::endl;
  }

  if (repoReturnType().tag() != RepoAuthType::Tag::Gen) {
    out << "repoReturnType: " << show(repoReturnType()) << '\n';
  }
  if (repoAwaitedReturnType().tag() != RepoAuthType::Tag::Gen) {
    out << "repoAwaitedReturnType: " << show(repoAwaitedReturnType()) << '\n';
  }
  out << "maxStackCells: " << maxStackCells() << '\n'
      << "numLocals: " << numLocals() << '\n'
      << "numIterators: " << numIterators() << '\n'
      << "numClsRefSlots: " << numClsRefSlots() << '\n';

  if (auto const f = dynCallWrapper()) {
    out << "dynCallWrapper: " << f->fullName()->data() << '\n';
  }
  if (auto const f = dynCallTarget()) {
    out << "dynCallTarget: " << f->fullName()->data() << '\n';
  }

  const EHEntVec& ehtab = shared()->m_ehtab;
  size_t ehId = 0;
  for (auto it = ehtab.begin(); it != ehtab.end(); ++it, ++ehId) {
    bool catcher = it->m_type == EHEnt::Type::Catch;
    out << " EH " << ehId << " " << (catcher ? "Catch" : "Fault") << " for " <<
      it->m_base << ":" << it->m_past;
    if (it->m_parentIndex != -1) {
      out << " outer EH " << it->m_parentIndex;
    }
    if (it->m_iterId != -1) {
      out << " iterId " << it->m_iterId;
      out << " itRef " << (it->m_itRef ? "true" : "false");
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

  if (opts.fpi) {
    for (auto& fpi : fpitab()) {
      out << " FPI " << fpi.m_fpushOff << "-" << fpi.m_fpiEndOff
          << "; fpOff = " << fpi.m_fpOff;
      if (fpi.m_parentIndex != -1) {
        out << " parentIndex = " << fpi.m_parentIndex
            << " (depth " << fpi.m_fpiDepth << ")";
      }
      out << '\n';
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// SharedData.

Func::SharedData::SharedData(PreClass* preClass, Offset base, Offset past,
                             int line1, int line2, bool top,
                             const StringData* docComment)
  : m_base(base)
  , m_preClass(preClass)
  , m_numLocals(0)
  , m_numIterators(0)
  , m_line1(line1)
  , m_docComment(docComment)
  , m_refBitPtr(0)
  , m_top(top)
  , m_isClosureBody(false)
  , m_isAsync(false)
  , m_isGenerator(false)
  , m_isPairGenerator(false)
  , m_isGenerated(false)
  , m_hasExtendedSharedData(false)
  , m_returnByValue(false)
  , m_isMemoizeWrapper(false)
  , m_numClsRefSlots(0)
  , m_originalFilename(nullptr)
{
  m_pastDelta = std::min<uint32_t>(past - base, kSmallDeltaLimit);
  m_line2Delta = std::min<uint32_t>(line2 - line1, kSmallDeltaLimit);
}

Func::SharedData::~SharedData() {
  free(m_refBitPtr);
}

void Func::SharedData::atomicRelease() {
  if (UNLIKELY(m_hasExtendedSharedData)) {
    delete (ExtendedSharedData*)this;
  } else {
    delete this;
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace {

using FuncSet = std::unordered_set<std::string,string_hashi,string_eqstri>;

/*
 * This is a conservative list of functions that we are certain won't inspect
 * the caller frame (generally by either CallerFrame or vm_call_user_func).
 */
FuncSet s_ignores_frame = {
  "array_key_exists",
  "key_exists",
  "array_keys",
  "array_pop",
  "array_push",
  "array_rand",
  "array_search",
  "array_shift",
  "array_slice",
  "array_splice",
  "array_unique",
  "array_unshift",
  "array_values",
  "compact",
  "shuffle",
  "count",
  "sizeof",
  "each",
  "current",
  "in_array",
  "range",
  "sort",
  "rsort",
  "asort",
  "arsort",
  "ksort",
  "krsort",
  "natsort",
  "natcasesort",
  "hphp_array_idx",
  "ctype_alnum",
  "ctype_alpha",
  "ctype_cntrl",
  "ctype_digit",
  "ctype_graph",
  "ctype_lower",
  "ctype_print",
  "ctype_punct",
  "ctype_space",
  "ctype_upper",
  "ctype_xdigit",
  "fb_serialize",
  "fb_unserialize",
  "fb_compact_serialize",
  "fb_compact_unserialize",
  "fb_utf8ize",
  "fb_utf8_strlen",
  "fb_utf8_strlen_deprecated",
  "fb_utf8_substr",
  "fb_get_code_coverage",
  "fb_output_compression",
  "fb_set_exit_callback",
  "fb_get_last_flush_size",
  "fb_lazy_lstat",
  "fb_lazy_realpath",
  "hash",
  "hash_algos",
  "hash_file",
  "hash_final",
  "hash_init",
  "hash_update",
  "hash_copy",
  "hash_equals",
  "furchash_hphp_ext",
  "hphp_murmurhash",
  "get_declared_classes",
  "get_declared_interfaces",
  "get_declared_traits",
  "class_alias",
  "class_exists",
  "interface_exists",
  "trait_exists",
  "enum_exists",
  "get_class_methods",
  "get_class_constants",
  "is_a",
  "is_subclass_of",
  "method_exists",
  "property_exists",
  "error_log",
  "error_reporting",
  "restore_error_handler",
  "restore_exception_handler",
  "set_error_handler",
  "set_exception_handler",
  "hphp_set_error_page",
  "hphp_clear_unflushed",
  "get_defined_functions",
  "function_exists",
  "min",
  "max",
  "abs",
  "is_finite",
  "is_infinite",
  "is_nan",
  "ceil",
  "floor",
  "round",
  "deg2rad",
  "rad2deg",
  "decbin",
  "dechex",
  "decoct",
  "bindec",
  "hexdec",
  "octdec",
  "base_convert",
  "pow",
  "exp",
  "expm1",
  "log10",
  "log1p",
  "log",
  "cos",
  "cosh",
  "sin",
  "sinh",
  "tan",
  "tanh",
  "acos",
  "acosh",
  "asin",
  "asinh",
  "atan",
  "atanh",
  "atan2",
  "hypot",
  "fmod",
  "sqrt",
  "getrandmax",
  "srand",
  "rand",
  "mt_getrandmax",
  "mt_srand",
  "mt_rand",
  "lcg_value",
  "intdiv",
  "flush",
  "hphp_crash_log",
  "hphp_stats",
  "hphp_get_stats",
  "hphp_get_status",
  "hphp_get_iostatus",
  "hphp_set_iostatus_address",
  "hphp_get_timers",
  "hphp_output_global_state",
  "hphp_instruction_counter",
  "hphp_get_hardware_counters",
  "hphp_set_hardware_events",
  "hphp_clear_hardware_events",
  "wordwrap",
  "sprintf",
  "is_null",
  "is_bool",
  "is_int",
  "is_float",
  "is_numeric",
  "is_string",
  "is_scalar",
  "is_array",
  "HH\\is_vec",
  "HH\\is_dict",
  "HH\\is_keyset",
  "HH\\is_varray_or_darray",
  "is_object",
  "is_resource",
  "boolval",
  "intval",
  "floatval",
  "strval",
  "gettype",
  "get_resource_type",
  "settype",
  "serialize",
  "unserialize",
  "addcslashes",
  "stripcslashes",
  "addslashes",
  "stripslashes",
  "bin2hex",
  "hex2bin",
  "nl2br",
  "quotemeta",
  "str_shuffle",
  "strrev",
  "strtolower",
  "strtoupper",
  "ucfirst",
  "lcfirst",
  "ucwords",
  "strip_tags",
  "trim",
  "ltrim",
  "rtrim",
  "chop",
  "explode",
  "implode",
  "join",
  "str_split",
  "chunk_split",
  "strtok",
  "str_replace",
  "str_ireplace",
  "substr_replace",
  "substr",
  "str_pad",
  "str_repeat",
  "html_entity_decode",
  "htmlentities",
  "htmlspecialchars_decode",
  "htmlspecialchars",
  "fb_htmlspecialchars",
  "quoted_printable_encode",
  "quoted_printable_decode",
  "convert_uudecode",
  "convert_uuencode",
  "str_rot13",
  "crc32",
  "crypt",
  "md5",
  "sha1",
  "strtr",
  "convert_cyr_string",
  "get_html_translation_table",
  "hebrev",
  "hebrevc",
  "setlocale",
  "localeconv",
  "nl_langinfo",
  "chr",
  "ord",
  "money_format",
  "number_format",
  "strcmp",
  "strncmp",
  "strnatcmp",
  "strcasecmp",
  "strncasecmp",
  "strnatcasecmp",
  "strcoll",
  "substr_compare",
  "strchr",
  "strrchr",
  "strstr",
  "stristr",
  "strpbrk",
  "strpos",
  "stripos",
  "strrpos",
  "strripos",
  "substr_count",
  "strspn",
  "strcspn",
  "strlen",
  "str_getcsv",
  "count_chars",
  "str_word_count",
  "levenshtein",
  "similar_text",
  "soundex",
  "metaphone",
  "base64_decode",
  "base64_encode",
  "get_headers",
  "get_meta_tags",
  "http_build_query",
  "parse_url",
  "rawurldecode",
  "rawurlencode",
  "urldecode",
  "urlencode",
  "HH\\vec",
  "HH\\dict",
  "HH\\keyset",
  "HH\\varray",
  "HH\\darray",
  "HH\\array_key_cast"
};

const StaticString s_assert("assert");

}

bool disallowDynamicVarEnvFuncs() {
  return (RuntimeOption::RepoAuthoritative &&
          Repo::global().DisallowDynamicVarEnvFuncs) ||
    RuntimeOption::DisallowDynamicVarEnvFuncs == HackStrictOption::ON;
}

bool funcWritesLocals(const Func* callee) {
  assertx(callee != nullptr);

  // A skip-frame function can dynamically call a function which writes to the
  // caller's frame. If we don't forbid such dynamic calls, we have to be
  // pessimistic.
  if (callee->isSkipFrame() && !disallowDynamicVarEnvFuncs()) {
    return true;
  }

  if (!callee->writesCallerFrame()) return false;

  if (callee->fullName()->isame(s_assert.get())) {
    /*
     * Assert is somewhat special.  If RepoAuthoritative isn't set and the
     * first parameter is a string, it will be evaled and can have arbitrary
     * effects.  If the assert fails, it may execute an arbitrary
     * pre-registered callback which still might try to write to the assert
     * caller's frame.
     *
     * This can't happen if calling such frame accessing functions dynamically
     * is forbidden.
     */
    return !RuntimeOption::RepoAuthoritative || !disallowDynamicVarEnvFuncs();
  }
  return true;
}

bool funcReadsLocals(const Func* callee) {
  assertx(callee != nullptr);

  // Any function which can write locals is assumed to read them as well.
  if (funcWritesLocals(callee)) return true;

  // A skip-frame function can dynamically call a function which reads from the
  // caller's frame. If we don't forbid such dynamic calls, we have to be
  // pessimistic.
  if (callee->isSkipFrame() && !disallowDynamicVarEnvFuncs()) {
    return true;
  }

  if (!callee->readsCallerFrame()) return false;

  if (callee->fullName()->isame(s_assert.get())) {
    /*
     * Assert is somewhat special.  If RepoAuthoritative isn't set and the first
     * parameter is a string, it will be evaled and can have arbitrary effects.
     * If the assert fails, it may execute an arbitrary pre-registered callback
     * which still might try to read from the assert caller's frame.
     *
     * This can't happen if calling such frame accessing functions dynamically
     * is forbidden.
     */
    return !RuntimeOption::RepoAuthoritative || !disallowDynamicVarEnvFuncs();
  }
  return true;
}

bool funcNeedsCallerFrame(const Func* callee) {
  assertx(callee != nullptr);

  return
    (callee->isCPPBuiltin() &&
      s_ignores_frame.count(callee->name()->data()) == 0) ||
    funcReadsLocals(callee) ||
    funcWritesLocals(callee);
}

///////////////////////////////////////////////////////////////////////////////
}
