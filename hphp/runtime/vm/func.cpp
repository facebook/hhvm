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
#include "hphp/runtime/vm/func.h"

#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "hphp/runtime/base/base_includes.h"
#include "hphp/util/atomic_vector.h"
#include "hphp/util/util.h"
#include "hphp/util/trace.h"
#include "hphp/util/debug.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/base/file_repository.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/func-inline.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/util/parser/parser.h"

namespace HPHP {

TRACE_SET_MOD(hhbc);
const StringData* Func::s___call = StringData::GetStaticString("__call");
const StringData* Func::s___callStatic =
  StringData::GetStaticString("__callStatic");

//=============================================================================
// Func.

static void decl_incompat(const PreClass* implementor,
                          const Func* imeth) {
  const char* name = imeth->name()->data();
  raise_error("Declaration of %s::%s() must be compatible with "
              "that of %s::%s()", implementor->name()->data(), name,
              imeth->cls()->preClass()->name()->data(), name);
}

// Check compatibility vs interface and abstract declarations
void Func::parametersCompat(const PreClass* preClass, const Func* imeth) const {
  const Func::ParamInfoVec& params = this->params();
  const Func::ParamInfoVec& iparams = imeth->params();
  // Verify that meth has at least as many parameters as imeth.
  if ((params.size() < iparams.size())) {
    decl_incompat(preClass, imeth);
  }
  // Verify that the typehints for meth's parameters are compatible with
  // imeth's corresponding parameter typehints.
  unsigned firstOptional = 0;
  for (unsigned i = 0; i < iparams.size(); ++i) {
    if (!params[i].typeConstraint().compat(iparams[i].typeConstraint())) {
      decl_incompat(preClass, imeth);
    }
    if (!iparams[i].hasDefaultValue()) {
      // The leftmost of imeth's contiguous trailing optional parameters
      // must start somewhere to the right of this parameter.
      firstOptional = i + 1;
    }
  }
  // Verify that meth provides defaults, starting with the parameter that
  // corresponds to the leftmost of imeth's contiguous trailing optional
  // parameters.
  for (unsigned i = firstOptional; i < params.size(); ++i) {
    if (!params[i].hasDefaultValue()) {
      decl_incompat(preClass, imeth);
    }
  }
}

static std::atomic<FuncId> s_nextFuncId(0);

// This size hint will create a ~6MB vector and is rarely hit in
// practice. Note that this is just a hint and exceeding it won't
// affect correctness.
constexpr size_t kFuncVecSizeHint = 750000;
static AtomicVector<const Func*> s_funcVec(kFuncVecSizeHint, nullptr);

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

void Func::setFullName() {
  assert(m_name->isStatic());
  if (m_cls) {
    m_fullName = StringData::GetStaticString(
      std::string(m_cls->name()->data()) + "::" + m_name->data());
  } else {
    m_fullName = m_name;
    m_namedEntity = Unit::GetNamedEntity(m_name);
  }
  if (RuntimeOption::DynamicInvokeFunctions.size()) {
    if (RuntimeOption::DynamicInvokeFunctions.find(m_fullName->data()) !=
        RuntimeOption::DynamicInvokeFunctions.end()) {
      m_attrs = Attr(m_attrs | AttrDynamicInvoke);
    }
  }
}

void Func::initPrologues(int numParams, bool isGenerator) {
  m_funcBody = (TCA)HPHP::Transl::funcBodyHelperThunk;

  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;

  TRACE(2, "initPrologues func %p %d\n", this, numPrologues);
  for (int i = 0; i < numPrologues; i++) {
    m_prologueTable[i] = (TCA)HPHP::Transl::fcallHelperThunk;
  }
}

void Func::init(int numParams, bool isGenerator) {
  // For methods, we defer setting the full name until m_cls is initialized
  m_maybeIntercepted = -1;
  if (!preClass()) {
    setNewFuncId();
    setFullName();
  } else {
    m_fullName = 0;
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
  initPrologues(numParams, isGenerator);
}

void* Func::allocFuncMem(
    const StringData* name, int numParams, bool needsNextClonedClosure) {
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numExtraPrologues =
    maxNumPrologues > kNumFixedPrologues ?
    maxNumPrologues - kNumFixedPrologues :
    0;
  size_t funcSize = sizeof(Func) + numExtraPrologues * sizeof(unsigned char*);
  if (needsNextClonedClosure) {
    funcSize += sizeof(Func*);
  }
  void* mem = Util::low_malloc(funcSize);
  if (needsNextClonedClosure) {
    // make room for nextClonedClosure to work
    Func** startOfFunc = (Func**) mem;
    *startOfFunc = nullptr;
    return startOfFunc + 1;
  }
  return mem;
}

Func::Func(Unit& unit, Id id, int line1, int line2,
           Offset base, Offset past, const StringData* name,
           Attr attrs, bool top, const StringData* docComment, int numParams,
           bool isGenerator)
  : m_unit(&unit)
  , m_cls(nullptr)
  , m_baseCls(nullptr)
  , m_name(name)
  , m_namedEntity(nullptr)
  , m_refBitVal(0)
  , m_cachedOffset(0)
  , m_maxStackCells(0)
  , m_numParams(0)
  , m_attrs(attrs)
  , m_funcId(InvalidFuncId)
  , m_hasPrivateAncestor(false)
{
  m_shared = new SharedData(nullptr, id, base, past, line1, line2,
                            top, docComment);
  init(numParams, isGenerator);
}

// Class method
Func::Func(Unit& unit, PreClass* preClass, int line1, int line2, Offset base,
           Offset past, const StringData* name, Attr attrs,
           bool top, const StringData* docComment, int numParams,
           bool isGenerator)
  : m_unit(&unit)
  , m_cls(nullptr)
  , m_baseCls(nullptr)
  , m_name(name)
  , m_namedEntity(nullptr)
  , m_refBitVal(0)
  , m_cachedOffset(0)
  , m_maxStackCells(0)
  , m_numParams(0)
  , m_attrs(attrs)
  , m_funcId(InvalidFuncId)
  , m_hasPrivateAncestor(false)
{
  Id id = -1;
  m_shared = new SharedData(preClass, id, base, past, line1, line2,
                            top, docComment);
  init(numParams, isGenerator);
}

Func::~Func() {
  if (m_fullName != nullptr && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameRef(), &m_maybeIntercepted);
  }
  if (m_funcId != InvalidFuncId) {
    DEBUG_ONLY auto oldVal = s_funcVec.exchange(m_funcId, nullptr);
    assert(oldVal == this);
  }
  int maxNumPrologues = getMaxNumPrologues(numParams());
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;
  TranslatorX64::Get()->smashPrologueGuards((TCA *)m_prologueTable,
                                            numPrologues, this);
#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void Func::destroy(Func* func) {
  void* mem = func;
  if (func->isClosureBody() || func->isGeneratorFromClosure()) {
    Func** startOfFunc = (Func**) mem;
    mem = startOfFunc - 1; // move back by a pointer
    if (Func* f = startOfFunc[-1]) {
      Func::destroy(f);
    }
  }
  func->~Func();
  Util::low_free(mem);
}

Func* Func::clone() const {
  Func* f = new (allocFuncMem(
        m_name,
        m_numParams,
        isClosureBody() || isGeneratorFromClosure()
  )) Func(*this);
  f->initPrologues(m_numParams, isGenerator());
  f->m_funcId = InvalidFuncId;
  return f;
}

const Func* Func::cloneAndSetClass(Class* cls) const {
  if (const Func* ret = findCachedClone(cls)) {
    return ret;
  }

  static Mutex s_clonedFuncListMutex;
  Lock l(s_clonedFuncListMutex);
  // Check again now that I'm the writer
  if (const Func* ret = findCachedClone(cls)) {
    return ret;
  }

  Func* clonedFunc = clone();
  clonedFunc->setNewFuncId();
  clonedFunc->setCls(cls);

  // Save it so we don't have to keep cloning it and retranslating
  Func** nextFunc = &this->nextClonedClosure();
  while (*nextFunc) {
    nextFunc = &nextFunc[0]->nextClonedClosure();
  }
  *nextFunc = clonedFunc;

  return clonedFunc;
}

const Func* Func::findCachedClone(Class* cls) const {
  const Func* nextFunc = this;
  while (nextFunc) {
    if (nextFunc->cls() == cls) {
      return nextFunc;
    }
    nextFunc = nextFunc->nextClonedClosure();
  }
  return nullptr;
}

void Func::rename(const StringData* name) {
  m_name = name;
  setFullName();
  // load the renamed function
  Unit::loadFunc(this);
}

int Func::numSlotsInFrame() const {
  return shared()->m_numLocals + shared()->m_numIterators * kNumIterCells;
}

bool Func::checkIterScope(Offset o, Id iterId, bool& itRef) const {
  const EHEntVec& ehtab = shared()->m_ehtab;
  assert(o >= base() && o < past());
  for (unsigned i = 0, n = ehtab.size(); i < n; i++) {
    const EHEnt* eh = &ehtab[i];
    if (eh->m_type == EHEnt::Type::Fault &&
        eh->m_base <= o && o < eh->m_past &&
        eh->m_iterId == iterId) {
      itRef = eh->m_itRef;
      return true;
    }
  }
  return false;
}

const EHEnt* Func::findEH(Offset o) const {
  assert(o >= base() && o < past());
  const EHEnt* eh = nullptr;
  unsigned int i;

  const EHEntVec& ehtab = shared()->m_ehtab;
  for (i = 0; i < ehtab.size(); i++) {
    if (ehtab[i].m_base <= o && o < ehtab[i].m_past) {
      eh = &ehtab[i];
    }
  }
  return eh;
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

bool Func::isClonedClosure() const {
  if (!isClosureBody()) return false;
  if (!cls()) return true;
  return cls()->lookupMethod(name()) != this;
}

bool Func::isNameBindingImmutable(const Unit* fromUnit) const {
  if (RuntimeOption::EvalJitEnableRenameFunction ||
      m_attrs & AttrDynamicInvoke) {
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

bool Func::byRef(int32_t arg) const {
  const uint64_t* ref = &m_refBitVal;
  assert(arg >= 0);
  if (UNLIKELY(arg >= kBitsPerQword)) {
    // Super special case. A handful of builtins are varargs functions where the
    // (not formally declared) varargs are pass-by-reference. psychedelic-kitten
    if (arg >= m_numParams) {
      return m_attrs & AttrVariadicByRef;
    }
    ref = &shared()->m_refBitPtr[(uint32_t)arg / kBitsPerQword - 1];
  }
  int bit = (uint32_t)arg % kBitsPerQword;
  return *ref & (1ull << bit);
}

bool Func::mustBeRef(int32_t arg) const {
  if (byRef(arg)) {
    return arg < m_numParams || !(m_attrs & AttrVariadicByRef) ||
      !info() || !(info()->attribute & ClassInfo::MixedVariableArguments);
  }
  return false;
}

void Func::appendParam(bool ref, const Func::ParamInfo& info,
                       std::vector<ParamInfo>& pBuilder) {
  int qword = m_numParams / kBitsPerQword;
  int bit   = m_numParams % kBitsPerQword;
  m_numParams++;
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

Id Func::lookupVarId(const StringData* name) const {
  assert(name != nullptr);
  return shared()->m_localNames.findIndex(name);
}

void Func::prettyPrint(std::ostream& out) const {
  if (isPseudoMain()) {
    out << "Pseudo-main";
  } else if (preClass() != nullptr) {
    out << "Method ";
    if (m_attrs & AttrStatic) { out << "static "; }
    if (m_attrs & AttrPublic) { out << "public "; }
    if (m_attrs & AttrProtected) { out << "protected "; }
    if (m_attrs & AttrPrivate) { out << "private "; }
    if (m_attrs & AttrAbstract) { out << "abstract "; }
    if (m_attrs & AttrFinal) { out << "final "; }
    if (m_attrs & AttrPhpLeafFn) { out << "(leaf) "; }
    if (cls() != nullptr) {
      out << fullName()->data();
    } else {
      out << preClass()->name()->data() << "::" << m_name->data();
    }
  } else {
    out << "Function " << m_name->data();
  }

  if (m_attrs & AttrHot) out << " (hot)";

  out << " at " << base();
  if (shared()->m_id != -1) {
    out << " (ID " << shared()->m_id << ")";
  }
  out << std::endl;
  const ParamInfoVec& params = shared()->m_params;
  for (uint i = 0; i < params.size(); ++i) {
    if (params[i].funcletOff() != InvalidAbsoluteOffset) {
      out << " DV for parameter " << i << " at " << params[i].funcletOff();
      if (params[i].phpCode()) {
        out << " = " << params[i].phpCode()->data();
      }
      out << std::endl;
    }
  }
  const EHEntVec& ehtab = shared()->m_ehtab;
  for (EHEntVec::const_iterator it = ehtab.begin(); it != ehtab.end(); ++it) {
    bool catcher = it->m_type == EHEnt::Type::Catch;
    out << " EH " << (catcher ? "Catch" : "Fault") << " for " <<
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
      for (EHEnt::CatchVec::const_iterator it2 = it->m_catches.begin();
           it2 != it->m_catches.end(); ++it2) {
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
}

HphpArray* Func::getStaticLocals() const {
  return g_vmContext->getFuncStaticCtx(this);
}

void Func::getFuncInfo(ClassInfo::MethodInfo* mi) const {
  assert(mi);
  if (info() != nullptr) {
    // Very large operator=() invocation.
    *mi = *info();
    // Deep copy the vectors of mi-owned pointers.
    cloneMembers(mi->parameters);
    cloneMembers(mi->staticVariables);
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
    if (!(attr & ClassInfo::IsProtected || attr & ClassInfo::IsPrivate)) {
      attr |= ClassInfo::IsPublic;
    }
    if (preClass() &&
        (!strcasecmp(m_name->data(), "__construct") ||
         (!(preClass()->attrs() & AttrTrait) &&
          !strcasecmp(m_name->data(), preClass()->name()->data()) &&
          !preClass()->hasMethod(String("__construct").get())))) {
      attr |= ClassInfo::IsConstructor;
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
    for (unsigned i = 0; i < unsigned(m_numParams); ++i) {
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
            tvAsVariant((TypedValue*)&fpi.defaultValue())).get()->data());
        } else {
          // Eval PHP code to get default value, and serialize the result. Note
          // that access of undefined class constants can cause the eval() to
          // fatal. Zend lets such fatals propagate, so don't bother catching
          // exceptions here.
          CVarRef v = g_vmContext->getEvaledArg(
            fpi.phpCode(),
            cls() ? cls()->nameRef() : nameRef()
          );
          pi->value = strdup(f_serialize(v).get()->data());
        }
        // This is a raw char*, but its lifetime should be at least as long
        // as the the Func*. At this writing, it's a merged anon string
        // owned by ParamInfo.
        pi->valueText = fpi.phpCode()->data();
      }
      pi->type = fpi.typeConstraint().hasConstraint() ?
        fpi.typeConstraint().typeName()->data() : "";
      for (UserAttributeMap::const_iterator it = fpi.userAttributes().begin();
           it != fpi.userAttributes().end(); ++it) {
        // convert the typedvalue to a cvarref and push into pi.
        auto userAttr = new ClassInfo::UserAttributeInfo;
        assert(it->first->isStatic());
        userAttr->name = const_cast<StringData*>(it->first);
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
      ci->name = *(String*)(&(*it).name);
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

DVFuncletsVec Func::getDVFunclets() const {
 DVFuncletsVec dvs;
  int nParams = numParams();
  for (int i = 0; i < nParams; ++i) {
    const ParamInfo& pi = params()[i];
    if (pi.hasDefaultValue()) {
      dvs.push_back(std::make_pair(i, pi.funcletOff()));
    }
  }
  return dvs;
}


Func::SharedData::SharedData(PreClass* preClass, Id id,
                             Offset base, Offset past, int line1, int line2,
                             bool top, const StringData* docComment)
  : m_preClass(preClass), m_id(id), m_base(base),
    m_numLocals(0), m_numIterators(0),
    m_past(past), m_line1(line1), m_line2(line2),
    m_info(nullptr), m_refBitPtr(0), m_builtinFuncPtr(nullptr),
    m_docComment(docComment), m_top(top), m_isClosureBody(false),
    m_isGenerator(false), m_isGeneratorFromClosure(false),
    m_isPairGenerator(false), m_hasGeneratorAsBody(false),
    m_isGenerated(false), m_originalFilename(nullptr) {
}

Func::SharedData::~SharedData() {
  free(m_refBitPtr);
}

void Func::SharedData::atomicRelease() {
  delete this;
}

Func** Func::getCachedAddr() {
  assert(!isMethod());
  return getCachedFuncAddr(m_cachedOffset);
}

void Func::setCached() {
  setCachedFunc(this, isDebuggerAttached());
}

const Func* Func::getGeneratorBody(const StringData* name) const {
  if (isNonClosureMethod()) {
    return cls()->lookupMethod(name);
  } else {
    return Unit::lookupFunc(name);
  }
}

//=============================================================================
// FuncEmitter.

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n)
  : m_ue(ue)
  , m_pce(nullptr)
  , m_sn(sn)
  , m_id(id)
  , m_name(n)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_retTypeConstraint(nullptr)
  , m_returnType(KindOfInvalid)
  , m_top(false)
  , m_isClosureBody(false)
  , m_isGenerator(false)
  , m_isGeneratorFromClosure(false)
  , m_isPairGenerator(false)
  , m_hasGeneratorAsBody(false)
  , m_containsCalls(false)
  , m_info(nullptr)
  , m_builtinFuncPtr(nullptr)
  , m_originalFilename(nullptr)
{}

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
                         PreClassEmitter* pce)
  : m_ue(ue)
  , m_pce(pce)
  , m_sn(sn)
  , m_name(n)
  , m_numLocals(0)
  , m_numUnnamedLocals(0)
  , m_activeUnnamedLocals(0)
  , m_numIterators(0)
  , m_nextFreeIterator(0)
  , m_retTypeConstraint(nullptr)
  , m_returnType(KindOfInvalid)
  , m_top(false)
  , m_isClosureBody(false)
  , m_isGenerator(false)
  , m_isGeneratorFromClosure(false)
  , m_isPairGenerator(false)
  , m_hasGeneratorAsBody(false)
  , m_containsCalls(false)
  , m_info(nullptr)
  , m_builtinFuncPtr(nullptr)
  , m_originalFilename(nullptr)
{}

FuncEmitter::~FuncEmitter() {
}

void FuncEmitter::init(int line1, int line2, Offset base, Attr attrs, bool top,
                       const StringData* docComment) {
  m_line1 = line1;
  m_line2 = line2;
  m_base = base;
  m_attrs = attrs;
  m_top = top;
  m_docComment = docComment;
  if (!SystemLib::s_inited) {
    m_attrs = m_attrs | AttrBuiltin;
    if (!pce()) m_attrs = m_attrs | AttrSkipFrame;
  }
}

void FuncEmitter::finish(Offset past, bool load) {
  m_past = past;
  sortEHTab();
  sortFPITab(load);
}

EHEnt& FuncEmitter::addEHEnt() {
  m_ehtab.push_back(EHEnt());
  return m_ehtab.back();
}

FPIEnt& FuncEmitter::addFPIEnt() {
  m_fpitab.push_back(FPIEnt());
  return m_fpitab.back();
}

Id FuncEmitter::newLocal() {
  return m_numLocals++;
}

void FuncEmitter::appendParam(const StringData* name, const ParamInfo& info) {
  allocVarId(name);
  m_params.push_back(info);
}

void FuncEmitter::allocVarId(const StringData* name) {
  assert(name != nullptr);
  // Unnamed locals are segregated (they all come after the named locals).
  assert(m_numUnnamedLocals == 0);
  UNUSED Id id;
  if (m_localNames.find(name) == m_localNames.end()) {
    id = newLocal();
    assert(id == (int)m_localNames.size());
    m_localNames.add(name, name);
  }
}

Id FuncEmitter::lookupVarId(const StringData* name) const {
  assert(this->hasVar(name));
  return m_localNames.find(name)->second;
}

bool FuncEmitter::hasVar(const StringData* name) const {
  assert(name != nullptr);
  return m_localNames.find(name) != m_localNames.end();
}

Id FuncEmitter::allocIterator() {
  assert(m_numIterators >= m_nextFreeIterator);
  Id id = m_nextFreeIterator++;
  if (m_numIterators < m_nextFreeIterator) {
    m_numIterators = m_nextFreeIterator;
  }
  return id;
}

void FuncEmitter::freeIterator(Id id) {
  --m_nextFreeIterator;
  assert(id == m_nextFreeIterator);
}

void FuncEmitter::setNumIterators(Id numIterators) {
  assert(m_numIterators == 0);
  m_numIterators = numIterators;
}

Id FuncEmitter::allocUnnamedLocal() {
  ++m_activeUnnamedLocals;
  if (m_activeUnnamedLocals > m_numUnnamedLocals) {
    newLocal();
    ++m_numUnnamedLocals;
  }
  return m_numLocals - m_numUnnamedLocals + (m_activeUnnamedLocals - 1);
}

void FuncEmitter::freeUnnamedLocal(Id id) {
  assert(m_activeUnnamedLocals > 0);
  --m_activeUnnamedLocals;
}

void FuncEmitter::setNumLocals(Id numLocals) {
  assert(numLocals >= m_numLocals);
  m_numLocals = numLocals;
}

void FuncEmitter::addStaticVar(Func::SVInfo svInfo) {
  m_staticVars.push_back(svInfo);
}

namespace {

/*
 * Ordering on EHEnts where e1 < e2 iff
 *
 *    a) e1 and e2 do not overlap, and e1 comes first
 *    b) e1 encloses e2
 *    c) e1 and e2 have the same region, but e1 is a Catch funclet and
 *       e2 is a Fault funclet.
 */
struct EHEntComp {
  bool operator()(const EHEnt& e1, const EHEnt& e2) const {
    if (e1.m_base == e2.m_base) {
      if (e1.m_past == e2.m_past) {
        return e1.m_type == EHEnt::Type::Catch;
      }
      return e1.m_past > e2.m_past;
    }
    return e1.m_base < e2.m_base;
  }
};

}

void FuncEmitter::sortEHTab() {
  std::sort(m_ehtab.begin(), m_ehtab.end(), EHEntComp());

  for (unsigned int i = 0; i < m_ehtab.size(); i++) {
    m_ehtab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (m_ehtab[j].m_past >= m_ehtab[i].m_past) {
        // parent EHEnt better enclose this one.
        assert(m_ehtab[j].m_base <= m_ehtab[i].m_base);
        m_ehtab[i].m_parentIndex = j;
        break;
      }
    }
  }
}

void FuncEmitter::sortFPITab(bool load) {
  // Sort it and fill in parent info
  std::sort(m_fpitab.begin(), m_fpitab.end(), FPIEntComp());
  for (unsigned int i = 0; i < m_fpitab.size(); i++) {
    m_fpitab[i].m_parentIndex = -1;
    m_fpitab[i].m_fpiDepth = 1;
    for (int j = i - 1; j >= 0; j--) {
      if (m_fpitab[j].m_fcallOff > m_fpitab[i].m_fcallOff) {
        m_fpitab[i].m_parentIndex = j;
        m_fpitab[i].m_fpiDepth = m_fpitab[j].m_fpiDepth + 1;
        break;
      }
    }
    if (!load) {
      // m_fpOff does not include the space taken up by locals, iterators and
      // the AR itself. Fix it here.
      m_fpitab[i].m_fpOff += m_numLocals
        + m_numIterators * kNumIterCells
        + (m_fpitab[i].m_fpiDepth) * kNumActRecCells;
    }
  }
}

void FuncEmitter::addUserAttribute(const StringData* name, TypedValue tv) {
  m_userAttributes[name] = tv;
}

void FuncEmitter::commit(RepoTxn& txn) const {
  Repo& repo = Repo::get();
  FuncRepoProxy& frp = repo.frp();
  int repoId = m_ue.repoId();
  int64_t usn = m_ue.sn();

  frp.insertFunc(repoId)
     .insert(*this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, m_name, m_top);
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  bool isGenerated = isdigit(m_name->data()[0]) ||
    ParserBase::IsClosureName(m_name->toCPPString()) || m_isGenerator;

  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      ((RuntimeOption::EvalJitEnableRenameFunction && !isGenerated) ||
       (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited))) {
    attrs = Attr(attrs & ~AttrPersistent);
  }
  if (RuntimeOption::EvalJitEnableRenameFunction &&
      !m_name->empty() &&
      !Func::isSpecial(m_name) &&
      !m_isClosureBody &&
      !m_isGenerator) {
    // intercepted functions need to pass all args through
    // to the interceptee
    attrs = attrs | AttrMayUseVV;
  }

  if (!m_containsCalls) attrs = Attr(attrs | AttrPhpLeafFn);

  Func* f = (m_pce == nullptr)
    ? m_ue.newFunc(this, unit, m_id, m_line1, m_line2, m_base,
                   m_past, m_name, attrs, m_top, m_docComment,
                   m_params.size(), m_isClosureBody | m_isGeneratorFromClosure,
                   m_isGenerator)
    : m_ue.newFunc(this, unit, preClass, m_line1, m_line2, m_base,
                   m_past, m_name, attrs, m_top, m_docComment,
                   m_params.size(), m_isClosureBody | m_isGeneratorFromClosure,
                   m_isGenerator);
  f->shared()->m_info = m_info;
  f->shared()->m_returnType = m_returnType;
  std::vector<Func::ParamInfo> pBuilder;
  for (unsigned i = 0; i < m_params.size(); ++i) {
    Func::ParamInfo pi;
    pi.setFuncletOff(m_params[i].funcletOff());
    pi.setDefaultValue(m_params[i].defaultValue());
    pi.setPhpCode(m_params[i].phpCode());
    pi.setTypeConstraint(m_params[i].typeConstraint());
    pi.setUserAttributes(m_params[i].userAttributes());
    pi.setBuiltinType(m_params[i].builtinType());
    pi.setUserType(m_params[i].userType());
    f->appendParam(m_params[i].ref(), pi, pBuilder);
  }
  if (!m_params.size()) {
    assert(!f->m_refBitVal && !f->shared()->m_refBitPtr);
    f->m_refBitVal = attrs & AttrVariadicByRef ? -1uLL : 0uLL;
  }

  f->shared()->m_params = pBuilder;
  f->shared()->m_localNames.create(m_localNames);
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = m_maxStackCells;
  f->shared()->m_staticVars = m_staticVars;
  f->shared()->m_ehtab = m_ehtab;
  f->shared()->m_fpitab = m_fpitab;
  f->shared()->m_isClosureBody = m_isClosureBody;
  f->shared()->m_isGenerator = m_isGenerator;
  f->shared()->m_isGeneratorFromClosure = m_isGeneratorFromClosure;
  f->shared()->m_isPairGenerator = m_isPairGenerator;
  f->shared()->m_hasGeneratorAsBody = m_hasGeneratorAsBody;
  f->shared()->m_userAttributes = m_userAttributes;
  f->shared()->m_builtinFuncPtr = m_builtinFuncPtr;
  f->shared()->m_nativeFuncPtr = m_nativeFuncPtr;
  f->shared()->m_retTypeConstraint = m_retTypeConstraint;
  f->shared()->m_originalFilename = m_originalFilename;
  f->shared()->m_isGenerated = isGenerated;
  return f;
}

void FuncEmitter::setBuiltinFunc(const ClassInfo::MethodInfo* info,
                                 BuiltinFunction bif, BuiltinFunction nif,
                                 Offset base) {
  assert(info);
  assert(bif);
  m_info = info;
  m_builtinFuncPtr = bif;
  m_nativeFuncPtr = nif;
  m_base = base;
  m_top = true;
  m_docComment = StringData::GetStaticString(info->docComment);
  m_line1 = 0;
  m_line2 = 0;
  m_attrs = AttrBuiltin | AttrSkipFrame;
  // TODO: Task #1137917: See if we can avoid marking most builtins with
  // "MayUseVV" and still make things work
  m_attrs = m_attrs | AttrMayUseVV;
  if (info->attribute & (ClassInfo::RefVariableArguments |
                         ClassInfo::MixedVariableArguments)) {
    m_attrs = m_attrs | AttrVariadicByRef;
  }
  if (info->attribute & ClassInfo::IsReference) {
    m_attrs = m_attrs | AttrReference;
  }
  if (info->attribute & ClassInfo::NoInjection) {
    m_attrs = m_attrs | AttrNoInjection;
  }
  if (pce()) {
    if (info->attribute & ClassInfo::IsStatic) {
      m_attrs = m_attrs | AttrStatic;
    }
    if (info->attribute & ClassInfo::IsFinal) {
      m_attrs = m_attrs | AttrFinal;
    }
    if (info->attribute & ClassInfo::IsAbstract) {
      m_attrs = m_attrs | AttrAbstract;
    }
    if (info->attribute & ClassInfo::IsPrivate) {
      m_attrs = m_attrs | AttrPrivate;
    } else if (info->attribute & ClassInfo::IsProtected) {
      m_attrs = m_attrs | AttrProtected;
    } else {
      m_attrs = m_attrs | AttrPublic;
    }
  } else if (info->attribute & ClassInfo::AllowOverride) {
    m_attrs = m_attrs | AttrAllowOverride;
  }

  m_returnType = info->returnType;
  for (unsigned i = 0; i < info->parameters.size(); ++i) {
    // For builtin only, we use a dummy ParamInfo
    FuncEmitter::ParamInfo pi;
    pi.setRef((bool)(info->parameters[i]->attribute & ClassInfo::IsReference));
    pi.setBuiltinType(info->parameters[i]->argType);
    appendParam(StringData::GetStaticString(info->parameters[i]->name), pi);
  }
}

template<class SerDe>
void FuncEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name, top, and a few other fields currently serialized
  // outside of this.
  sd(m_line1)
    (m_line2)
    (m_base)
    (m_past)
    (m_attrs)
    (m_returnType)
    (m_docComment)
    (m_numLocals)
    (m_numIterators)
    (m_maxStackCells)
    (m_isClosureBody)
    (m_isGenerator)
    (m_isGeneratorFromClosure)
    (m_isPairGenerator)
    (m_hasGeneratorAsBody)
    (m_containsCalls)

    (m_params)
    (m_localNames)
    (m_staticVars)
    (m_ehtab)
    (m_fpitab)
    (m_userAttributes)
    (m_retTypeConstraint)
    (m_originalFilename)
    ;
}

//=============================================================================
// FuncRepoProxy.

FuncRepoProxy::FuncRepoProxy(Repo& repo)
  : RepoProxy(repo)
#define FRP_OP(c, o) \
  , m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral)
    FRP_OPS
#undef FRP_OP
{
#define FRP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  FRP_OPS
#undef FRP_OP
}

FuncRepoProxy::~FuncRepoProxy() {
}

void FuncRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  std::stringstream ssCreate;
  ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Func")
           << "(unitSn INTEGER, funcSn INTEGER, preClassId INTEGER,"
              " name TEXT, top INTEGER,"
              " extraData BLOB,"
              " PRIMARY KEY (unitSn, funcSn));";
  txn.exec(ssCreate.str());
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(const FuncEmitter& fe,
                           RepoTxn& txn, int64_t unitSn, int funcSn,
                           Id preClassId, const StringData* name,
                           bool top) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Func")
             << " VALUES(@unitSn, @funcSn, @preClassId, @name, "
                "        @top, @extraData);";
    txn.prepare(*this, ssInsert.str());
  }

