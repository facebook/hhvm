/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/depth_first_visitor.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/if_branch_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/loop_statement.h>
#include <compiler/analysis/symbol_table.h>
#include <util/logger.h>
#include <compiler/package.h>
#include <compiler/parser/parser.h>
#include <util/util.h>
#include <util/hash.h>
#include <compiler/option.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/builtin_symbols.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/array_pair_expression.h>
#include <util/process.h>
#include <runtime/base/rtti_info.h>
#include <runtime/base/array/small_array.h>
#include <runtime/ext/ext_json.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// initialization

AnalysisResult::AnalysisResult()
  : BlockScope("Root", "", StatementPtr(), BlockScope::ProgramScope),
    m_package(NULL), m_parseOnDemand(false), m_phase(AnalyzeInclude),
    m_dynamicClass(false), m_dynamicFunction(false),
    m_optCounter(0),
    m_scalarArraysCounter(0), m_paramRTTICounter(0),
    m_insideScalarArray(false), m_inExpression(false),
    m_wrappedExpression(false),
    m_scalarArraySortedAvgLen(0), m_scalarArraySortedIndex(0),
    m_scalarArraySortedSumLen(0), m_scalarArrayCompressedTextSize(0),
    m_system(false) {
  m_dependencyGraph = DependencyGraphPtr(new DependencyGraph());
  m_classForcedVariants[0] = m_classForcedVariants[1] = false;
}

void AnalysisResult::appendExtraCode(const std::string &code) {
  if (m_extraCode.empty()) {
    m_extraCode = "<?php\n";
  }
  m_extraCode += code + "\n";
}

///////////////////////////////////////////////////////////////////////////////
// general functions

void AnalysisResult::addFileScope(FileScopePtr fileScope) {
  ASSERT(fileScope);

  StringToFileScopePtrMap::const_iterator iter =
    m_files.find(fileScope->getName());
  ASSERT(iter == m_files.end());
  m_files[fileScope->getName()] = fileScope;
  vertex_descriptor vertex = add_vertex(m_depGraph);
  fileScope->setVertex(vertex);
  m_fileVertMap[vertex] = fileScope;
  m_fileScopes.push_back(fileScope);
}

bool AnalysisResult::inParseOnDemandDirs(const string &filename) {
  for (size_t i = 0; i < m_parseOnDemandDirs.size(); i++) {
    if (filename.find(m_parseOnDemandDirs[i]) == 0) return true;
  }
  return false;
}

FileScopePtr AnalysisResult::findFileScope(const std::string &name,
                                           bool parseOnDemand) {
  StringToFileScopePtrMap::const_iterator iter = m_files.find(name);
  if (iter != m_files.end()) return iter->second;

  if (!parseOnDemand ||
      Option::PackageExcludeFiles.find(name) !=
      Option::PackageExcludeFiles.end()) {
    return FileScopePtr();
  }

  if (parseOnDemand && (m_parseOnDemand || inParseOnDemandDirs(name)) &&
      m_package && m_package->parse(name.c_str())) {
    iter = m_files.find(name);
    ASSERT(iter != m_files.end());
    return iter->second;
  }
  return FileScopePtr();
}

void AnalysisResult::pushStatement(StatementPtr stmt) {
  m_stmt = stmt;
  m_stmts.push_back(stmt);
}

void AnalysisResult::popStatement() {
  ASSERT(!m_stmts.empty());
  m_stmts.pop_back();
  if (m_stmts.empty()) {
    m_stmt.reset();
  } else {
    m_stmt = m_stmts.back();
  }
}

StatementPtr AnalysisResult::getStatementForSilencer() const {
  // Because of how we parse if/else statements, we need
  // to handle them differently
  if (m_stmt && m_stmt->is(Statement::KindOfIfBranchStatement)) {
    if (m_stmts.size() < 3)
      return StatementPtr();
    // If the current statement is an IfBranchStatement, we want to
    // return the enclosing IfStatement. The parser guarantees that
    // each IfBranchStatement is the grandchild of the enclosing
    // IfStatement.
    ASSERT(m_stmts[m_stmts.size()-3]->is(Statement::KindOfIfStatement));
    return m_stmts[m_stmts.size()-3];
  }
  return m_stmt;
}

ClassScopePtr AnalysisResult::resolveClass(ConstructPtr cs,
                                           std::string &className) {
  // resolving self and parent
  if (className == "self") {
    ClassScopePtr cls = cs->getClassScope();
    if (cls) {
      className = cls->getName();
      return cls;
    }
  } else if (className == "parent") {
    ClassScopePtr cls = cs->getClassScope();
    if (cls && !cls->getParent().empty()) {
      className = cls->getParent();
    }
  }
  return findClass(className);
}

FunctionScopePtr AnalysisResult::findFunction(const std::string &funcName) {
  StringToFunctionScopePtrVecMap::const_iterator bit =
    m_functions.find(funcName);
  if (bit != m_functions.end()) {
    return bit->second[0];
  }
  StringToFunctionScopePtrVecMap::const_iterator iter =
    m_functionDecs.find(funcName);
  if (iter != m_functionDecs.end()) {
    return iter->second.back();
  }
  return FunctionScopePtr();
}

FunctionScopePtr AnalysisResult::findHelperFunction(const std::string &
                                                    funcName) {
  StringToFunctionScopePtrVecMap::const_iterator bit =
    m_helperFunctions.find(funcName);
  if (bit != m_helperFunctions.end()) {
    return bit->second[0];
  }
  return FunctionScopePtr();
}

BlockScopePtr AnalysisResult::findConstantDeclarer(const std::string &name) {
  if (getConstants()->isPresent(name)) return shared_from_this();
  StringToFileScopePtrMap::const_iterator iter = m_constDecs.find(name);
  if (iter != m_constDecs.end()) return iter->second;
  return BlockScopePtr();
}

ClassScopePtr AnalysisResult::findClass(const std::string &name,
                                        FindClassBy by /* = ClassName */) {
  AnalysisResultPtr ar = shared_from_this();
  if (by == PropertyName) return ClassScopePtr();

  string lname = Util::toLower(name);
  if (by == MethodName) {
    StringToClassScopePtrVecMap::iterator iter =
      m_methodToClassDecs.find(lname);
    if (iter != m_methodToClassDecs.end()) {
      if (iter->second.size() == 1) {
        iter->second[0]->findFunction(ar, lname, true)->setDynamic();
        return ClassScopePtr();
      } else {
        // The call to findClass by method name means all these
        // same-named methods should be dynamic since there will
        // be an invoke to call one of them.
        BOOST_FOREACH(ClassScopePtr cls, iter->second) {
          FunctionScopePtr func = cls->findFunction(ar, lname, true);
          // Something fishy here
          if (func) {
            func->setDynamic();
          }
        }
        iter->second.clear();
      }
    }
  } else {
    StringToClassScopePtrMap::const_iterator sysIter =
      m_systemClasses.find(lname);
    if (sysIter != m_systemClasses.end()) return sysIter->second;

    StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.find(lname);
    if (iter != m_classDecs.end() && iter->second.size()) {
      return iter->second.back();
    }
  }
  return ClassScopePtr();
}

const ClassScopePtrVec &AnalysisResult::findRedeclaredClasses
(const std::string &name) {
  ASSERT(name == Util::toLower(name));

  StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.find(name);
  if (iter == m_classDecs.end()) {
    static ClassScopePtrVec empty;
    empty.clear();
    return empty;
  }
  return iter->second;
}

ClassScopePtrVec AnalysisResult::findClasses(const std::string &name) {
  ASSERT(name == Util::toLower(name));

  StringToClassScopePtrMap::const_iterator sysIter =
    m_systemClasses.find(name);
  if (sysIter != m_systemClasses.end()) {
    return ClassScopePtrVec(1, sysIter->second);
  }

  return findRedeclaredClasses(name);
}

bool AnalysisResult::classMemberExists(const std::string &name,
                                       FindClassBy by) {
  if (by == MethodName) {
    return m_methodToClassDecs.find(name) != m_methodToClassDecs.end();
  }
  return m_classDecs.find(name) != m_classDecs.end();
}

ClassScopePtr AnalysisResult::findExactClass(ConstructPtr cs,
                                             const std::string &name) {
  ClassScopePtr cls = findClass(name);
  if (!cls || !cls->isRedeclaring()) return cls;
  std::string lowerName = Util::toLower(name);
  if (ClassScopePtr currentCls = cs->getClassScope()) {
    if (lowerName == currentCls->getName()) {
      return currentCls;
    }
  }
  if (FileScopePtr currentFile = cs->getFileScope()) {
    StatementList &stmts = *currentFile->getStmt();
    for (int i = stmts.getCount(); i--; ) {
      StatementPtr s = stmts[i];
      if (s && s->is(Statement::KindOfClassStatement)) {
        ClassScopePtr scope =
          static_pointer_cast<ClassStatement>(s)->getClassScope();
        if (lowerName == scope->getName()) {
          return scope;
        }
      }
    }
  }
  return ClassScopePtr();
}

bool AnalysisResult::checkClassPresent(ConstructPtr cs,
                                       const std::string &name) {
  if (name == "self" || name == "parent") return true;
  std::string lowerName = Util::toLower(name);
  if (ClassScopePtr currentCls = cs->getClassScope()) {
    if (lowerName == currentCls->getName() ||
        currentCls->derivesFrom(shared_from_this(), lowerName,
                                true, false)) {
      return true;
    }
  }
  if (FileScopePtr currentFile = cs->getFileScope()) {
    StatementList &stmts = *currentFile->getStmt();
    for (int i = stmts.getCount(); i--; ) {
      StatementPtr s = stmts[i];
      if (s && s->is(Statement::KindOfClassStatement)) {
        ClassScopePtr scope =
          static_pointer_cast<ClassStatement>(s)->getClassScope();
        if (lowerName == scope->getName()) {
          return true;
        }
        if (scope->derivesFrom(shared_from_this(), lowerName,
                               true, false)) {
          return true;
        }
      }
    }
  }
  return false;
}

int AnalysisResult::getFunctionCount() const {
  int total = 0;
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    total += iter->second->getFunctionCount();
  }
  return total;
}

int AnalysisResult::getClassCount() const {
  int total = 0;
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    total += iter->second->getClassCount();
  }
  return total;
}

void AnalysisResult::countReturnTypes(std::map<std::string, int> &counts) {
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->countReturnTypes(counts);
  }
}

void AnalysisResult::addNonFinal(const std::string &className) {
  m_nonFinalClasses.insert(className);
}

bool AnalysisResult::isNonFinalClass(const std::string &className) {
  return m_nonFinalClasses.find(className) != m_nonFinalClasses.end();
}

bool AnalysisResult::needStaticArray(ClassScopePtr cls, FunctionScopePtr func) {
  return cls && isNonFinalClass(cls->getName()) && !func->isPrivate();
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool AnalysisResult::declareFunction(FunctionScopePtr funcScope) {
  string fname = funcScope->getName();
  // System functions override
  if (m_functions.find(fname) != m_functions.end()) {
    return false;
  }

  FunctionScopePtrVec &funcs = m_functionDecs[fname];
  if (funcs.size() == 1) {
    funcs[0]->setRedeclaring(0);
  }
  if (funcs.size() > 0 ||
      Option::DynamicInvokeFunctions.find(fname) !=
      Option::DynamicInvokeFunctions.end()) {
    funcScope->setRedeclaring(funcs.size());
  }
  funcs.push_back(funcScope);
  return true;
}

bool AnalysisResult::declareClass(ClassScopePtr classScope) {
  string cname = classScope->getName();
  // System classes override
  if (m_systemClasses.find(cname) != m_systemClasses.end()) {
    return false;
  }
  AnalysisResultPtr ar = shared_from_this();
  ClassScopePtrVec &classes = m_classDecs[cname];
  if (classes.size() == 1) {
    classes[0]->setRedeclaring(ar, 0);
  }
  if (classes.size() > 0) {
    classScope->setRedeclaring(ar, classes.size());
  }

  int mask =
    (m_classForcedVariants[0] ? VariableTable::NonPrivateNonStaticVars : 0) |
    (m_classForcedVariants[1] ? VariableTable::NonPrivateStaticVars : 0);

  if (mask) {
    classScope->getVariables()->forceVariants(ar, mask);
  }
  classes.push_back(classScope);
  return true;
}

void AnalysisResult::declareUnknownClass(const std::string &name) {
  ASSERT(name == Util::toLower(name));
  m_classDecs.operator[](name);
}

bool AnalysisResult::declareConst(FileScopePtr fs, const string &name) {
  if (getConstants()->isPresent(name) ||
      m_constDecs.find(name) != m_constDecs.end()) {
    m_constRedeclared.insert(name);
    return false;
  } else {
    m_constDecs[name] = fs;
    return true;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dependencies

void AnalysisResult::link(FileScopePtr user, FileScopePtr provider) {
  if (user != provider) {
    add_edge(user->vertex(), provider->vertex(), m_depGraph);
  }
}

bool AnalysisResult::addClassDependency(FileScopePtr usingFile,
                                        const std::string &className) {
  if (BuiltinSymbols::s_classes.find(className) !=
      BuiltinSymbols::s_classes.end())
    return true;

  StringToClassScopePtrVecMap::const_iterator iter =
    m_classDecs.find(className);
  if (iter == m_classDecs.end() || iter->second.size() != 1) return false;
  ClassScopePtr classScope = iter->second[0];
  FileScopePtr fileScope = classScope->getContainingFile();
  link(usingFile, fileScope);
  return true;
}
bool AnalysisResult::addFunctionDependency(FileScopePtr usingFile,
                                           const std::string &functionName) {
  if (BuiltinSymbols::s_functions.find(functionName) !=
      BuiltinSymbols::s_functions.end())
    return true;
  StringToFunctionScopePtrVecMap::const_iterator iter =
    m_functionDecs.find(functionName);
  if (iter == m_functionDecs.end() ||
      iter->second.size() != 1) return false;
  FunctionScopePtr functionScope = iter->second[0];
  FileScopePtr fileScope = functionScope->getContainingFile();
  link(usingFile, fileScope);
  return true;
}
bool AnalysisResult::addIncludeDependency(FileScopePtr usingFile,
                                          const std::string &includeFilename) {
  ASSERT(!includeFilename.empty());
  FileScopePtr fileScope = findFileScope(includeFilename, true);
  if (fileScope) {
    link(usingFile, fileScope);
    return true;
  } else {
    return false;
  }
}
bool AnalysisResult::addConstantDependency(FileScopePtr usingFile,
                                           const std::string &constantName) {
  if (m_constants->isPresent(constantName))
    return true;

  StringToFileScopePtrMap::const_iterator iter = m_constDecs.find(constantName);
  if (iter == m_constDecs.end()) return false;
  FileScopePtr fileScope = iter->second;
  link(usingFile, fileScope);
  return true;
}

bool AnalysisResult::isConstantDeclared(const std::string &constName) {
  if (m_constants->isPresent(constName)) return true;
  StringToFileScopePtrMap::const_iterator iter = m_constDecs.find(constName);
  if (iter == m_constDecs.end()) return false;
  FileScopePtr fileScope = iter->second;
  ConstantTablePtr constants = fileScope->getConstants();
  ConstructPtr decl = constants->getValue(constName);
  if (decl) return true;
  return false;
}

bool AnalysisResult::isBaseSysRsrcClass(const std::string &className) {
  return m_baseSysRsrcClasses.find(className) != m_baseSysRsrcClasses.end();
}

bool AnalysisResult::isConstantRedeclared(const std::string &constName) {
  return m_constRedeclared.find(constName) != m_constRedeclared.end();
}

bool AnalysisResult::isSystemConstant(const std::string &constName) {
  return m_constants->isSystem(constName);
}

///////////////////////////////////////////////////////////////////////////////
// Program

void AnalysisResult::loadBuiltinFunctions() {
  AnalysisResultPtr ar = shared_from_this();
  BuiltinSymbols::LoadFunctions(ar, m_functions);
  BuiltinSymbols::LoadHelperFunctions(ar, m_helperFunctions);
}

void AnalysisResult::loadBuiltins() {
  AnalysisResultPtr ar = shared_from_this();
  BuiltinSymbols::LoadFunctions(ar, m_functions);
  BuiltinSymbols::LoadHelperFunctions(ar, m_helperFunctions);
  BuiltinSymbols::LoadClasses(ar, m_systemClasses);
  BuiltinSymbols::LoadVariables(ar, m_variables);
  BuiltinSymbols::LoadConstants(ar, m_constants);
  BuiltinSymbols::LoadBaseSysRsrcClasses(ar, m_baseSysRsrcClasses);
}

void AnalysisResult::checkClassDerivations() {
  AnalysisResultPtr ar = shared_from_this();
  ClassScopePtr cls;
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(cls, iter->second) {
      hphp_string_set seen;
      cls->checkDerivation(ar, seen);
    }
  }
}

void AnalysisResult::analyzeProgram(bool system /* = false */) {
  AnalysisResultPtr ar = shared_from_this();

  if (system) m_system = true;
  getVariables()->forceVariants(ar, VariableTable::AnyVars);
  getVariables()->setAttribute(VariableTable::ContainsLDynamicVariable);
  getVariables()->setAttribute(VariableTable::ContainsExtract);

  checkClassDerivations();

  // Analyze Includes
  Logger::Verbose("Analyzing Includes");
  setPhase(AnalysisResult::AnalyzeInclude);
  unsigned int i;
  for (i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }
  if (!m_extraCode.empty()) {
    m_extraCodeFileName = Option::LambdaPrefix + "lambda";
    Compiler::Parser::ParseString(m_extraCode.c_str(), ar,
                                  m_extraCodeFileName.c_str());
  }
  for (; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }

  for (set<string>::const_iterator it = Option::VolatileClasses.begin();
       it != Option::VolatileClasses.end(); ++it) {
    ClassScopePtr cls = findClass(Util::toLower(*it));
    if (cls) {
      cls->setVolatile();
    }
  }

  // I think we need one more round of checking, as new includes may bring in
  // more classes.
  checkClassDerivations();

  // Analyze All
  Logger::Verbose("Analyzing All");
  setPhase(AnalysisResult::AnalyzeAll);
  for (i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }

  // Collect methods
  string cname;
  ClassScopePtr cls;
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(cls, iter->second) {
      if (cls->isRedeclaring()) {
        cls->setStaticDynamic(ar);
      }
      StringToFunctionScopePtrMap methods;
      cls->collectMethods(ar, methods);
      bool needAbstractMethodImpl =
        (!cls->isAbstract() && !cls->isInterface());
      for (StringToFunctionScopePtrMap::const_iterator iterMethod =
           methods.begin(); iterMethod != methods.end(); ++iterMethod) {
        FunctionScopePtr func = iterMethod->second;
        if (!func->hasImpl() && needAbstractMethodImpl) {
          FunctionScopePtr tmpFunc =
            cls->findFunction(ar, func->getName(), true, true);
          if (!tmpFunc || !tmpFunc->hasImpl()) {
            Compiler::Error(Compiler::MissingAbstractMethodImpl,
                            cls->getStmt(), func->getStmt());
          }
        }
        m_methodToClassDecs[iterMethod->first].push_back(cls);
      }
    }
  }
  BOOST_FOREACH(tie(cname, cls), m_systemClasses) {
    StringToFunctionScopePtrMap methods;
    cls->collectMethods(ar, methods);
    for (StringToFunctionScopePtrMap::const_iterator iterMethod =
           methods.begin(); iterMethod != methods.end(); ++iterMethod) {
      m_methodToClassDecs[iterMethod->first].push_back(cls);
    }
  }

  // Analyze perfect virtuals
  if (Option::AnalyzePerfectVirtuals && !system) {
    analyzePerfectVirtuals();
  }
}

static void addClassRootMethods(AnalysisResultPtr ar, ClassScopePtr cls,
                                hphp_string_set &methods) {
  const StringToFunctionScopePtrVecMap &funcs = cls->getFunctions();
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         funcs.begin(); iter != funcs.end(); ++iter) {
    ClassScopePtrVec roots;
    cls->getRootParents(ar, iter->first, roots, cls);
    for (unsigned int i = 0; i < roots.size(); i++) {
      methods.insert(roots[i]->getName() + "::" + iter->first);
    }
  }
}

