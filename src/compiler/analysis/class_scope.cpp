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

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/construct.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/statement/interface_statement.h>
#include <util/util.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/file_scope.h>
#include <runtime/base/class_info.h>
#include <compiler/parser/parser.h>
#include <compiler/statement/method_statement.h>
#include <runtime/base/zend/zend_string.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(KindOf kindOf, const std::string &name,
                       const std::string &parent,
                       const vector<string> &bases,
                       const std::string &docComment, StatementPtr stmt)
  : BlockScope(name, docComment, stmt, BlockScope::ClassScope),
    m_kindOf(kindOf), m_parent(parent), m_bases(bases), m_attribute(0),
    m_redeclaring(-1), m_volatile(false), m_needStaticInitializer(false),
    m_derivesFromRedeclaring(FromNormal), m_derivedByDynamic(false),
    m_sep(false), m_needsCppCtor(false), m_needsInit(true) {

  m_dynamic = Option::IsDynamicClass(m_name);

  // dynamic class is also volatile
  m_volatile = Option::AllVolatile || m_dynamic;

  ASSERT(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}


// System
ClassScope::ClassScope(AnalysisResultPtr ar,
                       const std::string &name, const std::string &parent,
                       const std::vector<std::string> &bases,
                       const FunctionScopePtrVec &methods)
  : BlockScope(name, "", StatementPtr(), BlockScope::ClassScope),
    m_kindOf(KindOfObjectClass), m_parent(parent), m_bases(bases),
    m_attribute(0), m_dynamic(false), m_redeclaring(-1), m_volatile(false),
    m_needStaticInitializer(false),
    m_derivesFromRedeclaring(FromNormal), m_derivedByDynamic(false),
    m_sep(false), m_needsCppCtor(false), m_needsInit(true) {
  BOOST_FOREACH(FunctionScopePtr f, methods) {
    if (f->getName() == "__construct") setAttribute(HasConstructor);
    else if (f->getName() == "__destruct") setAttribute(HasDestructor);
    else if (f->getName() == "__get")  setAttribute(HasUnknownPropGetter);
    else if (f->getName() == "__set")  setAttribute(HasUnknownPropSetter);
    else if (f->getName() == "__call") setAttribute(HasUnknownMethodHandler);
    else if (f->getName() == "__callstatic") {
      setAttribute(HasUnknownStaticMethodHandler);
    } else if (f->getName() == "__isset") setAttribute(HasUnknownPropTester);
    else if (f->getName() == "__unset")   setAttribute(HasPropUnsetter);
    else if (f->getName() == "__invoke")  setAttribute(HasInvokeMethod);
    addFunction(ar, f);
  }
  setAttribute(Extension);
  setAttribute(System);

  ASSERT(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

const std::string &ClassScope::getOriginalName() const {
  if (m_stmt) {
    return dynamic_pointer_cast<InterfaceStatement>(m_stmt)->
      getOriginalName();
  }
  return m_originalName;
}

std::string ClassScope::getId() const {
  string name = CodeGenerator::FormatLabel(getOriginalName());
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
}

bool ClassScope::NeedStaticArray(ClassScopePtr cls, FunctionScopePtr func) {
  return cls && cls->getAttribute(NotFinal) && !func->isPrivate();
}

///////////////////////////////////////////////////////////////////////////////

void ClassScope::derivedMagicMethods(ClassScopePtr super) {
  super->setAttribute(NotFinal);
  if (m_attribute & (HasUnknownPropGetter|MayHaveUnknownPropGetter)) {
    super->setAttribute(MayHaveUnknownPropGetter);
  }
  if (m_attribute & (HasUnknownPropSetter|MayHaveUnknownPropSetter)) {
    super->setAttribute(MayHaveUnknownPropSetter);
  }
  if (m_attribute & (HasUnknownPropTester|MayHaveUnknownPropTester)) {
    super->setAttribute(MayHaveUnknownPropTester);
  }
  if (m_attribute & (HasPropUnsetter|MayHavePropUnsetter)) {
    super->setAttribute(MayHavePropUnsetter);
  }
  if (m_attribute & (HasUnknownMethodHandler|MayHaveUnknownMethodHandler)) {
    super->setAttribute(MayHaveUnknownMethodHandler);
  }
  if (m_attribute &
      (HasUnknownStaticMethodHandler|MayHaveUnknownStaticMethodHandler)) {
    super->setAttribute(MayHaveUnknownStaticMethodHandler);
  }
  if (m_attribute & (HasInvokeMethod|MayHaveInvokeMethod)) {
    super->setAttribute(MayHaveInvokeMethod);
  }
  if (m_attribute & (HasArrayAccess|MayHaveArrayAccess)) {
    super->setAttribute(MayHaveArrayAccess);
  }
}

void ClassScope::inheritedMagicMethods(ClassScopePtr super) {
  if (super->m_attribute &
      (HasUnknownPropGetter|InheritsUnknownPropGetter)) {
    setAttribute(InheritsUnknownPropGetter);
  }
  if (super->m_attribute & (HasUnknownPropSetter|InheritsUnknownPropSetter)) {
    setAttribute(InheritsUnknownPropSetter);
  }
  if (super->m_attribute & (HasUnknownPropTester|InheritsUnknownPropTester)) {
    setAttribute(InheritsUnknownPropTester);
  }
  if (super->m_attribute & (HasPropUnsetter|InheritsPropUnsetter)) {
    setAttribute(InheritsPropUnsetter);
  }
  if (super->m_attribute &
      (HasUnknownMethodHandler|InheritsUnknownMethodHandler)) {
    setAttribute(InheritsUnknownMethodHandler);
  }
  if (super->m_attribute &
      (HasUnknownStaticMethodHandler|InheritsUnknownStaticMethodHandler)) {
    setAttribute(InheritsUnknownStaticMethodHandler);
  }
  if (super->m_attribute & (HasInvokeMethod|InheritsInvokeMethod)) {
    setAttribute(InheritsInvokeMethod);
  }
  if (super->m_attribute & (HasArrayAccess|InheritsArrayAccess)) {
    setAttribute(InheritsArrayAccess);
  }
}

bool ClassScope::implementsArrayAccess() {
  return
    getAttribute(MayHaveArrayAccess) |
    getAttribute(HasArrayAccess) |
    getAttribute(InheritsArrayAccess);
}

bool ClassScope::implementsAccessor(int prop) {
  if (m_attribute & prop) return true;
  if (prop & MayHaveUnknownPropGetter) {
    prop |= HasUnknownPropGetter | InheritsUnknownPropGetter;
  }
  if (prop & MayHaveUnknownPropSetter) {
    prop |= HasUnknownPropSetter | InheritsUnknownPropSetter;
  }
  if (prop & MayHaveUnknownPropTester) {
    prop |= HasUnknownPropTester | InheritsUnknownPropTester;
  }
  if (prop & MayHavePropUnsetter) {
    prop |= HasPropUnsetter | InheritsPropUnsetter;
  }
  return m_attribute & prop;
}

void ClassScope::checkDerivation(AnalysisResultPtr ar, hphp_string_set &seen) {
  seen.insert(m_name);

  hphp_string_set bases;
  for (int i = m_bases.size() - 1; i >= 0; i--) {
    const string &base = m_bases[i];

    if (seen.find(base) != seen.end() || bases.find(base) != bases.end()) {
      Compiler::Error(Compiler::InvalidDerivation, m_stmt, base);
      if (i == 0 && !m_parent.empty()) {
        ASSERT(base == m_parent);
        m_parent.clear();
      }
      m_bases.erase(m_bases.begin() + i);
      continue;
    }
    bases.insert(base);

    ClassScopePtrVec parents = ar->findClasses(base);
    for (unsigned int j = 0; j < parents.size(); j++) {
      parents[j]->checkDerivation(ar, seen);
    }
  }

  seen.erase(m_name);
}

void ClassScope::collectMethods(AnalysisResultPtr ar,
                                StringToFunctionScopePtrMap &funcs,
                                bool collectPrivate /* = true */,
                                bool forInvoke /* = false */) {
  // add all functions this class has
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!collectPrivate && iter->second.back()->isPrivate()) continue;

    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcs.find(iter->first);
    if (iterFuncs == funcs.end()) {
      funcs[iter->first] = iter->second.back();
    } else {
      iterFuncs->second->setVirtual();
      iter->second.back()->setVirtual();
      iter->second.back()->setHasOverride();
    }
  }

  BOOST_FOREACH(string miss, m_missingMethods) {
    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcs.find(miss);
    if (iterFuncs != funcs.end()) {
      iterFuncs->second->setVirtual();
    }
  }

  // walk up
  for (int i = m_bases.size() - 1; i >= 0; i--) {
    const string &base = m_bases[i];
    if (forInvoke && base != m_parent) {
      continue;
    }
    ClassScopePtr super = ar->findClass(base);
    if (super) {
      if (derivedByDynamic()) {
        super->m_derivedByDynamic = true;
      }
      if (super->isRedeclaring()) {
        if (base == m_parent) {
          if (forInvoke) continue;
          const ClassScopePtrVec &classes = ar->findRedeclaredClasses(m_parent);
          StringToFunctionScopePtrMap pristine(funcs);
          BOOST_FOREACH(ClassScopePtr cls, classes) {
            cls->m_derivedByDynamic = true;
            StringToFunctionScopePtrMap cur(pristine);
            derivedMagicMethods(cls);
            cls->collectMethods(ar, cur, false, forInvoke);
            inheritedMagicMethods(cls);
            funcs.insert(cur.begin(), cur.end());
          }
          m_derivesFromRedeclaring = DirectFromRedeclared;
          getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars);
          getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
        } else if (isInterface()) {
          m_derivesFromRedeclaring = DirectFromRedeclared;
        }
        setVolatile();
      } else {
        derivedMagicMethods(super);
        super->collectMethods(ar, funcs, false, forInvoke);
        inheritedMagicMethods(super);
        if (super->derivesFromRedeclaring()) {
          if (base == m_parent) {
            m_derivesFromRedeclaring = IndirectFromRedeclared;
            getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars);
          } else if (isInterface()) {
            m_derivesFromRedeclaring = IndirectFromRedeclared;
          }
          setVolatile();
        } else if (super->isVolatile()) {
          setVolatile();
        }
      }
    } else {
      Compiler::Error(Compiler::UnknownBaseClass, m_stmt, base);
      if (base == m_parent) {
        ar->declareUnknownClass(m_parent);
        m_derivesFromRedeclaring = DirectFromRedeclared;
        getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
        getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars);
        setVolatile();
      } else {
        if (isInterface()) {
          m_derivesFromRedeclaring = DirectFromRedeclared;
        }
        m_bases.erase(m_bases.begin() + i);
      }
    }
  }
}

