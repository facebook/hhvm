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

#include <iomanip>
#include <algorithm>
#include <sstream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/depth_first_visitor.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/if_branch_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/loop_statement.h>
#include <compiler/statement/class_variable.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/analysis/symbol_table.h>
#include <compiler/package.h>
#include <compiler/parser/parser.h>
#include <compiler/option.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/builtin_symbols.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/array_pair_expression.h>
#include <runtime/base/rtti_info.h>
#include <runtime/ext/ext_json.h>
#include <runtime/base/zend/zend_printf.h>
#include <util/atomic.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/hash.h>
#include <util/process.h>
#include <util/job_queue.h>
#include <util/timer.h>

using namespace HPHP;
using std::map;
using std::set;
using std::ostringstream;
using std::ofstream;
using std::ifstream;
using std::pair;

///////////////////////////////////////////////////////////////////////////////
// initialization

IMPLEMENT_THREAD_LOCAL(BlockScopeRawPtr,
                       AnalysisResult::s_currentScopeThreadLocal);

IMPLEMENT_THREAD_LOCAL(BlockScopeRawPtrFlagsHashMap,
                       AnalysisResult::s_changedScopesMapThreadLocal);

AnalysisResult::AnalysisResult()
  : BlockScope("Root", "", StatementPtr(), BlockScope::ProgramScope),
    m_arrayLitstrKeyMaxSize(0), m_arrayIntegerKeyMaxSize(0),
    m_package(NULL), m_parseOnDemand(false), m_phase(ParseAllFiles),
    m_scalarArraysCounter(0), m_paramRTTICounter(0),
    m_scalarArraySortedAvgLen(0), m_scalarArraySortedIndex(0),
    m_scalarArraySortedSumLen(0), m_scalarArrayCompressedTextSize(0),
    m_pregenerating(false), m_pregenerated(false),
    m_system(false), m_sepExtension(false) {
  m_classForcedVariants[0] = m_classForcedVariants[1] = false;
}

void AnalysisResult::appendExtraCode(const std::string &key,
                                     const std::string &code) {
  string &extraCode = m_extraCodes[key];

  if (extraCode.empty()) {
    extraCode = "<?php\n";
  }
  extraCode += code + "\n";
}

void AnalysisResult::appendExtraCode(const std::string &key,
                                     const std::string &code) const {
  lock()->appendExtraCode(key, code);
}

