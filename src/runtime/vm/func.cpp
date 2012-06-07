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
#include <runtime/base/strings.h>
#include "runtime/vm/core_types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/runtime.h"
#include "runtime/vm/repo.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/eval/runtime/file_repository.h"
#include "runtime/vm/translator/translator-x64.h"

namespace HPHP {
namespace VM {

static const Trace::Module TRACEMOD = Trace::bcinterp;
bool Func::s_interceptsEnabled = false;
const StringData* Func::s___call = StringData::GetStaticString("__call");
const StringData* Func::s___callStatic =
  StringData::GetStaticString("__callStatic");

//=============================================================================
// Func.

static bool decl_incompat(bool failIsFatal, const PreClass* implementor,
                          const Func* imeth) {
  if (failIsFatal) {
    const char* name = imeth->name()->data();
    raise_error("Declaration of %s::%s() must be compatible with "
                "that of %s::%s()", implementor->name()->data(), name,
                imeth->cls()->preClass()->name()->data(), name);
  }
  return false;
}

// Check compatibility vs interface and abstract declarations
bool Func::parametersCompat(const PreClass* preClass, const Func* imeth,
                            bool failIsFatal) const {
  const Func::ParamInfoVec& params = this->params();
  const Func::ParamInfoVec& iparams = imeth->params();
  // Verify that meth has at least as many parameters as imeth.
  if ((params.size() < iparams.size())) {
    return decl_incompat(failIsFatal, preClass, imeth);
  }
  // Verify that the typehints for meth's parameters are compatible with
  // imeth's corresponding parameter typehints.
  unsigned firstOptional = 0;
  for (unsigned i = 0; i < iparams.size(); ++i) {
    if (!params[i].typeConstraint().compat(iparams[i].typeConstraint())) {
      return decl_incompat(failIsFatal, preClass, imeth);
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
      return decl_incompat(failIsFatal, preClass, imeth);
    }
  }
  return true;
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
}

void Func::initPrologues(int numParams) {
  TCA fcallHelper = (TCA)HPHP::VM::Transl::fcallHelperThunk;
  int maxNumPrologues = Func::getMaxNumPrologues(numParams);
  int numPrologues =
    maxNumPrologues > kNumFixedPrologues ? maxNumPrologues
                                          : kNumFixedPrologues;
  for (int i=0; i < numPrologues; i++) {
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
  return operator new (funcSize);
}

Func::Func(Unit& unit, Id id, int line1, int line2,
           Offset base, Offset past, const StringData* name,
           Attr attrs, bool top, const StringData* docComment, int numParams)
  : m_unit(&unit), m_cls(NULL), m_baseCls(NULL), m_name(name),
    m_hasPrivateAncestor(false), m_attrs(attrs), m_funcId(InvalidId),
    m_namedEntity(NULL), m_cachedOffset(-1), m_refBitVec(NULL),
    m_maxStackCells(0), m_numParams(0)
{
  m_shared = new SharedData(NULL, id, base, past, line1, line2,
                            top, docComment);
  init(numParams);
}

// Class method
Func::Func(Unit& unit, PreClass* preClass, int line1, int line2, Offset base,
           Offset past, const StringData* name, Attr attrs,
           bool top, const StringData* docComment, int numParams)
  : m_unit(&unit), m_cls(NULL), m_baseCls(NULL), m_name(name),
    m_hasPrivateAncestor(false), m_attrs(attrs), m_funcId(InvalidId),
    m_namedEntity(NULL), m_cachedOffset(-1), m_refBitVec(NULL),
    m_maxStackCells(0), m_numParams(0)
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

bool Func::isNameBindingImmutable(const Unit* fromUnit) const {
  if (RuntimeOption::EvalJitEnableRenameFunction) {
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

  // Using global analysis, we may be able to return true more often; for
  // example, if we statically know this function's name is globally unique.  If
  // we do this, we'll need to be careful about the reloading-code case; unless
  // we throw away the entire TC, we may need to disable this optimization in
  // sandbox mode.
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

Id Func::newLocal() {
  return shared()->m_numLocals++;
}

void Func::appendParam(const StringData* name, bool ref,
                       const Func::ParamInfo& info) {
  allocVarId(name);
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

void Func::allocVarId(const StringData* name) {
  ASSERT(name != NULL);
  Id id;
  PnameMap& pnameMap = shared()->m_pnameMap;
  PnameVec& pnames = shared()->m_pnames;
  if (!mapGet(pnameMap, name, &id)) {
    id = newLocal();
    ASSERT(id == (int)pnames.size());
    pnameMap[name] = id;
    pnames.push_back(name);
  }
}

Id Func::lookupVarId(const StringData* name) const {
  ASSERT(name != NULL);
  const PnameMap& pnameMap = shared()->m_pnameMap;
  ASSERT(mapContains(pnameMap, name));
  return mapGet(pnameMap, name);
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
          << " at " << it2->second << std::endl;
      }
    } else {
      out << " to " << it->m_fault << std::endl;
    }
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
    // hphpc and hphpi set the ClassInfo::VariableArguments attribute if the
    // method contains a call to func_get_arg, func_get_args, or func_num_args.
    // We don't do this in the VM currently and hopefully we never will need to.
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
      pi->name = shared()->m_pnames[i]->data();
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

Func::SharedData::SharedData(PreClass* preClass,
                             const ClassInfo::MethodInfo* info,
                             BuiltinFunction builtinFuncPtr)
  : m_preClass(preClass), m_id(-1), m_base(0),
    m_numLocals(0), m_numIterators(0),
    m_past(0), m_line1(0), m_line2(0),
    m_info(info), m_refBitVec(NULL), m_builtinFuncPtr(builtinFuncPtr),
    m_docComment(NULL), m_top(false), m_isClosureBody(false),
    m_isGenerator(false), m_isGeneratorFromClosure(false) {
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

void Func::enableIntercept(CStrRef name) {
  // we are protected by s_mutex in intercept.cpp
  if (!s_interceptsEnabled) {
    s_interceptsEnabled = true;
  }
}

Func** Func::getCachedAddr() {
  ASSERT(!isMethod());
  if (UNLIKELY(m_cachedOffset == (unsigned)-1)) {
    Unit::loadFunc(this);
  }
  return (Func**)Transl::TargetCache::handleToPtr(m_cachedOffset);
}

void Func::setCached() {
  ASSERT(!isMethod());
  Func** funcAddr = getCachedAddr();
  if (UNLIKELY(*funcAddr != NULL)) {
    if (*funcAddr == this) return;
    if (!(*funcAddr)->isIgnoreRedefinition()) {
      raise_error(Strings::FUNCTION_ALREADY_DEFINED, name()->data());
    }
  }
  *funcAddr = this;
  DEBUGGER_ATTACHED_ONLY(phpDefFuncHook(this));
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
  Id id;
  if (!mapGet(m_pnameMap, name, &id)) {
    id = newLocal();
    ASSERT(id == (int)m_pnames.size());
    m_pnameMap[name] = id;
    m_pnames.push_back(name);
  }
}

Id FuncEmitter::lookupVarId(const StringData* name) const {
  ASSERT(name != NULL);
  ASSERT(mapContains(m_pnameMap, name));
  return mapGet(m_pnameMap, name);
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
     .insert(txn, usn, m_sn, m_pce ? m_pce->id() : -1, m_id, m_base,
             m_past, m_line1, m_line2, m_name, m_numLocals, m_numIterators,
             m_maxStackCells, m_attrs, m_top, m_docComment, m_isClosureBody,
             m_isGenerator, m_isGeneratorFromClosure);
  for (unsigned i = 0; i < m_params.size(); ++i) {
    const TypeConstraint& tc = m_params[i].typeConstraint();
    frp.insertFuncParam(repoId)
       .insert(txn, usn, m_sn, i, m_pnames[i], m_params[i].funcletOff(),
               m_params[i].defaultValue(), m_params[i].phpCode(), tc,
               m_params[i].ref());
  }
  for (unsigned i = m_params.size(); i < m_pnames.size(); ++i) {
    frp.insertFuncVar(repoId).insert(txn, usn, m_sn, i, m_pnames[i]);
  }
  for (unsigned i = 0; i < m_staticVars.size(); ++i) {
    const Func::SVInfo& svi = m_staticVars[i];
    frp.insertFuncStaticVar(repoId)
       .insert(txn, usn, m_sn, i, svi.name, svi.phpCode);
  }
  for (unsigned i = 0; i < m_ehtab.size(); ++i) {
    const EHEnt& eh = m_ehtab[i];
    frp.insertFuncEH(repoId)
       .insert(txn, usn, m_sn, i, eh.m_ehtype, eh.m_base, eh.m_past,
               eh.m_iterId, eh.m_parentIndex, eh.m_fault);
    for (unsigned j = 0; j < eh.m_catches.size(); ++j) {
      const std::pair<Id, Offset>& ehcatch = eh.m_catches[j];
      frp.insertFuncEHCatch(repoId)
         .insert(txn, usn, m_sn, i, j, ehcatch.first, ehcatch.second);
    }
  }
  for (unsigned i = 0; i < m_fpitab.size(); ++i) {
    const FPIEnt& fpi = m_fpitab[i];
    frp.insertFuncFPI(repoId)
       .insert(txn, usn, m_sn, i, fpi.m_fpushOff, fpi.m_fcallOff,
               fpi.m_fpOff, fpi.m_parentIndex, fpi.m_fpiDepth);
  }
  for (Func::UserAttributeMap::const_iterator it = m_userAttributes.begin();
       it != m_userAttributes.end(); ++it) {
    const StringData* name = it->first;
    const TypedValue& tv = it->second;
    frp.insertFuncUserAttribute(repoId)
       .insert(txn, usn, m_sn, name, tv);
  }
}

Func* FuncEmitter::create(Unit& unit, PreClass* preClass /* = NULL */) const {
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
    f->appendParam(m_pnames[i], m_params[i].ref(), pi);
  }
  for (unsigned i = m_params.size(); i < m_pnames.size(); ++i) {
    f->allocVarId(m_pnames[i]);
  }
  f->shared()->m_numLocals = m_numLocals;
  f->shared()->m_numIterators = m_numIterators;
  f->m_maxStackCells = m_maxStackCells;
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
  m_docComment = StringData::GetStaticString("");
  m_attrs = AttrNone;
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

//=============================================================================
// FuncRepoProxy.

FuncRepoProxy::FuncRepoProxy(Repo& repo)
  : RepoProxy(repo),
#define FRP_OP(c, o) \
    m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral),
    FRP_OPS
#undef FRP_OP
    m_dummy(0) {
#define FRP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  FRP_OPS
#undef FRP_OP
}

FuncRepoProxy::~FuncRepoProxy() {
}

void FuncRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Func")
             << "(unitSn INTEGER, funcSn INTEGER, preClassId INTEGER,"
                " funcId INTEGER, base INTEGER, past INTEGER,"
                " line1 INTEGER, line2 INTEGER, name TEXT, numLocals INTEGER,"
                " numIterators INTEGER, maxStackCells INTEGER, attrs INTEGER,"
                " top INTEGER, docComment TEXT, isClosureBody INTEGER,"
                " isGenerator INTEGER, isGeneratorFromClosure INTEGER,"
                " PRIMARY KEY (unitSn, funcSn));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncParam")
             << "(unitSn INTEGER, funcSn INTEGER, localId INTEGER, name TEXT,"
                " funcletOff INTEGER, defaultValue BLOB, phpCode TEXT,"
                " typeConstraint TEXT, typeConstraintNullable INTEGER,"
                " ref INTEGER, PRIMARY KEY (unitSn, funcSn, localId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncVar")
             << "(unitSn INTEGER, funcSn INTEGER, localId INTEGER, name TEXT,"
                " PRIMARY KEY (unitSn, funcSn, localId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncStaticVar")
             << "(unitSn INTEGER, funcSn INTEGER, staticVarSn INTEGER,"
                " name TEXT, phpCode TEXT, PRIMARY KEY (unitSn, funcSn,"
                " staticVarSn));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncEH")
             << "(unitSn INTEGER, funcSn INTEGER, ehSn INTEGER,"
                " type INTEGER, base INTEGER, past INTEGER,"
                " iterId INTEGER, parentIndex INTEGER, fault INTEGER,"
                " PRIMARY KEY (unitSn, funcSn, ehSn));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncEHCatch")
             << "(unitSn INTEGER, funcSn INTEGER, ehSn INTEGER,"
                " ehCatchSn INTEGER, nameId INTEGER, offset INTEGER,"
                " PRIMARY KEY (unitSn, funcSn, ehSn, ehCatchSn));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncFPI")
             << "(unitSn INTEGER, funcSn INTEGER, fpiSn INTEGER,"
                " base INTEGER, past INTEGER, fpOff INTEGER,"
                " parentIndex INTEGER, fpiDepth INTEGER,"
                " PRIMARY KEY (unitSn, funcSn, fpiSn));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "FuncUserAttribute")
             << "(unitSn INTEGER, funcSn INTEGER, name TEXT, value BLOB,"
                " PRIMARY KEY (unitSn, funcSn, name));";
    txn.exec(ssCreate.str());
  }
}