static void addClassRootMethods(AnalysisResultPtr ar, ClassScopePtr cls,
                                StringToFunctionScopePtrVecMap &methods) {
  const StringToFunctionScopePtrVecMap &funcs = cls->getFunctions();
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         funcs.begin(); iter != funcs.end(); ++iter) {
    ClassScopePtr root = cls->getRootParent(ar, iter->first);
    string cluster = root->getName() + "::" + iter->first;
    FunctionScopePtrVec &fs = methods[cluster];
    fs.insert(fs.end(), iter->second.begin(), iter->second.end());
  }
}

void AnalysisResult::analyzePerfectVirtuals() {
  AnalysisResultPtr ar = shared_from_this();

  StringToFunctionScopePtrVecMap methods;
  hphp_string_set redeclaringMethods;
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      ClassScopePtr cls = iter->second[i];

      // being conservative, not to do redeclaring classes at all
      if (cls->derivesFromRedeclaring()) {
        addClassRootMethods(ar, cls, redeclaringMethods);
        continue;
      }

      // classes derived from system or extension classes can be complicated
      ClassScopePtr root = cls->getRootParent(ar);
      if (!root->isUserClass() || root->isExtensionClass()) continue;

      // cluster virtual methods by a root parent that also defined the method
      addClassRootMethods(ar, cls, methods);
    }
  }
    // if ANY class in the hierarchy is a reclaring one, ignore
  for (hphp_string_set::const_iterator iter = redeclaringMethods.begin();
       iter != redeclaringMethods.end(); ++iter) {
    methods.erase(*iter);
  }
  for (StringToFunctionScopePtrVecMap::const_iterator iter = methods.begin();
       iter != methods.end(); ++iter) {
    // if it's unique, ignore
    const FunctionScopePtrVec &funcs = iter->second;
    if (funcs.size() < 2) {
      continue;
    }

    bool perfect = true;
    for (unsigned int i = 1; i < funcs.size(); i++) {
      if (!funcs[0]->matchParams(funcs[i])) {
        perfect = false;
        break;
      }
    }
    if (perfect) {
      for (unsigned int i = 0; i < funcs.size(); i++) {
        funcs[i]->setPerfectVirtual();
      }
    }
  }
}

void AnalysisResult::analyzeProgramFinal() {
  AnalysisResultPtr ar = shared_from_this();
  setPhase(AnalysisResult::AnalyzeFinal);
  for (uint i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }
  setPhase(AnalysisResult::CodeGen);
}

static void dumpVisitor(AnalysisResultPtr ar, StatementPtr s, void *data) {
  s->dump(0, ar);
}

void AnalysisResult::dump() {
  visitFiles(dumpVisitor, 0);
  fflush(0);
}

void AnalysisResult::visitFiles(void (*cb)(AnalysisResultPtr,
                                           StatementPtr, void*), void *data) {
  AnalysisResultPtr ar = shared_from_this();
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    FileScopePtr file = iter->second;

    file->visit(ar, cb, data);
  }
}

void AnalysisResult::getFuncScopesSet(BlockScopeRawPtrQueue &v,
                                      FunctionContainerPtr fc) {
  const StringToFunctionScopePtrVecMap &funcMap = fc->getFunctions();
  for (StringToFunctionScopePtrVecMap::const_iterator
         iter = funcMap.begin(), end = funcMap.end();
       iter != end; ++iter) {
    for (FunctionScopePtrVec::const_iterator it = iter->second.begin(),
           e = iter->second.end(); it != e; ++it) {
      FunctionScopePtr f = *it;
      if (f->isUserFunction()) {
        v.push_back(f);
      }
    }
  }
}

void AnalysisResult::getScopesSet(BlockScopeRawPtrQueue &v) {
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    FileScopePtr file = iter->second;
    getFuncScopesSet(v, file);
  }

  for (StringToClassScopePtrVecMap::iterator iter = m_classDecs.begin(),
         end = m_classDecs.end(); iter != end; ++iter) {
    for (ClassScopePtrVec::iterator it = iter->second.begin(),
           e = iter->second.end(); it != e; ++it) {
      ClassScopePtr cls = *it;
      if (cls->isUserClass()) {
        v.push_back(cls);
        getFuncScopesSet(v, cls);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// optimization functions

namespace HPHP {

struct PreOptVisitor {
  PreOptVisitor(AnalysisResultPtr ar) : m_ar(ar) {}

  AnalysisResultPtr m_ar;
};

template<>
int DepthFirstVisitor<PreOptVisitor>::visit(BlockScopeRawPtr scope) {
  int optCount = m_optCounter + this->m_data.m_ar->getOptCounter();
  StatementPtr stmt = scope->getStmt();
  int flags = 0;
  if (Option::LocalCopyProp | Option::EliminateDeadCode) {
    if (MethodStatementPtr m =
        dynamic_pointer_cast<MethodStatement>(stmt)) {
      int flag;
      while (true) {
        AliasManager am;
        flag = am.optimize(this->m_data.m_ar, m);
        if (!flag) break;
        flags |= BlockScope::UseKindAny;
      }
    }
  }

  StatementPtr rep = this->visitStmtRecur(stmt);
  assert(!rep);
  if (optCount != m_optCounter + this->m_data.m_ar->getOptCounter()) {
    flags |= BlockScope::UseKindAny;
  }

  return flags;
}

template<>
ExpressionPtr DepthFirstVisitor<PreOptVisitor>::visit(ExpressionPtr e) {
  return e->preOptimize(this->m_data.m_ar);
}

template<>
StatementPtr DepthFirstVisitor<PreOptVisitor>::visit(StatementPtr stmt) {
  return stmt->preOptimize(this->m_data.m_ar);
}

ExpressionPtr AnalysisResult::preOptimize(ExpressionPtr e) {
  DepthFirstVisitor<PreOptVisitor> dfv(shared_from_this());
  return dfv.visitExprRecur(e);
}

void AnalysisResult::preOptimize() {
  setPhase(FirstPreOptimize);
  DepthFirstVisitor<PreOptVisitor> dfv(shared_from_this());
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);
  dfv.visitDepthFirst(scopes);
}

struct InferTypesVisitor {
  InferTypesVisitor(AnalysisResultPtr ar) : m_ar(ar) {}

  AnalysisResultPtr m_ar;
};

template<>
int DepthFirstVisitor<InferTypesVisitor>::visitScope(BlockScopeRawPtr scope) {
  do {
    scope->clearUpdated();
    StatementPtr stmt = scope->getStmt();
    if (MethodStatementPtr m =
        dynamic_pointer_cast<MethodStatement>(stmt)) {
      m->inferFunctionTypes(this->m_data.m_ar);
    } else {
      for (int i = 0, n = stmt->getKidCount(); i < n; i++) {
        StatementPtr kid(
          dynamic_pointer_cast<Statement>(stmt->getNthKid(i)));
        if (kid) {
          kid->inferTypes(this->m_data.m_ar);
        }
      }
    }

    scope->incPass();
  } while (scope->getUpdated());

  return 0;
}

void AnalysisResult::inferTypes() {
  setPhase(FirstInference);
  DepthFirstVisitor<InferTypesVisitor> dfv(shared_from_this());
  BlockScopeRawPtrQueue changed;

  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);
  for (BlockScopeRawPtrQueue::iterator it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    (*it)->setChangedScopes(&changed);
  }
  dfv.visitDepthFirst(scopes);

  setPhase(LastInference);
  getScopesSet(scopes);
  for (BlockScopeRawPtrQueue::iterator it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    dfv.visitScope(*it);
    (*it)->setChangedScopes(0);
  }
}

struct PostOptVisitor {
  PostOptVisitor(AnalysisResultPtr ar) : m_ar(ar) {}

  AnalysisResultPtr m_ar;
};

template<>
int DepthFirstVisitor<PostOptVisitor>::visit(BlockScopeRawPtr scope) {
  int optCount = m_optCounter + this->m_data.m_ar->getOptCounter();
  StatementPtr stmt = scope->getStmt();
  int flags = 0;
  if (Option::LocalCopyProp |
      Option::EliminateDeadCode |
      Option::StringLoopOpts) {
    if (MethodStatementPtr m =
        dynamic_pointer_cast<MethodStatement>(stmt)) {
      int flag;
      while (true) {
        AliasManager am;
        flag = am.optimize(this->m_data.m_ar, m);
        if (!flag) break;
        flags |= BlockScope::UseKindAny;
      }
    }
  }

  StatementPtr rep = this->visitStmtRecur(stmt);
  assert(!rep);
  if (optCount != m_optCounter + this->m_data.m_ar->getOptCounter()) {
    flags |= BlockScope::UseKindAny;
  }

  return flags;
}

template<>
ExpressionPtr DepthFirstVisitor<PostOptVisitor>::visit(ExpressionPtr e) {
  return e->postOptimize(this->m_data.m_ar);
}

template<>
StatementPtr DepthFirstVisitor<PostOptVisitor>::visit(StatementPtr stmt) {
  return stmt->postOptimize(this->m_data.m_ar);
}

void AnalysisResult::postOptimize() {
  setPhase(AnalysisResult::PostOptimize);
  DepthFirstVisitor<PostOptVisitor> dfv(shared_from_this());
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);
  dfv.visitDepthFirst(scopes);
}

}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

int AnalysisResult::registerScalarArray(FileScopePtr scope,
                                        ExpressionPtr pairs, int &hash,
                                        int &index, string &text) {
  int id = -1;
  hash = -1;
  index = -1;
  bool found = false;
  if (!Option::ScalarArrayOptimization || m_insideScalarArray) {
    return -1;
  }

  if (pairs) {
    // Normal picked PHP wouldn't work here, because predefined constants,
    // e.g., __CLASS__, need to be translated.
    text = pairs->getText(false, true);
  }
  std::map<std::string, int>::const_iterator iter = m_scalarArrays.find(text);
  if (iter != m_scalarArrays.end()) {
    id = iter->second;
    found = true;
  } else {
    id = m_scalarArraysCounter++;
    m_scalarArrays[text] = id;
    m_scalarArrayIds.push_back(pairs);
  }
  if (Option::UseNamedScalarArray) {
    hash = hash_string(text.data(), text.size());
    if (hash < 0) hash = -hash;
    vector<string> &strings = m_namedScalarArrays[hash];
    unsigned int i = 0;
    for (; i < strings.size(); i++) {
      if (strings[i] == text) break;
    }
    if (i == strings.size()) {
      assert(!found);
      strings.push_back(text);
    }
    index = i;
    scope->addUsedScalarArray(text);
  }
  return id;
}

int AnalysisResult::checkScalarArray(const string &text, int &index) {
  assert(Option::ScalarArrayOptimization && Option::UseNamedScalarArray);
  int hash = hash_string(text.data(), text.size());
  if (hash < 0) hash = -hash;
  vector<string> &strings = m_namedScalarArrays[hash];
  unsigned int i = 0;
  for (; i < strings.size(); i++) {
    if (strings[i] == text) break;
  }
  assert(i < strings.size());
  index = i;
  return hash;
}

int AnalysisResult::getScalarArrayId(const string &text) {
  std::map<std::string, int>::const_iterator iter = m_scalarArrays.find(text);
  assert(iter != m_scalarArrays.end());
  return iter->second;
}

void AnalysisResult::outputCPPNamedScalarArrays(const std::string &file) {
  AnalysisResultPtr ar = shared_from_this();
  string filename = file + ".h";
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  if (Option::SystemGen) {
    cg_printf("#ifndef __GENERATED_gen_sys_scalar_arrays_h__\n");
    cg_printf("#define __GENERATED_gen_sys_scalar_arrays_h__\n");
  } else {
    cg_printf("#ifndef __GENERATED_sys_scalar_arrays_h__\n");
    cg_printf("#define __GENERATED_sys_scalar_arrays_h__\n");
  }
  cg_printInclude("<runtime/base/hphp.h>");
  if (!Option::SystemGen) {
    cg_printInclude("<sys/global_variables.h>");
  }
  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarArrays.begin(); it != m_namedScalarArrays.end();
       it++) {
    int hash = it->first;
    vector<string> &strings = m_namedScalarArrays[hash];
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarArrayName(hash, i);
      cg_printf("extern StaticArray %s;\n", name.c_str());
    }
  }
  cg.namespaceEnd();
  if (Option::SystemGen) {
    cg_printf("#endif // __GENERATED_gen_sys_scalar_arrays_h__\n");
  } else {
    cg_printf("#endif // __GENERATED_sys_scalar_arrays_h__\n");
  }
  f.close();

  filename = file + ".no.cpp";
  f.open(filename.c_str());
  cg_printf("\n");
  cg_printInclude(string(Option::SystemFilePrefix) + "scalar_arrays.h");
  cg_printf("\n");
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarArrays.begin(); it != m_namedScalarArrays.end();
       it++) {
    int hash = it->first;
    vector<string> &strings = m_namedScalarArrays[hash];
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarArrayName(hash, i);
      cg_printf("StaticArray %s;\n", name.c_str());
    }
  }
  cg_printf("\n");
  const char *clsname =
    Option::SystemGen ? "SystemScalarArrays" : "ScalarArrays";
  const char *prefix = Option::SystemGen ? Option::SystemScalarArrayName :
    Option::ScalarArrayName;
  cg_indentBegin("void %s::initializeNamed() {\n", clsname);
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarArrays.begin(); it != m_namedScalarArrays.end();
       it++) {
    int hash = it->first;
    vector<string> &strings = m_namedScalarArrays[hash];
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarArrayName(hash, i);
      int id = getScalarArrayId(strings[i]);
      cg_printf("%s = %s[%d];\n", name.c_str(), prefix, id);
    }
  }
  cg_indentEnd("}\n");
  cg.namespaceEnd();
}

bool AnalysisResult::getInsideScalarArray() {
  return m_insideScalarArray;
}

void AnalysisResult::setInsideScalarArray(bool flag) {
  m_insideScalarArray = flag;
}

string AnalysisResult::prepareFile(const char *root, const string &fileName,
                                   bool chop) {
  string fullPath = root;
  if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/') {
    fullPath += "/";
  }

  string file = fileName;
  size_t npos = file.rfind('/');
  if (npos != string::npos) {
    file = file.substr(npos + 1);
  }

  if (chop && file.size() > 4 && file.substr(file.length() - 4) == ".php") {
    fullPath += file.substr(0, file.length() - 4);
  } else {
    fullPath += file;
  }
  for (int pos = strlen(root); pos < (int)fullPath.size(); pos++) {
    if (fullPath[pos] == '/') {
      mkdir(fullPath.substr(0, pos).c_str(), 0777);
    }
  }
  return fullPath;
}

void AnalysisResult::forceClassVariants(ClassScopePtr curScope, bool doStatic) {
  if (curScope) {
    curScope->getVariables()->forceVariants(
      shared_from_this(), VariableTable::GetVarClassMask(true, doStatic));
  }

  if (m_classForcedVariants[doStatic]) {
    return;
  }
  m_classForcedVariants[doStatic] = true;

  AnalysisResultPtr ar = shared_from_this();
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      cls->getVariables()->forceVariants(
        ar, VariableTable::GetVarClassMask(false, doStatic));
    }
  }
}

void AnalysisResult::forceClassVariants(const std::string &name,
                                        ClassScopePtr curScope, bool doStatic) {
  if (curScope) {
    curScope->getVariables()->forceVariant(
      shared_from_this(), name, VariableTable::GetVarClassMask(true, doStatic));
  }
  if (m_classForcedVariants[doStatic]) {
    return;
  }

  AnalysisResultPtr ar = shared_from_this();
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      cls->getVariables()->forceVariant(
        ar, name, VariableTable::GetVarClassMask(false, doStatic));
    }
  }
}

bool AnalysisResult::outputAllPHP(CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  switch (output) {
  case CodeGenerator::PickledPHP:
    for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
         iter != m_files.end(); ++iter) {
      string fullPath = prepareFile(m_outputPath.c_str(), iter->first, false);
      ofstream f(fullPath.c_str());
      if (f) {
        CodeGenerator cg(&f, output);
        cg_printf("<?php\n");
        Logger::Info("Generating %s...", fullPath.c_str());
        iter->second->getStmt()->outputPHP(cg, ar);
        f.close();
      } else {
        Logger::Error("Unable to open %s for write", fullPath.c_str());
      }
    }
    return true; // we are done
  default:
    ASSERT(false);
  }

  return true;
}

void AnalysisResult::getTrueDeps(FileScopePtr f,
                                 map<string, FileScopePtr> &trueDeps) {
  adjacency_iterator adj, adjEnd;
  for (tie(adj, adjEnd) = adjacent_vertices(f->vertex(), m_depGraph);
       adj != adjEnd; adj++) {
    VertexToFileScopePtrMap::const_iterator iter = m_fileVertMap.find(*adj);
    ASSERT(iter != m_fileVertMap.end());
    trueDeps[iter->second->getName()] = iter->second;
  }
}

