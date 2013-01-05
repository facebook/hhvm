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

#ifndef __FILE_SCOPE_H__
#define __FILE_SCOPE_H__

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_container.h>
#include <compiler/analysis/code_error.h>
#include <compiler/code_generator.h>
#include <boost/graph/adjacency_list.hpp>
#include <util/json.h>
#include <runtime/base/md5.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionScope);

/**
 * A FileScope stores what's parsed from one single source file. It's up to
 * AnalysisResult objects to grab statements, functions and classes from
 * FileScope objects to form execution paths.
 */
class FileScope : public BlockScope,
                  public FunctionContainer,
                  public JSON::DocTarget::ISerializable {
public:
  enum Attribute {
    ContainsDynamicVariable   = 0x001,
    ContainsLDynamicVariable  = 0x002,
    VariableArgument          = 0x004,
    ContainsExtract           = 0x008, // need special VariableTable
    ContainsCompact           = 0x010, // need RVariableTable + exists()
    ContainsReference         = 0x020, // returns ref or has ref parameters
    ReferenceVariableArgument = 0x040, // like sscanf or fscanf
    ContainsUnset             = 0x080, // need special handling
    NoEffect                  = 0x100, // does not side effect
    HelperFunction            = 0x200, // runtime helper function
    ContainsGetDefinedVars    = 0x400, // need VariableTable with getDefinedVars
    MixedVariableArgument     = 0x800, // variable args, may or may not be ref'd
    IsFoldable                = 0x1000,// function can be constant folded
    NeedsActRec               = 0x2000,// builtin function needs ActRec
    IgnoreRedefinition        = 0x4000,// ignore redefinition of builtin function
  };

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
  FileScope(const std::string &fileName, int fileSize, const MD5 &md5);
  ~FileScope() { delete m_redeclaredFunctions; }
  int getSize() const { return m_size;}

  // implementing FunctionContainer
  virtual std::string getParentName() const { ASSERT(false); return "";}

  const std::string &getName() const { return m_fileName;}
  const MD5& getMd5() const { return m_md5; }
  StatementListPtr getStmt() const { return m_tree;}
  const StringToClassScopePtrVecMap &getClasses() const {
    return m_classes;
  }
  void getClassesFlattened(ClassScopePtrVec &classes) const;
  ClassScopePtr getClass(const char *name);
  void getScopesSet(BlockScopeRawPtrQueue &v);

  int getFunctionCount() const;
  void countReturnTypes(std::map<std::string, int> &counts);
  int getClassCount() const { return m_classes.size();}

  void pushAttribute();
  void setAttribute(Attribute attr);
  int getGlobalAttribute() const;
  int popAttribute();

  void serialize(JSON::DocTarget::OutputStream &out) const;

  /**
   * Whether this file has top level non-declaration statements that
   * have CPP implementation.
   */
  ExpressionPtr getEffectiveImpl(AnalysisResultConstPtr ar) const;

  bool canUseDummyPseudoMain(AnalysisResultConstPtr ar) const;

  /**
   * Parser functions. Parser only deals with a FileScope object, and these
   * are the only functions a parser calls upon analysis results.
   */
  FunctionScopePtr setTree(AnalysisResultConstPtr ar, StatementListPtr tree);
  void cleanupForError(AnalysisResultConstPtr ar,
                       int line, const std::string &msg);

  bool addFunction(AnalysisResultConstPtr ar, FunctionScopePtr funcScope);
  bool addClass(AnalysisResultConstPtr ar, ClassScopePtr classScope);
  const StringToFunctionScopePtrVecMap *getRedecFunctions() {
    return m_redeclaredFunctions;
  }
  void addUsedClosure(FunctionScopeRawPtr closure) {
    m_usedClosures.insert(closure);
  }
  void addUsedLiteralString(std::string s) {
    m_usedLiteralStrings.insert(s);
  }
  void addUsedLitVarString(std::string s) {
    m_usedLitVarStrings.insert(s);
  }
  std::set<std::string> &getUsedLiteralStrings() {
    return m_usedLiteralStrings;
  }
  std::set<std::string> &getUsedLitVarStrings() {
    return m_usedLitVarStrings;
  }
  void addUsedLiteralStringHeader(std::string s) {
    m_usedLiteralStringsHeader.insert(s);
  }
  void addUsedLitVarStringHeader(std::string s) {
    m_usedLitVarStringsHeader.insert(s);
  }
  void addUsedScalarVarInteger(int64 i) {
    m_usedScalarVarIntegers.insert(i);
  }
  std::set<int64> &getUsedScalarVarIntegers() {
    return m_usedScalarVarIntegers;
  }
  void addUsedScalarVarIntegerHeader(int64 i) {
    m_usedScalarVarIntegersHeader.insert(i);
  }
  void addUsedScalarVarDouble(double d) {
    m_usedScalarVarDoubles.insert(d);
  }
  std::set<double> &getUsedScalarVarDoubles() {
    return m_usedScalarVarDoubles;
  }
  void addUsedScalarVarDoubleHeader(double d) {
    m_usedScalarVarDoublesHeader.insert(d);
  }
  void addUsedScalarArray(std::string s) {
    m_usedScalarArrays.insert(s);
  }
  void addUsedScalarVarArray(std::string s) {
    m_usedScalarVarArrays.insert(s);
  }
  void addUsedDefaultValueScalarArray(std::string s) {
    m_usedDefaultValueScalarArrays.insert(s);
  }
  void addUsedDefaultValueScalarVarArray(std::string s) {
    m_usedDefaultValueScalarVarArrays.insert(s);
  }

  void addUsedConstHeader(const std::string &s) {
    m_usedConstsHeader.insert(s);
  }

  void addUsedClassConstHeader(ClassScopeRawPtr cls, const std::string &s) {
    m_usedClassConstsHeader.insert(CodeGenerator::UsedClassConst(cls, s));
  }

  void addUsedClassHeader(ClassScopeRawPtr s) {
    m_usedClassesHeader.insert(s);
  }

  void addUsedClassFullHeader(ClassScopeRawPtr s) {
    m_usedClassesFullHeader.insert(s);
  }

  /**
   * For separate compilation
   * These add edges between filescopes in the other dep graph and
   * save the symbols for our iface.
   * This stuff only happens in the filechanged state.
   */
  void addConstant(const std::string &name, TypePtr type, ExpressionPtr value,
                   AnalysisResultPtr ar, ConstructPtr con);
  void declareConstant(AnalysisResultPtr ar, const std::string &name);
  void getConstantNames(std::vector<std::string> &names);
  TypePtr getConstantType(const std::string &name);

  void addIncludeDependency(AnalysisResultPtr ar, const std::string &file,
                            bool byInlined);
  void addClassDependency(AnalysisResultPtr ar,
                          const std::string &classname);
  void addFunctionDependency(AnalysisResultPtr ar,
                             const std::string &funcname, bool byInlined);
  void addConstantDependency(AnalysisResultPtr ar,
                             const std::string &decname);

  /**
   * Called only by World
   */
  vertex_descriptor vertex() { return m_vertex; }
  void setVertex(vertex_descriptor vertex) {
    m_vertex = vertex;
  }

  void setModule() { m_module = true; }
  void setPrivateInclude() { m_privateInclude = true; }
  bool isPrivateInclude() const { return m_privateInclude && !m_externInclude; }
  void setExternInclude() { m_externInclude = true; }

  void analyzeProgram(AnalysisResultPtr ar);
  void analyzeIncludes(AnalysisResultPtr ar);
  void analyzeIncludesHelper(AnalysisResultPtr ar);
  bool insertClassUtil(AnalysisResultPtr ar, ClassScopeRawPtr cls, bool def);

  bool checkClass(const std::string &cls);
  ClassScopeRawPtr resolveClass(ClassScopeRawPtr cls);
  FunctionScopeRawPtr resolveFunction(FunctionScopeRawPtr func);
  void visit(AnalysisResultPtr ar,
             void (*cb)(AnalysisResultPtr, StatementPtr, void*),
             void *data);
  const std::string &pseudoMainName();
  void outputFileCPP(AnalysisResultPtr ar, CodeGenerator &cg);
  bool load();
  bool needPseudoMainVariables() const;
  std::string outputFilebase() const;

  void addPseudoMainVariable(const std::string &name) {
    m_pseudoMainVariables.insert(name);
  }
  std::set<std::string> &getPseudoMainVariables() {
    return m_pseudoMainVariables;
  }

  FunctionScopeRawPtr getPseudoMain() const {
    return m_pseudoMain;
  }
  void outputCPPForwardStaticDecl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPDeclHeader(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPForwardDeclarations(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPDeclarations(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPClassHeaders(AnalysisResultPtr ar,
                             CodeGenerator::Output output);
  void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPPseudoMain(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputCPPFFI(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputHSFFI(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputJavaFFI(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputJavaFFICPPStub(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputSwigFFIStubs(CodeGenerator &cg, AnalysisResultPtr ar);

  FileScopePtr shared_from_this() {
    return boost::static_pointer_cast<FileScope>
      (BlockScope::shared_from_this());
  }
private:
  int m_size;
  MD5 m_md5;
  unsigned m_module : 1;
  unsigned m_privateInclude : 1;
  unsigned m_externInclude : 1;
  unsigned m_includeState : 2;

  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToFunctionScopePtrVecMap *m_redeclaredFunctions;
  StringToClassScopePtrVecMap m_classes;      // name => class
  FunctionScopeRawPtr m_pseudoMain;

  vertex_descriptor m_vertex;

  std::set<FunctionScopeRawPtr> m_usedClosures;
  std::set<std::string> m_usedFuncsInline;
  CodeGenerator::ClassScopeSet m_usedClassesHeader;
  CodeGenerator::ClassScopeSet m_usedClassesFullHeader;
  std::set<std::string> m_usedConstsHeader;
  CodeGenerator::UsedClassConstSet m_usedClassConstsHeader;
  std::set<std::string> m_usedIncludesInline;
  std::set<std::string> m_usedLiteralStrings;
  std::set<std::string> m_usedLitVarStrings;
  std::set<std::string> m_usedLiteralStringsHeader;
  std::set<std::string> m_usedLitVarStringsHeader;
  std::set<int64> m_usedScalarVarIntegers;
  std::set<int64> m_usedScalarVarIntegersHeader;
  std::set<double> m_usedScalarVarDoubles;
  std::set<double> m_usedScalarVarDoublesHeader;
  std::set<std::string> m_usedScalarArrays;
  std::set<std::string> m_usedScalarVarArrays;
  std::set<std::string> m_usedDefaultValueScalarArrays;
  std::set<std::string> m_usedDefaultValueScalarVarArrays;
  std::string m_pseudoMainName;
  std::set<std::string> m_pseudoMainVariables;
  BlockScopeSet m_providedDefs;
  std::set<std::string> m_redecBases;

  FunctionScopePtr createPseudoMain(AnalysisResultConstPtr ar);
  void setFileLevel(StatementListPtr stmt);
  void outputCPPHelper(CodeGenerator &cg, AnalysisResultPtr ar,
                       bool classes = true);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FILE_SCOPE_H__
