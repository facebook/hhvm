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

#include "hphp/compiler/analysis/class_scope.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/construct.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/trait-method-import-data.h"

#include "hphp/util/text-util.h"

#include <folly/Conv.h>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include <map>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace HPHP;
using std::map;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(FileScopeRawPtr fs,
                       KindOf kindOf, const std::string &originalName,
                       const std::string &parent,
                       const vector<string> &bases,
                       const std::string &docComment, StatementPtr stmt,
                       const std::vector<UserAttributePtr> &attrs)
  : BlockScope(originalName, docComment, stmt, BlockScope::ClassScope),
    m_parent(parent), m_bases(bases), m_attribute(0), m_redeclaring(-1),
    m_kindOf(kindOf), m_derivesFromRedeclaring(Derivation::Normal),
    m_traitStatus(NOT_FLATTENED), m_volatile(false),
    m_persistent(false), m_derivedByDynamic(false),
    m_needsCppCtor(false), m_needsInit(true) {

  m_dynamic = Option::IsDynamicClass(m_scopeName);

  // dynamic class is also volatile
  m_volatile = Option::AllVolatile || m_dynamic;

  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(fs,
                               Compiler::DeclaredAttributeTwice,
                               "Redeclared attribute %s",
                               attrs[i]->getName().c_str());
    }
    m_userAttributes[attrs[i]->getName()] = attrs[i]->getExp();
  }

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

