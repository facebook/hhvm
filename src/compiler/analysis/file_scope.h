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
    ContainsDynamicVariable = 1,
    ContainsLDynamicVariable = 2,
    VariableArgument = 4,
    ContainsExtract = 8, // need special VariableTable
    ContainsCompact = 16, // need RVariableTable + exists()
    ContainsReference = 32, // either returning ref or with ref parameters
    ReferenceVariableArgument = 64, // like sscanf or fscanf
    ContainsUnset = 128, // need special handling
    NoEffect = 256, // does not side effect
    HelperFunction = 512, // runtime helper function
    ContainsGetDefinedVars = 1024, // need VariableTable with getDefinedVars
  };

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
  FileScope(const std::string &fileName, int fileSize);
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

  /**
   * Parser functions. Parser only deals with a FileScope object, and these
   * are the only functions a parser calls upon analysis results.
   */
  void setTree(StatementListPtr tree);
  void addClass(AnalysisResultPtr ar, ClassScopePtr classScope);

  void addDeclare(std::string d) { m_declares.push_back(d); }
  void addSuppressError(CodeError::ErrorType e) {
    m_suppressedErrors.push_back(e);
  }
  bool isErrorSuppressed(CodeError::ErrorType e);

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
  void preOptimize(AnalysisResultPtr ar);
  void postOptimize(AnalysisResultPtr ar);
  const std::string &pseudoMainName();
  void outputFileCPP(AnalysisResultPtr ar, CodeGenerator &cg);
  bool load();

  std::string outputFilebase();

  void addPseudoMainVariable(const std::string &name) {
    m_pseudoMainVariables.insert(name);
  }
  std::set<std::string> &getPseudoMainVariables() {
    return m_pseudoMainVariables;
  }

  void outputCPPForwardDeclHeader(CodeGenerator &cg, AnalysisResultPtr ar);
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

  std::string addLambda(const std::string &rt, const std::string &args,
                        const std::string &body) {
    lambda l;
    l.name = CodeGenerator::GetNewLambda();
    l.rt = rt;
    l.args = args;
    l.body = body;
    m_lambdas.push_back(l);
    return l.name;
  }

private:
  int m_size;
  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToClassScopePtrVecMap m_classes;      // name => class
  ClassScopePtrVec m_ignoredClasses;

  std::vector<std::string> m_declares;
  std::vector<CodeError::ErrorType> m_suppressedErrors;

  vertex_descriptor m_vertex;

  std::set<std::string> m_usedFuncsInline;
  std::set<std::string> m_usedClasses;
  std::set<std::string> m_usedConsts;
  std::set<std::string> m_usedIncludesInline;
  std::string m_pseudoMainName;
  std::set<std::string> m_pseudoMainVariables;

  struct lambda {
    std::string rt;
    std::string name;
    std::string args;
    std::string body;
  };

  std::vector<lambda> m_lambdas;

  void createPseudoMain(AnalysisResultPtr ar);
  void outputCPPHelper(CodeGenerator &cg, AnalysisResultPtr ar,
                       bool classes = true);

  FileScopePtr shared_from_this() {
    return boost::static_pointer_cast<FileScope>
      (BlockScope::shared_from_this());
  }

};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FILE_SCOPE_H__
