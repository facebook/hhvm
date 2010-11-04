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

#ifndef __ANALYSIS_RESULT_H__
#define __ANALYSIS_RESULT_H__

#include <compiler/code_generator.h>
#include <compiler/analysis/code_error.h>
#include <compiler/option.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_container.h>
#include <compiler/package.h>
#include <compiler/analysis/method_slot.h>
#include <boost/graph/adjacency_list.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(DependencyGraph);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(ScalarExpression);
DECLARE_BOOST_TYPES(LoopStatement);

class AnalysisResult : public BlockScope, public FunctionContainer {
public:
  /**
   * There are multiple passes over our syntax trees. This lists all of them.
   */
  enum Phase {
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
    SecondInference,
    MoreInference,
    LastInference,

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

public:
  AnalysisResult();

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
  void appendExtraCode(const std::string &code);

  Phase getPhase() const { return m_phase;}
  void setPhase(Phase phase) { m_phase = phase;}
  bool isFirstPass() const {
    return m_phase == AnalyzeInclude || m_phase == FirstInference;
  }
  bool isSecondPass() const {
    return m_phase == SecondInference;
  }

  DependencyGraphPtr getDependencyGraph() {
    return m_dependencyGraph;
  }
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
  void inferTypes(int maxPass = 100);
  void dump();
  void visitFiles(void (*cb)(AnalysisResultPtr, StatementPtr, void*),
                  void *data);

  void getFuncScopesSet(BlockScopeRawPtrQueue &v, FunctionContainerPtr fc);
  void getScopesSet(BlockScopeRawPtrQueue &v);

  ExpressionPtr preOptimize(ExpressionPtr e);
  void preOptimize();
  void postOptimize();

  void incOptCounter() { m_optCounter++; }
  int getOptCounter() const { return m_optCounter; }

  /**
   * When types are newly inferred, we need more passes, until no new types
   * are inferred.
   */
  void incNewlyInferred() {
    m_newlyInferred++;
  }

  void containsDynamicFunctionCall() { m_dynamicFunction = true;}
  void containsDynamicClass() { m_dynamicClass = true;}

  /**
   * Scalar array handling.
   */
  int registerScalarArray(FileScopePtr scope, ExpressionPtr pairs,
                          int &hash, int &index, std::string &text);
  int checkScalarArray(const std::string &text, int &index);
  int getScalarArrayId(const std::string &text);
  void outputCPPNamedScalarArrays(const std::string &file);

  void setInsideScalarArray(bool flag);
  bool getInsideScalarArray();
  std::string getScalarArrayCompressedText();
  std::string getScalarArrayName(int hash, int index);

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
  void outputAllCPP(CodeGenerator &cg); // mainly for unit test

  void outputCPPSystemImplementations(CodeGenerator &cg);
  void outputCPPFileRunDecls(CodeGenerator &cg);
  void outputCPPFileRunImpls(CodeGenerator &cg);
  void outputCPPRedeclaredFunctionDecl(CodeGenerator &cg, bool constructor);
  void outputCPPRedeclaredFunctionImpl(CodeGenerator &cg);
  void outputCPPRedeclaredClassDecl(CodeGenerator &cg);
  void outputCPPRedeclaredClassImpl(CodeGenerator &cg);
  void outputCPPDynamicConstantDecl(CodeGenerator &cg);
  void outputCPPDynamicConstantImpl(CodeGenerator &cg);
  void outputCPPScalarArrayDecl(CodeGenerator &cg);
  void outputCPPScalarArrayImpl(CodeGenerator &cg);
  void outputCPPScalarArrayInit(CodeGenerator &cg, int fileCount, int part);
  void outputCPPScalarArrayId(CodeGenerator &cg, int id, int hash, int index);
  void outputCPPClassStaticInitializerFlags(CodeGenerator &cg,
                                            bool constructor);
  void outputCPPClassDeclaredFlags(CodeGenerator &cg);
  bool inExpression() { return m_inExpression; }
  void setInExpression(bool in) { m_inExpression = in; }
  bool wrapExpressionBegin(CodeGenerator &);
  bool wrapExpressionEnd(CodeGenerator &);
  LoopStatementPtr getLoopStatement() const { return m_loopStatement; }
  void setLoopStatement(LoopStatementPtr loop) {
    m_loopStatement = loop;
  }

