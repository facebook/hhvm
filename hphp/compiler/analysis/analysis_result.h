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

#ifndef incl_HPHP_ANALYSIS_RESULT_H_
#define incl_HPHP_ANALYSIS_RESULT_H_

#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/package.h"
#include "hphp/compiler/hphp.h"

#include "hphp/runtime/vm/as.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/string-bag.h"
#include "hphp/util/thread-local.h"

#include <tbb/concurrent_hash_map.h>
#include <atomic>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <functional>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ScalarExpression);
DECLARE_EXTENDED_BOOST_TYPES(ClassScope);
DECLARE_EXTENDED_BOOST_TYPES(FileScope);

struct UnitEmitter;

struct AnalysisResult : BlockScope, FunctionContainer {
  /**
   * There are multiple passes over our syntax trees. This lists all of them.
   */
  enum Phase {
    // parse
    ParseAllFiles,

    // analyzeProgram
    AnalyzeAll,
    AnalyzeFinal,

    // pre-optimize
    FirstPreOptimize,

    CodeGen,
  };

  enum GlobalSymbolType {
    KindOfStaticGlobalVariable,
    KindOfDynamicGlobalVariable,
    KindOfMethodStaticVariable,
    KindOfClassStaticVariable,
    KindOfDynamicConstant,
    KindOfPseudoMain,
    KindOfRedeclaredFunction,
    KindOfRedeclaredClass,
    KindOfRedeclaredClassId,
    KindOfLazyStaticInitializer,

    GlobalSymbolTypeCount
  };

  struct Locker {
    explicit Locker(const AnalysisResult *ar) :
        m_ar(const_cast<AnalysisResult*>(ar)),
        m_mutex(m_ar->getMutex()) {
      m_mutex.lock();
    }
    explicit Locker(AnalysisResultConstRawPtr ar) :
        m_ar(const_cast<AnalysisResult*>(ar.get())),
        m_mutex(m_ar->getMutex()) {
      m_mutex.lock();
    }
    Locker(const Locker &l) : m_ar(l.m_ar), m_mutex(l.m_mutex) {
      const_cast<Locker&>(l).m_ar = 0;
    }
    ~Locker() {
      if (m_ar) m_mutex.unlock();
    }
    AnalysisResultRawPtr get() const {
      return AnalysisResultRawPtr{m_ar};
    }
    AnalysisResult *operator->() const {
      return m_ar;
    }
  private:
    AnalysisResult *m_ar;
    Mutex &m_mutex;
  };

  struct ParseOnDemandCalbacks : AsmCallbacks {
    explicit ParseOnDemandCalbacks(AnalysisResultConstRawPtr ar) : m_ar(ar) {}

    virtual void onInclude(const std::string& include) override {
      m_ar->parseOnDemand(include);
    }
    virtual void onConstantRef(const std::string& constant) override {
      m_ar->parseOnDemandByConstant(constant);
    }
    virtual void onFunctionRef(const std::string& function) override {
      m_ar->parseOnDemandByFunction(function);
    }
    virtual void onClassRef(const std::string& cls) override {
      m_ar->parseOnDemandByClass(cls);
    }

   private:
    AnalysisResultConstRawPtr m_ar;
  };

public:
  AnalysisResult();
  ~AnalysisResult() override;
  Locker lock() const { return Locker(this); }
  void setPackage(Package *package) { m_package = package;}
  void setParseOnDemand(bool v) { m_parseOnDemand = v;}
  bool isParseOnDemand() const { return m_package && m_parseOnDemand;}
  void setParseOnDemandDirs(const std::vector<std::string> &dirs) {
    assert(m_package && !m_parseOnDemand);
    m_parseOnDemandDirs = dirs;
  }
  void setFinish(std::function<void(AnalysisResultPtr)>&& fn) {
    m_finish = std::move(fn);
  }
  void finish();

  Mutex &getMutex() { return m_mutex; }