void AnalysisResult::clusterByFileSizes(StringToFileScopePtrVecMap &clusters,
                                        int clusterCount) {
  ASSERT(clusterCount > 0);

  std::map<std::string, FileScopePtr> sortedFiles;
  long totalSize = 0;
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    totalSize += f->getSize();
    sortedFiles[f->getName()] = f;
  }

  const int FUZZYNESS = 1024; // 1kB

  int clusterSize = totalSize / clusterCount;
  int size = 0;
  int count = 1;
  string clusterName = Option::FormatClusterFile(count);
  FileScopePtrVec largeFiles;
  for (std::map<std::string, FileScopePtr>::const_iterator iter =
         sortedFiles.begin(); iter != sortedFiles.end(); ++iter) {
    FileScopePtr f = iter->second;
    if (f->getSize() > clusterSize) {
      largeFiles.push_back(f);
    } else {
      size += f->getSize();
      if ((size / FUZZYNESS) > (clusterSize / FUZZYNESS)) {
        clusterName = Option::FormatClusterFile(++count);
        size = f->getSize();
      }
      clusters[clusterName].push_back(f);
    }
  }
  for (unsigned int i = 0; i < largeFiles.size(); i++) {
    clusters[Option::FormatClusterFile(++count)].push_back(largeFiles[i]);
  }
}

void AnalysisResult::repartitionCPP(const string &filename, int64 targetSize,
                                    bool insideHPHP, bool force) {
  if (!force) {
    struct stat results;
    if (stat(filename.c_str(), &results)) {
      // error, shouldn't happen
      return;
    }
    int64 size = (int64)results.st_size;
    if (size <= targetSize * 2) {
      // tolerable size
      return;
    }
  }

  // for updating source info
  map<string, map<int, LocationPtr> >::iterator fileInfoIter =
    m_sourceInfos.find(filename);
  bool inSourceInfos = fileInfoIter != m_sourceInfos.end();
  map<int, LocationPtr> linemap;
  if (inSourceInfos) {
    linemap.swap(fileInfoIter->second);
    m_sourceInfos.erase(fileInfoIter);
  }
  map<int, LocationPtr>::const_iterator lineIter = linemap.begin();
  int origLine = 0;
  int newLine = 0;

  vector<string> includes;
  vector<string> preface;
  bool inPreface = false;
  string line;
  int64 current = 0;
  int seq = 0;
  ifstream fin(filename.c_str());
  string base = filename.substr(0, filename.length() - 4);
  char foutName[PATH_MAX];
  snprintf(foutName, sizeof(foutName), "%s-%d.cpp", base.c_str(), seq);
  ofstream fout(foutName);
  while (getline(fin, line)) {
    fout << line << endl;

    origLine++;
    newLine++;
    if (lineIter != linemap.end() && origLine == lineIter->first) {
      m_sourceInfos[foutName][newLine] = lineIter->second;
      ++lineIter;
    }

    current += line.length() + 1;
    if (inPreface) preface.push_back(line);

    if (line[0] == '#') {
      includes.push_back(line);
    } else if (line == "/* preface starts */") {
      inPreface = true;
      preface.clear();
      preface.push_back(line);
    } else if (line == "/* preface finishes */") {
      inPreface = false;
    } else if (line == CodeGenerator::SPLITTER_MARKER) {
      // a possible cut point
      if (current > targetSize) {
        if (insideHPHP) {
          // namespace HPHP
          fout << "}" << endl;
        }

        fout.close();
        snprintf(foutName, sizeof(foutName),
                 "%s-%d.cpp", base.c_str(), ++seq);
        fout.open(foutName);
        newLine = 0;
        current = 0;
        for (unsigned int j = 0; j < includes.size(); j++) {
          fout << includes[j] << endl;
          newLine++;
        }
        if (insideHPHP) {
          fout << "namespace HPHP {" << endl;
          newLine++;
        }
        for (unsigned int j = 0; j < preface.size(); j++) {
          fout << preface[j] << endl;
          newLine++;
        }
      }
    }
  }
  fout.close();

  fin.close();
  remove(filename.c_str());
}

void AnalysisResult::repartitionLargeCPP(const vector<string> &filenames,
                                         const vector<string> &additionals) {
  std::map<std::string, int> ppp;
  if (!Option::PreprocessedPartitionConfig.empty()) {
    Hdf pppc(Option::PreprocessedPartitionConfig);
    for (Hdf node = pppc["splits"].firstChild(); node.exists();
         node = node.next()) {
      ppp[getOutputPath() + "/" + node["name"].getString()] =
        node["count"].getInt32();
    }
  }

  int64 totalSize = 0;
  int count = 0;
  for (unsigned int i = 0; i < filenames.size(); i++) {
    struct stat results;
    if (stat(filenames[i].c_str(), &results) == 0) {
      totalSize += results.st_size;
      count++;
    }
  }
  int64 averageSize = totalSize / count;
  for (unsigned int i = 0; i < filenames.size(); i++) {
    const string &filename = filenames[i];

    if (Option::PreprocessedPartitionConfig.empty()) {
      repartitionCPP(filename, averageSize, true, false);
    } else {
      int count = ppp[filename];
      if (count > 1) {
        struct stat results;
        if (stat(filename.c_str(), &results) == 0) {
          repartitionCPP(filename, results.st_size / count, true, true);
        }
      }
    }
  }
  for (unsigned int i = 0; i < additionals.size(); i++) {
    repartitionCPP(additionals[i], averageSize, false, false);
  }
}

void AnalysisResult::outputAllCPP(CodeGenerator::Output output,
                                  int clusterCount,
                                  const std::string *compileDir) {
  if (output == CodeGenerator::SystemCPP) {
    Option::GenerateCPPMain = false;
  }

  FileScopePtrVec trueDeps;
  StringToFileScopePtrVecMap clusters;
  if (clusterCount > 0) {
    clusterByFileSizes(clusters, clusterCount);
  } else {
    BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
      clusters[f->outputFilebase()].push_back(f);
    }
  }

  if (Option::GenerateCPPMacros && output != CodeGenerator::SystemCPP) {
    // system functions are currently unchanged
    createGlobalFuncTable();
  }

  vector <string> filenames;

  AnalysisResultPtr ar = shared_from_this();
  MethodSlot::genMethodSlot(ar);
  string root = getOutputPath() + "/";
  for (StringToFileScopePtrVecMap::const_iterator iter = clusters.begin();
       iter != clusters.end(); ++iter) {
    // for each cluster, generate one implementation file
    Util::mkdir(root + iter->first);
    string filename = root + iter->first + ".cpp";
    filenames.push_back(filename);
    ofstream f(filename.c_str());
    if (compileDir) {
      // this is the file that will be compiled, so we need to use this
      // for source info:
      filename = *compileDir + "/" + iter->first + ".cpp";
    }
    CodeGenerator cg(&f, output, &filename);
    outputCPPClusterImpl(cg, iter->second);

    // for each file, generate one header and a list of class headers
    BOOST_FOREACH(FileScopePtr fs, iter->second) {
      string fileBase = fs->outputFilebase();
      Util::mkdir(root + fileBase);
      string header = fileBase + ".h";
      string fwheader = fileBase + ".fw.h";
      string fwsheader = fileBase + ".fws.h";
      string fileHeader = root + header;
      string fwFileHeader = root + fwheader;
      string fwsFileHeader = root + fwsheader;
      {
        ofstream f(fileHeader.c_str());
        CodeGenerator cg(&f, output);
        fs->outputCPPDeclHeader(cg, ar);
        f.close();
      }
      fs->outputCPPClassHeaders(cg, ar, output);
      {
        ofstream f(fwFileHeader.c_str());
        CodeGenerator cg(&f, output);
        fs->outputCPPForwardDeclHeader(cg, ar);
        f.close();
      }
      {
        ofstream f(fwsFileHeader.c_str());
        CodeGenerator cg(&f, output);
        fs->outputCPPForwardStaticDecl(cg, ar);
        f.close();
      }
    }
  }

  if (Option::GenerateCPPMacros) {
    outputCPPDynamicTables(output);
  }
  if (output == CodeGenerator::SystemCPP) {
    // this calls outputCPPScalarArrays, so must happen
    // after function bodies have been output
    outputCPPSystem();
    string file = m_outputPath + "/" + Option::SystemFilePrefix +
      "literal_strings";
    outputCPPNamedLiteralStrings(false, file);
    if (Option::UseNamedScalarArray) {
      string file = m_outputPath + "/" + Option::SystemFilePrefix +
        "scalar_arrays";
      outputCPPNamedScalarArrays(file);
    }
  } else if (Option::GenerateCPPMain) {
    outputCPPGlobalDeclarations();
    outputCPPMain();
    outputCPPScalarArrays(false);
    outputCPPGlobalVariablesMethods(1);
    outputCPPGlobalVariablesMethods(2);
    outputCPPGlobalVariablesMethods(3);
    outputCPPGlobalVariablesMethods(4);
    string file = m_outputPath + "/" + Option::SystemFilePrefix +
      "literal_strings";
    outputCPPNamedLiteralStrings(false, file);
    if (Option::UseNamedScalarArray) {
      string file = m_outputPath + "/" + Option::SystemFilePrefix +
        "scalar_arrays";
      outputCPPNamedScalarArrays(file);
    }
    outputCPPGlobalState();
    outputCPPFiberGlobalState();
    outputCPPSepExtensionMake();
  }
  if (!Option::SystemGen) outputCPPUtilDecl(output);
  if (Option::GenerateCPPMacros && output != CodeGenerator::SystemCPP) {
    outputCPPClassMapFile();
    outputCPPNameMaps();
  }

  vector<string> additionalCPPs;
  if (Option::GenerateFFI) {
    outputCPPFFIStubs();
    additionalCPPs.push_back(m_outputPath + "/" + Option::FFIFilePrefix +
                             "stubs.cpp");
    outputHSFFIStubs();
    outputJavaFFIStubs();
    outputJavaFFICppImpl();
    additionalCPPs.push_back(m_outputPath + "/" + Option::FFIFilePrefix +
                             "java_stubs.cpp");
    outputJavaFFICppDecl();
    outputSwigFFIStubs();
  }

  if (clusterCount > 0) repartitionLargeCPP(filenames, additionalCPPs);

  if (Option::GenerateCPPMacros && output != CodeGenerator::SystemCPP) {
    outputCPPSourceInfos();
  }

  if (Option::GenRTTIProfileData) {
    outputRTTIMetaData(Option::RTTIOutputFile.c_str());
  }

  if (!Option::SystemGen) outputCPPUtilImpl(output);
}

void AnalysisResult::outputAllCPP(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();
  cg.setOutput(CodeGenerator::MonoCPP);
  cg.setContext(CodeGenerator::CppImplementation);

  if (Option::GenerateCPPMain) {
    cg_printf("\n");
    cg_printInclude("<runtime/base/hphp.h>");
    cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
    cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
    cg_printf("\n");
  }

  cg.namespaceBegin();
  getVariables()->setAttribute(VariableTable::ForceGlobal);
  getVariables()->outputCPP(cg, ar);
  getVariables()->clearAttribute(VariableTable::ForceGlobal);

  cg.setContext(CodeGenerator::CppImplementation);
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputCPPImpl(cg, ar);
  }
  cg.setContext(CodeGenerator::CppPseudoMain);
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputCPPPseudoMain(cg, ar);
  }
  cg.setContext(CodeGenerator::CppForwardDeclaration);
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputCPPForwardDeclarations(cg, ar);
  }
  cg.setContext(CodeGenerator::CppDeclaration);
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputCPPDeclarations(cg, ar);
  }
  cg.namespaceEnd();

  if (Option::GenerateCPPMain) {
    cg_printf("#ifndef HPHP_BUILD_LIBRARY\n");
    cg_indentBegin("int main(int argc, char** argv) {\n");
    cg_printf("return HPHP::execute_program(argc, argv);\n");
    cg_indentEnd("}\n");
    cg_printf("#endif\n");
  }
}

void AnalysisResult::outputCPPExtClassImpl(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();

  // output create proxies
  vector<const char *> classes;
  StringToClassScopePtrVecMap merged(m_classDecs);
  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    ClassScopePtr cls = iter->second;
    bool extension = cls->getAttribute(ClassScope::Extension);
    if (cls->isInterface()) continue;

    classes.push_back(cls->getOriginalName().c_str());
    merged[cls->getName()].push_back(cls);
    cls->outputCPPDynamicClassImpl(cg, ar);
    if (extension) {
      cls->outputCPPSupportMethodsImpl(cg, ar);
    }
  }

  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    if (iter->second.empty()) {
      throw Exception("During code gen, class %s is undefined",
                      iter->first.c_str());
    }
    for (ClassScopePtrVec::const_iterator iter2 = iter->second.begin();
         iter2 != iter->second.end(); ++iter2) {
      ClassScopePtr cls = *iter2;
      if (!cls->isInterface()) {
        classes.push_back(cls->getOriginalName().c_str());
        break;
      }
    }
  }

  ClassScope::outputCPPClassVarInitImpl(cg, merged, classes);
  ClassScope::outputCPPDynamicClassCreateImpl(cg, merged, classes);
  ClassScope::outputCPPInvokeStaticMethodImpl(cg, merged, classes);
  ClassScope::outputCPPGetCallInfoStaticMethodImpl(cg, merged, classes);
  ClassScope::outputCPPGetStaticPropertyImpl(cg, merged, classes);
  ClassScope::outputCPPGetClassConstantImpl(cg, merged, classes);
}

void AnalysisResult::outputCPPClassMapFile() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "class_map.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  cg_printf("\n");

  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();
  outputCPPClassMap(cg);
  cg.namespaceEnd();
  f.close();
}

void AnalysisResult::recordSourceInfo(const std::string &file, int line,
                                      LocationPtr loc) {
  // With FrameInjection, there is normally no need to generate the source info
  // map, so to save memory.
  if (Option::GenerateSourceInfo) {
    // we only need one to one mapping, and there doesn't seem to be a need
    // to display multiple PHP file locations for one C++ frame
    m_sourceInfos[file][line] = loc;
  }
}

void AnalysisResult::recordClassSource(const std::string &clsname,
                                       LocationPtr loc,
                                       const std::string filename) {
  string file; int line;
  if (loc) {
    file = loc->file;
    line = loc->line1 ? loc->line1 : loc->line0;
  } else {
    file = filename;
    line = 0;
  }
  m_clsNameMap[clsname].insert(pair<string, int>(file, line));
}

void AnalysisResult::recordFunctionSource(const std::string &funcname,
                                          LocationPtr loc,
                                          const std::string filename) {
  string file; int line;
  if (loc) {
    file = loc->file;
    line = loc->line1;
  } else {
    file = filename;
    line = 0;
  }
  m_funcNameMap[funcname].insert(pair<string, int>(file, line));
}

void AnalysisResult::outputCPPSourceInfos() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "source_info.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");

  cg.printImplStarter();
  cg.namespaceBegin();
  cg_printf("const char *g_source_root = \"%s\";\n",
            (Process::GetCurrentDirectory() + '/').c_str());
  cg_indentBegin("const char *g_source_info[] = {\n");

  for (map<string, map<int, LocationPtr> >::const_iterator
         i = m_sourceInfos.begin(); i != m_sourceInfos.end(); ++i) {
    for (map<int, LocationPtr>::const_iterator j = i->second.begin();
         j != i->second.end(); ++j) {
      LocationPtr loc = j->second;
      string fileline = i->first + ":" + lexical_cast<string>(j->first);
      cg_printf("\"%s\", \"%s\", (const char *)%d,\n",
                fileline.c_str(), loc->file, loc->line1);
    }
  }

  cg_printf("NULL\n");
  cg_indentEnd("};\n");
  cg.namespaceEnd();
  f.close();
}

void AnalysisResult::outputCPPNameMaps() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "name_maps.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");

  cg.printImplStarter();
  cg.namespaceBegin();

  cg.printSection("Class -> File Line");
  cg_indentBegin("const char *g_source_cls2file[] = {\n");
  for (map<string, set<pair<string, int> > >::const_iterator
         iter = m_clsNameMap.begin(); iter != m_clsNameMap.end(); ++iter) {
    for (set<pair<string, int> >::const_iterator iterInner =
           iter->second.begin();
         iterInner != iter->second.end(); ++iterInner) {
      cg_printf("\"%s\", \"%s\", (const char *)%d,\n", iter->first.c_str(),
                iterInner->first.c_str(), iterInner->second);
    }
  }
  cg_printf("NULL\n");
  cg_indentEnd("};\n");


  cg.printSection("Function -> File Line");
  cg_indentBegin("const char *g_source_func2file[] = {\n");
  for (map<string, set<pair<string, int> > >::const_iterator
         iter = m_funcNameMap.begin(); iter != m_funcNameMap.end(); ++iter) {
    for (set<pair<string, int> >::const_iterator iterInner =
           iter->second.begin();
         iterInner != iter->second.end(); ++iterInner) {
      cg_printf("\"%s\", \"%s\", (const char *)%d,\n", iter->first.c_str(),
                iterInner->first.c_str(), iterInner->second);
    }
  }
  cg_printf("NULL\n");
  cg_indentEnd("};\n");

  cg.printSection("Param RTTI Id -> Name");
  cg_indentBegin("const char *g_paramrtti_map[] = {\n");
  if (Option::GenRTTIProfileData) {
    char **params = (char **)calloc(m_paramRTTICounter, sizeof(char*));
    for (map<string, int>::const_iterator
         iter = m_paramRTTIs.begin(); iter != m_paramRTTIs.end(); ++iter) {
      ASSERT(params[iter->second] == NULL);
      params[iter->second] = (char *)iter->first.c_str();
    }
    for (int i = 0; i < m_paramRTTICounter; i++) {
      ASSERT(params[i]);
      cg_printf("\"%s\", // %d\n", params[i], i);
    }
    free(params);
  }
  cg_printf("NULL\n");
  cg_indentEnd("};\n");

  cg.namespaceEnd();
  f.close();
}

void AnalysisResult::outputRTTIMetaData(const char *filename) {
  ASSERT(filename && *filename);
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    throw Exception("Unable to open %s: %s", filename,
                    Util::safe_strerror(errno).c_str());
  }
  fprintf(f, "%d\n", m_paramRTTICounter);
  for (set<string>::const_iterator
       iter = m_rttiFuncs.begin(); iter != m_rttiFuncs.end(); ++iter) {
    fprintf(f, "%s\n", iter->c_str());
  }
  fclose(f);
}