void AnalysisResult::parseExtraCode(const string &key) {
  Lock lock(getMutex());
  map<string, string>::iterator iter = m_extraCodes.find(key);
  if (iter != m_extraCodes.end()) {
    string code = iter->second;
    string sfilename = iter->first + "." + Option::LambdaPrefix + "lambda";
    m_extraCodes.erase(key);

    const char *filename = m_extraCodeFileNames.add(sfilename.c_str());
    Compiler::Parser::ParseString(code, shared_from_this(), filename, true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// general functions

void AnalysisResult::addFileScope(FileScopePtr fileScope) {
  ASSERT(fileScope);
  FileScopePtr &res = m_files[fileScope->getName()];
  ASSERT(!res);
  res = fileScope;
  vertex_descriptor vertex = add_vertex(m_depGraph);
  fileScope->setVertex(vertex);
  m_fileVertMap[vertex] = fileScope;
  m_fileScopes.push_back(fileScope);
}

bool AnalysisResult::inParseOnDemandDirs(const string &filename) const {
  for (size_t i = 0; i < m_parseOnDemandDirs.size(); i++) {
    if (filename.find(m_parseOnDemandDirs[i]) == 0) return true;
  }
  return false;
}

void AnalysisResult::parseOnDemand(const std::string &name) const {
  if (m_package) {
    const std::string &root = m_package->getRoot();
    string rname = name;
    if (name.find(root) == 0) {
      rname = name.substr(root.length());
    }
    if ((m_parseOnDemand || inParseOnDemandDirs(rname)) &&
        Option::PackageExcludeFiles.find(rname) ==
        Option::PackageExcludeFiles.end() &&
        !Option::IsFileExcluded(rname, Option::PackageExcludePatterns)) {
      {
        Locker l(this);
        if (m_files.find(rname) != m_files.end()) return;
      }
      m_package->addSourceFile(rname.c_str());
    }
  }
}

void AnalysisResult::parseOnDemandBy(const string &name,
                                     const map<string,string> &amap) const {
  if (m_package) {
    auto it = amap.find(name);
    if (it != amap.end()) {
      parseOnDemand(Option::AutoloadRoot + it->second);
    }
  }
}

FileScopePtr AnalysisResult::findFileScope(const std::string &name) const {
  StringToFileScopePtrMap::const_iterator iter = m_files.find(name);
  if (iter != m_files.end()) {
    return iter->second;
  }
  return FileScopePtr();
}

FunctionScopePtr AnalysisResult::findFunction(
  const std::string &funcName) const {
  StringToFunctionScopePtrMap::const_iterator bit =
    m_functions.find(funcName);
  if (bit != m_functions.end()) {
    return bit->second;
  }
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functionDecs.find(funcName);
  if (iter != m_functionDecs.end()) {
    return iter->second;
  }
  return FunctionScopePtr();
}

BlockScopePtr AnalysisResult::findConstantDeclarer(
  const std::string &name) {
  if (getConstants()->isPresent(name)) return shared_from_this();
  StringToFileScopePtrMap::const_iterator iter = m_constDecs.find(name);
  if (iter != m_constDecs.end()) return iter->second;
  return BlockScopePtr();
}

ClassScopePtr AnalysisResult::findClass(const std::string &name) const {
  AnalysisResultConstPtr ar = shared_from_this();
  string lname = Util::toLower(name);
  StringToClassScopePtrMap::const_iterator sysIter =
    m_systemClasses.find(lname);
  if (sysIter != m_systemClasses.end()) return sysIter->second;

  StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.find(lname);
  if (iter != m_classDecs.end() && iter->second.size()) {
    return iter->second.back();
  }
  return ClassScopePtr();
}

ClassScopePtr AnalysisResult::findClass(const std::string &name,
                                        FindClassBy by) {
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
    return findClass(name);
  }
  return ClassScopePtr();
}

const ClassScopePtrVec &AnalysisResult::findRedeclaredClasses
(const std::string &name) const {
  StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.find(name);
  if (iter == m_classDecs.end()) {
    static ClassScopePtrVec empty;
    empty.clear();
    return empty;
  }
  return iter->second;
}

ClassScopePtrVec AnalysisResult::findClasses(const std::string &name) const {
  StringToClassScopePtrMap::const_iterator sysIter =
    m_systemClasses.find(name);
  if (sysIter != m_systemClasses.end()) {
    return ClassScopePtrVec(1, sysIter->second);
  }

  return findRedeclaredClasses(name);
}

bool AnalysisResult::classMemberExists(const std::string &name,
                                       FindClassBy by) const {
  if (by == MethodName) {
    return m_methodToClassDecs.find(name) != m_methodToClassDecs.end();
  }
  return m_classDecs.find(name) != m_classDecs.end();
}

ClassScopePtr AnalysisResult::findExactClass(ConstructPtr cs,
                                             const std::string &name) const {
  ClassScopePtr cls = findClass(name);
  if (!cls || !cls->isRedeclaring()) return cls;
  if (ClassScopePtr currentCls = cs->getClassScope()) {
    if (cls->getName() == currentCls->getName()) {
      return currentCls;
    }
  }
  if (FileScopePtr currentFile = cs->getFileScope()) {
    return currentFile->resolveClass(cls);
  }
  return ClassScopePtr();
}

bool AnalysisResult::checkClassPresent(ConstructPtr cs,
                                       const std::string &name) const {
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

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool AnalysisResult::declareFunction(FunctionScopePtr funcScope) const {
  ASSERT(m_phase < AnalyzeAll);

  string fname = funcScope->getName();
  // System functions override
  if (m_functions.find(fname) != m_functions.end()) {
    // we need someone to hold on to a reference to it
    // even though we're not going to do anything with it
    this->lock()->m_ignoredScopes.push_back(funcScope);
    return false;
  }

  return true;
}

bool AnalysisResult::declareClass(ClassScopePtr classScope) const {
  ASSERT(m_phase < AnalyzeAll);

  string cname = classScope->getName();
  // System classes override
  if (m_systemClasses.find(cname) != m_systemClasses.end()) {
    // we need someone to hold on to a reference to it
    // even though we're not going to do anything with it
    this->lock()->m_ignoredScopes.push_back(classScope);
    return false;
  }

  int mask =
    (m_classForcedVariants[0] ? VariableTable::NonPrivateNonStaticVars : 0) |
    (m_classForcedVariants[1] ? VariableTable::NonPrivateStaticVars : 0);

  if (mask) {
    AnalysisResultConstPtr ar = shared_from_this();
    classScope->getVariables()->forceVariants(ar, mask);
  }
  return true;
}

void AnalysisResult::declareUnknownClass(const std::string &name) {
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

static bool by_source(const BlockScopePtr &b1, const BlockScopePtr &b2) {
  return b1->getStmt()->getLocation()->
    compare(b2->getStmt()->getLocation().get()) < 0;
}

void AnalysisResult::canonicalizeSymbolOrder() {
  getConstants()->canonicalizeSymbolOrder();
  getVariables()->canonicalizeSymbolOrder();

  AnalysisResultPtr ar = shared_from_this();
  for (StringToClassScopePtrVecMap::iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    ClassScopePtrVec &classes = iter->second;
    if (classes.size() > 1) {
      sort(classes.begin(), classes.end(), by_source);
      for (unsigned int i = 0; i < classes.size(); i++) {
        classes[i]->setRedeclaring(ar, i);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dependencies

void AnalysisResult::link(FileScopePtr user, FileScopePtr provider) {
  if (user != provider) {
    bool needsLock = getPhase() != AnalyzeAll &&
                     getPhase() != AnalyzeFinal;
    ConditionalLock lock(m_depGraphMutex, needsLock);
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
  if (iter == m_classDecs.end() || !iter->second.size()) return false;
  ClassScopePtr classScope = iter->second[0];
  if (iter->second.size() != 1) {
    classScope = usingFile->resolveClass(classScope);
    if (!classScope) return false;
  }
  FileScopePtr fileScope = classScope->getContainingFile();
  link(usingFile, fileScope);
  return true;
}

bool AnalysisResult::addFunctionDependency(FileScopePtr usingFile,
                                           const std::string &functionName) {
  if (BuiltinSymbols::s_functions.find(functionName) !=
      BuiltinSymbols::s_functions.end())
    return true;
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functionDecs.find(functionName);
  if (iter == m_functionDecs.end()) return false;
  FunctionScopePtr functionScope = iter->second;
  if (functionScope->isRedeclaring()) {
    functionScope = usingFile->resolveFunction(functionScope);
    if (!functionScope) return false;
  }
  FileScopePtr fileScope = functionScope->getContainingFile();
  link(usingFile, fileScope);
  return true;
}

bool AnalysisResult::addIncludeDependency(FileScopePtr usingFile,
                                          const std::string &includeFilename) {
  ASSERT(!includeFilename.empty());
  FileScopePtr fileScope = findFileScope(includeFilename);
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

  StringToFileScopePtrMap::const_iterator iter =
    m_constDecs.find(constantName);
  if (iter == m_constDecs.end()) return false;
  FileScopePtr fileScope = iter->second;
  link(usingFile, fileScope);
  return true;
}

bool AnalysisResult::isConstantDeclared(const std::string &constName) const {
  if (m_constants->isPresent(constName)) return true;
  StringToFileScopePtrMap::const_iterator iter = m_constDecs.find(constName);
  if (iter == m_constDecs.end()) return false;
  FileScopePtr fileScope = iter->second;
  ConstantTablePtr constants = fileScope->getConstants();
  ConstructPtr decl = constants->getValue(constName);
  if (decl) return true;
  return false;
}

bool AnalysisResult::isConstantRedeclared(const std::string &constName) const {
  return m_constRedeclared.find(constName) != m_constRedeclared.end();
}

bool AnalysisResult::isSystemConstant(const std::string &constName) const {
  return m_constants->isSystem(constName);
}

///////////////////////////////////////////////////////////////////////////////
// Program

void AnalysisResult::loadBuiltinFunctions() {
  AnalysisResultPtr ar = shared_from_this();
  BuiltinSymbols::LoadFunctions(ar, m_functions);
}

void AnalysisResult::loadBuiltins() {
  AnalysisResultPtr ar = shared_from_this();
  BuiltinSymbols::LoadFunctions(ar, m_functions);
  BuiltinSymbols::LoadClasses(ar, m_systemClasses);
  BuiltinSymbols::LoadVariables(ar, m_variables);
  BuiltinSymbols::LoadConstants(ar, m_constants);
}

void AnalysisResult::checkClassDerivations() {
  AnalysisResultPtr ar = shared_from_this();
  ClassScopePtr cls;
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(cls, iter->second) {
      hphp_string_iset seen;
      cls->checkDerivation(ar, seen);
      if (Option::WholeProgram || !Option::OutputHHBC) {
        cls->importUsedTraits(ar);
      }
    }
  }
}

void AnalysisResult::collectFunctionsAndClasses(FileScopePtr fs) {
  const StringToFunctionScopePtrMap &funcs = fs->getFunctions();

  for (StringToFunctionScopePtrMap::const_iterator iter = funcs.begin();
       iter != funcs.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    if (!func->inPseudoMain()) {
      FunctionScopePtr &funcDec = m_functionDecs[iter->first];
      if (funcDec) {
        FunctionScopePtrVec &funcVec = m_functionReDecs[iter->first];
        int sz = funcVec.size();
        if (!sz) {
          funcDec->setRedeclaring(sz++);
          funcVec.push_back(funcDec);
        }
        func->setRedeclaring(sz++);
        funcVec.push_back(func);
      } else {
        funcDec = func;
      }
    }
  }

  if (const StringToFunctionScopePtrVecMap *redec = fs->getRedecFunctions()) {
    for (StringToFunctionScopePtrVecMap::const_iterator iter = redec->begin();
         iter != redec->end(); ++iter) {
      FunctionScopePtrVec::const_iterator i = iter->second.begin();
      FunctionScopePtrVec::const_iterator e = iter->second.end();
      FunctionScopePtr &funcDec = m_functionDecs[iter->first];
      ASSERT(funcDec); // because the first one was in funcs above
      FunctionScopePtrVec &funcVec = m_functionReDecs[iter->first];
      int sz = funcVec.size();
      if (!sz) {
        funcDec->setRedeclaring(sz++);
        funcVec.push_back(funcDec);
      }
      while (++i != e) { // we already added the first one
        (*i)->setRedeclaring(sz++);
        funcVec.push_back(*i);
      }
    }
  }

  const StringToClassScopePtrVecMap &classes = fs->getClasses();
  for (StringToClassScopePtrVecMap::const_iterator iter = classes.begin();
       iter != classes.end(); ++iter) {
    ClassScopePtrVec &clsVec = m_classDecs[iter->first];
    clsVec.insert(clsVec.end(), iter->second.begin(), iter->second.end());
  }
}

static bool by_filename(const FileScopePtr &f1, const FileScopePtr &f2) {
  return f1->getName() < f2->getName();
}

void AnalysisResult::analyzeProgram(bool system /* = false */) {
  AnalysisResultPtr ar = shared_from_this();

  if (system) m_system = true;
  getVariables()->forceVariants(ar, VariableTable::AnyVars);
  getVariables()->setAttribute(VariableTable::ContainsLDynamicVariable);
  getVariables()->setAttribute(VariableTable::ContainsExtract);
  getVariables()->setAttribute(VariableTable::ForceGlobal);

  // Analyze Includes
  Logger::Verbose("Analyzing Includes");
  sort(m_fileScopes.begin(), m_fileScopes.end(), by_filename); // fixed order
  unsigned int i = 0;
  for (i = 0; i < m_fileScopes.size(); i++) {
    collectFunctionsAndClasses(m_fileScopes[i]);
  }

  // Keep generated code identical without randomness
  canonicalizeSymbolOrder();

  // Analyze some special cases
  for (set<string>::const_iterator it = Option::VolatileClasses.begin();
       it != Option::VolatileClasses.end(); ++it) {
    ClassScopePtr cls = findClass(Util::toLower(*it));
    if (cls && cls->isUserClass()) {
      cls->setVolatile();
    }
  }

  checkClassDerivations();

  // Analyze All
  Logger::Verbose("Analyzing All");
  setPhase(AnalysisResult::AnalyzeAll);
  for (i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }

  /*
    Note that cls->collectMethods() can add entries to m_classDecs,
    which can invalidate iterators. So we have to create an array
    and then iterate over that.
    The new entries added to m_classDecs are always empty, so it
    doesnt matter that we dont include them in the iteration
  */
  ClassScopePtr cls;
  std::vector<ClassScopePtr> classes;
  classes.reserve(m_classDecs.size());
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(cls, iter->second) {
      classes.push_back(cls);
    }
  }

  // Collect methods
  BOOST_FOREACH(cls, classes) {
    if (cls->isRedeclaring()) {
      cls->setStaticDynamic(ar);
    }
    StringToFunctionScopePtrMap methods;
    cls->collectMethods(ar, methods);
    bool needAbstractMethodImpl =
      (!cls->isAbstract() && !cls->isInterface() &&
       !cls->derivesFromRedeclaring() &&
       !cls->getAttribute(ClassScope::UsesUnknownTrait));
    for (StringToFunctionScopePtrMap::const_iterator iterMethod =
           methods.begin(); iterMethod != methods.end(); ++iterMethod) {
      FunctionScopePtr func = iterMethod->second;
      if (!func->hasImpl() && needAbstractMethodImpl) {
        FunctionScopePtr tmpFunc =
          cls->findFunction(ar, func->getName(), true, true);
        always_assert(!tmpFunc || !tmpFunc->hasImpl());
        Compiler::Error(Compiler::MissingAbstractMethodImpl,
                        func->getStmt(), cls->getStmt());
      }
      m_methodToClassDecs[iterMethod->first].push_back(cls);
    }
  }

  string cname;
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

void AnalysisResult::analyzeIncludes() {
  AnalysisResultPtr ar = shared_from_this();
  for (unsigned i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeIncludes(ar);
  }
}

static void addClassRootMethods(AnalysisResultPtr ar, ClassScopePtr cls,
                                hphp_string_set &methods) {
  const StringToFunctionScopePtrMap &funcs = cls->getFunctions();
  for (StringToFunctionScopePtrMap::const_iterator iter =
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
  const StringToFunctionScopePtrMap &funcs = cls->getFunctions();
  for (StringToFunctionScopePtrMap::const_iterator iter =
         funcs.begin(); iter != funcs.end(); ++iter) {
    ClassScopePtr root = cls->getRootParent(ar, iter->first);
    string cluster = root->getName() + "::" + iter->first;
    FunctionScopePtrVec &fs = methods[cluster];
    fs.push_back(iter->second);
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

    if (!funcs[0]->isPrivate()) {
      bool perfect = true;
      for (unsigned int i = 1; i < funcs.size(); i++) {
        if (funcs[i]->isPrivate() || !funcs[0]->matchParams(funcs[i])) {
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
}

void AnalysisResult::analyzeProgramFinal() {
  AnalysisResultPtr ar = shared_from_this();
  setPhase(AnalysisResult::AnalyzeFinal);
  for (uint i = 0; i < m_fileScopes.size(); i++) {
    m_fileScopes[i]->analyzeProgram(ar);
  }
  for (StringToClassScopePtrVecMap::iterator iter = m_classDecs.begin(),
       end = m_classDecs.end(); iter != end; ++iter) {
    for (ClassScopePtrVec::iterator it = iter->second.begin(),
         e = iter->second.end(); it != e; ++it) {
      ClassScopePtr cls = *it;
      if (cls->isUserClass()) {
        ClassStatementPtr clsStmt =
          dynamic_pointer_cast<ClassStatement>(cls->getStmt());
        bool needsCppCtor = false;
        bool needsInit = false;
        if (clsStmt) clsStmt->getCtorAndInitInfo(needsCppCtor, needsInit);
        cls->setNeedsCppCtor(needsCppCtor);
        cls->setNeedsInitMethod(needsInit);
      }
    }
  }
  // Keep generated code identical without randomness
  canonicalizeSymbolOrder();
  setPhase(AnalysisResult::CodeGen);
}

static void dumpVisitor(AnalysisResultPtr ar, StatementPtr s, void *data) {
  s->dump(0, ar);
}

void AnalysisResult::dump() {
  visitFiles(dumpVisitor, 0);
  fflush(0);
}

void AnalysisResult::docJson(const string &filename) {
  ofstream f(filename.c_str());
  if (f.fail()) {
    Logger::Error("Could not open file for writing doc JSON: %s",
                  filename.c_str());
    return;
  }
  JSON::DocTarget::OutputStream out(f, shared_from_this());
  JSON::DocTarget::MapStream ms(out);

  ms.add("userland", m_fileScopes);

  ClassScopePtrVec systemClasses;
  systemClasses.reserve(m_systemClasses.size());
  for (StringToClassScopePtrMap::iterator it = m_systemClasses.begin();
       it != m_systemClasses.end(); ++it) {
    systemClasses.push_back(it->second);
  }
  // just generate system classes for now
  ms.add("system", systemClasses);

  ms.done();
  f.close();
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

void AnalysisResult::getScopesSet(BlockScopeRawPtrQueue &v) {
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    FileScopePtr file = iter->second;
    file->getScopesSet(v);
  }
}

///////////////////////////////////////////////////////////////////////////////
// optimization functions

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <typename When>
struct OptWorker;

template <typename When>
struct OptVisitor {
  typedef OptVisitor<When> Visitor;

  OptVisitor(AnalysisResultPtr ar, unsigned nscope) :
      m_ar(ar), m_nscope(nscope), m_dispatcher(0) {
  }
  OptVisitor(const Visitor &po) : m_ar(po.m_ar),
                                  m_nscope(po.m_nscope),
                                  m_dispatcher(po.m_dispatcher) {
    const_cast<Visitor&>(po).m_dispatcher = 0;
  }
  ~OptVisitor() {
    delete m_dispatcher;
  }

  void start() {
    m_dispatcher->start();
  }

  void wait() {
    m_dispatcher->waitEmpty(false);
  }

  void stop() {
    m_dispatcher->waitEmpty();
  }

  int getQueuedJobs() {
    return m_dispatcher->getQueuedJobs();
  }

  int getActiveWorker() {
    return m_dispatcher->getActiveWorker();
  }

  AnalysisResultPtr m_ar;
  unsigned m_nscope;
  JobQueueDispatcher<BlockScope *, OptWorker<When> > *m_dispatcher;
};

template <typename When>
class OptWorker : public JobQueueWorker<BlockScope *, true, true> {
public:
  OptWorker() {}

  virtual void doJob(BlockScope *scope) {
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
    atomic_inc(AnalysisResult::s_NumDoJobCalls);
    ConcurrentBlockScopeRawPtrIntHashMap::accessor acc;
    AnalysisResult::s_DoJobUniqueScopes.insert(acc,
      BlockScopeRawPtr(scope));
    acc->second += 1;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
    try {
      DepthFirstVisitor<When, OptVisitor > *visitor =
        (DepthFirstVisitor<When, OptVisitor >*)m_opaque;
      {
        Lock ldep(BlockScope::s_depsMutex);
        Lock lstate(BlockScope::s_jobStateMutex);
        always_assert(scope->getMark() == BlockScope::MarkReady);
        if (scope->getNumDepsToWaitFor()) {
          scope->setMark(BlockScope::MarkWaiting);
          return;
        }
        scope->setMark(BlockScope::MarkProcessing);
      }

      scope->setForceRerun(false);
      scope->setNeedsReschedule(false);

      // creates on demand
      AnalysisResult::s_changedScopesMapThreadLocal->clear();
      int useKinds = visitor->visitScope(BlockScopeRawPtr(scope));
      ASSERT(useKinds >= 0);

      {
        Lock l2(BlockScope::s_depsMutex);
        Lock l1(BlockScope::s_jobStateMutex);

        ASSERT(scope->getMark() == BlockScope::MarkProcessing);
        ASSERT(scope->getNumDepsToWaitFor() == 0);
        scope->assertNumDepsSanity();

        // re-enqueue changed scopes, regardless of rescheduling exception.
        // this is because we might have made changes to other scopes which we
        // do not undo, so we need to announce their updates
        BlockScopeRawPtrFlagsHashMap::const_iterator localIt =
          AnalysisResult::s_changedScopesMapThreadLocal->begin();
        BlockScopeRawPtrFlagsHashMap::const_iterator localEnd =
          AnalysisResult::s_changedScopesMapThreadLocal->end();
        for (; localIt != localEnd; ++localIt) {
          const BlockScopeRawPtrFlagsVec &ordered =
            localIt->first->getOrderedUsers();
          for (BlockScopeRawPtrFlagsVec::const_iterator userIt =
                 ordered.begin(), userEnd = ordered.end();
               userIt != userEnd; ++userIt) {
            BlockScopeRawPtrFlagsVec::value_type pf = *userIt;
            if ((pf->second & GetPhaseInterestMask<When>()) &&
                (pf->second & localIt->second)) {
              int m = pf->first->getMark();
              switch (m) {
              case BlockScope::MarkWaiting:
              case BlockScope::MarkReady:
                ; // no-op
                break;
              case BlockScope::MarkProcessing:
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
                atomic_inc(AnalysisResult::s_NumForceRerunGlobal);
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                pf->first->setForceRerun(true);
                break;
              case BlockScope::MarkProcessed:
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
                atomic_inc(AnalysisResult::s_NumReactivateGlobal);
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                if (visitor->activateScope(pf->first)) {
                  visitor->enqueue(pf->first);
                }
                break;
              default: ASSERT(false);
              }
            }
          }
        }
        AnalysisResult::s_changedScopesMapThreadLocal.destroy();

        if (scope->needsReschedule()) {
          // This signals an error in visitScope() which the scope can possibly
          // recover from if run again. an example is a lock contention error
          // (where the scope had to bail out). thus, we simply want to
          // re-enqueue it (w/o activating dependents, since this scope hasn't
          // actually finished running)
          scope->setRescheduleFlags(
              scope->rescheduleFlags() | useKinds);
          if (visitor->activateScope(BlockScopeRawPtr(scope))) {
            visitor->enqueue(BlockScopeRawPtr(scope));
          }
        } else {
          useKinds |= scope->rescheduleFlags();
          scope->setRescheduleFlags(0);

          const BlockScopeRawPtrFlagsVec &ordered = scope->getOrderedUsers();
          for (BlockScopeRawPtrFlagsVec::const_iterator it = ordered.begin(),
               end = ordered.end(); it != end; ++it) {
            BlockScopeRawPtrFlagsVec::value_type pf = *it;
            if (pf->second & GetPhaseInterestMask<When>()) {
              int m = pf->first->getMark();
              if (pf->second & useKinds && m == BlockScope::MarkProcessed) {
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
                atomic_inc(AnalysisResult::s_NumReactivateUseKinds);
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                bool ready = visitor->activateScope(pf->first);
                always_assert(!ready);
                m = BlockScope::MarkWaiting;
              }

              if (m == BlockScope::MarkWaiting || m == BlockScope::MarkReady) {
                int nd = pf->first->getNumDepsToWaitFor();
                always_assert(nd >= 1);
                if (!pf->first->decNumDepsToWaitFor() &&
                    m == BlockScope::MarkWaiting) {
                  pf->first->setMark(BlockScope::MarkReady);
                  visitor->enqueue(pf->first);
                }
              } else if (pf->second & useKinds &&
                         m == BlockScope::MarkProcessing) {
                // This is conservative: If we have a user who is currently
                // processing (yes, this can potentially happen if we add a
                // user *after* the initial dep graph has been formed), then we
                // have no guarantee that the scope read this scope's updates
                // in its entirety. Thus, we must force it to run again in
                // order to be able to observe all the updates.
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
                atomic_inc(AnalysisResult::s_NumForceRerunUseKinds);
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                always_assert(pf->first->getNumDepsToWaitFor() == 0);
                pf->first->setForceRerun(true);
              }
            }
          }
          scope->setMark(BlockScope::MarkProcessed);
          if (scope->forceRerun()) {
            if (visitor->activateScope(BlockScopeRawPtr(scope))) {
              visitor->enqueue(BlockScopeRawPtr(scope));
            }
          } else {
            const BlockScopeRawPtrFlagsPtrVec &deps = scope->getDeps();
            for (BlockScopeRawPtrFlagsPtrVec::const_iterator it = deps.begin(),
                 end = deps.end(); it != end; ++it) {
              const BlockScopeRawPtrFlagsPtrPair &p(*it);
              if (*p.second & GetPhaseInterestMask<When>()) {
                if (p.first->getMark() == BlockScope::MarkProcessing) {
                  bool ready = visitor->activateScope(BlockScopeRawPtr(scope));
                  always_assert(!ready);
                  break;
                }
              }
            }
          }
        }
      }
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
    }
  }
};

// Pre, InferTypes, and Post defined in depth_first_visitor.h

typedef   OptVisitor<Pre>          PreOptVisitor;
typedef   OptWorker<Pre>           PreOptWorker;
typedef   OptVisitor<InferTypes>   InferTypesVisitor;
typedef   OptWorker<InferTypes>    InferTypesWorker;
typedef   OptVisitor<Post>         PostOptVisitor;
typedef   OptWorker<Post>          PostOptWorker;

/**
 * Unfortunately we cannot template specialize on something like this w/o
 * complaints about incomplete class declarations:
 *
 *   template <class When>
 *   void DepthFirstVisitor<When, OptVisitor>::setup() { ... }
 *
 * And as such, this evil exists
 */

#define IMPLEMENT_OPT_VISITOR_SETUP(worker) \
  do { \
    unsigned int threadCount = Option::ParserThreadCount; \
    if (threadCount > this->m_data.m_nscope) { \
      threadCount = this->m_data.m_nscope; \
    } \
    if (threadCount <= 0) threadCount = 1; \
    this->m_data.m_dispatcher = \
      new JobQueueDispatcher<BlockScope *, worker >( \
        threadCount, true, 0, false, this); \
  } while (0)

#define IMPLEMENT_OPT_VISITOR_ENQUEUE(scope) \
  do { \
    ASSERT((scope)->getMark() == BlockScope::MarkReady); \
    this->m_data.m_dispatcher->enqueue((scope).get()); \
  } while (0)

template<>
void DepthFirstVisitor<Pre, OptVisitor>::setup() {
  IMPLEMENT_OPT_VISITOR_SETUP(PreOptWorker);
}

template<>
void DepthFirstVisitor<InferTypes, OptVisitor>::setup() {
  IMPLEMENT_OPT_VISITOR_SETUP(InferTypesWorker);
}

template<>
void DepthFirstVisitor<Post, OptVisitor>::setup() {
  IMPLEMENT_OPT_VISITOR_SETUP(PostOptWorker);
}

template<>
void DepthFirstVisitor<Pre, OptVisitor>::enqueue(BlockScopeRawPtr scope) {
  IMPLEMENT_OPT_VISITOR_ENQUEUE(scope);
}

template<>
void DepthFirstVisitor<InferTypes, OptVisitor>::enqueue(
  BlockScopeRawPtr scope) {
  IMPLEMENT_OPT_VISITOR_ENQUEUE(scope);
}

template<>
void DepthFirstVisitor<Post, OptVisitor>::enqueue(BlockScopeRawPtr scope) {
  IMPLEMENT_OPT_VISITOR_ENQUEUE(scope);
}

template <typename When>
void
AnalysisResult::preWaitCallback(bool first,
                                const BlockScopeRawPtrQueue &scopes,
                                void *opaque) {
  // default is no-op
}

template <typename When>
bool
AnalysisResult::postWaitCallback(bool first,
                                 bool again,
                                 const BlockScopeRawPtrQueue &scopes,
                                 void *opaque) {
  // default is no-op
  return again;
}

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
int AnalysisResult::s_NumDoJobCalls         = 0;
int AnalysisResult::s_NumForceRerunGlobal   = 0;
int AnalysisResult::s_NumReactivateGlobal   = 0;
int AnalysisResult::s_NumForceRerunUseKinds = 0;
int AnalysisResult::s_NumReactivateUseKinds = 0;

ConcurrentBlockScopeRawPtrIntHashMap
  AnalysisResult::s_DoJobUniqueScopes;

static inline int CountScopesWaiting(const BlockScopeRawPtrQueue &scopes) {
  int s = 0;
  for (BlockScopeRawPtrQueue::const_iterator it = scopes.begin();
       it != scopes.end(); ++it) {
    int m = (*it)->getMark();
    ASSERT(m == BlockScope::MarkWaiting ||
           m == BlockScope::MarkProcessed);
    if (m == BlockScope::MarkWaiting) s++;
  }
  return s;
}

static inline void DumpScope(BlockScopeRawPtr scope, const char *prefix,
                             bool newline = true) {
  ASSERT(scope->is(BlockScope::FunctionScope) ||
         scope->is(BlockScope::ClassScope));
  const char *type = scope->is(BlockScope::FunctionScope) ?
    "function" : "class";
  std::cout << prefix << type << " " << scope->getName() << " @ "
            << scope->getContainingFile()->getName();
  if (newline) std::cout << std::endl;
}

static inline void DumpScopeWithDeps(BlockScopeRawPtr scope) {
  ASSERT(scope->is(BlockScope::FunctionScope) ||
         scope->is(BlockScope::ClassScope));
  DumpScope(scope, "");
  const BlockScopeRawPtrFlagsVec &ordered = scope->getOrderedUsers();
  for (BlockScopeRawPtrFlagsVec::const_iterator it = ordered.begin(),
       end = ordered.end(); it != end; ++it) {
    BlockScopeRawPtrFlagsVec::value_type pf = *it;
    string prefix = "    ";
    prefix += "(";
    prefix += boost::lexical_cast<string>(pf->second);
    prefix += ") ";
    DumpScope(pf->first, prefix.c_str());
  }
}

typedef std::pair<BlockScopeRawPtr, int> BIPair;
struct BIPairCmp {
  inline bool operator()(const BIPair &lhs, const BIPair &rhs) const {
    return lhs.second > rhs.second;
  }
};
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

template <typename When>
void
AnalysisResult::processScopesParallel(const char *id,
                                      void *opaque /* = NULL */) {
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
  std::cout << "processScopesParallel(" << id << "): "
    << scopes.size() << " scopes" << std::endl;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

  DepthFirstVisitor<When, OptVisitor> dfv(
    OptVisitor<When>(shared_from_this(), scopes.size()));

  bool first = true;
  bool again;
  dfv.data().start();
  do {

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
    std::cout << "-----------------------------------" << std::endl;
    AnalysisResult::s_NumDoJobCalls         = 0;
    AnalysisResult::s_NumForceRerunGlobal   = 0;
    AnalysisResult::s_NumReactivateGlobal   = 0;
    AnalysisResult::s_NumForceRerunUseKinds = 0;
    AnalysisResult::s_NumReactivateUseKinds = 0;

    AnalysisResult::s_DoJobUniqueScopes.clear();
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

#ifdef HPHP_INSTRUMENT_TYPE_INF
    ASSERT(RescheduleException::s_NumReschedules          == 0);
    ASSERT(RescheduleException::s_NumForceRerunSelfCaller == 0);
    ASSERT(RescheduleException::s_NumRetTypesChanged      == 0);
    ASSERT(BaseTryLock::s_LockProfileMap.empty());
#endif /* HPHP_INSTRUMENT_TYPE_INF */

    BlockScopeRawPtrQueue enqueued;
    again = dfv.visitParallel(scopes, first, enqueued);
    preWaitCallback<When>(first, scopes, opaque);

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
    {
      std::cout << "Enqueued " << enqueued.size() <<
        " scopes in visitParallel()" << std::endl;
      if (enqueued.size() < 100) {
        for (BlockScopeRawPtrQueue::const_iterator it = enqueued.begin();
             it != enqueued.end(); ++it) {
          DumpScopeWithDeps(*it);
        }
      }
      Timer timer(Timer::WallTime, "dfv.wait()");
      dfv.data().wait();
    }
#else
    dfv.data().wait();
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

    ASSERT(!dfv.data().getQueuedJobs());
    ASSERT(!dfv.data().getActiveWorker());

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
    std::cout << "Number of doJob() calls: "
      << AnalysisResult::s_NumDoJobCalls << std::endl;
    std::cout << "Number of scopes which got doJob() called: "
      << AnalysisResult::s_DoJobUniqueScopes.size() << std::endl;
    std::vector<BIPair> v(
        AnalysisResult::s_DoJobUniqueScopes.begin(),
        AnalysisResult::s_DoJobUniqueScopes.end());
    if (!v.empty()) {
      sort(v.begin(), v.end(), BIPairCmp());
      std::vector<BIPair>::const_iterator end =
        v.size() > 20 ? v.begin() + 20 : v.end();
      for (std::vector<BIPair>::const_iterator it = v.begin();
          it != end; ++it) {
        string prefix;
        prefix += boost::lexical_cast<string>((*it).second);
        prefix += " times: ";
        DumpScope((*it).first, prefix.c_str());
      }
      std::cout << "Number of global force reruns: "
        << AnalysisResult::s_NumForceRerunGlobal << std::endl;
      std::cout << "Number of global reactivates: "
        << AnalysisResult::s_NumReactivateGlobal << std::endl;
      std::cout << "Number of use kind force reruns: "
        << AnalysisResult::s_NumForceRerunUseKinds << std::endl;
      std::cout << "Number of use kind reactivates: "
        << AnalysisResult::s_NumReactivateUseKinds << std::endl;
    }
    int numWaiting = CountScopesWaiting(scopes);
    std::cout << "Number of waiting scopes: " << numWaiting << std::endl;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

    again = postWaitCallback<When>(first, again, scopes, opaque);
    first = false;
  } while (again);
  dfv.data().stop();

  for (BlockScopeRawPtrQueue::iterator
       it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    ASSERT((*it)->getMark() == BlockScope::MarkProcessed);
    ASSERT((*it)->getNumDepsToWaitFor() == 0);
    ASSERT(!(*it)->needsReschedule());
    ASSERT((*it)->rescheduleFlags() == 0);
  }
}

///////////////////////////////////////////////////////////////////////////////
// pre-opt

template<>
int DepthFirstVisitor<Pre, OptVisitor>::visitScope(BlockScopeRawPtr scope) {
  int updates, all_updates = 0;
  StatementPtr stmt = scope->getStmt();
  if (MethodStatementPtr m =
      dynamic_pointer_cast<MethodStatement>(stmt)) {
    WriteLock lock(m->getFunctionScope()->getInlineMutex());
    do {
      scope->clearUpdated();
      if (Option::LocalCopyProp || Option::EliminateDeadCode) {
        AliasManager am(-1);
        if (am.optimize(this->m_data.m_ar, m)) {
          scope->addUpdates(BlockScope::UseKindCaller);
        }
      } else {
        StatementPtr rep = this->visitStmtRecur(stmt);
        always_assert(!rep);
      }
      updates = scope->getUpdated();
      all_updates |= updates;
    } while (updates);
    if (all_updates & BlockScope::UseKindCaller &&
        !m->getFunctionScope()->getInlineAsExpr()) {
      all_updates &= ~BlockScope::UseKindCaller;
    }
    return all_updates;
  }

  do {
    scope->clearUpdated();
    StatementPtr rep = this->visitStmtRecur(stmt);
    always_assert(!rep);
    updates = scope->getUpdated();
    all_updates |= updates;
  } while (updates);

  return all_updates;
}

template<>
ExpressionPtr DepthFirstVisitor<Pre, OptVisitor>::visit(ExpressionPtr e) {
  return e->preOptimize(this->m_data.m_ar);
}

template<>
StatementPtr DepthFirstVisitor<Pre, OptVisitor>::visit(StatementPtr stmt) {
  return stmt->preOptimize(this->m_data.m_ar);
}

void AnalysisResult::preOptimize() {
  setPhase(FirstPreOptimize);
  processScopesParallel<Pre>("PreOptimize");
}

///////////////////////////////////////////////////////////////////////////////
// infer types

template<>
int
DepthFirstVisitor<InferTypes, OptVisitor>::visitScope(BlockScopeRawPtr scope) {
  // acquire a lock on the scope
  SimpleLock lock(scope->getInferTypesMutex());

  // set the thread local to this scope-
  // use an object to do this so if an exception is thrown we can take
  // advantage of stack-unwinding
  //
  // NOTE: this *must* happen *after* the lock has been acquired, since there
  // is code which depends on this ordering
  SetCurrentScope sc(scope);

  StatementPtr stmt = scope->getStmt();
  MethodStatementPtr m =
    dynamic_pointer_cast<MethodStatement>(stmt);
  bool pushPrev = m && !scope->isFirstPass() &&
    !scope->getContainingFunction()->inPseudoMain();
  if (m) {
    if (pushPrev) scope->getVariables()->beginLocal();
    scope->getContainingFunction()->pushReturnType();
  }

  int ret = 0;
  try {
    bool done;
    do {
      scope->clearUpdated();
      if (m) {
        scope->getContainingFunction()->clearRetExprs();
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

      done = !scope->getUpdated();
      ret |= scope->getUpdated();
      scope->incPass();
    } while (!done);

    if (m) {
      bool changed = scope->getContainingFunction()->popReturnType();
      if (changed && scope->selfUser() & BlockScope::UseKindCallerReturn) {
        // for a recursive caller, we must let the scope run again, because
        // there are potentially AST nodes which are interested in the updated
        // return type
#ifdef HPHP_INSTRUMENT_TYPE_INF
        atomic_inc(RescheduleException::s_NumForceRerunSelfCaller);
#endif /* HPHP_INSTRUMENT_TYPE_INF */
        scope->setForceRerun(true);
      }
      if (pushPrev) {
        scope->getVariables()->endLocal();
        ret = 0; // since we really care about the updated flags *after*
                 // endLocal()
      }
      scope->getContainingFunction()->fixRetExprs();
      ret |= scope->getUpdated();
      scope->clearUpdated();
    }
  } catch (RescheduleException &e) {
    // potential deadlock detected- reschedule
    // this scope to run at a later time
#ifdef HPHP_INSTRUMENT_TYPE_INF
    atomic_inc(RescheduleException::s_NumReschedules);
#endif /* HPHP_INSTRUMENT_TYPE_INF */
    ret |= scope->getUpdated();
    if (m) {
      scope->getContainingFunction()->resetReturnType();
      if (pushPrev) {
        scope->getVariables()->resetLocal();
        ret = 0; // since we really care about the updated flags *after*
                 // resetLocal()
      }
      scope->getContainingFunction()->fixRetExprs();
      ret |= scope->getUpdated();
      scope->clearUpdated();
    }
    scope->setNeedsReschedule(true);
  }

  // inc regardless of reschedule exception or not, since these are the
  // semantics of run id
  scope->incRunId();

  return ret;
}

template<>
bool AnalysisResult::postWaitCallback<InferTypes>(
    bool first, bool again, const BlockScopeRawPtrQueue &scopes, void *opaque) {

#ifdef HPHP_INSTRUMENT_TYPE_INF
  std::cout << "Number of rescheduled: " <<
    RescheduleException::s_NumReschedules << std::endl;
  RescheduleException::s_NumReschedules = 0;

  std::cout << "Number of force rerun self callers: " <<
    RescheduleException::s_NumForceRerunSelfCaller << std::endl;
  RescheduleException::s_NumForceRerunSelfCaller = 0;

  std::cout << "Number of return types changed: " <<
    RescheduleException::s_NumRetTypesChanged << std::endl;
  RescheduleException::s_NumRetTypesChanged = 0;

  std::cout << "Lock contention: " << std::endl;
  for (LProfileMap::const_iterator it = BaseTryLock::s_LockProfileMap.begin();
       it != BaseTryLock::s_LockProfileMap.end(); ++it) {
    const LEntry &entry = it->first;
    int count = it->second;
    std::cout << "(" << entry.first << "@" << entry.second << "): " <<
      count << std::endl;
  }

  BaseTryLock::s_LockProfileMap.clear();
#endif /* HPHP_INSTRUMENT_TYPE_INF */

  return again;
}

#ifdef HPHP_INSTRUMENT_TYPE_INF
int RescheduleException::s_NumReschedules          = 0;
int RescheduleException::s_NumForceRerunSelfCaller = 0;
int RescheduleException::s_NumRetTypesChanged      = 0;
LProfileMap BaseTryLock::s_LockProfileMap;
#endif /* HPHP_INSTRUMENT_TYPE_INF */

void AnalysisResult::inferTypes() {
  if (isSystem()) {
    forceClassVariants(ClassScopePtr(), true);
    forceClassVariants(ClassScopePtr(), false);
  }

  setPhase(FirstInference);
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);

  for (BlockScopeRawPtrQueue::iterator
       it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    (*it)->setInTypeInference(true);
    (*it)->clearUpdated();
    ASSERT((*it)->getNumDepsToWaitFor() == 0);
  }

#ifdef HPHP_INSTRUMENT_TYPE_INF
  ASSERT(RescheduleException::s_NumReschedules          == 0);
  ASSERT(RescheduleException::s_NumForceRerunSelfCaller == 0);
  ASSERT(BaseTryLock::s_LockProfileMap.empty());
#endif /* HPHP_INSTRUMENT_TYPE_INF */

  processScopesParallel<InferTypes>("InferTypes");

  for (BlockScopeRawPtrQueue::iterator
       it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    (*it)->setInTypeInference(false);
    (*it)->clearUpdated();
    ASSERT((*it)->getMark() == BlockScope::MarkProcessed);
    ASSERT((*it)->getNumDepsToWaitFor() == 0);
  }
}

///////////////////////////////////////////////////////////////////////////////
// post-opt

template<>
int DepthFirstVisitor<Post, OptVisitor>::visit(BlockScopeRawPtr scope) {
  scope->clearUpdated();
  StatementPtr stmt = scope->getStmt();
  bool done = false;
  if (MethodStatementPtr m =
      dynamic_pointer_cast<MethodStatement>(stmt)) {

    AliasManager am(1);
    if (am.optimize(this->m_data.m_ar, m)) {
      scope->addUpdates(BlockScope::UseKindCaller);
    }
    if (Option::LocalCopyProp || Option::EliminateDeadCode) {
      done = true;
    }
  }

  if (!done) {
    StatementPtr rep = this->visitStmtRecur(stmt);
    always_assert(!rep);
  }

  return scope->getUpdated();
}

template<>
ExpressionPtr DepthFirstVisitor<Post, OptVisitor>::visit(ExpressionPtr e) {
  return e->postOptimize(this->m_data.m_ar);
}

template<>
StatementPtr DepthFirstVisitor<Post, OptVisitor>::visit(StatementPtr stmt) {
  return stmt->postOptimize(this->m_data.m_ar);
}

class FinalWorker : public JobQueueWorker<MethodStatementPtr> {
public:
  virtual void doJob(MethodStatementPtr m) {
    try {
      AliasManager am(1);
      am.finalSetup(((AnalysisResult*)m_opaque)->shared_from_this(), m);
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
    }
  }
};

template<>
void AnalysisResult::preWaitCallback<Post>(
    bool first, const BlockScopeRawPtrQueue &scopes, void *opaque) {
  ASSERT(!Option::ControlFlow || opaque != NULL);
  if (first && Option::ControlFlow) {
    JobQueueDispatcher<FinalWorker::JobType, FinalWorker> *dispatcher
      = (JobQueueDispatcher<FinalWorker::JobType, FinalWorker> *) opaque;
    for (BlockScopeRawPtrQueue::const_iterator it = scopes.begin(),
           end = scopes.end(); it != end; ++it) {
      BlockScopeRawPtr scope = *it;
      if (MethodStatementPtr m =
          dynamic_pointer_cast<MethodStatement>(scope->getStmt())) {
        dispatcher->enqueue(m);
      }
    }
  }
}

void AnalysisResult::postOptimize() {
  setPhase(AnalysisResult::PostOptimize);
  if (Option::ControlFlow) {
    BlockScopeRawPtrQueue scopes;
    getScopesSet(scopes);

    unsigned int threadCount = Option::ParserThreadCount;
    if (threadCount > scopes.size()) {
      threadCount = scopes.size();
    }
    if (threadCount <= 0) threadCount = 1;

    JobQueueDispatcher<FinalWorker::JobType, FinalWorker> dispatcher(
      threadCount, true, 0, false, this);

    processScopesParallel<Post>("PostOptimize", &dispatcher);

    dispatcher.start();
    dispatcher.stop();
  } else {
    processScopesParallel<Post>("PostOptimize");
  }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

///////////////////////////////////////////////////////////////////////////////
// code generation functions

int AnalysisResult::registerScalarArray(bool insideScalarArray,
                                        FileScopePtr scope,
                                        ExpressionPtr pairs, int &hash,
                                        int &index, string &text) {
  Lock lock(m_namedScalarArraysMutex);

  int id = -1;
  hash = -1;
  index = -1;
  bool found = false;
  if (!Option::ScalarArrayOptimization || insideScalarArray) {
    return -1;
  }

  if (pairs) {
    // Normal pickled PHP wouldn't work here, because predefined constants,
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
    hash = hash_string_cs(text.data(), text.size());
    vector<string> &strings = m_namedScalarArrays[hash];
    unsigned int i = 0;
    for (; i < strings.size(); i++) {
      if (strings[i] == text) break;
    }
    if (i == strings.size()) {
      always_assert(!found);
      strings.push_back(text);
    }
    index = i;
    scope->addUsedScalarArray(text);
  }
  return id;
}

int AnalysisResult::checkScalarArray(const string &text, int &index) {
  Lock lock(m_namedScalarArraysMutex);

  always_assert(Option::ScalarArrayOptimization && Option::UseNamedScalarArray);
  int hash = hash_string_cs(text.data(), text.size());
  vector<string> &strings = m_namedScalarArrays[hash];
  unsigned int i = 0;
  for (; i < strings.size(); i++) {
    if (strings[i] == text) break;
  }
  always_assert(i < strings.size());
  index = i;
  return hash;
}

int AnalysisResult::getScalarArrayId(const string &text) {
  Lock lock(m_namedScalarArraysMutex);

  std::map<std::string, int>::const_iterator iter = m_scalarArrays.find(text);
  always_assert(iter != m_scalarArrays.end());
  return iter->second;
}

void AnalysisResult::outputCPPNamedScalarArrays(const std::string &file) {
  AnalysisResultPtr ar = shared_from_this();

  string filename = file + ".no.cpp";
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  if (Option::SystemGen) {
    cg_printInclude("<system/gen/sys/scalar_arrays_remap.h>");
  } else {
    cg_printInclude("<sys/scalar_arrays_remap.h>");
  }
  if (Option::UseScalarVariant && !Option::SystemGen) {
    cg_printInclude("<sys/scalar_integers_remap.h>");
  }
  if (!Option::SystemGen) {
    cg_printInclude("<sys/global_variables.h>");
  }
  cg_printf("\n");
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarArrays.begin(); it != m_namedScalarArrays.end();
       it++) {
    int hash = it->first;
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarArrayName(hash, i);
      cg_printf("StaticArray %s;\n", name.c_str());
      if (m_namedScalarVarArrays.find(strings[i]) !=
          m_namedScalarVarArrays.end()) {
        cg_printf("VarNR %s;\n", getScalarVarArrayName(hash, i).c_str());
      }
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
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarArrayName(hash, i);
      int id = getScalarArrayId(strings[i]);
      cg_printf("%s = %s[%d];\n", name.c_str(), prefix, id);
      if (m_namedScalarVarArrays.find(strings[i]) !=
          m_namedScalarVarArrays.end()) {
        cg_printf("new (&%s) VarNR(%s);\n",
                  getScalarVarArrayName(hash, i).c_str(), name.c_str());
      }
    }
  }
  cg_indentEnd("}\n");
  cg.namespaceEnd();
}

void AnalysisResult::outputCPPNamedScalarVarIntegers(const std::string &file) {
  if (m_namedScalarVarIntegers.size() == 0) return;

  AnalysisResultPtr ar = shared_from_this();

  string filename = file + ".cpp";
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  if (Option::UseScalarVariant && !Option::SystemGen) {
    cg_printInclude("<sys/scalar_integers_remap.h>");
  }
  if (!Option::SystemGen) {
    cg_printInclude("<sys/global_variables.h>");
  }
  cg_printf("\n");
  cg.namespaceBegin();
  always_assert((sizeof(VarNR) % sizeof(int64) == 0));
  int multiple = (sizeof(VarNR) / sizeof(int64));
  cg_indentBegin("static const uint64 ivalues[] = {\n");
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarVarIntegers.begin(); it != m_namedScalarVarIntegers.end();
       it++) {
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      int64 val;
      sscanf(strings[i].c_str(), "%llx", &val);
      Variant v(val);
      int64 *startp = (int64 *)&v;
      int64 *endp = (startp + multiple);
      for (int64 *p = startp; p < endp; p++) {
        cg_printf("0x%016llxLL, ", *p);
      }
      cg_printf("// %lld\n", val);
    }
  }
  cg_indentEnd("};\n");
  int count = 0;
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarVarIntegers.begin(); it != m_namedScalarVarIntegers.end();
       it++) {
    int hash = it->first;
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarVarIntegerName(hash, i);
      cg_printf("const VarNR &%s = *(const VarNR*)(ivalues + %d);\n",
                name.c_str(), count * multiple);
      count++;
    }
  }
  cg_printf("\n");
  cg.namespaceEnd();
}

void AnalysisResult::outputCPPFiniteDouble(CodeGenerator &cg, double dval) {
  ASSERT(finite(dval));
  char *buf = NULL;
  if (dval == 0.0) dval = 0.0; // so to avoid "-0" output
  // 17 to ensure lossless conversion
  vspprintf(&buf, 0, "%.*G", 17, dval);
  ASSERT(buf);
  cg_printf("%s", buf);
  if (round(dval) == dval && !strchr(buf, '.') && !strchr(buf, 'E')) {
    cg.printf(".0"); // for integer value, cg_printf would break 0.0 token
  }
  free(buf);
}

void AnalysisResult::outputCPPNamedScalarVarDoubles(const std::string &file) {
  if (m_namedScalarVarDoubles.size() == 0) return;

  AnalysisResultPtr ar = shared_from_this();

  string filename = file + ".cpp";
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  if (!Option::SystemGen) {
    cg_printInclude("<sys/global_variables.h>");
  }
  cg_printf("\n");
  cg.namespaceBegin();
  always_assert((sizeof(int64) == sizeof(double)));
  always_assert((sizeof(VarNR) % sizeof(double) == 0));
  int multiple = (sizeof(VarNR) / sizeof(double));
  cg_indentBegin("static const uint64 dvalues[] = {\n");
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarVarDoubles.begin(); it != m_namedScalarVarDoubles.end();
       it++) {
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      double val;
      sscanf(strings[i].c_str(), "%llx", (int64*)(&val));
      Variant v(val);
      int64 *startp = (int64 *)&v;
      int64 *endp = (startp + multiple);
      for (int64 *p = startp; p < endp; p++) {
        cg_printf("0x%016llxLL, ", *p);
      }
      cg_printf("// ");
      outputCPPFiniteDouble(cg, val);
      cg_printf("\n");
    }
  }
  cg_indentEnd("};\n");
  int count = 0;
  for (map<int, vector<string> >::const_iterator it =
       m_namedScalarVarDoubles.begin(); it != m_namedScalarVarDoubles.end();
       it++) {
    int hash = it->first;
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getScalarVarDoubleName(hash, i);
      cg_printf("const VarNR &%s = *(const VarNR*)(dvalues + %d);\n",
                name.c_str(), count * multiple);
      count++;
    }
  }
  cg_printf("\n");
  cg.namespaceEnd();
}

void AnalysisResult::addInteger(int64 n) {
  Lock lock(m_allIntegersMutex);
  m_allIntegers.insert(n);
}

int AnalysisResult::checkScalarVarInteger(int64 val, int &index) {
  Lock lock(m_namedScalarVarIntegersMutex);

  always_assert(Option::UseScalarVariant);
  int hash = hash_int64(val);
  vector<string> &integers = m_namedScalarVarIntegers[hash];
  unsigned int i = 0;
  string hexval = boost::str(boost::format("%016x") % val);
  for (; i < integers.size(); i++) {
    if (integers[i] == hexval) break;
  }
  bool notFound = (i == integers.size());
  if (notFound) integers.push_back(hexval);
  index = i;
  return hash;
}

string AnalysisResult::getScalarVarIntegerName(int hash, int index) {
  return getHashedName(hash, index, Option::StaticVarIntPrefix);
}

int AnalysisResult::checkScalarVarDouble(double dval, int &index) {
  Lock lock(m_namedScalarVarDoublesMutex);

  always_assert(Option::UseScalarVariant);
  int64 ival = *(int64*)(&dval);
  int hash = hash_int64(ival);
  vector<string> &integers = m_namedScalarVarDoubles[hash];
  unsigned int i = 0;
  string hexval = boost::str(boost::format("%016x") % ival);
  for (; i < integers.size(); i++) {
    if (integers[i] == hexval) break;
  }
  bool notFound = (i == integers.size());
  if (notFound) integers.push_back(hexval);
  index = i;
  return hash;
}

string AnalysisResult::getScalarVarDoubleName(int hash, int index) {
  return getHashedName(hash, index, Option::StaticVarDblPrefix);
}

string AnalysisResult::prepareFile(const char *root, const string &fileName,
                                   bool chop, bool stripPath /* = true */) {
  string fullPath = root;
  if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/') {
    fullPath += "/";
  }

  string file = fileName;
  if (stripPath) {
    size_t npos = file.rfind('/');
    if (npos != string::npos) {
      file = file.substr(npos + 1);
    }
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

void
AnalysisResult::forceClassVariants(
    ClassScopePtr curScope,
    bool doStatic,
    bool acquireLocks /* = false */) {
  if (curScope) {
    COND_TRY_LOCK(curScope, acquireLocks);
    curScope->getVariables()->forceVariants(
      shared_from_this(), VariableTable::GetVarClassMask(true, doStatic),
      false);
  }

  ConditionalLock lock(getMutex(), acquireLocks);
  if (m_classForcedVariants[doStatic]) {
    return;
  }

  AnalysisResultPtr ar = shared_from_this();
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      COND_TRY_LOCK(cls, acquireLocks);
      cls->getVariables()->forceVariants(
        ar, VariableTable::GetVarClassMask(false, doStatic), false);
    }
  }

  m_classForcedVariants[doStatic] = true;
}

void AnalysisResult::forceClassVariants(
    const std::string &name,
    ClassScopePtr curScope,
    bool doStatic,
    bool acquireLocks /* = false */) {
  if (curScope) {
    COND_TRY_LOCK(curScope, acquireLocks);
    curScope->getVariables()->forceVariant(
      shared_from_this(), name, VariableTable::GetVarClassMask(true, doStatic));
  }

  ConditionalLock lock(getMutex(), acquireLocks);
  if (m_classForcedVariants[doStatic]) {
    return;
  }

  AnalysisResultPtr ar = shared_from_this();
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      COND_TRY_LOCK(cls, acquireLocks);
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
    totalSize += getFileSize(f);
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
    int fileSize = getFileSize(f);
    if (fileSize > clusterSize) {
      largeFiles.push_back(f);
    } else {
      size += fileSize;
      if ((size / FUZZYNESS) > (clusterSize / FUZZYNESS)) {
        clusterName = Option::FormatClusterFile(++count);
        size = fileSize;
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
    fout << line << "\n";

    origLine++;
    newLine++;
    if (lineIter != linemap.end() && origLine == lineIter->first) {
      m_sourceInfos[foutName][newLine] = lineIter->second;
      ++lineIter;
    }

    current += line.length() + 1;
    if (line.find(CodeGenerator::HASH_INCLUDE) == 0) {
      includes.push_back(line);
    } else if (line == "/* preface starts */") {
      inPreface = true;
    } else if (line == "/* preface finishes */") {
      inPreface = false;
    } else if (inPreface) {
      preface.push_back(line);
    } else if (line == CodeGenerator::SPLITTER_MARKER) {
      // a possible cut point
      if (current > targetSize) {
        if (insideHPHP) {
          // namespace HPHP
          fout << "}" << "\n";
        }

        fout.close();
        snprintf(foutName, sizeof(foutName),
                 "%s-%d.cpp", base.c_str(), ++seq);
        fout.open(foutName);
        newLine = 0;
        current = 0;
        for (unsigned int j = 0; j < includes.size(); j++) {
          fout << includes[j] << "\n";
          newLine++;
        }
        if (insideHPHP) {
          fout << "namespace HPHP {\n";
          newLine++;
        }
        fout << "/* preface starts */\n";
        for (unsigned int j = 0; j < preface.size(); j++) {
          fout << preface[j] << "\n";
          newLine++;
        }
        fout << "/* preface finishes */\n";
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
    } else {
      Logger::Error("unable to stat %s", filenames[i].c_str());
    }
  }
  int64 averageSize = count > 1 ? (totalSize / count) : totalSize;
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

void AnalysisResult::renameStaticNames(map<int, vector<string> > &names,
                                       const char *file, const char *prefix) {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix + file;
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg.headerBegin(filename);

  for (map<int, vector<string> >::const_iterator it = names.begin();
       it != names.end();
       it++) {
    int hash = it->first;
    if (names[hash].size() > 1) {
      vector<string> &strings = names[hash];
      vector<string> sortedStrings = strings;
      unsigned int nstrings = strings.size();
      sort(sortedStrings.begin(), sortedStrings.end());
      for (unsigned int i = 0; i < strings.size(); i++) {
        string &s = strings[i];
        unsigned int j;
        for (j = 0; j < nstrings; j++) {
          if (sortedStrings[j] == s) break;
        }
        ASSERT(j < nstrings);
        // remap i to j
        int64 newHash = hash_string_cs(s.data(), s.size());
        string name = getHashedName(hash, i, prefix);
        string newName = getHashedName(newHash, j, prefix, true);
        cg_printf("#define %s %s\n", name.c_str(), newName.c_str());

        if (!strcmp(prefix, Option::StaticStringPrefix)) {
          if (m_namedVarStringLiterals.find(name) !=
              m_namedVarStringLiterals.end()) {
            name = getHashedName(hash, i, Option::StaticVarStrPrefix);
            newName = getHashedName(newHash, j,
                                    Option::StaticVarStrPrefix, true);
            cg_printf("#define %s %s\n", name.c_str(), newName.c_str());
          }
        }
      }
    }
  }
  cg.headerEnd(filename);
  f.close();
}

class OutputJob {
public:
  OutputJob(AnalysisResultPtr ar)
      : m_ar(ar) {
  }
  OutputJob(AnalysisResultPtr ar, CodeGenerator::Output output)
      : m_ar(ar), m_output(output) {
  }
  OutputJob(AnalysisResultPtr ar, const std::string &root,
            CodeGenerator::Output output, const std::string *compileDir)
      : m_ar(ar), m_root(root), m_output(output), m_compileDir(compileDir) {
  }

  virtual ~OutputJob() {}

  void output() {
    outputImpl();
  }

  virtual void outputImpl() = 0;

  std::string m_filename;

protected:
  AnalysisResultPtr m_ar;
  std::string m_root;
  CodeGenerator::Output m_output;
  const std::string *m_compileDir;
};

class OutputClusterJob : public OutputJob {
public:
  OutputClusterJob(AnalysisResultPtr ar, const std::string &root,
                   CodeGenerator::Output output, const std::string *compileDir,
                   const std::string &name, const FileScopePtrVec &files)
      : OutputJob(ar, root, output, compileDir),
        m_name(name), m_files(files) {
    Util::mkdir(m_root + m_name);
    BOOST_FOREACH(FileScopePtr fs, m_files) {
      string fileBase = fs->outputFilebase();
      Util::mkdir(m_root + fileBase);
    }
  }

  virtual void outputImpl() {
    // for each cluster, generate one implementation file
    string filename = m_root + m_name + ".cpp";
    m_filename = filename;
    ofstream f(filename.c_str());
    if (m_compileDir) {
      // this is the file that will be compiled, so we need to use this
      // for source info:
      filename = *m_compileDir + "/" + m_name + ".cpp";
    }
    CodeGenerator cg(&f, m_output, &filename);
    m_ar->outputCPPClusterImpl(cg, m_files);

    // for each file, generate one header and a list of class headers
    BOOST_FOREACH(FileScopePtr fs, m_files) {
      string fileBase = fs->outputFilebase();
      string header = fileBase + ".h";
      string fwsheader = fileBase + ".fws.h";
      string fileHeader = m_root + header;
      string fwsFileHeader = m_root + fwsheader;
      {
        ofstream f(fileHeader.c_str());
        CodeGenerator cg(&f, m_output);
        fs->outputCPPDeclHeader(cg, m_ar);
        f.close();
      }
      fs->outputCPPClassHeaders(m_ar, m_output);
      {
        ofstream f(fwsFileHeader.c_str());
        CodeGenerator cg(&f, m_output);
        fs->outputCPPForwardStaticDecl(cg, m_ar);
        f.close();
      }
    }
  }

protected:
  const std::string &m_name;
  const FileScopePtrVec &m_files;
};

class PreGenerateCPPJob : public OutputJob {
public:
  PreGenerateCPPJob(AnalysisResultPtr ar, CodeGenerator::Output output,
                    FileScopePtr file)
      : OutputJob(ar, output), m_file(file) {
  }

  virtual void outputImpl() {
    ostringstream out;
    CodeGenerator cg(&out, m_output, &m_file->getName());
    m_ar->outputCPPFileImpl(cg, m_file);
    string code = out.str();
    m_ar->addPregeneratedCPP(m_file->getName(), code);
  }

private:
  FileScopePtr m_file;
};

#define DECLARE_JOB(name, body)                                     \
class name ## Job : public OutputJob {                              \
public:                                                             \
  name ## Job(AnalysisResultPtr ar, CodeGenerator::Output output)   \
    : OutputJob(ar, output) {                                       \
  }                                                                 \
  virtual void outputImpl() {                                       \
    m_ar->body;                                                     \
  }                                                                 \
};                                                                  \

#define SCHEDULE_JOB(name)                                          \
  {                                                                 \
    OutputJob *job = new name ## Job(ar, output);                   \
    jobs.push_back(job);                                            \
    dispatcher.enqueue(job);                                        \
  }                                                                 \

DECLARE_JOB(DynamicTable, outputCPPDynamicTables(m_output));
DECLARE_JOB(System,       outputCPPSystem());
DECLARE_JOB(SepExtMake,   outputCPPSepExtensionMake());
DECLARE_JOB(ClassMap,     outputCPPClassMapFile(m_output));
DECLARE_JOB(NameMaps,     outputCPPNameMaps());
DECLARE_JOB(SourceInfos,  outputCPPSourceInfos());
DECLARE_JOB(RTTIMeta,     outputRTTIMetaData(Option::RTTIOutputFile.c_str()));
DECLARE_JOB(UtilDecl,     outputCPPUtilDecl(m_output));
DECLARE_JOB(UtilImpl,     outputCPPUtilImpl(m_output));
DECLARE_JOB(GlobalDecl,   outputCPPGlobalDeclarations());
DECLARE_JOB(Main,         outputCPPMain());
DECLARE_JOB(ScalarArrays, outputCPPScalarArrays(false));
DECLARE_JOB(GlobalVarMeth,outputCPPGlobalVariablesMethods());
DECLARE_JOB(GlobalState,  outputCPPGlobalState());

class RepartitionJob : public OutputJob {
public:
  RepartitionJob(AnalysisResultPtr ar, const vector<string> &filenames,
                 const vector<string> &additionalCPPs)
      : OutputJob(ar), m_filenames(filenames),
        m_additionalCPPs(additionalCPPs) {
  }

  virtual void outputImpl() {
    m_ar->repartitionLargeCPP(m_filenames, m_additionalCPPs);
  }

private:
  const vector<string> &m_filenames;
  const vector<string> &m_additionalCPPs;
};

class OutputWorker : public JobQueueWorker<OutputJob*> {
public:
  virtual void doJob(OutputJob *job) { job->output();}
};

///////////////////////////////////////////////////////////////////////////////

void AnalysisResult::outputAllCPP(CodeGenerator::Output output,
                                  int clusterCount,
                                  const std::string *compileDir) {
  AnalysisResultPtr ar = shared_from_this();

  if (output == CodeGenerator::SystemCPP) {
    Option::GenerateCPPMain = false;
  }
  if (Option::GenerateCPPMacros && output != CodeGenerator::SystemCPP) {
    // system functions are currently unchanged
    createGlobalFuncTable();
  }

  unsigned int threadCount = Option::ParserThreadCount * 2;
  if (threadCount > m_fileScopes.size()) {
    threadCount = m_fileScopes.size();
  }
  if (threadCount <= 0) threadCount = 1;

  if (Option::SystemGen) {
    threadCount = 1;
  }

  StringToFileScopePtrVecMap clusters;
  if (clusterCount > 0) {
    if (Option::PregenerateCPP) {
      preGenerateCPP(output, m_fileScopes, threadCount);
      m_pregenerated = true;
    }
    clusterByFileSizes(clusters, clusterCount);
  } else {
    BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
      clusters[f->outputFilebase()].push_back(f);
    }
  }

  if (threadCount > clusters.size()) {
    threadCount = clusters.size();
  }

  // 1st round doing cpp/*.cpp
  vector<string> filenames;
  vector<string> additionalCPPs;
  {
    vector<OutputJob*> jobs;
    JobQueueDispatcher<OutputJob*, OutputWorker>
      dispatcher(threadCount, true, 0, false, NULL);

    string root = getOutputPath() + "/";
    for (StringToFileScopePtrVecMap::const_iterator iter = clusters.begin();
         iter != clusters.end(); ++iter) {
      OutputClusterJob *job =
        new OutputClusterJob(ar, root, output, compileDir,
                             iter->first, iter->second);
      jobs.push_back(job);
      dispatcher.enqueue(job);
    }
    dispatcher.start();

    // main thread
    if (Option::GenerateFFI) {
      outputFFI(additionalCPPs);
    }

    dispatcher.stop();

    for (unsigned int i = 0; i < jobs.size(); i++) {
      if (!jobs[i]->m_filename.empty()) {
        filenames.push_back(jobs[i]->m_filename);
      }
      delete jobs[i];
    }
    jobs.clear();
  }

  // 2nd round code generation
  {
    vector<OutputJob*> jobs;
    JobQueueDispatcher<OutputJob*, OutputWorker>
      dispatcher(threadCount, true, 0, false, NULL);

    if (clusterCount > 0) {
      OutputJob *job = new RepartitionJob(ar, filenames, additionalCPPs);
      jobs.push_back(job);
      dispatcher.enqueue(job);
    }

    if (Option::GenerateCPPMacros) {
      SCHEDULE_JOB(DynamicTable);
    }
    if (output == CodeGenerator::SystemCPP) {
      SCHEDULE_JOB(System);
    } else if (Option::GenerateCPPMain) {
      SCHEDULE_JOB(SepExtMake);
    }
    SCHEDULE_JOB(ClassMap); // TODO(#1960978): eventually don't need this
    if (Option::GenerateCPPMacros && output != CodeGenerator::SystemCPP) {
      SCHEDULE_JOB(NameMaps);
      SCHEDULE_JOB(SourceInfos);
    }
    if (Option::GenRTTIProfileData) {
      SCHEDULE_JOB(RTTIMeta);
    }

    if (!Option::SystemGen) {
      SCHEDULE_JOB(UtilDecl);
      SCHEDULE_JOB(UtilImpl);
    }
    getVariables()->canonicalizeStaticGlobals();
    if (output != CodeGenerator::SystemCPP && Option::GenerateCPPMain) {
      SCHEDULE_JOB(GlobalDecl);
      SCHEDULE_JOB(Main);
      SCHEDULE_JOB(ScalarArrays);
      SCHEDULE_JOB(GlobalVarMeth);
      SCHEDULE_JOB(GlobalState);
    }
    dispatcher.run();

    for (unsigned int i = 0; i < jobs.size(); i++) {
      delete jobs[i];
    }
  }

  // 3rd round code generation
  renameStaticNames(m_namedStringLiterals, "literal_strings_remap.h",
                    Option::StaticStringProxyPrefix);
  renameStaticNames(m_namedScalarArrays, "scalar_arrays_remap.h",
                    Option::StaticArrayPrefix);
  if (Option::UseScalarVariant && !Option::SystemGen) {
    renameStaticNames(m_namedScalarVarIntegers, "scalar_integers_remap.h",
                      Option::StaticVarIntPrefix);
  }

  {
    string file = m_outputPath + "/" + Option::SystemFilePrefix +
      "literal_strings";
    outputCPPNamedLiteralStrings(false, file);
    if (Option::UseNamedScalarArray) {
      string file = m_outputPath + "/" + Option::SystemFilePrefix +
        "scalar_arrays";
      outputCPPNamedScalarArrays(file);
    }
    if (Option::UseScalarVariant) {
      string file = m_outputPath + "/" + Option::SystemFilePrefix +
        "scalar_integers";
      outputCPPNamedScalarVarIntegers(file);
      file = m_outputPath + "/" + Option::SystemFilePrefix +
        "scalar_doubles";
      outputCPPNamedScalarVarDoubles(file);
    }
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

  ClassScope::outputCPPHashTableClasses(cg, merged, classes);
  ClassScope::outputCPPClassVarInitImpl(cg, merged, classes);
  ClassScope::outputCPPDynamicClassCreateImpl(cg, merged, classes);
  ClassScope::outputCPPGetCallInfoStaticMethodImpl(cg, merged, classes);
  ClassScope::outputCPPGetStaticPropertyImpl(cg, merged, classes);
  ClassScope::outputCPPGetClassConstantImpl(cg, merged);
  ClassScope::outputCPPGetClassPropTableImpl(cg, ar,
    ar->getExtensionClasses(), true);
}

void AnalysisResult::outputCPPClassMapFile(CodeGenerator::Output output) {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "class_map.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  if (output != CodeGenerator::SystemCPP) {
    cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
    if (Option::GenArrayCreate) {
      cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
    }
  }
  cg_printf("\n");

  cg.printImplStarter();
  cg.namespaceBegin();
  outputCPPClassMap(cg, output);
  cg.namespaceEnd();
  f.close();
}

void AnalysisResult::recordSourceInfo(const std::string &file, int line,
                                      LocationPtr loc) {
  // With FrameInjection, there is normally no need to generate the source info
  // map, so to save memory.
  if (Option::GenerateSourceInfo) {
    Lock lock(m_sourceInfoMutex);
    SourceInfo &target = m_pregenerating ? m_sourceInfoPregen : m_sourceInfos;
    // we only need one to one mapping, and there doesn't seem to be a need
    // to display multiple PHP file locations for one C++ frame
    target[file][line] = loc;
  }
}

void AnalysisResult::recordClassSource(const std::string &clsname,
                                       LocationPtr loc,
                                       const std::string &filename) {
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
                                          const std::string &filename) {
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
      cg_printf("\"%s\", \"%s\", (const char *)%d,\n",
                CodeGenerator::EscapeLabel(iter->first).c_str(),
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
      cg_printf("\"%s\", \"%s\", (const char *)%d,\n",
                CodeGenerator::EscapeLabel(iter->first).c_str(),
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
  if (!Option::GenArrayCreate) return;

  string filename = string(Option::SystemFilePrefix) + "cpputil.h";
  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream f(headerPath.c_str());
  CodeGenerator cg(&f, output);
  cg_printf("\n");
  cg.headerBegin(filename);
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();
  if (Option::GenArrayCreate) {
    outputArrayCreateDecl(cg);
  }
  cg.namespaceEnd();
  cg.headerEnd(filename);
}

void AnalysisResult::outputCPPUtilImpl(CodeGenerator::Output output) {
  if (!Option::GenArrayCreate) return;

  string filename = string(Option::SystemFilePrefix) + "cpputil.cpp";
  string headerPath = m_outputPath + "/" + filename;
  Util::mkdir(headerPath);
  ofstream f(headerPath.c_str());
  CodeGenerator cg(&f, output);
  cg_printf("\n");
  if (Option::GenArrayCreate) {
    cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  }
  cg_printInclude("<runtime/base/array/zend_array.h>");
  cg_printInclude("<runtime/base/array/vector_array.h>");
  cg_printInclude("<runtime/base/taint/taint_observer.h>");
  cg_printInclude("<runtime/base/taint/taint_data.h>");
  cg.printImplStarter();
  cg.namespaceBegin();
  if (Option::GenArrayCreate) {
    outputArrayCreateImpl(cg);
  }
  cg_indentBegin("\nstatic const int64 pre_converted_integers[] = {\n");
  for (set<int64>::const_iterator it = m_allIntegers.begin();
       it != m_allIntegers.end(); it++) {
    if (!String::HasConverted(*(it))) {
      if (*(it) == LONG_MIN) {
        cg_printf("(int64)0x%llxLL,\n", (uint64)LONG_MIN);
      } else {
        cg_printf("%lldLL,\n", *(it));
      }
    }
  }
  cg_indentEnd("};\n");

  const char text[] =
    "static int precompute_integers() {\n"
    "  for (unsigned int i = 0;\n"
    "       i < sizeof(pre_converted_integers)/sizeof(int64); i++) {\n"
    "    String::PreConvertInteger(pre_converted_integers[i]);\n"
    "  }\n"
    "  return 0;\n"
    "}\n"
    "\n"
    "static int ATTRIBUTE_UNUSED initIntegers = precompute_integers();\n";
  cg_print(text);

  cg.namespaceEnd();
}

void AnalysisResult::outputArrayCreateDecl(CodeGenerator &cg) {
  if (m_arrayLitstrKeyMaxSize > 0) {
    cg_printf("ArrayData *array_createvs(int64 n, ...);\n");
  }
  if (m_arrayIntegerKeyMaxSize > 0) {
    cg_printf("ArrayData *array_createvi(int64 n, ...);\n");
  }
}

void AnalysisResult::outputArrayCreateImpl(CodeGenerator &cg) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "HOT_FUNC\n"
    "ArrayData *array_createvs(int64 n, ...) {\n"
    "  va_list ap;\n"
    "  va_start(ap, n);\n"
    "  ZendArray::Bucket *p[%d], **pp = p;\n"
    "  SmartAllocator<HPHP::ZendArray::Bucket, SmartAllocatorImpl::Bucket> *a =\n"
    "      ZendArray::Bucket::AllocatorType::getNoCheck();\n"
    "  for (int64 kk = 0; kk < n; kk++) {\n"
    "    const String *k = va_arg(ap, const String *);\n"
    "    const Variant *v = va_arg(ap, const Variant *);\n"
    "    *pp++ = new (a) ZendArray::Bucket(k->get(), *v);\n"
    "  }\n"
    "  *pp = NULL;\n"
    "  va_end(ap);\n"
    "  return NEW(ZendArray)(n, 0, p);\n"
    "}\n";
  const char text2[] =
    "HOT_FUNC\n"
    "ArrayData *array_createvi(int64 n, ...) {\n"
    "  va_list ap;\n"
    "  va_start(ap, n);\n"
    "  if (enable_vector_array && RuntimeOption::UseVectorArray) {\n"
    "    const Variant *p[%d], **pp = p;\n"
    "    for (int64 k = 0; k < n; k++) {\n"
    "      *pp++ = va_arg(ap, const Variant *);\n"
    "    }\n"
    "    va_end(ap);\n"
    "    return NEW(VectorArray)(n, p);\n"
    "  }\n"
    "  ZendArray::Bucket *p[%d], **pp = p;\n"
    "  SmartAllocator<HPHP::ZendArray::Bucket, SmartAllocatorImpl::Bucket> *a =\n"
    "      ZendArray::Bucket::AllocatorType::getNoCheck();\n"
    "  for (int64 k = 0; k < n; k++) {\n"
    "    const Variant *v = va_arg(ap, const Variant *);\n"
    "    *pp++ = new (a) ZendArray::Bucket(k, *v);\n"
    "  }\n"
    "  *pp = NULL;\n"
    "  va_end(ap);\n"
    "  return NEW(ZendArray)(n, n, p);\n"
    "}\n";
  if (m_arrayLitstrKeyMaxSize > 0) {
    cg_printf(text1,
              m_arrayLitstrKeyMaxSize + 1);
  }
  if (m_arrayIntegerKeyMaxSize > 0) {
    cg_printf(text2,
              m_arrayIntegerKeyMaxSize,
              m_arrayIntegerKeyMaxSize + 1);
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
    cg.namespaceBegin();
  }
}

void AnalysisResult::createGlobalFuncTable() {
  vector<const char *> funcs;
  for (StringToFunctionScopePtrMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    if (func->isDynamic() || func->isRedeclaring()) {
      funcs.push_back(iter->second->getOriginalName().c_str());
    }
  }
  for (StringToFunctionScopePtrMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    if (func->isSepExtension()) {
      funcs.push_back(func->getOriginalName().c_str());
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
  FileScopePtr fs = findFileScope(file);
  cg_printf("if (s.empty()) return ");
  if (hhvm) {
    cg_printf("vm_default_invoke_file(once);\n");
  } else {
    if (fs->canUseDummyPseudoMain(shared_from_this())) {
      cg_printf("dummy_pm(once, variables, get_globals());\n");
    } else {
      cg_printf("%s%s(once, variables, get_globals());\n",
                Option::PseudoMainPrefix,
                Option::MangleFilename(file, true).c_str());
    }
  }
}

void AnalysisResult::outputCPPHashTableInvokeFile(
  CodeGenerator &cg, const vector<const char*> &entries, bool needEvalHook) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "static Variant dummy_pm(bool oncOnce, LVariableTable* variables, "
    "  Globals *globals) { return true; }\n"
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
    "static hashNodeFile fileBuckets[%zd];\n"
    "\n"
    "static class FileTableInitializer {\n"
    "  public: FileTableInitializer() {\n"
    "    const char *fileMapData[] = {\n";

  const char text2[] =
    "      NULL, NULL, (const char *)&dummy_pm,\n"
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
    "pm_t ptr = findFile(s.c_str(), s->hash());\n"
    "if (ptr) return ptr(once, variables, get_globals());\n";

  const char text4[] =
  "  return throw_missing_file(s.c_str());\n"
  "}\n";

  int tableSize = Util::roundUpToPowerOfTwo(entries.size() * 2);
  cg_printf(text1, tableSize, entries.size());
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    if (!f->getPseudoMain()) continue;
//    if (f->isPrivateInclude()) continue;
    cg_printf("      (const char *)\"%s\", (const char *)&",
              f->getName().c_str());
    if (f->canUseDummyPseudoMain(shared_from_this())) {
      cg_printf("dummy_pm,\n");
    } else {
      cg_printf("%s%s,\n",
                Option::PseudoMainPrefix,
                Option::MangleFilename(f->getName(), true).c_str());
    }
  }
  cg_printf(text2, tableSize - 1, tableSize - 1);
  outputCPPInvokeFileHeader(cg);
  cg_print(text3);
  if (needEvalHook) outputCPPEvalHook(cg);
  if (entries.size() == 1) outputCPPDefaultInvokeFile(cg, entries[0]);
  cg_indentEnd();
  cg_print(text4);

  cg_printf("bool hphp_could_invoke_file(CStrRef s, void*) {\n"
            "  return findFile(s.c_str(), s->hash());\n"
            "}\n");
}

void AnalysisResult::outputCPPDynamicClassTables(
  CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  bool system = output == CodeGenerator::SystemCPP;
  string n;
  string tablePath =
    m_outputPath + "/" + Option::SystemFilePrefix + "dynamic_table_class.cpp";

  Util::mkdir(tablePath);
  ofstream fTable(tablePath.c_str());
  CodeGenerator cg(&fTable, output);

  outputCPPDynamicTablesHeader(cg, true, false);
  cg.printSection("Class Invoke Tables");
  vector<const char*> classes;
  ClassScopePtr cls;
  StringToClassScopePtrVecMap classScopes;
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    if (iter->second.size()) {
      for (ClassScopePtrVec::const_iterator iter2 = iter->second.begin();
           iter2 != iter->second.end(); ++iter2) {
        cls = *iter2;
        if (cls->isUserClass() &&
            (!cls->isInterface() || cls->checkHasPropTable(ar))) {
          classes.push_back(cls->getOriginalName().c_str());
          classScopes[cls->getName()].push_back(cls);
          if (!cls->isRedeclaring()) {
            cls->outputCPPGlobalTableWrappersDecl(cg, ar);
          }
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
    ClassScope::outputCPPHashTableClasses(cg, classScopes, classes);
    ClassScope::outputCPPClassVarInitImpl(cg, classScopes, classes);
    ClassScope::outputCPPDynamicClassCreateImpl(cg, classScopes, classes);
    ClassScope::outputCPPGetCallInfoStaticMethodImpl(cg, classScopes, classes);
    ClassScope::outputCPPGetStaticPropertyImpl(cg, classScopes, classes);
    ClassScope::outputCPPGetClassConstantImpl(cg, classScopes);
  }
  cg.namespaceEnd();
  fTable.close();
}

void AnalysisResult::outputCPPHashTableGetConstant(
  CodeGenerator &cg,
  bool system,
  const map<string, TypePtr> &constMap,
  const hphp_string_map<bool> &dyns) {
  ASSERT(constMap.size() > 0);
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "class hashNodeCon {\n"
    "public:\n"
    "  hashNodeCon() {}\n"
    "  hashNodeCon(int64 h, const char *n, int64 o, int64 t,\n"
    "              int64 *p) :\n"
    "    hash(h), name(n), off(o), type(t), next(NULL) {\n"
    "    if (off > 0) return;\n"
    "    switch (t) {\n"
    "    case %d: case %d: case %d: case %d: case %d: case %d: case %d:\n"
    "      value = p;\n"
    "      break;\n"
    "    default: not_reached();\n"
    "    }\n"
    "  }\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  int64 off;\n"
    "  int64 type;\n"
    "  int64 *value;\n"
    "  hashNodeCon *next;\n"
    "};\n"
    "static hashNodeCon *conMapTable[%d];\n"
    "static hashNodeCon conBuckets[%zd];\n"
    "\n"
    "void init_%sconstant_table() {\n%s"
    "  const char *conMapData[] = {\n";

  const char text2[] =
    "    NULL, NULL, NULL, NULL\n"
    "  };\n"
    "  hashNodeCon *b = conBuckets;\n"
    "  for (const char **s = conMapData; *s; s++, b++) {\n"
    "    const char *name = *s++;\n"
    "    int64 off = (int64)(*s++);\n"
    "    int64 type = (int64)(*s++);\n"
    "    int64 *p = (int64*)(*s);\n"
    "    int64 hash = hash_string(name, strlen(name));\n"
    "    hashNodeCon *node =\n"
    "      new(b) hashNodeCon(hash, name, off, type, p);\n"
    "    int h = hash & %d;\n"
    "    if (conMapTable[h]) node->next = conMapTable[h];\n"
    "    conMapTable[h] = node;\n"
    "  }\n"
    "}\n"
    "\n"
    "static const hashNodeCon *\n"
    "findCon(const char *name, int64 hash) {\n"
    "  for (const hashNodeCon *p = conMapTable[hash & %d]; p; p = p->next) {\n"
    "    if (p->hash == hash && !strcmp(p->name, name)) return p;\n"
    "  }\n"
    "  return NULL;\n"
    "}\n";

  const char text3[] =
    "ConstantType check_constant(CStrRef name) {\n"
    "  const char *s = name.data();\n"
    "  const hashNodeCon *p = findCon(s, name->hash());\n"
    "  if (!p) return NoneBuiltinConstant;\n"
    "  if (p->off > 0) return DynamicBuiltinConstant;\n"
    "  if (strcmp(s, \"STDIN\") == 0 ||\n"
    "      strcmp(s, \"STDOUT\") == 0 ||\n"
    "      strcmp(s, \"STDERR\") == 0) {\n"
    "    return StdioBuiltinConstant;\n"
    "  }\n"
    "  return StaticBuiltinConstant;\n"
    "}\n";

  int tableSize = Util::roundUpToPowerOfTwo(constMap.size() * 2);
  cg_printf(text1,
            Type::KindOfBoolean,
            Type::KindOfInt64,
            Type::KindOfDouble,
            Type::KindOfString,
            Type::KindOfArray,
            Type::KindOfObject,
            Type::KindOfVariant,
            tableSize, constMap.size(),
            system ? "builtin_" : "",
            system ? "" : "  init_builtin_constant_table();\n");
  for (map<string, TypePtr>::const_iterator iter = constMap.begin();
       iter != constMap.end(); iter++) {
    const char *name = iter->first.c_str();
    string escaped = CodeGenerator::EscapeLabel(name);
    string varName = string(Option::ConstantPrefix) +
                     CodeGenerator::FormatLabel(name);
    hphp_string_map<bool>::const_iterator it = dyns.find(iter->first);
    bool dyn = it != dyns.end() && it->second;
    if (dyn) {
      const char *globals =
        system ? "SystemGlobals" : "GlobalVariables";
      cg_printf("      (const char *)\"%s\", "
                "(const char *)"
                "((offsetof(%s, %s) -"
                "  offsetof(%s, %stgv_Variant)) / "
                "sizeof(Variant) + 1), "
                "(const char *)NULL, "
                "(const char *)NULL,\n",
                escaped.c_str(), globals,
                varName.c_str(), globals,
                system ? "s" : "");
    } else {
      TypePtr type = iter->second;
      Type::KindOf kindOf = type->getKindOf();
      cg_printf("      (const char *)\"%s\", "
                "(const char *)-1, "
                "(const char *)%d, ",
                escaped.c_str(), kindOf);
      switch (kindOf) {
      case Type::KindOfBoolean:
      case Type::KindOfInt64:
      case Type::KindOfDouble:
      case Type::KindOfString:
      case Type::KindOfArray:
      case Type::KindOfVariant:
        cg_printf("(const char *)&%s,\n", varName.c_str());
        break;
      case Type::KindOfObject:
        always_assert(system);
        if (strcmp(name, "STDERR") == 0) {
          cg_printf("(const char *)&BuiltinFiles::GetSTDERR,\n");
        } else if (strcmp(name, "STDIN") == 0) {
          cg_printf("(const char *)&BuiltinFiles::GetSTDIN,\n");
        } else if (strcmp(name, "STDOUT") == 0) {
          cg_printf("(const char *)&BuiltinFiles::GetSTDOUT,\n");
        } else {
          not_reached();
        }
        break;
      default:
        throw Exception("During code gen, constant with type %s "
                        "is not expected", type->toString().c_str());
      }
    }
  }
  cg_printf(text2, tableSize - 1, tableSize - 1);
  if (system) cg_print(text3);
}

void AnalysisResult::outputCPPDynamicConstantTable(
  CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  bool system = output == CodeGenerator::SystemCPP;
  string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
    "dynamic_table_constant.cpp";
  Util::mkdir(tablePath);
  ofstream fTable(tablePath.c_str());
  CodeGenerator cg(&fTable, output);

  outputCPPDynamicTablesHeader(cg, true, false);
  map<string, TypePtr> constMap;
  hphp_string_map<bool> dyns;
  ConstantTablePtr ct = getConstants();
   vector<string> syms;
  ct->getSymbols(syms);
  BOOST_FOREACH(string sym, syms) {
    if (system || ct->isSepExtension(sym)) {
      constMap[sym] = ct->getSymbol(sym)->getFinalType();
      dyns[sym] = ct->isDynamic(sym);
    }
  }
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    ConstantTablePtr ct = fs->getConstants();
    vector<string> syms;
    ct->getSymbols(syms);
    BOOST_FOREACH(string sym, syms) {
      if (ct->isSystem(sym) && !system) continue;
      ClassScopePtr defClass;
      if (!ct->getDeclarationRecur(ar, sym, defClass)) {
        always_assert(!defClass);
        continue;
      }
      constMap[sym] = ct->getSymbol(sym)->getFinalType();
      dyns[sym] = ct->isDynamic(sym);
    }
    ct->outputCPP(cg, ar);
  }

  cg.printSection("Get Constant Table");
  const char text1[] = {
    "if (p->off < 0) {\n"
    "  switch (p->type) {\n"
    "  case %d: return *(bool*)(p->value);\n"
    "  case %d: return *(int64*)(p->value);\n"
    "  case %d: return *(double*)(p->value);\n"
    "  case %d: return *(StaticString*)(p->value);\n"
    "  case %d: return *(StaticArray*)(p->value);\n"
    "  case %d: { CVarRef (*f)()=(CVarRef(*)())(p->value); return (*f)(); }\n"
    "  case %d: return *(Variant*)(p->value);\n"
    "  default: not_reached();\n"
    "  }\n"
    "}\n"
  };
  const char text2[] = {
    "if (error) raise_notice(\"Use of undefined constant %s - "
    "assumed '%s'\", s, s);\n"
    "return name;\n"
  };
  bool useHashTable = (constMap.size() > 0);
  if (useHashTable) {
    outputCPPHashTableGetConstant(cg, system, constMap, dyns);
  } else if (system) {
    cg_printf("void init_builtin_constant_table() {}\n");
  } else {
    cg_printf("void init_constant_table() { "
              "init_builtin_constant_table(); }\n");
  }
  cg_indentBegin("Variant get_%sconstant(CStrRef name, bool error) {\n",
      system ? "builtin_" : "");
  cg.printDeclareGlobals();

  if (useHashTable) {
    if (system) cg_printf("const char* s = name.data();\n");
    cg_printf("const hashNodeCon *p = findCon(name.data(), name->hash());\n");
    if (system) {
      cg_indentBegin("if (!p) {\n");
      cg_print(text2);
      cg_indentEnd("}\n");
    } else {
      cg_printf("if (!p) return get_builtin_constant(name, error);\n");
    }
    cg_printf(text1,
              Type::KindOfBoolean,
              Type::KindOfInt64,
              Type::KindOfDouble,
              Type::KindOfString,
              Type::KindOfArray,
              Type::KindOfObject,
              Type::KindOfVariant);
    cg_printf("return getDynamicConstant(g->%stgv_Variant[p->off-1], "
              "name);\n", system ? "s" : "");
  }

  if (!useHashTable) {
    if (system) {
      cg_print(text2);
    } else {
      cg_printf("return get_builtin_constant(name, error);\n");
    }
  }
  cg_indentEnd("}\n");
  cg.namespaceEnd();
  fTable.close();
}

void AnalysisResult::outputCPPDynamicTables(CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  bool system = output == CodeGenerator::SystemCPP;
  bool useSwitch = false;
  {
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      (!useSwitch ? "dynamic_table_func.cpp" : "dynamic_table_func.no.cpp");
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, true, false);

    cg.printSection("Function Invoke Table");
    if (system) {
      bool needGlobals;
      outputCPPJumpTableSupport(cg, ar, 0, needGlobals);
      outputCPPCodeInfoTable(cg, ar, useSwitch, m_functions);
    } else {
      // For functions declared in separable extensions, generate CallInfo and
      // add to declaration list to be included it the table.
      for (StringToFunctionScopePtrMap::const_iterator iter =
             m_functions.begin(); iter != m_functions.end(); ++iter) {
        FunctionScopePtr func = iter->second;
        if (func->isSepExtension()) {
          outputCPPJumpTableSupportMethod(cg, ar, func, Option::FunctionPrefix);
          func->outputCPPCallInfo(cg, ar);
          FunctionScopePtr &funcDec = m_functionDecs[iter->first];
          ASSERT(!funcDec);
          funcDec = func;
        }
      }
      outputCPPCodeInfoTable(cg, ar, useSwitch, m_functionDecs);
    }
    cg.namespaceEnd();
    fTable.close();
  }
  outputCPPDynamicClassTables(output);
  outputCPPDynamicConstantTable(output);
  if (output != CodeGenerator::SystemCPP) {
    string tablePath = m_outputPath + "/" + Option::SystemFilePrefix +
      "dynamic_table_file.cpp";
    Util::mkdir(tablePath);
    ofstream fTable(tablePath.c_str());
    CodeGenerator cg(&fTable, output);

    outputCPPDynamicTablesHeader(cg, false, false);
    cg_printf("typedef Variant (*pm_t)(bool incOnce, "
              "LVariableTable* variables, Globals *globals);\n");
    cg.printSection("File Invoke Table");
    vector<const char*> entries;
    BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
      if (!f->getPseudoMain()) continue;
//      if (f->isPrivateInclude()) continue;
      entries.push_back(f->getName().c_str());
      if (!f->canUseDummyPseudoMain(shared_from_this())) {
        cg_printf("Variant %s%s(bool incOnce, "
                  "LVariableTable* variables, "
                  "Globals *globals);\n",
                  Option::PseudoMainPrefix,
                  Option::MangleFilename(f->getName(), true).c_str());
      }
    }

    cg_printf("\n");
    bool needEvalHook = !system && Option::EnableEval == Option::FullEval;
    outputCPPHashTableInvokeFile(cg, entries, needEvalHook);
    cg.namespaceEnd();
    fTable.close();
  }
}

void AnalysisResult::getCPPClassDeclaredFlags
(CodeGenerator &cg, Type2SymbolSetMap &type2names) {
  SymbolSet &symbols = type2names["bool"];
  for (StringToClassScopePtrVecMap::const_iterator it = m_classDecs.begin();
       it != m_classDecs.end(); ++it) {
    const string &name = CodeGenerator::FormatLabel(Util::toLower(it->first));
    if (!it->second.size() || it->second[0]->isVolatile()) {
      symbols.insert(string("cdec_") + name);
    }
  }
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

  cg.headerBegin(filename);
  BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
    cg_printInclude(fs->outputFilebase());
  }

  cg.printImplStarter();
  cg.namespaceBegin();
  AnalysisResultPtr ar = shared_from_this();
  getVariables()->outputCPP(cg, ar);
  getConstants()->outputCPP(cg, ar);
  cg.namespaceEnd();

  cg.headerEnd(filename);

  cg.setContext(CodeGenerator::CppImplementation);
  cg.useStream(CodeGenerator::ImplFile);
  cg_printf("\n");
  cg.printBasicIncludes();
  cg_printInclude(filename.c_str());
  cg.namespaceBegin();
  outputCPPGlobalImplementations(cg);
  cg.namespaceEnd();

  fSystemImpl.close();
  fSystem.close();

  outputCPPScalarArrays(true);
}

void AnalysisResult::getCPPRedeclaredFunctionDecl
(CodeGenerator &cg, Type2SymbolSetMap &type2names) {
  SymbolSet &symbols = type2names["RedeclaredCallInfoConst*"];
  SymbolSet &bools = type2names["bool"];
  for (StringToFunctionScopePtrMap::const_iterator iter =
      m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    if (iter->second->isVolatile()) {
      std::string fname = CodeGenerator::FormatLabel(iter->first);
      const char *name = fname.c_str();
      if (iter->second->isRedeclaring()) {
        symbols.insert(string("cim_") + name);
      }
      if (strcmp(name, "__autoload")) {
        bools.insert(string(FVF_PREFIX) + name);
      }
    }
  }
}

void AnalysisResult::getCPPRedeclaredClassDecl
(CodeGenerator &cg, Type2SymbolSetMap &type2names) {
  SymbolSet &callbacks = type2names["RedeclaredObjectStaticCallbacksConst*"];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    const string &name = CodeGenerator::FormatLabel(iter->first);
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      callbacks.insert(string(Option::ClassStaticsCallbackPrefix) + name);
    }
  }
}

void AnalysisResult::outputCPPRedeclaredClassImpl(CodeGenerator &cg) {
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    if (iter->second.size() != 1) {
      string s = CodeGenerator::EscapeLabel(iter->first);
      string l = CodeGenerator::FormatLabel(iter->first);
      const char *str = s.c_str();
      const char *lab = l.c_str();
      cg_printf("static StaticString s_%s = \"%s\";\n", lab, str);
      cg_printf("static const RedeclaredObjectStaticCallbacks %s%s = {\n"
                "  {\n"
                "    coo_ObjectData,\n"
                "    0,0,0,0,&s_%s,0,0,0,0,0\n"
                "  },\n"
                "  -1\n"
                "};\n",
                Option::ClassStaticsCallbackNullPrefix, lab, lab);
      cg_printf("%s%s = &%s%s;\n",
                Option::ClassStaticsCallbackPrefix, lab,
                Option::ClassStaticsCallbackNullPrefix, lab);
    }
  }
}

void AnalysisResult::getCPPDynamicConstantDecl
(CodeGenerator &cg, Type2SymbolSetMap &type2names) {
  AnalysisResultPtr ar = shared_from_this();
  getConstants()->getCPPDynamicDecl(cg, ar, type2names);
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->getConstants()->getCPPDynamicDecl(cg, ar, type2names);
  }
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    for (vector<ClassScopePtr>::const_iterator viter = iter->second.begin();
         viter != iter->second.end(); ++viter) {
      const ClassScopePtr &cls = *viter;
      cls->getConstants()->getCPPDynamicDecl(cg, ar, type2names);
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
    cg_printf("static StaticArray %s[%lu];\n", prefix, m_scalarArrayIds.size());
  }
}