  /**
   * create_function() generates extra PHP code that defines the lambda.
   * Stores the code in a temporary string, so we can parse this as an
   * extra file appended to parsed code.
   */
  void appendExtraCode(const std::string &key, const std::string &code);
  void appendExtraCode(const std::string &key, const std::string &code) const;
  void parseExtraCode(const std::string &key);

  Phase getPhase() const { return m_phase;}
  void setPhase(Phase phase) { m_phase = phase;}

  int getFunctionCount() const;
  int getClassCount() const;

  void addEntryPoint(const std::string &name);
  void addEntryPoints(const std::vector<std::string> &names);

  void addNSFallbackFunc(ConstructPtr c, FileScopePtr fs);

  void addSystemFunction(FunctionScopeRawPtr fs);
  void addSystemClass(ClassScopeRawPtr cs);
  void analyzeProgram(ConstructPtr) const;
  void analyzeProgram(Phase);
  void analyzeProgram();
  void analyzeProgramFinal();
  void dump();


  void addClonedLambda(ClosureExpressionRawPtr c) { m_lambdas.push_back(c); }

  void visitFiles(void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                  void *data);

  /**
   * Code generation functions.
   */
  bool outputAllPHP(CodeGenerator::Output output);

  /**
   * Parser creates a FileScope upon parsing a new file.
   */
  void parseOnDemand(const std::string &name) const;
  void parseOnDemandByClass(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadClassMap);
  }
  void parseOnDemandByFunction(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadFuncMap);
  }
  void parseOnDemandByConstant(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadConstMap);
  }
  template <class Map>
  void parseOnDemandBy(const std::string &name,
                       const Map& amap) const;
  FileScopePtr findFileScope(const std::string &name) const;
  const StringToFileScopePtrMap &getAllFiles() { return m_files;}
  const std::vector<FileScopePtr> &getAllFilesVector() {
    return m_fileScopes;
  }
  ParseOnDemandCalbacks* getParseOnDemandCallBacks() {
    if (isParseOnDemand()) {
      return &m_asmCallbacks;
    }

    return nullptr;
  }

  void addFileScope(FileScopePtr fileScope);

  /**
   * For function declaration parsing.
   */
  static std::string prepareFile(const char *root, const std::string &fileName,
                                 bool chop, bool stripPath = true);

  void setOutputPath(const std::string &path) {
    m_outputPath = path;
  }
  const std::string &getOutputPath() {
    return m_outputPath;
  }

  void addHhasFile(std::unique_ptr<UnitEmitter>&& ue);
  std::vector<std::unique_ptr<UnitEmitter>> getHhasFiles();
private:
  std::function<void(AnalysisResultPtr)> m_finish;
  Package *m_package;
  bool m_parseOnDemand;
  std::vector<std::string> m_parseOnDemandDirs;
  std::set<std::pair<ConstructPtr, FileScopePtr> > m_nsFallbackFuncs;
  Phase m_phase;
  StringToFileScopePtrMap m_files;
  std::vector<FileScopePtr> m_fileScopes;
  std::vector<std::unique_ptr<UnitEmitter>> m_hhasFiles;

  StringBag m_extraCodeFileNames;
  std::map<std::string, std::string> m_extraCodes;

  StringToClassScopePtrMap m_systemClasses;

  std::vector<StatementPtr> m_stmts;
  StatementPtr m_stmt;

  std::string m_outputPath;

  ParseOnDemandCalbacks m_asmCallbacks;
public:
  AnalysisResultPtr shared_from_this() {
    return static_pointer_cast<AnalysisResult>
      (BlockScope::shared_from_this());
  }

  AnalysisResultConstRawPtr shared_from_this() const = delete;

private:
  Mutex m_mutex;

  // Temporary vector of lambda expressions; populated
  // during analyzeProgram, and then processed at the end
  // of AnalysisResult::analyzeProgram.
  CompactVector<ClosureExpressionRawPtr> m_lambdas;

  /**
   * Checks whether the file is in one of the on-demand parsing directories.
   */
  bool inParseOnDemandDirs(const std::string &filename) const;

  int getFileSize(FileScopePtr fs);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ANALYSIS_RESULT_H_