void AnalysisResult::outputCPPUtilDecl(CodeGenerator::Output output) {
  if (!Option::GenConcat && !Option::GenArrayCreate) return;

  string filename = string(Option::SystemFilePrefix) + "cpputil.h";
  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream f(headerPath.c_str());
  CodeGenerator cg(&f, output);
  cg_printf("\n");
  cg_printf("#ifndef __GENERATED_cpputil_h__\n");
  cg_printf("#define __GENERATED_cpputil_h__\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();
  if (Option::GenConcat) {
    outputConcatDecl(cg);
  }
  if (Option::GenArrayCreate) {
    outputArrayCreateDecl(cg);
  }
  cg.namespaceEnd();
  cg_printf("#endif // __GENERATED_cpputil_h__\n");
}

void AnalysisResult::outputCPPUtilImpl(CodeGenerator::Output output) {
  if (!Option::GenConcat && !Option::GenArrayCreate) return;

  string filename = string(Option::SystemFilePrefix) + "cpputil.cpp";
  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream f(headerPath.c_str());
  CodeGenerator cg(&f, output);
  cg_printf("\n");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  cg_printInclude("<runtime/base/array/zend_array.h>");
  cg_printInclude("<runtime/base/array/small_array.h>");
  cg_printInclude("<runtime/base/taint.h>");
  cg.printImplStarter();
  cg.namespaceBegin();
  if (Option::GenConcat) {
    outputConcatImpl(cg);
  }
  if (Option::GenArrayCreate) {
    outputArrayCreateImpl(cg);
  }
  cg.namespaceEnd();
}

void AnalysisResult::outputConcatNumDecl(CodeGenerator &cg, int num) {
  cg_printf("String concat%d(", num);
  for (int i = 1; i <= num; i++) {
    cg_printf("CStrRef s%d", i);
    if (i < num) {
      cg_printf(", ");
    } else {
      cg_printf(")");
    }
  }
}

void AnalysisResult::outputConcatDecl(CodeGenerator &cg) {
  for (set<int>::const_iterator iter = m_concatLengths.begin();
       iter != m_concatLengths.end(); ++iter) {
    int num = *iter;
    ASSERT(num > MAX_CONCAT_ARGS);
    outputConcatNumDecl(cg, num);
    cg_printf(";\n");
  }
}

void AnalysisResult::outputConcatImpl(CodeGenerator &cg) {
  for (set<int>::const_iterator iter = m_concatLengths.begin();
       iter != m_concatLengths.end(); ++iter) {
    int num = *iter;
    ASSERT(num > MAX_CONCAT_ARGS);
    outputConcatNumDecl(cg, num);
    cg_indentBegin(" {\n");
    for (int i = 1; i <= num; i++) {
      cg_printf("int len%d = s%d.size();\n", i, i);
    }
    cg_printf("int len =");
    for (int i = 1; i <= num; i++) {
      cg_printf(" len%d", i);
      if (i < num) {
        cg_printf(" +");
      } else {
        cg_printf(";\n");
      }
    }
    cg_printf("char *buf = (char*)malloc(len + 1);\n");
    cg_printf("if (buf == NULL) {\n");
    cg_printf("  throw FatalErrorException(0,\"malloc failed: %%d\", len);\n");
    cg_printf("}\n");
    for (int i = 1; i <= num; i++) {
      cg_printf("memcpy(buf");
      for (int j = 1; j < i; j++) {
        cg_printf(" + len%d", j);
      }
      cg_printf(", s%d.data(), len%d);\n", i, i);
    }
    cg_printf("buf[len] = 0;\n");
    cg_printf("String r = String(buf, len, AttachString);\n");
    cg_printf("#ifdef TAINTED\n");
    cg_printf("Taint(r) ");
    for (int i = 1; i <= num; i++) {
      cg_printf("<< s%d", i);
    }
    cg_printf(";\n");
    cg_printf("#endif\n");
    cg_printf("return r;\n");
    cg_indentEnd("}\n");
  }
}

void AnalysisResult::outputArrayCreateNumDecl(CodeGenerator &cg, int num,
                                              const char *type) {
  cg_printf("ArrayData *array_create%d(unsigned long n, ", num);
  for (int i = 1; i <= num; i++) {
    cg_printf("%s k%d, CVarRef v%d", type, i, i);
    if (i < num) {
      cg_printf(", ");
    } else {
      cg_printf(")");
    }
  }
}

void AnalysisResult::outputArrayCreateDecl(CodeGenerator &cg) {
  for (set<int>::const_iterator iter = m_arrayLitstrKeySizes.begin();
       iter != m_arrayLitstrKeySizes.end(); ++iter) {
    int num = *iter;
    ASSERT(num > 0);
    outputArrayCreateNumDecl(cg, num, "CStrRef");
    cg_printf(";\n");
  }
  for (set<int>::const_iterator iter = m_arrayIntegerKeySizes.begin();
       iter != m_arrayIntegerKeySizes.end(); ++iter) {
    int num = *iter;
    ASSERT(num > 0);
    outputArrayCreateNumDecl(cg, num, "int64");
    cg_printf(";\n");
  }
}

void AnalysisResult::outputArrayCreateImpl(CodeGenerator &cg) {
  for (set<int>::const_iterator iter = m_arrayLitstrKeySizes.begin();
       iter != m_arrayLitstrKeySizes.end(); ++iter) {
    int num = *iter;
    ASSERT(num > 0);
    outputArrayCreateNumDecl(cg, num, "CStrRef");
    cg_indentBegin(" {\n");
    if (num <= SmallArray::SARR_SIZE) {
      cg_indentBegin("if (RuntimeOption::UseSmallArray) {\n");
      cg_indentBegin("StringData *keys[] = {\n");
      for (int i = 1; i <= num; i++) {
        cg_printf("k%d.get(), ", i);
      }
      cg_indentEnd("NULL,\n");
      cg_printf("};\n");
      cg_indentBegin("const Variant *values[] = {\n");
      for (int i = 1; i <= num; i++) {
        cg_printf("&v%d, ", i);
      }
      cg_indentEnd("NULL,\n");
      cg_printf("};\n");
      cg_printf("return NEW(SmallArray)(%d, n, keys, values);\n", num);
      cg_indentEnd("}\n");
    }
    cg_indentBegin("ZendArray::Bucket *p[] = {\n");
    for (int i = 1; i <= num; i++) {
      cg_printf("NEW(ZendArray::Bucket)(k%d.get(), v%d), ",
                i, i);
    }
    cg_indentEnd("NULL,\n");
    cg_printf("};\n");
    cg_printf("return NEW(ZendArray)(%d, n, p);\n", num);
    cg_indentEnd("}\n");
  }
  for (set<int>::const_iterator iter = m_arrayIntegerKeySizes.begin();
       iter != m_arrayIntegerKeySizes.end(); ++iter) {
    int num = *iter;
    ASSERT(num > 0);
    outputArrayCreateNumDecl(cg, num, "int64");
    cg_indentBegin(" {\n");
    if (num <= SmallArray::SARR_SIZE) {
      cg_indentBegin("if (RuntimeOption::UseSmallArray) {\n");
      cg_indentBegin("int64 keys[] = {\n");
      for (int i = 1; i <= num; i++) {
        cg_printf("k%d, ", i);
      }
      cg_printf("};\n");
      cg_indentBegin("const Variant *values[] = {\n");
      for (int i = 1; i <= num; i++) {
        cg_printf("&v%d, ", i);
      }
      cg_indentEnd("NULL,\n");
      cg_printf("};\n");
      cg_printf("return NEW(SmallArray)(%d, n, keys, values);\n", num);
      cg_indentEnd("}\n");
    }
    cg_indentBegin("ZendArray::Bucket *p[] = {\n");
    for (int i = 1; i <= num; i++) {
      cg_printf("NEW(ZendArray::Bucket)(k%d, v%d), ",
                i, i);
    }
    cg_indentEnd("NULL,\n");
    cg_printf("};\n");
    cg_printf("return NEW(ZendArray)(%d, n, p);\n", num);
    cg_indentEnd("}\n");
  }
}

void AnalysisResult::outputCPPDynamicTablesHeader
    (CodeGenerator &cg,
     bool includeGlobalVars /* = true*/, bool includes /* = true */,
     bool noNamespace /* = false */) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  if (system) {
    cg_printf("\n");
    cg_printInclude("<runtime/base/hphp_system.h>");
    cg_printInclude("<runtime/ext/ext.h>");
    cg_printInclude("<runtime/eval/eval.h>");
    cg_printInclude(string(Option::SystemFilePrefix) + "literal_strings.h");
    cg_printf("\n");
  } else {
    cg_printf("\n");
    cg_printInclude("<runtime/base/hphp.h>");
    if (includeGlobalVars) {
      cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
    }
    if (Option::EnableEval >= Option::LimitedEval) {
      cg_printInclude("<runtime/eval/eval.h>");
    }
    cg_printf("\n");
  }
  if (includes) {
    string n;
    FileScopePtr f;
    BOOST_FOREACH(tie(n, f), m_files) {
      cg_printInclude(f->outputFilebase());
    }
  }

  cg.printImplStarter();
  if (!noNamespace) {
    cg_printf("\n");
    cg_printf("using namespace std;\n");
    cg.namespaceBegin();
  }
}

void AnalysisResult::createGlobalFuncTable() {
  vector<const char *> funcs;
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    FunctionScopePtr func = iter->second[0];
    if (func->isDynamic() || func->isRedeclaring()) {
      funcs.push_back(iter->second[0]->getOriginalName().c_str());
    }
  }
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second[0];
    if (func->isSepExtension()) {
      funcs.push_back(iter->second[0]->getOriginalName().c_str());
    }
  }
  if (funcs.size() > 0) {
    m_funcTableSize = Util::roundUpToPowerOfTwo(funcs.size() * 2);
    CodeGenerator::BuildJumpTable(funcs, m_funcTable, m_funcTableSize, true);
  } else {
    m_funcTableSize = 0;
  }
}

vector<const char *> &
AnalysisResult::getFuncTableBucket(FunctionScopePtr func) {
  string name = Util::toLower(func->getOriginalName());
  int64 hash = hash_string_i(name.c_str());
  int64 index = hash % m_funcTableSize;
  return m_funcTable[index];
}

void AnalysisResult::outputCPPInvokeFileHeader(CodeGenerator &cg) {
  cg_indentBegin("Variant invoke_file(CStrRef s, "
                 "bool once /* = false */, "
                 "LVariableTable* variables /* = NULL */,"
                 "const char *currentDir /* = NULL */) {\n");
}

void AnalysisResult::outputCPPEvalHook(CodeGenerator &cg) {
  cg_indentBegin("{\n");
  cg_printf("Variant r;\n");
  cg_printf("if (eval_invoke_file_hook(r, s, once, variables, "
            "currentDir)) "
            "return r;\n");
  cg_indentEnd("}\n");
}

void AnalysisResult::outputCPPDefaultInvokeFile(CodeGenerator &cg,
                                                const char *file) {
  cg_printf("if (s.empty()) return %s%s(once, variables);\n",
            Option::PseudoMainPrefix,
            Option::MangleFilename(file, true).c_str());
}

void AnalysisResult::outputCPPHashTableInvokeFile(
  CodeGenerator &cg, const vector<const char*> &entries, bool needEvalHook) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "class hashNodeFile {\n"
    "public:\n"
    "  hashNodeFile() {}\n"
    "  hashNodeFile(int64 h, const char *n, const void *p) :\n"
    "    hash(h), name(n), ptr(p), next(NULL) {}\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  const void *ptr;\n"
    "  hashNodeFile *next;\n"
    "};\n"
    "static hashNodeFile *fileMapTable[%d];\n"
    "static hashNodeFile fileBuckets[%d];\n"
    "\n"
    "static class FileTableInitializer {\n"
    "  public: FileTableInitializer() {\n"
    "    const char *fileMapData[] = {\n";

  const char text2[] =
    "      NULL, NULL,\n"
    "    };\n"
    "    hashNodeFile *b = fileBuckets;\n"
    "    for (const char **s = fileMapData; *s; s++, b++) {\n"
    "      const char *name = *s++;\n"
    "      const void *ptr = *s;\n"
    "      int64 hash = hash_string(name, strlen(name));\n"
    "      hashNodeFile *node = new(b) hashNodeFile(hash, name, ptr);\n"
    "      int h = hash & %d;\n"
    "      if (fileMapTable[h]) node->next = fileMapTable[h];\n"
    "      fileMapTable[h] = node;\n"
    "    }\n"
    "  }\n"
    "} file_table_initializer;\n"
    "\n"
    "static inline pm_t findFile(const char *name, int64 hash) {\n"
    "  for (const hashNodeFile *p = fileMapTable[hash & %d ]; "
    "p; p = p->next) {\n"
    "    if (p->hash == hash && !strcmp(p->name, name)) return (pm_t)p->ptr;\n"
    "  }\n"
    "  return NULL;\n"
    "}\n"
    "\n";

  const char text3[] =
    "  pm_t ptr = findFile(s.c_str(), s->hash());\n"
    "  if (ptr) return ptr(once, variables, get_globals());\n";

  const char text4[] =
  "  return throw_missing_file(s.c_str());\n"
  "}\n";

  int tableSize = Util::roundUpToPowerOfTwo(entries.size() * 2);
  cg_printf(text1, tableSize, entries.size());
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    cg_printf("      (const char *)\"%s\", (const char *)&%s%s,\n",
              f->getName().c_str(),
              Option::PseudoMainPrefix,
              Option::MangleFilename(f->getName(), true).c_str());
  }
  cg_printf(text2, tableSize - 1, tableSize - 1);
  outputCPPInvokeFileHeader(cg);
  if (needEvalHook) outputCPPEvalHook(cg);
  cg_indentEnd("");
  cg_printf(text3);
  if (entries.size() == 1) {
    cg_indentBegin("\n");
    outputCPPDefaultInvokeFile(cg, entries[0]);
    cg_indentEnd("");
  }
  cg_printf(text4);
}

void AnalysisResult::outputCPPDynamicTables(CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  bool system = output == CodeGenerator::SystemCPP;
  {
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      (Option::GenHashTableInvokeFunc ? "dynamic_table_func.cpp"
                                        : "dynamic_table_func.no.cpp");
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, true, false);

    cg.printSection("Function Invoke Table");
    if (system) {
      bool needGlobals;
      outputCPPJumpTableSupport(cg, ar, needGlobals);
      outputCPPEvalInvokeTable(cg, ar);
      outputCPPCodeInfoTable(cg, ar, true);
    } else {
      outputCPPEvalInvokeTable(cg, ar);
      outputCPPCodeInfoTable(cg, ar, false, &m_functionDecs);
    }
    cg.namespaceEnd();
    fTable.close();
  }
  {
    string n;
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      "dynamic_table_class.no.cpp";
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, true, false);
    cg.printSection("Class Invoke Tables");
    MethodSlot::emitMethodSlot(cg, ar, system); // FMC broken for IDL tests(?)
    vector<const char*> classes;
    ClassScopePtr cls;
    StringToClassScopePtrVecMap classScopes;
    for (StringToClassScopePtrVecMap::const_iterator iter =
        m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
      if (iter->second.size()) {
        for (ClassScopePtrVec::const_iterator iter2 = iter->second.begin();
            iter2 != iter->second.end(); ++iter2) {
          cls = *iter2;
          if (cls->isUserClass() && !cls->isInterface()) {
            classes.push_back(cls->getOriginalName().c_str());
            classScopes[cls->getName()].push_back(cls);
            if (!system) {
              cls->outputCPPDynamicClassDecl(cg);
            }
            cls->outputCPPGlobalTableWrappersDecl(cg, ar);
            break;
          }
        }
      }
    }
    if (system) {
      BOOST_FOREACH(tie(n, cls), m_systemClasses) {
        if (!cls->isInterface() && !cls->isSepExtension()) {
          classes.push_back(cls->getOriginalName().c_str());
        }
      }
      outputCPPExtClassImpl(cg);
    } else {
      BOOST_FOREACH(tie(n, cls), m_systemClasses) {
        if (!cls->isInterface() && cls->isSepExtension()) {
          classes.push_back(cls->getOriginalName().c_str());
          cls->outputCPPDynamicClassDecl(cg);
          cls->outputCPPGlobalTableWrappersDecl(cg, ar);
          classScopes[cls->getName()].push_back(cls);
        }
      }
      ClassScope::outputCPPClassVarInitImpl(cg, classScopes, classes);
      ClassScope::outputCPPDynamicClassCreateImpl(cg, classScopes, classes);
      ClassScope::outputCPPInvokeStaticMethodImpl(cg, classScopes, classes);
      ClassScope::outputCPPGetCallInfoStaticMethodImpl(cg, classScopes,
          classes);
      ClassScope::outputCPPGetStaticPropertyImpl(cg, classScopes, classes);
      ClassScope::outputCPPGetClassConstantImpl(cg, classScopes, classes);
    }
    cg.namespaceEnd();
    fTable.close();
  }
  {
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      "dynamic_table_constant.no.cpp";
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, true, false);

    vector<const char *> strings;
    hphp_const_char_map<bool> dyns;
    ConstantTablePtr ct = getConstants();
    vector<string> syms;
    ct->getSymbols(syms);
    BOOST_FOREACH(string sym, syms) {
      if (system || ct->isSepExtension(sym)) {
        strings.push_back(sym.c_str());
        dyns[sym.c_str()] = ct->isDynamic(sym);
      }
    }
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      ConstantTablePtr ct = fs->getConstants();
      vector<string> syms;
      ct->getSymbols(syms);
      BOOST_FOREACH(string sym, syms) {
        if (ct->isSystem(sym) && !system) continue;
        strings.push_back(sym.c_str());
        dyns[sym.c_str()] = ct->isDynamic(sym);
      }
      ct->outputCPP(cg, ar);
    }

    cg.printSection("Get Constant Table");
    cg_indentBegin("Variant get_%sconstant(CStrRef name) {\n",
        system ? "builtin_" : "");
    cg.printDeclareGlobals();

    if (!system && Option::EnableEval == Option::FullEval) {
      // See if there's an eval'd version
      cg_indentBegin("{\n");
      cg_printf("Variant r;\n");
      cg_printf("if (eval_constant_hook(r, name)) return r;\n");
      cg_indentEnd("}\n");
    }

    if (strings.size() > 0) {
      cg_printf("const char* s = name.data();\n");
      for (JumpTable jt(cg, strings, false, false, false); jt.ready();
          jt.next()) {
        const char *name = jt.key();
        string varName = string(Option::ConstantPrefix) + cg.formatLabel(name);
        hphp_const_char_map<bool>::const_iterator it = dyns.find(name);
        bool dyn = it != dyns.end() && it->second;
        if (dyn) {
          string escaped = cg.escapeLabel(name);
          cg_printf("HASH_RETURN(0x%016llXLL, "
                    "getDynamicConstant(g->%s, \"%s\"), \"%s\");\n",
                    hash_string(name), varName.c_str(),
                    escaped.c_str(), escaped.c_str());
        } else {
          cg_printf("HASH_RETURN(0x%016llXLL, %s, \"%s\");\n",
                    hash_string(name), varName.c_str(),
                    cg.escapeLabel(name).c_str());
        }
      }
    }

    if (system) {
      cg_printf("raise_notice(\"Use of undefined constant %%s -- "
          "assumed '%%s'.\", s, s);\n"),
        cg_printf("return name;\n");
    } else {
      cg_printf("return get_builtin_constant(name);\n");
    }
    cg_indentEnd("}\n");
    cg.namespaceEnd();
    fTable.close();
  }
  if (output != CodeGenerator::SystemCPP) {
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      (Option::GenHashTableInvokeFile ? "dynamic_table_file.cpp"
                                      : "dynamic_table_file.no.cpp");
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, false, false);
    if (Option::GenHashTableInvokeFile) {
      cg_printf("typedef Variant (*pm_t)(bool incOnce, "
                "LVariableTable* variables, Globals *globals);\n");
    }
    cg.printSection("File Invoke Table");
    vector<const char*> entries;
    BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
      entries.push_back(f->getName().c_str());
      cg_printf("Variant %s%s(bool incOnce = false, "
                "LVariableTable* variables = NULL, "
                "Globals *globals = get_globals());\n",
                Option::PseudoMainPrefix,
                Option::MangleFilename(f->getName(), true).c_str());
    }

    cg_printf("\n");
    bool needEvalHook = !system && Option::EnableEval == Option::FullEval;
    if (Option::GenHashTableInvokeFile) {
      outputCPPHashTableInvokeFile(cg, entries, needEvalHook);
      cg.namespaceEnd();
      fTable.close();
      return;
    }
    outputCPPInvokeFileHeader(cg);
    // See if there's an eval'd version
    if (needEvalHook) outputCPPEvalHook(cg);

    string root;

    for (JumpTable jt(cg, entries, false, false, true); jt.ready();
         jt.next()) {
      const char *file = jt.key();
      cg_printf("HASH_INCLUDE(0x%016llXLL, \"%s\", %s);\n",
                hash_string(file), file,
                Option::MangleFilename(file, true).c_str());
    }


    // when we only have one file, we default to running the file
    if (entries.size() == 1) outputCPPDefaultInvokeFile(cg, entries[0]);

    cg_printf("return throw_missing_file(s.data());\n");
    cg_indentEnd("}\n");

    cg.namespaceEnd();
    fTable.close();
  }
}

