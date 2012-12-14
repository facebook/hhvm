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

#ifndef __ANALYSIS_RESULT_H__
#define __ANALYSIS_RESULT_H__

#include <compiler/code_generator.h>
#include <compiler/analysis/code_error.h>
#include <compiler/option.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/symbol_table.h>
#include <compiler/analysis/function_container.h>
#include <compiler/package.h>

#include <util/string_bag.h>
#include <util/thread_local.h>

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
    Locker(const AnalysisResult *ar) :
        m_ar(const_cast<AnalysisResult*>(ar)),
        m_mutex(m_ar->getMutex()) {
      m_mutex.lock();
    }
    Locker(AnalysisResultConstPtr ar) :
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
    ASSERT(m_package && !m_parseOnDemand);
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

  void loadBuiltinFunctions();
  void loadBuiltins();
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
   * Scalar array handling.
   */
  int registerScalarArray(bool insideScalarArray, FileScopePtr scope,
                          ExpressionPtr pairs, int &hash, int &index,
                          std::string &text);
  int checkScalarArray(const std::string &text, int &index);
  int getScalarArrayId(const std::string &text);
  void outputCPPNamedScalarArrays(const std::string &file);

  std::string getScalarArrayCompressedText();
  std::string getScalarArrayName(int hash, int index);
  std::string getScalarVarArrayName(int hash, int index);

  int checkScalarVarInteger(int64 val, int &index);
  std::string getScalarVarIntegerName(int hash, int index);
  void outputCPPNamedScalarVarIntegers(const std::string &file);

  int checkScalarVarDouble(double dval, int &index);
  std::string getScalarVarDoubleName(int hash, int index);
  void outputCPPNamedScalarVarDoubles(const std::string &file);

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
  void outputAllCPP(CodeGenerator::Output output, int clusterCount,
                    const std::string *compileDir);

  void outputCPPSystemImplementations(CodeGenerator &cg);
  void getCPPFileRunDecls(CodeGenerator &cg, Type2SymbolSetMap &type2names);
  void getCPPRedeclaredFunctionDecl(CodeGenerator &cg,
                                    Type2SymbolSetMap &type2names);
  void getCPPRedeclaredClassDecl(CodeGenerator &cg,
                                 Type2SymbolSetMap &type2names);
  void outputCPPRedeclaredClassImpl(CodeGenerator &cg);
  void getCPPDynamicConstantDecl(CodeGenerator &cg,
                                 Type2SymbolSetMap &type2names);
  void outputCPPDynamicConstantImpl(CodeGenerator &cg);
  void outputCPPScalarArrayDecl(CodeGenerator &cg);
  void outputCPPScalarArrayImpl(CodeGenerator &cg);
  void outputCPPScalarArrayInit(CodeGenerator &cg, int fileCount, int part);
  void outputCPPScalarArrayId(CodeGenerator &cg, int id, int hash, int index,
                              bool scalarVariant = false);
  void getCPPClassStaticInitializerFlags(CodeGenerator &cg,
                                         Type2SymbolSetMap &type2names);
  void getCPPClassDeclaredFlags(CodeGenerator &cg,
                                Type2SymbolSetMap &type2names);

  void outputCPPFiniteDouble(CodeGenerator &cg, double dval);

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
   * PHP source info functions.
   */
  void recordSourceInfo(const std::string &file, int line, LocationPtr loc);
  void recordClassSource(const std::string &clsname, LocationPtr loc,
                         const std::string &filename);
  void recordFunctionSource(const std::string &funcname, LocationPtr loc,
                            const std::string &filename);

  /**
   * Literal string to String precomputation
   */
  std::string getLiteralStringName(int64 hash, int index, bool iproxy = false);
  std::string getLitVarStringName(int64 hash, int index, bool iproxy = false);
  int getLiteralStringId(const std::string &s, int &index);

  /**
   * Profiling runtime parameter type
   */
  std::string getFuncId(ClassScopePtr cls, FunctionScopePtr func);
  std::string getParamRTTIEntryKey(ClassScopePtr cls,
                                   FunctionScopePtr func,
                                   const std::string &paramName);
  void addParamRTTIEntry(ClassScopePtr cls,
                         FunctionScopePtr func,
                         const std::string &paramName);
  int getParamRTTIEntryId(ClassScopePtr cls,
                          FunctionScopePtr func,
                          const std::string &paramName);
  void addRTTIFunction(const std::string &id);
  void cloneRTTIFuncs(const char *RTTIDirectory);

  std::vector<const char *> &getFuncTableBucket(FunctionScopePtr func);

  /**
   * Generate default implementation for separable extension classes.
   */
  void outputCPPSepExtensionImpl(const std::string &filename);

  void outputCPPClusterImpl(CodeGenerator &cg, const FileScopePtrVec &files);
  void outputCPPFileImpl(CodeGenerator &cg, FileScopePtr fs);

  void addPregeneratedCPP(const std::string &name, std::string &code);
  const std::string &getPregeneratedCPP(const std::string &name);

  std::set<std::string> m_variableTableFunctions;
  std::set<int> m_concatLengths;
  int m_arrayLitstrKeyMaxSize;
  int m_arrayIntegerKeyMaxSize;

  void setSystem() { m_system = true; }
  bool isSystem() const { return m_system; }

  void setSepExtension() { m_sepExtension = true; }
  bool isSepExtension() { return m_sepExtension; }

  std::string getHashedName(int64 hash, int index, const char *prefix,
                            bool longName = false);
  void addNamedLiteralVarString(const std::string &s);
  void addNamedScalarVarArray(const std::string &s);
  StringToClassScopePtrVecMap getExtensionClasses();
  void addInteger(int64 n);
