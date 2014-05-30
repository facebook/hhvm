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
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/verifier/cfg.h"

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

using JIT::tx;
using JIT::mcg;

const StringData* Func::s___call       = makeStaticString("__call");
const StringData* Func::s___callStatic = makeStaticString("__callStatic");

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


///////////////////////////////////////////////////////////////////////////////
// Creation and destruction.

Func::Func(Unit& unit, PreClass* preClass, int line1, int line2,
           Offset base, Offset past, const StringData* name, Attr attrs,
           bool top, const StringData* docComment, int numParams)
  : m_name(name)
  , m_unit(&unit)
  , m_attrs(attrs)
{
  m_hasPrivateAncestor = false;
  m_shared = new SharedData(preClass, base, past,
                            line1, line2, top, docComment);
  init(numParams);
}

Func::~Func() {
  if (m_fullName != nullptr && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameStr(), &m_maybeIntercepted);
  }
  if (m_funcId != InvalidFuncId) {
    DEBUG_ONLY auto oldVal = s_funcVec.exchange(m_funcId, nullptr);
    assert(oldVal == this);
  }
  int maxNumPrologues = getMaxNumPrologues(numParams());
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;
  if (mcg != nullptr) {
    mcg->smashPrologueGuards((JIT::TCA*)m_prologueTable,
                             numPrologues, this);
  }
#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void* Func::allocFuncMem(
  const StringData* name, int numParams,
  bool needsNextClonedClosure,
  bool lowMem) {
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numExtraPrologues =
    maxNumPrologues > kNumFixedPrologues ?
    maxNumPrologues - kNumFixedPrologues :
    0;
  int numExtraFuncPtrs = (int) needsNextClonedClosure;
  size_t funcSize =
    sizeof(Func) +
    numExtraPrologues * sizeof(unsigned char*) +
    numExtraFuncPtrs * sizeof(Func*);

  void* mem = lowMem ? low_malloc(funcSize) : malloc(funcSize);

  /**
   * The Func object can have optional nextClonedClosure pointer to Func
   * in front of the actual object. The layout is as follows:
   *
   *               +--------------------------------+ low address
   *               |  nextClonedClosure (optional)  |
   *               |  in closures                   |
   *               +--------------------------------+ Func* address
   *               |  Func object                   |
   *               +--------------------------------+ high address
   */
  memset(mem, 0, numExtraFuncPtrs * sizeof(Func*));
  return ((Func**) mem) + numExtraFuncPtrs;
}

void Func::destroy(Func* func) {
  /*
   * Funcs in PreClasses are just templates, and don't get used
   * until they are cloned so we don't put them in low memory.
   */
  bool lowMem = !func->preClass() || func->m_cls;
  void* mem = func;
  if (func->isClosureBody()) {
    Func** startOfFunc = ((Func**) mem) - 1; // move back by a pointer
    mem = startOfFunc;
    if (Func* f = *startOfFunc) {
      /*
       * cloned closures use the prolog array to hold
       * the per-clone post-prolog entry points.
       * They're not real prologs, and they shouldn't be
       * smashed, so clear them out here.
       */
      f->initPrologues(f->numParams());
      Func::destroy(f);
    }
  }
  func->~Func();
  if (lowMem) {
    low_free(mem);
  } else {
    free(mem);
  }
}

Func* Func::clone(Class* cls, const StringData* name) const {
  auto numParams = this->numParams();
  Func* f = new (allocFuncMem(
                   m_name,
                   numParams,
                   isClosureBody(),
                   cls || !preClass())) Func(*this);

  f->initPrologues(numParams);
  f->m_funcId = InvalidFuncId;
  if (name) {
    f->m_name = name;
  }
  if (cls != f->m_cls) {
    f->m_cls = cls;
  }
  f->setFullName(numParams);
  f->m_profCounter = 0;
  return f;
}

