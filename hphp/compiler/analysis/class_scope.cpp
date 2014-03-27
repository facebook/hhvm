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
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/util/text-util.h"

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
#include <set>
#include <utility>
#include <vector>

using namespace HPHP;
using std::map;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(KindOf kindOf, const std::string &name,
                       const std::string &parent,
                       const vector<string> &bases,
                       const std::string &docComment, StatementPtr stmt,
                       const std::vector<UserAttributePtr> &attrs)
  : BlockScope(name, docComment, stmt, BlockScope::ClassScope),
    m_parent(parent), m_bases(bases), m_attribute(0), m_redeclaring(-1),
    m_kindOf(kindOf), m_derivesFromRedeclaring(Derivation::Normal),
    m_traitStatus(NOT_FLATTENED), m_volatile(false),
    m_persistent(false), m_derivedByDynamic(false),
    m_needsCppCtor(false), m_needsInit(true), m_knownBases(0) {

  m_dynamic = Option::IsDynamicClass(m_name);

  // dynamic class is also volatile
  m_volatile = Option::AllVolatile || m_dynamic;

  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(Compiler::DeclaredAttributeTwice,
                               "Redeclared attribute %s",
                               attrs[i]->getName().c_str());
    }
    m_userAttributes[attrs[i]->getName()] = attrs[i]->getExp();
  }

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

// System
ClassScope::ClassScope(AnalysisResultPtr ar,
                       const std::string &name, const std::string &parent,
                       const std::vector<std::string> &bases,
                       const FunctionScopePtrVec &methods)
  : BlockScope(name, "", StatementPtr(), BlockScope::ClassScope),
    m_parent(parent), m_bases(bases),
    m_attribute(0), m_redeclaring(-1),
    m_kindOf(KindOfObjectClass), m_derivesFromRedeclaring(Derivation::Normal),
    m_traitStatus(NOT_FLATTENED), m_dynamic(false),
    m_volatile(false), m_persistent(false),
    m_derivedByDynamic(false), m_needsCppCtor(false),
    m_needsInit(true), m_knownBases(0) {
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

  assert(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

const std::string &ClassScope::getOriginalName() const {
  if (m_stmt) {
    return dynamic_pointer_cast<InterfaceStatement>(m_stmt)->
      getOriginalName();
  }
  return m_originalName;
}

std::string ClassScope::getDocName() const {
  string name = getOriginalName();
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
  seen.insert(m_name);

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

    ClassScopePtrVec parents = ar->findClasses(toLower(base));
    for (unsigned int j = 0; j < parents.size(); j++) {
      parents[j]->checkDerivation(ar, seen);
    }
  }

  seen.erase(m_name);
}

void ClassScope::collectMethods(AnalysisResultPtr ar,
                                StringToFunctionScopePtrMap &funcs,
                                bool collectPrivate) {
  // add all functions this class has
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    const FunctionScopePtr &fs = *iter;
    if (!collectPrivate && fs->isPrivate()) continue;

    FunctionScopePtr &func = funcs[fs->getName()];
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
          cls->getVariables()->
            forceVariants(ar, VariableTable::AnyNonPrivateVars);
        }

        m_derivesFromRedeclaring = Derivation::Redeclaring;
        getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars,
                                      false);
        getVariables()->setAttribute(VariableTable::NeedGlobalPointer);

        setVolatile();
      } else {
        derivedMagicMethods(super);
        super->collectMethods(ar, funcs, false);
        inheritedMagicMethods(super);
        if (super->derivesFromRedeclaring() == Derivation::Redeclaring) {
          m_derivesFromRedeclaring = Derivation::Redeclaring;
          getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars);
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
        getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
        getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars);
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
        cloneProp->resetScope(shared_from_this(), true);
        cloneProp->addTraitPropsToScope(ar,
                      dynamic_pointer_cast<ClassScope>(shared_from_this()));
      }
    }
  }
}