private:
  Package *m_package;
  bool m_parseOnDemand;
  std::vector<std::string> m_parseOnDemandDirs;
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

  bool m_classForcedVariants[2];

  StatementPtrVec m_stmts;
  StatementPtr m_stmt;

  std::string m_outputPath;

  std::map<std::string, int> m_scalarArrays;
  Mutex m_namedScalarArraysMutex;
  std::map<int, std::vector<std::string> > m_namedScalarArrays;
  std::set<std::string> m_namedScalarVarArrays;
  int m_scalarArraysCounter;
  std::vector<ExpressionPtr> m_scalarArrayIds;

  Mutex m_namedScalarVarIntegersMutex;
  std::map<int, std::vector<std::string> > m_namedScalarVarIntegers;
  Mutex m_allIntegersMutex;
  std::set<int64> m_allIntegers;

  Mutex m_namedScalarVarDoublesMutex;
  std::map<int, std::vector<std::string> > m_namedScalarVarDoubles;

  std::map<std::string, int> m_paramRTTIs;
  std::set<std::string> m_rttiFuncs;
  int m_paramRTTICounter;

public:
  struct ScalarArrayExp {
    int id;
    int len;
    ExpressionPtr exp;
  };

  void outputCPPDynamicTables(CodeGenerator::Output output);
  void outputCPPClassMapFile(CodeGenerator::Output output);
  void outputCPPSourceInfos();
  void outputCPPNameMaps();
  void outputRTTIMetaData(const char *filename);
  void outputCPPClassMap(CodeGenerator &cg, CodeGenerator::Output);
  void outputCPPSystem();
  void outputCPPSepExtensionMake();
  void outputFFI(std::vector<std::string> &additionalCPPs);
  void repartitionLargeCPP(const std::vector<std::string> &filenames,
                           const std::vector<std::string> &additionals);

  void outputCPPUtilDecl(CodeGenerator::Output output);
  void outputCPPUtilImpl(CodeGenerator::Output output);
  void outputCPPGlobalDeclarations();
  void outputCPPMain();
  void outputCPPScalarArrays(bool system);
  void outputCPPGlobalVariablesMethods();
  void outputCPPGlobalState();

  AnalysisResultPtr shared_from_this() {
    return boost::static_pointer_cast<AnalysisResult>
      (BlockScope::shared_from_this());
  }

  AnalysisResultConstPtr shared_from_this() const {
    return boost::static_pointer_cast<const AnalysisResult>
      (BlockScope::shared_from_this());
  }