void AnalysisResult::outputHexBuffer(CodeGenerator &cg, const char *name,
                                     const char *buf, int len) {
  cg_printf("static const unsigned char %s[%d] = {\n", name, len);
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

  cg_printf("StaticArray %s::%s[%lu];\n",
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
        cg_printf(");\n");
      } else {
        ExpressionListPtr subExpList =
          dynamic_pointer_cast<ExpressionList>(expList->clone());
        for (int i = Option::ScalarArrayOverflowLimit; i < numElems; i++) {
          subExpList->removeElement(Option::ScalarArrayOverflowLimit);
        }
        cg.setInsideScalarArray(true);
        subExpList->outputCPP(cg, ar);
        cg.setInsideScalarArray(false);
        cg_printf(");\n");
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
      }
    } else {
      cg_printf("ArrayData::Create());\n");
    }
    cg_printf("%s[%d].setEvalScalar();\n", prefix, exp.id);
  }
}

string AnalysisResult::getHashedName(int64 hash, int index,
                                     const char *prefix,
                                     bool longName /* = false */) {
  always_assert(index >= 0);
  string name(Option::ScalarPrefix);
  if (Option::SystemGen) name += Option::SysPrefix;
  name += prefix;
  name += longName ? boost::str(boost::format("%016x") % hash)
                   : boost::str(boost::format("%08x") % (int)hash);
  if (index > 0) name += ("_" + lexical_cast<string>(index));
  return name;
}