void AnalysisResult::outputCPPClassDeclaredFlags
(CodeGenerator &cg, Type2SymbolListMap &type2names) {
  SymbolList &symbols = type2names["bool"];
  for (StringToClassScopePtrVecMap::const_iterator it = m_classDecs.begin();
       it != m_classDecs.end(); ++it) {
    if (!it->second.size() || it->second[0]->isVolatile()) {
      symbols.push_back(string("cdec_") + Util::toLower(it->first));
    }
  }
}

void AnalysisResult::outputCPPClassDeclaredFlagsLookup(CodeGenerator &cg) {
  vector <const char *> classes;
  for (StringToClassScopePtrVecMap::const_iterator it = m_classDecs.begin();
       it != m_classDecs.end(); ++it) {
    if (!it->second.size() || it->second[0]->isVolatile()) {
      classes.push_back(it->first.c_str());
    }
  }
  cg_indentBegin("bool GlobalVariables::class_exists(const char *s) {\n");
  for (JumpTable jt(cg, classes, true, false, false); jt.ready(); jt.next()) {
    const char *name = jt.key();
    cg_printf("HASH_GUARD(0x%016llXLL, %s) return CDEC(%s);\n",
              hash_string_i(name), name, name);
  }
  cg_printf("return false;\n");
  cg_indentEnd("}\n");
}

void AnalysisResult::outputCPPSystem() {
  string filename = string(Option::SystemFilePrefix) + "system_globals.h";

  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream fSystem(headerPath.c_str());
  CodeGenerator cg(&fSystem, CodeGenerator::SystemCPP);

  string implPath = m_outputPath + "/" + Option::SystemFilePrefix +
    "system_globals.cpp";
  ofstream fSystemImpl(implPath.c_str());
  cg.setStream(CodeGenerator::ImplFile, &fSystemImpl);

  cg.headerBegin(filename.c_str());
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    cg_printInclude(fs->outputFilebase());
  }

  cg.printImplStarter();
  cg.namespaceBegin();
  AnalysisResultPtr ar = shared_from_this();
  getVariables()->setAttribute(VariableTable::ForceGlobal);
  getVariables()->outputCPP(cg, ar);
  getVariables()->clearAttribute(VariableTable::ForceGlobal);
  getConstants()->outputCPP(cg, ar);
  cg.namespaceEnd();

  cg.headerEnd(filename.c_str());

  cg.setContext(CodeGenerator::CppImplementation);
  cg.useStream(CodeGenerator::ImplFile);
  cg_printf("\n");
  cg_printInclude(filename.c_str());
  cg.namespaceBegin();
  outputCPPGlobalImplementations(cg);
  cg.namespaceEnd();

  fSystemImpl.close();
  fSystem.close();

  outputCPPScalarArrays(true);
}

void AnalysisResult::outputCPPRedeclaredFunctionDecl
(CodeGenerator &cg, Type2SymbolListMap &type2names) {
  SymbolList &symbols = type2names["CallInfo*"];
  SymbolList &bools = type2names["bool"];
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
      m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    if (iter->second[0]->isVolatile()) {
      std::string fname = cg.formatLabel(iter->first);
      const char *name = fname.c_str();
      if (iter->second[0]->isRedeclaring()) {
        symbols.push_back(string("cim_") + name);
      }
      if (strcmp(name, "__autoload")) {
        bools.push_back(string(FVF_PREFIX) + name);
      }
    }
  }
}

void AnalysisResult::outputCPPRedeclaredClassDecl
(CodeGenerator &cg, Type2SymbolListMap &type2names) {
  SymbolList &statics = type2names["ClassStaticsPtr"];
  SymbolList &callbacks = type2names["ObjectStaticCallbacks*"];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      statics.push_back(string(Option::ClassStaticsObjectPrefix) + name);
      callbacks.push_back(string(Option::ClassStaticsCallbackPrefix) + name);
    }
  }
}

void AnalysisResult::outputCPPRedeclaredClassImpl(CodeGenerator &cg) {
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      const char *name = iter->first.c_str();
      cg_printf("%s%s = ClassStaticsPtr(NEW(ClassStatics)(\"%s\"));\n",
                Option::ClassStaticsObjectPrefix, name, name);
    }
  }
}

void AnalysisResult::outputCPPDynamicConstantDecl
(CodeGenerator &cg, Type2SymbolListMap &type2names) {
  AnalysisResultPtr ar = shared_from_this();
  getConstants()->outputCPPDynamicDecl(cg, ar, type2names);
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->getConstants()->outputCPPDynamicDecl(cg, ar, type2names);
  }
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    for (vector<ClassScopePtr>::const_iterator viter = iter->second.begin();
         viter != iter->second.end(); ++viter) {
      const ClassScopePtr &cls = *viter;
      cls->getConstants()->outputCPPDynamicDecl(cg, ar, type2names);
    }
  }
}

void AnalysisResult::outputCPPDynamicConstantImpl(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();
  getConstants()->outputCPPDynamicImpl(cg, ar);
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->getConstants()->outputCPPDynamicImpl(cg, ar);
  }
}

void AnalysisResult::outputCPPScalarArrayDecl(CodeGenerator &cg) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *prefix = system ? Option::SystemScalarArrayName :
    Option::ScalarArrayName;
  if (m_scalarArrayIds.size() > 0) {
    cg_printf("static StaticArray %s[%d];\n", prefix, m_scalarArrayIds.size());
  }
}

bool AnalysisResult::wrapExpressionBegin(CodeGenerator &cg) {
  if (!m_wrappedExpression) {
    m_wrappedExpression = true;
    cg_indentBegin("{\n");
    return true;
  }
  return false;
}

bool AnalysisResult::wrapExpressionEnd(CodeGenerator &cg) {
  if (m_wrappedExpression) {
    m_wrappedExpression = false;
    cg_indentEnd("}\n");
    return true;
  }
  return false;
}

void AnalysisResult::outputHexBuffer(CodeGenerator &cg, const char *name,
                                     const char *buf, int len) {
  cg_printf("static const char %s[%d] = {\n", name, len);
  for (int i = 0; i < len; i++) {
    if (i % 10 == 0) cg_printf("  ");
    cg_printf("0x%02x, ", (unsigned char)buf[i]);
    if (i % 10 == 9) cg_printf("\n");
  }
  cg_printf("};\n\n");
}

string AnalysisResult::getScalarArrayCompressedText() {
  Array arrs;
  for (unsigned int i = 0; i < m_scalarArrayIds.size(); i++) {
    ExpressionPtr exp = m_scalarArrayIds[i];
    if (exp) {
      Variant value;
      bool ret = exp->getScalarValue(value);
      if (!ret) ASSERT(false);
      if (!value.isArray()) ASSERT(false);
      arrs.append(value);
    } else {
      arrs.append(StaticArray(ArrayData::Create()));
    }
  }
  String s = f_serialize(arrs);
  int len = s.size();
  char *zd = gzencode(s.data(), len, 9, CODING_GZIP);
  m_scalarArrayCompressedTextSize = len;
  return string(zd, len);
}

void AnalysisResult::getLiteralStringCompressed(std::string &zsdata,
                                                std::string &zldata) {
  int nstrings = m_stringLiterals.size();
  ASSERT(nstrings > 0);
  string *sortedById = new string[nstrings];
  for (map<string, int>::const_iterator it = m_stringLiterals.begin();
       it != m_stringLiterals.end(); ++it) {
    int index = it->second;
    ASSERT(0 <= index && index < nstrings);
    sortedById[index] = it->first;
  }
  string sdata;
  string ldata;
  for (int i = 0; i < nstrings; i++) {
    sdata += sortedById[i];
    sdata += string("\0", 1);

    int size = sortedById[i].size();
    char buf[sizeof(size)];
    memcpy(buf, &size, sizeof(size));
    ldata += string(buf, sizeof(size));
  }
  int sdlen = sdata.size();
  char *zsd = gzencode(sdata.data(), sdlen, 9, CODING_GZIP);
  int ldlen = ldata.size();
  char *zld = gzencode(ldata.data(), ldlen, 9, CODING_GZIP);
  zsdata = string(zsd, sdlen);
  zldata = string(zld, ldlen);
}

void AnalysisResult::outputCPPScalarArrayImpl(CodeGenerator &cg) {
  if (m_scalarArrayIds.size() == 0) return;
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  AnalysisResultPtr ar = shared_from_this();
  if (Option::ScalarArrayCompression && !system) {
    string s = getScalarArrayCompressedText();
    outputHexBuffer(cg, "sa_cdata", s.data(), s.size());
  }
  const char *clsname = system ? "SystemScalarArrays" : "ScalarArrays";
  const char *prefix = system ? Option::SystemScalarArrayName :
    Option::ScalarArrayName;

  cg_printf("StaticArray %s::%s[%d];\n",
            clsname, prefix, m_scalarArrayIds.size());
}

static bool SortByExpressionLength(const AnalysisResult::ScalarArrayExp &p1,
                                   const AnalysisResult::ScalarArrayExp &p2) {
  return p1.len < p2.len;
}

void AnalysisResult::outputCPPScalarArrayInit(CodeGenerator &cg, int fileCount,
                                              int part) {
  if (part == 0) {
    int sumLen = 0;
    m_scalarArraySorted.clear();
    if (m_scalarArrayIds.size() > 0) {
      m_scalarArraySorted.resize(m_scalarArrayIds.size());
      for (unsigned int i = 0; i < m_scalarArrayIds.size(); i++) {
        ScalarArrayExp &exp = m_scalarArraySorted[i];
        exp.id = i;
        exp.len = m_scalarArrayIds[i] ?
                  m_scalarArrayIds[i]->getText(true).size() : 0;
        exp.exp = m_scalarArrayIds[i];
        sumLen += exp.len;
      }
      sort(m_scalarArraySorted.begin(), m_scalarArraySorted.end(),
           SortByExpressionLength);
    }
    m_scalarArraySortedAvgLen = sumLen / fileCount;
  }

  if (m_scalarArraySorted.empty()) return;
  AnalysisResultPtr ar = shared_from_this();
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *prefix = system ? Option::SystemScalarArrayName :
    Option::ScalarArrayName;

  bool nonEmpty = false;
  while (m_scalarArraySortedIndex < (int)m_scalarArraySorted.size()) {
    ScalarArrayExp &exp = m_scalarArraySorted[m_scalarArraySortedIndex++];
    m_scalarArraySortedSumLen += exp.len;
    // In case we hit a huge scalar array, leave it for the next file unless
    // the current file is still empty.
    if ((m_scalarArraySortedSumLen > (part + 1) * m_scalarArraySortedAvgLen)
         && (part < fileCount - 1) && nonEmpty) {
      m_scalarArraySortedIndex--;
      break;
    }
    nonEmpty = true;
    cg_printf("%s[%d] = StaticArray(", prefix, exp.id);
    if (exp.exp) {
      ExpressionListPtr expList =
        dynamic_pointer_cast<ExpressionList>(exp.exp);
      int numElems = expList->getCount();
      if (numElems <= Option::ScalarArrayOverflowLimit) {
        expList->outputCPP(cg, ar);
        cg_printf(", NULL);\n");
      } else {
        ExpressionListPtr subExpList =
          dynamic_pointer_cast<ExpressionList>(expList->clone());
        for (int i = Option::ScalarArrayOverflowLimit; i < numElems; i++) {
          subExpList->removeElement(Option::ScalarArrayOverflowLimit);
        }
        ar->setInsideScalarArray(true);
        subExpList->outputCPP(cg, ar);
        ar->setInsideScalarArray(false);
        cg_printf(", NULL);\n");
        for (int i = Option::ScalarArrayOverflowLimit; i < numElems; i++) {
          ExpressionPtr elemExp = (*expList)[i];
          ArrayPairExpressionPtr pair =
            dynamic_pointer_cast<ArrayPairExpression>(elemExp);
          ExpressionPtr name = pair->getName();
          ExpressionPtr value = pair->getValue();
          if (name) {
            cg_printf("%s[%d].set(", prefix, exp.id);
            name->outputCPP(cg, ar);
            cg_printf(", ");
            value->outputCPP(cg, ar);
            cg_printf(");");
          } else {
            cg_printf("%s[%d].append(", prefix, exp.id);
            value->outputCPP(cg, ar);
            cg_printf(");");
          }
          cg_printf("\n");
        }
        cg_printf("%s[%d].setStatic();\n", prefix, exp.id);
      }
    } else {
      cg_printf("ArrayData::Create());\n");
    }
  }
}

string AnalysisResult::getScalarArrayName(int hash, int index) {
  assert(index >= 0);
  string name(Option::SystemGen ? "s_sys_sa" : "s_sa");
  name += boost::str(boost::format("%08x") % hash);
  if (index > 0) name += ("_" + lexical_cast<string>(index));
  return name;
}

void AnalysisResult::outputCPPScalarArrayId(CodeGenerator &cg, int id,
                                            int hash, int index) {
  if (Option::UseNamedScalarArray) {
    string name = getScalarArrayName(hash, index);
    cg_printf("%s", name.c_str());
    return;
  }
  if (cg.getOutput() == CodeGenerator::SystemCPP) {
    cg_printf("SystemScalarArrays::%s[%d]", Option::SystemScalarArrayName, id);
    return;
  }
  cg_printf("ScalarArrays::%s[%d]", Option::ScalarArrayName, id);
}

void AnalysisResult::outputCPPGlobalDeclarations() {
  string filename = string(Option::SystemFilePrefix) + "global_variables.h";

  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream f(headerPath.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg.headerBegin(filename.c_str());
  cg_printInclude("<runtime/base/hphp.h>");

  // This isn't necessarily the right place for separable extension's
  // includes, but more of a "convenient" place.
  outputCPPSepExtensionIncludes(cg);

  cg.printImplStarter();
  cg.namespaceBegin();

  AnalysisResultPtr ar = shared_from_this();
  getVariables()->setAttribute(VariableTable::ForceGlobal);
  getVariables()->outputCPP(cg, ar);
  getVariables()->clearAttribute(VariableTable::ForceGlobal);

  cg_printf("\n");
  if (getVariables()->getAttribute(VariableTable::ContainsLDynamicVariable)) {
    cg_printf("LVariableTable *get_variable_table();\n");
  } else if (getVariables()->
             getAttribute(VariableTable::ContainsDynamicVariable)) {
    cg_printf("RVariableTable *get_variable_table();\n");
  }

  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      cg_printf("int %s%s();\n", Option::ClassStaticsIdGetterPrefix,
                     iter->first.c_str());
    }
  }
  cg.namespaceEnd();
  cg.headerEnd(filename.c_str());
  f.close();
}

void AnalysisResult::outputCPPGlobalImplementations(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();
  CodeGenerator::Context con = cg.getContext();
  cg.setContext(CodeGenerator::CppImplementation);
  getVariables()->setAttribute(VariableTable::ForceGlobal);
  getVariables()->outputCPP(cg, ar);
  getVariables()->clearAttribute(VariableTable::ForceGlobal);

  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      cg_indentBegin("int %s%s() {\n", Option::ClassStaticsIdGetterPrefix,
                     iter->first.c_str());
      cg.printDeclareGlobals();
      cg_printf("return g->%s%s->getRedeclaringId();\n",
                Option::ClassStaticsObjectPrefix, iter->first.c_str());
      cg_indentEnd("}\n");
    }
  }
  cg.setContext(con);
}

void AnalysisResult::outputCPPSystemImplementations(CodeGenerator &cg) {
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    if (f->hasImpl(shared_from_this())) {
      cg_printf("%s%s(false);\n", Option::PseudoMainPrefix,
                f->pseudoMainName().c_str());
    }
  }
}

void AnalysisResult::outputCPPFileRunDecls(CodeGenerator &cg,
                                           Type2SymbolListMap &type2names) {
  SymbolList &symbols = type2names["bool"];
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    symbols.push_back(string("run_") + Option::PseudoMainPrefix +
                      f->pseudoMainName());
  }
}

void AnalysisResult::outputCPPClassStaticInitializerDecls(CodeGenerator &cg) {
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      cls->outputCPPStaticInitializerDecl(cg);
    }
  }
}