void FuncRepoProxy::InsertFuncStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           Id preClassId, Id funcId, Offset base,
                           Offset past, int line1, int line2,
                           const StringData* name, Id numLocals,
                           Id numIterators, int maxStackCells, Attr attrs,
                           bool top, const StringData* docComment,
                           bool isClosureBody, bool isGenerator,
                           bool isGeneratorFromClosure) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Func")
             << " VALUES(@unitSn, @funcSn, @preClassId, @funcId, @base,"
                " @past, @line1, @line2, @name, @numLocals,"
                " @numIterators, @maxStackCells, @attrs, @top, @docComment,"
                " @isClosureBody, @isGenerator, @isGeneratorFromClosure);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@preClassId", preClassId);
  query.bindId("@funcId", funcId);
  query.bindOffset("@base", base);
  query.bindOffset("@past", past);
  query.bindInt("@line1", line1);
  query.bindInt("@line2", line2);
  query.bindStaticString("@name", name);
  query.bindId("@numLocals", numLocals);
  query.bindId("@numIterators", numIterators);
  query.bindInt("@maxStackCells", maxStackCells);
  query.bindAttr("@attrs", attrs);
  query.bindBool("@top", top);
  query.bindStaticString("@docComment", docComment);
  query.bindBool("@isClosureBody", isClosureBody);
  query.bindBool("@isGenerator", isGenerator);
  query.bindBool("@isGeneratorFromClosure", isGeneratorFromClosure);
  query.exec();
}