string AnalysisResult::getScalarArrayName(int hash, int index) {
  return getHashedName(hash, index, Option::StaticArrayPrefix);
}

string AnalysisResult::getScalarVarArrayName(int hash, int index) {
  return getHashedName(hash, index, Option::StaticVarArrPrefix);
}

void AnalysisResult::outputCPPScalarArrayId(CodeGenerator &cg, int id,
                                            int hash, int index,
                                            bool scalarVariant /* = false */) {
  if (Option::UseNamedScalarArray) {
    string name = scalarVariant ? getScalarVarArrayName(hash, index)
                                : getScalarArrayName(hash, index);
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

  cg.headerBegin(filename);
  cg_printInclude("<runtime/base/hphp.h>");

  // This isn't necessarily the right place for separable extension's
  // includes, but more of a "convenient" place.
  outputCPPSepExtensionIncludes(cg);

  cg.printImplStarter();
  cg.namespaceBegin();

  AnalysisResultPtr ar = shared_from_this();
  getVariables()->outputCPP(cg, ar);

  cg_printf("\n");
  if (getVariables()->getAttribute(VariableTable::ContainsLDynamicVariable)) {
    cg_printf("LVariableTable *get_variable_table();\n");
  } else if (getVariables()->
             getAttribute(VariableTable::ContainsDynamicVariable)) {
    cg_printf("RVariableTable *get_variable_table();\n");
  }

  cg.namespaceEnd();
  cg.headerEnd(filename);
  f.close();
}

void AnalysisResult::outputCPPGlobalImplementations(CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();
  CodeGenerator::Context con = cg.getContext();
  cg.setContext(CodeGenerator::CppImplementation);
  getVariables()->outputCPP(cg, ar);
  cg.setContext(con);
}

void AnalysisResult::outputCPPSystemImplementations(CodeGenerator &cg) {
  cg_printf("Globals *globals = get_globals();\n");
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    if (!f->canUseDummyPseudoMain(shared_from_this())) {
      cg_printf("%s%s(false, ",
                Option::PseudoMainPrefix, f->pseudoMainName().c_str());
      if (!f->needPseudoMainVariables()) {
        cg_printf("NULL, globals);\n");
      } else {
        cg_printf("globals, globals);\n");
      }
    }
  }
}