  /**
   * Parser creates a FileScope upon parsing a new file.
   */
  FileScopePtr findFileScope(const std::string &name, bool parseOnDemand);
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
  bool declareFunction(FunctionScopePtr funcScope);
  bool declareClass(ClassScopePtr classScope);
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

  /**
   * Find class by that name, and update class name if it's "self" or
   * "parent".
   */
  ClassScopePtr resolveClass(ConstructPtr cs, std::string &className);
  ClassScopePtr findClass(const std::string &className,
                          FindClassBy by = ClassName);
  /**
   * Find all the redeclared classes by the name, excluding system classes.
   * Note that system classes cannot be redeclared.
   */
  const ClassScopePtrVec &findRedeclaredClasses(const std::string &className);
  /**
   * Find all the classes by the name, including system classes.
   */
  ClassScopePtrVec findClasses(const std::string &className);
  bool classMemberExists(const std::string &name, FindClassBy by);
  ClassScopePtr findExactClass(ConstructPtr cs, const std::string &name);
  bool checkClassPresent(ConstructPtr cs, const std::string &name);
  FunctionScopePtr findFunction(const std::string &funcName);
  FunctionScopePtr findHelperFunction(const std::string &funcName);
  BlockScopePtr findConstantDeclarer(const std::string &constName);
  bool isConstantDeclared(const std::string &constName);
  bool isConstantRedeclared(const std::string &constName);
  bool isSystemConstant(const std::string &constName);
  bool isBaseSysRsrcClass(const std::string &className);
  void addNonFinal(const std::string &className);
  bool isNonFinalClass(const std::string &className);
  bool needStaticArray(ClassScopePtr cls, FunctionScopePtr func);

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
  std::string getLiteralStringName(int hash, int index);
  int getLiteralStringId(const std::string &s, int &index);
  void getLiteralStringCompressed(std::string &zsdata, std::string &zldata);

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

  std::set<std::string> m_variableTableFunctions;
  std::set<int> m_concatLengths;
  std::set<int> m_arrayLitstrKeySizes;
  std::set<int> m_arrayIntegerKeySizes;
  void pushCallInfo(int cit);
  void popCallInfo();
  int callInfoTop();

  void setSystem() { m_system = true; }
  bool isSystem() const { return m_system; }
private:
  Package *m_package;
  bool m_parseOnDemand;
  std::vector<std::string> m_parseOnDemandDirs;
  Phase m_phase;
  int m_newlyInferred;
  DependencyGraphPtr m_dependencyGraph;
  StringToFileScopePtrMap m_files;
  FileScopePtrVec m_fileScopes;
  std::string m_extraCodeFileName;
  std::string m_extraCode;

  StringToClassScopePtrMap m_systemClasses;
  StringToFunctionScopePtrVecMap m_functionDecs;
  StringToClassScopePtrVecMap m_classDecs;
  StringToClassScopePtrVecMap m_methodToClassDecs;
  StringToFileScopePtrMap m_constDecs;
  std::set<std::string> m_constRedeclared;
  std::set<std::string> m_baseSysRsrcClasses;
  std::set<std::string> m_nonFinalClasses;

  bool m_dynamicClass;
  bool m_dynamicFunction;
  bool m_classForcedVariants[2];

  StatementPtrVec m_stmts;
  StatementPtr m_stmt;

  std::string m_outputPath;
  int m_optCounter;

  std::map<std::string, int> m_scalarArrays;
  std::map<int, std::vector<std::string> > m_namedScalarArrays;
  int m_scalarArraysCounter;
  std::vector<ExpressionPtr> m_scalarArrayIds;
  std::map<std::string, int> m_paramRTTIs;
  std::set<std::string> m_rttiFuncs;
  int m_paramRTTICounter;

  bool m_insideScalarArray;
  bool m_inExpression;
  bool m_wrappedExpression;

  LoopStatementPtr m_loopStatement;
public:
  struct ScalarArrayExp {
    int id;
    int len;
    ExpressionPtr exp;
  };
private:
  int m_scalarArraySortedAvgLen;
  int m_scalarArraySortedIndex;
  int m_scalarArraySortedSumLen;
  std::vector<ScalarArrayExp> m_scalarArraySorted;
  int m_scalarArrayCompressedTextSize;

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

