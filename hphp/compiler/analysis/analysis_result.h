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
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/package.h"
#include "hphp/compiler/hphp.h"

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
    KindOfVolatileClass,
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

  void getScopesSet(BlockScopeRawPtrQueue &v);

  void preOptimize();

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

  void addFileScope(FileScopePtr fileScope);

  /**
   * Declarations
   */
  bool declareFunction(FunctionScopePtr funcScope) const;
  bool declareClass(ClassScopePtr classScope) const;
  void declareUnknownClass(const std::string &name);
  bool declareConst(FileScopePtr fs, const std::string &name);

  ClassScopePtr findClass(const std::string &className) const;

  /**
   * Find all the redeclared classes by the name, excluding system classes.
   * Note that system classes cannot be redeclared.
   */
  const std::vector<ClassScopePtr>& findRedeclaredClasses(
    const std::string &className) const;

  /**
   * Find all the classes by the name, including system classes.
   */
  std::vector<ClassScopePtr> findClasses(const std::string &className) const;
  ClassScopePtr findExactClass(ConstructPtr cs, const std::string &name) const;
  FunctionScopePtr findFunction(const std::string &funcName) const ;
  BlockScopeConstPtr findConstantDeclarer(const std::string &constName) const {
    return const_cast<AnalysisResult*>(this)->findConstantDeclarer(constName);
  }
  BlockScopePtr findConstantDeclarer(const std::string &constName);

  bool isConstantDeclared(const std::string &constName) const;
  bool isConstantRedeclared(const std::string &constName) const;
  bool isSystemConstant(const std::string &constName) const;

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
  StringToFunctionScopePtrMap m_functionDecs;
  StringToFunctionScopePtrVecMap m_functionReDecs;
  StringToClassScopePtrVecMap m_classDecs;
  StringToFileScopePtrMap m_constDecs;
  std::set<std::string> m_constRedeclared;

  // Names of type aliases.
  std::set<std::string> m_typeAliasNames;

  std::vector<StatementPtr> m_stmts;
  StatementPtr m_stmt;

  std::string m_outputPath;
public:
  AnalysisResultPtr shared_from_this() {
    return static_pointer_cast<AnalysisResult>
      (BlockScope::shared_from_this());
  }

  AnalysisResultConstRawPtr shared_from_this() const = delete;

private:
  std::vector<BlockScopePtr> m_ignoredScopes;

  // Temporary vector of lambda expressions; populated
  // during analyzeProgram, and then processed at the end
  // of AnalysisResult::analyzeProgram.
  CompactVector<ClosureExpressionRawPtr> m_lambdas;

  /**
   * Checks whether the file is in one of the on-demand parsing directories.
   */
  bool inParseOnDemandDirs(const std::string &filename) const;

  /*
   * Find the names of all functions and classes in the program; mark
   * functions with duplicate names as redeclaring, but duplicate
   * classes aren't yet marked.  See markRedeclaringClasses.
   */
  void collectFunctionsAndClasses(FileScopePtr fs);

  /**
   * Making sure symbol orders are not different even with multithreading, so
   * to make sure generated code are consistent every time.
   */
  void canonicalizeSymbolOrder();

  /*
   * After all the class names have been collected and symbol order is
   * canonicalized, this passes through and marks duplicate class
   * names as redeclaring.
   */
  void markRedeclaringClasses();

  /**
   * Checks circular class derivations that can cause stack overflows for
   * subsequent analysis. Also checks to make sure no two redundant parents.
   */
  void checkClassDerivations();

  void resolveNSFallbackFuncs();

  int getFileSize(FileScopePtr fs);

public:
  static DECLARE_THREAD_LOCAL(BlockScopeRawPtr, s_currentScopeThreadLocal);
  static DECLARE_THREAD_LOCAL(BlockScopeRawPtrFlagsHashMap,
                              s_changedScopesMapThreadLocal);

private:
  void processScopesParallel(const char* id, void* context = nullptr);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ANALYSIS_RESULT_H_
