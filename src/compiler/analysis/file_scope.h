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
class FileScope : public BlockScope, public FunctionContainer {
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
  };

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
  FileScope(const std::string &fileName, int fileSize);
  ~FileScope() {}
  int getSize() const { return m_size;}

  // implementing FunctionContainer
  virtual std::string getParentName() const { ASSERT(false); return "";}

  const std::string &getName() const { return m_fileName;}
  StatementListPtr getStmt() const { return m_tree;}
  const StringToClassScopePtrVecMap &getClasses() const {
    return m_classes;
  }
  ClassScopePtr getClass(const char *name);

  virtual int getFunctionCount() const;
  virtual void countReturnTypes(std::map<std::string, int> &counts);
  int getClassCount() const { return m_classes.size();}

  void pushAttribute();
  void setAttribute(Attribute attr);
  int getGlobalAttribute() const;
  int popAttribute();


  /**
   * Whether this file has top level non-declaration statements that
   * have CPP implementation.
   */
  bool hasImpl(AnalysisResultPtr ar) const;
  ExpressionPtr getEffectiveImpl(AnalysisResultConstPtr ar) const;

  /**
   * Parser functions. Parser only deals with a FileScope object, and these
   * are the only functions a parser calls upon analysis results.
   */
  FunctionScopePtr setTree(AnalysisResultConstPtr ar, StatementListPtr tree);
  bool addClass(AnalysisResultConstPtr ar, ClassScopePtr classScope);

  void addDeclare(std::string d) { m_declares.push_back(d); }

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

  void addUsedClassConstHeader(const std::string &cls, const std::string &s) {
    m_usedClassConstsHeader.insert(UsedClassConst(cls, s));
  }

  void addUsedClassHeader(const std::string &s) {
    m_usedClassesHeader.insert(s);
  }

  void addUsedClassFullHeader(const std::string &s) {
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

  void analyzeProgram(AnalysisResultPtr ar);
  void inferTypes(AnalysisResultPtr ar);
  void visit(AnalysisResultPtr ar,
             void (*cb)(AnalysisResultPtr, StatementPtr, void*),
             void *data);
  const std::string &pseudoMainName();
  void outputFileCPP(AnalysisResultPtr ar, CodeGenerator &cg);
  bool load();

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
  void outputCPPForwardDeclHeader(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPDeclHeader(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPForwardDeclarations(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPDeclarations(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPClassHeaders(CodeGenerator &cg, AnalysisResultPtr ar,
                             CodeGenerator::Output output);
  void outputCPPForwardClassHeaders(CodeGenerator &cg, AnalysisResultPtr ar,
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
  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToClassScopePtrVecMap m_classes;      // name => class
  ClassScopePtrVec m_ignoredClasses;
  FunctionScopeRawPtr m_pseudoMain;

  std::vector<std::string> m_declares;

  vertex_descriptor m_vertex;

  std::set<std::string> m_usedFuncsInline;
  std::set<std::string> m_usedClassesHeader;
  std::set<std::string> m_usedClassesFullHeader;
  std::set<std::string> m_usedConstsHeader;
  typedef std::pair<std::string, std::string> UsedClassConst;
  std::set<UsedClassConst> m_usedClassConstsHeader;
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

  struct lambda {
    std::string rt;
    std::string name;
    std::string args;
    std::string body;
  };

  FunctionScopePtr createPseudoMain(AnalysisResultConstPtr ar);
  void setFileLevel(StatementListPtr stmt);
  void outputCPPHelper(CodeGenerator &cg, AnalysisResultPtr ar,
                       bool classes = true);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FILE_SCOPE_H__
