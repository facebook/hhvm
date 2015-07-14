/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/debug.h"
#include "hphp/util/trace.h"

#include <atomic>
#include <ostream>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

using jit::mcg;

const StringData*     Func::s___call       = makeStaticString("__call");
const StringData*     Func::s___callStatic = makeStaticString("__callStatic");
std::atomic<bool>     Func::s_treadmill;

/*
 * This size hint will create a ~6MB vector and is rarely hit in practice.
 * Note that this is just a hint and exceeding it won't affect correctness.
 */
constexpr size_t kFuncVecSizeHint = 750000;

/*
 * FuncId high water mark and FuncId -> Func* table.
 */
static std::atomic<FuncId> s_nextFuncId(0);
static AtomicVector<const Func*> s_funcVec(kFuncVecSizeHint, nullptr);

const AtomicVector<const Func*>& Func::getFuncVec() {
  return s_funcVec;
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
}

Func::~Func() {
  if (m_fullName != nullptr && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameStr(), &m_maybeIntercepted);
  }
  if (mcg != nullptr && !RuntimeOption::EvalEnableReusableTC) {
    // If Reusable TC is enabled then the prologue may have already been smashed
    // and the memory may now be in use by another function.
    smashPrologues();
  }
#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void* Func::allocFuncMem(int numParams) {
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numExtraPrologues = std::max(maxNumPrologues - kNumFixedPrologues, 0);

  size_t funcSize = sizeof(Func) + numExtraPrologues * sizeof(unsigned char*);

  return low_malloc(funcSize);
}

void Func::destroy(Func* func) {
  if (func->m_funcId != InvalidFuncId) {
    if (mcg && RuntimeOption::EvalEnableReusableTC) {
      // Free TC-space associated with func
      jit::reclaimFunction(func);
    }

    DEBUG_ONLY auto oldVal = s_funcVec.exchange(func->m_funcId, nullptr);
    assert(oldVal == func);
    func->m_funcId = InvalidFuncId;

    if (s_treadmill.load(std::memory_order_acquire)) {
      Treadmill::enqueue([func](){ destroy(func); });
      return;
    }
  }
  func->~Func();
  low_free(func);
}

void Func::smashPrologues() {
  int maxNumPrologues = getMaxNumPrologues(numParams());
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;
  mcg->smashPrologueGuards((jit::TCA*)m_prologueTable,
                           numPrologues, this);
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

  if (f != this) {
    f->m_cachedFunc = rds::Link<Func*>{rds::kInvalidHandle};
    f->m_maybeIntercepted = -1;
    f->m_isPreFunc = false;
  }

  return f;
}

void Func::rescope(Class* ctx, Attr attrs) {
  m_cls = ctx;
  if (attrs != AttrNone) m_attrs = attrs;

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
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;

  if (mcg == nullptr) {
    m_funcBody = nullptr;
    for (int i = 0; i < numPrologues; i++) {
      m_prologueTable[i] = nullptr;
    }
    return;
  }

  auto const& stubs = mcg->tx().uniqueStubs;

  m_funcBody = stubs.funcBodyHelperThunk;

  TRACE(2, "initPrologues func %p %d\n", this, numPrologues);
  for (int i = 0; i < numPrologues; i++) {
    m_prologueTable[i] = stubs.fcallHelperThunk;
  }
}