void AnalysisResult::getCPPFileRunDecls(CodeGenerator &cg,
                                        Type2SymbolSetMap &type2names) {
  SymbolSet &symbols = type2names["bool"];
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    if (!f->canUseDummyPseudoMain(shared_from_this())) {
      symbols.insert(string("run_") + Option::PseudoMainPrefix +
                     f->pseudoMainName());
    }
  }
}

void AnalysisResult::getCPPClassStaticInitializerFlags
(CodeGenerator &cg, Type2SymbolSetMap &type2names) {
  SymbolSet &symbols = type2names["bool"];
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      if (cls->needLazyStaticInitializer()) {
        symbols.insert(string(Option::ClassStaticInitializerFlagPrefix) +
                       cls->getId());
      }
    }
  }
}

void AnalysisResult::outputCPPScalarArrays(bool system) {
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

    cg.setInsideScalarArray(true);
    cg_printf("\n");
    cg_printInclude("<runtime/base/hphp.h>");
    cg_printInclude(string(Option::SystemFilePrefix) +
                    (system ? "system_globals.h" : "global_variables.h"));
    if (system) {
      cg_printInclude(string(Option::SystemFilePrefix) +
                      "literal_strings.h");
    }

    cg_printf("\n");
    cg.printImplStarter();
    cg.namespaceBegin();

    AnalysisResultPtr ar = shared_from_this();
    CodeGenerator::Context con = cg.getContext();
    cg.setContext(CodeGenerator::CppImplementation);
    outputCPPScalarArrays(cg, fileCount, i);
    cg.setContext(con);

    cg.namespaceEnd();
    f.close();
  }
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
        cg_printf("ArrayUtil::InitScalarArrays(%s, %lu, "
                  "reinterpret_cast<const char*>(sa_cdata), %d);\n",
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

void AnalysisResult::outputCPPGlobalVariablesMethods() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "global_variables.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  AnalysisResultPtr ar = shared_from_this();

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  if (Option::GenArrayCreate) {
    cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  }
  cg_printInclude(string(Option::SystemFilePrefix) + "literal_strings.h");
  getVariables()->outputCPPGlobalVariablesDtorIncludes(cg, ar);
  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();

  CodeGenerator::Context con = cg.getContext();
  cg.setContext(CodeGenerator::CppImplementation);
  cg_printf("bool has_eval_support = %s;\n",
            (Option::EnableEval > Option::NoEval) ? "true" : "false");
  getVariables()->outputCPPGlobalVariablesDtor(cg);
  getVariables()->outputCPPGlobalVariablesGetImpl(cg, ar);
  getVariables()->outputCPPGlobalVariablesExists  (cg, ar);
  getVariables()->outputCPPGlobalVariablesGetIndex(cg, ar);
  getVariables()->outputCPPGlobalVariablesMethods (cg, ar);
  cg.setContext(con);

  cg.namespaceEnd();

  f.close();
}