// System
ClassScope::ClassScope(AnalysisResultPtr ar,
                       const std::string &originalName,
                       const std::string &parent,
                       const std::vector<std::string> &bases,
                       const FunctionScopePtrVec &methods)
  : BlockScope(originalName, "", StatementPtr(), BlockScope::ClassScope),
    m_parent(parent), m_bases(bases),
    m_attribute(0), m_redeclaring(-1),
    m_kindOf(KindOf::ObjectClass),
    m_derivesFromRedeclaring(Derivation::Normal),
    m_traitStatus(NOT_FLATTENED), m_dynamic(false),
    m_volatile(false), m_persistent(false),
    m_derivedByDynamic(false), m_needsCppCtor(false),
    m_needsInit(true) {
  for (FunctionScopePtr f: methods) {
    if (f->isNamed("__construct")) setAttribute(HasConstructor);
    else if (f->isNamed("__destruct")) setAttribute(HasDestructor);
    else if (f->isNamed("__get"))  setAttribute(HasUnknownPropGetter);
    else if (f->isNamed("__set"))  setAttribute(HasUnknownPropSetter);
    else if (f->isNamed("__call")) setAttribute(HasUnknownMethodHandler);
    else if (f->isNamed("__callstatic")) {
      setAttribute(HasUnknownStaticMethodHandler);
    } else if (f->isNamed("__isset")) setAttribute(HasUnknownPropTester);
    else if (f->isNamed("__unset"))   setAttribute(HasPropUnsetter);
    else if (f->isNamed("__invoke"))  setAttribute(HasInvokeMethod);
    addFunction(ar, FileScopeRawPtr(), f);
  }
  setAttribute(Extension);
  setAttribute(System);

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

bool ClassScope::isNamed(const char* n) const {
  return !strcasecmp(getOriginalName().c_str(), n);
}

const std::string &ClassScope::getOriginalName() const {
  return m_scopeName;
}

std::string ClassScope::getDocName() const {
  string name = getOriginalName();
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix + folly::to<std::string>(m_redeclaring);
}

///////////////////////////////////////////////////////////////////////////////

void ClassScope::derivedMagicMethods(ClassScopePtr super) {
  super->setAttribute(NotFinal);
  if (derivedByDynamic()) {
    super->m_derivedByDynamic = true;
  }
  if (m_attribute & (HasUnknownPropGetter|
                     MayHaveUnknownPropGetter|
                     InheritsUnknownPropGetter)) {
    super->setAttribute(MayHaveUnknownPropGetter);
  }
  if (m_attribute & (HasUnknownPropSetter|
                     MayHaveUnknownPropSetter|
                     InheritsUnknownPropSetter)) {
    super->setAttribute(MayHaveUnknownPropSetter);
  }
  if (m_attribute & (HasUnknownPropTester|
                     MayHaveUnknownPropTester|
                     InheritsUnknownPropTester)) {
    super->setAttribute(MayHaveUnknownPropTester);
  }
  if (m_attribute & (HasPropUnsetter|
                     MayHavePropUnsetter|
                     InheritsPropUnsetter)) {
    super->setAttribute(MayHavePropUnsetter);
  }
  if (m_attribute & (HasUnknownMethodHandler|
                     MayHaveUnknownMethodHandler|
                     InheritsUnknownMethodHandler)) {
    super->setAttribute(MayHaveUnknownMethodHandler);
  }
  if (m_attribute & (HasUnknownStaticMethodHandler|
                     MayHaveUnknownStaticMethodHandler|
                     InheritsUnknownStaticMethodHandler)) {
    super->setAttribute(MayHaveUnknownStaticMethodHandler);
  }
  if (m_attribute & (HasInvokeMethod|
                     MayHaveInvokeMethod|
                     InheritsInvokeMethod)) {
    super->setAttribute(MayHaveInvokeMethod);
  }
  if (m_attribute & (HasArrayAccess|
                     MayHaveArrayAccess|
                     InheritsArrayAccess)) {
    super->setAttribute(MayHaveArrayAccess);
  }
}

void ClassScope::inheritedMagicMethods(ClassScopePtr super) {
  if (super->m_attribute & UsesUnknownTrait) {
    setAttribute(UsesUnknownTrait);
  }
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

void ClassScope::checkDerivation(AnalysisResultPtr ar, hphp_string_iset &seen) {
  seen.insert(m_scopeName);

  hphp_string_iset bases;
  for (int i = m_bases.size() - 1; i >= 0; i--) {
    const string &base = m_bases[i];

    if (seen.find(base) != seen.end() || bases.find(base) != bases.end()) {
      Compiler::Error(
        Compiler::InvalidDerivation,
        m_stmt,
        "The class hierarchy contains a circular reference involving " + base);
      if (i == 0 && !m_parent.empty()) {
        assert(base == m_parent);
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

  seen.erase(m_scopeName);
}

void ClassScope::collectMethods(AnalysisResultPtr ar,
                                StringToFunctionScopePtrMap &funcs,
                                bool collectPrivate) {
  // add all functions this class has
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    const FunctionScopePtr &fs = *iter;
    if (!collectPrivate && fs->isPrivate()) continue;

    FunctionScopePtr &func = funcs[fs->getScopeName()];
    if (!func) {
      func = fs;
    } else {
      func->setVirtual();
      fs->setVirtual();
      fs->setHasOverride();
      if (fs->isFinal()) {
        std::string s__MockClass = "__MockClass";
        ClassScopePtr derivedClass = func->getContainingClass();
        if (derivedClass->m_userAttributes.find(s__MockClass) ==
            derivedClass->m_userAttributes.end()) {
          Compiler::Error(Compiler::InvalidOverride,
                          fs->getStmt(), func->getStmt());
        }
      }
    }
  }

  int n = m_bases.size();
  for (int i = 0; i < n; i++) {
    const string &base = m_bases[i];
    ClassScopePtr super = ar->findClass(base);
    if (super) {
      if (super->isRedeclaring()) {
        const ClassScopePtrVec &classes = ar->findRedeclaredClasses(base);
        StringToFunctionScopePtrMap pristine(funcs);

        for (auto& cls : classes) {
          cls->m_derivedByDynamic = true;
          StringToFunctionScopePtrMap cur(pristine);
          derivedMagicMethods(cls);
          cls->collectMethods(ar, cur, false);
          inheritedMagicMethods(cls);
          funcs.insert(cur.begin(), cur.end());
        }

        m_derivesFromRedeclaring = Derivation::Redeclaring;

        setVolatile();
      } else {
        derivedMagicMethods(super);
        super->collectMethods(ar, funcs, false);
        inheritedMagicMethods(super);
        if (super->derivesFromRedeclaring() == Derivation::Redeclaring) {
          m_derivesFromRedeclaring = Derivation::Redeclaring;
          setVolatile();
        } else if (super->isVolatile()) {
          setVolatile();
        }
      }
    } else {
      Compiler::Error(Compiler::UnknownBaseClass, m_stmt, base);
      if (base == m_parent) {
        ar->declareUnknownClass(m_parent);
        m_derivesFromRedeclaring = Derivation::Redeclaring;
        setVolatile();
      } else {
        /*
         * TODO(#3685260): this should not be removing interfaces from
         * the base list.
         */
        if (isInterface()) {
          m_derivesFromRedeclaring = Derivation::Redeclaring;
        }
        m_bases.erase(m_bases.begin() + i);
        n--;
        i--;
      }
    }
  }
}

void ClassScope::importTraitProperties(AnalysisResultPtr ar) {

  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    ClassScopePtr tCls = ar->findClass(m_usedTraitNames[i]);
    if (!tCls) continue;
    ClassStatementPtr tStmt =
      dynamic_pointer_cast<ClassStatement>(tCls->getStmt());
    StatementListPtr tStmts = tStmt->getStmts();
    if (!tStmts) continue;
    for (int s = 0; s < tStmts->getCount(); s++) {
      ClassVariablePtr prop =
        dynamic_pointer_cast<ClassVariable>((*tStmts)[s]);
      if (prop) {
        ClassVariablePtr cloneProp = dynamic_pointer_cast<ClassVariable>(
          dynamic_pointer_cast<ClassStatement>(m_stmt)->addClone(prop));
        cloneProp->resetScope(shared_from_this());
        cloneProp->addTraitPropsToScope(ar,
                      dynamic_pointer_cast<ClassScope>(shared_from_this()));
      }
    }
  }
}

MethodStatementPtr
ClassScope::importTraitMethod(const TraitMethod&  traitMethod,
                              AnalysisResultPtr   ar,
                              std::string         methName) {
  MethodStatementPtr meth = traitMethod.method;
  std::string origMethName = traitMethod.originalName;
  ModifierExpressionPtr modifiers = traitMethod.modifiers;

  MethodStatementPtr cloneMeth = dynamic_pointer_cast<MethodStatement>(
    dynamic_pointer_cast<ClassStatement>(m_stmt)->addClone(meth));
  cloneMeth->setOriginalName(origMethName);
  // Note: keep previous modifiers if none specified when importing the trait
  if (modifiers && modifiers->getCount()) {
    cloneMeth->setModifiers(modifiers);
  }
  FunctionScopePtr funcScope = meth->getFunctionScope();

  // Trait method typehints, self and parent, need to be converted
  ClassScopePtr cScope = dynamic_pointer_cast<ClassScope>(shared_from_this());
  cloneMeth->fixupSelfAndParentTypehints( cScope );

  auto cloneFuncScope =
    std::make_shared<HPHP::FunctionScope>(
      funcScope, ar, origMethName, cloneMeth,
      cloneMeth->getModifiers(), cScope->isUserClass());
  cloneMeth->resetScope(cloneFuncScope);
  cloneFuncScope->setOuterScope(shared_from_this());
  informClosuresAboutScopeClone(cloneMeth, cloneFuncScope, ar);

  cloneMeth->addTraitMethodToScope(ar,
               dynamic_pointer_cast<ClassScope>(shared_from_this()));

  // Preserve original filename (as this varies per-function and not per-unit
  // in the case of methods imported from flattened traits)
  cloneMeth->setOriginalFilename(meth->getFileScope()->getName());

  return cloneMeth;
}

void ClassScope::informClosuresAboutScopeClone(
    ConstructPtr root,
    FunctionScopePtr outerScope,
    AnalysisResultPtr ar) {

  if (!root) {
    return;
  }

  for (int i = 0; i < root->getKidCount(); i++) {
    ConstructPtr cons = root->getNthKid(i);
    ClosureExpressionPtr closure =
      dynamic_pointer_cast<ClosureExpression>(cons);

    if (!closure) {
      informClosuresAboutScopeClone(cons, outerScope, ar);
      continue;
    }

    FunctionStatementPtr func = closure->getClosureFunction();
    HPHP::FunctionScopePtr funcScope = func->getFunctionScope();
    assert(funcScope->isClosure());
    funcScope->addClonedTraitOuterScope(outerScope);
    // Don't need to recurse
  }
}

void ClassScope::addClassRequirement(const string &requiredName,
                                     bool isExtends) {
  assert(isTrait() || (isInterface() && isExtends)
         // when flattening traits, their requirements get flattened
         || Option::WholeProgram);
  if (isExtends) {
    m_requiredExtends.insert(requiredName);
  } else {
    m_requiredImplements.insert(requiredName);
  }
}

void ClassScope::importClassRequirements(AnalysisResultPtr ar,
                                         ClassScopePtr trait) {
  /* Defer enforcement of requirements until the creation of the class
   * happens at runtime. */
  for (auto const& req : trait->getClassRequiredExtends()) {
    addClassRequirement(req, true);
  }
  for (auto const& req : trait->getClassRequiredImplements()) {
    addClassRequirement(req, false);
  }
}

bool ClassScope::hasMethod(const string &methodName) const {
  return m_functions.find(methodName) != m_functions.end();
}

///////////////////////////////////////////////////////////////////////////////
// Trait flattening.

namespace {

void findTraitMethodsToImport(AnalysisResultPtr ar,
                              ClassScopePtr trait,
                              ClassScope::TMIData& tmid) {
  assert(Option::WholeProgram);
  ClassStatementPtr tStmt =
    dynamic_pointer_cast<ClassStatement>(trait->getStmt());
  StatementListPtr tStmts = tStmt->getStmts();
  if (!tStmts) return;

  for (int s = 0; s < tStmts->getCount(); s++) {
    MethodStatementPtr meth =
      dynamic_pointer_cast<MethodStatement>((*tStmts)[s]);
    if (meth) {
      ClassScope::TraitMethod traitMethod(trait, meth,
                                          ModifierExpressionPtr(),
                                          MethodStatementPtr());
      tmid.add(traitMethod, meth->getOriginalName());
    }
  }
}

MethodStatementPtr findTraitMethodImpl(AnalysisResultPtr ar,
                                       ClassScopePtr trait,
                                       const std::string& methodName,
                                       std::set<ClassScopePtr>& visitedTraits) {
  if (visitedTraits.find(trait) != visitedTraits.end()) {
    return MethodStatementPtr();
  }
  visitedTraits.insert(trait);

  ClassStatementPtr tStmt =
    dynamic_pointer_cast<ClassStatement>(trait->getStmt());
  StatementListPtr tStmts = tStmt->getStmts();

  // Look in the current trait.
  for (int s = 0; s < tStmts->getCount(); s++) {
    MethodStatementPtr meth =
      dynamic_pointer_cast<MethodStatement>((*tStmts)[s]);
    if (meth) { // handle methods
      if (meth->isNamed(methodName)) {
        return meth;
      }
    }
  }

  // Look into children traits.
  for (int s = 0; s < tStmts->getCount(); s++) {
    UseTraitStatementPtr useTraitStmt =
      dynamic_pointer_cast<UseTraitStatement>((*tStmts)[s]);
    if (useTraitStmt) {
      vector<string> usedTraits;
      useTraitStmt->getUsedTraitNames(usedTraits);
      for (unsigned i = 0; i < usedTraits.size(); i++) {
        MethodStatementPtr foundMethod =
          findTraitMethodImpl(ar, ar->findClass(usedTraits[i]),
                              methodName, visitedTraits);
        if (foundMethod) return foundMethod;
      }
    }
  }
  return MethodStatementPtr(); // not found
}

}

void ClassScope::TMIOps::addTraitAlias(ClassScope* cs,
                                       TraitAliasStatementPtr stmt,
                                       ClassScopePtr traitCls) {
  auto const& traitName = stmt->getTraitName();
  auto const& origMethName = stmt->getMethodName();
  auto const& newMethName = stmt->getNewMethodName();

  auto origName = traitName.empty() ? "(null)" : traitName;
  origName += "::" + origMethName;

  cs->m_traitAliases.push_back(std::make_pair(newMethName, origName));
}

ClassScopePtr
ClassScope::TMIOps::findSingleTraitWithMethod(ClassScope* cs,
                                              const std::string& origMethName) {
  ClassScopePtr trait{};

  for (auto const& name : cs->m_usedTraitNames) {
    ClassScopePtr tCls = cs->getContainingProgram()->findClass(name);
    if (!tCls) continue;

    if (tCls->hasMethod(origMethName)) {
      if (trait) {
        // More than one trait contains the method.
        return ClassScopePtr();
      }
      trait = tCls;
    }
  }
  return trait;
}

ClassScopePtr
ClassScope::TMIOps::findTraitClass(ClassScope* cs,
                                   const std::string& traitName) {
  return cs->getContainingProgram()->findClass(traitName);
}

MethodStatementPtr
ClassScope::TMIOps::findTraitMethod(ClassScope* cs,
                                    ClassScopePtr traitCls,
                                    const std::string& origMethName) {
  std::set<ClassScopePtr> visitedTraits;
  return findTraitMethodImpl(cs->getContainingProgram(), traitCls,
                             origMethName, visitedTraits);
}

void ClassScope::applyTraitRules(TMIData& tmid) {
  ClassStatementPtr classStmt =
    dynamic_pointer_cast<ClassStatement>(getStmt());
  assert(classStmt);

  StatementListPtr stmts = classStmt->getStmts();
  if (!stmts) return;

  for (int s = 0; s < stmts->getCount(); s++) {
    StatementPtr stmt = (*stmts)[s];

    UseTraitStatementPtr useStmt =
      dynamic_pointer_cast<UseTraitStatement>(stmt);
    if (!useStmt) continue;

    StatementListPtr rules = useStmt->getStmts();
    for (int r = 0; r < rules->getCount(); r++) {
      StatementPtr rule = (*rules)[r];
      TraitPrecStatementPtr precStmt =
        dynamic_pointer_cast<TraitPrecStatement>(rule);
      if (precStmt) {
        tmid.applyPrecRule(precStmt);
      } else {
        TraitAliasStatementPtr aliasStmt =
          dynamic_pointer_cast<TraitAliasStatement>(rule);
        assert(aliasStmt);
        tmid.applyAliasRule(aliasStmt, this);
      }
    }
  }
}

void ClassScope::importUsedTraits(AnalysisResultPtr ar) {
  // Trait flattening is supposed to happen only when we have awareness of the
  // whole program.
  assert(Option::WholeProgram);

  if (m_traitStatus == FLATTENED) return;
  if (m_traitStatus == BEING_FLATTENED) {
    getStmt()->analysisTimeFatal(
      Compiler::CyclicDependentTraits,
      "Cyclic dependency between traits involving %s",
      getScopeName().c_str()
    );
    return;
  }
  if (m_usedTraitNames.size() == 0) {
    m_traitStatus = FLATTENED;
    return;
  }
  m_traitStatus = BEING_FLATTENED;

  m_numDeclMethods = m_functionsVec.size();

  // First, make sure that parent classes have their traits imported.
  if (!m_parent.empty()) {
    ClassScopePtr parent = ar->findClass(m_parent);
    if (parent) {
      parent->importUsedTraits(ar);
    }
  }

  TMIData tmid;

  if (isTrait()) {
    for (auto const& req : getClassRequiredExtends()) {
      ClassScopePtr rCls = ar->findClass(req);
      if (!rCls || rCls->isFinal() || rCls->isInterface()) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_BAD_REQ_EXTENDS,
          m_scopeName.c_str(),
          req.c_str(),
          req.c_str()
        );
      }
    }
    for (auto const& req : getClassRequiredImplements()) {
      ClassScopePtr rCls = ar->findClass(req);
      if (!rCls || !(rCls->isInterface())) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_BAD_REQ_IMPLEMENTS,
          m_scopeName.c_str(),
          req.c_str(),
          req.c_str()
        );
      }
    }
  }

  // Find trait methods to be imported.
  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    ClassScopePtr tCls = ar->findClass(m_usedTraitNames[i]);
    if (!tCls || !(tCls->isTrait())) {
      setAttribute(UsesUnknownTrait); // XXX: is this useful ... for anything?
      getStmt()->analysisTimeFatal(
        Compiler::UnknownTrait,
        Strings::TRAITS_UNKNOWN_TRAIT,
        m_usedTraitNames[i].c_str()
      );
    }

    // First, make sure the used trait is flattened.
    tCls->importUsedTraits(ar);

    findTraitMethodsToImport(ar, tCls, tmid);

    // Import any interfaces implemented.
    tCls->getInterfaces(ar, m_bases, /* recursive */ false);

    importClassRequirements(ar, tCls);
  }

  // Apply rules.
  applyTraitRules(tmid);

  // Remove methods declared on the current class from the trait import list;
  // the class methods take precedence.
  for (auto const& methName : tmid.methodNames()) {
    if (findFunction(ar, methName, false /* recursive */,
                     false /* exclIntfBase */)) {
      // This does not affect the methodNames() vector.
      tmid.erase(methName);
    }
  }

  auto traitMethods = tmid.finish(this);

  std::map<string, MethodStatementPtr, stdltistr> importedTraitMethods;
  std::vector<std::pair<string,const TraitMethod*>> importedTraitsWithOrigName;

  // Actually import the methods.
  for (auto const& mdata : traitMethods) {
    if ((mdata.tm.modifiers ? mdata.tm.modifiers
                            : mdata.tm.method->getModifiers()
        )->isAbstract()) {
      // Skip abstract methods, if the method already exists in the class.
      if (findFunction(ar, mdata.name, true) ||
          importedTraitMethods.count(mdata.name)) {
        continue;
      }
    }

    auto sourceName = mdata.tm.ruleStmt
      ? ((TraitAliasStatement*)mdata.tm.ruleStmt.get())->getMethodName()
      : mdata.name;

    importedTraitMethods[sourceName] = MethodStatementPtr();
    importedTraitsWithOrigName.push_back(
      std::make_pair(sourceName, &mdata.tm));
  }

  // Make sure there won't be 2 constructors after importing
  auto traitConstruct = importedTraitMethods.count("__construct");
  auto traitName = importedTraitMethods.count(getScopeName());
  auto classConstruct = m_functions.count("__construct");
  auto className = m_functions.count(getScopeName());
  if ((traitConstruct && traitName) ||
      (traitConstruct && className) ||
      (classConstruct && traitName)) {
    getStmt()->analysisTimeFatal(
      Compiler::InvalidDerivation,
      "%s has colliding constructor definitions coming from traits",
      getScopeName().c_str()
    );
  }

  for (auto const& traitPair : importedTraitsWithOrigName) {
    auto traitMethod = traitPair.second;

    MethodStatementPtr newMeth = importTraitMethod(
      *traitMethod,
      ar,
      traitMethod->originalName
    );
  }

  // Import trait properties
  importTraitProperties(ar);

  m_traitStatus = FLATTENED;
}