bool ClassScope::needsInvokeParent(AnalysisResultConstPtr ar,
                                   bool considerSelf /* = true */) {
  // check all functions this class has
  if (considerSelf) {
    for (StringToFunctionScopePtrVecMap::const_iterator iter =
           m_functions.begin(); iter != m_functions.end(); ++iter) {
      if (iter->second.back()->isPrivate()) return true;
    }
  }

  // walk up
  if (!m_parent.empty()) {
    ClassScopePtr super = ar->findClass(m_parent);
    return !super || super->isRedeclaring() || super->needsInvokeParent(ar);
  }
  return false;
}

bool ClassScope::derivesDirectlyFrom(const std::string &base) const {
  BOOST_FOREACH(std::string base_i, m_bases) {
    if (base_i == base) return true;
  }
  return false;
}

bool ClassScope::derivesFrom(AnalysisResultConstPtr ar,
                             const std::string &base,
                             bool strict, bool def) const {

  if (derivesDirectlyFrom(base)) return true;

  BOOST_FOREACH(std::string base_i, m_bases) {
    ClassScopePtr cl = ar->findClass(base_i);
    if (cl) {
      if (strict && cl->isRedeclaring()) {
        if (def) return true;
        continue;
      }
      if (cl->derivesFrom(ar, base, strict, def)) return true;
    }
  }
  return false;
}

ClassScopePtr ClassScope::FindCommonParent(AnalysisResultConstPtr ar,
                                           const std::string &cn1,
                                           const std::string &cn2) {

  ClassScopePtr cls1 = ar->findClass(cn1);
  ClassScopePtr cls2 = ar->findClass(cn2);

  if (!cls1 || !cls2) return ClassScopePtr();
  if (cls1->getName() == cls2->getName())      return cls1;
  if (cls1->derivesFrom(ar, cn2, true, false)) return cls2;
  if (cls2->derivesFrom(ar, cn1, true, false)) return cls1;

  // walk up the class hierarchy.
  BOOST_FOREACH(const std::string &base1, cls1->m_bases) {
    BOOST_FOREACH(const std::string &base2, cls2->m_bases) {
      ClassScopePtr parent = FindCommonParent(ar, base1, base2);
      if (parent) return parent;
    }
  }

  return ClassScopePtr();
}

void ClassScope::setVolatile() {
  if (!m_volatile) {
    m_volatile = true;
    Lock lock(s_depsMutex);
    const BlockScopeRawPtrFlagsVec &orderedUsers = getOrderedUsers();
    for (BlockScopeRawPtrFlagsVec::const_iterator it = orderedUsers.begin(),
           end = orderedUsers.end(); it != end; ++it) {
      BlockScopeRawPtrFlagsVec::value_type pf = *it;
      if (pf->second & UseKindParentRef) {
        BlockScopeRawPtr scope = pf->first;
        if (scope->is(BlockScope::ClassScope)) {
          ((HPHP::ClassScope*)scope.get())->setVolatile();
        }
      }
    }
  }
}

FunctionScopePtr ClassScope::findFunction(AnalysisResultConstPtr ar,
                                          const std::string &name,
                                          bool recursive,
                                          bool exclIntfBase /*= false */) {
  ASSERT(Util::toLower(name) == name);
  StringToFunctionScopePtrVecMap::const_iterator iter;
  iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    ASSERT(iter->second.back());
    return iter->second.back();
  }

  // walk up
  if (recursive) {
    int s = m_bases.size();
    for (int i = 0; i < s; i++) {
      const string &base = m_bases[i];
      ClassScopePtr super = ar->findClass(base);
      if (!super) continue;
      if (exclIntfBase && super->isInterface()) break;
      if (super->isRedeclaring()) {
        if (base == m_parent) {
          m_derivesFromRedeclaring = DirectFromRedeclared;
          break;
        }
        continue;
      }
      FunctionScopePtr func =
        super->findFunction(ar, name, true, exclIntfBase);
      if (func) return func;
    }
  }
  if (derivesFromRedeclaring() == DirectFromRedeclared) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

FunctionScopePtr ClassScope::findConstructor(AnalysisResultConstPtr ar,
                                             bool recursive) {
  StringToFunctionScopePtrVecMap::const_iterator iter;
  string name;
  if (classNameCtor()) {
    name = getName();
  }
  else {
    name = "__construct";
  }
  iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    ASSERT(iter->second.back());
    return iter->second.back();
  }

  // walk up
  if (recursive && derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr super = ar->findClass(m_parent);
    if (super) {
      FunctionScopePtr func =
        super->findConstructor(ar, true);
      if (func) return func;
    }
  }
  if (derivesFromRedeclaring() == DirectFromRedeclared) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

void ClassScope::setStaticDynamic(AnalysisResultConstPtr ar) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      if (fs->isStatic()) fs->setDynamic();
    }
  }
  if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == DirectFromRedeclared) {
      const ClassScopePtrVec &parents = ar->findRedeclaredClasses(m_parent);
      BOOST_FOREACH(ClassScopePtr cl, parents) {
        cl->setStaticDynamic(ar);
      }
    } else {
      ClassScopePtr parent = ar->findClass(m_parent);
      if (parent) {
        parent->setStaticDynamic(ar);
      }
    }
  }
}

void ClassScope::setDynamic(AnalysisResultConstPtr ar,
                            const std::string &name) {
  StringToFunctionScopePtrVecMap::const_iterator iter =
    m_functions.find(name);
  if (iter != m_functions.end()) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      fs->setDynamic();
    }
  } else if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == DirectFromRedeclared) {
      const ClassScopePtrVec &parents = ar->findRedeclaredClasses(m_parent);
      BOOST_FOREACH(ClassScopePtr cl, parents) {
        cl->setDynamic(ar, name);
      }
    } else {
      ClassScopePtr parent = ar->findClass(m_parent);
      if (parent) {
        parent->setDynamic(ar, name);
      }
    }
  }
}

void ClassScope::setSystem() {
  setAttribute(ClassScope::System);
  m_volatile = m_dynamic = false;
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    iter->second[0]->setSystem();
  }
}

bool ClassScope::needLazyStaticInitializer() {
  return getVariables()->getAttribute(VariableTable::ContainsDynamicStatic) ||
    getConstants()->hasDynamic();
}

void ClassScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar) {
  // header
  int attribute = ClassInfo::IsNothing;
  if (!isUserClass()) attribute |= ClassInfo::IsSystem;
  if (isRedeclaring()) attribute |= ClassInfo::IsRedeclared;
  if (isVolatile()) attribute |= ClassInfo::IsVolatile;
  if (isInterface()) attribute |= ClassInfo::IsInterface|ClassInfo::IsAbstract;
  if (m_kindOf == KindOfAbstractClass) attribute |= ClassInfo::IsAbstract;
  if (m_kindOf == KindOfFinalClass) attribute |= ClassInfo::IsFinal;
  if (needLazyStaticInitializer()) attribute |= ClassInfo::IsLazyInit;

  attribute |= m_attributeClassInfo;
  if (!m_docComment.empty() && Option::GenerateDocComments) {
    attribute |= ClassInfo::HasDocComment;
  } else {
    attribute &= ~ClassInfo::HasDocComment;
  }

  string parent;
  if (!m_parent.empty()) {
    ClassScopePtr parCls = ar->findClass(m_parent);
    if (parCls) {
      parent = parCls->getOriginalName();
    } else {
      parent = m_parent;
    }
  }
  cg_printf("(const char *)0x%04X, \"%s\", \"%s\", \"%s\", (const char *)%d, "
            "(const char *)%d,\n", attribute,
            CodeGenerator::EscapeLabel(getOriginalName()).c_str(),
            CodeGenerator::EscapeLabel(parent).c_str(),
            m_stmt ? m_stmt->getLocation()->file : "",
            m_stmt ? m_stmt->getLocation()->line0 : 0,
            m_stmt ? m_stmt->getLocation()->line1 : 0);

  if (!m_docComment.empty() && Option::GenerateDocComments) {
    char *dc = string_cplus_escape(m_docComment.c_str(), m_docComment.size());
    cg_printf("\"%s\",\n", dc);
    free(dc);
  }

  // parent interfaces
  for (unsigned int i = (m_parent.empty() ? 0 : 1); i < m_bases.size(); i++) {
    string base;
    ClassScopePtr baseCls = ar->findClass(m_bases[i]);
    if (baseCls) {
      base = baseCls->getOriginalName();
    } else {
      base = m_bases[i];
    }
    cg_printf("\"%s\", ", CodeGenerator::EscapeLabel(base).c_str());
  }
  cg_printf("NULL,\n");

  // methods
  for (unsigned int i = 0; i < m_functionsVec.size(); i++) {
    m_functionsVec[i]->outputCPPClassMap(cg, ar);
  }
  cg_printf("NULL,\n");

  // properties && constants
  m_variables->outputCPPClassMap(cg, ar);
  m_constants->outputCPPClassMap(cg, ar);
}

bool ClassScope::hasConst(const string &name) const {
  const Symbol *sym = m_constants->getSymbol(name);
  ASSERT(!sym || sym->isPresent());
  return sym;
}

Symbol *ClassScope::findProperty(ClassScopePtr &cls,
                                 const string &name,
                                 AnalysisResultConstPtr ar) {
  return getVariables()->findProperty(cls, name, ar);
}

TypePtr ClassScope::checkProperty(Symbol *sym, TypePtr type,
                                  bool coerce, AnalysisResultConstPtr ar) {
  return getVariables()->checkProperty(sym, type, coerce, ar);
}

TypePtr ClassScope::checkConst(const std::string &name, TypePtr type,
                               bool coerce, AnalysisResultConstPtr ar,
                               ConstructPtr construct,
                               const std::vector<std::string> &bases,
                               BlockScope *&defScope) {
  defScope = NULL;
  return getConstants()->check(name, type, coerce,
                                    ar, construct, m_bases, defScope);
}

