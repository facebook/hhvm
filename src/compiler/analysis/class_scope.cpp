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
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/construct.h>
#include <compiler/expression/class_constant_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/option.h>
#include <compiler/parser/parser.h>
#include <compiler/statement/interface_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/statement_list.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/class_info.h>
#include <compiler/statement/class_variable.h>
#include <compiler/statement/class_constant.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/statement/trait_prec_statement.h>
#include <compiler/statement/trait_alias_statement.h>
#include <runtime/base/zend/zend_string.h>
#include <util/util.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(KindOf kindOf, const std::string &name,
                       const std::string &parent,
                       const vector<string> &bases,
                       const std::string &docComment, StatementPtr stmt)
  : BlockScope(name, docComment, stmt, BlockScope::ClassScope),
    m_parent(parent), m_bases(bases), m_attribute(0), m_redeclaring(-1),
    m_kindOf(kindOf), m_derivesFromRedeclaring(FromNormal),
    m_traitStatus(NOT_FLATTENED), m_volatile(false), m_derivedByDynamic(false),
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
    m_parent(parent), m_bases(bases),
    m_attribute(0), m_redeclaring(-1),
    m_kindOf(KindOfObjectClass), m_derivesFromRedeclaring(FromNormal),
    m_traitStatus(NOT_FLATTENED), m_dynamic(false), m_volatile(false),
    m_derivedByDynamic(false), m_sep(false), m_needsCppCtor(false),
    m_needsInit(true) {
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

// like getId(), but without the label formatting
std::string ClassScope::getDocName() const {
  string name = getOriginalName();
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
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
  if (derivedByDynamic()) {
    super->m_derivedByDynamic = true;
  }
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

void ClassScope::checkDerivation(AnalysisResultPtr ar, hphp_istring_set &seen) {
  seen.insert(m_name);

  hphp_istring_set bases;
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
        Compiler::Error(Compiler::InvalidOverride,
                        fs->getStmt(), func->getStmt());
      }
    }
  }

  int n = forInvoke ? m_parent.empty() ? 0 : 1 : m_bases.size();
  // walk up
  for (int i = 0; i < n; i++) {
    const string &base = m_bases[i];
    ClassScopePtr super = ar->findClass(base);
    if (super) {
      if (super->isRedeclaring()) {
        if (forInvoke) continue;
        if (base == m_parent) {
          const ClassScopePtrVec &classes = ar->findRedeclaredClasses(m_parent);
          StringToFunctionScopePtrMap pristine(funcs);
          BOOST_FOREACH(ClassScopePtr cls, classes) {
            cls->m_derivedByDynamic = true;
            StringToFunctionScopePtrMap cur(pristine);
            derivedMagicMethods(cls);
            cls->collectMethods(ar, cur, false, forInvoke);
            inheritedMagicMethods(cls);
            funcs.insert(cur.begin(), cur.end());
            cls->getVariables()->
              forceVariants(ar, VariableTable::AnyNonPrivateVars);
          }
          m_derivesFromRedeclaring = DirectFromRedeclared;
          getVariables()->forceVariants(ar, VariableTable::AnyNonPrivateVars,
                                        false);
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

void ClassScope::importTraitMethod(const TraitMethod &traitMethod,
                                   AnalysisResultPtr ar,
                                   const string &methName) {
  MethodStatementPtr meth = traitMethod.m_method;
  const string &origMethName = traitMethod.m_originalName;
  ModifierExpressionPtr modifiers = traitMethod.m_modifiers;

  // Check for errors before cloning
  if (modifiers && modifiers->isStatic()) {
    Compiler::Error(Compiler::InvalidAccessModifier, traitMethod.m_ruleStmt);
    return;
  }

  MethodStatementPtr cloneMeth = dynamic_pointer_cast<MethodStatement>(
    dynamic_pointer_cast<ClassStatement>(m_stmt)->addClone(meth));
  cloneMeth->setName(methName);
  cloneMeth->setOriginalName(origMethName);
  // Note: keep previous modifiers if none specified when importing the trait
  if (modifiers && modifiers->getCount()) {
    cloneMeth->setModifiers(modifiers);
  }
  FunctionScopePtr funcScope = meth->getFunctionScope();
  FunctionScopePtr cloneFuncScope
    (new HPHP::FunctionScope(funcScope, ar, methName, origMethName, cloneMeth,
                             cloneMeth->getModifiers()));
  cloneMeth->resetScope(cloneFuncScope, true);
  cloneFuncScope->setOuterScope(shared_from_this());

  cloneMeth->addTraitMethodToScope(ar,
               dynamic_pointer_cast<ClassScope>(shared_from_this()));
  cloneMeth->analyzeProgram(ar);
}

void ClassScope::addImportTraitMethod(const TraitMethod &traitMethod,
                                      const string &methName) {
  m_importMethToTraitMap[methName].push_back(traitMethod);
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
                            set<ClassScopePtr> &visitedTraits) {
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

void ClassScope::applyTraitPrecRule(TraitPrecStatementPtr stmt) {
  const string methodName = Util::toLower(stmt->getMethodName());
  const string selectedTraitName = Util::toLower(stmt->getTraitName());
  set<string> otherTraitNames;
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
    Compiler::Error(Compiler::UnknownTrait, stmt);
  }

  // Sanity checking: otherTraitNames should be empty now
  if (otherTraitNames.size()) {
    Compiler::Error(Compiler::UnknownTrait, stmt);
  }
}

bool ClassScope::hasMethod(const string &methodName) const {
  return m_functions.find(methodName) != m_functions.end();
}

ClassScopePtr
ClassScope::findSingleTraitWithMethod(AnalysisResultPtr ar,
                                      const string &methodName) const {
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
  const string &traitName = aliasStmt->getTraitName();
  const string &origMethName = aliasStmt->getMethodName();
  const string &newMethName = aliasStmt->getNewMethodName();
  string origName = traitName.empty() ? "(null)" : traitName;
  origName += "::" + origMethName;
  m_traitAliases.push_back(pair<string, string>(newMethName, origName));
}

void ClassScope::applyTraitAliasRule(AnalysisResultPtr ar,
                                     TraitAliasStatementPtr stmt) {
  const string traitName = Util::toLower(stmt->getTraitName());
  const string origMethName = Util::toLower(stmt->getMethodName());
  const string newMethName = Util::toLower(stmt->getNewMethodName());

  // Get the trait's "class"
  ClassScopePtr traitCls;
  if (traitName.empty()) {
    traitCls = findSingleTraitWithMethod(ar, origMethName);
  } else {
    traitCls = ar->findClass(traitName);
  }
  if (!traitCls || !(traitCls->isTrait())) {
    Compiler::Error(Compiler::UnknownTrait, stmt);
    return;
  }

  // Keep record of alias rule
  addTraitAlias(stmt);

  // Get the method
  set<ClassScopePtr> visitedTraits;
  MethodStatementPtr methStmt = findTraitMethod(ar, traitCls, origMethName,
                                                visitedTraits);
  if (!methStmt) {
    Compiler::Error(Compiler::UnknownTraitMethod, stmt);
    return;
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
  ClassStatementPtr classStmt = dynamic_pointer_cast<ClassStatement>(getStmt());
  ASSERT(classStmt);
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
        ASSERT(aliasStmt);
        applyTraitAliasRule(ar, aliasStmt);
      }
    }
  }
}

void ClassScope::importUsedTraits(AnalysisResultPtr ar) {
  if (m_traitStatus == FLATTENED) return;
  if (m_traitStatus == BEING_FLATTENED) {
    Compiler::Error(Compiler::CyclicDependentTraits, getStmt());
    return;
  }
  if (m_usedTraitNames.size() == 0) {
    m_traitStatus = FLATTENED;
    return;
  }
  m_traitStatus = BEING_FLATTENED;

  // Find trait methods to be imported
  for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
    ClassScopePtr tCls = ar->findClass(m_usedTraitNames[i]);
    if (!tCls || !(tCls->isTrait())) {
      Compiler::Error(Compiler::UnknownTrait, getStmt());
      continue;
    }
    // First, make sure the used trait is flattened
    tCls->importUsedTraits(ar);

    findTraitMethodsToImport(ar, tCls);
  }

  // Apply rules
  applyTraitRules(ar);

  // Apply precedence of current class over used traits
  for (map<string,TraitMethodList>::iterator
         iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); ) {
    map<string,TraitMethodList>::iterator thisiter = iter;
    iter++;
    if (findFunction(ar, thisiter->first, 0, 0) != FunctionScopePtr()) {
      m_importMethToTraitMap.erase(thisiter);
    }
  }

  // Actually import the methods
  for (map<string,TraitMethodList>::const_iterator
         iter = m_importMethToTraitMap.begin();
       iter != m_importMethToTraitMap.end(); iter++) {

    // The rules may rule out a method from all traits.
    // In this case, simply don't import the method.
    if (iter->second.size() == 0) {
      continue;
    }
    // Consistency checking: each name must only refer to one imported method
    if (iter->second.size() > 1) {
      Compiler::Error(Compiler::MethodInMultipleTraits, getStmt());
    } else {
      TraitMethodList::const_iterator traitMethIter = iter->second.begin();
      importTraitMethod(*traitMethIter, ar, iter->first);
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
  ASSERT(Util::toLower(name) == name);
  StringToFunctionScopePtrMap::const_iterator iter;
  iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    ASSERT(iter->second);
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
  if (!Option::AllDynamic &&
      derivesFromRedeclaring() == DirectFromRedeclared) {
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
    ASSERT(iter->second);
    return iter->second;
  }

  // walk up
  if (recursive && derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr super = ar->findClass(m_parent);
    if (super) {
      FunctionScopePtr func = super->findConstructor(ar, true);
      if (func) return func;
    }
  }
  if (!Option::AllDynamic &&
      derivesFromRedeclaring() == DirectFromRedeclared) {
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
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functions.find(name);
  if (iter != m_functions.end()) {
    FunctionScopePtr fs = iter->second;
    fs->setDynamic();
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
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    (*iter)->setSystem();
  }
}

bool ClassScope::needLazyStaticInitializer() {
  if (isTrait()) return false;
  return getVariables()->getAttribute(VariableTable::ContainsDynamicStatic) ||
    getConstants()->hasDynamic();
}

void ClassScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar) {
  // header
  int attribute = ClassInfo::IsNothing;
  if (!isUserClass()) attribute |= ClassInfo::IsSystem;
  if (isVolatile() && !isRedeclaring()) attribute |= ClassInfo::IsVolatile;
  if (isInterface()) attribute |= ClassInfo::IsInterface|ClassInfo::IsAbstract;
  if (isTrait()) attribute |= ClassInfo::IsTrait;
  if (m_usedTraitNames.size() > 0) attribute |= ClassInfo::UsesTraits;
  if (m_traitAliases.size() > 0) attribute |= ClassInfo::HasAliasedMethods;
  if (m_kindOf == KindOfAbstractClass) attribute |= ClassInfo::IsAbstract;
  if (m_kindOf == KindOfFinalClass) attribute |= ClassInfo::IsFinal;
  if (needLazyStaticInitializer()) attribute |= ClassInfo::IsLazyInit;

  attribute |= m_attributeClassInfo;
  if (!m_docComment.empty() && Option::GenerateDocComments) {
    attribute |= ClassInfo::HasDocComment;
  } else {
    attribute &= ~ClassInfo::HasDocComment;
  }

  cg_printf("(const char *)0x%04X, \"%s\", \"%s\", \"%s\", (const char *)%d, "
            "(const char *)%d,\n", attribute,
            CodeGenerator::EscapeLabel(getOriginalName()).c_str(),
            CodeGenerator::EscapeLabel(m_parent).c_str(),
            m_stmt ? m_stmt->getLocation()->file : "",
            m_stmt ? m_stmt->getLocation()->line0 : 0,
            m_stmt ? m_stmt->getLocation()->line1 : 0);

  if (attribute & ClassInfo::IsVolatile) {
    cg_printf("(const char *)offsetof(GlobalVariables, CDEC(%s)),\n",
              CodeGenerator::FormatLabel(m_name).c_str());
  }

  if (!m_docComment.empty() && Option::GenerateDocComments) {
    char *dc = string_cplus_escape(m_docComment.c_str(), m_docComment.size());
    cg_printf("\"%s\",\n", dc);
    free(dc);
  }

  // parent interfaces
  for (unsigned int i = (m_parent.empty() ? 0 : 1); i < m_bases.size(); i++) {
    cg_printf("\"%s\", ", CodeGenerator::EscapeLabel(m_bases[i]).c_str());
  }
  cg_printf("NULL,\n");

  // traits
  if (attribute & ClassInfo::UsesTraits) {
    for (unsigned i = 0; i < m_usedTraitNames.size(); i++) {
      cg_printf("\"%s\", ",
                CodeGenerator::EscapeLabel(m_usedTraitNames[i]).c_str());
    }
    cg_printf("NULL,\n");
  }

  // trait alias rules
  if (attribute & ClassInfo::HasAliasedMethods) {
    for (unsigned i = 0; i < m_traitAliases.size(); i++) {
      cg_printf("\"%s\", \"%s\", ",
                CodeGenerator::EscapeLabel(m_traitAliases[i].first).c_str(),
                CodeGenerator::EscapeLabel(m_traitAliases[i].second).c_str());
    }
    cg_printf("NULL,\n");
  }

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
  defScope = NULL;
  return getConstants()->check(context, name, type, coerce,
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

void ClassScope::getInterfaces(AnalysisResultConstPtr ar,
                               std::vector<std::string> &names,
                               bool recursive /* = true */) const {
  ClassScope *self = const_cast<ClassScope*>(this);
  if (recursive && !m_parent.empty()) {
    ClassScopePtr cls(ar->findClass(m_parent));
    if (cls && cls->isRedeclaring()) {
      cls = self->findExactClass(m_parent);
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
        cls = self->findExactClass(*it);
      }
      if (cls) names.push_back(cls->getDocName());
      else     names.push_back(*it);
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
    ClassScopePtr exact(scope->findExactClass(name));
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
    out << JSON::Null;
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

void ClassScope::outputCPPDynamicClassDecl(CodeGenerator &cg) {
  if (isTrait()) return;
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg_printf("ObjectData *%s%s(%s) NEVER_INLINE;\n",
            Option::CreateObjectOnlyPrefix, clsName,
            isRedeclaring() && derivedByDynamic() ?
            "ObjectData *root = NULL" : "");
}

void ClassScope::outputCPPDynamicClassCreateDecl(CodeGenerator &cg) {
  cg_printf("Object create_object_only("
            "CStrRef s, ObjectData *root);\n");
  cg_printf("ObjectData *create_object_only_no_init("
            "CStrRef s, ObjectData *root);\n");
}

void ClassScope::outputCPPDynamicClassImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  if (isTrait()) return;
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg_indentBegin("ObjectData *%s%s(%s) {\n",
                 Option::CreateObjectOnlyPrefix, clsName,
                 isRedeclaring() && derivedByDynamic() ?
                 "ObjectData *root /* = NULL */" : "");
  cg_printf("return NEWOBJ(%s%s)(%s);\n",
            Option::ClassPrefix, clsName,
            isRedeclaring() && derivedByDynamic() ? "root" : "");
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPHashTableClasses
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  if (classes.size()) {
    ASSERT(cg.getCurrentIndentation() == 0);
    const char text1[] =
      "struct hashNodeCTD {\n"
      "  int64 hash;\n"
      "  int32 flags;\n"
      "  int32 cdec;\n"
      "  const char *name;\n"
      "  int64 ptv1;\n"
      "};\n";

    const char text2[] =
      "#define GET_CS_OFFSET(n) offsetof(GlobalVariables, %s ## n)\n"
      "inline ALWAYS_INLINE "
      "const ObjectStaticCallbacks *getCallbacks(\n"
      "  const hashNodeCTD *p, CStrRef s) {\n"
      "  int64 off = p->ptv1;\n"
      "  if (LIKELY(!(off & 1))) return ((const ObjectStaticCallbacks *)off);\n"
      "  DECLARE_GLOBAL_VARIABLES(g);\n"
      "  checkClassExistsThrow(s, (bool*)((char*)g + p->cdec));\n"
      "  if (LIKELY(!(off & 2))) /* volatile class */ return "
      "((const ObjectStaticCallbacks *)(off-1));\n"
      "  /* redeclared class */\n"
      "  return *(ObjectStaticCallbacks**)((char*)g + (off-3));\n"
      "}\n";

    const char text3[] =
      "\n"
      "static const hashNodeCTD *\n"
      "findCTD(CStrRef name) {\n"
      "  int64 hash = name->hash();\n"
      "  int o = ctdMapTable[hash & %d];\n"
      "  if (UNLIKELY(o < 0)) return NULL;\n"
      "  const hashNodeCTD *p = &ctdBuckets[o];\n"
      "  do {\n"
      "    int64 h = p->hash;\n"
      "    if (h == hash && "
      "(LIKELY(p->name==name.data())||"
      "LIKELY(!strcasecmp(p->name, name.data())))) return p;\n"
      "  } while (!(p++->flags & 1));\n"
      "  return NULL;\n"
      "}\n";

    JumpTable jt(cg, classes, true, true, true, true);
    cg_printf(text1);
    if (!system) {
      cg_printf(text2, Option::ClassStaticsCallbackPrefix);
    }
    cg_printf("static const hashNodeCTD ctdBuckets[] = {\n");

    vector<int> offsets;
    int prev = -1;
    for (int n = 0; jt.ready(); ++n, jt.next()) {
      int cur = jt.current();
      if (prev != cur) {
        while (++prev != cur) {
          offsets.push_back(-1);
        }
        offsets.push_back(n);
      }
      const char *clsName = jt.key();
      StringToClassScopePtrVecMap::const_iterator iterClasses =
        classScopes.find(clsName);
      ClassScopeRawPtr cls = iterClasses->second[0];
      cg_printf("  {0x%016llXLL,%d,",
                hash_string_i(clsName), jt.last() ? 1 : 0);
      if (cls->isVolatile()) {
        cg_printf("offsetof(GlobalVariables,CDEC(%s))",
                  CodeGenerator::FormatLabel(cls->getName()).c_str());
      } else {
        cg_printf("0");
      }
      cg_printf(",\"%s\",", CodeGenerator::EscapeLabel(clsName).c_str());
      if (cls->isRedeclaring()) {
        ASSERT(!system);
        cg_printf("GET_CS_OFFSET(%s)+3",
                  CodeGenerator::FormatLabel(cls->getName()).c_str());
      } else {
        string clsFmt = CodeGenerator::FormatLabel(clsName);
        cg_printf("(int64)&%s%s%s",
                  Option::ClassStaticsCallbackPrefix,
                  clsFmt.c_str(),
                  cls->isVolatile() ? "+1" : "");
      }
      cg_printf(" },\n");
    }

    cg_printf("};\n");
    cg_indentBegin("static const int ctdMapTable[] = {\n");
    for (int i = 0, e = jt.size(), s = offsets.size(); i < e; i++) {
      cg_printf("%d,", i < s ? offsets[i] : -1);
      if ((i & 7) == 7) cg_printf("\n");
    }
    cg_printf("\n");
    cg_indentEnd("};\n");

    cg_printf(text3, jt.size() - 1);
  }

  cg_indentBegin("const ObjectStaticCallbacks * "
                 "get%s_object_static_callbacks(CStrRef s) {\n",
                 system ? "_builtin" : "");
  if (classes.size()) {
    if (system) {
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (p) {\n"
                "  return "
                "((const ObjectStaticCallbacks *)p->ptv1);\n"
                "}\n"
                "return NULL;\n");
    } else {
      cg_printf("const hashNodeCTD *p = findCTD(s);\n"
                "if (!p) return get_builtin_object_static_callbacks(s);\n"
                "return getCallbacks(p,s);\n");
    }
  } else {
    if (system) {
      cg_printf("return NULL;\n");
    } else {
      cg_printf("return get_builtin_object_static_callbacks(s);\n");
    }
  }
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPClassVarInitImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

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
  if (classes.size()) {
    cg_printf("const ObjectStaticCallbacks *cwo = "
              "get_%sobject_static_callbacks(s);\n"
              "return LIKELY(cwo != 0) ? "
              "cwo->os_getInit(var) : throw_missing_class(s);\n",
              system ? "builtin_" : "");
  } else {
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
  if (!classes.size()) {
    if (system) {
      cg_printf("throw_missing_class(s);\n");
      cg_printf("return 0;\n");
    } else {
      cg_printf("return create_builtin_object_only_no_init(s, root);\n");
    }
  } else {
    cg_printf("const ObjectStaticCallbacks *cwo = "
              "get_%sobject_static_callbacks(s);\n"
              "if (LIKELY(cwo != 0)) return cwo->createOnlyNoInit(root);\n"
              "throw_missing_class(s);\n"
              "return 0;\n",
              system ? "builtin_" : "");
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
    cg_printf("const ObjectStaticCallbacks *cwo = "
              "get_%sobject_static_callbacks(s);\n"
              "return ObjectStaticCallbacks::GetCallInfo(cwo,mcp,-1);\n" ,
              system ? "builtin_" : "");
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

  cg.printf("const ObjectStaticCallbacks * cwo = "
            "get%s_object_static_callbacks(s);\n",
            system ? "_builtin" : "");
  cg.printf("if (cwo) return cwo->os_get(prop);\n");

  cg_printf("return null;\n");
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

  cg.printf("const ObjectStaticCallbacks * cwo = "
            "get%s_object_static_callbacks(s);\n",
            system ? "_builtin" : "");
  cg.printf("if (cwo) return &cwo->os_lval(prop);\n");

  cg_printf("return NULL;\n");
  cg_indentEnd("}\n");
}

void ClassScope::outputCPPGetClassConstantImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg_indentBegin("Variant get%s_class_constant(CStrRef s, "
                 "const char *constant, int fatal /* = true */) {\n",
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

  cg_indentBegin("if (fatal > 0) {\n");
  cg_printf("raise_error(\"Couldn't find constant %%s::%%s\", s.data(), "
            "constant);\n");
  cg_indentEnd();
  cg_indentBegin("} else if (!fatal) {\n");
  cg_printf("raise_warning(\"Couldn't find constant %%s::%%s\", s.data(), "
            "constant);\n");
  cg_indentEnd("}\n");
  cg_printf("return null;\n");

  cg_indentEnd("}\n");
}

static int propTableSize(int entries) {
  if (!entries) return 0;
  int size = Util::roundUpToPowerOfTwo(entries * 2);
  return size < 8 ? 8 : size;
}

static bool buildClassPropTableMap(
  CodeGenerator &cg, AnalysisResultPtr ar,
  const StringToClassScopePtrVecMap &classScopes,
  ClassScope::ClassPropTableMap &tables) {
  typedef ClassScope::IndexedSym IndexedSym;
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool ret = false;
  for (StringToClassScopePtrVecMap::const_iterator iter = classScopes.begin();
       iter != classScopes.end(); ++iter) {
    const ClassScopePtrVec &classes = iter->second;
    if (system) ASSERT(classes.size() == 1);
    for (unsigned int i = 0; i < classes.size(); i++) {
      ClassScopePtr cls = classes[i];
      vector<const Symbol *> entries[3];

      const std::vector<Symbol*> &symbolVec =
        cls->getVariables()->getSymbols();
      for (unsigned j = 0; j < symbolVec.size(); j++) {
        const Symbol *sym = symbolVec[j];
        assert(!sym->isStatic() || !sym->isOverride());
        entries[sym->isStatic()].push_back(sym);
      }
      const std::vector<Symbol*> &constVec =
        cls->getConstants()->getSymbols();
      for (unsigned j = 0; j < constVec.size(); j++) {
        const Symbol *sym = constVec[j];
        if (!system && !sym->getValue()) continue;
        entries[2].push_back(sym);
      }

      if (!entries[0].size() && !entries[1].size() && !entries[2].size()) {
        continue;
      }
      ret = true;

      ClassScope::ClassPropTableInfo &info = tables[cls->getId()];
      info.cls = cls;

      for (int s = 0; s < 3; s++) {
        int tableSize = propTableSize(entries[s].size());

        unsigned p = 0;
        for (unsigned j = 0; j < entries[s].size(); j++) {
          const Symbol *sym = entries[s][j];
          int hash = hash_string_i(sym->getName().c_str(),
                                   sym->getName().size()) & (tableSize - 1);
          int pix = -1;
          switch (s) {
            case 0:
              if (sym->isPrivate()) {
                pix = p++;
              }
              break;
            case 1:
              if (static_pointer_cast<Expression>(sym->getClassInitVal())->
                  containsDynamicConstant(ar)) {
                pix = p++;
              }
              break;
            case 2:
              if (sym->isDynamic()) {
                pix = p++;
              }
              break;
          }
          info.syms[s][hash].push_back(IndexedSym(j, pix, sym));
        }
        info.actualIndex[s].resize(entries[s].size() + 1, -1);
        info.privateIndex[s].resize(p);
        int newIndex = 0;
        for (map<int, vector<IndexedSym> >::iterator it = info.syms[s].begin(),
               end = info.syms[s].end(); it != end; ++it) {
          const vector<IndexedSym> &v = it->second;
          for (int k = 0, sz = v.size(); k < sz; k++, newIndex++) {
            const IndexedSym &is = v[k];
            info.actualIndex[s][is.origIndex] = newIndex;
            if (is.privIndex >= 0) {
              info.privateIndex[s][is.privIndex] = newIndex;
            }
          }
        }
      }
    }
  }
  return ret;
}

bool ClassScope::checkHasPropTable() {
  VariableTablePtr variables = getVariables();

  if (variables->getSymbols().size()) return true;

  ConstantTablePtr constants = getConstants();
  const std::vector<Symbol*> &constVec = constants->getSymbols();
  if (!constVec.size()) return false;
  for (int i = 0, sz = constVec.size(); i < sz; i++) {
    const Symbol *sym = constVec[i];
    if (getAttribute(ClassScope::System) || sym->getValue()) return true;
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

enum {
  ConstValid = 1,
  ConstDynamic = 2,
  ConstRedeclared = 4,
  ConstDerivesFromRedeclared = 8,
  ConstMagicIO = 16,
  ConstNeedsName = 32,
  ConstNeedsClass = 64,
  ConstNeedsSysCon = 128,
  ConstNonScalarValue = 256,

  ConstPlain = 0x200,
  ConstNull = 0x400,
  ConstFalse = 0x800,
  ConstZero = 0x1000,
  ConstDZero = 0x2000,
};

static string findScalar(ExpressionPtr val,
                         string &name, string &cls, int *flags = 0) {
  if (flags) *flags = 0;
  if (!val) {
    if (flags) *flags |= ConstNull | ConstPlain;
    return "N;";
  }
  Variant v;
  if (val->getScalarValue(v)) {
    if (flags) {
      *flags |= ConstPlain;
      if (!v.toBoolean()) {
        if (v.isNull()) {
          *flags |= ConstNull;
        } else if (v.isBoolean()) {
          *flags |= ConstFalse;
        } else if (v.isInteger()) {
          *flags |= ConstZero;
        } else if (v.isDouble()) {
          *flags |= ConstDZero;
        }
      }
    }
    return f_serialize(v).c_str();
  }
  if (val->is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr con = static_pointer_cast<ConstantExpression>(val);
    name = con->getName();
    if (flags) {
      if (con->isValid()) {
        *flags |= ConstValid;
        if (con->isDynamic()) {
          *flags |= ConstDynamic | ConstNeedsName;
        }
        if (name == "STDIN" || name == "STDOUT" || name == "STDERR") {
          *flags |= ConstMagicIO;
        }
      } else {
        *flags |= ConstNeedsName;
      }
    }
    cls = "";
  } else if (val->is(Expression::KindOfClassConstantExpression)) {
    ClassConstantExpressionPtr con =
      static_pointer_cast<ClassConstantExpression>(val);
    name = con->getConName();
    cls = con->getActualClassName();
    if (flags) {
      if (con->isValid()) {
        *flags |= ConstValid;
        if (con->isDynamic()) *flags |= ConstDynamic;
      } else if (con->isRedeclared()) {
        *flags |= ConstRedeclared | ConstNeedsName;
      } else if (con->hasClass()) {
        *flags |= ConstDerivesFromRedeclared | ConstNeedsName;
      } else {
        *flags |= ConstNeedsName | ConstNeedsClass;
      }
    }
  } else {
    if (flags) *flags |= ConstNonScalarValue;
    return val->getText(false, true);
  }

  return cls + "::" + name;
}

static ExpressionPtr getSymInit(const Symbol *sym) {
  ConstructPtr c;
  if (sym->isConstant()) {
    c = sym->getValue();
  } else {
    c = sym->getClassInitVal();
  }
  return static_pointer_cast<Expression>(c);
}

void ClassScope::outputCPPGetClassPropTableImpl(
  CodeGenerator &cg, AnalysisResultPtr ar,
  const StringToClassScopePtrVecMap &classScopes,
  bool extension /* = false */) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  ClassPropTableMap tables;
  if (!buildClassPropTableMap(cg, ar, classScopes, tables)) return;

  cg.printSection("Class tables");

  int s; // 1 => static

  int index = 0;
  std::map<string,int> svarIndex;
  std::map<string,ExpressionRawPtr> nonScalarArrays;

  if (!Option::UseScalarVariant) {
    cg_indentBegin("static Variant %sstatic_init_vals[] = {\n",
                   Option::ClassPropTablePrefix);
  }

  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    for (s = 3; s--; ) {
      if (!info.syms[s].size()) continue;
      for (map<int, vector<IndexedSym> >::const_iterator
             it = info.syms[s].begin(), end = info.syms[s].end();
           it != end; ++it) {
        const vector<IndexedSym> &v = it->second;
        for (int k = 0, sz = v.size(); k < sz; k++) {
          const Symbol *sym = v[k].sym;
          ExpressionPtr val = getSymInit(sym);
          int flags = 0;
          string name,cls,id;
          if (!val) continue;
          id = findScalar(val, name, cls, &flags);
          if (flags & ConstNonScalarValue) {
            nonScalarArrays[id] = val;
            continue;
          }
          if (Option::UseScalarVariant) {
            continue;
          }
          if (name.empty()) {
            int *p = &svarIndex[id];
            if (!*p) {
              val->outputCPP(cg, ar);
              cg_printf(",\n");
              *p = ++index;
            }
          }
          if (flags & ConstNeedsName) {
            int *p = &svarIndex[f_serialize(name).c_str()];
            if (!*p) {
              *p = ++index;
              ScalarExpression::OutputCPPString(name, cg, ar,
                                                info.cls, true);
            }
          }
          if (flags & ConstNeedsClass) {
            int *p = &svarIndex[f_serialize(cls).c_str()];
            if (!*p) {
              *p = ++index;
              ScalarExpression::OutputCPPString(cls, cg, ar,
                                                info.cls, true);
            }
          }
        }
      }
    }
  }

  if (!Option::UseScalarVariant) {
    cg_indentEnd("};\n");
  }

  index = 0;
  std::map<string,int> siIndex;

  for (std::map<string,ExpressionRawPtr>::iterator
         it = nonScalarArrays.begin(), end = nonScalarArrays.end();
       it != end; ++it) {
    siIndex[it->first] = index += 2;

    cg_indentBegin("static Variant %snon_scalar_constant_%d() {\n",
                   Option::ClassPropTablePrefix, index);
    cg.printDeclareGlobals();
    cg_printf("return ");
    it->second->outputCPP(cg, ar);
    cg_printf(";\n");
    cg_indentEnd("}\n");
  }

  nonScalarArrays.clear();

  cg_indentBegin("static const int64 %sstatic_inits[] = {\n",
                 Option::ClassPropTablePrefix);

  for (int i = 0; i < index; i += 2) {
    cg_printf("(int64)%snon_scalar_constant_%d, 0x%07x6,\n",
              Option::ClassPropTablePrefix, i+2, i);
  }

  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    for (s = 3; s--; ) {
      if (!info.syms[s].size()) continue;
      for (map<int, vector<IndexedSym> >::const_iterator
             it = info.syms[s].begin(), end = info.syms[s].end();
           it != end; ++it) {
        const vector<IndexedSym> &v = it->second;
        for (int k = 0, sz = v.size(); k < sz; k++) {
          const Symbol *sym = v[k].sym;
          ExpressionPtr val = getSymInit(sym);
          string name, cls, id;
          int flags = 0;
          int type = 0;
          if (!val) {
            if (sym->isConstant()) {
              name = sym->getName();
              cls = info.cls->getId();
              id = cls + "::" + name;
              flags = ConstValid|ConstNeedsSysCon;
              type = sym->getFinalType()->getDataType();
            } else {
              flags |= ConstPlain;
              id = "N;";
            }
          } else {
            id = findScalar(val, name, cls, &flags);
            if (flags & ConstNonScalarValue) continue;
            if (id == "N;") {
              val.reset();
            } else if (flags & ConstValid &&
                       !(flags & (ConstDynamic|ConstMagicIO))) {
              flags |= ConstNeedsSysCon;
              type = val->getActualType()->getDataType();
            }
          }
          int *p;
          int name_ix = -1, cls_ix = -1;
          if ((flags & ConstNeedsSysCon) && type != KindOfVariant) {
            string n = cls + "$$" + name;
            p = &siIndex[n];
            if (!*p) {
              if (cls.size()) {
                cg_printf("(int64)&%s%s%s%s,\n",
                          Option::ClassConstantPrefix,
                          cls.c_str(),
                          Option::IdPrefix.c_str(),
                          CodeGenerator::FormatLabel(name).c_str());
              } else {
                cg_printf("(int64)&%s%s,\n",
                          Option::ConstantPrefix,
                          CodeGenerator::FormatLabel(name).c_str());
              }
              *p = ++index;
            }
            name_ix = *p - 1;
          }
          if (flags & ConstNeedsName) {
            string n = f_serialize(name).c_str();
            p = &siIndex[n];
            if (!*p) {
              cg_printf("(int64)&");
              if (Option::UseScalarVariant) {
                cg.setScalarVariant();
                ScalarExpression::OutputCPPString(name, cg, ar,
                                                  info.cls, true);
                cg.clearScalarVariant();
              } else {
                cg_printf("%sstatic_init_vals[%d]",
                          Option::ClassPropTablePrefix,
                          svarIndex[n] - 1);
              }
              cg_printf(",\n");
              *p = ++index;
            }
            name_ix = *p - 1;
          }
          if (flags & ConstNeedsClass) {
            string n = f_serialize(cls).c_str();
            p = &siIndex[n];
            if (!*p) {
              cg_printf("(int64)&");
              if (Option::UseScalarVariant) {
                cg.setScalarVariant();
                ScalarExpression::OutputCPPString(cls, cg, ar,
                                                  info.cls, true);
                cg.clearScalarVariant();
              } else {
                cg_printf("%sstatic_init_vals[%d]",
                          Option::ClassPropTablePrefix,
                          svarIndex[n] - 1);
              }
              cg_printf(",\n");
              *p = ++index;
            }
            cls_ix = *p - 1;
          } else if (cls.size() &&
                     flags & (ConstDerivesFromRedeclared|ConstDynamic)) {
            string n = Option::ClassStaticsCallbackPrefix + cls;
            p = &siIndex[n];
            if (!*p) {
              cg_printf("(int64)&%s,\n", n.c_str());
              *p = ++index;
            }
            cls_ix = *p - 1;
          }

          p = &siIndex[id];
          if (!*p) {
            if (flags & ConstPlain) {
              if (!val) {
                cg_printf("(int64)&null_variant");
              } else {
                cg_printf("(int64)&");
                if (Option::UseScalarVariant) {
                  cg.setScalarVariant();
                  val->outputCPP(cg, ar);
                  cg.clearScalarVariant();
                } else {
                  cg_printf("%sstatic_init_vals[%d]",
                            Option::ClassPropTablePrefix, svarIndex[id] - 1);
                }
              }
            } else if ((flags & ConstNeedsSysCon) && type != KindOfVariant) {
              cg_printf("0x%08x%07x7", name_ix, type);
            } else if (flags & ConstMagicIO) {
              cg_printf("(int64)&BuiltinFiles::Get%s, 0x1%07x6",
                        name.c_str(), index++);
            } else if (flags & ConstValid && !(flags & ConstDynamic)) {
              if (Type::IsMappedToVariant(val->getActualType())) {
                cg_printf("(int64)&");
                val->outputCPP(cg, ar);
              } else {
                cg_printf("(int64)&%sstatic_init_vals[%d]",
                          Option::ClassPropTablePrefix, svarIndex[id] - 1);
              }
            } else if (cls.empty()) {
              if (flags & ConstDynamic) {
                cg_printf("(int64(offsetof(%s,%s%s))<<32)+",
                          system ? "SystemGlobals" : "GlobalVariables",
                          Option::ConstantPrefix,
                          CodeGenerator::FormatLabel(name).c_str());
              }
              cg_printf("0x%x1LL", name_ix);
            } else if (flags & ConstRedeclared) {
              cg_printf("offsetof(%s,%s%s)+0x%x00000003",
                        system ? "SystemGlobals" : "GlobalVariables",
                        Option::ClassStaticsCallbackPrefix,
                        CodeGenerator::FormatLabel(cls).c_str(),
                        name_ix);
            } else if (flags & ConstDynamic) {
              cg_printf("(int64(offsetof(%s,%s%s%s%s)<<32))+0x%x2",
                        system ? "SystemGlobals" : "GlobalVariables",
                        Option::ClassConstantPrefix,
                        cls.c_str(),
                        Option::IdPrefix.c_str(),
                        CodeGenerator::FormatLabel(name).c_str(),
                        cls_ix);
            } else {
              cg_printf("0x%x%07x%dLL", name_ix, cls_ix,
                        flags & ConstDerivesFromRedeclared ? 4 : 5);
            }
            *p = ++index;
            cg_printf(",\n");
          }
        }
      }
    }
  }
  cg_indentEnd("};\n");

  vector<int> static_inits;
  int curEntry = 0;
  cg_indentBegin("static const ClassPropTableEntry %stable_entries[] = {\n",
                 Option::ClassPropTablePrefix);
  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;
    for (s = 3; s--; ) {
      if (!info.syms[s].size()) continue;
      ClassScopePtr cls = info.cls;
      for (map<int, vector<IndexedSym> >::const_iterator
             it = info.syms[s].begin(), end = info.syms[s].end();
           it != end; ++it) {
        const vector<IndexedSym> &v = it->second;
        for (int k = 0, sz = v.size(); k < sz; k++) {
          const Symbol *sym = v[k].sym;
          int cur = info.actualIndex[s][v[k].origIndex];
          int next = info.actualIndex[s][v[k].origIndex + 1];
          if (next < 0) next = cur;

          int off;
          int cflags = 0;
          {
            ExpressionPtr val = getSymInit(sym);
            if (!val && sym->isConstant()) {
              off = siIndex[info.cls->getId() + "::" + sym->getName()] - 1;
            } else {
              string name;
              string cls;
              string id = findScalar(val, name, cls, &cflags);
              off = siIndex[id] - 1;
            }
            assert(off >= 0);
          }

          string prop(sym->getName());
          int flags = 0;
          int dtype = sym->getFinalType()->getDataType();
          if (sym->isStatic()) {
            flags |= ClassPropTableEntry::Static;
            if (v[k].privIndex < 0) {
              bool needsInit = true;
              switch (dtype) {
                case KindOfBoolean:
                  if (cflags & ConstFalse) needsInit = false;
                  goto check_plain;
                case KindOfInt32:
                case KindOfInt64:
                  if (cflags & ConstZero) needsInit = false;
                  goto check_plain;
                case KindOfDouble:
                  if (cflags & ConstDZero) needsInit = false;
                  goto check_plain;
                case KindOfVariant:
                case KindOfString:
                case KindOfStaticString:
                case KindOfArray:
                check_plain:
                  if (cflags & ConstPlain && !system) {
                    flags |= ClassPropTableEntry::FastInit;
                  }
                  break;
                default:
                  break;
              }
              if (needsInit) {
                static_inits.push_back(curEntry);
              }
            }
          }
          if (sym->isPublic())    flags |= ClassPropTableEntry::Public;
          if (sym->isProtected()) flags |= ClassPropTableEntry::Protected;
          if (sym->isConstant())  flags |= ClassPropTableEntry::Constant;
          if (sym->isPrivate()) {
            ASSERT(!sym->isOverride());
            flags |= ClassPropTableEntry::Private;
            if (!sym->isStatic()) {
              prop = '\0' + cls->getOriginalName() + '\0' + prop;
            }
          } else if (sym->isOverride() ||
                     (!sym->isStatic() && cls->derivesFromRedeclaring())) {
            ASSERT(!system);
            flags |= ClassPropTableEntry::Override;
          }
          if (k == sz - 1) flags |= ClassPropTableEntry::Last;
          curEntry++;
          cg_printf("{0x%016llXLL,%d,%d,%d,%d,%d,",
                    hash_string(sym->getName().c_str(),
                                sym->getName().size()),
                    next - cur, off,
                    int(s ? 0 : prop.size() - sym->getName().size()),
                    flags, dtype);
          if (s) {
            if (s == 2 && !sym->isDynamic()) {
              cg_printf("0,");
            } else {
              cg_printf("offsetof(%s,%s%s%s%s),",
                        system ? "SystemGlobals" : "GlobalVariables",
                        s == 2 ?
                        Option::Option::ClassConstantPrefix :
                        Option::Option::StaticPropertyPrefix,
                        cls->getId().c_str(),
                        Option::IdPrefix.c_str(), sym->getName().c_str());
            }
          } else {
            if (flags & ClassPropTableEntry::Override) {
              cg_printf("0,");
            } else {
              cg_printf("GET_PROPERTY_OFFSET(%s%s, %s%s),",
                        Option::ClassPrefix, cls->getId().c_str(),
                        Option::PropertyPrefix, sym->getName().c_str());
            }
          }
          cg_printf("&");
          cg_printString(prop, ar, cls);
          cg_printf(" },\n");
        }
      }
      cg_printf("\n");
    }
  }
  cg_indentEnd("};\n");

  cg_indentBegin("static const int %shash_entries[] = {\n",
                 Option::ClassPropTablePrefix);

  curEntry = 0;
  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;

    cg_printf("// %s hash\n", info.cls->getId().c_str());

    int m = 0;
    int off[3];
    for (s = 3; s--; ) {
      off[s] = m;
      if (!info.syms[s].size()) continue;
      int n = 0;
      vector<int> offsets;
      for (map<int, vector<IndexedSym> >::const_iterator
             it = info.syms[s].begin(), end = info.syms[s].end();
           it != end; ++it) {
        while (n < it->first) {
          offsets.push_back(-1);
          n++;
        }
        offsets.push_back(m);
        n++;
        m += it->second.size();
      }
      int sz = propTableSize(info.actualIndex[s].size() - 1);
      assert(n <= sz);
      while (n < sz) {
        offsets.push_back(-1);
        n++;
      }
      if (s) {
        while (n--) {
          cg_printf("%d,", offsets[n]);
        }
      } else {
        for (n = 0; n < sz; n++) {
          cg_printf("%d,", offsets[n]);
        }
      }
      cg_printf("\n");
      curEntry += sz;
    }

    cg_printf("// %s lists\n", info.cls->getId().c_str());
    for (s = 0; s < 3; s++) {
      unsigned psize=info.privateIndex[s].size();
      for (unsigned i = 0; i < psize; i++) {
        cg_printf("%d,", off[s] + info.privateIndex[s][i]);
      }
      cg_printf("-1,\n");
      curEntry += psize + 1;
    }
  }

  if (static_inits.size()) {
    cg_printf("%d,", (int)static_inits.size());
    for (unsigned i = 0; i < static_inits.size(); i++) {
      cg_printf("%d,", static_inits[i]);
    }
  }

  cg_indentEnd("};\n");

  int curOffset = 0;
  int hashOffset = 0;
  for (ClassPropTableMap::const_iterator iter = tables.begin();
       iter != tables.end(); iter++) {
    const ClassPropTableInfo &info = iter->second;

    cg_indentBegin("const ClassPropTable %s%s::%sprop_table = {\n",
                   Option::ClassPrefix, iter->first.c_str(),
                   Option::ObjectStaticPrefix);
    int tableSizeN = propTableSize(info.actualIndex[0].size() - 1);
    int tableSizeS = propTableSize(info.actualIndex[1].size() - 1);
    int tableSizeC = propTableSize(info.actualIndex[2].size() - 1);
    cg_printf("%d,%d,%d,%d,%d,%d,%d,",
              tableSizeN - 1,
              tableSizeN ?
              int(info.actualIndex[2].size() + info.actualIndex[1].size()) +
              info.actualIndex[0][0] - 2 : -1,
              tableSizeS - 1,
              int(tableSizeS ? info.actualIndex[2].size() +
                  info.actualIndex[1][0] - 1 : -1),
              tableSizeC - 1,
              tableSizeC ? info.actualIndex[2][0] : -1,
              int(tableSizeN + info.privateIndex[0].size() + 1));
    if (info.cls->needLazyStaticInitializer()) {
      cg_printf("offsetof(%s,%s%s),",
                system ? "SystemGlobals" : "GlobalVariables",
                Option::ClassStaticInitializerFlagPrefix,
                info.cls->getId().c_str());
    } else {
      cg_printf("0,\n");
    }

    hashOffset += tableSizeS + tableSizeC;
    cg_printf("%shash_entries+%d,",
              Option::ClassPropTablePrefix, hashOffset);
    hashOffset += tableSizeN + 3 +
      info.privateIndex[0].size() +
      info.privateIndex[1].size() +
      info.privateIndex[2].size();
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
    if (siIndex.size()) {
      cg_printf("%sstatic_inits\n", Option::ClassPropTablePrefix);
    } else {
      cg_printf("0\n");
    }
    cg_indentEnd("};\n");

    curOffset += info.actualIndex[0].size() + info.actualIndex[1].size() +
      info.actualIndex[2].size() - 3;
  }

  if (static_inits.size()) {
    ClassPropTableMap::const_iterator iter = tables.begin();
    cg_printf("const Globals::StaticInits %sstatic_initializer("
              "&%s%s::%sprop_table, %shash_entries+%d);\n",
              Option::ClassPropTablePrefix,
              Option::ClassPrefix, iter->first.c_str(),
              Option::ObjectStaticPrefix,
              Option::ClassPropTablePrefix, curEntry);
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

void ClassScope::outputCPPHeader(AnalysisResultPtr ar,
                                 CodeGenerator::Output output) {
  if (isTrait()) return;
  string filename = getHeaderFilename();
  string root = ar->getOutputPath() + "/";
  Util::mkdir(root + filename);
  ofstream f((root + filename).c_str());
  CodeGenerator cg(&f, output);
  cg.setFileOrClassHeader(true);

  cg.headerBegin(filename);

  if (Option::GenerateCppLibCode ||
      cg.getOutput() == CodeGenerator::SystemCPP) {
    cg.printBasicIncludes();
  }

  // 1. includes
  BOOST_FOREACH(string base, m_bases) {
    ClassScopePtr cls = ar->findClass(base);
    if (cls && cls->isUserClass()) {
      cg_printInclude(cls->getHeaderFilename());
    }
  }

  // 2. Declarations
  cg.namespaceBegin();
  outputCPPForwardHeader(cg, ar);
  cg.setContext(CodeGenerator::CppDeclaration);
  getStmt()->outputCPP(cg, ar);

  cg.namespaceEnd();

  cg.headerEnd(filename);
}

void ClassScope::outputCPPForwardHeader(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  cg.setContext(CodeGenerator::CppForwardDeclaration);

  BOOST_FOREACH(const string &dep, m_usedClassesFullHeader) {
    ClassScopePtr cls = ar->findClass(dep);
    if (cls && cls->isUserClass()) {
      cg_printInclude(cls->getHeaderFilename());
    }
  }

  bool done = false;
  BOOST_FOREACH(const string &str, m_usedLiteralStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    assert(index != -1);
    string lisnam = ar->getLiteralStringName(stringId, index);
    done = true;
    cg_printf("extern StaticString %s;\n", lisnam.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const int64 &val, m_usedScalarVarIntegersHeader) {
    int index = -1;
    int hash = ar->checkScalarVarInteger(val, index);
    assert(index != -1);
    string name = ar->getScalarVarIntegerName(hash, index);
    done = true;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const double &val, m_usedScalarVarDoublesHeader) {
    int index = -1;
    int hash = ar->checkScalarVarDouble(val, index);
    assert(index != -1);
    string name = ar->getScalarVarDoubleName(hash, index);
    done = true;
    cg_printf("extern const VarNR &%s;\n", name.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const string &str, m_usedLitVarStringsHeader) {
    int index = -1;
    int stringId = cg.checkLiteralString(str, index, ar, BlockScopePtr());
    assert(index != -1);
    string lisnam = ar->getLitVarStringName(stringId, index);
    done = true;
    cg_printf("extern VarNR %s;\n", lisnam.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    assert(hash != -1 && index != -1);
    string name = ar->getScalarArrayName(hash, index);
    done = true;
    cg_printf("extern StaticArray %s;\n", name.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const string &str, m_usedDefaultValueScalarVarArrays) {
    int index = -1;
    int hash = ar->checkScalarArray(str, index);
    assert(hash != -1 && index != -1);
    string name = ar->getScalarVarArrayName(hash, index);
    done = true;
    cg_printf("extern VarNR %s;\n", name.c_str());
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const string &str, m_usedConstsHeader) {
    BlockScopeConstPtr block = ar->findConstantDeclarer(str);
    assert(block);
    ConstantTableConstPtr constants = block->getConstants();
    done = true;
    constants->outputSingleConstant(cg, ar, str);
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const UsedClassConst& item, m_usedClassConstsHeader) {
    ClassScopePtr cls = ar->findClass(item.first);
    assert(cls);
    done = true;
    cls->getConstants()->outputSingleConstant(cg, ar, item.second);
  }
  if (done) cg_printf("\n");

  done = false;
  BOOST_FOREACH(const string &str, m_usedClassesHeader) {
    ClassScopePtr usedClass = ar->findClass(str);
    assert(usedClass);
    done = true;
    usedClass->outputForwardDeclaration(cg);
  }
  if (done) cg_printf("\n");
}

void ClassScope::outputCPPSupportMethodsImpl(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  if (isTrait()) return;
  string clsNameStr = getId();
  const char *clsName = clsNameStr.c_str();

  cg.addClass(getName(), getContainingClass());

  if (Option::GenerateCPPMacros) {
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
                    Option::ClassStaticsCallbackPrefix,
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
    outputCPPCallInfoTableSupport(cg, ar, 0, hasRedec);
    outputCPPHelperClassAllocSupport(cg, ar, 0);
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

void ClassScope::outputCPPStaticMethodWrappers(CodeGenerator &cg,
                                               AnalysisResultPtr ar,
                                               set<string> &done,
                                               const char *cls) {
  if (isTrait()) return;
  for (FunctionScopePtrVec::const_iterator it = m_functionsVec.begin();
       it != m_functionsVec.end(); ++it) {
    const string &name = (*it)->getName();
    if (done.find(name) != done.end()) continue;
    MethodStatementPtr m =
      dynamic_pointer_cast<MethodStatement>((*it)->getStmt());
    if (!m) continue; // system classes
    m->outputCPPStaticMethodWrapper(cg, ar, cls);
    done.insert(name);
  }
  if (derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr par = getParentScope(ar);
    if (par) par->outputCPPStaticMethodWrappers(cg, ar, done, cls);
  }
}

void ClassScope::outputCPPGlobalTableWrappersDecl(CodeGenerator &cg,
                                                  AnalysisResultPtr ar) {
  if (isTrait()) return;
  string id = getId();
  cg_printf("extern const %sObjectStaticCallbacks %s%s;\n",
            isRedeclaring() ? "Redeclared" : "",
            Option::ClassStaticsCallbackPrefix, id.c_str());
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
  if (isTrait()) return;
  string id = getId();
  string prop = getClassPropTableId(ar);
  cg_indentBegin("const %sObjectStaticCallbacks %s%s = {\n",
                 isRedeclaring() ? "Redeclared" : "",
                 Option::ClassStaticsCallbackPrefix, id.c_str());
  if (isRedeclaring()) {
    cg_indentBegin("{\n");
  }
  if (isInterface()) {
    cg_printf("0,0,0,0,0,0,0,0,0,0\n");
  } else {
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

    FunctionScopeRawPtr fs = findConstructor(ar, true);
    if (fs && !fs->isAbstract()) {
      cg_printf("&%s%s::%s%s,",
                Option::ClassPrefix, fs->getContainingClass()->getId().c_str(),
                Option::CallInfoPrefix,
                CodeGenerator::FormatLabel(fs->getName()).c_str());
    } else {
      cg_printf("0,");
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
                Option::ClassStaticsCallbackPrefix, par->getId().c_str());
    } else {
      cg_printf("0\n");
    }
  }

  if (isRedeclaring()) {
    cg_indentEnd("},\n");
    cg_printf("%d\n", m_redeclaring);
  }
  cg_indentEnd("};\n");
}

bool ClassScope::addFunction(AnalysisResultConstPtr ar,
                             FunctionScopePtr funcScope) {
  FunctionScopePtr &func = m_functions[funcScope->getName()];
  if (func) {
    throw Exception("Redeclared method %s::%s",
                    getOriginalName().c_str(), func->getOriginalName().c_str());
  }
  func = funcScope;
  m_functionsVec.push_back(funcScope);
  return true;
}

void ClassScope::findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                                      bool staticOnly,
                                      vector<const char *> &funcs) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  // output invoke support methods
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    FunctionScopePtr func = *iter;
    ASSERT(!func->isRedeclaring());
    if (func->isAbstract() ||
        (staticOnly && !func->isStatic()) ||
        !(systemcpp || func->isDynamic() || func->isVirtual())) continue;
    const char *name = func->getName().c_str();
    funcs.push_back(name);
  }
}

void ClassScope::outputCPPMethodInvokeBareObjectSupport(
    CodeGenerator &cg, AnalysisResultPtr ar,
    FunctionScopePtr func, bool fewArgs) {
  if (isTrait()) return;

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
    const StringToFunctionScopePtrMap &funcScopes, bool fewArgs) {
  if (isTrait()) return;
  string id = getId();
  ClassScopePtr self = dynamic_pointer_cast<ClassScope>(shared_from_this());
  for (vector<const char*>::const_iterator it = keys.begin();
      it != keys.end(); ++it) {
    const char *name = *it;
    string lname = CodeGenerator::FormatLabel(name);
    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcScopes.find(name);
    ASSERT(iterFuncs != funcScopes.end());
    FunctionScopePtr func = iterFuncs->second;

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
    if (!fewArgs && func->getMaxParamCount() <= Option::InvokeFewArgsCount &&
        !func->isVariableArgument()) {

      if (Option::InvokeWithSpecificArgs && !func->getMaxParamCount() &&
          !ar->isSystem() && !ar->isSepExtension()) {
        // For functions with no parameter, we can combine the i_ wrapper and
        // the ifa_ wrapper.
        cg_printf("return ((CallInfo::MethInvoker0Args)&%s%s)(mcp, 0);\n",
                  Option::InvokeFewArgsPrefix, lname.c_str());
      } else {
        cg_printf("return invoke_meth_few_handler(mcp, params, &%s%s);\n",
                  Option::InvokeFewArgsPrefix, lname.c_str());
      }
    } else {
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
        cg_printf("%s%s *self ATTRIBUTE_UNUSED "
                  "(static_cast<%s%s*>(mcp.obj));\n",
                  Option::ClassPrefix, id.c_str(),
                  Option::ClassPrefix, id.c_str());
      } else if (func->needsClassParam()) {
        // If mcp contains an object, was a static method
        // invoked instance style.
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
    }
    cg_indentEnd("}\n");
  }
}

void ClassScope::outputCPPMethodInvokeTable(
  CodeGenerator &cg, AnalysisResultPtr ar,
  const vector<const char*> &keys,
  const StringToFunctionScopePtrMap &funcScopes,
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
    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcScopes.find(name);
    FunctionScopePtr func;
    string origName;
    if (iterFuncs == funcScopes.end()) {
      assert(classNameCtor() && !strcmp(name, "__construct"));
      func = findConstructor(ar, false);
      lname = CodeGenerator::FormatLabel(func->getName());
      origName = name;
    } else {
      func = iterFuncs->second;
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
  if (isTrait()) return;
  for (FunctionScopePtrVec::const_iterator iter =
         m_functionsVec.begin(); iter != m_functionsVec.end(); ++iter) {
    FunctionScopePtr func = *iter;
    string id = CodeGenerator::FormatLabel(func->getName());
    bool needsWrapper = func->getName() == "__invoke";
    cg_printf("DECLARE_METHOD_INVOKE_HELPERS(%s);\n", id.c_str());
    if (needsWrapper) {
      cg_printf("DECLARE_METHOD_INVOKE_WRAPPER_HELPERS(%s);\n",
                id.c_str());
    }
  }
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
  bool exist = ar->findClass(origName);
  cg_printf("%s%s(",
            exist ? "checkClassExists" : "autoloadClass",
            noThrow ? "NoThrow" : "Throw");
  cg_printString(origName, ar, bs);
  if (exist) {
    string lwrName(Util::toLower(origName));
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