///////////////////////////////////////////////////////////////////////////////

bool ClassScope::usesTrait(const string &traitName) const {
  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    if (traitName == m_usedTraitNames[i]) {
      return true;
    }
  }
  return false;
}

bool ClassScope::needsInvokeParent(AnalysisResultConstPtr ar,
                                   bool considerSelf /* = true */) {
  // check all functions this class has
  if (considerSelf) {
    for (FunctionScopePtrVec::const_iterator iter =
           m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
      if ((*iter)->isPrivate()) return true;
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
  for (std::string base_i: m_bases) {
    if (strcasecmp(base_i.c_str(), base.c_str()) == 0) return true;
  }
  return false;
}

bool ClassScope::derivesFrom(AnalysisResultConstPtr ar,
                             const std::string &base,
                             bool strict, bool def) const {

  if (derivesDirectlyFrom(base)) return true;

  for (std::string base_i: m_bases) {
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
  if (cls1->isNamed(cls2->getScopeName()))  return cls1;
  if (cls1->derivesFrom(ar, cn2, true, false)) return cls2;
  if (cls2->derivesFrom(ar, cn1, true, false)) return cls1;

  // walk up the class hierarchy.
  for (const std::string &base1: cls1->m_bases) {
    for (const std::string &base2: cls2->m_bases) {
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
                                          bool exclIntfBase /* = false */) {
  auto iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    assert(iter->second);
    return iter->second;
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
          m_derivesFromRedeclaring = Derivation::Redeclaring;
          break;
        }
        continue;
      }
      FunctionScopePtr func =
        super->findFunction(ar, name, true, exclIntfBase);
      if (func) return func;
    }
  }
  if (!Option::AllDynamic &&
      derivesFromRedeclaring() == Derivation::Redeclaring) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

FunctionScopePtr ClassScope::findConstructor(AnalysisResultConstPtr ar,
                                             bool recursive) {
  string name;
  if (classNameCtor()) {
    name = getScopeName();
  } else {
    name = "__construct";
  }
  auto iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    assert(iter->second);
    return iter->second;
  }

  // walk up
  if (recursive && derivesFromRedeclaring() != Derivation::Redeclaring) {
    ClassScopePtr super = ar->findClass(m_parent);
    if (super) {
      FunctionScopePtr func = super->findConstructor(ar, true);
      if (func) return func;
    }
  }
  if (!Option::AllDynamic &&
      derivesFromRedeclaring() == Derivation::Redeclaring) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

void ClassScope::setStaticDynamic(AnalysisResultConstPtr ar) {
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    FunctionScopePtr fs = *iter;
    if (fs->isStatic()) fs->setDynamic();
  }
  if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == Derivation::Redeclaring) {
      const ClassScopePtrVec &parents = ar->findRedeclaredClasses(m_parent);
      for (ClassScopePtr cl: parents) {
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
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functions.find(name);
  if (iter != m_functions.end()) {
    FunctionScopePtr fs = iter->second;
    fs->setDynamic();
  } else if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == Derivation::Redeclaring) {
      const ClassScopePtrVec &parents = ar->findRedeclaredClasses(m_parent);
      for (ClassScopePtr cl: parents) {
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
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    (*iter)->setSystem();
  }
}

bool ClassScope::needLazyStaticInitializer() {
  return getVariables()->getAttribute(VariableTable::ContainsDynamicStatic) ||
    getConstants()->hasDynamic();
}

bool ClassScope::hasConst(const string &name) const {
  const Symbol *sym = m_constants->getSymbol(name);
  assert(!sym || sym->isPresent());
  return sym;
}

Symbol *ClassScope::findProperty(ClassScopePtr &cls,
                                 const string &name,
                                 AnalysisResultConstPtr ar) {
  return getVariables()->findProperty(cls, name, ar);
}

void ClassScope::getInterfaces(AnalysisResultConstPtr ar,
                               std::vector<std::string> &names,
                               bool recursive /* = true */) const {
  ClassScope *self = const_cast<ClassScope*>(this);
  if (recursive && !m_parent.empty()) {
    ClassScopePtr cls(ar->findClass(m_parent));
    if (cls && cls->isRedeclaring()) {
      cls = self->findExactClass(cls);
    }
    if (cls) cls->getInterfaces(ar, names, true);
  }
  if (!m_bases.empty()) {
    for (auto const& base : m_bases) {
      if (base == m_parent) continue;
      ClassScopePtr cls(ar->findClass(base));
      if (cls && cls->isRedeclaring()) {
        cls = self->findExactClass(cls);
      }
      if (cls && recursive) {
        names.push_back(cls ? cls->getDocName() : base);
        cls->getInterfaces(ar, names, true);
      } else if (std::find_if(names.begin(), names.end(),
                              [base](std::string& b) {
                                return string_eqstri()(base,b);
                              }) == names.end()) {
        names.push_back(base);
      }
    }
  }
}

ClassScopePtr ClassScope::getParentScope(AnalysisResultConstPtr ar) const {
  if (m_parent.empty()) return ClassScopePtr();
  return ar->findClass(m_parent);
}

void ClassScope::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::MapStream ms(out);
  std::map<string, int> propMap;
  std::set<string> names;
  m_variables->getNames(names);
  for (const string& name: names) {
    int pm = 0;
    if (m_variables->isPublic(name)) pm |= ClassScope::Public;
    else if (m_variables->isPrivate(name)) pm |= ClassScope::Private;
    else if (m_variables->isProtected(name)) pm |= ClassScope::Protected;
    if (m_variables->isStatic(name)) pm |= ClassScope::Static;
    propMap[name] = pm;
  }
  names.clear();

  // What's a mod again?
  ms.add("attributes", m_attribute)
    .add("kind", (int) m_kindOf)
    .add("parent", m_parent)
    .add("bases", m_bases)
    .add("properties", propMap)
    .add("functions", m_functions);

  ms.done();
}

static inline string GetDocName(AnalysisResultPtr ar,
                                BlockScopeRawPtr scope,
                                const string &name) {
  ClassScopePtr c(ar->findClass(name));
  if (c && c->isRedeclaring()) {
    ClassScopePtr exact(scope->findExactClass(c));
    return exact ?
      exact->getDocName() :
      c->getScopeName(); // if we can't tell which redec class,
                            // then don't use the redec name
  }
  // TODO: pick a better way of signaling unknown?
  return c ? c->getDocName() : "UnknownClass";
}

class GetDocNameFunctor {
public:
  GetDocNameFunctor(AnalysisResultPtr ar, BlockScopeRawPtr scope) :
    m_ar(ar), m_scope(scope) {}
  inline string operator()(const string &name) const {
    return GetDocName(m_ar, m_scope, name);
  }
private:
  AnalysisResultPtr m_ar;
  BlockScopeRawPtr  m_scope;
};

void ClassScope::serialize(JSON::DocTarget::OutputStream &out) const {
  // TODO(stephentu): fix this hack
  ClassScopeRawPtr self(const_cast<ClassScope*>(this));

  JSON::DocTarget::MapStream ms(out);

  ms.add("name", getDocName());
  ms.add("line", getStmt() ? getStmt()->line0() : 0);
  ms.add("docs", m_docComment);

  ms.add("parent");
  if (m_parent.empty()) {
    out << JSON::Null();
  } else {
    out << GetDocName(out.analysisResult(), self, m_parent);
  }

  vector<string> ifaces;
  getInterfaces(out.analysisResult(), ifaces, true);
  vector<string> origIfaces;
  origIfaces.resize(ifaces.size());
  transform(ifaces.begin(), ifaces.end(), origIfaces.begin(),
            GetDocNameFunctor(out.analysisResult(), self));
  ms.add("interfaces", origIfaces);

  int mods = 0;
  switch (m_kindOf) {
    case KindOf::AbstractClass: mods |= ClassInfo::IsAbstract; break;
    case KindOf::Enum:
    case KindOf::FinalClass:
      mods |= ClassInfo::IsFinal; break;
    case KindOf::UtilClass:
      mods |= ClassInfo::IsFinal | ClassInfo::IsAbstract; break;
    case KindOf::Interface:     mods |= ClassInfo::IsInterface; break;
    case KindOf::Trait:         mods |= ClassInfo::IsTrait; break;
    case KindOf::ObjectClass:
      break;
  }
  ms.add("modifiers", mods);

  FunctionScopePtrVec funcs;
  getFunctionsFlattened(0, funcs);
  ms.add("methods", funcs);

  vector<Symbol*> rawSymbols;
  getVariables()->getSymbols(rawSymbols, true);
  vector<SymClassVarWrapper> wrappedSymbols;
  for (vector<Symbol*>::iterator it = rawSymbols.begin();
       it != rawSymbols.end(); ++it) {
    wrappedSymbols.push_back(SymClassVarWrapper(*it));
  }
  ms.add("properties", wrappedSymbols);

  // TODO: constants

  ms.done();
}

bool ClassScope::hasProperty(const string &name) const {
  const Symbol *sym = m_variables->getSymbol(name);
  assert(!sym || sym->isPresent());
  return sym;
}

void ClassScope::setRedeclaring(AnalysisResultConstPtr ar, int redecId) {
  if (isTrait()) {
    Compiler::Error(Compiler::RedeclaredTrait, m_stmt);
  }
  m_redeclaring = redecId;
  setVolatile(); // redeclared class is also volatile
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    (*iter)->setDynamic();
  }
}

bool ClassScope::addFunction(AnalysisResultConstPtr ar,
                             FileScopeRawPtr fileScope,
                             FunctionScopePtr funcScope) {
  FunctionScopePtr &func = m_functions[funcScope->getScopeName()];
  if (func) {
    func->getStmt()->parseTimeFatal(fileScope,
                                    Compiler::DeclaredMethodTwice,
                                    "Redeclared method %s::%s",
                                    getScopeName().c_str(),
                                    func->getScopeName().c_str());
  }
  func = funcScope;
  m_functionsVec.push_back(funcScope);
  return true;
}

bool ClassScope::canSkipCreateMethod(AnalysisResultConstPtr ar) const {
  // create() is not necessary if
  // 1) not inheriting from any class
  // 2) no constructor defined (__construct or class name)
  // 3) no init() defined

  if (derivesFromRedeclaring() == Derivation::Redeclaring ||
      getAttribute(HasConstructor) ||
      getAttribute(ClassNameConstructor) ||
      needsInitMethod()) {
    return false;
  }

  if (!m_parent.empty()) {
    ClassScopePtr parent = getParentScope(ar);
    if (parent) return parent->canSkipCreateMethod(ar);
  }

  return true;
}