void ClassScope::getAllParents(AnalysisResultConstPtr ar,
                               std::vector<std::string> &names) {
  if (m_stmt) {
    if (isInterface()) {
      boost::dynamic_pointer_cast<InterfaceStatement>
        (m_stmt)->getAllParents(ar, names);
    } else {
      boost::dynamic_pointer_cast<ClassStatement>
        (m_stmt)->getAllParents(ar, names);
    }
  } else {
    for (unsigned i = 0; i < m_bases.size(); i++) {
      const string &base = m_bases[i];
      names.push_back(base);
      if (ClassScopePtr cls = ar->findClass(base)) {
        if (!cls->isRedeclaring()) {
          cls->getAllParents(ar, names);
        }
      }
    }
  }
}

ClassScopePtr ClassScope::getParentScope(AnalysisResultConstPtr ar) const {
  if (m_parent.empty()) return ClassScopePtr();
  return ar->findClass(m_parent);
}

void ClassScope::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  map<string, int> propMap;
  set<string> names;
  m_variables->getNames(names);
  BOOST_FOREACH(string name, names) {
    int pm = 0;
    if (m_variables->isPublic(name)) pm |= ClassScope::Public;
    else if (m_variables->isPrivate(name)) pm |= ClassScope::Private;
    else if (m_variables->isProtected(name)) pm |= ClassScope::Protected;
    if (m_variables->isStatic(name)) pm |= ClassScope::Static;
    propMap[name] = pm;
  }
  names.clear();
  vector<string> cnames;
  m_constants->getSymbols(cnames);

  // What's a mod again?
  ms.add("attributes", m_attribute)
    .add("kind", m_kindOf)
    .add("parent", m_parent)
    .add("bases", m_bases)
    .add("properties", propMap)
    .add("functions", m_functions);

  ms.add("consts");

  JSON::MapStream cs(out);
  BOOST_FOREACH(string cname, cnames) {
    TypePtr type =  m_constants->getType(cname);
    if (!type) {
      cs.add(cname, -1);
    } else if (type->isSpecificObject()) {
      cs.add(cname, type->getName());
    } else {
      cs.add(cname, type->getKindOf());
    }
  }
  cs.done();
  ms.done();
}

void ClassScope::outputCPPDynamicClassDecl(CodeGenerator &cg) {
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg_printf("ObjectData *%s%s(%s) NEVER_INLINE;\n",
            Option::CreateObjectOnlyPrefix, clsName,
            isRedeclaring() ? "ObjectData *root = NULL" : "");
}

void ClassScope::outputCPPDynamicClassCreateDecl(CodeGenerator &cg) {
  cg_printf("Object create_object_only("
            "CStrRef s, ObjectData *root);\n");
  cg_printf("ObjectData *create_object_only_no_init("
            "CStrRef s, ObjectData *root);\n");
}

void ClassScope::outputCPPDynamicClassImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg_indentBegin("ObjectData *%s%s(%s) {\n",
                 Option::CreateObjectOnlyPrefix, clsName,
                 isRedeclaring() ? "ObjectData *root /* = NULL */" : "");
  cg_printf("return NEWOBJ(%s%s)(%s);\n",
            Option::ClassPrefix, clsName,
            isRedeclaring() ? "root" : "");
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPClassJumpTable
(CodeGenerator &cg,
 const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes, const char* macro,
 bool useString /* = false */) {
  cg.printDeclareGlobals();
  for (JumpTable jt(cg, classes, true, false, useString); jt.ready();
       jt.next()) {
    const char *clsName = jt.key();
    StringToClassScopePtrVecMap::const_iterator iterClasses =
      classScopes.find(Util::toLower(clsName));
    bool redeclaring = iterClasses->second[0]->isRedeclaring();
    if (iterClasses != classScopes.end()) {
      const char *suffix = "";
      if (redeclaring) suffix = "_REDECLARED";
      else if (iterClasses->second[0]->isVolatile()) suffix = "_VOLATILE";
      cg_printf("%s%s", macro, suffix);
      cg_printf("(0x%016llXLL, %s);\n",
                hash_string_i(clsName),
                redeclaring ? iterClasses->second[0]->getName().c_str() :
                CodeGenerator::FormatLabel(clsName).c_str());
    }
  }
}

void ClassScope::outputCPPHashTableClassVarInit
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "struct hashNodeCTD {\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  int64 ptv1;\n"
    "  ObjectData*(* const ptr2)();\n"
    "};\n";

  const char text2[] =
    "#define GET_CS_OFFSET(n) "
    "((offsetof(GlobalVariables, %s ## n) -"
    "  offsetof(GlobalVariables, tgv_RedeclaredObjectStaticCallbacksConstPtr))/"
    "sizeof(RedeclaredObjectStaticCallbacksConst*))\n"
    "inline ALWAYS_INLINE "
    "const ObjectStaticCallbacks *getCallbacks(\n"
    "  const hashNodeCTD *p, CStrRef s, GlobalVariables *g) {\n"
    "  int64 off = p->ptv1;\n"
    "  if (LIKELY(!(off & 1))) return ((const ObjectStaticCallbacks *)off);\n"
    "  checkClassExists(s, g);\n"
    "  if (LIKELY(p->ptr2!=0)) /* volatile class */ return "
    "((const ObjectStaticCallbacks *)(off-1));\n"
    "  /* redeclared class */\n"
    "  return &g->tgv_RedeclaredObjectStaticCallbacksConstPtr[off>>1]->oscb;\n"
    "}\n";

  const char text3[] =
    "\n"
    "static const hashNodeCTD *\n"
    "findCTD(CStrRef name) {\n"
    "  int64 hash = name->hash();\n"
    "  int o = ctdMapTable[hash & %d];\n"
    "  if (UNLIKELY(o < 0)) return NULL;\n"
    "  const hashNodeCTD *p = &ctdBuckets[o];\n"
    "  int64 h = p->hash & (uint64(-1)>>1);\n"
    "  do {\n"
    "    if (h == hash && "
    "(LIKELY(p->name==name.data())||"
    "LIKELY(!strcasecmp(p->name, name.data())))) return p;\n"
    "    h = (++p)->hash;\n"
    "  } while (h >= 0);\n"
    "  return NULL;\n"
    "}\n";

  JumpTable jt(cg, classes, true, true, true, true);
  cg_printf(text1);
  if (!system) {
    cg_printf(text2, Option::ClassStaticsCallbackPrefix);
  }
  cg_printf("static const hashNodeCTD ctdBuckets[] = {\n");

  int64 min64 = -1-int64(uint64(-1)>>1);
  vector<int> offsets;
  int prev = -1;
  for (int n = 0; jt.ready(); ++n, jt.next()) {
    int cur = jt.current();
    bool changed = false;
    if (prev != cur) {
      changed = true;
      while (++prev != cur) {
        offsets.push_back(-1);
      }
      offsets.push_back(n);
    }
    const char *clsName = jt.key();
    StringToClassScopePtrVecMap::const_iterator iterClasses =
      classScopes.find(Util::toLower(clsName));
    cg_printf("  {0x%016llXLL,\"%s\",",
              hash_string_i(clsName) + (changed ? min64 : 0),
              CodeGenerator::EscapeLabel(clsName).c_str());
    ClassScopeRawPtr cls = iterClasses->second[0];
    if (cls->isRedeclaring()) {
      ASSERT(!system);
      cg_printf("GET_CS_OFFSET(%s)*2+1,0",
                CodeGenerator::FormatLabel(cls->getName()).c_str());
    } else {
      string clsFmt = CodeGenerator::FormatLabel(clsName);
      cg_printf("%s(int64)&%s%s,&%s%s",
                cls->isVolatile() ? "1+" : "",
                Option::ClassWrapperFunctionPrefix,
                clsFmt.c_str(),
                Option::CreateObjectOnlyPrefix,
                clsFmt.c_str());
    }
    cg_printf("},\n");
  }

  cg_printf("  { -1,0,0,0 } };\n");
  cg_indentBegin("static const int ctdMapTable[] = {\n");
  for (int i = 0, e = jt.size(), s = offsets.size(); i < e; i++) {
    cg_printf("%d,", i < s ? offsets[i] : -1);
    if ((i & 7) == 7) cg_printf("\n");
  }
  cg_printf("\n");
  cg_indentEnd("};\n");

  cg_printf(text3, jt.size() - 1);
}

void ClassScope::outputCPPClassVarInitImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool useHashTable = (classes.size() > 0);
  if (useHashTable) {
    outputCPPHashTableClassVarInit(cg, classScopes, classes);
  }
  cg_indentBegin("Variant get%s_class_var_init(CStrRef s, "
                 "const char *var) {\n",
                 system ? "_builtin" : "");
  bool withEval = !system && Option::EnableEval == Option::FullEval;
  if (withEval) {
    // See if there's an eval'd version
    cg_indentBegin("{\n");
    cg_printf("Variant r;\n");
    cg_printf("if (eval_get_class_var_init_hook(r, s, var)) "
              "return r;\n");
    cg_indentEnd("}\n");
  }
  if (!useHashTable) {
    outputCPPClassJumpTable(cg, classScopes, classes,
                            "HASH_GET_CLASS_VAR_INIT");
  } else {
    if (system) {
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (p) {\n"
                "  return "
                "((const ObjectStaticCallbacks *)p->ptv1)->os_getInit(var);\n"
                "}\n"
                "return throw_missing_class(s);\n");
    } else {
      cg.printDeclareGlobals();
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (!p) return get_builtin_class_var_init(s, var);\n"
                "return getCallbacks(p,s,g)->os_getInit(var);\n");
    }
  }
  if (!useHashTable) {
    if (system) {
      cg_printf("return throw_missing_class(s);\n");
    } else {
      cg_printf("return get_builtin_class_var_init(s, var);\n");
    }
  }
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPDynamicClassCreateImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool withEval = !system && Option::EnableEval == Option::FullEval;
  bool useHashTable = (classes.size() > 0);

  // output create_object_only_no_init()
  cg_indentBegin("ObjectData *create%s_object_only_no_init(CStrRef s, "
                 "ObjectData* root /* = NULL*/) {\n",
                 system ?  "_builtin" : "");
  if (withEval) {
    // See if there's an eval'd version
    cg_indentBegin("{\n");
    cg_printf("if (ObjectData * r = eval_create_object_only_hook(s, root)) "
              "return r;\n");
    cg_indentEnd("}\n");
  }
  if (!useHashTable) {
    outputCPPClassJumpTable(cg, classScopes, classes,
                            "HASH_CREATE_OBJECT_ONLY");
    if (system) {
      cg_printf("throw_missing_class(s);\n");
      cg_printf("return 0;\n");
    } else {
      cg_printf("return create_builtin_object_only_no_init(s, root);\n");
    }
  } else {
    if (system) {
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (p) {\n"
                "  return p->ptr2();\n"
                "}\n"
                "throw_missing_class(s);\n"
                "return 0;\n");
    } else {
      cg.printDeclareGlobals();
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (!p) return create_builtin_object_only_no_init(s, root);\n"
                "return getCallbacks(p,s,g)->createOnlyNoInit(root);\n");
    }
  }
  cg_indentEnd("}\n");
  // output create_object_only()
  cg_indentBegin("Object create%s_object_only(CStrRef s, "
                 "ObjectData* root /* = NULL*/) {\n",
                 system ?  "_builtin" : "");
  cg_printf("Object r(create%s_object_only_no_init(s, root));\n",
            system ? "_builtin" : "");
  cg_printf("r->init();\n");
  cg_printf("return r;\n");
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPGetCallInfoStaticMethodImpl(
  CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
  const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool useHashTable = (classes.size() > 0);
  cg_indentBegin("bool get_call_info_static_method%s(MethodCallPackage &mcp)"
      " {\n", system ? "_builtin" : "");

  cg_printf("StringData *s ATTRIBUTE_UNUSED (mcp.rootCls);\n");

  if (!system && Option::EnableEval == Option::FullEval) {
    cg_printf("bool foundClass = false;\n");
    cg_printf("if (eval_get_call_info_static_method_hook(mcp, foundClass)) "
              "return true;\n");
    cg_indentBegin("else if (foundClass) {\n");
    cg_printf("mcp.fail();\n");
    cg_printf("return false;\n");
    cg_indentEnd("}\n");
  }
  if (useHashTable) {
    if (system) {
      cg_printf("const hashNodeCTD *p = findCTD(StrNR(s));\n"
                "const ObjectStaticCallbacks *osc=p?"
                "(const ObjectStaticCallbacks *)p->ptv1:0;\n"
                "return ObjectStaticCallbacks::GetCallInfo(osc,mcp,-1);\n");
    } else {
      cg.printDeclareGlobals();
      cg_printf("const hashNodeCTD *p = findCTD(StrNR(s));\n"
                "if (!p) return get_call_info_static_method_builtin(mcp);\n"
                "return getCallbacks(p,s,g)->os_get_call_info(mcp, -1);\n");
    }
  } else {
    if (system) {
      cg_printf("return ObjectStaticCallbacks::GetCallInfo(0,mcp,-1);\n");
    } else {
      cg_printf("return get_call_info_static_method_builtin(mcp);\n");
    }
  }

  cg_indentEnd("}\n");
}


