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

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/json.h"

#include "hphp/util/deprecated/declare-boost-types.h"
#include "hphp/util/md5.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_EXTENDED_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(Expression)
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(StatementList);

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
    ContainsDynamicVariable  = 0x0001,
    ContainsLDynamicVariable = 0x0002,
    VariableArgument         = 0x0004,
    ContainsExtract          = 0x0008, // contains call to extract()
    ContainsCompact          = 0x0010, // contains call to compact()
    ContainsReference        = 0x0020, // returns ref or has ref parameters
    ReferenceVariableArgument = 0x0040, // like sscanf or fscanf
    ContainsUnset            = 0x0080, // need special handling
    NoEffect                 = 0x0100, // does not side effect
    HelperFunction           = 0x0200, // runtime helper function
    ContainsGetDefinedVars   = 0x0400, // need VariableTable with getDefinedVars
    IsFoldable               = 0x01000,// function can be constant folded
    NoFCallBuiltin           = 0x02000,// function should not use FCallBuiltin
    AllowOverride            = 0x04000,// allow override of systemlib or builtin
    NeedsFinallyLocals       = 0x08000,
    VariadicArgumentParam    = 0x10000,// ...$ capture of variable arguments
    ContainsAssert           = 0x20000,// contains call to assert()
  };

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
  void addConstant(const std::string &name, ExpressionPtr value,
                   AnalysisResultPtr ar, ConstructPtr con);
  void declareConstant(AnalysisResultPtr ar, const std::string &name);
  void getConstantNames(std::vector<std::string> &names);

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

  void setSystem();
  bool isSystem() const { return m_system; }

  void setHHFile();
  bool isHHFile() const { return m_isHHFile; }

  void setPreloadPriority(int p) { m_preloadPriority = p; }
  int preloadPriority() const { return m_preloadPriority; }

  void analyzeProgram(AnalysisResultPtr ar);

  void visit(AnalysisResultPtr ar,
             void (*cb)(AnalysisResultPtr, StatementPtr, void*),
             void *data);
  const std::string &pseudoMainName();
  bool load();

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
  unsigned m_system : 1;
  unsigned m_isHHFile : 1;
  int m_preloadPriority;

  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToFunctionScopePtrVecMap *m_redeclaredFunctions;
  StringToClassScopePtrVecMap m_classes;      // name => class
  FunctionScopeRawPtr m_pseudoMain;

  std::string m_pseudoMainName;
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