///////////////////////////////////////////////////////////////////////////////
// output_global_state()

void AnalysisResult::outputCPPGlobalStateFileHeader(CodeGenerator &cg) {
  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  if (Option::GenArrayCreate) {
    cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  }
  if (Option::EnableEval >= Option::LimitedEval) {
    cg_printInclude("<runtime/eval/eval.h>");
  }
  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();
  cg_printf("namespace global_state {\n");
}

void AnalysisResult::outputCPPGlobalStateBegin(CodeGenerator &cg,
                                               const char *section) {
  cg_indentBegin("void get_%s(Array &res) {\n", section);
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
(CodeGenerator &cg, const StringPairSet &names, const char *section,
 const char *prefix /* = "g->" */, const char *name_prefix /* = "" */) {
  cg_printf("void get_%s(Array &res);\n", section);

  {
    string filename = m_outputPath + "/" + Option::SystemFilePrefix +
      "global_state_" + section + ".no.cpp";
    Util::mkdir(filename);
    ofstream f(filename.c_str());
    CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
    outputCPPGlobalStateFileHeader(cg);

    unsigned size = names.size();
    const unsigned MAXSIZE = 30000;
    if (size > MAXSIZE) {
      unsigned num = (size + MAXSIZE - 1) / MAXSIZE;
      unsigned elems = (size + num - 1) / num;
      StringPairSet::const_iterator iter = names.begin();
      for (unsigned i = 0, j = 0, end = elems; j < num; j++) {
        cg_indentBegin("static void get_%s_%u(Array &%s) {\n",
                       section, j, section);
        cg_printf("DECLARE_GLOBAL_VARIABLES(g);\n");
        while (i < end) {
          cg_printf("%s.set(\"%s%s\", %s%s%s);\n", section,
                    name_prefix, iter->first.c_str(),
                    prefix, name_prefix, iter->second.c_str());
          i++;
          iter++;
        }
        end += elems;
        if (end > size) end = size;
        cg_indentEnd("}\n");
      }
      outputCPPGlobalStateBegin(cg, section);
      for (unsigned j = 0; j < num; j++) {
        cg_printf("get_%s_%u(%s);\n", section, j, section);
      }
      outputCPPGlobalStateEnd(cg, section);
    } else {
      outputCPPGlobalStateBegin(cg, section);
      for (StringPairSet::const_iterator iter = names.begin();
           iter != names.end(); iter++) {
        cg_printf("%s.set(\"%s%s\", %s%s%s);\n", section,
                  name_prefix, iter->first.c_str(),
                  prefix, name_prefix, iter->second.c_str());
      }
      outputCPPGlobalStateEnd(cg, section);
    }
    cg_printf("}\n");
    cg.namespaceEnd();
    f.close();
  }
}

void AnalysisResult::outputCPPGlobalState() {
  string filename = m_outputPath + "/" + Option::SystemFilePrefix +
    "global_state.cpp";
  Util::mkdir(filename);
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);
  AnalysisResultPtr ar = shared_from_this();
  outputCPPGlobalStateFileHeader(cg);

  if (!Option::GenGlobalState) {
    cg_printf("}\n\nusing namespace global_state;\n\n");
    cg_indentBegin("Array get_global_state() {\n");
    cg_printf("return Array();\n");
    cg_indentEnd("}\n\n");
    cg.namespaceEnd();
    f.close();
    return;
  }
  StringPairSetVec symbols(GlobalSymbolTypeCount);
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

  cg_printf("}\n\nusing namespace global_state;\n\n");

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

void AnalysisResult::collectCPPGlobalSymbols(StringPairSetVec &symbols,
                                             CodeGenerator &cg) {
  AnalysisResultPtr ar = shared_from_this();

  // dynamic constants
  StringPairSet *names = &symbols[KindOfDynamicConstant];
  getConstants()->collectCPPGlobalSymbols(*names, cg, ar);
  for (StringToFileScopePtrMap::const_iterator iter = m_files.begin();
       iter != m_files.end(); ++iter) {
    iter->second->getConstants()->collectCPPGlobalSymbols(*names, cg, ar);
  }

  // pseudomain variables
  names = &symbols[KindOfPseudoMain];
  BOOST_FOREACH(FileScopePtr f, m_fileScopes) {
    if (!f->getPseudoMain()) continue;
    if (!f->canUseDummyPseudoMain(ar)) {
      string name = string("run_") + Option::PseudoMainPrefix +
        f->pseudoMainName();
      names->insert(StringPair(name, name));
    }
  }

  // redeclared functions
  names = &symbols[KindOfRedeclaredFunction];
  for (StringToFunctionScopePtrMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    if (iter->second->isVolatile()) {
      std::string fname = CodeGenerator::FormatLabel(iter->first);
      const char *name = fname.c_str();
      if (iter->second->isRedeclaring()) {
        string varname = string("cim_") + name;
        names->insert(StringPair(varname, varname));
      }
      if (strcmp(name, "__autoload")) {
        string varname = string(FVF_PREFIX) + name;
        names->insert(StringPair(varname, varname));
      }
    }
  }

  // redeclared classes
  names = &symbols[KindOfRedeclaredClassId];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    std::string cname = CodeGenerator::FormatLabel(iter->first);
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      string varname = string(Option::ClassStaticsCallbackPrefix) + cname;
      string memname = varname + "->getRedeclaringId()";
      names->insert(StringPair(varname, memname));
    }
  }
  names = &symbols[KindOfRedeclaredClass];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    std::string cname = CodeGenerator::FormatLabel(iter->first);
    if (!iter->second.size() || iter->second[0]->isRedeclaring()) {
      string varname = string(Option::ClassStaticsCallbackPrefix) + cname;
      names->insert(StringPair(varname, varname));
    }
  }

  // volatile classes
  names = &symbols[KindOfVolatileClass];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    std::string cname = CodeGenerator::FormatLabel(iter->first);
    if (iter->second.size() && iter->second[0]->isVolatile()) {
      string varname = string("CDEC(") + cname + ")";
      names->insert(StringPair(varname, varname));
    }
  }

  // classes that need lazy static initializer
  names = &symbols[KindOfLazyStaticInitializer];
  for (StringToClassScopePtrVecMap::const_iterator iter =
         m_classDecs.begin(); iter != m_classDecs.end(); ++iter) {
    std::string cname = CodeGenerator::FormatLabel(iter->first);
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      if (iter->second[i]->needLazyStaticInitializer()) {
        string varname = string(Option::ClassStaticInitializerFlagPrefix) +
          cname;
        string memname = string(Option::ClassStaticInitializerFlagPrefix) +
          iter->second[i]->getId();
        names->insert(StringPair(varname, memname));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void AnalysisResult::outputCPPMain() {
  string mainPath = m_outputPath + "/" + Option::SystemFilePrefix +
    "main.cpp";
  Util::mkdir(mainPath);
  ofstream fMain(mainPath.c_str());
  CodeGenerator cg(&fMain, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg_printInclude("<runtime/base/hphp.h>");
  cg_printInclude(string(Option::SystemFilePrefix) + "global_variables.h");
  if (Option::GenArrayCreate) {
    cg_printInclude(string(Option::SystemFilePrefix) + "cpputil.h");
  }

  cg_printf("\n");
  cg.printImplStarter();
  cg.namespaceBegin();
  outputCPPGlobalImplementations(cg);
  cg_printf("HphpBinary::Type getHphpBinaryType() {\n"
            "#if defined(HHVM_BINARY)\n"
            "  return HphpBinary::hhvm;\n"
            "#elif defined(HPHPI_BINARY)\n"
            "  return HphpBinary::hphpi;\n"
            "#else\n"
            "  return HphpBinary::program;\n"
            "#endif\n"
            "}\n");
  cg.namespaceEnd();

   /*
    * The weird science with preserving argv[0] in case of failure
    * is to avoid messing up dladdr, which we rely on for backtraces.
    * Also realpath the executable so it doesn't look bizarre in top.
    */
  cg_indentBegin("void reExec(const char* pName, char* argv[]) {\n");
  cg_printf("if (strlen(pName) <= 0) return;\n");
  cg_printf("char* oldArgv0 = argv[0];\n");
  cg_printf("char rpath[PATH_MAX];\n");
  cg_printf("realpath(pName, rpath);\n");
  cg_printf("argv[0] = rpath;\n");
  cg_printf("execvp(rpath, argv);\n");
  cg_printf("argv[0] = oldArgv0;\n");
  cg_indentEnd("}\n");

  cg_printf("\n");
  cg_printf("#ifndef HPHP_BUILD_LIBRARY\n");
  cg_indentBegin("int main(int argc, char** argv) {\n");
  cg_printf("#if defined(HPHPI_BINARY) && defined(THUNK_FILENAME)\n");
  cg_printf("reExec(THUNK_FILENAME, argv);\n");
  cg_printf("#endif\n");
  cg_printf("return HPHP::execute_program(argc, argv);\n");
  cg_indentEnd("}\n");
  cg_printf("#endif\n");

  fMain.close();
}

void AnalysisResult::outputCPPClassMap(CodeGenerator &cg,
                                       CodeGenerator::Output cgOutput) {
  AnalysisResultPtr ar = shared_from_this();

  if (!Option::GenerateCPPMetaInfo) return;

  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(ClassScopePtr cls, iter->second) {
      ConstantTablePtr constants = cls->getConstants();
      const std::vector<Symbol*> &constVec =
        constants->getSymbols();
      if (!constVec.size()) continue;
      int i;
      for (i = constVec.size(); i--; ) {
        const Symbol *sym = constVec[i];
        ConstructPtr v = sym->getValue();
        if (!v) continue;
        ExpressionPtr e = static_pointer_cast<Expression>(v);
        if (e->is(Expression::KindOfScalarExpression)) continue;
        if (!e->isScalar()) break;
      }
      if (i >= 0) {
        cg_printf("extern const %sObjectStaticCallbacks %s%s;\n",
                  cls->isRedeclaring() ? "Redeclared" : "",
                  Option::ClassStaticsCallbackPrefix, cls->getId().c_str());
      }
    }
  }

  if (cgOutput == CodeGenerator::SystemCPP) {
    cg_indentBegin("const char* g_system_class_map[] = {\n");
  } else {
    cg_indentBegin("const char *g_class_map[] = {\n");
  }

  // system functions
  cg_printf("(const char *)ClassInfo::IsSystem, NULL, \"\","
            " \"\", NULL, NULL,\n");
  cg_printf("NULL,\n"); // interfaces
  for (StringToFunctionScopePtrMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    ASSERT(!func->isUserFunction());
    func->outputCPPClassMap(cg, ar);
  }

  cg_printf("NULL,\n"); // methods
  cg_printf("NULL,\n"); // properties

  // constants
  int len;
  string output = SymbolTable::getEscapedText(false, len);
  cg_printf("\"false\", (const char *)%d, \"%s\",\n",
            len, output.c_str());
  output = SymbolTable::getEscapedText(true, len);
  cg_printf("\"true\", (const char *)%d, \"%s\",\n",
            len, output.c_str());
  output = SymbolTable::getEscapedText(null, len);
  cg_printf("\"null\", (const char *)%d, \"%s\",\n",
            len, output.c_str());
  if (cgOutput == CodeGenerator::SystemCPP) {
    /*
     * We need system constants to show up in the g_system_class_map
     * for use in the hhvm build, even though they aren't necessarily
     * loaded in our m_constants.
     *
     * Use an empty AnalysisResult so that all constants get output
     * regardless of the ar->isConstantRedeclared check.
     */
    AnalysisResultPtr emptyAr(new AnalysisResult());
    BuiltinSymbols::LoadSystemConstants()->
      outputCPPClassMap(cg, emptyAr, false /* last */);
  }
  m_constants->outputCPPClassMap(cg, ar);

  cg_printf("NULL,\n"); // attributes

  // user functions
  cg_printf("(const char *)ClassInfo::IsNothing, NULL, \"\","
            " \"\", NULL, NULL,\n");
  cg_printf("NULL,\n"); // interfaces
  for (StringToFunctionScopePtrMap::const_iterator iter =
         m_functionDecs.begin(); iter != m_functionDecs.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    if (func->isUserFunction()) {
      if (func->isRedeclaring()) {
        cg_printf("(const char *)(ClassInfo::IsRedeclared), \"%s\", "
                  "(const char *)offsetof(GlobalVariables, GCI(%s)),\n",
                  CodeGenerator::EscapeLabel(iter->first).c_str(),
                  CodeGenerator::FormatLabel(iter->first).c_str());
        StringToFunctionScopePtrVecMap::const_iterator it =
          m_functionReDecs.find(iter->first);
        BOOST_FOREACH(func, it->second) {
          func->outputCPPClassMap(cg, ar);
        }
        cg_printf("NULL,\n");
      } else {
        func->outputCPPClassMap(cg, ar);
      }
    }
  }
  cg_printf("NULL,\n"); // methods
  cg_printf("NULL,\n"); // properties

  // user defined constants
  for (int i = 0; i < (int)m_fileScopes.size(); i++) {
    ConstantTablePtr constants = m_fileScopes[i]->getConstants();
    constants->outputCPPClassMap(cg, ar, (i == (int)m_fileScopes.size() - 1));
  }

  cg_printf("NULL,\n"); // attributes

  // system classes
  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    iter->second->outputCPPClassMap(cg, ar);
  }

  // user classes
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    bool redec = iter->second.size() != 1;
    if (redec) {
      cg_printf("(const char *)(ClassInfo::IsRedeclared | "
                "ClassInfo::IsVolatile), \"%s\", "
                "(const char *)offsetof(GlobalVariables,%s%s),\n",
                CodeGenerator::EscapeLabel(iter->first).c_str(),
                Option::ClassStaticsCallbackPrefix,
                CodeGenerator::FormatLabel(iter->first).c_str());
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
  cg_printf("\n");

  cg.printBasicIncludes();
  // includes
  map<string, FileScopePtr> toInclude;
  BOOST_FOREACH(FileScopePtr fs, files) {
    getTrueDeps(fs, toInclude);
  }
  BOOST_FOREACH(FileScopePtr fs, files) {
    cg_printInclude(fs->outputFilebase() + ".fws");
    cg_printInclude(fs->outputFilebase());
    toInclude.erase(fs->getName());
  }
  cg.printSection("Dependencies");
  for (map<string, FileScopePtr>::const_iterator iter = toInclude.begin();
       iter != toInclude.end(); ++iter) {
    FileScopePtr fs = iter->second;
    cg_printInclude(fs->outputFilebase());
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
    if (m_pregenerated) {
      const string &code = getPregeneratedCPP(fs->getName());
      if (Option::GenerateSourceInfo) {
        int offset = cg.getLineNo(CodeGenerator::PrimaryStream) - 1;
        movePregeneratedSourceInfo(fs->getName(), cg.getFileName(), offset);
      }
      cg.printRaw(code.c_str());
    } else {
      outputCPPFileImpl(cg, fs);
    }
  }
  ClassScope::outputCPPGetClassPropTableImpl(cg, shared_from_this(),
                                             cg.getClasses());
  cg.namespaceEnd();
}

void AnalysisResult::outputCPPFileImpl(CodeGenerator &cg, FileScopePtr fs) {
  AnalysisResultPtr ar = shared_from_this();
  cg.setContext(CodeGenerator::CppImplementation);
  fs->outputCPPImpl(cg, ar);
  cg.setContext(CodeGenerator::CppPseudoMain);
  fs->outputCPPPseudoMain(cg, ar);
}

void AnalysisResult::preGenerateCPP(CodeGenerator::Output output,
                                    const FileScopePtrVec &files,
                                    int threadCount) {
  AnalysisResultPtr ar = shared_from_this();
  vector<PreGenerateCPPJob*> jobs;
  JobQueueDispatcher<OutputJob*, OutputWorker>
    dispatcher(threadCount, true, 0, false, NULL);

  m_pregenerating = true;
  BOOST_FOREACH(FileScopePtr fs, files) {
    PreGenerateCPPJob *job = new PreGenerateCPPJob(ar, output, fs);
    jobs.push_back(job);
    dispatcher.enqueue(job);
  }

  dispatcher.run();
  m_pregenerating = false;
  for (unsigned int i = 0; i < jobs.size(); ++i) {
    delete jobs[i];
  }
}

void AnalysisResult::addPregeneratedCPP(const std::string &name,
                                        std::string &code) {
  Lock lock(m_pregenMapMutex);
  code.swap(m_pregenMap[name]);
}

const string &AnalysisResult::getPregeneratedCPP(const string &name) {
  Lock lock(m_pregenMapMutex);
  StringMap::const_iterator iter = m_pregenMap.find(name);
  ASSERT(iter != m_pregenMap.end());
  return iter->second;
}

void AnalysisResult::movePregeneratedSourceInfo(const std::string &source,
                                                const std::string &target,
                                                int offset) {
  Lock lock(m_sourceInfoMutex);
  SourceInfo::const_iterator iterPregen = m_sourceInfoPregen.find(source);
  ASSERT(iterPregen != m_sourceInfoPregen.end());

  const SourceLocationMap &sourceLoc = iterPregen->second;
  SourceLocationMap &targetLoc = m_sourceInfos[target];
  for (SourceLocationMap::const_iterator iterLoc = sourceLoc.begin();
       iterLoc != sourceLoc.end(); ++iterLoc) {
    targetLoc[iterLoc->first + offset] = iterLoc->second;
  }
}

int AnalysisResult::getFileSize(FileScopePtr fs) {
  if (m_pregenerated) {
    StringMap::const_iterator iterPregen = m_pregenMap.find(fs->getName());
    ASSERT(iterPregen != m_pregenMap.end());
    return iterPregen->second.size();
  } else {
    return fs->getSize();
  }
}

void AnalysisResult::outputFFI(vector<string> &additionalCPPs) {
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputCPPFFI(cg, ar);
    }
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
    const StringToFunctionScopePtrMap &fns = fs->getFunctions();
    for (StringToFunctionScopePtrMap::const_iterator it = fns.begin();
         it != fns.end(); ++it) {
      FunctionScopePtr func = it->second;
      if (func->inPseudoMain()) continue;
      if (func->isLocalRedeclaring()) {
        StringToFunctionScopePtrVecMap::const_iterator iter =
          fs->getRedecFunctions()->find(it->first);
        BOOST_FOREACH(FunctionScopePtr func, iter->second) {
          if (first) {
            first = false;
          } else {
            cg_printf(", ");
          }
          cg_printf("f_%s", func->getId().c_str());
        }
      } else {
        if (first) {
          first = false;
        } else {
          cg_printf(", ");
        }
        cg_printf("f_%s", func->getId().c_str());
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputJavaFFI(cg, ar);
    }
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputJavaFFICPPStub(cg, ar);
    }
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputJavaFFICPPStub(cg, ar);
    }
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputSwigFFIStubs(cg, ar);
    }
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

  if (Option::GenerateFFIStaticBinding) {
    BOOST_FOREACH(FileScopePtr fs, m_fileScopes) {
      fs->outputSwigFFIStubs(cg, ar);
    }
  }

  f.close();
}