Func* Func::cloneAndSetClass(Class* cls) const {
  if (Func* ret = findCachedClone(cls)) {
    return ret;
  }

  static Mutex s_clonedFuncListMutex;
  Lock l(s_clonedFuncListMutex);
  // Check again now that I'm the writer
  if (Func* ret = findCachedClone(cls)) {
    return ret;
  }

  Func* clonedFunc = clone(cls);
  clonedFunc->setNewFuncId();

  // Save it so we don't have to keep cloning it and retranslating
  Func** nextFunc = &this->nextClonedClosure();
  while (*nextFunc) {
    nextFunc = &nextFunc[0]->nextClonedClosure();
  }
  *nextFunc = clonedFunc;

  return clonedFunc;
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
     * i)  We dont want these compiler generated functions to
     *     appear in backtraces.
     *
     * ii) 86sinit and 86pinit construct NameValueTableWrappers
     *     on the stack. So we MUST NOT allow those to leak into
     *     the backtrace (since the backtrace will outlive the
     *     variables).
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

  if (tx == nullptr) {
    m_funcBody = nullptr;
    for (int i = 0; i < numPrologues; i++) {
      m_prologueTable[i] = nullptr;
    }
    return;
  }

  auto const& stubs = tx->uniqueStubs;

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
    m_namedEntity = Unit::GetNamedEntity(m_name);
  }
  if (RuntimeOption::EvalPerfDataMap) {
    int numPre = isClosureBody() ? 1 : 0;
    char* from = (char*)this - numPre * sizeof(Func);

    int maxNumPrologues = Func::getMaxNumPrologues(numParams);
    int numPrologues = maxNumPrologues > kNumFixedPrologues ?
      maxNumPrologues : kNumFixedPrologues;
    char* to = (char*)(m_prologueTable + numPrologues);
    Debug::DebugInfo::recordDataMap(
      from, to, folly::format("Func-{}-{}", numPre,
                              (isPseudoMain() ?
                               m_unit->filepath()->data() :
                               m_fullName->data())).str());
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

bool Func::mustBeRef(int32_t arg) const {
  if (!byRef(arg)) return false;
  if (arg == 0) {
    if (UNLIKELY(m_attrs & AttrBuiltin)) {
      // This hacks mustBeRef() to return false for the first parameter of
      // extract(), current(), and key(). These functions try to take their
      // first parameter by reference but they also allow expressions that
      // cannot be taken by reference (ex. an array literal).
      // TODO Task #4442937: Come up with a cleaner way to do this.
      if (name() == s_extract.get() && !cls()) return false;
      if (name() == s_extractNative.get() && !cls()) return false;
      if (name() == s_current.get() && !cls()) return false;
      if (name() == s_key.get() && !cls()) return false;
    }
  }
  return
    arg < numParams() ||
    !(m_attrs & AttrVariadicByRef) ||
    !methInfo() ||
    !(methInfo()->attribute & ClassInfo::MixedVariableArguments);
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
// Closures.

bool Func::isClonedClosure() const {
  if (!isClosureBody()) return false;
  if (!cls()) return true;
  return cls()->lookupMethod(name()) != this;
}

Func* Func::findCachedClone(Class* cls) const {
  Func* nextFunc = const_cast<Func*>(this);
  while (nextFunc) {
    if (nextFunc->cls() == cls) {
      return nextFunc;
    }
    nextFunc = nextFunc->nextClonedClosure();
  }
  return nullptr;
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

  if (isUnique() && RuntimeOption::RepoAuthoritative) {
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
  const FPIEnt* fe = &fpitab[0];
  unsigned int i;
  for (i = 1; i < fpitab.size(); i++) {
    const FPIEnt* cur = &fpitab[i];
    if (o > cur->m_fcallOff &&
        fe->m_fcallOff < cur->m_fcallOff) {
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
  auto const& stubs = tx->uniqueStubs;
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
  if (attrs & AttrInterceptable) { out << " (interceptable)"; }
  if (attrs & AttrPersistent) { out << " (persistent)"; }
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
  for (uint i = 0; i < params.size(); ++i) {
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
// Other methods.

void Func::getFuncInfo(ClassInfo::MethodInfo* mi) const {
  assert(mi);
  if (methInfo() != nullptr) {
    // Very large operator=() invocation.
    *mi = *methInfo();
    // Deep copy the vectors of mi-owned pointers.
    for (auto& p : mi->parameters)      p = new ClassInfo::ParameterInfo(*p);
    for (auto& p : mi->staticVariables) p = new ClassInfo::ConstantInfo(*p);
  } else {
    // hphpc sets the ClassInfo::VariableArguments attribute if the method
    // contains a call to func_get_arg, func_get_args, or func_num_args. We
    // don't do this in the VM currently and hopefully we never will need to.
    int attr = 0;
    if (m_attrs & AttrReference) attr |= ClassInfo::IsReference;
    if (m_attrs & AttrAbstract) attr |= ClassInfo::IsAbstract;
    if (m_attrs & AttrFinal) attr |= ClassInfo::IsFinal;
    if (m_attrs & AttrProtected) attr |= ClassInfo::IsProtected;
    if (m_attrs & AttrPrivate) attr |= ClassInfo::IsPrivate;
    if (m_attrs & AttrStatic) attr |= ClassInfo::IsStatic;
    if (m_attrs & AttrHPHPSpecific) attr |= ClassInfo::HipHopSpecific;
    if (!(attr & ClassInfo::IsProtected || attr & ClassInfo::IsPrivate)) {
      attr |= ClassInfo::IsPublic;
    }
    if (attr == 0) attr = ClassInfo::IsNothing;
    mi->attribute = (ClassInfo::Attribute)attr;
    mi->name = m_name->data();
    mi->file = m_unit->filepath()->data();
    mi->line1 = line1();
    mi->line2 = line2();
    if (docComment() && !docComment()->empty()) {
      mi->docComment = docComment()->data();
    }
    // Get the parameter info
    auto limit = numParams();
    for (unsigned i = 0; i < limit; ++i) {
      ClassInfo::ParameterInfo* pi = new ClassInfo::ParameterInfo;
      attr = 0;
      if (byRef(i)) {
        attr |= ClassInfo::IsReference;
      }
      if (attr == 0) {
        attr = ClassInfo::IsNothing;
      }
      const ParamInfoVec& params = shared()->m_params;
      const ParamInfo& fpi = params[i];
      pi->attribute = (ClassInfo::Attribute)attr;
      pi->name = shared()->m_localNames[i]->data();
      if (params.size() <= i || !fpi.hasDefaultValue()) {
        pi->value = nullptr;
        pi->valueText = "";
      } else {
        if (fpi.hasScalarDefaultValue()) {
          // Most of the time the default value is scalar, so we can
          // avoid evaling in the common case
          pi->value = strdup(f_serialize(
            tvAsVariant((TypedValue*)&fpi.defaultValue)).get()->data());
        } else {
          // Eval PHP code to get default value, and serialize the result. Note
          // that access of undefined class constants can cause the eval() to
          // fatal. Zend lets such fatals propagate, so don't bother catching
          // exceptions here.
          const Variant& v = g_context->getEvaledArg(
            fpi.phpCode,
            cls() ? cls()->nameStr() : nameStr()
          );
          pi->value = strdup(f_serialize(v).get()->data());
        }
        // This is a raw char*, but its lifetime should be at least as long
        // as the the Func*. At this writing, it's a merged anon string
        // owned by ParamInfo.
        pi->valueText = fpi.phpCode->data();
      }
      pi->type = fpi.typeConstraint.hasConstraint() ?
        fpi.typeConstraint.typeName()->data() : "";
      for (auto it = fpi.userAttributes.begin();
          it != fpi.userAttributes.end(); ++it) {
        // convert the typedvalue to a cvarref and push into pi.
        auto userAttr = new ClassInfo::UserAttributeInfo;
        assert(it->first->isStatic());
        userAttr->name = const_cast<StringData*>(it->first.get());
        userAttr->setStaticValue(tvAsCVarRef(&it->second));
        pi->userAttrs.push_back(userAttr);
      }
      mi->parameters.push_back(pi);
    }
    // XXX ConstantInfo is abused to store static variable metadata, and
    // although ConstantInfo::callbacks provides a mechanism for registering
    // callbacks, it does not pass enough information through for the callback
    // functions to know the function context whence the callbacks came.
    // Furthermore, the callback mechanism isn't employed in a fashion that
    // would allow repeated introspection to reflect updated values.
    // Supporting introspection of static variable values will require
    // different plumbing than currently exists in ConstantInfo.
    const SVInfoVec& staticVars = shared()->m_staticVars;
    for (SVInfoVec::const_iterator it = staticVars.begin();
         it != staticVars.end(); ++it) {
      ClassInfo::ConstantInfo* ci = new ClassInfo::ConstantInfo;
      ci->name = StrNR(it->name);
      if ((*it).phpCode != nullptr) {
        ci->valueLen = (*it).phpCode->size();
        ci->valueText = (*it).phpCode->data();
      } else {
        ci->valueLen = 0;
        ci->valueText = "";
      }

      mi->staticVariables.push_back(ci);
    }
  }
}

bool Func::shouldPGO() const {
  if (!RuntimeOption::EvalJitPGO) return false;

  // Non-cloned closures simply contain prologues that redispacth to
  // cloned closures.  They don't contain a translation for the
  // function entry, which is what triggers an Optimize retranslation.
  // So don't generate profiling translations for them -- there's not
  // much to do with PGO anyway here, since they just have prologues.
  if (isClosureBody() && !isClonedClosure()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return attrs() & AttrHot;
}

void Func::incProfCounter() {
  if (m_attrs & AttrHot) return;
  __sync_fetch_and_add(&m_profCounter, 1);
  if (m_profCounter >= RuntimeOption::EvalHotFuncThreshold) {
    m_attrs = (Attr)(m_attrs | AttrHot);
  }
}

bool Func::anyBlockEndsAt(Offset off) const {
  assert(JIT::Translator::WriteLease().amOwner());
  // The empty() check relies on a Func's bytecode always being nonempty
  assert(base() != past());
  if (m_shared->m_blockEnds.empty()) {
    using namespace Verifier;

    Arena arena;
    GraphBuilder builder{arena, this};
    Graph* cfg = builder.build();

    for (LinearBlocks blocks = linearBlocks(cfg); !blocks.empty(); ) {
      auto last = blocks.popFront()->last - m_unit->entry();
      m_shared->m_blockEnds.insert(last);
    }

    assert(!m_shared->m_blockEnds.empty());
  }

  return m_shared->m_blockEnds.count(off) != 0;
}


///////////////////////////////////////////////////////////////////////////////
// SharedData.

Func::SharedData::SharedData(PreClass* preClass, Offset base, Offset past,
                             int line1, int line2, bool top,
                             const StringData* docComment)
  : m_preClass(preClass)
  , m_base(base)
  , m_past(past)
  , m_numLocals(0)
  , m_numIterators(0)
  , m_line1(line1)
  , m_line2(line2)
  , m_info(nullptr)
  , m_refBitPtr(0)
  , m_builtinFuncPtr(nullptr)
  , m_docComment(docComment)
  , m_top(top)
  , m_isClosureBody(false)
  , m_isAsync(false)
  , m_isGenerator(false)
  , m_isPairGenerator(false)
  , m_isGenerated(false)
  , m_originalFilename(nullptr)
{}

Func::SharedData::~SharedData() {
  free(m_refBitPtr);
}

void Func::SharedData::atomicRelease() {
  delete this;
}

///////////////////////////////////////////////////////////////////////////////
}