void FuncRepoProxy::GetFuncsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT funcSn,preClassId,funcId,base,past,line1,"
                "line2,name,numLocals,numIterators,maxStackCells,attrs,top,"
                "docComment,isClosureBody,isGenerator,isGeneratorFromClosure "
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
      Id funcId;                /**/ query.getId(2, funcId);
      Offset base;              /**/ query.getOffset(3, base);
      Offset past;              /**/ query.getOffset(4, past);
      int line1;                /**/ query.getInt(5, line1);
      int line2;                /**/ query.getInt(6, line2);
      StringData* name;         /**/ query.getStaticString(7, name);
      Id numLocals;             /**/ query.getId(8, numLocals);
      Id numIterators;          /**/ query.getId(9, numIterators);
      int maxStackCells;        /**/ query.getInt(10, maxStackCells);
      Attr attrs;               /**/ query.getAttr(11, attrs);
      bool top;                 /**/ query.getBool(12, top);
      StringData* docComment;   /**/ query.getStaticString(13, docComment);
      bool isClosureBody;       /**/ query.getBool(14, isClosureBody);
      bool isGenerator;         /**/ query.getBool(15, isGenerator);
      bool isGeneratorFromClosure;
                                /**/ query.getBool(16, isGeneratorFromClosure);
      FuncEmitter* fe;
      if (preClassId < 0) {
        fe = ue.newFuncEmitter(name, top);
        ASSERT(fe->id() == funcId);
      } else {
        PreClassEmitter* pce = ue.pce(preClassId);
        fe = ue.newMethodEmitter(name, pce);
        bool added UNUSED = pce->addMethod(fe);
        ASSERT(added);
      }
      ASSERT(fe->sn() == funcSn);
      fe->init(line1, line2, base, attrs, top, docComment);
      m_repo.frp().getFuncParams(m_repoId).get(*fe);
      m_repo.frp().getFuncVars(m_repoId).get(*fe);
      m_repo.frp().getFuncStaticVars(m_repoId).get(*fe);
      m_repo.frp().getFuncEHs(m_repoId).get(*fe);
      m_repo.frp().getFuncFPIs(m_repoId).get(*fe);
      m_repo.frp().getFuncUserAttributes(m_repoId).get(*fe);
      fe->setNumLocals(numLocals);
      fe->setNumIterators(numIterators);
      fe->setMaxStackCells(maxStackCells);
      fe->setIsClosureBody(isClosureBody);
      fe->setIsGenerator(isGenerator);
      fe->setIsGeneratorFromClosure(isGeneratorFromClosure);
      fe->finish(past, true);
      ue.recordFunction(fe);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncParamStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           Id localId, const StringData* name,
                           Offset funcletOff, const TypedValue& defaultValue,
                           const StringData* phpCode, const TypeConstraint& tc,
                           bool ref) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncParam")
             << " VALUES(@unitSn, @funcSn, @localId, @name, @funcletOff,"
                " @defaultValue, @phpCode, @typeConstraint,"
                " @typeConstraintNullable, @ref);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@localId", localId);
  query.bindStaticString("@name", name);
  query.bindOffset("@funcletOff", funcletOff);
  query.bindTypedValue("@defaultValue", defaultValue);
  query.bindStaticString("@phpCode", phpCode);
  query.bindStaticString("@typeConstraint", tc.typeName());
  query.bindBool("@typeConstraintNullable", tc.nullable());
  query.bindBool("@ref", ref);
  query.exec();
}

