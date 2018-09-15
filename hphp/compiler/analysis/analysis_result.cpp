/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <folly/portability/SysStat.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/exceptions.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/hash.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// initialization

AnalysisResult::AnalysisResult()
  : BlockScope("Root", "", StatementPtr(), BlockScope::ProgramScope),
    m_package(nullptr), m_parseOnDemand(false), m_phase(ParseAllFiles),
    m_asmCallbacks(AnalysisResultConstRawPtr{this}) {
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

void AnalysisResult::addHhasFile(std::unique_ptr<UnitEmitter>&& ue) {
  m_hhasFiles.emplace_back(std::move(ue));
}

std::vector<std::unique_ptr<UnitEmitter>> AnalysisResult::getHhasFiles() {
  return std::move(m_hhasFiles);
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
      m_package->addSourceFile(rname);
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

static bool by_filename(const FileScopePtr &f1, const FileScopePtr &f2) {
  return f1->getName() < f2->getName();
}

void AnalysisResult::analyzeProgram(ConstructPtr c) const {
  if (!c) return;
  for (auto i = 0, n = c->getKidCount(); i < n; ++i) {
    analyzeProgram(c->getNthKid(i));
  }
  c->analyzeProgram(AnalysisResultConstRawPtr{this});
}

namespace {

class AnalyzeWorker
  : public JobQueueWorker<FileScopeRawPtr, const AnalysisResult*, true, true> {
 public:
  AnalyzeWorker() {}
  void doJob(JobType job) override {
    try {
      job->analyzeProgram(AnalysisResultConstRawPtr{m_context});
    } catch (Exception& e) {
      Logger::Error(e.getMessage());
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
    }
  }
  void onThreadEnter() override {
    hphp_session_init(Treadmill::SessionKind::CompilerAnalysis);
  }
  void onThreadExit() override {
    hphp_context_exit();
    hphp_session_exit();
  }
};

}

void AnalysisResult::analyzeProgram(AnalysisResult::Phase phase) {
  {
    Timer timer(Timer::WallTime,
                phase == AnalysisResult::AnalyzeFinal ?
                "analyze final" : "analyze all");
    setPhase(phase);

    auto const nFiles = m_fileScopes.size();
    auto threadCount = Option::ParserThreadCount;
    if (threadCount > nFiles) {
      threadCount = nFiles;
    }
    if (!threadCount) threadCount = 1;
    JobQueueDispatcher<AnalyzeWorker> dispatcher(
      threadCount, threadCount, 0, false, this
    );

    dispatcher.start();
    for (auto const& file : m_fileScopes) {
      dispatcher.enqueue(file);
    }
    dispatcher.waitEmpty();
  }

  if (phase == AnalysisResult::AnalyzeAll) {
    Timer timer2(Timer::WallTime, "processImportedLambdas");
    ClosureExpression::processLambdas(AnalysisResultConstRawPtr{this},
                                      std::move(m_lambdas));
  } else {
    always_assert(m_lambdas.empty());
  }
}

void AnalysisResult::analyzeProgram() {
  AnalysisResultPtr ar = shared_from_this();

  // Analyze Includes
  Logger::Verbose("Analyzing Includes");
  sort(m_fileScopes.begin(), m_fileScopes.end(), by_filename); // fixed order

  // Analyze All
  Logger::Verbose("Analyzing All");
  analyzeProgram(AnalysisResult::AnalyzeAll);
}

void AnalysisResult::analyzeProgramFinal() {
  analyzeProgram(AnalysisResult::AnalyzeFinal);

  setPhase(AnalysisResult::CodeGen);
}

static void dumpVisitor(AnalysisResultPtr ar, StatementPtr s, void* /*data*/) {
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
