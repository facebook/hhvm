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

#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ForEachStatement::ForEachStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr array, ExpressionPtr name, bool nameRef,
 ExpressionPtr value, bool valueRef, bool awaitAs, StatementPtr stmt)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ForEachStatement)),
    m_array(array), m_name(name), m_value(value), m_ref(valueRef),
    m_awaitAs(awaitAs), m_stmt(stmt) {
  if (!m_value) {
    m_value = m_name;
    m_ref = nameRef;
    m_name.reset();
  }
  if (m_name) {
    m_name->setContext(Expression::LValue);
    m_name->setContext(Expression::NoLValueWrapper);
  }
  m_value->setContext(Expression::LValue);
  m_value->setContext(Expression::NoLValueWrapper);
  if (m_ref) {
    m_array->setContext(Expression::RefValue);
    m_value->setContext(Expression::RefValue);
  }
}

StatementPtr ForEachStatement::clone() {
  ForEachStatementPtr stmt(new ForEachStatement(*this));
  stmt->m_array = Clone(m_array);
  stmt->m_name = Clone(m_name);
  stmt->m_value = Clone(m_value);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ForEachStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_array->analyzeProgram(ar);
  if (m_name) m_name->analyzeProgram(ar);
  m_value->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr ForEachStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_array;
    case 1:
      return m_name;
    case 2:
      return m_value;
    case 3:
      return m_stmt;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ForEachStatement::getKidCount() const {
  return 4;
}

void ForEachStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_array = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_name = dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_value = dynamic_pointer_cast<Expression>(cp);
      break;
    case 3:
      m_stmt = dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ForEachStatement::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 4;
  if (m_name != nullptr) numProps++;
  cg.printObjectHeader("ForeachStatement", numProps);
  cg.printPropertyHeader("collection");
  m_array->outputCodeModel(cg);
  if (m_name != nullptr) {
    cg.printPropertyHeader("key");
    m_name->outputCodeModel(cg);
  }
  cg.printPropertyHeader("value");
  cg.printExpression(m_value, m_ref);
  cg.printPropertyHeader("block");
  cg.printAsBlock(m_stmt);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ForEachStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("foreach (");
  m_array->outputPHP(cg, ar);
  if (m_awaitAs) cg_printf(" await");
  cg_printf(" as ");
  if (m_name) {
    m_name->outputPHP(cg, ar);
    cg_printf(" => ");
  }
  if (m_ref) cg_printf("&");
  m_value->outputPHP(cg, ar);
  cg_printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
}
