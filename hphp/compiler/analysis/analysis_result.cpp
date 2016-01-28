/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Conv.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "hphp/compiler/analysis/exceptions.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/class_require_statement.h"
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
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/util/logger.h"
#include "hphp/util/hash.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/timer.h"

using namespace HPHP;

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
}

AnalysisResult::~AnalysisResult() {
  always_assert(!m_finish);
}

void AnalysisResult::finish() {
  if (m_finish) {
    decltype(m_finish) f;
    f.swap(m_finish);
    f(shared_from_this());
  }
}

void AnalysisResult::appendExtraCode(const std::string &key,
                                     const std::string &code) {
  auto& extraCode = m_extraCodes[key];

  if (extraCode.empty()) {
    extraCode = "<?php\n";
  }
  extraCode += code + "\n";
}

void AnalysisResult::appendExtraCode(const std::string &key,
                                     const std::string &code) const {
  lock()->appendExtraCode(key, code);
}

void AnalysisResult::parseExtraCode(const std::string &key) {
  Lock lock(getMutex());
  auto iter = m_extraCodes.find(key);
  if (iter != m_extraCodes.end()) {
    auto const code = iter->second;
    auto const sfilename = iter->first + "." + Option::LambdaPrefix + "lambda";
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
  m_fileScopes.push_back(fileScope);
}

bool AnalysisResult::inParseOnDemandDirs(const std::string &filename) const {
  for (size_t i = 0; i < m_parseOnDemandDirs.size(); i++) {
    if (filename.find(m_parseOnDemandDirs[i]) == 0) return true;
  }
  return false;
}

void AnalysisResult::parseOnDemand(const std::string &name) const {
  if (m_package) {
    auto const& root = m_package->getRoot();
    auto rname = name;
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

template <class Map>
void AnalysisResult::parseOnDemandBy(const std::string &name,
                                     const Map &amap) const {
  if (m_package) {
    auto it = amap.find(name);
    if (it != amap.end()) {
      parseOnDemand(Option::AutoloadRoot + it->second);
    }
  }
}

template void AnalysisResult::parseOnDemandBy(
  const std::string &name, const std::map<std::string,std::string> &amap) const;

template void AnalysisResult::parseOnDemandBy(
  const std::string &name,
  const std::map<std::string,std::string,stdltistr> &amap) const;

void AnalysisResult::addNSFallbackFunc(ConstructPtr c, FileScopePtr fs) {
  m_nsFallbackFuncs.insert(std::make_pair(c, fs));
}

FileScopePtr AnalysisResult::findFileScope(const std::string &name) const {
  auto iter = m_files.find(name);
  if (iter != m_files.end()) {
    return iter->second;
  }
  return FileScopePtr();
}

FunctionScopePtr AnalysisResult::findFunction(
  const std::string &funcName) const {
  auto bit = m_functions.find(funcName);
  if (bit != m_functions.end() && !bit->second->allowOverride()) {
    return bit->second;
  }
  auto iter = m_functionDecs.find(funcName);
  if (iter != m_functionDecs.end()) {
    return iter->second;
  }
  return bit != m_functions.end() ? bit->second : FunctionScopePtr();
}

BlockScopePtr AnalysisResult::findConstantDeclarer(
  const std::string &name) {
  if (getConstants()->isPresent(name)) return shared_from_this();
  auto iter = m_constDecs.find(name);
  if (iter != m_constDecs.end()) return iter->second;
  return BlockScopePtr();
}

ClassScopePtr AnalysisResult::findClass(const std::string &name) const {
  AnalysisResultConstPtr ar = shared_from_this();
  auto const lname = toLower(name);
  auto const sysIter = m_systemClasses.find(lname);
  if (sysIter != m_systemClasses.end()) return sysIter->second;

  auto const iter = m_classDecs.find(lname);
  if (iter != m_classDecs.end() && iter->second.size()) {
    return iter->second.back();
  }
  return ClassScopePtr();
}

const std::vector<ClassScopePtr>&
AnalysisResult::findRedeclaredClasses(const std::string &name) const {
  auto iter = m_classDecs.find(name);
  if (iter == m_classDecs.end()) {
    static std::vector<ClassScopePtr> empty;
    empty.clear();
    return empty;
  }
  return iter->second;
}

std::vector<ClassScopePtr> AnalysisResult::findClasses(
  const std::string &name
) const {
  auto const sysIter = m_systemClasses.find(name);
  if (sysIter != m_systemClasses.end()) {
    return {sysIter->second};
  }

  return findRedeclaredClasses(name);
}

ClassScopePtr AnalysisResult::findExactClass(ConstructPtr cs,
                                             const std::string &name) const {
  ClassScopePtr cls = findClass(name);
  if (!cls || !cls->isRedeclaring()) return cls;
  if (ClassScopePtr currentCls = cs->getClassScope()) {
    if (cls->isNamed(currentCls->getScopeName())) {
      return currentCls;
    }
  }
  return ClassScopePtr();
}

int AnalysisResult::getFunctionCount() const {
  int total = 0;
  for (auto& pair : m_files) {
    total += pair.second->getFunctionCount();
  }
  return total;
}

int AnalysisResult::getClassCount() const {
  int total = 0;
  for (auto& pair : m_files) {
    total += pair.second->getClassCount();
  }
  return total;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool AnalysisResult::declareFunction(FunctionScopePtr funcScope) const {
  assert(m_phase < AnalyzeAll);

  // System functions override
  auto it = m_functions.find(funcScope->getScopeName());
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

  // System classes override
  if (m_systemClasses.count(classScope->getScopeName())) {
    // we need someone to hold on to a reference to it
    // even though we're not going to do anything with it
    this->lock()->m_ignoredScopes.push_back(classScope);
    return false;
  }

  return true;
}

void AnalysisResult::declareUnknownClass(const std::string &name) {
  m_classDecs.operator[](name);
}

bool AnalysisResult::declareConst(FileScopePtr fs, const std::string &name) {
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
  if (auto d = b1->getStmt()->getRange().compare(b2->getStmt()->getRange())) {
    return d < 0;
  }
  return b1->getContainingFile()->getName() <
    b2->getContainingFile()->getName();
}

void AnalysisResult::canonicalizeSymbolOrder() {
  getConstants()->canonicalizeSymbolOrder();
  getVariables()->canonicalizeSymbolOrder();
}

void AnalysisResult::markRedeclaringClasses() {
  AnalysisResultPtr ar = shared_from_this();
  for (auto& pair : m_classDecs) {
    auto& classes = pair.second;
    if (classes.size() > 1) {
      sort(classes.begin(), classes.end(), by_source);
      for (size_t i = 0; i < classes.size(); i++) {
        classes[i]->setRedeclaring(ar, i);
      }
    }
  }

  auto markRedeclaring = [&] (const std::string& name) {
    auto it = m_classDecs.find(name);
    if (it != m_classDecs.end()) {
      auto& classes = it->second;
      for (size_t i = 0; i < classes.size(); ++i) {
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
    assert(kv.first == toLower(kv.first));
    assert(kv.second == toLower(kv.second));
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
    assert(toLower(name) == name);
    markRedeclaring(name);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dependencies

bool AnalysisResult::isConstantDeclared(const std::string &constName) const {
  if (m_constants->isPresent(constName)) return true;
  auto const iter = m_constDecs.find(constName);
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
  FunctionScopePtr& entry = m_functions[fs->getScopeName()];
  assert(!entry);
  entry = fs;
}

void AnalysisResult::addSystemClass(ClassScopeRawPtr cs) {
  ClassScopePtr& entry = m_systemClasses[cs->getScopeName()];
  assert(!entry);
  entry = cs;
}

void AnalysisResult::checkClassDerivations() {
  AnalysisResultPtr ar = shared_from_this();
  for (auto& pair : m_classDecs) {
    for (ClassScopePtr cls : pair.second) {
      if (Option::WholeProgram) {
        try {
          cls->importUsedTraits(ar);
        } catch (const AnalysisTimeFatalException& e) {
          cls->setFatal(e);
        }
      }
      hphp_string_iset seen;
      cls->checkDerivation(ar, seen);
    }
  }
}

void AnalysisResult::resolveNSFallbackFuncs() {
  for (auto &pair : m_nsFallbackFuncs) {
    auto sfc = static_pointer_cast<SimpleFunctionCall>(pair.first);
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
          auto& funcVec = m_functionReDecs[iter.first];
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

  if (const auto redec = fs->getRedecFunctions()) {
    for (const auto &iter : *redec) {
      auto i = iter.second.begin();
      auto e = iter.second.end();
      auto& funcDec = m_functionDecs[iter.first];
      assert(funcDec); // because the first one was in funcs above
      auto& funcVec = m_functionReDecs[iter.first];
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
    auto& clsVec = m_classDecs[iter.first];
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
  for (auto& cls_name : Option::VolatileClasses) {
    ClassScopePtr cls = findClass(toLower(cls_name));
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
  std::vector<ClassScopePtr> classes;
  classes.reserve(m_classDecs.size());
  for (auto& pair : m_classDecs) {
    for (auto cls : pair.second) {
      classes.push_back(cls);
    }
  }

  // Collect methods
  for (auto cls : classes) {
    StringToFunctionScopePtrMap methods;
    cls->collectMethods(ar, methods, true /* include privates */);
    bool needAbstractMethodImpl =
      (!cls->isAbstract() && !cls->isInterface() &&
       cls->derivesFromRedeclaring() == Derivation::Normal &&
       !cls->getAttribute(ClassScope::UsesUnknownTrait));
    for (auto& pair : methods) {
      auto func = pair.second;
      if (Option::WholeProgram && !func->hasImpl() && needAbstractMethodImpl) {
        auto tmpFunc = cls->findFunction(ar, func->getScopeName(), true, true);
        always_assert(!tmpFunc || !tmpFunc->hasImpl());
        Compiler::Error(Compiler::MissingAbstractMethodImpl,
                        func->getStmt(), cls->getStmt());
      }
    }
  }

  for (auto& item : m_systemClasses) {
    StringToFunctionScopePtrMap methods;
    item.second->collectMethods(ar, methods, true /* include privates */);
  }
}

void AnalysisResult::analyzeProgramFinal() {
  AnalysisResultPtr ar = shared_from_this();
  setPhase(AnalysisResult::AnalyzeFinal);
  for (size_t i = 0; i < m_fileScopes.size(); i++) {
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

void AnalysisResult::visitFiles(void (*cb)(AnalysisResultPtr,
                                           StatementPtr, void*), void *data) {
  AnalysisResultPtr ar = shared_from_this();
  for (auto& pair : m_files) {
    pair.second->visit(ar, cb, data);
  }
}

void AnalysisResult::getScopesSet(BlockScopeRawPtrQueue &v) {
  for (auto& pair: m_files) {
    pair.second->getScopesSet(v);
  }
}

///////////////////////////////////////////////////////////////////////////////
// optimization functions

constexpr int kPhaseInterestMask = BlockScope::UseKindAny;

namespace {
////////////////////////////////////////////////////////////////////////////////

struct OptWorker : JobQueueWorker<BlockScope*, void*, true, true> {
  explicit OptWorker() {}

  void onThreadEnter() override {
    hphp_session_init();
    hphp_context_init();
  }

  void onThreadExit() override {
    hphp_context_exit();
    hphp_session_exit();
  }

  void doJob(BlockScope*) override;
};

////////////////////////////////////////////////////////////////////////////////

struct OptVisitor {
  OptVisitor(AnalysisResultPtr ar, unsigned nscope)
    : m_ar(ar)
    , m_nscope(nscope)
    , m_dispatcher(nullptr)
  {}

  OptVisitor(OptVisitor&& other) noexcept
    : m_ar(other.m_ar)
    , m_nscope(other.m_nscope)
    , m_dispatcher(other.m_dispatcher)
  {
    other.m_dispatcher = nullptr;
  }

  OptVisitor& operator=(OptVisitor&&) = delete;

  OptVisitor(const OptVisitor&) = delete;
  OptVisitor& operator=(const OptVisitor&) = delete;

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
  JobQueueDispatcher<OptWorker>* m_dispatcher;
};

////////////////////////////////////////////////////////////////////////////////

struct DepthFirstVisitor {
  explicit DepthFirstVisitor(OptVisitor&& d) : m_data(std::move(d)) {
    setup();
  }

  void setup() {
    auto threadCount = Option::ParserThreadCount;
    if (threadCount > m_data.m_nscope) {
      threadCount = m_data.m_nscope;
    }
    if (threadCount <= 0) threadCount = 1;
    m_data.m_dispatcher = new JobQueueDispatcher<OptWorker>(
      threadCount, true, 0, false, this
    );
  }

  ExpressionPtr visitExprRecur(ExpressionPtr e) {
    for (int i = 0, n = e->getKidCount(); i < n; i++) {
      if (auto kid = e->getNthExpr(i)) {
        auto rep = visitExprRecur(kid);
        if (rep) {
          e->getScope()->addUpdates(BlockScope::UseKindCaller);
          e->setNthKid(i, rep);
        }
      }
    }
    return this->visit(e);
  }

  StatementPtr visitStmtRecur(StatementPtr stmt) {
    auto scope = dynamic_pointer_cast<LoopStatement>(stmt) ?
      stmt->getScope() : BlockScopeRawPtr();

    for (int i = 0, n = stmt->getKidCount(); i < n; i++) {
      if (auto kid = stmt->getNthKid(i)) {
        if (auto s = dynamic_pointer_cast<Statement>(kid)) {
          if (Construct::SkipRecurse(s)) continue;
          if (scope) scope->incLoopNestedLevel();
          if (auto rep = visitStmtRecur(s)) {
            stmt->setNthKid(i, rep);
            stmt->getScope()->addUpdates(BlockScope::UseKindCaller);
          }
          if (scope) scope->decLoopNestedLevel();
        } else {
          auto e = dynamic_pointer_cast<Expression>(kid);
          if (auto rep = visitExprRecur(e)) {
            stmt->setNthKid(i, rep);
            stmt->getScope()->addUpdates(BlockScope::UseKindCaller);
          }
        }
      }
    }

    return this->visit(stmt);
  }

  int visitScope(BlockScopeRawPtr scope) {
    int all_updates = 0;
    int updates;
    auto stmt = scope->getStmt();
    if (auto m = dynamic_pointer_cast<MethodStatement>(stmt)) {
      do {
        scope->clearUpdated();
        StatementPtr rep = this->visitStmtRecur(stmt);
        always_assert(!rep);
        updates = scope->getUpdated();
        all_updates |= updates;
      } while (updates);
      if (all_updates & BlockScope::UseKindCaller) {
        all_updates &= ~BlockScope::UseKindCaller;
      }
      return all_updates;
    }

    do {
      scope->clearUpdated();
      auto rep = this->visitStmtRecur(stmt);
      always_assert(!rep);
      updates = scope->getUpdated();
      all_updates |= updates;
    } while (updates);

    return all_updates;
  }

  bool activateScope(BlockScopeRawPtr scope) {
    if (scope->getMark() == BlockScope::MarkProcessed) {
      for (const auto& pf : scope->getOrderedUsers()) {
        if (pf->second & kPhaseInterestMask) {
          auto m = pf->first->getMark();
          if (m == BlockScope::MarkWaiting ||
              m == BlockScope::MarkReady) {
            pf->first->incNumDepsToWaitFor();
          }
        }
      }
    }

    int numDeps = 0;
    for (const auto& p : scope->getDeps()) {
      if (*p.second & kPhaseInterestMask) {
        auto m = p.first->getMark();
        if (m == BlockScope::MarkWaiting ||
            m == BlockScope::MarkReady ||
            m == BlockScope::MarkProcessing) {
          numDeps++;
        }
      }
    }

    scope->setNumDepsToWaitFor(numDeps);
    scope->setMark(BlockScope::MarkWaiting);
    if (!numDeps) {
      scope->setMark(BlockScope::MarkReady);
      return true;
    }
    return false;
  }

  void collectOrdering(BlockScopeRawPtrQueue& queue, BlockScopeRawPtr scope) {
    assert(scope->getMark() != BlockScope::MarkProcessingDeps);
    scope->setMark(BlockScope::MarkProcessingDeps);
    for (const auto& p : scope->getDeps()) {
      if (*p.second & kPhaseInterestMask) {
        if (p.first->getMark() == BlockScope::MarkWaitingInQueue) {
          collectOrdering(queue, p.first);
        }
      }
    }
    queue.push_back(scope);
  }

  /**
   * Assumes that scopes contains no duplicate entries
   */
  bool visitParallel(const BlockScopeRawPtrQueue& scopes,
                     bool first,
                     BlockScopeRawPtrQueue& enqueued) {
    bool ret = false;
    int numSetMarks = 0;
    enqueued.clear();

    for (auto& scope : scopes) {
      if (!first) {
        if (scope->getMark() != BlockScope::MarkWaiting) {
          always_assert(scope->getMark() == BlockScope::MarkProcessed);
          continue;
        }
      } else {
        assert(scope->getNumDepsToWaitFor() == 0);
      }
      scope->setMark(BlockScope::MarkWaitingInQueue);
      numSetMarks++;
      ret = true;
    }

    BlockScopeRawPtrQueue buffer;
    for (auto& scope : scopes) {
      if (scope->getMark() == BlockScope::MarkWaitingInQueue) {
        collectOrdering(buffer, scope);
      }
    }
    assert((int)buffer.size() == numSetMarks);

    {
      Lock l1(BlockScope::s_depsMutex);
      Lock l2(BlockScope::s_jobStateMutex);
      for (auto& scope : buffer) {
        if (activateScope(scope)) {
          enqueue(scope);
          enqueued.push_back(scope);
        }
      }
    }

    // assert that we will make some progress in this iteration
    assert(!ret || !enqueued.empty());
    return ret;
  }

  ExpressionPtr visit(ExpressionPtr e) {
    return e->preOptimize(m_data.m_ar);
  }

  StatementPtr visit(StatementPtr stmt) {
    return stmt->preOptimize(m_data.m_ar);
  }

  int visit(BlockScopeRawPtr scope) {
    return 0;
  }

  void enqueue(BlockScopeRawPtr scope) {
    assert(scope->getMark() == BlockScope::MarkReady);
    m_data.m_dispatcher->enqueue(scope.get());
  }

  OptVisitor& data() {
    return m_data;
  }

private:
  OptVisitor m_data;
};

////////////////////////////////////////////////////////////////////////////////

void OptWorker::doJob(BlockScope* scope) {
  try {
    auto visitor = (DepthFirstVisitor*)m_context;
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

    // creates on demand
    AnalysisResult::s_changedScopesMapThreadLocal->clear();
    auto useKinds = visitor->visitScope(BlockScopeRawPtr(scope));
    assert(useKinds >= 0);

    {
      Lock l2(BlockScope::s_depsMutex);
      Lock l1(BlockScope::s_jobStateMutex);

      assert(scope->getMark() == BlockScope::MarkProcessing);
      assert(scope->getNumDepsToWaitFor() == 0);

      // re-enqueue changed scopes, regardless of rescheduling exception.
      // this is because we might have made changes to other scopes which we
      // do not undo, so we need to announce their updates
      for (const auto& local : *AnalysisResult::s_changedScopesMapThreadLocal) {
        for (const auto& pf : local.first->getOrderedUsers()) {
          if ((pf->second & kPhaseInterestMask) &&
              (pf->second & local.second)) {
            auto m = pf->first->getMark();
            switch (m) {
              case BlockScope::MarkWaiting:
              case BlockScope::MarkReady:
                ; // no-op
                break;
              case BlockScope::MarkProcessing:
                pf->first->setForceRerun(true);
                break;
              case BlockScope::MarkProcessed:
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

      useKinds |= scope->rescheduleFlags();
      scope->setRescheduleFlags(0);

      for (const auto& pf : scope->getOrderedUsers()) {
        if (pf->second & kPhaseInterestMask) {
          auto m = pf->first->getMark();
          if (pf->second & useKinds && m == BlockScope::MarkProcessed) {
            auto ready = visitor->activateScope(pf->first);
            always_assert(!ready);
            m = BlockScope::MarkWaiting;
          }

          if (m == BlockScope::MarkWaiting || m == BlockScope::MarkReady) {
            auto nd = pf->first->getNumDepsToWaitFor();
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
        for (const auto& p : scope->getDeps()) {
          if (*p.second & kPhaseInterestMask) {
            if (p.first->getMark() == BlockScope::MarkProcessing) {
              auto ready = visitor->activateScope(BlockScopeRawPtr(scope));
              always_assert(!ready);
              break;
            }
          }
        }
      }
    }
  } catch (Exception &e) {
    Logger::Error("%s", e.getMessage().c_str());
  }
}

////////////////////////////////////////////////////////////////////////////////
}

void AnalysisResult::processScopesParallel(const char *id,
                                           void* context /* = NULL */) {
  BlockScopeRawPtrQueue scopes;
  getScopesSet(scopes);

  DepthFirstVisitor dfv(OptVisitor(shared_from_this(), scopes.size()));

  bool first = true;
  bool again;
  dfv.data().start();
  do {
    BlockScopeRawPtrQueue enqueued;
    again = dfv.visitParallel(scopes, first, enqueued);

    dfv.data().wait();

    assert(!dfv.data().getQueuedJobs());
    assert(!dfv.data().getActiveWorker());

    first = false;
  } while (again);
  dfv.data().stop();

  if (debug) {
    for (DEBUG_ONLY auto& scope : scopes) {
      assert(scope->getMark() == BlockScope::MarkProcessed);
      assert(scope->getNumDepsToWaitFor() == 0);
      assert(scope->rescheduleFlags() == 0);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// pre-opt


void AnalysisResult::preOptimize() {
  setPhase(FirstPreOptimize);
  processScopesParallel("PreOptimize");
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// code generation functions

std::string AnalysisResult::prepareFile(const char *root,
                                        const std::string &fileName,
                                        bool chop,
                                        bool stripPath /* = true */) {
  std::string fullPath = root;
  if (!fullPath.empty() &&
    !FileUtil::isDirSeparator(fullPath[fullPath.size() - 1])) {
    fullPath += FileUtil::getDirSeparator();
  }

  auto file = fileName;
  if (stripPath) {
    size_t npos = file.rfind('/');
    if (npos != std::string::npos) {
      file = file.substr(npos + 1);
    }
  }

  if (chop && file.size() > 4 && file.substr(file.length() - 4) == ".php") {
    fullPath += file.substr(0, file.length() - 4);
  } else {
    fullPath += file;
  }
  for (size_t pos = strlen(root); pos < fullPath.size(); pos++) {
    if (FileUtil::isDirSeparator(fullPath[pos])) {
      mkdir(fullPath.substr(0, pos).c_str(), 0777);
    }
  }
  return fullPath;
}

bool AnalysisResult::outputAllPHP(CodeGenerator::Output output) {
  AnalysisResultPtr ar = shared_from_this();
  switch (output) {
  case CodeGenerator::PickledPHP:
    for (auto& pair : m_files) {
      auto fullPath = prepareFile(m_outputPath.c_str(), pair.first, false);
      std::ofstream f(fullPath.c_str());
      if (f) {
        CodeGenerator cg(&f, output);
        cg_printf("<?php\n");
        Logger::Info("Generating %s...", fullPath.c_str());
        pair.second->getStmt()->outputPHP(cg, ar);
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