void FuncRepoProxy::GetFuncParamsStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT localId,name,funcletOff,defaultValue,phpCode,"
                "typeConstraint,typeConstraintNullable,ref FROM "
             << m_repo.table(m_repoId, "FuncParam")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY localId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      Id localId;                  /**/ query.getId(0, localId);
      StringData* name;            /**/ query.getStaticString(1, name);
      Offset funcletOff;           /**/ query.getOffset(2, funcletOff);
      TypedValue defaultValue;     /**/ query.getTypedValue(3, defaultValue);
      StringData* phpCode;         /**/ query.getStaticString(4, phpCode);
      StringData* typeConstraint;  /**/ query.getStaticString(5,
                                                              typeConstraint);
      bool typeConstraintNullable; /**/ query.getBool(6,
                                                      typeConstraintNullable);
      bool ref;                    /**/ query.getBool(7, ref);
      FuncEmitter::ParamInfo pi;
      pi.setFuncletOff(funcletOff);
      pi.setDefaultValue(defaultValue);
      pi.setPhpCode(phpCode);
      TypeConstraint tc(typeConstraint, typeConstraintNullable);
      pi.setTypeConstraint(tc);
      pi.setRef(ref);
      fe.appendParam(name, pi);
      ASSERT(fe.lookupVarId(name) == localId);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncVarStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           Id localId, const StringData* name) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncVar")
             << " VALUES(@unitSn, @funcSn, @localId, @name);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindId("@localId", localId);
  query.bindStaticString("@name", name);
  query.exec();
}