void Func::setFullName(int numParams) {
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
      m_namedEntity = NamedEntity::get(m_name);
    }
  }
  if (RuntimeOption::EvalPerfDataMap) {
    int maxNumPrologues = Func::getMaxNumPrologues(numParams);
    int numPrologues = maxNumPrologues > kNumFixedPrologues
      ? maxNumPrologues
      : kNumFixedPrologues;

    char* from = (char*)this;
    char* to = (char*)(m_prologueTable + numPrologues);

    Debug::DebugInfo::recordDataMap(
      from,
      to,
      folly::format(
        "Func-{}",
        isPseudoMain() ? m_unit->filepath() : m_fullName.get()
      ).str()
    );
  }
  if (RuntimeOption::DynamicInvokeFunctions.size()) {
    if (RuntimeOption::DynamicInvokeFunctions.find(m_fullName->data()) !=
        RuntimeOption::DynamicInvokeFunctions.end()) {
      m_attrs = Attr(m_attrs | AttrInterceptable);
    }
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
  assert(id < s_nextFuncId);
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
const StaticString s_extractNative("__SystemLib\\extract");
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
      if (name() == s_extract.get() && !cls()) return false;
      if (name() == s_extractNative.get() && !cls()) return false;
      if (name() == s_current.get() && !cls()) return false;
      if (name() == s_key.get() && !cls()) return false;
      if (name() == s_array_multisort.get() && !cls()) return false;
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

int Func::numSlotsInFrame() const {
  return shared()->m_numLocals + shared()->m_numIterators * kNumIterCells;
}


///////////////////////////////////////////////////////////////////////////////
// Persistence.

bool Func::isNameBindingImmutable(const Unit* fromUnit) const {
  if (RuntimeOption::EvalJitEnableRenameFunction ||
      m_attrs & AttrInterceptable) {
    return false;
  }

  if (isBuiltin()) {
    return true;
  }

  if (isUnique()) {
    return true;
  }

  // Defined at top level, in the same unit as the caller. This precludes
  // conditionally defined functions and cross-module calls -- both phenomena
  // can change name->Func mappings during the lifetime of a TC.
  return top() && (fromUnit == m_unit);
}


///////////////////////////////////////////////////////////////////////////////
// Unit table entries.

const EHEnt* Func::findEH(Offset o) const {
  assert(o >= base() && o < past());
  return HPHP::findEH(shared()->m_ehtab, o);
}

const FPIEnt* Func::findFPI(Offset o) const {
  assert(o >= base() && o < past());
  const FPIEnt* fe = nullptr;
  unsigned int i;

  const FPIEntVec& fpitab = shared()->m_fpitab;
  for (i = 0; i < fpitab.size(); i++) {
    /*
     * We consider the "FCall" instruction part of the FPI region, but
     * the corresponding push is not considered part of it.  (This
     * means all offsets in the FPI region will have the partial
     * ActRec on the stack.)
     */
    if (fpitab[i].m_fpushOff < o && o <= fpitab[i].m_fcallOff) {
      fe = &fpitab[i];
    }
  }
  return fe;
}

const FPIEnt* Func::findPrecedingFPI(Offset o) const {
  assert(o >= base() && o < past());
  const FPIEntVec& fpitab = shared()->m_fpitab;
  assert(fpitab.size());
  const FPIEnt* fe = 0;
  for (unsigned i = 0; i < fpitab.size(); i++) {
    const FPIEnt* cur = &fpitab[i];
    if (o > cur->m_fcallOff &&
        (!fe || fe->m_fcallOff < cur->m_fcallOff)) {
      fe = cur;
    }
  }
  assert(fe);
  return fe;
}


///////////////////////////////////////////////////////////////////////////////
// JIT data.

int Func::numPrologues() const {
  int maxNumPrologues = Func::getMaxNumPrologues(numParams());
  int nPrologues = maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                                        : kNumFixedPrologues;
  return nPrologues;
}

void Func::resetPrologue(int numParams) {
  auto const& stubs = mcg->tx().uniqueStubs;
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
}

void Func::prettyPrint(std::ostream& out, const PrintOpts& opts) const {
  if (isPseudoMain()) {
    out << "Pseudo-main";
  } else if (preClass() != nullptr) {
    out << "Method";
    print_attrs(out, m_attrs);
    if (cls() != nullptr) {
      out << ' ' << fullName()->data();
    } else {
      out << ' ' << preClass()->name()->data() << "::" << m_name->data();
    }
  } else {
    out << "Function";
    print_attrs(out, m_attrs);
    out << ' ' << m_name->data();
  }

  out << " at " << base();
  out << std::endl;

  if (!opts.metadata) return;

  const ParamInfoVec& params = shared()->m_params;
  for (uint32_t i = 0; i < params.size(); ++i) {
    if (params[i].funcletOff != InvalidAbsoluteOffset) {
      out << " DV for parameter " << i << " at " << params[i].funcletOff;
      if (params[i].phpCode) {
        out << " = " << params[i].phpCode->data();
      }
      out << std::endl;
    }
  }
  out << "maxStackCells: " << maxStackCells() << '\n'
      << "numLocals: " << numLocals() << '\n'
      << "numIterators: " << numIterators() << '\n';

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
    if (catcher) {
      out << std::endl;
      for (auto it2 = it->m_catches.begin();
          it2 != it->m_catches.end();
          ++it2) {
        out << "  Handle " << m_unit->lookupLitstrId(it2->first)->data()
          << " at " << it2->second;
      }
    } else {
      out << " to " << it->m_fault;
    }
    if (it->m_parentIndex != -1) {
      out << " parentIndex " << it->m_parentIndex;
    }
    out << std::endl;
  }

  if (opts.fpi) {
    for (auto& fpi : fpitab()) {
      out << " FPI " << fpi.m_fpushOff << "-" << fpi.m_fcallOff
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
  , m_originalFilename(nullptr)
{
  m_pastDelta = std::min<uint32_t>(past - base, kSmallDeltaLimit);
  m_line2Delta = std::min<uint32_t>(line2 - line1, kSmallDeltaLimit);
}

Func::SharedData::~SharedData() {
  free(m_refBitPtr);
}

void Func::SharedData::atomicRelease() {
  delete this;
}

///////////////////////////////////////////////////////////////////////////////
}
