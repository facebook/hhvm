/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "runtime/base/base_includes.h"
#include "util/util.h"
#include "util/trace.h"
#include "util/debug.h"
#include "runtime/base/strings.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/repo.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/eval/runtime/file_repository.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/blob_helper.h"
#include "runtime/vm/func_inline.h"
#include "system/lib/systemlib.h"

namespace HPHP {
namespace VM {

static const Trace::Module TRACEMOD = Trace::bcinterp;
bool Func::s_interceptsEnabled = false;
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

static Func::FuncId s_nextFuncId = 0;
void Func::setFuncId(FuncId id) {
  ASSERT(m_funcId == InvalidId);
  ASSERT(id != InvalidId);
  m_funcId = id;
}

void Func::setNewFuncId() {
  ASSERT(m_funcId == InvalidId);
  m_funcId = __sync_fetch_and_add(&s_nextFuncId, 1);
}

void Func::setFullName() {
  ASSERT(m_name->isStatic());
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

void Func::initPrologues(int numParams) {
  TCA fcallHelper = (TCA)HPHP::VM::Transl::fcallHelperThunk;
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                         : kNumFixedPrologues;

  m_funcBody = (TCA)HPHP::VM::Transl::funcBodyHelperThunk;
  TRACE(2, "initPrologues func %p %d\n", this, numPrologues);
  for (int i = 0; i < numPrologues; i++) {
    m_prologueTable[i] = fcallHelper;
  }
}

void Func::init(int numParams) {
  // For methods, we defer setting the full name until m_cls is initialized
  m_maybeIntercepted = s_interceptsEnabled ? -1 : 0;
  if (!preClass()) {
    setNewFuncId();
    setFullName();
  } else {
    m_fullName = 0;
  }
#ifdef DEBUG
  m_magic = kMagic;
#endif
  ASSERT(m_name);
  initPrologues(numParams);
}

void* Func::allocFuncMem(const StringData* name, int numParams) {
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numExtraPrologues =
    maxNumPrologues > kNumFixedPrologues ?
    maxNumPrologues - kNumFixedPrologues :
    0;
  size_t funcSize = sizeof(Func) + numExtraPrologues * sizeof(unsigned char*);
  return Util::low_malloc(funcSize);
}

Func::Func(Unit& unit, Id id, int line1, int line2,
           Offset base, Offset past, const StringData* name,
           Attr attrs, bool top, const StringData* docComment, int numParams)
  : m_unit(&unit)
  , m_cls(NULL)
  , m_baseCls(NULL)
  , m_name(name)
  , m_namedEntity(NULL)
  , m_refBitVec(NULL)
  , m_cachedOffset(0)
  , m_maxStackCells(0)
  , m_numParams(0)
  , m_attrs(attrs)
  , m_funcId(InvalidId)
  , m_hasPrivateAncestor(false)
{
  m_shared = new SharedData(NULL, id, base, past, line1, line2,
                            top, docComment);
  init(numParams);
}

// Class method
Func::Func(Unit& unit, PreClass* preClass, int line1, int line2, Offset base,
           Offset past, const StringData* name, Attr attrs,
           bool top, const StringData* docComment, int numParams)
  : m_unit(&unit)
  , m_cls(NULL)
  , m_baseCls(NULL)
  , m_name(name)
  , m_namedEntity(NULL)
  , m_refBitVec(NULL)
  , m_cachedOffset(0)
  , m_maxStackCells(0)
  , m_numParams(0)
  , m_attrs(attrs)
  , m_funcId(InvalidId)
  , m_hasPrivateAncestor(false)
{
  Id id = -1;
  m_shared = new SharedData(preClass, id, base, past, line1, line2,
                            top, docComment);
  init(numParams);
}

Func::~Func() {
  if (m_fullName != NULL && s_interceptsEnabled && m_maybeIntercepted != -1) {
    unregister_intercept_flag(fullNameRef(), &m_maybeIntercepted);
  }
#ifdef DEBUG
  validate();
  m_magic = ~m_magic;
#endif
}

void Func::destroy(Func* func) {
  func->~Func();
  Util::low_free(func);
}

Func* Func::clone() const {
  Func* f = new (allocFuncMem(m_name, m_numParams)) Func(*this);
  f->initPrologues(m_numParams);
  f->m_funcId = InvalidId;
  return f;
}

void Func::rename(const StringData* name) {
  m_name = name;
  setFullName();
  // load the renamed function
  Unit::loadFunc(this);
}

/**
 * Return true if Offset o is inside the protected region of a fault
 * funclet for iterId, otherwise false.
 */
bool Func::checkIterScope(Offset o, Id iterId) const {
  const EHEntVec& ehtab = shared()->m_ehtab;
  ASSERT(o >= base() && o < past());
  for (unsigned i = 0, n = ehtab.size(); i < n; i++) {
    const EHEnt* eh = &ehtab[i];
    if (eh->m_ehtype == EHEnt::EHType_Fault &&
        eh->m_base <= o && o < eh->m_past &&
        eh->m_iterId == iterId) {
      return true;
    }
  }
  return false;
}

const EHEnt* Func::findEH(Offset o) const {
  ASSERT(o >= base() && o < past());
  const EHEnt* eh = NULL;
  unsigned int i;

  const EHEntVec& ehtab = shared()->m_ehtab;
  for (i = 0; i < ehtab.size(); i++) {
    if (ehtab[i].m_base <= o && o < ehtab[i].m_past) {
      eh = &ehtab[i];
    }
  }
  return eh;
}

Offset Func::findFaultPCFromEH(Offset o) const {
  ASSERT(o >= base() && o < past());
  unsigned int i = 0;
  int max = -1;

  const EHEntVec& ehtab = shared()->m_ehtab;
  for (i = 0; i < ehtab.size(); i++) {
    if (ehtab[i].m_ehtype == EHEnt::EHType_Catch) {
      continue;
    }
    if (ehtab[i].m_fault < o &&
        (max == -1 ||
         ehtab[i].m_fault > ehtab[max].m_fault)) {
      max = i;
    }
  }
  ASSERT(max != -1);
  return ehtab[max].m_past;
}

const FPIEnt* Func::findFPI(Offset o) const {
  ASSERT(o >= base() && o < past());
  const FPIEnt* fe = NULL;
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
  ASSERT(o >= base() && o < past());
  const FPIEntVec& fpitab = shared()->m_fpitab;
  ASSERT(fpitab.size());
  const FPIEnt* fe = &fpitab[0];
  unsigned int i;
  for (i = 1; i < fpitab.size(); i++) {
    const FPIEnt* cur = &fpitab[i];
    if (o > cur->m_fcallOff &&
        fe->m_fcallOff < cur->m_fcallOff) {
      fe = cur;
    }
  }
  ASSERT(fe);
  return fe;
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

bool Func::byRef(int32 arg) const {
  // Super special case. A handful of builtins are varargs functions where the
  // (not formally declared) varargs are pass-by-reference. psychedelic-kitten
  if (arg >= m_numParams && isBuiltin() &&
      (info()->attribute & (ClassInfo::RefVariableArguments |
                            ClassInfo::MixedVariableArguments))) {
    return true;
  }
  int qword = arg / kBitsPerQword;
  int bit   = arg % kBitsPerQword;
  bool retval = arg < m_numParams && (m_refBitVec[qword] & (1ull << bit)) != 0;
  return retval;
}

bool Func::mustBeRef(int32 arg) const {
  // return true if the argument is required to be a reference
  // (and thus should be an lvalue)
  if (arg >= m_numParams && isBuiltin() &&
      (info()->attribute & (ClassInfo::RefVariableArguments |
                            ClassInfo::MixedVariableArguments) ==
                            ClassInfo::RefVariableArguments)) {
    return true;
  }
  int qword = arg / kBitsPerQword;
  int bit   = arg % kBitsPerQword;
  bool retval = arg < m_numParams && (m_refBitVec[qword] & (1ull << bit)) != 0;
  return retval;
}

void Func::appendParam(bool ref, const Func::ParamInfo& info) {
  int qword = m_numParams / kBitsPerQword;
  int bit   = m_numParams % kBitsPerQword;
  // Grow args, if necessary.
  if ((m_numParams++ & (kBitsPerQword - 1)) == 0) {
    ASSERT(shared()->m_refBitVec == m_refBitVec);
    shared()->m_refBitVec = m_refBitVec = (uint64_t*)
      realloc(shared()->m_refBitVec,
              // E.g., 65th m_numParams -> 2 qwords
              (1 + m_numParams / kBitsPerQword) * sizeof(uint64_t));

    // The new word is either zerod or set to 1, depending on whether
    // we are one of the special builtins that takes variadic
    // reference arguments.  This is for use in the translator.
    shared()->m_refBitVec[m_numParams / kBitsPerQword] =
      (m_attrs & AttrVariadicByRef) ? -1ull : 0;
  }
  ASSERT(!!(shared()->m_refBitVec[qword] & (uint64(1) << bit)) ==
    !!(m_attrs & AttrVariadicByRef));
  shared()->m_refBitVec[qword] &= ~(1ull << bit);
  shared()->m_refBitVec[qword] |= uint64(ref) << bit;
  shared()->m_params.push_back(info);
}

Id Func::lookupVarId(const StringData* name) const {
  ASSERT(name != NULL);
  return shared()->m_localNames.findIndex(name);
}

void Func::prettyPrint(std::ostream& out) const {
  if (isPseudoMain()) {
    out << "Pseudo-main";
  } else if (preClass() != NULL) {
    out << "Method ";
    if (m_attrs & AttrStatic) { out << "static "; }
    if (m_attrs & AttrPublic) { out << "public "; }
    if (m_attrs & AttrProtected) { out << "protected "; }
    if (m_attrs & AttrPrivate) { out << "private "; }
    if (m_attrs & AttrAbstract) { out << "abstract "; }
    if (m_attrs & AttrFinal) { out << "final "; }
    out << preClass()->name()->data() << "::" << m_name->data();
  } else {
    out << "Function " << m_name->data();
  }
  out << " at " << base();
  if (shared()->m_id != -1) {
    out << " (ID " << shared()->m_id << ")";
  }
  out << std::endl;
  const ParamInfoVec& params = shared()->m_params;
  for (uint i = 0; i < params.size(); ++i) {
    if (params[i].funcletOff() != InvalidAbsoluteOffset) {
      out << " DV for parameter " << i << " at " << params[i].funcletOff()
        << " = " << params[i].phpCode()->data() << std::endl;
    }
  }
  const EHEntVec& ehtab = shared()->m_ehtab;
  for (EHEntVec::const_iterator it = ehtab.begin(); it != ehtab.end(); ++it) {
    bool catcher = it->m_ehtype == EHEnt::EHType_Catch;
    out << " EH " << (catcher ? "Catch" : "Fault") << " for " <<
      it->m_base << ":" << it->m_past;
    if (it->m_parentIndex != -1) {
      out << " outer EH " << it->m_parentIndex;
    }
    if (it->m_iterId != -1) {
      out << " iterId " << it->m_iterId;
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
  ASSERT(mi);
  if (info() != NULL) {
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
        pi->value = NULL;
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
          CVarRef v = g_vmContext->getEvaledArg(fpi.phpCode());
          pi->value = strdup(f_serialize(v).get()->data());
        }
        // This is a raw char*, but its lifetime should be at least as long
        // as the the Func*. At this writing, it's a merged anon string
        // owned by ParamInfo.
        pi->valueText = fpi.phpCode()->data();
      }
      pi->type = fpi.typeConstraint().exists() ?
        fpi.typeConstraint().typeName()->data() : "";
      for (UserAttributeMap::const_iterator it = fpi.userAttributes().begin();
           it != fpi.userAttributes().end(); ++it) {
        // convert the typedvalue to a cvarref and push into pi.
        auto userAttr = new ClassInfo::UserAttributeInfo;
        ASSERT(it->first->isStatic());
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
      if ((*it).phpCode != NULL) {
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

Func::SharedData::SharedData(PreClass* preClass, Id id,
                             Offset base, Offset past, int line1, int line2,
                             bool top, const StringData* docComment)
  : m_preClass(preClass), m_id(id), m_base(base),
    m_numLocals(0), m_numIterators(0),
    m_past(past), m_line1(line1), m_line2(line2),
    m_info(NULL), m_refBitVec(NULL), m_builtinFuncPtr(NULL),
    m_docComment(docComment), m_top(top), m_isClosureBody(false),
    m_isGenerator(false), m_isGeneratorFromClosure(false) {
}

Func::SharedData::~SharedData() {
  if (m_refBitVec) {
    free(m_refBitVec);
  }
}

void Func::SharedData::release() {
  delete this;
}

void Func::enableIntercept() {
  // we are protected by s_mutex in intercept.cpp
  if (!s_interceptsEnabled) {
    s_interceptsEnabled = true;
  }
}

Func** Func::getCachedAddr() {
  ASSERT(!isMethod());
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
  : m_ue(ue), m_pce(NULL), m_sn(sn), m_id(id), m_name(n), m_numLocals(0),
    m_numUnnamedLocals(0), m_activeUnnamedLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_top(false), m_isClosureBody(false),
    m_isGenerator(false), m_isGeneratorFromClosure(false), m_info(NULL),
    m_builtinFuncPtr(NULL) {
}

FuncEmitter::FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
                         PreClassEmitter* pce)
  : m_ue(ue), m_pce(pce), m_sn(sn), m_name(n), m_numLocals(0),
    m_numUnnamedLocals(0), m_activeUnnamedLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_top(false), m_isClosureBody(false),
    m_isGenerator(false), m_isGeneratorFromClosure(false), m_info(NULL),
    m_builtinFuncPtr(NULL) {
}

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
  ASSERT(name != NULL);
  // Unnamed locals are segregated (they all come after the named locals).
  ASSERT(m_numUnnamedLocals == 0);
  UNUSED Id id;
  if (m_localNames.find(name) == m_localNames.end()) {
    id = newLocal();
    ASSERT(id == (int)m_localNames.size());
    m_localNames.add(name, name);
  }
}

Id FuncEmitter::lookupVarId(const StringData* name) const {
  ASSERT(name != NULL);
  ASSERT(m_localNames.find(name) != m_localNames.end());
  return m_localNames.find(name)->second;
}

Id FuncEmitter::allocIterator() {
  ASSERT(m_numIterators >= m_nextFreeIterator);
  Id id = m_nextFreeIterator++;
  if (m_numIterators < m_nextFreeIterator) {
    m_numIterators = m_nextFreeIterator;
  }
  return id;
}

void FuncEmitter::freeIterator(Id id) {
  --m_nextFreeIterator;
  ASSERT(id == m_nextFreeIterator);
}

void FuncEmitter::setNumIterators(Id numIterators) {
  ASSERT(m_numIterators == 0);
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
  ASSERT(m_activeUnnamedLocals > 0);
  --m_activeUnnamedLocals;
}

void FuncEmitter::setNumLocals(Id numLocals) {
  ASSERT(numLocals >= m_numLocals);
  m_numLocals = numLocals;
}

void FuncEmitter::addStaticVar(Func::SVInfo svInfo) {
  m_staticVars.push_back(svInfo);
}

void FuncEmitter::sortEHTab() {
  // Sort m_ehtab.
  std::sort(m_ehtab.begin(), m_ehtab.end(), EHEntComp());
  for (unsigned int i = 0; i < m_ehtab.size(); i++) {
    m_ehtab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (m_ehtab[j].m_past > m_ehtab[i].m_past) {
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
  int64 usn = m_ue.sn();

  frp.insertFunc(repoId)
     .insert(*this, txn, usn, m_sn, m_pce ? m_pce->id() : -1, m_name, m_top);
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      (RuntimeOption::EvalJitEnableRenameFunction ||
       (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited))) {
    attrs = Attr(attrs & ~AttrPersistent);
  }

  Func* f = (m_pce == NULL)
    ? m_ue.newFunc(this, unit, m_id, m_line1, m_line2, m_base,
                   m_past, m_name, m_attrs, m_top, m_docComment,
                   m_params.size())
    : m_ue.newFunc(this, unit, preClass, m_line1, m_line2, m_base,
                   m_past, m_name, m_attrs, m_top, m_docComment,
                   m_params.size());
  f->shared()->m_info = m_info;
  for (unsigned i = 0; i < m_params.size(); ++i) {
    Func::ParamInfo pi;
    pi.setFuncletOff(m_params[i].funcletOff());
    pi.setDefaultValue(m_params[i].defaultValue());
    pi.setPhpCode(m_params[i].phpCode());
    pi.setTypeConstraint(m_params[i].typeConstraint());
    pi.setUserAttributes(m_params[i].userAttributes());
    f->appendParam(m_params[i].ref(), pi);
  }
  f->shared()->m_localNames.create(m_localNames);
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = m_maxStackCells;
  ASSERT(m_maxStackCells > 0 && "You probably didn't set m_maxStackCells");
  f->shared()->m_staticVars = m_staticVars;
  f->shared()->m_ehtab = m_ehtab;
  f->shared()->m_fpitab = m_fpitab;
  f->shared()->m_isClosureBody = m_isClosureBody;
  f->shared()->m_isGenerator = m_isGenerator;
  f->shared()->m_isGeneratorFromClosure = m_isGeneratorFromClosure;
  f->shared()->m_userAttributes = m_userAttributes;
  f->shared()->m_builtinFuncPtr = m_builtinFuncPtr;
  return f;
}

void FuncEmitter::setBuiltinFunc(const ClassInfo::MethodInfo* info,
                                 BuiltinFunction funcPtr, Offset base) {
  ASSERT(info);
  ASSERT(funcPtr);
  m_info = info;
  m_builtinFuncPtr = funcPtr;
  m_base = base;
  m_top = true;
  m_docComment = StringData::GetStaticString(info->docComment);
  m_line1 = 0;
  m_line2 = 0;
  m_attrs = AttrNone;
  // TODO: Task #1137917: See if we can avoid marking most builtins with
  // "MayUseVV" and still make things work
  m_attrs = (Attr)(m_attrs | AttrMayUseVV);
  if (info->attribute & (ClassInfo::RefVariableArguments |
                         ClassInfo::MixedVariableArguments)) {
    m_attrs = Attr(m_attrs | AttrVariadicByRef);
  }
  if (info->attribute & ClassInfo::IsReference) {
    m_attrs = (Attr)(m_attrs | AttrReference);
  }
  if (info->attribute & ClassInfo::NoInjection) {
    m_attrs = (Attr)(m_attrs | AttrNoInjection);
  }
  if (pce()) {
    if (info->attribute & ClassInfo::IsStatic) {
      m_attrs = (Attr)(m_attrs | AttrStatic);
    }
    if (info->attribute & ClassInfo::IsFinal) {
      m_attrs = (Attr)(m_attrs | AttrFinal);
    }
    if (info->attribute & ClassInfo::IsAbstract) {
      m_attrs = (Attr)(m_attrs | AttrAbstract);
    }
    if (info->attribute & ClassInfo::IsPrivate) {
      m_attrs = (Attr)(m_attrs | AttrPrivate);
    } else if (info->attribute & ClassInfo::IsProtected) {
      m_attrs = (Attr)(m_attrs | AttrProtected);
    } else {
      m_attrs = (Attr)(m_attrs | AttrPublic);
    }
  }

  for (unsigned i = 0; i < info->parameters.size(); ++i) {
    // For builtin only, we use a dummy ParamInfo
    FuncEmitter::ParamInfo pi;
    pi.setRef((bool)(info->parameters[i]->attribute & ClassInfo::IsReference));
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
    (m_docComment)
    (m_numLocals)
    (m_numIterators)
    (m_maxStackCells)
    (m_isClosureBody)
    (m_isGenerator)
    (m_isGeneratorFromClosure)

    (m_params)
    (m_localNames)
    (m_staticVars)
    (m_ehtab)
    (m_fpitab)
    (m_userAttributes)
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
              " name TEXT, top INTEGER, "
              " extraData BLOB,"
              " PRIMARY KEY (unitSn, funcSn));";
  txn.exec(ssCreate.str());
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(const FuncEmitter& fe,
                           RepoTxn& txn, int64 unitSn, int funcSn,
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
        fe = ue.newFuncEmitter(name, top);
      } else {
        PreClassEmitter* pce = ue.pce(preClassId);
        fe = ue.newMethodEmitter(name, pce);
        bool added UNUSED = pce->addMethod(fe);
        ASSERT(added);
      }
      ASSERT(fe->sn() == funcSn);
      fe->setTop(top);
      fe->serdeMetaData(extraBlob);
      fe->finish(fe->past(), true);
      ue.recordFunction(fe);
    }
  } while (!query.done());
  txn.commit();
}

} } // HPHP::VM