/**
 * Literal string to String precomputation
 */
string AnalysisResult::getLiteralStringName(int64 hash, int index,
  bool iproxy /* = false */) {
  return getHashedName(hash, index,
    iproxy ? Option::StaticStringProxyPrefix : Option::StaticStringPrefix);
}

string AnalysisResult::getLitVarStringName(int64 hash, int index,
  bool iproxy /* = false */) {
  return getHashedName(hash, index,
    iproxy ? Option::StaticVarStrProxyPrefix : Option::StaticVarStrPrefix);
}

int AnalysisResult::getLiteralStringId(const std::string &s, int &index) {
  Lock lock(m_namedStringLiteralsMutex);

  int hash = hash_string_cs(s.data(), s.size());
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

void AnalysisResult::addNamedLiteralVarString(const std::string &s) {
  Lock lock(m_namedStringLiteralsMutex);
  m_namedVarStringLiterals.insert(s);
}

void AnalysisResult::addNamedScalarVarArray(const std::string &s) {
  Lock lock(m_namedScalarArraysMutex);
  m_namedScalarVarArrays.insert(s);
}

string AnalysisResult::getFuncId(ClassScopePtr cls, FunctionScopePtr func) {
  CodeGenerator cg;
  if (cls) {
    return cls->getId() + "::" + func->getId();
  }
  return func->getId();
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

void AnalysisResult::cloneRTTIFuncs(
  const StringToFunctionScopePtrMap &functions,
  const StringToFunctionScopePtrVecMap *redecFunctions) {
  for (StringToFunctionScopePtrMap::const_iterator iter =
         functions.begin(); iter != functions.end(); ++iter) {
    FunctionScopePtr func = iter->second;
    if (func->isRedeclaring()) {
      ASSERT(redecFunctions);
      BOOST_FOREACH(func, redecFunctions->find(iter->first)->second) {
        const string funcId = getFuncId(func->getContainingClass(), func);
        if (RTTIInfo::TheRTTIInfo.exists(funcId.c_str())) {
          StatementPtr stmt = func->getStmt();
          func->setStmtCloned(stmt->clone());
        }
      }
    } else {
      const string funcId = getFuncId(func->getContainingClass(), func);
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
    cloneRTTIFuncs(m_fileScopes[i]->getFunctions(),
                   m_fileScopes[i]->getRedecFunctions());

    // class rtti methods
    for (StringToClassScopePtrVecMap::const_iterator iter =
         m_fileScopes[i]->getClasses().begin();
         iter != m_fileScopes[i]->getClasses().end(); ++iter) {
      for (unsigned int j = 0; j < iter->second.size(); j++) {
        ClassScopePtr cls = iter->second[j];
        cloneRTTIFuncs(cls->getFunctions(), 0);
      }
    }
  }
}

StringToClassScopePtrVecMap AnalysisResult::getExtensionClasses() {
  StringToClassScopePtrVecMap exts;
  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    ClassScopePtr cls = iter->second;
    if (cls->isExtensionClass()) exts[cls->getName()].push_back(cls);
  }
  return exts;
}

void AnalysisResult::outputInitLiteralVarStrings(CodeGenerator &cg,
  int fileIndex, vector<int> &litVarStrFileIndices,
  vector<pair<int, int> > &litVarStrs) {
  if (litVarStrs.size() >  0) {
    cg_indentBegin("void %sinit_literal_varstrings_%d() {\n",
                   (Option::SystemGen ? Option::SysPrefix : ""),  fileIndex);
    for (unsigned int i = 0; i < litVarStrs.size(); i++) {
      int hash = litVarStrs[i].first;
      int index = litVarStrs[i].second;
      cg_printf("%s = %s;\n",
                getLitVarStringName(hash, index).c_str(),
                getLiteralStringName(hash, index).c_str());
    }
    cg_indentEnd("}\n");
    litVarStrFileIndices.push_back(fileIndex);
  }
  litVarStrs.clear();
}

void AnalysisResult::outputStringProxyData(CodeGenerator &cg,
  int fileIndex, vector<string> &lStrings,
  vector<pair<string, int> > &bStrings) {
  if (lStrings.size() + bStrings.size() > 0) {
    cg_indentBegin("static const char *ss_data%d[] = {\n", fileIndex);
    for (uint i = 0; i < lStrings.size(); i++) {
      cg_printf("%s,\n", lStrings[i].c_str());
    }
    for (uint i = 0; i < bStrings.size(); i++) {
      cg_printf("%s, (const char *)%lldLL,\n",
                bStrings[i].first.c_str(), (int64)bStrings[i].second);
    }
    cg_indentEnd("};\n");
    cg_printf("static int ATTRIBUTE_UNUSED initLiteralStrings%d = "
              "StringUtil::InitLiteralStrings(ss_data%d, %ld, %ld);\n",
              fileIndex, fileIndex, lStrings.size(), bStrings.size());
    lStrings.clear();
    bStrings.clear();
  }
}

void AnalysisResult::outputVarStringProxyData(CodeGenerator &cg,
  int fileIndex, vector<pair<int, int> > &litVarStrs) {
  if (litVarStrs.empty()) return;
  cg_indentBegin("static const char *svs_data%d[] = {\n", fileIndex);
  for (unsigned int i = 0; i < litVarStrs.size(); i++) {
    int hash = litVarStrs[i].first;
    int index = litVarStrs[i].second;
    cg_printf("(const char *)&%s, (const char *)&%s,\n",
              getLitVarStringName(hash, index).c_str(),
              getLiteralStringName(hash, index).c_str());
  }
  cg_indentEnd("};\n");
  cg_printf("static int ATTRIBUTE_UNUSED initLiteralVarStrings%d = "
            "StringUtil::InitLiteralVarStrings(svs_data%d, %lu);\n",
            fileIndex, fileIndex, litVarStrs.size());
  litVarStrs.clear();
}

void AnalysisResult::outputCPPNamedLiteralStrings(bool genStatic,
                                                  const string &file) {
  AnalysisResultPtr ar = shared_from_this();
  string filename = genStatic ? file : (file + ".h");
  ofstream f(filename.c_str());
  CodeGenerator cg(&f, CodeGenerator::ClusterCPP);

  cg_printf("\n");
  cg.headerBegin(filename);
  if (!genStatic) {
    cg_printInclude("<runtime/base/hphp.h>");
    if (Option::SystemGen) {
      cg_printInclude("<system/gen/sys/literal_strings_remap.h>");
    } else {
      cg_printInclude("<sys/literal_strings_remap.h>");
    }
    cg_printf("\n");
  }
  int nstrings = 0;
  cg.printImplStarter();
  cg.namespaceBegin();
  for (map<int, vector<string> >::const_iterator it =
       m_namedStringLiterals.begin(); it != m_namedStringLiterals.end();
       it++) {
    int hash = it->first;
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getLiteralStringName(hash, i);
      if (genStatic) {
        cg_printf("static StaticString %s(", name.c_str());
        cg_printString(strings[i], ar, BlockScopePtr(), false);
        cg_printf(");\n");
      } else {
        if (Option::UseStaticStringProxy) {
          string proxyName = getLiteralStringName(hash, i, true);
          cg_printf("extern StaticStringProxy %s;\n", proxyName.c_str());
          cg_printf("#ifndef %s\n", name.c_str());
          cg_printf("#define %s (*(StaticString *)(&%s))\n",
                    name.c_str(), proxyName.c_str());
          cg_printf("#endif\n");
        } else {
          cg_printf("extern StaticString %s;\n", name.c_str());
        }
      }
      if (m_namedVarStringLiterals.find(name) !=
          m_namedVarStringLiterals.end()) {
        if (Option::UseStaticStringProxy) {
          string name = getLitVarStringName(hash, i);
          string proxyName = getLitVarStringName(hash, i, true);
          cg_printf("extern VariantProxy %s;\n", proxyName.c_str());
          cg_printf("#ifndef %s\n", name.c_str());
          cg_printf("#define %s (*(Variant*)(&%s))\n",
                    name.c_str(), proxyName.c_str());
          cg_printf("#endif\n");
        } else {
          cg_printf(genStatic ? "static " : "extern ");
          cg_printf("VarNR %s;\n", getLitVarStringName(hash, i).c_str());
        }
      }
    }
    nstrings += strings.size();
  }
  cg.namespaceEnd();
  cg.headerEnd(filename);
  f.close();

  if (genStatic || (nstrings == 0 && !Option::UseScalarVariant)) return;

  int chunkSize;
  if (ar->isSystem()) {
    // generate one single literal_strings file for system
    chunkSize = nstrings;
  } else {
    chunkSize = (nstrings + Option::LiteralStringFileCount) /
                Option::LiteralStringFileCount;
  }
  int count = 0;

  vector<int> litVarStrFileIndices;
  vector<pair<int, int> > litVarStrs;
  vector<string> lStrings;
  vector<pair<string, int> > bStrings;
  int fileIndex = 0;
  for (map<int, vector<string> >::const_iterator it =
       m_namedStringLiterals.begin(); it != m_namedStringLiterals.end();
       it++) {
    int hash = it->first;
    const vector<string> &strings = it->second;
    for (unsigned int i = 0; i < strings.size(); i++) {
      string name = getLiteralStringName(hash, i);
      if (count % chunkSize == 0) {
        if (count != 0) {
          outputStringProxyData(cg, fileIndex, lStrings, bStrings);
          if (Option::UseStaticStringProxy) {
            outputVarStringProxyData(cg, fileIndex, litVarStrs);
          } else {
            outputInitLiteralVarStrings(cg, fileIndex, litVarStrFileIndices,
                                        litVarStrs);
          }
          cg.namespaceEnd();
          f.close();
        }
        fileIndex = count / chunkSize;
        filename = file + "_" + lexical_cast<string>(fileIndex) + ".no.cpp";
        f.open(filename.c_str());
        cg_printf("\n");
        cg_printInclude("<runtime/base/complex_types.h>");
        if (Option::SystemGen) {
          cg_printInclude("<system/gen/sys/literal_strings_remap.h>");
        } else {
          cg_printInclude("<sys/literal_strings_remap.h>");
        }
        if (Option::UseStaticStringProxy) {
          cg_printInclude("<runtime/base/string_util.h>");
        }
        cg_printf("\n");
        cg.namespaceBegin();
      }
      count++;
      if (Option::UseStaticStringProxy) {
        string proxyName = getLiteralStringName(hash, i, true);
        cg_printf("StaticStringProxy %s;\n", proxyName.c_str());
        cg_printf("#ifndef %s\n", name.c_str());
        cg_printf("#define %s (*(StaticString *)(&%s))\n",
                  name.c_str(), proxyName.c_str());
        cg_printf("#endif\n");
        bool isBinary = false;
        string escaped = CodeGenerator::EscapeLabel(strings[i], &isBinary);
        string out("(const char *)&");
        out += proxyName;
        out += ", (const char *)\"";
        out += escaped;
        out += "\"";
        if (isBinary) {
          bStrings.push_back(
            pair<string, int>(out, (int)strings[i].size()));
        } else {
          lStrings.push_back(out);
        }
      } else {
        cg_printf("StaticString %s(", name.c_str());
        cg_printString(strings[i], ar, BlockScopePtr(), false);
        cg_printf(");\n");
      }
      if (m_namedVarStringLiterals.find(name) !=
          m_namedVarStringLiterals.end()) {
        if (Option::UseStaticStringProxy) {
          string name = getLitVarStringName(hash, i);
          string proxyName = getLitVarStringName(hash, i, true);
          cg_printf("VariantProxy %s;\n", proxyName.c_str());
          cg_printf("#ifndef %s\n", name.c_str());
          cg_printf("#define %s (*(Variant*)(&%s))\n",
                    name.c_str(), proxyName.c_str());
          cg_printf("#endif\n");
        } else {
          cg_printf("VarNR %s;\n", getLitVarStringName(hash, i).c_str());
        }
        litVarStrs.push_back(pair<int, int>(hash, i));
      }
    }
  }
  if (Option::UseScalarVariant) {
    if (nstrings == 0) {
      filename = file + ".cpp";
      f.open(filename.c_str());
      cg.namespaceBegin();
    }
    cg_printf("\n");
    for (unsigned int i = 0; i < litVarStrFileIndices.size(); i++) {
      cg_printf("extern void %sinit_literal_varstrings_%d();\n",
                (Option::SystemGen ? Option::SysPrefix : ""),
                litVarStrFileIndices[i]);
    }
    outputStringProxyData(cg, fileIndex, lStrings, bStrings);
    if (Option::UseStaticStringProxy) {
      outputVarStringProxyData(cg, fileIndex, litVarStrs);
    } else {
      outputInitLiteralVarStrings(cg, fileIndex, litVarStrFileIndices,
                                  litVarStrs);
    }
    cg_indentBegin("void %sinit_literal_varstrings() {\n",
                   Option::SystemGen ? Option::SysPrefix : "");
    if (!Option::SystemGen) {
      cg_printf("extern void %sinit_literal_varstrings();\n",
                Option::SysPrefix);
      cg_printf("%sinit_literal_varstrings();\n", Option::SysPrefix);
    }
    if (!Option::UseStaticStringProxy) {
      for (unsigned int i = 0; i < litVarStrFileIndices.size(); i++) {
        cg_printf("%sinit_literal_varstrings_%d();\n",
                  (Option::SystemGen ? Option::SysPrefix : ""),
                  litVarStrFileIndices[i]);
      }
    }
    cg_indentEnd("}\n");
  } else if (!Option::SystemGen) {
    cg_printf("void init_literal_varstrings() {}\n");
  }
  cg.namespaceEnd();
  f.close();
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
  cg.namespaceBegin();

  for (StringToClassScopePtrMap::const_iterator iter = m_systemClasses.begin();
       iter != m_systemClasses.end(); ++iter) {
    ClassScopePtr cls = iter->second;
    if (cls->isSepExtension()) {
      cls->outputCPPDynamicClassImpl(cg, ar);
      cls->outputCPPSupportMethodsImpl(cg, ar);
    }
  }
  ClassScope::outputCPPGetClassPropTableImpl(cg, shared_from_this(),
                                             cg.getClasses());
  cg.namespaceEnd();
  fTable.close();
  outputCPPNamedLiteralStrings(true, litstrFile);
  always_assert(m_scalarArrays.size() == 0);
}

