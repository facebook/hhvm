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

#ifndef incl_HPHP_BLOCK_SCOPE_H_
#define incl_HPHP_BLOCK_SCOPE_H_

#include "hphp/compiler/hphp.h"

#include "hphp/util/bits.h"
#include "hphp/util/lock.h"

#include <tbb/concurrent_hash_map.h>

#include <list>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CodeGenerator;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(IncludeExpression);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FileScope);

/**
 * Base class of ClassScope and FunctionScope.
 */
struct BlockScope : std::enable_shared_from_this<BlockScope> {
  enum KindOf {
    ClassScope,
    FunctionScope,
    FileScope,
    ProgramScope,
  };

  BlockScope(const BlockScope&) = delete;
  BlockScope& operator=(const BlockScope&) = delete;

  BlockScope(const std::string &name, const std::string &docComment,
             StatementPtr stmt, KindOf kind);
  virtual ~BlockScope() {}
  bool is(KindOf kind) const { return kind == m_kind;}
  const std::string &getScopeName() const { return m_scopeName;}
  virtual bool isBuiltin() const { return false; }
  StatementPtr getStmt() const { return m_stmt;}
  VariableTableConstPtr getVariables() const { return m_variables;}
  ConstantTableConstPtr getConstants() const { return m_constants;}
  VariableTablePtr getVariables() { return m_variables;}
  ConstantTablePtr getConstants() { return m_constants;}
  ClassScopeRawPtr getContainingClass();
  FunctionScopeRawPtr getContainingNonClosureFunction();
  FunctionScopeRawPtr getContainingFunction() const {
    return FunctionScopeRawPtr(is(FunctionScope) ?
                               (HPHP::FunctionScope*)this : 0);
  }
  FileScopeRawPtr getContainingFile();
  AnalysisResultRawPtr getContainingProgram();

  ClassScopeRawPtr findExactClass(ClassScopeRawPtr cls);

  const std::string &getDocComment() const { return m_docComment;}
  void setDocComment(const std::string &doc) { m_docComment = doc;}

  /**
   * Code gen
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual bool inPseudoMain() const {
    return false;
  }

  virtual ClassScopePtr getParentScope(AnalysisResultConstRawPtr /*ar*/) const {
    return ClassScopePtr();
  }

  void setOuterScope(BlockScopePtr o) { m_outerScope = o; }
  BlockScopePtr getOuterScope() { return m_outerScope.lock(); }
  bool isOuterScope() { return m_outerScope.expired(); }

  void incPass() { m_pass++; }
  bool isFirstPass() const { return !m_pass; }

protected:
  std::string m_scopeName;
  std::string m_docComment;
  StatementPtr m_stmt;
  KindOf m_kind;
  VariableTablePtr m_variables;
  ConstantTablePtr m_constants;
  BlockScopeRawPtr m_outerScope;

  int m_pass;
public:
  static Mutex s_constMutex;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BLOCK_SCOPE_H_