void ClassScope::outputCPPGetStaticPropertyImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool useHashTable = (classes.size() > 0);

  cg_indentBegin("const ObjectStaticCallbacks * "
                 "get%s_object_static_callbacks(CStrRef s) {\n",
                 system ? "_builtin" : "");
  if (!useHashTable) {
    outputCPPClassJumpTable(cg, classScopes, classes,
                            "HASH_GET_OBJECT_STATIC_CALLBACKS");
  } else {
    if (system) {
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (p) {\n"
                "  return "
                "((const ObjectStaticCallbacks *)p->ptv1);\n"
                "}\n"
                "return NULL;\n");
    } else {
      cg.printDeclareGlobals();
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (!p) return get_builtin_object_static_callbacks(s);\n"
                "return getCallbacks(p,s,g);\n");
    }
  }
  if (!useHashTable) {
    if (system) {
      cg_printf("return NULL;\n");
    } else {
      cg_printf("return get_builtin_object_static_callbacks(s);\n");
    }
  }
  cg_indentEnd("}\n");

  cg_indentBegin("Variant get%s_static_property(CStrRef s, "
                 "const char *prop) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg_indentBegin("{\n");
    cg_printf("Variant r;\n");
    cg_printf("if (eval_get_static_property_hook(r, s, prop)) "
              "return r;\n");
    cg_indentEnd("}\n");
  }

  cg.indentBegin("{\n");
  cg.printf("const ObjectStaticCallbacks * cwo = "
            "get%s_object_static_callbacks(s);\n",
            system ? "_builtin" : "");
  cg.printf("if (cwo) return cwo->os_get(prop);\n");
  cg.indentEnd("}\n");

  if (!system) {
    cg_printf("return get_builtin_static_property(s, prop);\n");
  } else {
    cg_printf("return null;\n");
  }
  cg_indentEnd("}\n");

  cg_indentBegin("Variant *get%s_static_property_lv(CStrRef s, "
                 "const char *prop) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg_indentBegin("{\n");
    cg_printf("Variant *r;\n");
    cg_printf("if (eval_get_static_property_lv_hook(r, s, prop)) "
              "return r;\n");
    cg_indentEnd("}\n");
  }

  cg.indentBegin("{\n");
  cg.printf("const ObjectStaticCallbacks * cwo = "
            "get%s_object_static_callbacks(s);\n",
            system ? "_builtin" : "");
  cg.printf("if (cwo) return &cwo->os_lval(prop);\n");
  cg.indentEnd("}\n");

  if (!system) {
    cg_printf("return get_builtin_static_property_lv(s, prop);\n");
  } else {
    cg_printf("return NULL;\n");
  }
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPGetClassConstantImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg_indentBegin("Variant get%s_class_constant(CStrRef s, "
                 "const char *constant, bool fatal /* = true */) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg_indentBegin("{\n");
    cg_printf("Variant r;\n");
    cg_printf("if (eval_get_class_constant_hook(r, s, constant)) "
              "return r;\n");
    cg_indentEnd("}\n");
  }

  cg.indentBegin("{\n");
  cg.printf("const ObjectStaticCallbacks * cwo = "
            "get%s_object_static_callbacks(s);\n",
            system ? "_builtin" : "");
  cg.printf("if (cwo) return cwo->os_constant(constant);\n");
  cg.indentEnd("}\n");

  if (!system) {
    cg_printf("return get_builtin_class_constant(s, constant, fatal);\n");
  } else {
    cg_indentBegin("if (fatal) {\n");
    cg_printf("raise_error(\"Couldn't find constant %%s::%%s\", s.data(), "
              "constant);\n");
    cg_indentEnd("");
    cg_indentBegin("} else {\n");
    cg_printf("raise_warning(\"Couldn't find constant %%s::%%s\", s.data(), "
              "constant);\n");
    cg_indentEnd("}\n");
    cg_printf("return null;\n");
  }
  cg_indentEnd("}\n");
}

static int propTableSize(int entries) {
  int size = Util::roundUpToPowerOfTwo(entries * 2);
  return size < 8 ? 8 : size;
}

static bool buildClassPropTableMap(
  CodeGenerator &cg,
  const StringToClassScopePtrVecMap &classScopes,
  ClassScope::ClassPropTableMap &tables) {
  typedef ClassScope::IndexedSym IndexedSym;
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  int totalEntries = 0;
  for (StringToClassScopePtrVecMap::const_iterator iter = classScopes.begin();
       iter != classScopes.end(); ++iter) {
    const ClassScopePtrVec &classes = iter->second;
    if (system) ASSERT(classes.size() == 1);
    for (unsigned int i = 0; i < classes.size(); i++) {
      ClassScopePtr cls = classes[i];
      const std::vector<Symbol*> &symbolVec =
        cls->getVariables()->getSymbols();
      ClassScope::Derivation dynamicObject = cls->derivesFromRedeclaring();
      vector<const Symbol *> entries;
      for (unsigned j = 0; j < symbolVec.size(); j++) {
        const Symbol *sym = symbolVec[j];
        if (sym->isStatic() || (dynamicObject && !sym->isPrivate())) continue;
        entries.push_back(sym);
      }
      if (!entries.size()) continue;
      ClassScope::ClassPropTableInfo &info = tables[cls->getId()];
      info.cls = cls;
      int tableSize = propTableSize(entries.size());

      unsigned p = 0;
      for (unsigned j = 0; j < entries.size(); j++) {
        const Symbol *sym = entries[j];
        int hash = hash_string_i(sym->getName().c_str(),
                                 sym->getName().size()) & (tableSize - 1);
        int pix = -1;
        if (sym->isPrivate()) {
          pix = p++;
        }
        info.syms[hash].push_back(IndexedSym(j, pix, sym));
      }
      info.actualIndex.resize(entries.size() + 1, -1);
      info.privateIndex.resize(p);
      int newIndex = 0;
      for (map<int, vector<IndexedSym> >::iterator it = info.syms.begin(),
             end = info.syms.end(); it != end; ++it) {
        const vector<IndexedSym> &v = it->second;
        for (int k = 0, s = v.size(); k < s; k++, newIndex++) {
          const IndexedSym &is = v[k];
          info.actualIndex[is.origIndex] = newIndex;
          if (is.privIndex >= 0) info.privateIndex[is.privIndex] = newIndex;
        }
      }
      totalEntries += entries.size();
    }
  }
  return totalEntries > 0;
}

bool ClassScope::checkHasPropTable() {
  VariableTablePtr variables = getVariables();
  bool nsp = variables->hasNonStaticPrivate();
  if (nsp || derivesFromRedeclaring()) return nsp;

  const std::vector<Symbol*> &symbolVec = variables->getSymbols();

  for (unsigned int j = 0; j < symbolVec.size(); j++) {
    const Symbol *sym = symbolVec[j];
    if (!sym->isStatic()) return true;
  }

  return false;
}

ClassScopePtr ClassScope::getNextParentWithProp(AnalysisResultPtr ar) {

  if (derivesFromRedeclaring() == DirectFromRedeclared) return ClassScopePtr();
  ClassScopePtr parentCls = getParentScope(ar);
  while (parentCls && !parentCls->checkHasPropTable()) {
    parentCls = parentCls->getParentScope(ar);
  }
  return parentCls;
}

void ClassScope::outputCPPGetClassPropTableImpl(
  CodeGenerator &cg, AnalysisResultPtr ar,
  const StringToClassScopePtrVecMap &classScopes,
  bool extension /* = false */) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  ClassPropTableMap tables;
  if (!buildClassPropTableMap(cg, classScopes, tables)) return;

  cg.printSection("Class tables");
  cg_indentBegin("static const ClassPropTableEntry %stable_entries[] = {\n",
                 Option::ClassPropTablePrefix);

  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    assert(info.syms.size());
    ClassScopePtr cls = info.cls;
    for (map<int, vector<IndexedSym> >::const_iterator it = info.syms.begin(),
           end = info.syms.end(); it != end; ++it) {
      const vector<IndexedSym> &v = it->second;
      for (int k = 0, s = v.size(); k < s; k++) {
        const Symbol *sym = v[k].sym;
        int cur = info.actualIndex[v[k].origIndex];
        int next = info.actualIndex[v[k].origIndex + 1];
        if (next < 0) next = cur;

        string prop(sym->getName());
        int flags = 0;
        if (sym->isPublic())    flags |= ClassInfo::IsPublic;
        if (sym->isProtected()) flags |= ClassInfo::IsProtected;
        if (sym->isPrivate()) {
          ASSERT(!sym->isOverride());
          flags |= ClassInfo::IsPrivate;
          prop = '\0' + cls->getOriginalName() + '\0' + prop;
        }
        ASSERT(!sym->isStatic());
        if (sym->isOverride()) {
          ASSERT(!system);
          flags |= ClassInfo::IsOverride;
        }
        if (k == s - 1) flags |= ClassInfo::IsRedeclared;
        cg_printf("{0x%016llXLL,%d,%d,%d,%d,",
                  hash_string_i(sym->getName().c_str(), sym->getName().size()),
                  next - cur, prop.size() - sym->getName().size(),
                  flags, sym->getFinalType()->getDataType());
        cg_printf("GET_PROPERTY_OFFSET(%s%s, %s%s),",
                  Option::ClassPrefix, cls->getId().c_str(),
                  Option::PropertyPrefix, sym->getName().c_str());
        cg_printf("&");
        cg_printString(prop, ar, cls);
        cg_printf(" },\n");
      }
    }
    cg_printf("\n");
  }
  cg_indentEnd("};\n");
  int curIndex = 0;
  int privateEntries = 0;
  cg_indentBegin("static const ClassPropTableEntry *%sprivate_entries[] = {\n",
                 Option::ClassPropTablePrefix);
  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    unsigned psize = info.privateIndex.size();
    for (unsigned i = 0; i < psize; i++) {
      cg_printf("%stable_entries+%d,\n",
                Option::ClassPropTablePrefix,
                curIndex + info.privateIndex[i]);
    }
    curIndex += info.actualIndex.size() - 1;

    if (psize) {
      privateEntries += psize + 1;
      cg_printf("0,\n");
    }
  }
  if (!privateEntries) {
    cg_printf("0\n");
    privateEntries++;
  }
  cg_indentEnd("};\n");

  cg_indentBegin("static const int %shash_entries[] = {\n",
                 Option::ClassPropTablePrefix);

  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    assert(info.syms.size());
    int n = 0, m = 0;
    for (map<int, vector<IndexedSym> >::const_iterator it = info.syms.begin(),
           end = info.syms.end(); it != end; ++it) {
      while (n < it->first) {
        cg_printf("-1,");
        n++;
      }
      cg_printf("%d,", m);
      n++;
      m += it->second.size();
    }
    int sz = propTableSize(info.actualIndex.size() - 1);
    assert(n <= sz);
    while (n < sz) {
      cg_printf("-1,");
      n++;
    }
    cg_printf("\n");
  }

  cg_indentEnd("};\n");

  curIndex = 0;
  int curOffset = 0;
  int privateOffset = 0;
  int hashOffset = 0;
  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    ASSERT(info.syms.size() > 0);

    int pCount = info.privateIndex.size();
    cg_indentBegin("const ClassPropTable %s%s::%sprop_table = {\n",
                   Option::ClassPrefix, iter->first.c_str(),
                   Option::ObjectStaticPrefix);
    int tableSize = propTableSize(info.actualIndex.size() - 1);
    cg_printf("%d,%d,", tableSize - 1, info.actualIndex[0]);
    cg_printf("%shash_entries+%d,\n",
              Option::ClassPropTablePrefix, hashOffset);
    hashOffset += tableSize;
    ClassScopePtr parentCls = info.cls->getNextParentWithProp(ar);
    if (parentCls) {
      cg_printf("&%s%s::%sprop_table,",
                Option::ClassPrefix, parentCls->getId().c_str(),
                Option::ObjectStaticPrefix);
    } else {
      cg_printf("0,");
    }
    cg_printf("%stable_entries+%d,",
              Option::ClassPropTablePrefix, curOffset);
    cg_printf("%sprivate_entries+%d\n",
              Option::ClassPropTablePrefix,
              pCount ? privateOffset : privateEntries - 1);
    cg_indentEnd("};\n");

    curOffset += info.actualIndex.size() - 1;
    privateOffset += pCount + (pCount > 0);
  }
}

