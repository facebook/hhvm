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
#include <compiler/analysis/method_slot.h>
#include <boost/graph/adjacency_list.hpp>
#include <util/string_bag.h>

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
    AnalyzeInclude,
    AnalyzeTopLevel,
    AnalyzeAll,
    AnalyzeFinal,

    // pre-optimize
    FirstPreOptimize,
    SecondPreOptimize,

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
  bool isParseOnDemand() { return m_package && m_parseOnDemand;}
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
  bool isAnalyzeInclude() const { return m_phase == AnalyzeInclude; }

  int getFunctionCount() const;
  int getClassCount() const;
  void countReturnTypes(std::map<std::string, int> &counts);

  void addEntryPoint(const std::string &name);
  void addEntryPoints(const std::vector<std::string> &names);

  void loadBuiltinFunctions();
  void loadBuiltins();
  void analyzeProgram(bool system = false);
  void analyzeProgramFinal();
  void analyzePerfectVirtuals();
  void dump();
  void visitFiles(void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                  void *data);

  void getFuncScopesSet(BlockScopeRawPtrQueue &v, FunctionContainerPtr fc);
  void getScopesSet(BlockScopeRawPtrQueue &v);

  void preOptimize();
  void inferTypes();
  void postOptimize();

  void containsDynamicFunctionCall() { m_dynamicFunction = true;}
  void containsDynamicClass() { m_dynamicClass = true;}

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
  void forceClassVariants(ClassScopePtr curScope, bool doStatic);

  /**
   * Force specified variable of all classes to be variants.
   */
  void forceClassVariants(const std::string &name, ClassScopePtr curScope,
                          bool doStatic);

  /**
   * Code generation functions.
   */
  bool outputAllPHP(CodeGenerator::Output output);
  void outputAllCPP(CodeGenerator::Output output, int clusterCount,
                    const std::string *compileDir);

  void outputCPPSystemImplementations(CodeGenerator &cg);
  void outputCPPFileRunDecls(CodeGenerator &cg,
                             Type2SymbolListMap &type2names);
  void outputCPPRedeclaredFunctionDecl(CodeGenerator &cg,
                                       Type2SymbolListMap &type2names);
  void outputCPPRedeclaredClassDecl(CodeGenerator &cg,
                                    Type2SymbolListMap &type2names);
  void outputCPPRedeclaredClassImpl(CodeGenerator &cg);
  void outputCPPDynamicConstantDecl(CodeGenerator &cg,
                                    Type2SymbolListMap &type2names);
  void outputCPPDynamicConstantImpl(CodeGenerator &cg);
  void outputCPPScalarArrayDecl(CodeGenerator &cg);
  void outputCPPScalarArrayImpl(CodeGenerator &cg);
  void outputCPPScalarArrayInit(CodeGenerator &cg, int fileCount, int part);
  void outputCPPScalarArrayId(CodeGenerator &cg, int id, int hash, int index,
                              bool scalarVariant = false);
  void outputCPPClassStaticInitializerFlags(CodeGenerator &cg,
                                            Type2SymbolListMap &type2names);
  void outputCPPClassDeclaredFlags(CodeGenerator &cg,
                                   Type2SymbolListMap &type2names);

  void outputCPPFiniteDouble(CodeGenerator &cg, double dval);

  /**
   * Parser creates a FileScope upon parsing a new file.
   */
  void parseOnDemand(const std::string &name) const;
  FileScopePtr findFileScope(const std::string &name) const;
  const StringToFileScopePtrMap &getAllFiles() { return m_files;}
  const std::vector<FileScopePtr> &getAllFilesVector() {
    return m_fileScopes;
  }

  void addFileScope(FileScopePtr fileScope);

  /**
   * To implement the silence operator correctly, we need to keep trace
   * of the current statement being parsed.
   */
  void pushStatement(StatementPtr stmt);
  void popStatement();
  StatementPtr getStatement() const { return m_stmt; }
  StatementPtr getStatementForSilencer() const;

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
  FunctionScopePtr findHelperFunction(const std::string &funcName) const;
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
  static std::string prepareFile(const char *root,
                                 const std::string &fileName, bool chop);

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
                         const std::string filename);
  void recordFunctionSource(const std::string &funcname, LocationPtr loc,
                            const std::string filename);

  /**
   * Literal string to String precomputation
   */
  std::string getLiteralStringName(int64 hash, int index);
  std::string getLitVarStringName(int64 hash, int index);
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
  StringToFunctionScopePtrVecMap m_functionDecs;
  StringToClassScopePtrVecMap m_classDecs;
  StringToClassScopePtrVecMap m_methodToClassDecs;
  StringToFileScopePtrMap m_constDecs;
  std::set<std::string> m_constRedeclared;

  bool m_dynamicClass;
  bool m_dynamicFunction;
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

  void genMethodSlots();

  void outputCPPDynamicTables(CodeGenerator::Output output);
  void outputCPPClassMapFile();
  void outputCPPSourceInfos();
  void outputCPPNameMaps();
  void outputRTTIMetaData(const char *filename);
  void outputCPPClassMap(CodeGenerator &cg);
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
  void outputCPPGlobalVariablesMethods(int part);
  void outputCPPGlobalState();
  void outputCPPFiberGlobalState();

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

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef boost::graph_traits<Graph>::adjacency_iterator adjacency_iterator;
  Graph m_depGraph;
  typedef std::map<vertex_descriptor, FileScopePtr> VertexToFileScopePtrMap;
  VertexToFileScopePtrMap m_fileVertMap;
  void link(FileScopePtr user, FileScopePtr provider);
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

  void collectCPPGlobalSymbols(StringPairVecVec &symbols,
                               CodeGenerator &cg);
  void outputCPPGlobalStateBegin(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateEnd(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateSection(CodeGenerator &cg,
                                   const StringPairVec &names,
                                   const char *section,
                                   const char *prefix = "g->",
                                   const char *name_prefix = "");

  void outputCPPClassStaticInitializerDecls(CodeGenerator &cg);
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

  void outputCPPClassDeclaredFlagsLookup(CodeGenerator &cg);

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

  void outputConcatNumDecl(CodeGenerator &cg, int num);
  void outputConcatDecl(CodeGenerator &cg);
  void outputConcatImpl(CodeGenerator &cg);
  void outputArrayCreateDecl(CodeGenerator &cg);
  void outputArrayCreateImpl(CodeGenerator &cg);
  void outputCPPHashTableInvokeFile(CodeGenerator &cg,
                                    const std::vector<const char*> &entries,
                                    bool needEvalHook);
  void outputCPPDynamicClassTables(CodeGenerator::Output output, int part);
  void outputCPPDynamicClassTables(CodeGenerator::Output output);
  void outputCPPDynamicConstantTable(CodeGenerator::Output output);
  void outputCPPHashTableGetConstant(CodeGenerator &cg, bool system,
                                     const std::vector<const char *> &strings,
                                     const std::vector<TypePtr> &types,
                                     const hphp_const_char_map<bool> &dyns);
  void cloneRTTIFuncs(ClassScopePtr cls,
                      const StringToFunctionScopePtrVecMap &functions);
  void outputInitLiteralVarStrings(CodeGenerator &cg, int fileIndex,
         std::vector<int> &litVarStrFileIndices,
         std::vector<std::pair<int, int> > &litVarStrs);

  void outputInitLiteralVarStrings();
  StringToMethodSlotMap stringToMethodSlotMap;
  CallIndexVectSet callIndexVectSet; // set of methods at this callIndex
  friend class MethodSlot;

public:
  const MethodSlot* getMethodSlot(const std::string & mname) const;
  const MethodSlot* getOrAddMethodSlot(const std::string & mname,
                                       ConstructPtr self);
private:
  MethodSlot* getMethodSlotUpdate(const std::string & mname);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __ANALYSIS_RESULT_H__
