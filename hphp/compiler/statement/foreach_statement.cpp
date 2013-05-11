/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <compiler/statement/foreach_statement.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/option.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <util/util.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ForEachStatement::ForEachStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr array, ExpressionPtr name, bool nameRef,
 ExpressionPtr value, bool valueRef, StatementPtr stmt)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ForEachStatement)),
    m_array(array), m_name(name), m_value(value), m_ref(valueRef),
    m_stmt(stmt) {
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
    m_value->setContext(Expression::NoRefWrapper);
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
      m_array = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_name = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 3:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void ForEachStatement::inferTypes(AnalysisResultPtr ar) {
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());

  m_array->inferAndCheck(ar, m_ref ? Type::Variant : Type::Array, m_ref);
  if (m_name) {
    m_name->inferAndCheck(ar, Type::Primitive, true);
  }
  m_value->inferAndCheck(ar, Type::Variant, true);
  if (m_ref) {
    TypePtr actualType = m_array->getActualType();
    if (!actualType ||
        actualType->is(Type::KindOfVariant) ||
        actualType->is(Type::KindOfObject)) {
      ar->forceClassVariants(getClassScope(), false, true);
    }
  }
  if (m_stmt) {
    getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    getScope()->decLoopNestedLevel();
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ForEachStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("foreach (");
  m_array->outputPHP(cg, ar);
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