MethodStatementPtr
ClassScope::importTraitMethod(const TraitMethod&  traitMethod,
                              AnalysisResultPtr   ar,
                              string              methName,
                              const std::map<string, MethodStatementPtr>&
                              importedTraitMethods) {
  MethodStatementPtr meth = traitMethod.m_method;
  string origMethName = traitMethod.m_originalName;
  ModifierExpressionPtr modifiers = traitMethod.m_modifiers;

  MethodStatementPtr cloneMeth = dynamic_pointer_cast<MethodStatement>(
    dynamic_pointer_cast<ClassStatement>(m_stmt)->addClone(meth));
  cloneMeth->setName(methName);
  cloneMeth->setOriginalName(origMethName);
  // Note: keep previous modifiers if none specified when importing the trait
  if (modifiers && modifiers->getCount()) {
    cloneMeth->setModifiers(modifiers);
  }
  FunctionScopePtr funcScope = meth->getFunctionScope();

  // Trait method typehints, self and parent, need to be converted
  ClassScopePtr cScope = dynamic_pointer_cast<ClassScope>(shared_from_this());
  cloneMeth->fixupSelfAndParentTypehints( cScope );

  FunctionScopePtr cloneFuncScope
    (new HPHP::FunctionScope(funcScope, ar, methName, origMethName, cloneMeth,
                             cloneMeth->getModifiers(), cScope->isUserClass()));
  cloneMeth->resetScope(cloneFuncScope, true);
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


void ClassScope::addImportTraitMethod(const TraitMethod &traitMethod,
                                      const string &methName) {
  m_importMethToTraitMap[methName].push_back(traitMethod);
}

void ClassScope::addTraitRequirement(const string &requiredName,
                                     bool isExtends) {
  assert(isTrait());

  if (isExtends) {
    m_traitRequiredExtends.insert(requiredName);
  } else {
    m_traitRequiredImplements.insert(requiredName);
  }
}

void
ClassScope::setImportTraitMethodModifiers(const string &methName,
                                          ClassScopePtr traitCls,
                                          ModifierExpressionPtr modifiers) {
  TraitMethodList &methList = m_importMethToTraitMap[methName];

  for (TraitMethodList::iterator iter = methList.begin();
       iter != methList.end(); iter++) {
    if (iter->m_trait == traitCls) {
      iter->m_modifiers = modifiers;
      return;
    }
  }
}

MethodStatementPtr
ClassScope::findTraitMethod(AnalysisResultPtr ar,
                            ClassScopePtr trait,
                            const string &methodName,
                            std::set<ClassScopePtr> &visitedTraits) {
  if (visitedTraits.find(trait) != visitedTraits.end()) {
    return MethodStatementPtr();
  }
  visitedTraits.insert(trait);

  ClassStatementPtr tStmt =
    dynamic_pointer_cast<ClassStatement>(trait->getStmt());
  StatementListPtr tStmts = tStmt->getStmts();

  // Look in the current trait
  for (int s = 0; s < tStmts->getCount(); s++) {
    MethodStatementPtr meth =
      dynamic_pointer_cast<MethodStatement>((*tStmts)[s]);
    if (meth) {    // Handle methods
      if (meth->getName() == methodName) {
        return meth;
      }
    }
  }

  // Look into children traits
  for (int s = 0; s < tStmts->getCount(); s++) {
    UseTraitStatementPtr useTraitStmt =
      dynamic_pointer_cast<UseTraitStatement>((*tStmts)[s]);
    if (useTraitStmt) {
      vector<string> usedTraits;
      useTraitStmt->getUsedTraitNames(usedTraits);
      for (unsigned i = 0; i < usedTraits.size(); i++) {
        MethodStatementPtr foundMethod =
          findTraitMethod(ar, ar->findClass(usedTraits[i]), methodName,
                          visitedTraits);
        if (foundMethod) return foundMethod;
      }
    }
  }
  return MethodStatementPtr(); // not found
}

void ClassScope::findTraitMethodsToImport(AnalysisResultPtr ar,
                                          ClassScopePtr trait) {
  assert(Option::WholeProgram);
  ClassStatementPtr tStmt =
    dynamic_pointer_cast<ClassStatement>(trait->getStmt());
  StatementListPtr tStmts = tStmt->getStmts();
  if (!tStmts) return;

  for (int s = 0; s < tStmts->getCount(); s++) {
    MethodStatementPtr meth =
      dynamic_pointer_cast<MethodStatement>((*tStmts)[s]);
    if (meth) {
      TraitMethod traitMethod(trait, meth, ModifierExpressionPtr(),
                              MethodStatementPtr());
      addImportTraitMethod(traitMethod, meth->getName());
    }
  }
}

void ClassScope::importTraitRequirements(AnalysisResultPtr ar,
                                         ClassScopePtr trait) {
  if (isTrait()) {
    for (auto const& req : trait->getTraitRequiredExtends()) {
      addTraitRequirement(req, true);
    }
    for (auto const& req : trait->getTraitRequiredImplements()) {
      addTraitRequirement(req, false);
    }
  } else {
    for (auto const& req : trait->getTraitRequiredExtends()) {
      if (!derivesFrom(ar, req, true, false)) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_REQ_EXTENDS,
          m_originalName.c_str(),
          req.c_str(),
          trait->getOriginalName().c_str(),
          "use"
        );
      }
    }
    for (auto const& req : trait->getTraitRequiredImplements()) {
      if (!derivesFrom(ar, req, true, false)) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_REQ_IMPLEMENTS,
          m_originalName.c_str(),
          req.c_str(),
          trait->getOriginalName().c_str(),
          "use"
        );
      }
    }
  }
}