void FuncRepoProxy::GetFuncVarsStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT localId,name FROM "
             << m_repo.table(m_repoId, "FuncVar")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY localId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      Id localId;       /**/ query.getId(0, localId);
      StringData* name; /**/ query.getStaticString(1, name);
      fe.allocVarId(name);
      ASSERT(fe.lookupVarId(name) == localId);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncStaticVarStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           int staticVarSn, const StringData* name,
                           const StringData* phpCode) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncStaticVar")
             << " VALUES(@unitSn, @funcSn, @staticVarSn, @name, @phpCode);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindInt("@staticVarSn", staticVarSn);
  query.bindStaticString("@name", name);
  query.bindStaticString("@phpCode", phpCode);
  query.exec();
}

void FuncRepoProxy::GetFuncStaticVarsStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT name,phpCode FROM "
             << m_repo.table(m_repoId, "FuncStaticVar")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY staticVarsn ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      StringData* name;        /**/ query.getStaticString(0, name);
      StringData* phpCode;     /**/ query.getStaticString(1, phpCode);
      Func::SVInfo svInfo;
      svInfo.name = name;
      svInfo.phpCode = phpCode;
      fe.addStaticVar(svInfo);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncEHStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           int ehSn, int type, Offset base, Offset past,
                           int iterId, int parentIndex, Offset fault) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncEH")
             << " VALUES(@unitSn, @funcSn, @ehSn, @type, @base, @past,"
                " @iterId, @parentIndex, @fault);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindInt("@ehSn", ehSn);
  query.bindInt("@type", type);
  query.bindOffset("@base", base);
  query.bindOffset("@past", past);
  query.bindInt("@iterId", iterId);
  query.bindInt("@parentIndex", parentIndex);
  query.bindOffset("@fault", fault);
  query.exec();
}