private:
  int m_scalarArraySortedAvgLen;
  int m_scalarArraySortedIndex;
  int m_scalarArraySortedSumLen;
  std::vector<ScalarArrayExp> m_scalarArraySorted;
  int m_scalarArrayCompressedTextSize;
  bool m_pregenerating, m_pregenerated;
  BlockScopePtrVec m_ignoredScopes;

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef boost::graph_traits<Graph>::adjacency_iterator adjacency_iterator;
  Mutex m_depGraphMutex;
  Graph m_depGraph;
  typedef std::map<vertex_descriptor, FileScopePtr> VertexToFileScopePtrMap;
  VertexToFileScopePtrMap m_fileVertMap;
  void getTrueDeps(FileScopePtr f,
                   std::map<std::string, FileScopePtr> &trueDeps);
  void clusterByFileSizes(StringToFileScopePtrVecMap &clusters,
                          int clusterCount);
  void renameStaticNames(std::map<int, std::vector<std::string> > &names,
                         const char *file, const char *prefix);

  typedef std::map<std::string, std::string> StringMap;
  Mutex m_pregenMapMutex;
  StringMap m_pregenMap;

  typedef std::map<int, LocationPtr> SourceLocationMap;
  typedef std::map<std::string, SourceLocationMap> SourceInfo;
  Mutex m_sourceInfoMutex;
  SourceInfo m_sourceInfos;
  SourceInfo m_sourceInfoPregen;
  std::map<std::string, std::set<std::pair<std::string, int> > > m_clsNameMap;
  std::map<std::string, std::set<std::pair<std::string, int> > > m_funcNameMap;

  Mutex m_namedStringLiteralsMutex;
  std::map<int, std::vector<std::string> > m_namedStringLiterals;
  std::set<std::string> m_namedVarStringLiterals;

  int m_funcTableSize;
  CodeGenerator::MapIntToStringVec m_funcTable;
  bool m_system;
  bool m_sepExtension;

  /**
   * Checks whether the file is in one of the on-demand parsing directories.
   */
  bool inParseOnDemandDirs(const std::string &filename) const;

  void collectFunctionsAndClasses(FileScopePtr fs);

  /**
   * Making sure symbol orders are not different even with multithreading, so
   * to make sure generated code are consistent every time.
   */
  void canonicalizeSymbolOrder();

  /**
   * Checks circular class derivations that can cause stack overflows for
   * subsequent analysis. Also checks to make sure no two redundant parents.
   */
  void checkClassDerivations();

  /**
   * Creates the global function table. Needs to be called before generating
   * cpp code for each toplevel function.
   */
  void createGlobalFuncTable();

  void outputCPPScalarArrays(CodeGenerator &cg, int fileCount, int part);

  void collectCPPGlobalSymbols(StringPairSetVec &symbols,
                               CodeGenerator &cg);
  void outputCPPGlobalStateFileHeader(CodeGenerator &cg);
  void outputCPPGlobalStateBegin(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateEnd(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateSection(CodeGenerator &cg,
                                   const StringPairSet &names,
                                   const char *section,
                                   const char *prefix = "g->",
                                   const char *name_prefix = "");

  void outputCPPClassIncludes(CodeGenerator &cg);
  void outputCPPExtClassImpl(CodeGenerator &cg);
  void outputCPPDynamicTablesHeader(CodeGenerator &cg,
                                    bool includeGlobalVars = true,
                                    bool includes = true,
                                    bool noNamespace = false);
  void outputCPPGlobalImplementations(CodeGenerator &cg);

  void preGenerateCPP(CodeGenerator::Output output,
                      const FileScopePtrVec &files, int threadCount);
  void movePregeneratedSourceInfo(const std::string &source,
                                  const std::string &target, int offset);
  int getFileSize(FileScopePtr fs);

  void repartitionCPP(const std::string &filename, int64 targetSize,
                      bool insideHPHP, bool force);

  void outputCPPFFIStubs();
  void outputHSFFIStubs();

  /**
   * Outputs Java stubs.
   *
   * Each PHP file becomes a Java package, in which the HphpMain class
   * contains all the toplevel function definitions, and the rest of the
   * classes are one-to-one corresponding to the php classes declared in
   * that file.
   */
  void outputJavaFFIStubs();
  /**
   * Outputs one .h file that declares all the Java native method stubs,
   * avoiding javah for performance reason.
   */
  void outputJavaFFICppDecl();
  /**
   * Outputs one .cpp file that implements all the native methods declared
   * in the Java classes generated by outputJavaFFIStubs().
   */
  void outputJavaFFICppImpl();

  void outputSwigFFIStubs();

  void outputHexBuffer(CodeGenerator &cg, const char *name,
                       const char *buf, int len);
  void outputCPPNamedLiteralStrings(bool genStatic, const std::string &file);
  void outputCPPSepExtensionIncludes(CodeGenerator &cg);

  void outputCPPInvokeFileHeader(CodeGenerator &cg);
  void outputCPPEvalHook(CodeGenerator &cg);
  void outputCPPDefaultInvokeFile(CodeGenerator &cg, const char *file);

  void outputArrayCreateDecl(CodeGenerator &cg);
  void outputArrayCreateImpl(CodeGenerator &cg);
  void outputCPPHashTableInvokeFile(CodeGenerator &cg,
                                    const std::vector<const char*> &entries,
                                    bool needEvalHook);
  void outputCPPDynamicClassTables(CodeGenerator::Output output);
  void outputCPPDynamicConstantTable(CodeGenerator::Output output);
  void outputCPPHashTableGetConstant(CodeGenerator &cg, bool system,
         const std::map<std::string, TypePtr> &constMap,
         const hphp_string_map<bool> &dyns);
  void cloneRTTIFuncs(const StringToFunctionScopePtrMap &functions,
                      const StringToFunctionScopePtrVecMap *redecFunctions);
  void outputInitLiteralVarStrings(CodeGenerator &cg, int fileIndex,
         std::vector<int> &litVarStrFileIndices,
         std::vector<std::pair<int, int> > &litVarStrs);

  void outputInitLiteralVarStrings();
  void outputStringProxyData(CodeGenerator &cg,
    int fileIndex, std::vector<std::string> &lStrings,
    std::vector<std::pair<std::string, int> > &bStrings);
  void outputVarStringProxyData(CodeGenerator &cg,
    int fileIndex, std::vector<std::pair<int, int> > &litVarStrs);


public:
  static DECLARE_THREAD_LOCAL(BlockScopeRawPtr, s_currentScopeThreadLocal);
  static DECLARE_THREAD_LOCAL(BlockScopeRawPtrFlagsHashMap,
                              s_changedScopesMapThreadLocal);

#ifdef HPHP_INSTRUMENT_PROCESS_PARALLEL
  static int                                  s_NumDoJobCalls;
  static ConcurrentBlockScopeRawPtrIntHashMap s_DoJobUniqueScopes;
  static int                                  s_NumForceRerunGlobal;
  static int                                  s_NumReactivateGlobal;
  static int                                  s_NumForceRerunUseKinds;
  static int                                  s_NumReactivateUseKinds;
#endif /* HPHP_INSTRUMENT_PROCESS_PARALLEL */

private:
  template <typename Visitor>
  void processScopesParallel(const char *id, void *opaque = NULL);

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
  RescheduleException(BlockScopeRawPtr scope) :
    Exception(false), m_scope(scope) {}
  BlockScopeRawPtr &getScope() { return m_scope; }
#ifdef HPHP_INSTRUMENT_TYPE_INF
  static int s_NumReschedules;
  static int s_NumForceRerunSelfCaller;
  static int s_NumRetTypesChanged;
#endif /* HPHP_INSTRUMENT_TYPE_INF */
private:
  BlockScopeRawPtr m_scope;
};

class SetCurrentScope {
public:
  SetCurrentScope(BlockScopeRawPtr scope) {
    ASSERT(!((*AnalysisResult::s_currentScopeThreadLocal).get()));
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
    ASSERT(AnalysisResult::s_currentScopeThreadLocal->get()); \
    ASSERT(AnalysisResult::s_currentScopeThreadLocal->get() == \
           (scope).get()); \
    (scope)->getInferTypesMutex().assertOwnedBySelf(); \
  } while (0)

#ifdef HPHP_INSTRUMENT_TYPE_INF
typedef std::pair < const char *, int > LEntry;

struct LEntryHasher {
  bool equal(const LEntry &l1, const LEntry &l2) const {
    ASSERT(l1.first);
    ASSERT(l2.first);
    return l1.second == l2.second &&
           strcmp(l1.first, l2.first) == 0;
  }
  size_t hash(const LEntry &l) const {
    ASSERT(l.first);
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
    ASSERT(current);
    ASSERT(!current->is(BlockScope::ClassScope) ||
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
      ASSERT(success);
      m_acquired = true;
      m_mutex.assertOwnedBySelf();
    }
  }
#else
  BaseTryLock(BlockScopeRawPtr scopeToLock,
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
      ASSERT(success);
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
  TryLock(BlockScopeRawPtr scopeToLock,
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
#endif // __ANALYSIS_RESULT_H__
