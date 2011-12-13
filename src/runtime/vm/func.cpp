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
#include "runtime/vm/core_types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/runtime.h"

namespace HPHP {
namespace VM {

static const Trace::Module TRACEMOD = Trace::bcinterp;

void Func::init() {
#ifdef DEBUG
  m_base = (Offset)0xdeadbeef;
  m_funclets = (Offset)0xdeadbeef;
  m_past = (Offset)0xdeadbeef;
  m_attrs = (Attr)0xdeadbeef;
  m_docComment = (StringData*)0xba5eba11acc01adeLL;
  m_magic = kMagic;
#endif
  if (m_info) {
    for (unsigned i = 0; i < m_info->parameters.size(); ++i) {
      appendParam(StringData::GetStaticString(m_info->parameters[i]->name),
                m_info->parameters[i]->attribute & ClassInfo::IsReference);
    }
  }
  if (m_preClass) {
    m_fullName = StringData::GetStaticString(
      std::string(m_preClass->m_name->data()) + "::" + m_name->data());
  } else {
    m_fullName = StringData::GetStaticString(m_name->data());
  }
}

//=============================================================================
// Func.

Func::Func(Unit& unit, Id id, const StringData* n)
  : m_unit(&unit), m_id(id), m_name(n), m_fullName(NULL), m_info(NULL),
    m_preClass(NULL), m_top(false), m_numLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_builtinFuncPtr(NULL),
    m_builtinClassFuncPtr(NULL), m_isClosureBody(false), m_isGenerator(false),
    m_needsStaticLocalCtx(false), m_numParams(0),
    m_refBitVec(NULL), m_nextFreeUnnamedLocal(0), m_maxStackCells(0) {
  init();
}

// Builtin
Func::Func(const StringData* n, const ClassInfo::MethodInfo* info,
           BuiltinFunction funcPtr)
  : m_unit(NULL), m_id(-1), m_name(n), m_fullName(NULL), m_info(info),
    m_preClass(NULL), m_top(false), m_numLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_builtinFuncPtr(funcPtr),
    m_builtinClassFuncPtr(NULL), m_isClosureBody(false), m_isGenerator(false),
    m_needsStaticLocalCtx(false), m_numParams(0), m_refBitVec(NULL),
    m_nextFreeUnnamedLocal(0), m_maxStackCells(0) {
  init();
  m_attrs = AttrNone;
}

// Builtin Class method
Func::Func(const StringData*n, const ClassInfo::MethodInfo* info,
           PreClass* preClass, BuiltinClassFunction funcPtr)
  : m_unit(NULL), m_id(-1), m_name(n), m_fullName(NULL), m_info(info),
    m_preClass(preClass), m_top(false), m_numLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_builtinFuncPtr(NULL),
    m_builtinClassFuncPtr(funcPtr), m_isClosureBody(false),
    m_isGenerator(false), m_needsStaticLocalCtx(false), m_numParams(0),
    m_refBitVec(NULL), m_nextFreeUnnamedLocal(0), m_maxStackCells(0) {
  init();
  m_attrs = AttrNone;
  if (m_info->attribute & ClassInfo::IsInterface) {
    m_attrs = (Attr)(m_attrs | AttrInterface);
  }
  if (m_info->attribute & ClassInfo::IsAbstract) {
    m_attrs = (Attr)(m_attrs | AttrAbstract);
  }
  if (m_info->attribute & ClassInfo::IsFinal) {
    m_attrs = (Attr)(m_attrs | AttrFinal);
  }
  if (m_info->attribute & ClassInfo::IsPublic) {
    m_attrs = (Attr)(m_attrs | AttrPublic);
  }
  if (m_info->attribute & ClassInfo::IsProtected) {
    m_attrs = (Attr)(m_attrs | AttrProtected);
  }
  if (m_info->attribute & ClassInfo::IsPrivate) {
    m_attrs = (Attr)(m_attrs | AttrPrivate);
  }
  if (m_info->attribute & ClassInfo::IsStatic) {
    m_attrs = (Attr)(m_attrs | AttrStatic);
  }
}

// Class method
Func::Func(Unit& unit, const StringData* n, PreClass* preClass)
  : m_unit(&unit), m_id(-1), m_name(n), m_fullName(NULL), m_info(NULL),
    m_preClass(preClass), m_top(false), m_numLocals(0), m_numIterators(0),
    m_nextFreeIterator(0), m_builtinFuncPtr(NULL),
    m_builtinClassFuncPtr(NULL), m_isClosureBody(false), m_isGenerator(false),
    m_needsStaticLocalCtx(false), m_numParams(0),
    m_refBitVec(NULL), m_nextFreeUnnamedLocal(0), m_maxStackCells(0) {
  init();
}

Func::~Func() {
#ifdef DEBUG
  ASSERT(m_magic == kMagic);
  m_magic = ~m_magic;
#endif
  if (m_refBitVec) {
    free(m_refBitVec);
  }
}

void Func::appendParam(const StringData* name, bool ref) {
  allocVarId(name);
  int qword = m_numParams / kBitsPerQword;
  int bit   = m_numParams % kBitsPerQword;
  // Grow args, if necessary.
  if ((m_numParams++ & (kBitsPerQword - 1)) == 0) {
    m_refBitVec = (uint64_t*)
      realloc(m_refBitVec,
              // E.g., 65th m_numParams -> 2 qwords
              (1 + m_numParams / kBitsPerQword) * sizeof(uint64_t));
    m_refBitVec[m_numParams / kBitsPerQword] = 0;
  }
  ASSERT((m_refBitVec[qword] & (uint64(1) << bit)) == 0);
  m_refBitVec[qword] |= uint64(ref) << bit;
}

EHEnt &Func::addEHEnt() {
  m_ehtab.push_back(EHEnt());
  return m_ehtab.back();
}

FEEnt &Func::addFEEnt() {
  m_fetab.push_back(FEEnt());
  return m_fetab.back();
}

FPIEnt &Func::addFPIEnt() {
  m_fpitab.push_back(FPIEnt());
  return m_fpitab.back();
}

bool Func::checkIterScope(Offset o, Id iterId) const {
  ASSERT(o >= m_base && o < m_past);
  for (unsigned i = 0; i < m_fetab.size(); i++) {
    if (m_fetab[i].m_base <= o &&
        m_fetab[i].m_past > o &&
        m_fetab[i].m_iterId == iterId) {
      return true;
    }
  }
  return false;
}

const FPIEnt* Func::findFPI(Offset o) const {
  if (isBuiltin()) return NULL;

  ASSERT(o >= m_base && o < m_past);
  const FPIEnt* fe = NULL;
  unsigned int i;

  for (i = 0; i < m_fpitab.size(); i++) {
    if (m_fpitab[i].m_base <= o && m_fpitab[i].m_past > o) {
      fe = &m_fpitab[i];
    }
  }
  return fe;
}

const EHEnt* Func::findEH(Offset o) const {
  if (isBuiltin()) return NULL;

  ASSERT(o >= m_base && o < m_past);
  const EHEnt* eh = NULL;
  unsigned int i;
  for (i = 0; i < m_ehtab.size(); i++) {
    if (m_ehtab[i].m_base < o && m_ehtab[i].m_past > o) {
      eh = &m_ehtab[i];
    }
  }
  return eh;
}

Offset Func::findFaultPCFromEH(Offset o) const {
  ASSERT(!isBuiltin());
  ASSERT(o >= m_base && o < m_past);
  unsigned int i = 0;
  int max = -1;

  for (i = 0; i < m_ehtab.size(); i++) {
    if (m_ehtab[i].m_ehtype == EHEnt::EHType_Catch) {
      continue;
    }
    if  (m_ehtab[i].m_fault < o &&
         (max == -1 ||
          m_ehtab[i].m_fault > m_ehtab[max].m_fault)) {
      max = i;
    }
  }
  ASSERT(max != -1);
  return m_ehtab[max].m_past;
}

bool Func::isNameBindingImmutable(const Unit* fromUnit) const {
  if (isBuiltin()) {
    return true;
  }

  if (RuntimeOption::EvalJitEnableRenameFunction) {
    return false;
  }

  // Defined at top level, in the same unit as the caller. This precludes
  // conditionally defined functions and cross-module calls -- both phenomena
  // can change name->Func mappings during the lifetime of a TC.
  return m_top && (fromUnit == m_unit);

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
      (m_info->attribute & (ClassInfo::RefVariableArguments |
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
      (m_info->attribute & (ClassInfo::RefVariableArguments |
                            ClassInfo::MixedVariableArguments) ==
                            ClassInfo::RefVariableArguments)) {
    return true;
  }
  int qword = arg / kBitsPerQword;
  int bit   = arg % kBitsPerQword;
  bool retval = arg < m_numParams && (m_refBitVec[qword] & (1ull << bit)) != 0;
  return retval;
}

Id Func::lookupVarId(const StringData* name) const {
  ASSERT(name != NULL);
  Id id;
  if (mapGet(m_name2pind, name, &id)) {
    return id;
  }
  ASSERT(false);
  return -1;
}

void Func::allocVarId(const StringData* name) {
  ASSERT(name != NULL);
  Id id;
  if (!mapGet(m_name2pind, name, &id)) {
    id = newLocal();
    ASSERT(id == (int)m_pnames.size());
    m_name2pind[name] = id;
    m_pnames.push_back(name);
  }
}

void Func::init(const Location* sLoc, Offset base, Attr attrs, bool top,
                const StringData* docComment) {
  m_line1 = sLoc->line0;
  m_line2 = sLoc->line1;
  m_base = base;
  m_attrs = attrs;
  m_top = top;
  m_docComment = docComment;
}

void Func::finish(Offset funclets, Offset past) {
  m_funclets = funclets;
  m_past = past;
}

void Func::prettyPrint(std::ostream &out) const {
  if (isPseudoMain()) {
    out << "Pseudo-main";
  } else if (m_preClass != NULL) {
    out << "Method ";
    if (m_attrs & AttrStatic) { out << "static "; }
    if (m_attrs & AttrPublic) { out << "public "; }
    if (m_attrs & AttrProtected) { out << "protected "; }
    if (m_attrs & AttrPrivate) { out << "private "; }
    if (m_attrs & AttrAbstract) { out << "abstract "; }
    if (m_attrs & AttrFinal) { out << "final "; }
    out << m_preClass->m_name->data() << "::" << m_name->data();
  } else {
    out << "Function " << m_name->data();
  }
  out << " at " << m_base;
  if (m_id != -1) {
    out << " (ID " << m_id << ")";
  }
  out << std::endl;
  for (uint i = 0; i < m_params.size(); ++i) {
    if (m_params[i].m_funcletOff != InvalidAbsoluteOffset) {
      out << " DV for parameter " << i << " at " << m_params[i].m_funcletOff
        << " = " << m_params[i].m_phpCode->data() << std::endl;
    }
  }
  for (std::vector<EHEnt>::const_iterator it = m_ehtab.begin();
       it != m_ehtab.end(); ++it) {
    bool catcher = it->m_ehtype == EHEnt::EHType_Catch;
    out << " EH " << (catcher ? "Catch" : "Fault") << " for " <<
      it->m_base << ":" << it->m_past;
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
  for (unsigned int i = 0; i < m_fetab.size(); i++) {
    out << " FE " << i << " iter " << m_fetab[i].m_iterId
      << " at " << m_fetab[i].m_base << " : " << m_fetab[i].m_past
      << " outer FE " << m_fetab[i].m_parentIndex << std::endl;
  }
}

void Func::getFuncInfo(ClassInfo::MethodInfo* mi) const {
  ASSERT(mi);
  if (m_info != NULL) {
    // Very large operator=() invocation.
    *mi = *m_info;
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
    if (m_preClass &&
        (!strcasecmp(m_name->data(), "__construct") ||
         (!(m_preClass->m_attrs & AttrTrait) &&
          !strcasecmp(m_name->data(), m_preClass->m_name->data()) &&
          !m_preClass->hasMethod(String("__construct").get())))) {
      attr |= ClassInfo::IsConstructor;
    }
    if (attr == 0) attr = ClassInfo::IsNothing;
    mi->attribute = (ClassInfo::Attribute)attr;
    mi->name = m_name->data();
    mi->file = m_unit->m_filepath->data();
    mi->line1 = m_line1;
    mi->line2 = m_line2;
    mi->docComment = m_docComment->data();
    // Get the parameter info
    for (unsigned i = 0; i < unsigned(m_numParams); ++i) {
      ClassInfo::ParameterInfo *pi = new ClassInfo::ParameterInfo;
      attr = 0;
      if (byRef(i)) {
        attr |= ClassInfo::IsReference;
      }
      if (attr == 0) {
        attr = ClassInfo::IsNothing;
      }
      const ParamInfo& fpi = m_params[i];
      if (m_params.size() > i && m_params[i].hasDefaultValue()) {
        attr |= ClassInfo::IsOptional;
      }
      pi->attribute = (ClassInfo::Attribute)attr;
      pi->name = m_pnames[i]->data();
      if (m_params.size() <= i || !fpi.hasDefaultValue()) {
        pi->value = NULL;
        pi->valueText = "";
      } else {
        // Eval PHP code to get default value, and serialize the result.  Note
        // that access of undefined class constants can cause the eval() to
        // fatal.  Zend lets such fatals propagate, so don't bother catching
        // exceptions here. Be sure the unit gets destroyed, though.
        String code = HPHP::concat(HPHP::concat("<?php return ",
                                                fpi.m_phpCode->data()),
                                   ";");
        Unit *up = HPHP::VM::compile_string(code->data(), code->size());
        boost::scoped_ptr<Unit> unit(up);
        ASSERT(unit != NULL);
        Variant v;
        g_context->invokeFunc((TypedValue*)&v, unit->getMain(),
                              Array::Create());
        pi->value = strdup(f_serialize(v).get()->data());
        pi->valueText = fpi.m_phpCode->data();
        // This is a raw char*, but its lifetime should be at least as long
        // as the the Func*. At this writing, it's a merged anon string
        // owned by ParamInfo.
      }
      pi->type = fpi.m_typeConstraint.exists() ?
        fpi.m_typeConstraint.typeName() : "";
      mi->parameters.push_back(pi);
    }
    // XXX ConstantInfo is abused to store static variable metadata, and although
    // ConstantInfo::callbacks provides a mechanism for registering callbacks, it
    // does not pass enough information through for the callback functions to
    // know the function context whence the callbacks came.  Furthermore, the
    // callback mechanism isn't employed in a fashion that would allow repeated
    // introspection to reflect updated values.  Supporting introspection of
    // static variable values will require different plumbing than currently
    // exists in ConstantInfo.
    for (std::vector<Func::SVInfo>::const_iterator it = m_staticVars.begin();
         it != m_staticVars.end(); ++it) {
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

} } // HPHP::VM