void AnalysisResult::outputCPPClassStaticInitializerFlags
(CodeGenerator &cg, Type2SymbolListMap &type2names) {
  SymbolList &symbols = type2names["bool"];
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      if (cls->needLazyStaticInitializer()) {
        symbols.push_back(string(Option::ClassStaticInitializerFlagPrefix) +
                          cls->getId(cg));
      }
    }
  }
}

void AnalysisResult::outputCPPScalarArrays(bool system) {
  setInsideScalarArray(true);

  int fileCount = system ? 1 : Option::ScalarArrayFileCount;
  if (system) {
    fileCount = 1;
  } else if ((m_scalarArrayIds.size() / Option::ScalarArrayFileCount) == 0) {
    fileCount = 1;
  } else {
    fileCount = Option::ScalarArrayFileCount;
  }
  if (Option::ScalarArrayCompression && !system) fileCount = 1;
  for (int i = 0; i < fileCount; i++) {
    string filename = m_outputPath + "/" + Option::SystemFilePrefix +
      "scalar_arrays_" + lexical_cast<string>(i) + ".no.cpp";
    Util::mkdir(filename);
    ofstream f(filename.c_str());
    CodeGenerator cg(&f, system ? CodeGenerator::SystemCPP :
                     CodeGenerator::ClusterCPP);

    cg_printf("\n");
    cg_printInclude("<runtime/base/hphp.h>");
    cg_printInclude(string(Option::SystemFilePrefix) +
                    (system ? "system_globals.h" : "global_variables.h"));

    cg_printf("\n");
    cg.printImplStarter();
    cg.namespaceBegin();

    AnalysisResultPtr ar = shared_from_this();
    CodeGenerator::Context con = cg.getContext();
    cg.setContext(CodeGenerator::CppImplementation);
    getVariables()->setAttribute(VariableTable::ForceGlobal);
    outputCPPScalarArrays(cg, fileCount, i);
    getVariables()->clearAttribute(VariableTable::ForceGlobal);
    cg.setContext(con);

    cg.namespaceEnd();
    f.close();
  }

  setInsideScalarArray(false);
}

void AnalysisResult::outputCPPScalarArrays(CodeGenerator &cg, int fileCount,
                                           int part) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *clsname = system ? "SystemScalarArrays" : "ScalarArrays";
  const char *prefix = system ? Option::SystemScalarArrayName :
    Option::ScalarArrayName;

  if (fileCount == 1) {
    outputCPPScalarArrayImpl(cg);
    cg_printf("\n");
    cg_indentBegin("void %s::initialize() {\n", clsname);
    if (!system) {
      cg_printf("SystemScalarArrays::initialize();\n");
    }
    if (m_scalarArrayIds.size() > 0) {
      if (Option::ScalarArrayCompression && !system) {
        ASSERT(m_scalarArrayCompressedTextSize > 0);
        cg_printf("ArrayUtil::InitScalarArrays(%s, %d, sa_cdata, %d);\n",
                  prefix, m_scalarArrayIds.size(),
                  m_scalarArrayCompressedTextSize);
        cg_printf("%s::initializeNamed();\n", clsname);
      } else {
        outputCPPScalarArrayInit(cg, 1, 0);
        cg_printf("%s::initializeNamed();\n", clsname);
      }
    }
    cg_indentEnd("}\n");
    return;
  }

  if (part == 0) {
    ASSERT(!(Option::ScalarArrayCompression && !system));
    outputCPPScalarArrayImpl(cg);
    cg_printf("\n");
    cg_indentBegin("void %s::initialize() {\n", clsname);
    if (!system) {
      cg_printf("SystemScalarArrays::initialize();\n");
    }
    for (int i = 0; i < fileCount; i++) {
      cg_printf("initialize_%d();\n", i);
    }
    cg_indentEnd("}\n");
  }

  cg_printf("\n");
  cg_indentBegin("void %s::initialize_%d() {\n", clsname, part);
  outputCPPScalarArrayInit(cg, fileCount, part);
  cg_indentEnd("}\n");
}

void AnalysisResult::outputCPPGlobalVariablesMethods(int part) {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "global_variables_" + lexical_cast<string>(part) + ".no.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  AnalysisResultPtr ar = shared_from_this();

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  if (part == 1 || part == 2) {
    cg_printInclude(string(Option::SystemFilePrefix) + "literal_strings.h");
  }
  if (part == 1) {
    getVariables()->outputCPPGlobalVariablesDtorIncludes(cg, ar);
  }
  cg_printf("\n");
  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();

  CodeGenerator::Context con = cg.getContext();
  cg.setContext(CodeGenerator::CppImplementation);
  getVariables()->setAttribute(VariableTable::ForceGlobal);
  switch (part) {
  case 1:
    getVariables()->outputCPPGlobalVariablesDtor(cg);
    getVariables()->outputCPPGlobalVariablesGetImpl (cg, ar);
    outputCPPClassDeclaredFlagsLookup(cg);
    break;
  case 2: getVariables()->outputCPPGlobalVariablesExists  (cg, ar); break;
  case 3: getVariables()->outputCPPGlobalVariablesGetIndex(cg, ar); break;
  case 4: getVariables()->outputCPPGlobalVariablesMethods (cg, ar); break;
  default: ASSERT(false);
  }
  getVariables()->clearAttribute(VariableTable::ForceGlobal);
  cg.setContext(con);

  cg.namespaceEnd();

  f.close();
}

///////////////////////////////////////////////////////////////////////////////
// output_global_state()

void AnalysisResult::outputCPPGlobalStateBegin(CodeGenerator &cg,
                                               const char *section) {
  cg_indentBegin("static void get_%s(Array &res) {\n", section);
  cg_printf("DECLARE_GLOBAL_VARIABLES(g);\n");
  cg_printf("Array %s;\n", section);
}

void AnalysisResult::outputCPPGlobalStateEnd(CodeGenerator &cg,
                                             const char *section) {
  if (Option::EnableEval >= Option::LimitedEval) {
    cg_printf("eval_get_%s(%s);\n", section, section);
  }
  cg_printf("res.set(\"%s\", %s);\n", section, section);
  cg_indentEnd("}\n\n");
}

void AnalysisResult::outputCPPGlobalStateSection
(CodeGenerator &cg, const StringPairVec &names, const char *section,
 const char *prefix /* = "g->" */, const char *name_prefix /* = "" */) {
  outputCPPGlobalStateBegin(cg, section);
  for (unsigned int i = 0; i < names.size(); i++) {
    cg_printf("%s.set(\"%s%s\", %s%s%s);\n", section,
              name_prefix, names[i].first.c_str(),
              prefix, name_prefix, names[i].second.c_str());
  }
  outputCPPGlobalStateEnd(cg, section);
}

void AnalysisResult::outputCPPGlobalState() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "global_state.no.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  AnalysisResultPtr ar = shared_from_this();

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  if (Option::EnableEval >= Option::LimitedEval) {
    cg_printInclude("<runtime/eval/eval.h>");
  }
  cg_printf("\n");
  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();

  StringPairVecVec symbols(GlobalSymbolTypeCount);
  getVariables()->collectCPPGlobalSymbols(symbols, cg, ar);
  collectCPPGlobalSymbols(symbols, cg);

  outputCPPGlobalStateSection(cg, symbols[KindOfStaticGlobalVariable],
                              "static_global_variables");

  const char *section = "dynamic_global_variables";
  ar->outputCPPGlobalStateBegin(cg, section);
  cg_printf("%s = *get_variable_table();\n", section);
  ar->outputCPPGlobalStateEnd(cg, section);

  outputCPPGlobalStateSection(cg, symbols[KindOfMethodStaticVariable],
                              "method_static_variables");
  outputCPPGlobalStateSection(cg, symbols[KindOfMethodStaticVariable],
                              "method_static_inited", "g->",
                              Option::InitPrefix);
  outputCPPGlobalStateSection(cg, symbols[KindOfClassStaticVariable],
                              "class_static_variables");
  outputCPPGlobalStateSection(cg, symbols[KindOfDynamicConstant],
                              "dynamic_constants");
  outputCPPGlobalStateSection(cg, symbols[KindOfPseudoMain],
                              "pseudomain_variables");
  outputCPPGlobalStateSection(cg, symbols[KindOfRedeclaredFunction],
                              "redeclared_functions", "(int64)g->");
  outputCPPGlobalStateSection(cg, symbols[KindOfRedeclaredClassId],
                              "redeclared_classes");

  cg_indentBegin("Array get_global_state() {\n");
  cg_printf("Array res(Array::Create());\n");
  cg_printf("get_static_global_variables(res);\n");
  cg_printf("get_dynamic_global_variables(res);\n");
  cg_printf("get_dynamic_constants(res);\n");
  cg_printf("get_method_static_variables(res);\n");
  cg_printf("get_method_static_inited(res);\n");
  cg_printf("get_class_static_variables(res);\n");
  cg_printf("get_pseudomain_variables(res);\n");
  cg_printf("get_redeclared_functions(res);\n");
  cg_printf("get_redeclared_classes(res);\n");
  cg_printf("return res;\n");
  cg_indentEnd("}\n\n");

  cg.namespaceEnd();
  f.close();
}

///////////////////////////////////////////////////////////////////////////////
// fiber_marshal/unmarshal_global_state()

void AnalysisResult::collectCPPGlobalSymbols(StringPairVecVec &symbols,
                                             CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();

  // dynamic constants
  StringPairVec *names = &symbols[KindOfDynamicConstant];
  getConstants()->collectCPPGlobalSymbols(*names, cg, ar);
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->getConstants()->collectCPPGlobalSymbols(*names, cg, ar);
  }

  // pseudomain variables
  names = &symbols[KindOfPseudoMain];
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    string name = string("run_") + Option::PseudoMainPrefix +
      f->pseudoMainName();
    names->push_back(pair<string, string>(name, name));
  }

  // redeclared functions
  names = &symbols[KindOfRedeclaredFunction];
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    if (iter->second[0]->isRedeclaring()) {
      string varname = string("cim_") + name;
      names->push_back(pair<string, string>(varname, varname));
    }
  }

  // redeclared classes
  names = &symbols[KindOfRedeclaredClassId];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      string varname = string(Option::ClassStaticsObjectPrefix) + name;
      string memname = varname + "->getRedeclaringId()";
      names->push_back(pair<string, string>(varname, memname));
    }
  }
  names = &symbols[KindOfRedeclaredClass];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      string varname = string(Option::ClassStaticsObjectPrefix) + name;
      names->push_back(pair<string, string>(varname, varname));
      varname = string(Option::ClassStaticsCallbackPrefix) + name;
      names->push_back(pair<string, string>(varname, varname));
    }
  }

  // volatile classes
  names = &symbols[KindOfVolatileClass];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    if (iter->second.size() && iter->second[0]->isVolatile()) {
      string varname = string("CDEC(") + name + ")";
      names->push_back(pair<string, string>(varname, varname));
    }
  }

  // classes that need lazy static initializer
  names = &symbols[KindOfLazyStaticInitializer];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const char *name = iter->first.c_str();
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      if (iter->second[i]->needLazyStaticInitializer()) {
        string varname = string(Option::ClassStaticInitializerFlagPrefix) +
          name;
        string memname = string(Option::ClassStaticInitializerFlagPrefix) +
          iter->second[i]->getId(cg);
        names->push_back(pair<string, string>(varname, memname));
      }
    }
  }
}

void AnalysisResult::outputCPPFiberGlobalState() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "global_state_fiber.no.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  AnalysisResultPtr ar = shared_from_this();

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude("<runtime/base/fiber_reference_map.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  cg_printf("\n");
  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();

  StringPairVecVec symbols(GlobalSymbolTypeCount);
  getVariables()->collectCPPGlobalSymbols(symbols, cg, ar);
  collectCPPGlobalSymbols(symbols, cg);

  // generate a map from symbol name to its numeric index
  int index = 0;
  cg_printf("static hphp_const_char_map<int> s_gsmap;\n");
  cg_indentBegin("class GlobalSymbolMapInitializer {\n");
  cg_indentBegin("public: GlobalSymbolMapInitializer() {\n");

  cg_indentBegin("static const char *names[] = {\n");
  for (int i = 0; i < GlobalSymbolTypeCount; i++) {
    StringPairVec &names = symbols[i];
    for (unsigned int j = 0; j < names.size(); j++) {
      cg_printf("\"%s\",\n", names[j].first.c_str());
      index++;
    }
  }
  cg_printf("0\n");
  cg_indentEnd("};\n");

  cg_printf("int index = 0;\n");
  cg_indentBegin("for (const char **p = names; *p; p++) {\n");
  cg_printf("s_gsmap[*p] = index++;\n");
  cg_indentEnd("}\n");

  cg_indentEnd("}\n");
  cg_indentEnd("};\n");
  cg_printf("static GlobalSymbolMapInitializer s_gsmap_initializer;\n\n");

  // generate fiber_marshal_global_state()
  cg_indentBegin("void fiber_marshal_global_state"
                 "(GlobalVariables *g1, GlobalVariables *g2,\n"
                 " FiberReferenceMap &refMap) {\n");
  for (int type = 0; type < GlobalSymbolTypeCount; type++) {
    if (type == KindOfRedeclaredClassId) continue;

    StringPairVec &names = symbols[type];
    for (unsigned int i = 0; i < names.size(); i++) {
      const char *name = names[i].second.c_str();
      switch (type) {
        case KindOfMethodStaticVariable:
          cg_printf("refMap.marshal(g1->inited_%s, g2->inited_%s);\n",
                    name, name);
          cg_printf("refMap.marshal(g1->%s, g2->%s);\n", name, name);
          break;
        default:
          cg_printf("refMap.marshal(g1->%s, g2->%s);\n", name, name);
          break;
      }
    }
  }
  cg_printf("refMap.marshal((Array&)(*g1), (Array&)(*g2));\n");
  cg_indentEnd("}\n");

  // generate fiber_unmarshal_global_state()
  cg_indentBegin("void fiber_unmarshal_global_state"
                 "(GlobalVariables *g1, GlobalVariables *g2,\n"
                 " FiberReferenceMap &refMap, char default_strategy,\n"
                 " const vector<pair<string, char> > &resolver) {\n");
  cg_printf("hphp_string_map<char> strategies;\n");
  cg_printf("char r[%d]; memset(r, default_strategy, sizeof(r));\n", index);
  cg_indentBegin("for (unsigned int i = 0; i < resolver.size(); i++) {\n");
  cg_printf("hphp_const_char_map<int>::const_iterator it =\n");
  cg_printf("  s_gsmap.find(resolver[i].first.c_str());\n");
  cg_printf("if (it != s_gsmap.end()) r[it->second] = resolver[i].second;\n");
  cg_printf("else strategies[resolver[i].first] = resolver[i].second;\n");
  cg_indentEnd("}\n");
  cg_printf("\n");
  index = 0;
  for (int type = 0; type < GlobalSymbolTypeCount; type++) {
    if (type == KindOfRedeclaredClassId) continue;

    StringPairVec &names = symbols[type];
    for (unsigned int i = 0; i < names.size(); i++) {
      const char *name = names[i].second.c_str();
      switch (type) {
        case KindOfRedeclaredFunction:
          cg_printf("if (g2->%s) g1->%s = g2->%s;\n", name, name, name);
          break;
        case KindOfRedeclaredClass:
          if (strncmp(name, "cso_", 4)) {
            cg_printf("if (g2->%s) g1->%s = g2->%s;\n", name, name, name);
          } else {
            cg_printf("if (g2->%s.get()) g1->%s = g2->%s;\n",name, name, name);
          }
          break;
        case KindOfPseudoMain:
        case KindOfVolatileClass:
        case KindOfLazyStaticInitializer:
          cg_printf("if (g2->%s) g1->%s = true;\n", name, name);
          break;
        case KindOfMethodStaticVariable:
          cg_indentBegin("if (toBoolean(g2->inited_%s)) {", name);
          cg_printf("refMap.unmarshal(g1->%s, g2->%s, r[%d]);\n",
                    name, name, index);
          cg_printf("refMap.unmarshal(g1->inited_%s, g2->inited_%s, r[%d]);\n",
                    name, name, index);
          cg_indentEnd("}\n");
          break;
        default:
          cg_printf("refMap.unmarshal(g1->%s, g2->%s, r[%d]);\n",
                    name, name, index);
      }
      index++;
    }
  }
  cg_printf("refMap.unmarshalDynamicGlobals((Array&)(*g1), (Array&)(*g2),"
            " default_strategy, strategies);\n");
  cg_indentEnd("}\n");


  cg.namespaceEnd();
  f.close();
}

///////////////////////////////////////////////////////////////////////////////

void AnalysisResult::outputCPPMain() {
  string mainPath = m_outputPath + "/" + Option::SystemFilePrefix +
    "main.no.cpp";
  Util::mkdir(mainPath);
  ofstream fMain(mainPath.c_str());
  CodeGenerator cg(&fMain, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");

  cg_printf("\n");
  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();
  outputCPPClassStaticInitializerDecls(cg);
  cg_printf("\n");
  outputCPPGlobalImplementations(cg);
  cg.namespaceEnd();

  cg_printf("\n");
  cg_printf("#ifndef HPHP_BUILD_LIBRARY\n");
  cg_indentBegin("int main(int argc, char** argv) {\n");
  cg_printf("return HPHP::execute_program(argc, argv);\n");
  cg_indentEnd("}\n");
  cg_printf("#endif\n");

  fMain.close();
}

void AnalysisResult::outputCPPClassMap(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();

  if (!Option::GenerateCPPMetaInfo) return;
  cg_indentBegin("const char *g_class_map[] = {\n");

  // system functions
  cg_printf("(const char *)ClassInfo::IsSystem, NULL, \"\","
            " \"\", NULL, NULL,\n");
  cg_printf("NULL,\n"); // interfaces
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second[0];
    ASSERT(!func->isUserFunction());
    func->outputCPPClassMap(cg, ar);
  }
  cg_printf("NULL,\n"); // methods
  cg_printf("NULL,\n"); // properties
  // system constants
  m_constants->outputCPPClassMap(cg, ar);

  // user functions
  cg_printf("(const char *)ClassInfo::IsNothing, NULL, \"\","
            " \"\", NULL, NULL,\n");
  cg_printf("NULL,\n"); // interfaces
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    FunctionScopePtr func = iter->second[0];
    if (func->isUserFunction()) {
      func->outputCPPClassMap(cg, ar);
    }
  }
  cg_printf("NULL,\n"); // methods
  cg_printf("NULL,\n"); // properties

  // user defined constants
  for (int i = 0; i < (int)m_fileScopes.size(); i++) {
    ConstantTablePtr constants = m_fileScopes[i]->getConstants();
    constants->outputCPPClassMap(cg, ar, (i == (int)m_fileScopes.size() - 1));
  }

  // system classes
  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    iter->second->outputCPPClassMap(cg, ar);
  }

  // user classes
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    bool redec = !iter->second.size() || iter->second[0]->isRedeclaring();
    if (redec) {
      bool isInterface = true;
      for (size_t i = 0; i < iter->second.size(); i++) {
        if (!iter->second[i]->isInterface()) {
          isInterface = false;
          break;
        }
      }
      cg_printf("(const char *)(ClassInfo::IsRedeclared | "
                "ClassInfo::IsVolatile%s), \"%s\", "
                "(const char *)%s%s,\n",
                isInterface ? " | ClassInfo::IsInterface" : "",
                iter->first.c_str(),
                Option::ClassStaticsIdGetterPrefix, iter->first.c_str());
    }
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      cls->outputCPPClassMap(cg, ar);
    }
    if (redec) {
      cg_printf("NULL,\n");
    }
  }

  cg_printf("NULL\n");
  cg_indentEnd("};\n");
}

