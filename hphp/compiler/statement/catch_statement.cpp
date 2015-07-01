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

#include "hphp/compiler/statement/catch_statement.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

CatchStatement::CatchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &variable,
 StatementPtr stmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(CatchStatement)),
    StaticClassName(std::make_shared<ScalarExpression>(
                      scope, r, T_STRING, className, false)),
    m_variable(std::make_shared<SimpleVariable>(scope, r, variable)),
    m_stmt(stmt), m_valid(true) {
  m_variable->setContext(Expression::LValue);
}

CatchStatement::CatchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 const std::string &className, const std::string &variable,
 StatementPtr stmt, StatementPtr finallyStmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(CatchStatement)),
    StaticClassName(std::make_shared<ScalarExpression>(
                      scope, r, T_STRING, className, false)),
    m_variable(std::make_shared<SimpleVariable>(scope, r, variable)),
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
// static analysis functions

void CatchStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_variable->analyzeProgram(ar);
  (void)resolveClass();
  if (m_stmt) m_stmt->analyzeProgram(ar);
  if (m_variable->isThis()) {
    getFunctionScope()->setContainsBareThis(true, true);
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
      assert(false);
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
      m_variable = dynamic_pointer_cast<SimpleVariable>(cp);
      break;
    case 1:
      m_stmt = dynamic_pointer_cast<Statement>(cp);
      break;
    case 2:
      m_finallyStmt = dynamic_pointer_cast<Statement>(cp);
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void CatchStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("CatchStatement", 4);
  cg.printPropertyHeader("class");
  cg.printTypeExpression(m_origClassName);
  cg.printPropertyHeader("variableName");
  cg.printValue(m_variable->getName());
  cg.printPropertyHeader("block");
  cg.printAsEnclosedBlock(m_stmt);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
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