void ClassScope::applyTraitPrecRule(TraitPrecStatementPtr stmt) {
  assert(Option::WholeProgram);
  const string methodName = toLower(stmt->getMethodName());
  const string selectedTraitName = toLower(stmt->getTraitName());
  std::set<string> otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);

  map<string,TraitMethodList>::iterator methIter =
    m_importMethToTraitMap.find(methodName);
  if (methIter == m_importMethToTraitMap.end()) {
    Compiler::Error(Compiler::UnknownObjectMethod, stmt);
    return;
  }
  bool foundSelectedTrait = false;

  TraitMethodList &methList = methIter->second;
  for (TraitMethodList::iterator nextTraitIter = methList.begin();
       nextTraitIter != methList.end(); ) {
    TraitMethodList::iterator traitIter = nextTraitIter++;
    string availTraitName = traitIter->m_trait->getName();
    if (availTraitName == selectedTraitName) {
      foundSelectedTrait = true;
    } else {
      if (otherTraitNames.find(availTraitName) != otherTraitNames.end()) {
        otherTraitNames.erase(availTraitName);
        methList.erase(traitIter);
      }
    }
  }

  // Report error if didn't find the selected trait
  if (!foundSelectedTrait) {
    stmt->analysisTimeFatal(
      Compiler::UnknownTrait,
      Strings::TRAITS_UNKNOWN_TRAIT,
      selectedTraitName.c_str()
    );
  }

  // Sanity checking: otherTraitNames should be empty now
  if (otherTraitNames.size()) {
    stmt->analysisTimeFatal(
      Compiler::UnknownTrait,
      Strings::TRAITS_UNKNOWN_TRAIT,
      selectedTraitName.c_str()
    );
  }
}

bool ClassScope::hasMethod(const string &methodName) const {
  return m_functions.find(methodName) != m_functions.end();
}

ClassScopePtr
ClassScope::findSingleTraitWithMethod(AnalysisResultPtr ar,
                                      const string &methodName) const {
  assert(Option::WholeProgram);
  ClassScopePtr trait = ClassScopePtr();

  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    ClassScopePtr tCls = ar->findClass(m_usedTraitNames[i]);
    if (!tCls) continue;

    if (tCls->hasMethod(methodName)) {
      if (trait) { // more than one trait contains method
        return ClassScopePtr();
      }
      trait = tCls;
    }
  }
  return trait;
}

void ClassScope::addTraitAlias(TraitAliasStatementPtr aliasStmt) {
  assert(Option::WholeProgram);
  const string &traitName = aliasStmt->getTraitName();
  const string &origMethName = aliasStmt->getMethodName();
  const string &newMethName = aliasStmt->getNewMethodName();
  string origName = traitName.empty() ? "(null)" : traitName;
  origName += "::" + origMethName;
  m_traitAliases.push_back(std::pair<string, string>(newMethName, origName));
}