void ClassScope::outputForwardDeclaration(CodeGenerator &cg) {
  string clsNameStr = getId();
  const char *clsName = clsNameStr.c_str();
  if (!isInterface()) {
    cg_printf("FORWARD_DECLARE_CLASS(%s);\n", clsName);
  } else if (!Option::UseVirtualDispatch || isRedeclaring()) {
    cg_printf("FORWARD_DECLARE_GENERIC_INTERFACE(%s);\n", clsName);
  } else {
    cg_printf("FORWARD_DECLARE_INTERFACE(%s);\n", clsName);
  }
}

bool ClassScope::hasProperty(const string &name) const {
  const Symbol *sym = m_variables->getSymbol(name);
  ASSERT(!sym || sym->isPresent());
  return sym;
}

void ClassScope::setRedeclaring(AnalysisResultConstPtr ar, int redecId) {
  m_redeclaring = redecId;
  setVolatile(); // redeclared class is also volatile
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      fs->setDynamic();
    }
  }
  m_variables->forceVariants(ar, VariableTable::AnyNonPrivateVars);
}

ClassScopePtr ClassScope::getRootParent(AnalysisResultConstPtr ar,
                                        const std::string &methodName) {
  ClassScopePtr root = dynamic_pointer_cast<ClassScope>(shared_from_this());
  for (ClassScopePtr cls = getParentScope(ar); cls;
       cls = cls->getParentScope(ar)) {
    if (methodName.empty() ||
        cls->m_functions.find(methodName) != cls->m_functions.end()) {
      root = cls;
    }
  }
  return root;
}

void ClassScope::getRootParents(AnalysisResultConstPtr ar,
                                const std::string &methodName,
                                ClassScopePtrVec &roots,
                                ClassScopePtr curClass) {
  ClassScopePtr root = dynamic_pointer_cast<ClassScope>(shared_from_this());
  if (m_parent.empty()) {
    roots.push_back(curClass);
  } else {
    ClassScopePtrVec parents = ar->findRedeclaredClasses(m_parent);
    for (unsigned int i = 0; i < parents.size(); i++) {
      ClassScopePtr cls = parents[i];
      if (methodName.empty() ||
          cls->m_functions.find(methodName) != cls->m_functions.end()) {
        curClass = cls;
      }
      cls->getRootParents(ar, methodName, roots, curClass);
    }
  }
}

string ClassScope::getBaseHeaderFilename() {
  FileScopePtr file = getContainingFile();
  ASSERT(file);
  string fileBase = file->outputFilebase();
  string headerFile = Option::ClassHeaderPrefix;
  headerFile += getId();
  return headerFile;
}

string ClassScope::getHeaderFilename() {
  return getBaseHeaderFilename() + ".h";
}

std::string ClassScope::getForwardHeaderFilename() {
  return getBaseHeaderFilename() + ".fw.h";
}

void ClassScope::outputCPPHeader(AnalysisResultPtr ar,
                                 CodeGenerator::Output output) {
  string filename = getHeaderFilename();
  string root = ar->getOutputPath() + "/";
  Util::mkdir(root + filename);
  ofstream f((root + filename).c_str());
  CodeGenerator cg(&f, output);
  cg.setFileOrClassHeader(true);

  cg.headerBegin(filename);

  cg_printInclude(getForwardHeaderFilename());

  // 1. includes
  BOOST_FOREACH(string base, m_bases) {
    ClassScopePtr cls = ar->findClass(base);
    if (cls && cls->isUserClass()) {
      cg_printInclude(cls->getHeaderFilename());
    }
  }

  // 2. Declarations
  cg.namespaceBegin();
  cg.setContext(CodeGenerator::CppDeclaration);
  getStmt()->outputCPP(cg, ar);

  cg.namespaceEnd();

  cg.headerEnd(filename);
}

