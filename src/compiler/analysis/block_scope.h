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

#ifndef __BLOCK_SCOPE_H__
#define __BLOCK_SCOPE_H__

#include <compiler/hphp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(IncludeExpression);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);

/**
 * Base class of ClassScope and FunctionScope.
 */
class BlockScope : public boost::enable_shared_from_this<BlockScope> {
public:
  enum KindOf {
    ClassScope,
    FunctionScope,
    FileScope,
    ProgramScope,
  };

  BlockScope(const std::string &name, const std::string &docComment,
             StatementPtr stmt, KindOf kind);
  virtual ~BlockScope() {}
  bool is(KindOf kind) const { return kind == m_kind;}
  const std::string &getName() const { return m_name;}
  void setName(const std::string name) { m_name = name;}
  virtual std::string getId() const { return getName(); }
  StatementPtr getStmt() { return m_stmt;}
  VariableTablePtr getVariables() { return m_variables;}
  ConstantTablePtr getConstants() { return m_constants;}

  /**
   * Helpers for keeping track of break/continue nested level.
   */
  void incLoopNestedLevel();
  void decLoopNestedLevel();
  int getLoopNestedLevel() const { return m_loopNestedLevel;}

  /**
   * Helpers for conditional includes.
   */
  void setIncludeLevel(int incLevel) { m_incLevel = incLevel;}
  int getIncludeLevel() const { return m_incLevel;}

  /**
   * Helpers for parsing class functions and variables.
   */
  ModifierExpressionPtr setModifiers(ModifierExpressionPtr modifiers);
  ModifierExpressionPtr getModifiers() { return m_modifiers;}

  /**
   * Movable includes.
   */
  void addMovableInclude(StatementPtr include);
  StatementListPtr getMovableIncludes() { return m_includes;}

  /**
   * Triggers type inference of all statements inside this block.
   */
  void inferTypes(AnalysisResultPtr ar);

  /**
   * Generate constant and variable declarations.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual bool inPseudoMain() {
    return false;
  }

  virtual ClassScopePtr getParentScope(AnalysisResultPtr ar) {
    return ClassScopePtr();
  }

protected:
  std::string m_name;
  std::string m_docComment;
  StatementPtr m_stmt;
  KindOf m_kind;
  VariableTablePtr m_variables;
  ConstantTablePtr m_constants;

  int m_loopNestedLevel;
  int m_incLevel;
  ModifierExpressionPtr m_modifiers;
  StatementListPtr m_includes;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BLOCK_SCOPE_H__