void ClassScope::applyTraitAliasRule(AnalysisResultPtr ar,
                                     TraitAliasStatementPtr stmt) {
  assert(Option::WholeProgram);
  const string traitName = toLower(stmt->getTraitName());
  const string origMethName = toLower(stmt->getMethodName());
  const string newMethName = toLower(stmt->getNewMethodName());

  // Get the trait's "class"
  ClassScopePtr traitCls;
  if (traitName.empty()) {
    traitCls = findSingleTraitWithMethod(ar, origMethName);
  } else {
    traitCls = ar->findClass(traitName);
  }
  if (!traitCls || !(traitCls->isTrait())) {
    stmt->analysisTimeFatal(
      Compiler::UnknownTrait,
      Strings::TRAITS_UNKNOWN_TRAIT,
      traitName.empty() ? origMethName.c_str() : traitName.c_str()
    );
  }

  // Keep record of alias rule
  addTraitAlias(stmt);

  // Get the method
  std::set<ClassScopePtr> visitedTraits;
  MethodStatementPtr methStmt = findTraitMethod(ar, traitCls, origMethName,
                                                visitedTraits);
  if (!methStmt) {
    stmt->analysisTimeFatal(
      Compiler::UnknownTraitMethod,
      Strings::TRAITS_UNKNOWN_TRAIT_METHOD, origMethName.c_str()
    );
  }

  if (origMethName == newMethName) {
    setImportTraitMethodModifiers(origMethName, traitCls, stmt->getModifiers());
  }
  else {
    // Insert renamed entry into the set of methods to be imported
    TraitMethod traitMethod(traitCls, methStmt, stmt->getModifiers(), stmt,
                            stmt->getNewMethodName());
    addImportTraitMethod(traitMethod, newMethName);
  }
}

void ClassScope::applyTraitRules(AnalysisResultPtr ar) {
  assert(Option::WholeProgram);
  ClassStatementPtr classStmt = dynamic_pointer_cast<ClassStatement>(getStmt());
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
        applyTraitPrecRule(precStmt);
      } else {
        TraitAliasStatementPtr aliasStmt =
          dynamic_pointer_cast<TraitAliasStatement>(rule);
        assert(aliasStmt);
        applyTraitAliasRule(ar, aliasStmt);
      }
    }
  }
}

// This method removes trait abstract methods that are either:
//   1) implemented by other traits
//   2) duplicate
void ClassScope::removeSpareTraitAbstractMethods(AnalysisResultPtr ar) {
  assert(Option::WholeProgram);
  for (MethodToTraitListMap::iterator iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    TraitMethodList& tMethList = iter->second;
    bool hasNonAbstractMeth = false;
    unsigned countAbstractMeths = 0;

    for (TraitMethodList::const_iterator traitMethIter = tMethList.begin();
         traitMethIter != tMethList.end(); traitMethIter++) {
      ModifierExpressionPtr modifiers = traitMethIter->m_modifiers ?
        traitMethIter->m_modifiers : traitMethIter->m_method->getModifiers();
      if (!(modifiers->isAbstract())) {
        hasNonAbstractMeth = true;
      } else {
        countAbstractMeths++;
      }
    }
    if (hasNonAbstractMeth || countAbstractMeths > 1) {
      // Erase spare abstract declarations
      bool firstAbstractMeth = true;
      for (TraitMethodList::iterator nextTraitIter = tMethList.begin();
           nextTraitIter != tMethList.end(); ) {
        TraitMethodList::iterator traitIter = nextTraitIter++;
        ModifierExpressionPtr modifiers = traitIter->m_modifiers ?
          traitIter->m_modifiers : traitIter->m_method->getModifiers();
        if (modifiers->isAbstract()) {
          if (hasNonAbstractMeth || !firstAbstractMeth) {
            tMethList.erase(traitIter);
          }
          firstAbstractMeth = false;
        }
      }
    }
  }
}

