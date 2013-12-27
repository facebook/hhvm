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

#ifndef incl_HPHP_ANALYSIS_RESULT_H_
#define incl_HPHP_ANALYSIS_RESULT_H_

#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/package.h"

#include "hphp/util/string-bag.h"
#include "hphp/util/thread-local.h"

#include <boost/graph/adjacency_list.hpp>
#include <tbb/concurrent_hash_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(ScalarExpression);

class AnalysisResult : public BlockScope, public FunctionContainer {
public:
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

    // inferTypes
    FirstInference,

    // post-optimize
    PostOptimize,

    CodeGen,
  };

  enum FindClassBy {
    ClassName,
    MethodName,
    PropertyName
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

  class Locker {
  public:
    explicit Locker(const AnalysisResult *ar) :
        m_ar(const_cast<AnalysisResult*>(ar)),
        m_mutex(m_ar->getMutex()) {
      m_mutex.lock();
    }
    explicit Locker(AnalysisResultConstPtr ar) :
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
    AnalysisResultPtr get() const {
      return m_ar->shared_from_this();
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
  Locker lock() const { return Locker(this); }
  void setPackage(Package *package) { m_package = package;}
  void setParseOnDemand(bool v) { m_parseOnDemand = v;}
  bool isParseOnDemand() const { return m_package && m_parseOnDemand;}
  void setParseOnDemandDirs(const std::vector<std::string> &dirs) {
    assert(m_package && !m_parseOnDemand);
    m_parseOnDemandDirs = dirs;
  }

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
  void countReturnTypes(std::map<std::string, int> &counts);

  void addEntryPoint(const std::string &name);
  void addEntryPoints(const std::vector<std::string> &names);

  void addNSFallbackFunc(ConstructPtr c, FileScopePtr fs);

  void addSystemFunction(FunctionScopeRawPtr fs);
  void addSystemClass(ClassScopeRawPtr cs);
  void analyzeProgram(bool system = false);
  void analyzeIncludes();
  void analyzeProgramFinal();
  void analyzePerfectVirtuals();
  void dump();

  void docJson(const std::string &filename);
  void visitFiles(void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                  void *data);

  void getScopesSet(BlockScopeRawPtrQueue &v);

  void preOptimize();
  void inferTypes();
  void postOptimize();

  /**
   * Force all class variables to be variants, since l-val or reference
   * of dynamic properties are used.
   */
  void forceClassVariants(
      ClassScopePtr curScope,
      bool doStatic,
      bool acquireLocks = false);

  /**
   * Force specified variable of all classes to be variants.
   */
  void forceClassVariants(
      const std::string &name,
      ClassScopePtr curScope,
      bool doStatic,
      bool acquireLocks = false);

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
  void parseOnDemandBy(const std::string &name,
                       const std::map<std::string,std::string>& amap) const;
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

  /**
   * Dependencies
   */
  void link(FileScopePtr user, FileScopePtr provider);
  bool addClassDependency(FileScopePtr usingFile,
                          const std::string &className);
  bool addFunctionDependency(FileScopePtr usingFile,
                             const std::string &functionName);
  bool addIncludeDependency(FileScopePtr usingFile,
                            const std::string &includeFilename);
  bool addConstantDependency(FileScopePtr usingFile,
                             const std::string &constantName);

  ClassScopePtr findClass(const std::string &className) const;
  ClassScopePtr findClass(const std::string &className,
                          FindClassBy by);

  /*
   * Returns: whether the given name is the name of any type aliases
   * in the whole program.
   */
  bool isTypeAliasName(const std::string& name) const {
    return m_typeAliasNames.count(name);
  }

  /**
   * Find all the redeclared classes by the name, excluding system classes.
   * Note that system classes cannot be redeclared.
   */
  const ClassScopePtrVec &findRedeclaredClasses(
    const std::string &className) const;

  /**
   * Find all the classes by the name, including system classes.
   */
  ClassScopePtrVec findClasses(const std::string &className) const;
  bool classMemberExists(const std::string &name, FindClassBy by) const;
  ClassScopePtr findExactClass(ConstructPtr cs, const std::string &name) const;
  bool checkClassPresent(ConstructPtr cs, const std::string &name) const;
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

  /**
   * Literal string to String precomputation
   */
  std::string getLiteralStringName(int64_t hash, int index, bool iproxy = false);
  std::string getLitVarStringName(int64_t hash, int index, bool iproxy = false);
  int getLiteralStringId(const std::string &s, int &index);

  /**
   * Profiling runtime parameter type
   */
  std::string getFuncId(ClassScopePtr cls, FunctionScopePtr func);
  std::vector<const char *> &getFuncTableBucket(FunctionScopePtr func);

  std::set<std::string> m_variableTableFunctions;
  std::set<int> m_concatLengths;
  int m_arrayLitstrKeyMaxSize;
  int m_arrayIntegerKeyMaxSize;

  std::string getHashedName(int64_t hash, int index, const char *prefix,
                            bool longName = false);
  void addNamedLiteralVarString(const std::string &s);
  void addNamedScalarVarArray(const std::string &s);
  StringToClassScopePtrVecMap getExtensionClasses();
  void addInteger(int64_t n);

private:
  Package *m_package;
  bool m_parseOnDemand;
  std::vector<std::string> m_parseOnDemandDirs;
  std::set<std::pair<ConstructPtr, FileScopePtr> > m_nsFallbackFuncs;
  Phase m_phase;
  StringToFileScopePtrMap m_files;
  FileScopePtrVec m_fileScopes;

  StringBag m_extraCodeFileNames;
  std::map<std::string, std::string> m_extraCodes;

  StringToClassScopePtrMap m_systemClasses;
  StringToFunctionScopePtrMap m_functionDecs;
  StringToFunctionScopePtrVecMap m_functionReDecs;
  StringToClassScopePtrVecMap m_classDecs;
  StringToClassScopePtrVecMap m_methodToClassDecs;
  StringToFileScopePtrMap m_constDecs;
  std::set<std::string> m_constRedeclared;

  // Map names of class aliases to the class names they will alias.
  // Only in WholeProgram mode.  See markRedeclaringClasses.
  std::multimap<std::string,std::string> m_classAliases;

  // Names of type aliases.
  std::set<std::string> m_typeAliasNames;

  bool m_classForcedVariants[2];

  StatementPtrVec m_stmts;
  StatementPtr m_stmt;

  std::string m_outputPath;
public:
  AnalysisResultPtr shared_from_this() {
    return static_pointer_cast<AnalysisResult>
      (BlockScope::shared_from_this());
  }

  AnalysisResultConstPtr shared_from_this() const {
    return static_pointer_cast<const AnalysisResult>
      (BlockScope::shared_from_this());
  }

private:
  BlockScopePtrVec m_ignoredScopes;

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef boost::graph_traits<Graph>::adjacency_iterator adjacency_iterator;
  Mutex m_depGraphMutex;
  Graph m_depGraph;
  typedef std::map<vertex_descriptor, FileScopePtr> VertexToFileScopePtrMap;
  VertexToFileScopePtrMap m_fileVertMap;

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

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
  static std::atomic<int>                     s_NumDoJobCalls;
  static ConcurrentBlockScopeRawPtrIntHashMap s_DoJobUniqueScopes;
  static std::atomic<int>                     s_NumForceRerunGlobal;
  static std::atomic<int>                     s_NumReactivateGlobal;
  static std::atomic<int>                     s_NumForceRerunUseKinds;
  static std::atomic<int>                     s_NumReactivateUseKinds;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

private:
  template <typename Visitor>
  void processScopesParallel(const char *id, void *opaque = nullptr);

  template <typename Visitor>
  void preWaitCallback(bool first,
                       const BlockScopeRawPtrQueue &scopes,
                       void *opaque);

  template <typename Visitor>
  bool postWaitCallback(bool first,
                        bool again,
                        const BlockScopeRawPtrQueue &scopes,
                        void *opaque);
};

///////////////////////////////////////////////////////////////////////////////
// Type Inference

class RescheduleException : public Exception {
public:
  explicit RescheduleException(BlockScopeRawPtr scope) :
    Exception(), m_scope(scope) {}
  BlockScopeRawPtr &getScope() { return m_scope; }
#ifdef HPHP_INSTRUMENT_TYPE_INF
  static std::atomic<int> s_NumReschedules;
  static std::atomic<int> s_NumForceRerunSelfCaller;
  static std::atomic<int> s_NumRetTypesChanged;
#endif /* HPHP_INSTRUMENT_TYPE_INF */
private:
  BlockScopeRawPtr m_scope;
};

class SetCurrentScope {
public:
  explicit SetCurrentScope(BlockScopeRawPtr scope) {
    assert(!((*AnalysisResult::s_currentScopeThreadLocal).get()));
    *AnalysisResult::s_currentScopeThreadLocal = scope;
    scope->setInVisitScopes(true);
  }
  ~SetCurrentScope() {
    (*AnalysisResult::s_currentScopeThreadLocal)->setInVisitScopes(false);
    AnalysisResult::s_currentScopeThreadLocal.destroy();
  }
};

#define IMPLEMENT_INFER_AND_CHECK_ASSERT(scope) \
  do { \
    assert(AnalysisResult::s_currentScopeThreadLocal->get()); \
    assert(AnalysisResult::s_currentScopeThreadLocal->get() == \
           (scope).get()); \
    (scope)->getInferTypesMutex().assertOwnedBySelf(); \
  } while (0)

#ifdef HPHP_INSTRUMENT_TYPE_INF
typedef std::pair < const char *, int > LEntry;

struct LEntryHasher {
  bool equal(const LEntry &l1, const LEntry &l2) const {
    assert(l1.first);
    assert(l2.first);
    return l1.second == l2.second &&
           strcmp(l1.first, l2.first) == 0;
  }
  size_t hash(const LEntry &l) const {
    assert(l.first);
    return hash_string(l.first) ^ l.second;
  }
};

typedef tbb::concurrent_hash_map < LEntry, int, LEntryHasher >
        LProfileMap;
#endif /* HPHP_INSTRUMENT_TYPE_INF */

class BaseTryLock {
  friend class TryLock;
  friend class ConditionalTryLock;
public:
#ifdef HPHP_INSTRUMENT_TYPE_INF
  static LProfileMap s_LockProfileMap;
#endif /* HPHP_INSTRUMENT_TYPE_INF */
private:
  inline bool acquireImpl(BlockScopeRawPtr scopeToLock) {
    // A class scope can NEVER grab a lock on a function scope
    BlockScopeRawPtr current ATTRIBUTE_UNUSED =
      *(AnalysisResult::s_currentScopeThreadLocal.get());
    assert(current);
    assert(!current->is(BlockScope::ClassScope) ||
           !scopeToLock->is(BlockScope::FunctionScope));
    return m_mutex.tryLock();
  }
#ifdef HPHP_INSTRUMENT_TYPE_INF
  BaseTryLock(BlockScopeRawPtr scopeToLock,
              const char * fromFunction,
              int fromLine,
              bool lockCondition = true,
              bool profile = true)
    : m_profiler(profile),
      m_mutex(scopeToLock->getInferTypesMutex()),
      m_acquired(false) {
    if (LIKELY(lockCondition)) {
      bool success = acquireImpl(scopeToLock);
      if (UNLIKELY(!success)) {
        // put entry in profiler
        LProfileMap::accessor acc;
        LEntry key(fromFunction, fromLine);
        if (!s_LockProfileMap.insert(acc, key)) {
          // pre-existing
          acc->second++;
        } else {
          acc->second = 1;
        }
        // could not acquire lock, throw reschedule exception
        throw RescheduleException(scopeToLock);
      }
      assert(success);
      m_acquired = true;
      m_mutex.assertOwnedBySelf();
    }
  }
#else
  explicit BaseTryLock(BlockScopeRawPtr scopeToLock,
                       bool lockCondition = true,
                       bool profile = true)
    : m_profiler(profile),
      m_mutex(scopeToLock->getInferTypesMutex()),
      m_acquired(false) {
    if (LIKELY(lockCondition)) {
      bool success = acquireImpl(scopeToLock);
      if (UNLIKELY(!success)) {
        // could not acquire lock, throw reschedule exception
        throw RescheduleException(scopeToLock);
      }
      assert(success);
      m_acquired = true;
      m_mutex.assertOwnedBySelf();
    }
  }
#endif /* HPHP_INSTRUMENT_TYPE_INF */

  ~BaseTryLock() {
    if (m_acquired) m_mutex.unlock();
  }

  LockProfiler     m_profiler;
  InferTypesMutex& m_mutex;
  bool             m_acquired;
};

class TryLock : public BaseTryLock {
public:
#ifdef HPHP_INSTRUMENT_TYPE_INF
  TryLock(BlockScopeRawPtr scopeToLock,
          const char * fromFunction,
          int fromLine,
          bool profile = true) :
    BaseTryLock(scopeToLock, fromFunction, fromLine, true, profile) {}
#else
  explicit TryLock(BlockScopeRawPtr scopeToLock,
                   bool profile = true) :
    BaseTryLock(scopeToLock, true, profile) {}
#endif /* HPHP_INSTRUMENT_TYPE_INF */
};

class ConditionalTryLock : public BaseTryLock {
public:
#ifdef HPHP_INSTRUMENT_TYPE_INF
  ConditionalTryLock(BlockScopeRawPtr scopeToLock,
                     const char * fromFunction,
                     int fromLine,
                     bool condition,
                     bool profile = true) :
    BaseTryLock(scopeToLock, fromFunction, fromLine, condition, profile) {}
#else
  ConditionalTryLock(BlockScopeRawPtr scopeToLock,
                     bool condition,
                     bool profile = true) :
    BaseTryLock(scopeToLock, condition, profile) {}
#endif /* HPHP_INSTRUMENT_TYPE_INF */
};

#define GET_LOCK(scopeToLock) \
  SimpleLock _lock((scopeToLock)->getInferTypesMutex())

#define COND_GET_LOCK(scopeToLock, condition) \
  SimpleConditionalLock _clock((scopeToLock)->getInferTypesMutex(), \
                               (condition))

#define GET_LOCK_THIS() \
  SimpleLock _lock(this->getInferTypesMutex())

#define COND_GET_LOCK_THIS(condition) \
  SimpleConditionalLock _clock(this->getInferTypesMutex(), (condition))

#ifdef HPHP_INSTRUMENT_TYPE_INF
  #define TRY_LOCK(scopeToLock) \
    TryLock _tl((scopeToLock), __PRETTY_FUNCTION__, __LINE__)

  #define COND_TRY_LOCK(scopeToLock, condition) \
    ConditionalTryLock _ctl((scopeToLock), __PRETTY_FUNCTION__, \
                            __LINE__, condition)
#else
  #define TRY_LOCK(scopeToLock) \
    TryLock _tl((scopeToLock))

  #define COND_TRY_LOCK(scopeToLock, condition) \
    ConditionalTryLock _ctl((scopeToLock), (condition))
#endif /* HPHP_INSTRUMENT_TYPE_INF */

#define TRY_LOCK_THIS() \
  TRY_LOCK(BlockScopeRawPtr(this))

#define COND_TRY_LOCK_THIS(condition) \
  COND_TRY_LOCK(BlockScopeRawPtr(this), condition)

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ANALYSIS_RESULT_H_
