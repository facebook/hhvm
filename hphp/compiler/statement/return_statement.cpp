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

#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/function_call.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ReturnStatement::ReturnStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ReturnStatement)),
    m_exp(exp) {
  if (exp) exp->setContext(Expression::ReturnContext);
}

StatementPtr ReturnStatement::clone() {
  ReturnStatementPtr stmt(new ReturnStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ReturnStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_exp) {
    FunctionScopePtr funcScope = getFunctionScope();
    if (funcScope) {
      if (funcScope->isRefReturn()) {
        m_exp->setContext(Expression::RefValue);
      }
    }
    m_exp->analyzeProgram(ar);
  }
}

ConstructPtr ReturnStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ReturnStatement::getKidCount() const {
  return 1;
}

void ReturnStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ReturnStatement::outputCodeModel(CodeGenerator &cg) {
  auto nump = m_exp ? 2 : 1;
  cg.printObjectHeader("ReturnStatement", nump);
  if (m_exp) {
    cg.printPropertyHeader("expression");
    m_exp->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ReturnStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp) {
    cg_printf("return ");
    m_exp->outputPHP(cg, ar);
    cg_printf(";\n");
  } else {
    cg_printf("return;\n");
  }
}