void ClassScope::outputCPPForwardHeader(AnalysisResultPtr ar,
                                        CodeGenerator::Output output) {
  string filename = getForwardHeaderFilename();
  string root = ar->getOutputPath() + "/";
  Util::mkdir(root + filename);
  ofstream f((root + filename).c_str());
  CodeGenerator cg(&f, output);
  cg.setContext(CodeGenerator::CppForwardDeclaration);

  cg.headerBegin(filename);
  cg.printBasicIncludes();

  BOOST_FOREACH(const string &dep, m_usedClassesFullHeader) {
    ClassScopePtr cls = ar->findClass(dep);
    if (cls && cls->isUserClass()) {
      cg_printInclude(cls->getHeaderFilename());
    }
  }

  bool first = true;
  BOOST_FOREACH(const string &str, m_usedLiteralStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    assert(index != -1);
    string lisnam = ar->getLiteralStringName(stringId, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern StaticString %s;\n", lisnam.c_str());
  }

  first = true;
  BOOST_FOREACH(const int64 &val, m_usedScalarVarIntegersHeader) {
    int index = -1;
    int hash = ar->checkScalarVarInteger(val, index);
    assert(index != -1);
    string name = ar->getScalarVarIntegerName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const double &val, m_usedScalarVarDoublesHeader) {
    int index = -1;
    int hash = ar->checkScalarVarDouble(val, index);
    assert(index != -1);
    string name = ar->getScalarVarDoubleName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedLitVarStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    assert(index != -1);
    string lisnam = ar->getLitVarStringName(stringId, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern VarNR %s;\n", lisnam.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    assert(hash != -1 && index != -1);
    string name = ar->getScalarArrayName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern StaticArray %s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarVarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    assert(hash != -1 && index != -1);
    string name = ar->getScalarVarArrayName(hash, index);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cg_printf("extern VarNR %s;\n", name.c_str());
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedConstsHeader) {
    BlockScopeConstPtr block = ar->findConstantDeclarer(str);
    assert(block);
    ConstantTableConstPtr constants = block->getConstants();
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    constants->outputSingleConstant(cg, ar, str);
  }

  first = true;
  BOOST_FOREACH(const UsedClassConst& item, m_usedClassConstsHeader) {
    ClassScopePtr cls = ar->findClass(item.first);
    assert(cls);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    cls->getConstants()->outputSingleConstant(cg, ar, item.second);
  }

  first = true;
  BOOST_FOREACH(const string &str, m_usedClassesHeader) {
    ClassScopePtr usedClass = ar->findClass(str);
    assert(usedClass);
    if (!cg.ensureInNamespace() && first) cg_printf("\n");
    first = false;
    usedClass->outputForwardDeclaration(cg);
  }

  cg.ensureOutOfNamespace();
  cg.headerEnd(filename);
}

void ClassScope::outputCPPSupportMethodsImpl(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  string clsNameStr = getId();
  const char *clsName = clsNameStr.c_str();
  bool dynamicObject = derivesFromRedeclaring() == DirectFromRedeclared;
  string parent = "ObjectData";
  string parentName = "ObjectData";
  if (!getParent().empty()) {
    parentName = getParent();
    ClassScopePtr cls = ar->findClass(parentName);
    if (cls) {
      parent = cls->getId();
    } else {
      parent = parentName;
    }
  }

  if (Option::GenerateCPPMacros) {
    // Constant Lookup Table
    getVariables()->outputCPPPropertyTable(cg, ar, parent.c_str(),
                                           parentName.c_str(),
                                           derivesFromRedeclaring());

    // If parent is redeclared, you have to go to their class statics object.
    if (dynamicObject) {
      cg_indentBegin("Variant %s%s::%sconstant(const char *s) {\n",
                     Option::ClassPrefix, clsName, Option::ObjectStaticPrefix);
      cg.printDeclareGlobals();
      getConstants()->outputCPPJumpTable(cg, ar, !dynamicObject, false);
      cg_printf("return %s->%s%s->%sconstant(s);\n", cg.getGlobals(ar),
                Option::ClassStaticsCallbackPrefix, parentName.c_str(),
                Option::ObjectStaticPrefix);
      cg_indentEnd("}\n");
    } else {
      cg.ifdefBegin(false, "OMIT_JUMP_TABLE_CLASS_CONSTANT_%s", clsName);
      cg_indentBegin("Variant %s%s::%sconstant(const char *s) {\n",
                     Option::ClassPrefix, clsName, Option::ObjectStaticPrefix);
      getConstants()->outputCPPJumpTable(cg, ar, !dynamicObject, false);
      cg_printf("return %s%s::%sconstant(s);\n", Option::ClassPrefix,
                parent.c_str(), Option::ObjectStaticPrefix);
      cg_indentEnd("}\n");
      cg.ifdefEnd("OMIT_JUMP_TABLE_CLASS_CONSTANT_%s", clsName);
    }

    if ((isUserClass() && !isSepExtension()) ||
        m_attributeClassInfo & ClassInfo::NoDefaultSweep) {
      cg_printf("IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(%s)\n", clsName);
    } else {
      cg_printf("IMPLEMENT_CLASS(%s)\n", clsName);
    }
  }

  // instanceof
  {
    vector<string> bases;
    getAllParents(ar, bases);
    // Eliminate duplicates
    sort(bases.begin(), bases.end());
    bases.erase(unique(bases.begin(), bases.end()), bases.end());
    vector<const char *> ancestors;
    // Convert to char * and add self
    ancestors.push_back(m_name.c_str());
    for (unsigned int i = 0; i < bases.size(); i++) {
      ancestors.push_back(bases[i].c_str());
    }
    cg_indentBegin("const InstanceOfInfo %s%s::s_instanceof_table[] = {\n",
                   Option::ClassPrefix, clsName);
    vector<int> offsets;
    int n = 0;
    JumpTable jt(cg, ancestors, true, false, true, true);
    for (; jt.ready(); ++n, jt.next()) {
      unsigned cur = jt.current();
      if (offsets.size() <= cur) {
        offsets.resize(cur, -1);
        offsets.push_back(n);
      }
      const char *name = jt.key();
      ClassScopePtr cls;
      bool knownClass;
      if (name == m_name) {
        cls = static_pointer_cast<ClassScope>(shared_from_this());
        knownClass = true;
      } else {
         cls = ar->findClass(name);
         knownClass = !cls->isRedeclaring();
      }
      if (knownClass) {
        name = cls->getOriginalName().c_str();
      }
      cg_printf("{0x%016llXLL,%d,\"%s\",",
                hash_string_i(name), jt.last(),
                CodeGenerator::EscapeLabel(name).c_str());
      if (knownClass) {
        if (cls->isInterface()) {
          cg_printf("(const ObjectStaticCallbacks*)2},\n");
        } else {
          cg_printf("&%s%s%s},\n",
                    Option::ClassWrapperFunctionPrefix,
                    cls->getId().c_str(),
                    cls->isRedeclaring() ? ".oscb" : "");
        }
      } else {
        cg_printf("(const ObjectStaticCallbacks*)"
                  "(offsetof(GlobalVariables, %s%s)+1)},\n",
                  Option::ClassStaticsCallbackPrefix,
                  CodeGenerator::FormatLabel(Util::toLower(name)).c_str());
      }
    }
    cg_indentEnd("};\n");
    cg_indentBegin("const int %s%s::s_instanceof_index[] = {\n",
                   Option::ClassPrefix, clsName);
    cg_printf("%d,\n", jt.size() - 1);
    for (int i = 0, e = jt.size(), s = offsets.size(); i < e; i++) {
      cg_printf("%d,", i < s ? offsets[i] : -1);
      if ((i & 7) == 7) cg_printf("\n");
    }
    cg_printf("\n");
    cg_indentEnd("};\n");
  }

  // Cloning
  cg_indentBegin("ObjectData *%s%s::cloneImpl() {\n",
                 Option::ClassPrefix, clsName);
  cg_printf("ObjectData *obj = %s%s();\n",
            Option::CreateObjectOnlyPrefix, clsName);
  cg_printf("%s%s::cloneSet(obj);\n", Option::ClassPrefix, clsName);
  cg_printf("return obj;\n");
  cg_indentEnd("}\n");
  cg_indentBegin("void %s%s::cloneSet(ObjectData *cl) {\n",
                 Option::ClassPrefix, clsName);
  cg_printf("%s%s *clone = static_cast<%s%s*>(cl);\n",
            Option::ClassPrefix, clsName, Option::ClassPrefix, clsName);
  if (derivesFromRedeclaring() == DirectFromRedeclared) {
    cg_printf("DynamicObjectData::cloneSet(clone);\n");
  } else if(!getParent().empty()) {
    cg_printf("%s%s::cloneSet(clone);\n", Option::ClassPrefix, parent.c_str());
  } else {
    cg_printf("ObjectData::cloneSet(clone);\n");
  }
  getVariables()->outputCPPPropertyClone(cg, ar, derivesFromRedeclaring());
  cg_indentEnd("}\n");

  // doCall
  if (getAttribute(ClassScope::HasUnknownMethodHandler)) {
    cg_indentBegin("Variant %s%s::doCall(Variant v_name, Variant "
                   "v_arguments, bool fatal) {\n",
                   Option::ClassPrefix, clsName);
    cg_printf("return t___call(v_name, !v_arguments.isNull() ? "
              "v_arguments : Variant(Array::Create()));\n");
    cg_indentEnd("}\n");
  }

  // __invoke
  if (getAttribute(ClassScope::HasInvokeMethod)) {
    FunctionScopePtr func = findFunction(ar, "__invoke", false);
    ASSERT(func);
    if (!func->isAbstract()) {
      // the closure class will generate its own version of
      // t___invokeCallInfoHelper, which will avoid a level
      // of indirection
      if (strcasecmp(clsName, "closure")) {
        cg_indentBegin("const CallInfo *"
                       "%s%s::t___invokeCallInfoHelper(void *&extra) {\n",
                       Option::ClassPrefix, clsName);
        cg_printf("extra = (void*) this;\n");
        cg_printf("return &%s%s::%s%s;\n",
                  Option::ClassPrefix,
                  clsName,
                  Option::CallInfoWrapperPrefix,
                  CodeGenerator::FormatLabel("__invoke").c_str());
        cg_indentEnd("}\n");
      }
    }
  }

  if (isRedeclaring() && !derivesFromRedeclaring() && derivedByDynamic()) {
    cg_indentBegin("Variant %s%s::doRootCall(Variant v_name, Variant "
                   "v_arguments, bool fatal) {\n",
                   Option::ClassPrefix, clsName);
    cg_printf("return root->doCall(v_name, v_arguments, fatal);\n");
    cg_indentEnd("}\n");
  }

  // Invoke tables
  if (Option::GenerateCPPMacros) {
    bool hasRedec;
    outputCPPCallInfoTableSupport(cg, ar, hasRedec);
    outputCPPHelperClassAllocSupport(cg, ar);
    vector<const char *> funcs;
    findJumpTableMethods(cg, ar, false, funcs);
    outputCPPMethodInvokeTableSupport(cg, ar, funcs, m_functions, false);
    outputCPPMethodInvokeTableSupport(cg, ar, funcs, m_functions, true);
    if (getAttribute(ClassScope::HasInvokeMethod)) {
      // see above - closure does not need the bare object support,
      // since we already have generated such a function (the
      // closure function itself)
      if (strcasecmp(clsName, "closure")) {
        FunctionScopePtr func(findFunction(ar, "__invoke", false));
        ASSERT(func);
        if (!func->isAbstract()) {
          outputCPPMethodInvokeBareObjectSupport(cg, ar, func, false);
          outputCPPMethodInvokeBareObjectSupport(cg, ar, func, true);
        }
      }
    }
    if (classNameCtor()) {
      funcs.push_back("__construct");
    }
    if (funcs.size() || derivesFromRedeclaring()) {
      outputCPPMethodInvokeTable(cg, ar, funcs, m_functions, false, true);
    } else {
      m_emptyJumpTables.insert(JumpTableCallInfo);
    }
  }

  // Create method
  if (getAttribute(ClassScope::HasConstructor) ||
      getAttribute(ClassScope::ClassNameConstructor)) {
    FunctionScopePtr func = findConstructor(ar, false);
    if (func && !func->isAbstract() && !isInterface()) {
      // abstract methods are not generated, neither should the create method
      // for an abstract constructor
      func->outputCPPCreateImpl(cg, ar);
    }
  }

  outputCPPGlobalTableWrappersImpl(cg, ar);
}

void ClassScope::outputCPPStaticInitializerDecl(CodeGenerator &cg) {
  if (needStaticInitializer()) {
    cg_printf("void %s%s();\n", Option::ClassStaticInitializerPrefix,
              getId().c_str());
  }
}

void ClassScope::outputCPPStaticMethodWrappers(CodeGenerator &cg,
                                               AnalysisResultPtr ar,
                                               set<string> &done,
                                               const char *cls) {
  const StringToFunctionScopePtrVecMap &fmap = getFunctions();
  for (StringToFunctionScopePtrVecMap::const_iterator it = fmap.begin();
       it != fmap.end(); ++it) {
    if (done.find(it->first) != done.end()) continue;
    MethodStatementPtr m =
      dynamic_pointer_cast<MethodStatement>(it->second[0]->getStmt());
    if (!m) continue; // system classes
    m->outputCPPStaticMethodWrapper(cg, ar, cls);
    done.insert(it->first);
  }
  if (derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr par = getParentScope(ar);
    if (par) par->outputCPPStaticMethodWrappers(cg, ar, done, cls);
  }
}

void ClassScope::outputCPPGlobalTableWrappersDecl(CodeGenerator &cg,
                                                  AnalysisResultPtr ar) {
  string id = getId();
  cg_printf("extern const %sObjectStaticCallbacks %s%s;\n",
            isRedeclaring() ? "Redeclared" : "",
            Option::ClassWrapperFunctionPrefix, id.c_str());
}

string ClassScope::getClassPropTableId(AnalysisResultPtr ar) {
  if (checkHasPropTable()) return getId();

  if (derivesFromRedeclaring() != DirectFromRedeclared) {
    if (ClassScopePtr p = getParentScope(ar)) {
      return p->getClassPropTableId(ar);
    }
  }

  return "";
}

void ClassScope::outputCPPGlobalTableWrappersImpl(CodeGenerator &cg,
                                                  AnalysisResultPtr ar) {
  string id = getId();
  string prop = getClassPropTableId(ar);
  cg_indentBegin("const %sObjectStaticCallbacks %s%s = {\n",
                 isRedeclaring() ? "Redeclared" : "",
                 Option::ClassWrapperFunctionPrefix, id.c_str());
  if (isRedeclaring()) {
    cg_indentBegin("{\n");
  }
  // This order must match the one in object_data.h
  cg_printf("%s%s::%sgetInit,\n", Option::ClassPrefix, id.c_str(),
            Option::ObjectStaticPrefix);
  cg_printf("%s%s::%sget,\n", Option::ClassPrefix, id.c_str(),
            Option::ObjectStaticPrefix);
  cg_printf("%s%s::%slval,\n", Option::ClassPrefix, id.c_str(),
            Option::ObjectStaticPrefix);
  cg_printf("%s%s::%sconstant,\n", Option::ClassPrefix, id.c_str(),
            Option::ObjectStaticPrefix);
  cg_printf("(ObjectData*(*)(ObjectData*))%s%s,\n",
            Option::CreateObjectOnlyPrefix, id.c_str());
  cg_printf("%s%s::s_call_info_table,%s%s::s_call_info_index,\n",
            Option::ClassPrefix, id.c_str(),
            Option::ClassPrefix, id.c_str());
  cg_printf("%s%s::s_instanceof_table,%s%s::s_instanceof_index,\n",
            Option::ClassPrefix, id.c_str(),
            Option::ClassPrefix, id.c_str());
  cg_printf("&%s%s::s_class_name,\n", Option::ClassPrefix, id.c_str());
  if (prop.empty()) {
    cg_printf("0,");
  } else {
    cg_printf("&%s%s::%sprop_table,",
              Option::ClassPrefix, prop.c_str(),
              Option::ObjectStaticPrefix);
  }

  ClassScopeRawPtr par;
  if (derivesFromRedeclaring() != FromNormal) {
    par = ClassScopeRawPtr(this);
    do {
      par = par->getParentScope(ar);
    } while (par && !par->isRedeclaring());
  }
  if (par) {
    cg_printf("offsetof(GlobalVariables, %s%s),",
              Option::ClassStaticsCallbackPrefix,
              CodeGenerator::FormatLabel(par->m_name).c_str());
  } else {
    cg_printf("0,");
  }

  if (derivesFromRedeclaring() != DirectFromRedeclared &&
      (par = getParentScope(ar))) {
    cg_printf("&%s%s\n",
              Option::ClassWrapperFunctionPrefix, par->getId().c_str());
  } else {
    cg_printf("0\n");
  }
  if (isRedeclaring()) {
    cg_indentEnd("},\n");
    cg_printf("%d\n", m_redeclaring);
  }
  cg_indentEnd("};\n");
}

bool ClassScope::addFunction(AnalysisResultConstPtr ar,
                             FunctionScopePtr funcScope) {
  FunctionScopePtrVec &funcs = m_functions[funcScope->getName()];
  if (funcs.size() == 1) {
    funcs[0]->setRedeclaring(0);
  }
  if (funcs.size() > 0) {
    funcScope->setRedeclaring(funcs.size());
  }
  funcs.push_back(funcScope);
  m_functionsVec.push_back(funcScope);
  return true;
}

void ClassScope::findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                                      bool staticOnly,
                                      vector<const char *> &funcs) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  // output invoke support methods
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!iter->second[0]->isRedeclaring()) {
      FunctionScopePtr func = iter->second[0];
      if (func->isAbstract() ||
          (staticOnly && !func->isStatic()) ||
          !(systemcpp || func->isDynamic() || func->isVirtual())) continue;
      const char *name = iter->first.c_str();
      funcs.push_back(name);
    }
  }
}