void AnalysisResult::outputCPPClusterImpl(CodeGenerator &cg,
                                          const FileScopePtrVec &files) {
  AnalysisResultPtr ar = shared_from_this();
  cg_printf("\n");

  // includes
  map<string, FileScopePtr> toInclude;
  BOOST_FOREACH(FileScopePtr fs, files) {
    getTrueDeps(fs, toInclude);
    toInclude[fs->getName()] = fs;
  }
  for (map<string, FileScopePtr>::const_iterator iter = toInclude.begin();
       iter != toInclude.end(); ++iter) {
    FileScopePtr fs = iter->second;
    cg_printInclude(fs->outputFilebase());
  }
  for (map<string, FileScopePtr>::const_iterator iter = toInclude.begin();
       iter != toInclude.end(); ++iter) {
    FileScopePtr fs = iter->second;
    cg_printInclude(fs->outputFilebase() + ".fws");
  }
  cg_printInclude("<runtime/ext/ext.h>");
  outputCPPSepExtensionIncludes(cg);
  if (Option::EnableEval >= Option::LimitedEval ||
      cg.getOutput() == CodeGenerator::SystemCPP) {
    cg_printInclude("<runtime/eval/eval.h>");
  }

  // implementations
  cg.printImplStarter();
  cg.namespaceBegin();
  BOOST_FOREACH(FileScopePtr fs, files) {
    cg.setContext(CodeGenerator::CppImplementation);
    fs->outputCPPImpl(cg, ar);
    cg.setContext(CodeGenerator::CppPseudoMain);
    fs->outputCPPPseudoMain(cg, ar);
  }
  cg.namespaceEnd();
}


void AnalysisResult::outputCPPFFIStubs() {
  AnalysisResultPtr ar = shared_from_this();
  string iPath = m_outputPath + "/" + Option::FFIFilePrefix +
    "stubs.cpp";
  string hPath = m_outputPath + "/" + Option::FFIFilePrefix +
    "stubs.h";
  Util::mkdir(iPath);
  ofstream fi(iPath.c_str());
  ofstream fh(hPath.c_str());
  CodeGenerator cg(&fh, CodeGenerator::ClusterCPP);
  cg_printInclude("<runtime/base/hphp_ffi.h>");
  cg.printImplStarter();
  cg_printf("using namespace HPHP;\n");
  cg_printf("extern \"C\" {\n");

  cg.setStream(CodeGenerator::ImplFile, &fi);
  cg.useStream(CodeGenerator::ImplFile);
  cg_printInclude("\"stubs.h\"");
  cg_printf("/* preface starts */\n");
  cg_printf("using namespace HPHP;\n\n");
  cg_printf("/* preface finishes */\n");

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputCPPFFI(cg, ar);
  }

  cg.useStream(CodeGenerator::PrimaryStream);
  cg_printf("}\n");
  fi.close();
  fh.close();
}
void AnalysisResult::outputHSFFIStubs() {
  AnalysisResultPtr ar = shared_from_this();
  string path = m_outputPath + "/" + Option::FFIFilePrefix +
    "HphpStubs.hs";
  Util::mkdir(path);
  ofstream f(path.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg_printf("{-# INCLUDE \"stubs.h\" #-}\n");
  cg_printf("{-# LANGUAGE ForeignFunctionInterface #-}\n");
  cg_printf("module HphpStubs (");
  bool first = true;
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    const StringToFunctionScopePtrVecMap &fns = fs->getFunctions();
    for (StringToFunctionScopePtrVecMap::const_iterator it = fns.begin();
         it != fns.end(); ++it) {
      BOOST_FOREACH(FunctionScopePtr func, it->second) {
        if (func->inPseudoMain()) continue;
        if (first) {
          first = false;
        } else {
          cg_printf(", ");
        }
        cg_printf("f_%s", func->getId(cg).c_str());
      }
    }
  }
  cg_printf(") where\n");
  cg_printf("import HphpFFI\n");
  cg_printf("import Foreign.C\n");
  cg_printf("import Foreign.Ptr\n");
  cg_printf("import Foreign.Marshal.Alloc (alloca)\n");
  cg_printf("import Foreign.Storable (peek)\n");
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputHSFFI(cg, ar);
  }
  f.close();
}

void AnalysisResult::outputJavaFFIStubs() {
  AnalysisResultPtr ar = shared_from_this();

  string packageName = Option::JavaFFIRootPackage;
  string packageDir = packageName;
  Util::replaceAll(packageDir, ".", "/");

  string outputDir = ar->getOutputPath() + "/" + Option::FFIFilePrefix +
    packageDir + "/";
  Util::mkdir(outputDir);

  string mainFile = outputDir + "HphpMain.java";
  ofstream fmain(mainFile.c_str());
  CodeGenerator cg(&fmain, CodeGenerator::FileCPP);
  cg.setContext(CodeGenerator::JavaFFI);

  cg_printf("package %s;\n\n", packageName.c_str());
  cg_printf("import hphp.*;\n\n");

  cg_indentBegin("public class HphpMain extends Hphp {\n");

  // generate an "identify" method that identifies the exact class type
  // of an HphpObject.
  cg_indentBegin("public static HphpVariant identify(HphpVariant v) {\n");
  cg_indentBegin("try {if (v instanceof HphpObject) {\n");
  cg_printf("return identify(v.getVariantPtr());\n");
  cg_indentEnd("} } catch (NoClassDefFoundError e) { }\n");
  cg_printf("return v;\n");
  cg_indentEnd("}\n\n");

  cg_printf("public static native HphpVariant identify(long ptr);\n\n");

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputJavaFFI(cg, ar);
  }

  cg_indentEnd("}\n");
  fmain.close();
}

void AnalysisResult::outputJavaFFICppDecl() {
  AnalysisResultPtr ar = shared_from_this();
  string path = m_outputPath + "/" + Option::FFIFilePrefix +
    "java_stubs.h";
  Util::mkdir(path);
  ofstream f(path.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg.setContext(CodeGenerator::JavaFFICppDecl);

  cg_printf("extern \"C\" {\n\n");

  // the native signature of "identify"
  string packageName = Option::JavaFFIRootPackage;
  string mangledName = "Java_" + packageName + "_HphpMain_identify";
  Util::replaceAll(mangledName, ".", "_");
  cg_printf("JNIEXPORT jobject JNICALL\n");
  cg_printf("%s(JNIEnv *env, jclass main, jlong ptr);\n\n",
            mangledName.c_str());

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputJavaFFICPPStub(cg, ar);
  }

  cg_printf("}\n");

  f.close();
}

void AnalysisResult::outputJavaFFICppImpl() {
  AnalysisResultPtr ar = shared_from_this();
  string path = m_outputPath + "/" + Option::FFIFilePrefix +
    "java_stubs.cpp";
  Util::mkdir(path);
  ofstream f(path.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg.setContext(CodeGenerator::JavaFFICppImpl);

  cg_printInclude("<jni.h>");
  cg_printInclude("<runtime/base/hphp_ffi.h>");
  cg_printInclude("<runtime/base/object_data.h>");
  cg_printInclude("<runtime/base/type_conversions.h>");
  cg_printInclude("<runtime/base/builtin_functions.h>");
  cg_printInclude("<runtime/ext/ext.h>");
  cg_printInclude("\"stubs.h\"");
  cg_printInclude("\"java_stubs.h\"");

  cg.printImplStarter();
  cg_printf("/* preface starts */\n");

  cg_printf("\nusing namespace HPHP;\n\n");

  // forward declaration of exportVariantToJava
  cg_printf("jobject exportVariantToJava(JNIEnv *, jclass, void *, int);\n\n");

  cg_printf("/* preface finishes */\n");

  // the native implementation of "identify"
  string packageName = Option::JavaFFIRootPackage;
  string mangledName = "Java_" + packageName + "_HphpMain_identify";
  Util::replaceAll(mangledName, ".", "_");
  cg_printf("JNIEXPORT jobject JNICALL\n");
  cg_indentBegin("%s(JNIEnv *env, jclass main, jlong ptr) {\n",
                 mangledName.c_str());
  Util::replaceAll(packageName, ".", "/");
  cg_printf("String clsName = concat(\"%s/\", "
            "toString(x_get_class((Variant *)ptr)));\n",
            packageName.c_str());
  cg_printf("jclass cls = env->FindClass(clsName.c_str());\n");
  cg_printf("jmethodID init = env->GetMethodID(cls, \"<init>\", \"(J)V\");\n");
  cg_printf("return env->NewObject(cls, init, ptr);\n");
  cg_indentEnd("}\n\n");

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputJavaFFICPPStub(cg, ar);
  }

  f.close();
}

void AnalysisResult::outputSwigFFIStubs() {
  AnalysisResultPtr ar = shared_from_this();
  string path = m_outputPath + "/" + Option::FFIFilePrefix +
    Option::ProgramName + ".i";
  Util::mkdir(path);
  ofstream f(path.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("%%module %s\n%%{\n", Option::ProgramName.c_str());
  cg_printInclude("<ffi/swig/swig.h>");
  cg_printInclude("\"stubs.h\"");
  cg_printf("using namespace HPHP;\n");

  cg.setContext(CodeGenerator::SwigFFIImpl);

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputSwigFFIStubs(cg, ar);
  }

  cg_printf("%%}\n\n");

  cg_printf("/////////////////////\n");
  cg_printf("// system definitions\n");
  cg_printf("/////////////////////\n\n");

  cg_printf("%%include swig.i\n");

  cg_printf("\n/////////////////\n");
  cg_printf(  "// user functions\n");
  cg_printf(  "/////////////////\n\n");

  cg.setContext(CodeGenerator::SwigFFIDecl);

  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    fs->outputSwigFFIStubs(cg, ar);
  }

  f.close();
}

/**
 * Literal string to String precomputation
 */
string AnalysisResult::getLiteralStringName(int hash, int index) {
  assert(index >= 0);
  string name(Option::SystemGen ? "s_sys_ss" : "s_ss");
  name += boost::str(boost::format("%08x") % hash);
  if (index > 0) name += ("_" + lexical_cast<string>(index));
  return name;
}

int AnalysisResult::getLiteralStringId(const std::string &s, int &index) {
  int hash = hash_string(s.data(), s.size());
  if (hash < 0) hash = -hash;
  vector<string> &strings = m_namedStringLiterals[hash];
  unsigned int i = 0;
  for (; i < strings.size(); i++) {
    if (strings[i] == s) break;
  }
  if (i == strings.size()) {
    strings.push_back(s);
  }
  index = i;
  return hash;
}

string AnalysisResult::getFuncId(ClassScopePtr cls, FunctionScopePtr func) {
  CodeGenerator cg;
  if (cls) {
    return cls->getId(cg) + "::" + func->getId(cg);
  }
  return func->getId(cg);
}

string AnalysisResult::getParamRTTIEntryKey(ClassScopePtr cls,
                                            FunctionScopePtr func,
                                            const std::string &paramName) {
  return getFuncId(cls, func) + "::" + paramName;
}

void AnalysisResult::addParamRTTIEntry(ClassScopePtr cls,
                                       FunctionScopePtr func,
                                       const std::string &paramName) {
  string key = getParamRTTIEntryKey(cls, func, paramName);
  m_paramRTTIs[key] = m_paramRTTICounter++;
}

int AnalysisResult::getParamRTTIEntryId(ClassScopePtr cls,
                                        FunctionScopePtr func,
                                        const std::string &paramName) {
  string key = getParamRTTIEntryKey(cls, func, paramName);
  map<string, int>::const_iterator it = m_paramRTTIs.find(key);
  if (it == m_paramRTTIs.end()) return -1;
  return it->second;
}

void AnalysisResult::addRTTIFunction(const std::string &id) {
  m_rttiFuncs.insert(id);
}

void AnalysisResult::cloneRTTIFuncs
(ClassScopePtr cls, const StringToFunctionScopePtrVecMap &functions) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
       functions.begin(); iter != functions.end(); ++iter) {
    for (unsigned int j = 0; j < iter->second.size(); j++) {
      FunctionScopePtr func = iter->second[j];
      const string funcId = getFuncId(cls, func);
      if (RTTIInfo::TheRTTIInfo.exists(funcId.c_str())) {
        StatementPtr stmt = func->getStmt();
        func->setStmtCloned(stmt->clone());
      }
    }
  }
}

void AnalysisResult::cloneRTTIFuncs(const char *RTTIDirectory) {
  RTTIInfo::TheRTTIInfo.loadMetaData(Option::RTTIOutputFile.c_str());
  RTTIInfo::TheRTTIInfo.loadProfData(RTTIDirectory);

  for (unsigned int i = 0; i < m_fileScopes.size(); i++) {
    // standalone rtti functions
    cloneRTTIFuncs(ClassScopePtr(), m_fileScopes[i]->getFunctions());

    // class rtti methods
    for (StringToClassScopePtrVecMap::const_iterator iter =
         m_fileScopes[i]->getClasses().begin();
         iter != m_fileScopes[i]->getClasses().end(); ++iter) {
      for (unsigned int j = 0; j < iter->second.size(); j++) {
        ClassScopePtr cls = iter->second[j];
        cloneRTTIFuncs(cls, cls->getFunctions());
      }
    }
  }
}


void AnalysisResult::pushCallInfo(int cit) {
  m_callInfos.push_back(cit);
}
void AnalysisResult::popCallInfo() {
  m_callInfos.pop_back();
}
int AnalysisResult::callInfoTop() {
  if (m_callInfos.empty()) return -1;
  return m_callInfos.back();
}

void AnalysisResult::outputCPPNamedLiteralStrings(bool genStatic,
                                                  const string &file) {
  AnalysisResultPtr ar = shared_from_this();
  string filename = genStatic ? file : (file + ".h");
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  if (Option::SystemGen) {
    cg_printf("#ifndef __GENERATED_gen_sys_literal_strings_h__\n");
    cg_printf("#define __GENERATED_gen_sys_literal_strings_h__\n");
  } else {
    cg_printf("#ifndef __GENERATED_sys_literal_strings_h__\n");
    cg_printf("#define __GENERATED_sys_literal_strings_h__\n");
  }
  if (!genStatic) {
    cg_printInclude("<runtime/base/hphp.h>");
    cg_printf("\n");
  }
  cg.printImplStarter();
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedStringLiterals.begin(); it != m_namedStringLiterals.end();
       it++) {
    int hash = it->first;
    vector<string> &strings = m_namedStringLiterals[hash];
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getLiteralStringName(hash, i);
      if (genStatic) {
        cg_printf("static StaticString %s(", name.c_str());
        cg_printString(strings[i], ar, BlockScopePtr(), false);
        cg_printf(");\n");
      } else {
        cg_printf("extern StaticString %s;\n", name.c_str());
      }
    }
  }
  cg.namespaceEnd();
  if (Option::SystemGen) {
    cg_printf("#endif // __GENERATED_gen_sys_literal_strings_h__\n");
  } else {
    cg_printf("#endif // __GENERATED_sys_literal_strings_h__\n");
  }
  f.close();

  if (genStatic) return;

  filename = file + ".no.cpp";
  f.open(filename.c_str());
  cg_printf("\n");
  cg_printInclude("\"literal_strings.h\"");
  cg_printf("\n");
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedStringLiterals.begin(); it != m_namedStringLiterals.end();
       it++) {
    int hash = it->first;
    vector<string> &strings = m_namedStringLiterals[hash];
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getLiteralStringName(hash, i);
      cg_printf("StaticString %s(", name.c_str());
      cg_printString(strings[i], ar, BlockScopePtr(), false);
      cg_printf(");\n");
    }
  }
  cg.namespaceEnd();
}

void AnalysisResult::outputCPPSepExtensionMake() {
  string filename = m_outputPath + "/sep_extensions.mk";
  Util::mkdir(filename);
  ofstream f(filename.c_str());

  f << "\nSEP_EXTENSION_INCLUDE_PATHS = \\\n";
  for (unsigned int i = 0; i < Option::SepExtensions.size(); i++) {
    Option::SepExtensionOptions &options = Option::SepExtensions[i];
    if (options.include_path.empty()) {
      f << "\t-I $(PROJECT_ROOT)/src/runtime/ext/sep/" << options.name;
    } else {
      f << "\t-I " << options.include_path;
    }
    f << " \\\n";
  }

  f << "\nSEP_EXTENSION_LIBS = \\\n";
  for (unsigned int i = 0; i < Option::SepExtensions.size(); i++) {
    Option::SepExtensionOptions &options = Option::SepExtensions[i];
    if (options.shared) {
      if (options.lib_path.empty()) {
        f << "\t-L $(PROJECT_ROOT)/src/runtime/ext/sep/" << options.name;
      } else {
        f << "\t-L " << options.lib_path;
      }
      f << " -l" << options.name << " \\\n";
    } else {
      if (options.lib_path.empty()) {
        f << "\t$(PROJECT_ROOT)/src/runtime/ext/sep/" << options.name;
      } else {
        f << "\t" << options.lib_path;
      }
      f << "/lib" << options.name << ".a \\\n";
    }
  }

  f.close();
}

void AnalysisResult::outputCPPSepExtensionIncludes(CodeGenerator &cg) {
  for (unsigned int i = 0; i < Option::SepExtensions.size(); i++) {
    Option::SepExtensionOptions &options = Option::SepExtensions[i];
    string include = options.include_path;
    if (!include.empty()) {
      if (include[include.size() - 1] != '/') {
        include += '/';
      }
      cg_printf("#include \"%sextprofile_%s.h\"\n",
                include.c_str(), options.name.c_str());
    } else {
      cg_printf("#include <extprofile_%s.h>\n", options.name.c_str());
    }
  }
}

