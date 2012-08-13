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

#include <compiler/statement/catch_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/code_error.h>
#include <util/util.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/option.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

CatchStatement::CatchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &variable,
 StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(CatchStatement)),
    StaticClassName(ExpressionPtr(
                      new ScalarExpression(scope, loc,
                                           T_STRING, className, false))),
    m_variable(new SimpleVariable(scope, loc, variable)),
    m_stmt(stmt), m_valid(true) {
  m_variable->setContext(Expression::LValue);
}

CatchStatement::CatchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &variable,
 StatementPtr stmt, StatementPtr finallyStmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(CatchStatement)),
    StaticClassName(ExpressionPtr(
                      new ScalarExpression(scope, loc,
                                           T_STRING, className, false))),
    m_variable(new SimpleVariable(scope, loc, variable)),
    m_stmt(stmt), m_finallyStmt(finallyStmt), m_valid(true) {
  m_variable->setContext(Expression::LValue);
}

StatementPtr CatchStatement::clone() {
  CatchStatementPtr stmt(new CatchStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_variable = Clone(m_variable);
  stmt->m_finallyStmt = Clone(m_finallyStmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void CatchStatement::analyzeProgram(AnalysisResultPtr ar) {
  addUserClass(ar, m_className);
  m_variable->analyzeProgram(ar);
  (void)resolveClass();
  if (m_stmt) m_stmt->analyzeProgram(ar);
  if (m_variable->isThis()) {
    // catch (Exception $this) { ... }
    // See note in alias_manager.cpp about why this forces a variable table
    VariableTablePtr variables(getScope()->getVariables());
    variables->forceVariants(ar, VariableTable::AnyVars);
    variables->setAttribute(VariableTable::ContainsLDynamicVariable);
  }
}

ConstructPtr CatchStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variable;
    case 1:
      return m_stmt;
    case 2:
      return m_finallyStmt;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int CatchStatement::getKidCount() const {
  return 2 + (m_finallyStmt ? 1 : 0);
}

void CatchStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variable = boost::dynamic_pointer_cast<SimpleVariable>(cp);
      break;
    case 1:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    case 2:
      m_finallyStmt = boost::dynamic_pointer_cast<Statement>(cp);
    default:
      ASSERT(false);
      break;
  }
}

void CatchStatement::inferTypes(AnalysisResultPtr ar) {
  ClassScopePtr cls = resolveClassWithChecks();
  TypePtr type;
  m_valid = cls || isRedeclared();

  // This can never be a specific exception type, because a future exception
  // class may be re-declaring, then generated code like this won't work with
  // DynamicObjectData: p_exception v_e = e;
  type = Type::Object;

  m_variable->inferAndCheck(ar, type, true);
  if (m_stmt) m_stmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void CatchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf(" catch (%s $%s) ", m_origClassName.c_str(),
            m_variable->getName().c_str());
  cg_indentBegin("{\n");
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}");
}

void CatchStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_valid) {
    cg_printf("if (e.instanceof(");
    cg_printString(m_className, ar, shared_from_this());
    cg_indentBegin(")) {\n");
    VariableTablePtr variables = getScope()->getVariables();
    ASSERT(m_variable->hasAssignableCPPVariable());
    const string &cppName =
      m_variable->getAssignableCPPVariable(ar);
    cg_printf("%s = e;\n", cppName.c_str());
  } else {
    cg_indentBegin("if (false) {\n");
  }
  if (m_stmt) m_stmt->outputCPP(cg, ar);
  cg_indentEnd("}");
}