  BlobEncoder extraBlob;
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@preClassId", preClassId);
  query.bindStaticString("@name", name);
  query.bindBool("@top", top);
  const_cast<FuncEmitter&>(fe).serdeMetaData(extraBlob);
  query.bindBlob("@extraData", extraBlob, /* static */ true);
  query.exec();
}

void FuncRepoProxy::GetFuncsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT funcSn,preClassId,name,top,extraData "
                "FROM "
             << m_repo.table(m_repoId, "Func")
             << " WHERE unitSn == @unitSn ORDER BY funcSn ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.sn());
  do {
    query.step();
    if (query.row()) {
      int funcSn;               /**/ query.getInt(0, funcSn);
      Id preClassId;            /**/ query.getId(1, preClassId);
      StringData* name;         /**/ query.getStaticString(2, name);
      bool top;                 /**/ query.getBool(3, top);
      BlobDecoder extraBlob =   /**/ query.getBlob(4);

      FuncEmitter* fe;
      if (preClassId < 0) {
        fe = ue.newFuncEmitter(name);
      } else {
        PreClassEmitter* pce = ue.pce(preClassId);
        fe = ue.newMethodEmitter(name, pce);
        bool added UNUSED = pce->addMethod(fe);
        assert(added);
      }
      assert(fe->sn() == funcSn);
      fe->setTop(top);
      fe->serdeMetaData(extraBlob);
      fe->finish(fe->past(), true);
      ue.recordFunction(fe);
    }
  } while (!query.done());
  txn.commit();
}

} // HPHP::VM