void AnalysisResult::outputCPPSepExtensionImpl(const std::string &filename) {
  AnalysisResultPtr ar = shared_from_this();
  MethodSlot::genMethodSlot(ar);

  ofstream fTable(filename.c_str());
  CodeGenerator cg(&fTable, CodeGenerator::SystemCPP);

  outputCPPDynamicTablesHeader(cg, true, false, true);

  for (unsigned int i = 0; i < Option::SepExtensions.size(); i++) {
    Option::SepExtensionOptions &options = Option::SepExtensions[i];
    cg_printf("#include \"ext_%s.h\"\n", options.name.c_str());
  }
  string litstrFile = filename + ".litstr";
  cg_printf("#include \"%s\"\n", litstrFile.c_str());

  cg_printf("\n");
  cg.printImplStarter();
  cg_printf("using namespace std;\n");
  cg.namespaceBegin();

  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    ClassScopePtr cls = iter->second;
    if (cls->isSepExtension()) {
      cls->outputCPPDynamicClassImpl(cg, ar);
      cls->outputCPPSupportMethodsImpl(cg, ar);
    }
  }

  cg.namespaceEnd();
  fTable.close();
  outputCPPNamedLiteralStrings(true, litstrFile);
  if (m_scalarArrays.size()) assert(false);
}

//
// Map every method to a (callIndex, class_id) pair
#define DEBUG_GMS 0

namespace HPHP {

ostream & operator<< (ostream &cout, const MethodSet& s){
  cout << '{' ;
  BOOST_FOREACH (int i, s)  cout << i << ',';
  return cout << '}';
}

ostream& MethodSlot::operator<<(ostream& cout) const {
  return cout << "MethodSlot{" << setw(20) << m_name
    << setw(5) << m_serialNum << ','
    << setw(4) << m_callIndex << ':' <<  m_overloadIndex
    << " (" << m_coalesces << ") "
    << conflictingMethods << "}";
}

std::string MethodSlot::runObj() const {
  if (isError()) {
    // statically named method does not exist,
    // caller must detect for cgenerating correct call
    return "MethodIndex.fail()" ;
  }
  boost::format ret ("MethodIndex(%d, %d) /* %s */ ") ;
  ret % m_callIndex % m_overloadIndex % m_name;
  return ret.str();
}

std::string MethodSlot::runObjParam() const {
  if (isError()) return "";
  return std::string("/* ")+m_name+" */ " + runObj() + ", ";
}

// Used for internal generation that mus succeed
const MethodSlot*
AnalysisResult::getMethodSlot(const std::string & mname) const {
  StringToMethodSlotMap::const_iterator method =
    stringToMethodSlotMap.find(mname);
  assert (method != stringToMethodSlotMap.end()) ;
  return &((*method).second);
}

// Used for a user-provided method name that might not exist,
// MethodSlot::runObj will return a call that creates this at runtime.
const MethodSlot*
AnalysisResult::getOrAddMethodSlot(const std::string &mname,
                                   ConstructPtr self) {
  if (!Option::UseMethodIndex) {
    return NULL;
  }
  StringToMethodSlotMap::const_iterator method =
    stringToMethodSlotMap.find(mname);
  if (method == stringToMethodSlotMap.end()) {
    Compiler::Error(Compiler::UnknownObjectMethod, self, mname);
    // too late to add to stringToMethodSlotMap, getMethodSlot already done
    return errorMethodSlot;
  };
  return &((*method).second);
}

ostream& operator << (ostream &cout, const MethodSlot& m) {
  return m.operator<<(cout);
}

ostream& operator<<(ostream & cout, const StringToMethodSlotMap & map) {
  cout << "StringToMethodSlotMap = {" << endl;
  BOOST_FOREACH(StringToMethodSlotMap::value_type slot,  map) {
    cout << slot.second << endl;
  }
  return cout << '}' << endl ;
}

ostream& operator<<(ostream & cout, const CallIndexVectSet & civs) {
  for(unsigned int i = 0; i < civs.size(); ++i) {
    cout << i << ':' << civs[i] << endl;
  }
  return cout;
}

struct CoalesceOrder {
  bool operator()(const MethodSlot *x, const MethodSlot *y) const {
    return *x > y;
  }
};

// Used only during table gen
MethodSlot*
AnalysisResult::getMethodSlotUpdate(const std::string & mname) {
  StringToMethodSlotMap::iterator method = stringToMethodSlotMap.find(mname);
  assert (method != stringToMethodSlotMap.end()) ;
  return &((*method).second);
}

const MethodSlot* errorMethodSlot(NULL);

void MethodSlot::genMethodSlot(AnalysisResultPtr ar) {

struct Nested {
static void buildMethodSlotsAndConflict(AnalysisResultPtr ar,
                                        ClassScopePtrVec & cspv) {
  ClassScopePtr cls;
  BOOST_FOREACH(cls, cspv) {
    // const StringToFunctionScopePtrVecMap & funcs = cls->getFunctions();
    StringToFunctionScopePtrMap funcs;
    cls->collectMethods(ar, funcs);
    vector<MethodSlot *> methodsProcessed;
    methodsProcessed.reserve(funcs.size());
    FunctionScopePtr fsp;
    BOOST_FOREACH(tie(tuples::ignore, fsp), funcs) {
      string mname (fsp->getOriginalName());
#     if DEBUG_GMS
      printf ("process %s::%s\n", cls->getName().c_str(), mname.c_str());
#     endif
      StringToMethodSlotMap::iterator name_it
        (ar->stringToMethodSlotMap.find(mname));
      if (name_it == ar->stringToMethodSlotMap.end() ) {
        unsigned int serialNum = ar->stringToMethodSlotMap.size() + 1;
        pair <StringToMethodSlotMap::iterator, bool> ret =
          ar->stringToMethodSlotMap.insert(
              (pair<string, MethodSlot>(mname, MethodSlot(mname, serialNum))));
        name_it = ret.first;
      }
      (*name_it).second.m_coalesces++;

      // build the interference graph
      MethodSlot* currentMethodSlot(ar->getMethodSlotUpdate(mname));
      BOOST_FOREACH(MethodSlot * prevMethodSlot, methodsProcessed) {
        currentMethodSlot->conflictingMethods.insert(
          prevMethodSlot->m_serialNum);
        prevMethodSlot->conflictingMethods.insert
          (currentMethodSlot->m_serialNum);
      }
      methodsProcessed.push_back(currentMethodSlot);
    }
  }
}
};

  errorMethodSlot = new MethodSlot("**ERROR**", 0);
  // consider: class Foo { fun1; } class Bar { fun2; fun4; }
  // class Baz { fun1; fun2; }  class Qux { fun1; fun3; }
  // need a conflict graph to be sure fun1 and fun2 have different m_callIndex
  // callIndex 1: fun1, fun4
  // callIndex 2: fun2, fun3

  // build a global index of all methods declared anywhere
  // and an interference graph of methods names that must have a different
  // callIndex

  ar->stringToMethodSlotMap.clear();
  string cname;
  ClassScopePtr cls;

  ClassScopePtrVec clsuser, clssystem;
  {
    unsigned int size = 0;
    ClassScopePtrVec cspv;
    BOOST_FOREACH(tie(cname, cspv), ar->m_classDecs) size += cspv.size();
    clsuser.reserve(size);
    BOOST_FOREACH(tie(cname, cspv), ar->m_classDecs) {
      BOOST_FOREACH(cls, cspv) {
        if (Option::SystemGen) {
          // on a system gen, some classes, e.g. serializable
          // look like user user classes
          clssystem.push_back(cls);
        } else {
          clsuser.push_back(cls);
        }
      }
    }
    clssystem.reserve(ar->m_systemClasses.size());
    BOOST_FOREACH(tie(cname, cls), ar->m_systemClasses) {
      clssystem.push_back(cls);
    }
  }

  // must have the same order when generating system files
  // as when compiling user code, to get the same methodIndex's
  sort(clssystem.begin(), clssystem.end(), bind(std::less<std::string>(),
                             bind(&ClassScope::getOriginalName, _1),
                             bind(&ClassScope::getOriginalName, _2) ));

  Nested::buildMethodSlotsAndConflict(ar, clssystem);
  Nested::buildMethodSlotsAndConflict(ar, clsuser);

  // assign m_callIndex & m_overloadIndex IDs for system classes.
  // The system classes are generated before the user program
  // into system/gen/*. When the user method invoke is generated later, it
  // must agree with that previously generated system class conflict graph.
  // There is no way to know ahead of time what user methods might
  // induce conflicts in the system classes.  Therefore, the system
  // methods must be laid out without any coalescing, and must be done
  // independent of user methods.
  // This is suboptimal because system class o_invoke's are no longer
  // compacted and no longer have jump-table switch statements.
  // Perhaps the MethodIndex range for system classes and user classes
  // could be split (e.g < 1000 for system), then the parent invoke
  // from a user class that derives from a system class would need a
  // separate methodIndexLookup to go the system::foo instead of
  // userclass::foo.  Direct invokes of system methods could start
  // with the right methodIndexLookup
  BOOST_FOREACH(cls, clssystem) {
    StringToFunctionScopePtrMap funcs;
    cls->collectMethods(ar, funcs);
    string mname;

    // assign the unassigned ones
    BOOST_FOREACH(tie(mname, tuples::ignore), funcs) {
      MethodSlot * methodSlot = ar->getMethodSlotUpdate(mname);
      // already assigned from a different class
      if (methodSlot->m_callIndex) continue;

      unsigned int callIndex = ar->callIndexVectSet.size() + 1;
      ar->callIndexVectSet.resize(callIndex);
      MethodSet& methodsAtCallIndex = ar->callIndexVectSet.back();
      methodsAtCallIndex.insert(methodSlot->m_serialNum);
      methodSlot->m_overloadIndex = 1;
      methodSlot->m_callIndex = callIndex;
      ++callIndex;
    }
  }

  unsigned int maxSystemMethodCallIndex = ar->callIndexVectSet.size();

  // assign m_callIndex & m_overloadIndex IDs for user classes
  map<string, unsigned int> classMax, classMin;
  BOOST_FOREACH(cls, clsuser) {
    StringToFunctionScopePtrMap funcs;
    cls->collectMethods(ar, funcs);
    vector<MethodSlot*> classMethods;
    classMethods.reserve(funcs.size());
    string mname;
    BOOST_FOREACH(tie(mname, tuples::ignore), funcs) {
      classMethods.push_back(ar->getMethodSlotUpdate(mname));
    }

    // number most conflicted first
    sort(classMethods.begin(), classMethods.end(), CoalesceOrder());

    // min previously assigned callIndex;
    unsigned int callIndex = maxSystemMethodCallIndex + 1;
    // FMC: callIndex could still start at 1, mixing user methods
    // with pre-assigned system method call indices, need to
    // experiment with this.
    BOOST_FOREACH (MethodSlot* method, classMethods) {
      if (method->m_callIndex && method->m_callIndex>maxSystemMethodCallIndex) {
        callIndex = std::min(callIndex, method->m_callIndex);
      }
    }
    // assign the unassigned ones
    BOOST_FOREACH (MethodSlot* methodSlot, classMethods) {
      // already assigned from a different class
      if (methodSlot->m_callIndex) continue;

      MethodSet& conflictingMethods (methodSlot->conflictingMethods);
      // start scanning up for a unconflicted callIndex
      MethodSet* methodsAtCallIndex;
      for (;callIndex <= ar->callIndexVectSet.size(); ++callIndex) {
        methodsAtCallIndex = &ar->callIndexVectSet[callIndex-1];
        bool conflict = false;
        // intersect methods used at this index with conflicting methods
        for (MethodSet::iterator i = methodsAtCallIndex->begin(),
            j = conflictingMethods.begin();
            !conflict && i != methodsAtCallIndex->end()
            && j != conflictingMethods.end(); ) {
          if (*i == *j) conflict = true;
          else if (*i < *j) ++i;
          else ++j;
        }
        if (!conflict) break;
        // e.g. "Qux::fun3"->conflictingMethods = {"fun1"}
        // callIndex==1 is {"fun1", "fun4"},
        // so fun3 can't have callIndex==1
      }
      if (callIndex >= ar->callIndexVectSet.size()) {
        ar->callIndexVectSet.resize(callIndex);
        methodsAtCallIndex = &ar->callIndexVectSet[callIndex-1];
      }
      methodsAtCallIndex->insert(methodSlot->m_serialNum);
      methodSlot->m_overloadIndex = methodsAtCallIndex->size();
      methodSlot->m_callIndex = callIndex;
      ++callIndex;

#     if DEBUG_GMS
      unsigned int & t (classMax[cls->getOriginalName()]) ;
      if (t < methodSlot->m_callIndex) t = methodSlot->m_callIndex;
      unsigned int & t2 (classMin[cls->getOriginalName()]) ;
      if (t2 == 0 || t2 > methodSlot->m_callIndex) t2 = methodSlot->m_callIndex;
#     endif
    }
  }
# if DEBUG_GMS
  cout << "stringToMethodSlotMap" << endl;
  cout << ar->stringToMethodSlotMap;
  cout << "callIndexVectSet" << endl;
  cout << ar->callIndexVectSet;

  cout << "max indices : number of functions" << endl;
  ClassScopePtrVec clsvec;
  clsvec.resize(clsuser.size()+clssystem.size());
  set_union(clsuser.begin(), clsuser.end(),
            clssystem.begin(), clssystem.end(),
            clsvec.begin());
  BOOST_FOREACH(cls, clsvec) {
    cout << setw(3) << classMax[cls->getOriginalName()]
         - classMin[cls->getOriginalName()]+1
      << ' ' << cls->getFunctions().size() << ' ' <<  cls->getOriginalName()
      << " {" ;
    string name;
    BOOST_FOREACH(tie(name, tuples::ignore), cls->getFunctions()) {
      cout << name << ", ";
    }
    cout << "}" << endl;
  }
# endif
}
void MethodSlot::emitMethodSlot(CodeGenerator &cg, AnalysisResultPtr ar,
                                bool system) {
  cg_printf("#define M(x, y) MethodIndex(x, y)\n");
  const char * sysTable = system ? "Sys" : "";

  // predefined, statically constructed, hash table for quick startup,
  // and hopefully fast lookup
  // methodIndexHMap[name]->MethodIndex
  // this uses linear probing since it's difficult to construct a hash bucket
  // chain statically.
  unsigned tableSize = ar->stringToMethodSlotMap.size() * 2;
  struct MethodIndexHMap {
    MethodIndexHMap() : name(NULL), methodIndex(0,0) {}
    MethodIndexHMap(const char *name, unsigned callIndex,
                    unsigned overloadIndex)
      : name(name), methodIndex(callIndex, overloadIndex) {}
    const char *name;
    MethodIndex methodIndex;
  };
  MethodIndexHMap methodIndexHMap [tableSize];

  BOOST_FOREACH(StringToMethodSlotMap::value_type slot,
                ar->stringToMethodSlotMap) {
    ASSERT(!slot.second.isFail());
    const char *name = slot.first.c_str();
    unsigned hash = (unsigned)(hash_string_i(name) % tableSize);
    while (methodIndexHMap[hash].name) {
      hash = hash ? hash - 1 : tableSize - 1;
    }
    methodIndexHMap[hash] = MethodIndexHMap(name,
                                            slot.second.getCallIndex(),
                                            slot.second.getOverloadIndex());
  }

  cg_printf("#define H(x,y,z) MethodIndexHMap(#x,MethodIndex(y,z))\n");
  cg_printf("#define Z MethodIndexHMap(0,MethodIndex(0,0))\n");
  cg_printf("const unsigned g_methodIndexHMapSize%s = %u;\n",
            sysTable, tableSize);
  cg_printf("extern const MethodIndexHMap g_methodIndexHMap%s [];\n",
      sysTable);
  cg_printf("const MethodIndexHMap g_methodIndexHMap%s "
      "[g_methodIndexHMapSize%s] "
            "= {\n", sysTable, sysTable);
  int wrap=0;
  for (unsigned i=0; i<tableSize; ++i)  {
    if (methodIndexHMap[i].name) {
      cg_printf ("H(%s,%d,%d)",
                 methodIndexHMap[i].name,
                 methodIndexHMap[i].methodIndex.m_callIndex,
                 methodIndexHMap[i].methodIndex.m_overloadIndex);
    } else {
      cg_printf ("Z");
    }
    if (i < tableSize - 1 ) cg_printf (", ");
    if (++wrap == 3) {
      cg_printf ("\n");
      wrap=0;
    }
  }
  cg_printf ("};\n");
  cg_printf("#undef M\n");
  cg_printf("#undef H\n");
  cg_printf("#undef Z\n");

  // reverse map, MethodIndex -> name
  unsigned mapSize = ar->callIndexVectSet.size();
  unsigned maxOverload = 0;
  // linearize all MethodIndex's, e.g. MI(1,1), MI(1,2), MI(1,3), MI(2,1)
  // methodIndexReverseIndex[methodIndexReverseCallIndex[
  //    methodIndex.getCallIndex()] + methodIndex.getOverloadIndex()] =
  //        methodSlot.getName()
  cg_printf("extern const unsigned g_methodIndexReverseCallIndex%s[];\n",
            sysTable);
  cg_printf("const unsigned g_methodIndexReverseCallIndex%s[] = {0, \n",
            sysTable);
  for (unsigned i=1; i<mapSize; ++i) {
    maxOverload += ar->callIndexVectSet[i - 1].size();
    cg_printf("%u", maxOverload);
    if (i < mapSize - 1 ) cg_printf (",");
    if (++wrap == 20) {
      cg_printf ("\n");
      wrap = 0;
    }
  }
  cg_printf("};\n");

  map<unsigned int, const char *> serialNumIndex; // serialNum -> methodName
  BOOST_FOREACH(StringToMethodSlotMap::value_type slot,
                ar->stringToMethodSlotMap) {
    serialNumIndex[slot.second.m_serialNum] = slot.first.c_str();
  }

  wrap = 0;
  cg_printf("extern const char * g_methodIndexReverseIndex%s[];\n", sysTable);
  cg_printf("const char * g_methodIndexReverseIndex%s[] = {\n", sysTable);
  for (unsigned callIndex=0; callIndex<mapSize; ++callIndex) {
    MethodSet& methodsAtCallIndex (ar->callIndexVectSet[callIndex]);
    BOOST_FOREACH(int serialNum, methodsAtCallIndex) {
      cg_printf("\"%s\"", serialNumIndex[serialNum]);
      if (callIndex < mapSize - 1 ) cg_printf(", ");
      if (++wrap == 5) {
        cg_printf ("\n");
        wrap = 0;
      }
    }
  }
  cg_printf("};\n");
}
}