  std::map<std::string, std::map<int, LocationPtr> > m_sourceInfos;
  std::map<std::string, std::set<std::pair<std::string, int> > > m_clsNameMap;
  std::map<std::string, std::set<std::pair<std::string, int> > > m_funcNameMap;

  std::map<std::string, int> m_stringLiterals;
  std::map<int, std::vector<std::string> > m_namedStringLiterals;

  int m_funcTableSize;
  CodeGenerator::MapIntToStringVec m_funcTable;
  bool m_system;

  std::deque<int> m_callInfos;

  /**
   * Checks whether the file is in one of the on-demand parsing directories.
   */
  bool inParseOnDemandDirs(const std::string &filename);

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

  void outputCPPGlobalDeclarations();
  void outputCPPMain();
  void outputCPPScalarArrays(bool system);
  void outputCPPScalarArrays(CodeGenerator &cg, int fileCount, int part);
  void outputCPPGlobalVariablesMethods(int part);

  void collectCPPGlobalSymbols(StringPairVecVec &symbols,
                               CodeGenerator &cg);
  void outputCPPGlobalState();
  void outputCPPGlobalStateBegin(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateEnd(CodeGenerator &cg, const char *section);
  void outputCPPGlobalStateSection(CodeGenerator &cg,
                                   const StringPairVec &names,
                                   const char *section,
                                   const char *prefix = "g->",
                                   const char *name_prefix = "");

  void outputCPPFiberGlobalState();

  void outputCPPClassStaticInitializerDecls(CodeGenerator &cg);
  void outputCPPClassIncludes(CodeGenerator &cg);
  void outputCPPExtClassImpl(CodeGenerator &cg);
  void outputCPPDynamicTables(CodeGenerator::Output output);
  void outputCPPDynamicTablesHeader(CodeGenerator &cg,
                                    bool includeGlobalVars = true,
                                    bool includes = true,
                                    bool noNamespace = false);
  void outputCPPClassMapFile();
  void outputCPPSourceInfos();
  void outputCPPNameMaps();
  void outputRTTIMetaData(const char *filename);
  void outputCPPClassMap(CodeGenerator &cg);
  void outputCPPSystem();
  void outputCPPGlobalImplementations(CodeGenerator &cg);
  void outputCPPClusterImpl(CodeGenerator &cg, const FileScopePtrVec &files);

  void outputCPPClassDeclaredFlagsLookup(CodeGenerator &cg);

  void outputCPPUtilDecl(CodeGenerator::Output output);
  void outputCPPUtilImpl(CodeGenerator::Output output);

  void repartitionCPP(const std::string &filename, int64 targetSize,
                      bool insideHPHP);
  void repartitionLargeCPP(const std::vector<std::string> &filenames,
                           const std::vector<std::string> &additionals);

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
  void outputCPPSepExtensionMake();
  void outputCPPSepExtensionIncludes(CodeGenerator &cg);

  void outputCPPInvokeFileHeader(CodeGenerator &cg);
  void outputCPPEvalHook(CodeGenerator &cg);
  void outputCPPDefaultInvokeFile(CodeGenerator &cg, const char *file);

  void outputTaintNumDecl(CodeGenerator &cg, int num);
  void outputTaintDecl(CodeGenerator &cg);
  void outputTaintImpl(CodeGenerator &cg);
  void outputConcatNumDecl(CodeGenerator &cg, int num);
  void outputConcatDecl(CodeGenerator &cg);
  void outputConcatImpl(CodeGenerator &cg);
  void outputArrayCreateNumDecl(CodeGenerator &cg, int num, const char *type);
  void outputArrayCreateDecl(CodeGenerator &cg);
  void outputArrayCreateImpl(CodeGenerator &cg);
  void outputCPPHashTableInvokeFile(CodeGenerator &cg,
                                    const std::vector<const char*> &entries,
                                    bool needEvalHook);

  void cloneRTTIFuncs(ClassScopePtr cls,
                      const StringToFunctionScopePtrVecMap &functions);

  AnalysisResultPtr shared_from_this() {
    return boost::static_pointer_cast<AnalysisResult>
      (BlockScope::shared_from_this());
  }

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