void FuncRepoProxy::GetFuncEHsStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT ehSn,type,base,past,iterId,parentIndex,fault FROM "
             << m_repo.table(m_repoId, "FuncEH")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY ehSn ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      int ehSn;                  /**/ query.getInt(0, ehSn);
      EHEnt& eh = fe.addEHEnt(); /**/ query.getInt(1, (int&)eh.m_ehtype);
                                 /**/ query.getOffset(2, eh.m_base);
                                 /**/ query.getOffset(3, eh.m_past);
                                 /**/ query.getInt(4, eh.m_parentIndex);
                                 /**/ query.getInt(5, eh.m_parentIndex);
                                 /**/ query.getOffset(6, eh.m_fault);
      m_repo.frp().getFuncEHCatches(m_repoId).get(fe, ehSn, eh);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncEHCatchStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           int ehSn, int ehCatchSn, Id nameId, Offset offset) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncEHCatch")
             << " VALUES(@unitSn, @funcSn, @ehSn, @ehCatchSn, @nameId,"
                " @offset);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindInt("@ehSn", ehSn);
  query.bindInt("@ehCatchSn", ehCatchSn);
  query.bindId("@nameId", nameId);
  query.bindOffset("@offset", offset);
  query.exec();
}

void FuncRepoProxy::GetFuncEHCatchesStmt
                  ::get(FuncEmitter& fe, int ehSn, EHEnt& eh) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT nameId,offset FROM "
             << m_repo.table(m_repoId, "FuncEHCatch")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " AND ehSn == @ehSn ORDER BY ehCatchSn ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  query.bindInt("@ehSn", ehSn);
  do {
    query.step();
    if (query.row()) {
      Id nameId;     /**/ query.getId(0, nameId);
      Offset offset; /**/ query.getOffset(1, offset);
      eh.m_catches.push_back(std::pair<Id, Offset>(nameId, offset));
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncFPIStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn, int fpiSn,
                           Offset base, Offset past, Offset fpOff,
                           int parentIndex, int fpiDepth) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncFPI")
             << " VALUES(@unitSn, @funcSn, @fpiSn, @base, @past, @fpOff,"
                " @parentIndex, @fpiDepth);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindInt("@fpiSn", fpiSn);
  query.bindOffset("@base", base);
  query.bindOffset("@past", past);
  query.bindOffset("@fpOff", fpOff);
  query.bindInt("@parentIndex", parentIndex);
  query.bindInt("@fpiDepth", fpiDepth);
  query.exec();
}

void FuncRepoProxy::GetFuncFPIsStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT base,past,fpOff,parentIndex,fpiDepth FROM "
             << m_repo.table(m_repoId, "FuncFPI")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY fpiSn ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      FPIEnt& fpi = fe.addFPIEnt(); /**/ query.getOffset(0, fpi.m_fpushOff);
                                    /**/ query.getOffset(1, fpi.m_fcallOff);
                                    /**/ query.getOffset(2, fpi.m_fpOff);
                                    /**/ query.getInt(3, fpi.m_parentIndex);
                                    /**/ query.getInt(4, fpi.m_fpiDepth);
    }
  } while (!query.done());
  txn.commit();
}

void FuncRepoProxy::InsertFuncUserAttributeStmt
                  ::insert(RepoTxn& txn, int64 unitSn, int funcSn,
                           const StringData* name, const TypedValue& tv) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "FuncUserAttribute")
             << " VALUES(@unitSn, @funcSn, @name, @value);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@funcSn", funcSn);
  query.bindStaticString("@name", name);
  query.bindTypedValue("@value", tv);
  query.exec();
}

void FuncRepoProxy::GetFuncUserAttributesStmt
                  ::get(FuncEmitter& fe) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT name, value FROM "
             << m_repo.table(m_repoId, "FuncUserAttribute")
             << " WHERE unitSn == @unitSn AND funcSn == @funcSn"
                " ORDER BY name ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", fe.ue().sn());
  query.bindInt("@funcSn", fe.sn());
  do {
    query.step();
    if (query.row()) {
      StringData* name;           /**/ query.getStaticString(0, name);
      TypedValue tv;              /**/ query.getTypedValue(1, tv);
      fe.addUserAttribute(name, tv);
    }
  } while (!query.done());
  txn.commit();
}

} } // HPHP::VM