void ClassScope::outputCPPMethodInvokeBareObjectSupport(
    CodeGenerator &cg, AnalysisResultPtr ar,
    FunctionScopePtr func, bool fewArgs) {

  if (Option::InvokeWithSpecificArgs && !fewArgs &&
      !ar->isSystem() && !ar->isSepExtension() &&
      func->getMaxParamCount() == 0 && !func->isVariableArgument()) {
    return;
  }

  const string &id(getId());
  const string &lname(func->getName());

  cg_indentBegin("Variant %s%s::%s%s(void *self, ",
                  Option::ClassPrefix, id.c_str(),
                  fewArgs ? Option::InvokeWrapperFewArgsPrefix :
                    Option::InvokeWrapperPrefix,
                  lname.c_str());
  if (fewArgs) {
    cg_printf("int count, INVOKE_FEW_ARGS_IMPL_ARGS");
  } else {
    cg_printf("CArrRef params");
  }
  cg_printf(") {\n");
  cg_printf("MethodCallPackage mcp;\n");
  if (func->isStatic() && func->needsClassParam()) {
    cg_printf("mcp.isObj = true;\n");
    cg_printf("mcp.rootObj = static_cast<ObjectData*>(self);\n");
  } else {
    cg_printf("mcp.obj = static_cast<ObjectData*>(self);\n");
  }
  if (fewArgs) {
    cg_printf("return %s%s::%s%s(mcp, count, INVOKE_FEW_ARGS_PASS_ARGS);\n",
              Option::ClassPrefix,
              id.c_str(),
              Option::InvokeFewArgsPrefix,
              lname.c_str());
  } else {
    cg_printf("return %s%s::%s%s(mcp, params);\n",
              Option::ClassPrefix,
              id.c_str(),
              Option::InvokePrefix,
              lname.c_str());
  }
  cg_indentEnd("}\n");

}

void ClassScope::outputCPPMethodInvokeTableSupport(CodeGenerator &cg,
    AnalysisResultPtr ar, const vector<const char*> &keys,
    const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs) {
  string id = getId();
  ClassScopePtr self = dynamic_pointer_cast<ClassScope>(shared_from_this());
  for (vector<const char*>::const_iterator it = keys.begin();
      it != keys.end(); ++it) {
    const char *name = *it;
    string lname = CodeGenerator::FormatLabel(name);
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs;
    iterFuncs = funcScopes.find(name);
    ASSERT(iterFuncs != funcScopes.end());
    FunctionScopePtr func = iterFuncs->second[0];

    // For functions with no parameter, we can combine the i_ wrapper and
    // the ifa_ wrapper.
    if (Option::InvokeWithSpecificArgs && !fewArgs &&
        !ar->isSystem() && !ar->isSepExtension() &&
        func->getMaxParamCount() == 0 && !func->isVariableArgument()) {
      continue;
    }

    const char *extra = NULL;
    string prefix;
    const char *instance = NULL;
    if (func->isStatic()) {
      prefix += Option::ClassPrefix;
      prefix += id;
      prefix += "::";
      if (func->needsClassParam()) {
        prefix += Option::MethodImplPrefix;
        extra = "c";
      } else {
        prefix += Option::MethodPrefix;
      }
    } else {
      instance = "self->";
      prefix += Option::MethodPrefix;
    }
    string origName = func->getOriginalFullName();
    cg_printf("Variant");
    if (fewArgs && Option::FunctionSections.find(origName) !=
                   Option::FunctionSections.end()) {
      string funcSection = Option::FunctionSections[origName];
      if (!funcSection.empty()) {
        cg_printf(" __attribute__ ((section (\".text.%s\")))",
                  funcSection.c_str());
      }
    }
    cg_indentBegin(" %s%s::%s%s(MethodCallPackage &mcp, ",
                   Option::ClassPrefix, id.c_str(),
                   fewArgs ? Option::InvokeFewArgsPrefix : Option::InvokePrefix,
                   lname.c_str());
    if (fewArgs) {
      cg_printf("int count, INVOKE_FEW_ARGS_IMPL_ARGS");
    } else {
      cg_printf("CArrRef params");
    }
    cg_printf(") {\n");
    const char *class_name = "";
    if (!func->isStatic()) {
      // Instance method called as such
      cg_indentBegin("if (UNLIKELY(mcp.obj == 0)) {\n");
      cg_printf("return ObjectData::%sdummy(mcp, ",
                fewArgs ? Option::InvokeFewArgsPrefix : Option::InvokePrefix);
      if (fewArgs) {
        cg_printf("count, INVOKE_FEW_ARGS_PASS_ARGS");
      } else {
        cg_printf("params");
      }
      cg_printf(", %s%s, %s%s);\n",
                fewArgs ? Option::InvokeFewArgsPrefix : Option::InvokePrefix,
                lname.c_str(),
                Option::CreateObjectOnlyPrefix, id.c_str());
      cg_indentEnd("}\n");
      cg_printf("%s%s *self ATTRIBUTE_UNUSED (static_cast<%s%s*>(mcp.obj));\n",
                Option::ClassPrefix, id.c_str(),
                Option::ClassPrefix, id.c_str());
    } else if (func->needsClassParam()) {
      // If mcp contains an object, was a static method invoked instance style.
      // Use rootObj's class name as invoking class
      class_name =
        "CStrRef c(mcp.isObj"
        " ? mcp.rootObj->o_getClassName()"
        " : String(mcp.rootCls));\n";
    }
    if (!fewArgs) FunctionScope::OutputCPPDynamicInvokeCount(cg);
    func->outputCPPDynamicInvoke(cg, ar, prefix.c_str(),
                                 lname.c_str(), false, fewArgs, true, extra,
                                 func->isConstructor(self), instance,
                                 class_name);
    cg_indentEnd("}\n");
  }
}

