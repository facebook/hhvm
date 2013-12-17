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

#include "hphp/compiler/analysis/analysis_result.h"

#include <iomanip>
#include <algorithm>
#include <sstream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include "hphp/compiler/analysis/alias_manager.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/depth_first_visitor.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/package.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/util/atomic.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/util/hash.h"
#include "hphp/util/process.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/timer.h"

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
    m_package(nullptr), m_parseOnDemand(false), m_phase(ParseAllFiles) {
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
  assert(fileScope);
  FileScopePtr &res = m_files[fileScope->getName()];
  assert(!res);
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

void AnalysisResult::addNSFallbackFunc(ConstructPtr c, FileScopePtr fs) {
  m_nsFallbackFuncs.insert(std::make_pair(c, fs));
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
  if (bit != m_functions.end() && !bit->second->allowOverride()) {
    return bit->second;
  }
  StringToFunctionScopePtrMap::const_iterator iter =
    m_functionDecs.find(funcName);
  if (iter != m_functionDecs.end()) {
    return iter->second;
  }
  return bit != m_functions.end() ? bit->second : FunctionScopePtr();
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

const ClassScopePtrVec &
AnalysisResult::findRedeclaredClasses(const std::string &name) const {
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
  assert(m_phase < AnalyzeAll);

  string fname = funcScope->getName();
  // System functions override
  auto it = m_functions.find(fname);
  if (it != m_functions.end()) {
    if (!it->second->allowOverride()) {
      // we need someone to hold on to a reference to it
      // even though we're not going to do anything with it
      this->lock()->m_ignoredScopes.push_back(funcScope);
      return false;
    }
  }

  return true;
}

bool AnalysisResult::declareClass(ClassScopePtr classScope) const {
  assert(m_phase < AnalyzeAll);

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
}

void AnalysisResult::markRedeclaringClasses() {
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

  auto markRedeclaring = [&] (const std::string& name) {
    auto it = m_classDecs.find(name);
    if (it != m_classDecs.end()) {
      auto& classes = it->second;
      for (unsigned int i = 0; i < classes.size(); ++i) {
        classes[i]->setRedeclaring(ar, i);
      }
    }
  };

  /*
   * In WholeProgram mode, during parse time we collected all
   * class_alias calls so we can mark the targets of such calls
   * redeclaring if necessary.
   *
   * Two cases here that definitely require this:
   *
   *  - If an alias name has the same name as another class, we need
   *    to mark *that* class as redeclaring, since it may mean
   *    different things in different requests now.
   *
   *  - If an alias name can refer to more than one class, each of
   *    those classes must be marked redeclaring.
   *
   * In the simple case of a unique alias name and a unique target
   * name, we might be able to get away with manipulating the target
   * classes' volatility.
   *
   * Rather than work through the various cases here, though, we've
   * just decided to just play it safe and mark all the names involved
   * as redeclaring for now.
   */
  for (auto& kv : m_classAliases) {
    assert(kv.first == Util::toLower(kv.first));
    assert(kv.second == Util::toLower(kv.second));
    markRedeclaring(kv.first);
    markRedeclaring(kv.second);
  }

  /*
   * Similar to class_alias, when a type alias is declared with the
   * same name as a class in the program, we need to make sure the
   * class is marked redeclaring.  It is possible in some requests
   * that things like 'instanceof Foo' will not mean the same thing.
   */
  for (auto& name : m_typeAliasNames) {
    assert(Util::toLower(name) == name);
    markRedeclaring(name);
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
  if (m_systemClasses.find(className) != m_systemClasses.end())
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
  if (m_functions.find(functionName) != m_functions.end())
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
  assert(!includeFilename.empty());
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

void AnalysisResult::addSystemFunction(FunctionScopeRawPtr fs) {
  FunctionScopePtr& entry = m_functions[fs->getName()];
  assert(!entry);
  entry = fs;
}

void AnalysisResult::addSystemClass(ClassScopeRawPtr cs) {
  ClassScopePtr& entry = m_systemClasses[cs->getName()];
  assert(!entry);
  entry = cs;
}

void AnalysisResult::checkClassDerivations() {
  AnalysisResultPtr ar = shared_from_this();
  ClassScopePtr cls;
  for (StringToClassScopePtrVecMap::const_iterator iter = m_classDecs.begin();
       iter != m_classDecs.end(); ++iter) {
    BOOST_FOREACH(cls, iter->second) {
      hphp_string_iset seen;
      cls->checkDerivation(ar, seen);
      if (Option::WholeProgram) {
        cls->importUsedTraits(ar);
      }
    }
  }
}

void AnalysisResult::resolveNSFallbackFuncs() {
  for (auto &pair : m_nsFallbackFuncs) {
    SimpleFunctionCallPtr sfc =
      static_pointer_cast<SimpleFunctionCall>(pair.first);
    sfc->resolveNSFallbackFunc(
      shared_from_this(),
      pair.second
    );
  }
}

void AnalysisResult::collectFunctionsAndClasses(FileScopePtr fs) {
  for (const auto& iter : fs->getFunctions()) {
    FunctionScopePtr func = iter.second;
    if (!func->inPseudoMain()) {
      FunctionScopePtr &funcDec = m_functionDecs[iter.first];
      if (funcDec) {
        if (funcDec->isSystem()) {
          assert(funcDec->allowOverride());
          funcDec = func;
        } else if (func->isSystem()) {
          assert(func->allowOverride());
        } else {
          FunctionScopePtrVec &funcVec = m_functionReDecs[iter.first];
          int sz = funcVec.size();
          if (!sz) {
            funcDec->setRedeclaring(sz++);
            funcVec.push_back(funcDec);
          }
          func->setRedeclaring(sz++);
          funcVec.push_back(func);
        }
      } else {
        funcDec = func;
      }
    }
  }

  if (const StringToFunctionScopePtrVecMap *redec = fs->getRedecFunctions()) {
    for (const auto &iter : *redec) {
      FunctionScopePtrVec::const_iterator i = iter.second.begin();
      FunctionScopePtrVec::const_iterator e = iter.second.end();
      FunctionScopePtr &funcDec = m_functionDecs[iter.first];
      assert(funcDec); // because the first one was in funcs above
      FunctionScopePtrVec &funcVec = m_functionReDecs[iter.first];
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

  for (const auto& iter : fs->getClasses()) {
    ClassScopePtrVec &clsVec = m_classDecs[iter.first];
    clsVec.insert(clsVec.end(), iter.second.begin(), iter.second.end());
  }

  m_classAliases.insert(fs->getClassAliases().begin(),
                        fs->getClassAliases().end());
  m_typeAliasNames.insert(fs->getTypeAliasNames().begin(),
                          fs->getTypeAliasNames().end());
}

static bool by_filename(const FileScopePtr &f1, const FileScopePtr &f2) {
  return f1->getName() < f2->getName();
}

void AnalysisResult::analyzeProgram(bool system /* = false */) {
  AnalysisResultPtr ar = shared_from_this();

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

  markRedeclaringClasses();

  // Analyze some special cases
  for (set<string>::const_iterator it = Option::VolatileClasses.begin();
       it != Option::VolatileClasses.end(); ++it) {
    ClassScopePtr cls = findClass(Util::toLower(*it));
    if (cls && cls->isUserClass()) {
      cls->setVolatile();
    }
  }

  checkClassDerivations();
  resolveNSFallbackFuncs();

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
      if (Option::WholeProgram && !func->hasImpl() && needAbstractMethodImpl) {
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

  // Keep generated code identical without randomness
  canonicalizeSymbolOrder();

  // XXX: this is only here because canonicalizeSymbolOrder used to do
  // it---is it necessary to repeat at this phase?  (Probably not ...)
  markRedeclaringClasses();

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
  /* implicit */ OptVisitor(const Visitor &po)
    : m_ar(po.m_ar)
    , m_nscope(po.m_nscope)
    , m_dispatcher(po.m_dispatcher)
  {
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
  JobQueueDispatcher<OptWorker<When>> *m_dispatcher;
};

template <typename When>
class OptWorker : public JobQueueWorker<BlockScope*,
                                        void*,
                                        true,
                                        true> {
public:
  OptWorker() {}

  virtual void onThreadEnter() {
  }

  virtual void onThreadExit() {
  }

  virtual void doJob(BlockScope *scope) {
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
    ++AnalysisResult::s_NumDoJobCalls;
    ConcurrentBlockScopeRawPtrIntHashMap::accessor acc;
    AnalysisResult::s_DoJobUniqueScopes.insert(acc,
      BlockScopeRawPtr(scope));
    acc->second += 1;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
    try {
      auto visitor =
        (DepthFirstVisitor<When, OptVisitor>*) m_context;
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
      assert(useKinds >= 0);

      {
        Lock l2(BlockScope::s_depsMutex);
        Lock l1(BlockScope::s_jobStateMutex);

        assert(scope->getMark() == BlockScope::MarkProcessing);
        assert(scope->getNumDepsToWaitFor() == 0);
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
                ++AnalysisResult::s_NumForceRerunGlobal;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                pf->first->setForceRerun(true);
                break;
              case BlockScope::MarkProcessed:
#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
                ++AnalysisResult::s_NumReactivateGlobal;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */
                if (visitor->activateScope(pf->first)) {
                  visitor->enqueue(pf->first);
                }
                break;
              default: assert(false);
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
                ++AnalysisResult::s_NumReactivateUseKinds;
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
                ++AnalysisResult::s_NumForceRerunUseKinds;
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

template<>
void OptWorker<Pre>::onThreadEnter() {
  hphp_session_init();
}

template<>
void OptWorker<Pre>::onThreadExit() {
  hphp_session_exit();
}

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
      new JobQueueDispatcher<worker>( \
        threadCount, true, 0, false, this); \
  } while (0)

#define IMPLEMENT_OPT_VISITOR_ENQUEUE(scope) \
  do { \
    assert((scope)->getMark() == BlockScope::MarkReady); \
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
                                void *context) {
  // default is no-op
}

template <typename When>
bool
AnalysisResult::postWaitCallback(bool first,
                                 bool again,
                                 const BlockScopeRawPtrQueue &scopes,
                                 void *context) {
  // default is no-op
  return again;
}

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
std::atomic<int> AnalysisResult::s_NumDoJobCalls(0);
std::atomic<int> AnalysisResult::s_NumForceRerunGlobal(0);
std::atomic<int> AnalysisResult::s_NumReactivateGlobal(0);
std::atomic<int> AnalysisResult::s_NumForceRerunUseKinds(0);
std::atomic<int> AnalysisResult::s_NumReactivateUseKinds(0);

ConcurrentBlockScopeRawPtrIntHashMap
  AnalysisResult::s_DoJobUniqueScopes;

static inline int CountScopesWaiting(const BlockScopeRawPtrQueue &scopes) {
  int s = 0;
  for (BlockScopeRawPtrQueue::const_iterator it = scopes.begin();
       it != scopes.end(); ++it) {
    int m = (*it)->getMark();
    assert(m == BlockScope::MarkWaiting ||
           m == BlockScope::MarkProcessed);
    if (m == BlockScope::MarkWaiting) s++;
  }
  return s;
}

static inline void DumpScope(BlockScopeRawPtr scope, const char *prefix,
                             bool newline = true) {
  assert(scope->is(BlockScope::FunctionScope) ||
         scope->is(BlockScope::ClassScope));
  const char *type = scope->is(BlockScope::FunctionScope) ?
    "function" : "class";
  std::cout << prefix << type << " " << scope->getName() << " @ "
            << scope->getContainingFile()->getName();
  if (newline) std::cout << std::endl;
}

static inline void DumpScopeWithDeps(BlockScopeRawPtr scope) {
  assert(scope->is(BlockScope::FunctionScope) ||
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
                                      void *context /* = NULL */) {
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
    assert(RescheduleException::s_NumReschedules          == 0);
    assert(RescheduleException::s_NumForceRerunSelfCaller == 0);
    assert(RescheduleException::s_NumRetTypesChanged      == 0);
    assert(BaseTryLock::s_LockProfileMap.empty());
#endif /* HPHP_INSTRUMENT_TYPE_INF */

    BlockScopeRawPtrQueue enqueued;
    again = dfv.visitParallel(scopes, first, enqueued);
    preWaitCallback<When>(first, scopes, context);

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

    assert(!dfv.data().getQueuedJobs());
    assert(!dfv.data().getActiveWorker());

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

    again = postWaitCallback<When>(first, again, scopes, context);
    first = false;
  } while (again);
  dfv.data().stop();

  for (BlockScopeRawPtrQueue::iterator
       it = scopes.begin(), end = scopes.end();
       it != end; ++it) {
    assert((*it)->getMark() == BlockScope::MarkProcessed);
    assert((*it)->getNumDepsToWaitFor() == 0);
    assert(!(*it)->needsReschedule());
    assert((*it)->rescheduleFlags() == 0);
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
        ++RescheduleException::s_NumForceRerunSelfCaller;
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
    ++RescheduleException::s_NumReschedules;
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
    bool first, bool again,
    const BlockScopeRawPtrQueue &scopes, void *context) {

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
std::atomic<int> RescheduleException::s_NumReschedules(0);
std::atomic<int> RescheduleException::s_NumForceRerunSelfCaller(0);
std::atomic<int> RescheduleException::s_NumRetTypesChanged(0);
LProfileMap BaseTryLock::s_LockProfileMap;
#endif /* HPHP_INSTRUMENT_TYPE_INF */

void AnalysisResult::inferTypes() {
  setPhase(FirstInference);
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);

  for (auto scope : scopes) {
    scope->setInTypeInference(true);
    scope->clearUpdated();
    assert(scope->getNumDepsToWaitFor() == 0);
  }

#ifdef HPHP_INSTRUMENT_TYPE_INF
  assert(RescheduleException::s_NumReschedules          == 0);
  assert(RescheduleException::s_NumForceRerunSelfCaller == 0);
  assert(BaseTryLock::s_LockProfileMap.empty());
#endif /* HPHP_INSTRUMENT_TYPE_INF */

  processScopesParallel<InferTypes>("InferTypes");

  for (auto scope : scopes) {
    scope->setInTypeInference(false);
    scope->clearUpdated();
    assert(scope->getMark() == BlockScope::MarkProcessed);
    assert(scope->getNumDepsToWaitFor() == 0);
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

class FinalWorker : public JobQueueWorker<MethodStatementPtr, AnalysisResult*> {
public:
  virtual void doJob(MethodStatementPtr m) {
    try {
      AliasManager am(1);
      am.finalSetup(m_context->shared_from_this(), m);
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
    }
  }
};

template<>
void AnalysisResult::preWaitCallback<Post>(
    bool first, const BlockScopeRawPtrQueue &scopes, void *context) {
  assert(!Option::ControlFlow || context != nullptr);
  if (first && Option::ControlFlow) {
    auto *dispatcher
      = (JobQueueDispatcher<FinalWorker> *) context;
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

    JobQueueDispatcher<FinalWorker> dispatcher(
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
    assert(false);
  }

  return true;
}