void ClassScope::importUsedTraits(AnalysisResultPtr ar) {
  // Trait flattening is supposed to happen only when we have awareness of
  // the whole program.
  assert(Option::WholeProgram);

  if (m_traitStatus == FLATTENED) return;
  if (m_traitStatus == BEING_FLATTENED) {
    getStmt()->analysisTimeFatal(
      Compiler::CyclicDependentTraits,
      "Cyclic dependency between traits involving %s",
      getOriginalName().c_str()
    );
    return;
  }
  if (m_usedTraitNames.size() == 0) {
    m_traitStatus = FLATTENED;
    return;
  }
  m_traitStatus = BEING_FLATTENED;

  // First, make sure that parent classes have their traits imported
  if (!m_parent.empty()) {
    ClassScopePtr parent = ar->findClass(m_parent);
    if (parent) {
      parent->importUsedTraits(ar);
    }
  }

  if (isTrait()) {
    for (auto const& req : getTraitRequiredExtends()) {
      ClassScopePtr rCls = ar->findClass(req);
      if (!rCls || rCls->isFinal() || rCls->isInterface()) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_BAD_REQ_EXTENDS,
          m_originalName.c_str(),
          req.c_str(),
          req.c_str()
        );
      }
    }
    for (auto const& req : getTraitRequiredImplements()) {
      ClassScopePtr rCls = ar->findClass(req);
      if (!rCls || !(rCls->isInterface())) {
        getStmt()->analysisTimeFatal(
          Compiler::InvalidDerivation,
          Strings::TRAIT_BAD_REQ_IMPLEMENTS,
          m_originalName.c_str(),
          req.c_str(),
          req.c_str()
        );
      }
    }
  }

  // Find trait methods to be imported
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
    // First, make sure the used trait is flattened
    tCls->importUsedTraits(ar);

    findTraitMethodsToImport(ar, tCls);

    // Import any interfaces implemented
    tCls->getInterfaces(ar, m_bases, false);

    importTraitRequirements(ar, tCls);
  }

  // Apply rules
  applyTraitRules(ar);

  // Remove trait abstract methods provided by other traits and duplicates
  removeSpareTraitAbstractMethods(ar);

  // Apply precedence of current class over used traits
  for (MethodToTraitListMap::iterator iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); ) {
    MethodToTraitListMap::iterator thisiter = iter;
    iter++;
    if (findFunction(ar, thisiter->first, 0, 0) != FunctionScopePtr()) {
      m_importMethToTraitMap.erase(thisiter);
    }
  }

  std::map<string, MethodStatementPtr> importedTraitMethods;
  std::vector<std::pair<string,const TraitMethod*>> importedTraitsWithOrigName;

  // Actually import the methods
  for (MethodToTraitListMap::const_iterator
         iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    // The rules may rule out a method from all traits.
    // In this case, simply don't import the method.
    if (iter->second.size() == 0) {
      continue;
    }
    // Consistency checking: each name must only refer to one imported method
    if (iter->second.size() > 1) {
      getStmt()->analysisTimeFatal(
        Compiler::MethodInMultipleTraits,
        Strings::METHOD_IN_MULTIPLE_TRAITS,
        iter->first.c_str()
      );
    } else {
      TraitMethodList::const_iterator traitMethIter = iter->second.begin();
      if ((traitMethIter->m_modifiers ? traitMethIter->m_modifiers :
           traitMethIter->m_method->getModifiers())->isAbstract()) {
        // Skip abstract methods, if method already exists in the class
        if (findFunction(ar, iter->first, true) ||
            importedTraitMethods.count(iter->first)) {
          continue;
        }
      }
      string sourceName = traitMethIter->m_ruleStmt ?
        toLower(((TraitAliasStatement*)traitMethIter->m_ruleStmt.get())->
                      getMethodName()) : iter->first;
      importedTraitMethods[sourceName] = MethodStatementPtr();
      importedTraitsWithOrigName.push_back(
        make_pair(sourceName, &*traitMethIter));
    }
  }

  // Make sure there won't be 2 constructors after importing
  auto traitConstruct = importedTraitMethods.count("__construct");
  auto traitName = importedTraitMethods.count(getName());
  auto classConstruct = m_functions.count("__construct");
  auto className = m_functions.count(getName());
  if ((traitConstruct && traitName) ||
      (traitConstruct && className) ||
      (classConstruct && traitName)) {
    getStmt()->analysisTimeFatal(
      Compiler::InvalidDerivation,
      "%s has colliding constructor definitions coming from traits",
      getOriginalName().c_str()
    );
  }

  for (unsigned i = 0; i < importedTraitsWithOrigName.size(); i++) {
    const string &sourceName = importedTraitsWithOrigName[i].first;
    const TraitMethod *traitMethod = importedTraitsWithOrigName[i].second;
    MethodStatementPtr newMeth = importTraitMethod(
      *traitMethod, ar, toLower(traitMethod->m_originalName),
      importedTraitMethods);
    if (newMeth) {
      importedTraitMethods[sourceName] = newMeth;
    }
  }

  // Import trait properties
  importTraitProperties(ar);

  m_traitStatus = FLATTENED;
}

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
  BOOST_FOREACH(std::string base_i, m_bases) {
    if (strcasecmp(base_i.c_str(), base.c_str()) == 0) return true;
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
                                          bool exclIntfBase /* = false */) {
  assert(toLower(name) == name);
  StringToFunctionScopePtrMap::const_iterator iter;
  iter = m_functions.find(name);
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
  StringToFunctionScopePtrMap::const_iterator iter;
  string name;
  if (classNameCtor()) {
    name = getName();
  } else {
    name = "__construct";
  }
  iter = m_functions.find(name);
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
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functions.find(name);
  if (iter != m_functions.end()) {
    FunctionScopePtr fs = iter->second;
    fs->setDynamic();
  } else if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == Derivation::Redeclaring) {
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

TypePtr ClassScope::checkProperty(BlockScopeRawPtr context,
                                  Symbol *sym, TypePtr type,
                                  bool coerce, AnalysisResultConstPtr ar) {
  return getVariables()->checkProperty(context, sym, type, coerce, ar);
}

TypePtr ClassScope::checkConst(BlockScopeRawPtr context,
                               const std::string &name, TypePtr type,
                               bool coerce, AnalysisResultConstPtr ar,
                               ConstructPtr construct,
                               const std::vector<std::string> &bases,
                               BlockScope *&defScope) {
  defScope = nullptr;
  return getConstants()->check(context, name, type, coerce,
                               ar, construct, m_bases, defScope);
}

void ClassScope::getAllParents(AnalysisResultConstPtr ar,
                               std::vector<std::string> &names) {
  if (m_stmt) {
    if (isInterface()) {
      dynamic_pointer_cast<InterfaceStatement>
        (m_stmt)->getAllParents(ar, names);
    } else {
      dynamic_pointer_cast<ClassStatement>
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
    vector<string>::const_iterator begin =
      m_parent.empty() ? m_bases.begin() : m_bases.begin() + 1;
    for (vector<string>::const_iterator it = begin;
         it != m_bases.end(); ++it) {
      ClassScopePtr cls(ar->findClass(*it));
      if (cls && cls->isRedeclaring()) {
        cls = self->findExactClass(cls);
      }
      names.push_back(cls ? cls->getDocName() : *it);
      if (cls && recursive) {
        cls->getInterfaces(ar, names, true);
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

  JSON::CodeError::MapStream cs(out);
  BOOST_FOREACH(string cname, cnames) {
    TypePtr type = m_constants->getType(cname);
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

static inline string GetDocName(AnalysisResultPtr ar,
                                BlockScopeRawPtr scope,
                                const string &name) {
  ClassScopePtr c(ar->findClass(name));
  if (c && c->isRedeclaring()) {
    ClassScopePtr exact(scope->findExactClass(c));
    return exact ?
      exact->getDocName() :
      c->getOriginalName(); // if we can't tell which redec class,
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
  ms.add("line", getStmt() ? getStmt()->getLocation()->line0 : 0);
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
  // TODO: you should really only get one of these, we should assert this
  if (m_kindOf == KindOfAbstractClass) mods |= ClassInfo::IsAbstract;
  if (m_kindOf == KindOfFinalClass)    mods |= ClassInfo::IsFinal;
  if (m_kindOf == KindOfInterface)     mods |= ClassInfo::IsInterface;
  if (m_kindOf == KindOfTrait)         mods |= ClassInfo::IsTrait;
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

bool ClassScope::addFunction(AnalysisResultConstPtr ar,
                             FunctionScopePtr funcScope) {
  FunctionScopePtr &func = m_functions[funcScope->getName()];
  if (func) {
    func->getStmt()->parseTimeFatal(Compiler::DeclaredMethodTwice,
                                    "Redeclared method %s::%s",
                                    getOriginalName().c_str(),
                                    func->getOriginalName().c_str());
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
