/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_FILE_SCOPE_H_
#define incl_HPHP_FILE_SCOPE_H_

#include <string>
#include <map>
#include <boost/algorithm/string.hpp>

#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/code_generator.h"
#include <boost/graph/adjacency_list.hpp>
#include <set>
#include <vector>
#include "hphp/compiler/json.h"
#include "hphp/util/md5.h"

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
    ContainsExtract           = 0x008, // contains call to extract()
    ContainsCompact           = 0x010, // contains call to compact()
    ContainsReference         = 0x020, // returns ref or has ref parameters
    ReferenceVariableArgument = 0x040, // like sscanf or fscanf
    ContainsUnset             = 0x080, // need special handling
    NoEffect                  = 0x100, // does not side effect
    HelperFunction            = 0x200, // runtime helper function
    ContainsGetDefinedVars    = 0x400, // need VariableTable with getDefinedVars
    MixedVariableArgument     = 0x800, // variable args, may or may not be ref'd
    IsFoldable                = 0x1000,// function can be constant folded
    NoFCallBuiltin            = 0x2000,// function should not use FCallBuiltin
    AllowOverride             = 0x4000,// allow override of systemlib or builtin
    NeedsFinallyLocals        = 0x8000,
  };

  typedef boost::adjacency_list<boost::setS, boost::vecS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
  FileScope(const std::string &fileName, int fileSize, const MD5 &md5);
  ~FileScope() { delete m_redeclaredFunctions; }
  int getSize() const { return m_size;}

  // implementing FunctionContainer
  virtual std::string getParentName() const { assert(false); return "";}

  const std::string &getName() const { return m_fileName;}
  const MD5& getMd5() const { return m_md5; }
  void setMd5(const MD5& md5) { m_md5 = md5; }
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

  /**
   * Parser functions. Parser only deals with a FileScope object, and these
   * are the only functions a parser calls upon analysis results.
   */
  FunctionScopePtr setTree(AnalysisResultConstPtr ar, StatementListPtr tree);
  void cleanupForError(AnalysisResultConstPtr ar);
  void makeFatal(AnalysisResultConstPtr ar,
                 const std::string& msg, int line);
  void makeParseFatal(AnalysisResultConstPtr ar,
                      const std::string& msg, int line);

  bool addFunction(AnalysisResultConstPtr ar, FunctionScopePtr funcScope);
  bool addClass(AnalysisResultConstPtr ar, ClassScopePtr classScope);
  const StringToFunctionScopePtrVecMap *getRedecFunctions() {
    return m_redeclaredFunctions;
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

  void addClassAlias(const std::string& target, const std::string& alias) {
    m_classAliasMap.insert(
      std::make_pair(
        boost::to_lower_copy(target),
        boost::to_lower_copy(alias)
      )
    );
  }

  std::multimap<std::string,std::string> const& getClassAliases() const {
    return m_classAliasMap;
  }

  void addTypeAliasName(const std::string& name) {
    m_typeAliasNames.insert(boost::to_lower_copy(name));
  }

  std::set<std::string> const& getTypeAliasNames() const {
    return m_typeAliasNames;
  }

  /**
   * Called only by World
   */
  vertex_descriptor vertex() { return m_vertex; }
  void setVertex(vertex_descriptor vertex) {
    m_vertex = vertex;
  }

  void setSystem();
  bool isSystem() const { return m_system; }

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
  std::string outputFilebase() const;

  FunctionScopeRawPtr getPseudoMain() const {
    return m_pseudoMain;
  }

  FileScopePtr shared_from_this() {
    return static_pointer_cast<FileScope>
      (BlockScope::shared_from_this());
  }

private:
  int m_size;
  MD5 m_md5;
  unsigned m_includeState : 2;
  unsigned m_system : 1;

  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToFunctionScopePtrVecMap *m_redeclaredFunctions;
  StringToClassScopePtrVecMap m_classes;      // name => class
  FunctionScopeRawPtr m_pseudoMain;

  vertex_descriptor m_vertex;

  std::string m_pseudoMainName;
  BlockScopeSet m_providedDefs;
  std::set<std::string> m_redecBases;

  // Map from class alias names to the class they are aliased to.
  // This is only needed in WholeProgram mode.
  std::multimap<std::string,std::string> m_classAliasMap;

  // Set of names that are on the left hand side of type alias
  // declarations.  We need this to make sure we don't mark classes
  // with the same name Unique.
  std::set<std::string> m_typeAliasNames;

  FunctionScopePtr createPseudoMain(AnalysisResultConstPtr ar);
  void setFileLevel(StatementListPtr stmt);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FILE_SCOPE_H_
