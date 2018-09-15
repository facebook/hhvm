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

#ifndef incl_HPHP_FILE_SCOPE_H_
#define incl_HPHP_FILE_SCOPE_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/function_container.h"
#include "hphp/compiler/hphp.h"

#include "hphp/compiler/statement/class_statement.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/deprecated/declare-boost-types.h"
#include "hphp/util/md5.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_EXTENDED_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(Expression)
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(ClosureExpression);

/**
 * A FileScope stores what's parsed from one single source file. It's up to
 * AnalysisResult objects to grab statements, functions and classes from
 * FileScope objects to form execution paths.
 */
struct FileScope : BlockScope, FunctionContainer {
  enum Attribute {
    ContainsDynamicVariable  = 0x0001,
    ContainsLDynamicVariable = 0x0002,
    VariableArgument         = 0x0004,
    ContainsReference        = 0x0020, // returns ref or has ref parameters
    ReferenceVariableArgument = 0x0040, // like sscanf or fscanf
    ContainsUnset            = 0x0080, // need special handling
    NoEffect                 = 0x0100, // does not side effect
    HelperFunction           = 0x0200, // runtime helper function
    NoFCallBuiltin           = 0x02000,// function should not use FCallBuiltin
    NeedsFinallyLocals       = 0x08000,
    VariadicArgumentParam    = 0x10000,// ...$ capture of variable arguments
    RefVariadicArgumentParam = 0x40000,// &...$ variadicargument by ref
  };

public:
  FileScope(const std::string &fileName, int fileSize, const MD5 &md5);
  ~FileScope() override {
    delete m_redeclaredFunctions;
  }
  int getSize() const { return m_size;}

  const std::string &getName() const { return m_fileName;}
  const MD5& getMd5() const { return m_md5; }
  void setMd5(const MD5& md5) { m_md5 = md5; }
  StatementListPtr getStmt() const { return m_tree;}
  const StringToClassScopePtrVecMap &getClasses() const {
    return m_classes;
  }

  int getFunctionCount() const;
  int getClassCount() const { return m_classes.size();}

  void pushAttribute();
  void setAttribute(Attribute attr);
  int popAttribute();

  /**
   * Parser functions. Parser only deals with a FileScope object, and these
   * are the only functions a parser calls upon analysis results.
   */
  FunctionScopePtr setTree(AnalysisResultConstRawPtr ar, StatementListPtr tree);
  void cleanupForError(AnalysisResultConstRawPtr ar);
  void makeFatal(AnalysisResultConstRawPtr ar,
                 const std::string& msg, int line);
  void makeParseFatal(AnalysisResultConstRawPtr ar,
                      const std::string& msg, int line);

  void addFunction(AnalysisResultConstRawPtr ar, FunctionScopePtr funcScope);
  void addClass(AnalysisResultConstRawPtr ar, ClassScopePtr classScope);
  const StringToFunctionScopePtrVecMap *getRedecFunctions() {
    return m_redeclaredFunctions;
  }
  void addAnonClass(ClassStatementPtr stmt);
  const std::vector<ClassStatementPtr>& getAnonClasses() const;

  void setSystem();
  bool isSystem() const { return m_system; }

  void setHHFile();
  bool isHHFile() const { return m_isHHFile; }

  void setUseStrictTypes();
  bool useStrictTypes() const { return m_useStrictTypes; }
  void setUseStrictTypesForBuiltins();
  bool useStrictTypesForBuiltins() const { return m_useStrictTypesForBuiltins; }

  void setPreloadPriority(int p) { m_preloadPriority = p; }
  int preloadPriority() const { return m_preloadPriority; }

  void analyzeProgram(AnalysisResultConstRawPtr ar);

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

  static FileScopeRawPtr getCurrent() {
    return FileScopeRawPtr { s_current };
  }

  void addLambda(ClosureExpressionRawPtr c) { m_lambdas.push_back(c); }
private:
  int m_size;
  MD5 m_md5;
  unsigned m_system : 1;
  unsigned m_isHHFile : 1;
  unsigned m_useStrictTypes : 1;
  unsigned m_useStrictTypesForBuiltins : 1;
  int m_preloadPriority;

  std::vector<int> m_attributes;
  std::string m_fileName;
  StatementListPtr m_tree;
  StringToFunctionScopePtrVecMap *m_redeclaredFunctions;
  StringToClassScopePtrVecMap m_classes;      // name => class
  std::vector<ClassStatementPtr> m_anonClasses;
  FunctionScopeRawPtr m_pseudoMain;

  std::string m_pseudoMainName;
  std::set<std::string> m_redecBases;

  // Temporary vector of lambda expressions; populated
  // during analyzeProgram, and then processed at the end
  // of FileScope::analyzeProgram.
  CompactVector<ClosureExpressionRawPtr> m_lambdas;

  FunctionScopePtr createPseudoMain(AnalysisResultConstRawPtr ar);
  void setFileLevel(StatementListPtr stmt);

  static __thread FileScope* s_current;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FILE_SCOPE_H_