void ClassScope::outputCPPMethodInvokeTable(
  CodeGenerator &cg, AnalysisResultPtr ar,
  const vector<const char*> &keys,
  const StringToFunctionScopePtrVecMap &funcScopes,
  bool fewArgs, bool staticOnly) {
  ClassScopePtr self = dynamic_pointer_cast<ClassScope>(shared_from_this());

  string clsid = self->getId();
  shared_ptr<JumpTable> jt(new JumpTable(cg, keys, true, true, true, true));
  vector<int> offsets;
  int prev = -1;
  cg_indentBegin("const MethodCallInfoTable %s%s::s_call_info_table[] = {\n",
                 Option::ClassPrefix, clsid.c_str());
  for (int n = 0; jt->ready(); ++n, jt->next()) {
    int cur = jt->current();
    bool changed = false;
    if (prev != cur) {
      changed = true;
      while (++prev != cur) {
        offsets.push_back(-1);
      }
      offsets.push_back(n);
    }
    const char *name = jt->key();
    string lname = CodeGenerator::FormatLabel(name);
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      funcScopes.find(name);
    FunctionScopePtr func;
    string origName;
    if (iterFuncs == funcScopes.end()) {
      assert(classNameCtor() && !strcmp(name, "__construct"));
      func = findConstructor(ar, false);
      lname = CodeGenerator::FormatLabel(func->getName());
      origName = name;
    } else {
      func = iterFuncs->second[0];
      origName = func->getOriginalName();
    }
    if (fewArgs &&
        func->getMinParamCount() > Option::InvokeFewArgsCount) {
      continue;
    }
    string id = func->getContainingClass()->getId();
    int index = -1;
    cg.checkLiteralString(origName, index, ar, shared_from_this());
    cg_printf("{ 0x%016llXLL, %d, %d, \"%s\", &%s%s::%s%s },\n",
              hash_string_i(origName.c_str()),
              (int)changed,
              (int)origName.size(),
              CodeGenerator::EscapeLabel(origName).c_str(),
              Option::ClassPrefix, id.c_str(),
              Option::CallInfoPrefix, lname.c_str());
  }
  cg_printf("{ 0, 1, 0, 0 }\n");
  cg_indentEnd("};\n");
  cg_indentBegin("const int %s%s::s_call_info_index[] = {\n",
                 Option::ClassPrefix, clsid.c_str());
  if (!jt->size()) {
    cg_printf("0,-1");
  } else {
    cg_printf("%d,\n", jt->size() - 1);
    for (int i = 0, e = jt->size(), s = offsets.size(); i < e; i++) {
      cg_printf("%d,", i < s ? offsets[i] : -1);
      if ((i & 7) == 7) cg_printf("\n");
    }
  }
  cg_printf("\n");
  cg_indentEnd("};\n");
}

void ClassScope::outputCPPJumpTableDecl(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second[0];
    string id = CodeGenerator::FormatLabel(func->getName());
    bool needsWrapper = func->getName() == "__invoke";
    if (Option::InvokeWithSpecificArgs &&
        !ar->isSystem() && !ar->isSepExtension() &&
        func->getMaxParamCount() == 0 && !func->isVariableArgument()) {
      cg_printf("DECLARE_METHOD_INVOKE_HELPERS_NOPARAM(%s);\n", id.c_str());
      if (needsWrapper) {
        cg_printf("DECLARE_METHOD_INVOKE_WRAPPER_HELPERS_NOPARAM(%s);\n",
                  id.c_str());
      }
    } else {
      cg_printf("DECLARE_METHOD_INVOKE_HELPERS(%s);\n", id.c_str());
      if (needsWrapper) {
        cg_printf("DECLARE_METHOD_INVOKE_WRAPPER_HELPERS(%s);\n",
                  id.c_str());
      }
    }
  }
}

void ClassScope::outputCPPJumpTable(CodeGenerator &cg,
    AnalysisResultPtr ar, bool staticOnly, bool dynamicObject) {
#if 0
  string id = getId();
  string scope;
  scope += Option::ClassPrefix;
  scope += id;
  scope += "::";
  string parentExpr, parent, parentName;
  if (m_parent.empty()) {
    parentName = "ObjectData";
    parent = "ObjectData";
  } else {
    parentName = m_parent;
    ClassScopePtr cls = ar->findClass(m_parent);
    if (cls) {
      parent = cls->getId();
    } else {
      parent = parentName;
    }
  }
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool needGlobals = false;
  if (dynamicObject) {
    if (staticOnly) {
      needGlobals = true;
      parentExpr = string("g->") + Option::ClassStaticsCallbackPrefix +
        parentName + "->";
    } else {
      parentExpr = string("parent->");
    }
  } else {
    /* needsInvokeParent(ar) ? */
    parentExpr = string(Option::ClassPrefix) + parent + "::";
  }
  string invokeName;
  invokeName += staticOnly ? Option::ObjectStaticPrefix : Option::ObjectPrefix;

  invokeName += "get_call_info";

  parentExpr += invokeName;
  StringToFunctionScopePtrVecMap flatScopes;
  bool flatten = false && Option::FlattenInvoke;
  if (flatten) {
    StringToFunctionScopePtrMap fss;
    collectMethods(ar, fss, true, true);
    for (StringToFunctionScopePtrMap::const_iterator it = fss.begin();
         it != fss.end(); ++it) {
      flatScopes[it->first].push_back(it->second);
    }
  }

  vector<const char *> funcs;
  findJumpTableMethods(cg, ar, false, funcs);

  if (flatten) {
    funcs.clear();
    for (StringToFunctionScopePtrVecMap::const_iterator iter =
           flatScopes.begin(); iter != flatScopes.end(); ++iter) {
      FunctionScopePtr func = iter->second[0];
      if (func->isAbstract() || func->inPseudoMain() ||
          (staticOnly && !func->isStatic()) ||
          !(system || func->isDynamic() || func->isVirtual())) continue;
      funcs.push_back(iter->first.c_str());
    }
  }

  if (classNameCtor()) {
    funcs.push_back("__construct");
  }

  StringToFunctionScopePtrVecMap &funcScopes = flatten ? flatScopes :
    m_functions;

  cg_indentBegin("bool %s%s(MethodCallPackage &mcp, int64 hash) {\n",
                 scope.c_str(), invokeName.c_str());
  if (needGlobals) cg.printDeclareGlobals();
  if (funcs.size()) {
    cg_printf("CStrRef s ATTRIBUTE_UNUSED (*mcp.name);\n");

    outputCPPMethodInvokeTable(cg, ar, funcs, funcScopes, false, staticOnly);
    cg_printf("if (hash < 0) hash = s->hash();\n");
    cg_indentBegin("if (ObjectData::LookupMCP("
                   "mcp, hash, s, mcit_ix, mcit)) {\n");
    if (!staticOnly) cg_printf("mcp.obj = this;\n");
    cg_printf("return true;\n");
    cg_indentEnd("}\n");
  }
  cg_printf("return %s(mcp, hash);\n", parentExpr.c_str());
  cg_indentEnd("}\n");
#endif
}

void ClassScope::outputVolatileCheckBegin(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          BlockScopePtr bs,
                                          const std::string &name) {
  if (isVolatile()) {
    OutputVolatileCheckBegin(cg, ar, bs, name);
  }
}
void ClassScope::outputVolatileCheckEnd(CodeGenerator &cg) {
  if (isVolatile()) {
    OutputVolatileCheckEnd(cg);
  }
}

void ClassScope::OutputVolatileCheckBegin(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          BlockScopePtr bs,
                                          const string &origName) {
  cg_printf("((");
  OutputVolatileCheck(cg, ar, bs, origName, false);
  cg_printf("), (");
}

void ClassScope::OutputVolatileCheckEnd(CodeGenerator &cg) {
  cg_printf("))");
}
void ClassScope::OutputVolatileCheck(CodeGenerator &cg, AnalysisResultPtr ar,
                                     BlockScopePtr bs, const string &origName,
                                     bool noThrow) {
  string lwrName(Util::toLower(origName));
  bool exist = ar->findClass(lwrName);
  cg_printf("%s%s(",
            exist ? "checkClassExists" : "autoloadClass",
            noThrow ? "NoThrow" : "Throw");
  cg_printString(origName, ar, bs);
  if (exist) {
    cg_printf(", &%s->CDEC(%s))",
              cg.getGlobals(ar), CodeGenerator::FormatLabel(lwrName).c_str());
  } else {
    cg_printf(", (bool*)0)");
  }
}

void ClassScope::outputMethodWrappers(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  if (!isInterface()) {
    string name = getId();

    FunctionScopePtr constructor = findConstructor(ar, true);
    if (constructor) {
      if (!constructor->isAbstract()) {
        constructor->outputMethodWrapper(cg, ar, name.c_str());
        cg_printf("\n");
      }
    } else {
      cg_indentBegin("static %s%s Create() {\n", Option::SmartPtrPrefix,
                     name.c_str());
      cg_printf("return NEWOBJ(%s%s)();\n", Option::ClassPrefix, name.c_str());
      cg_indentEnd("}\n");
      cg_printf("\n");
    }

    ClassScopePtr self = static_pointer_cast<ClassScope>(shared_from_this());
    for (unsigned int i = 0; i < m_functionsVec.size(); i++) {
      FunctionScopePtr func = m_functionsVec[i];
      if (func->isPublic() && !func->isConstructor(self) &&
          !func->isMagic() && !func->isAbstract()) {
        func->outputMethodWrapper(cg, ar, NULL);
      }
    }
  }
}

bool ClassScope::canSkipCreateMethod() const {
  // create() is not necessary if
  // 1) not inheriting from any class
  // 2) no constructor defined (__construct or class name)
  // 3) no init() defined
  if (!m_parent.empty())                  return false;
  if (getAttribute(HasConstructor) ||
      getAttribute(ClassNameConstructor)) return false;
  if (needsInitMethod())                  return false;
  return true;
}
