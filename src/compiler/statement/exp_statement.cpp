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

#include <compiler/statement/exp_statement.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/include_expression.h>
#include <compiler/option.h>
#include <compiler/parser/hphp.tab.hpp>
#include <compiler/expression/function_call.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/alias_manager.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ExpStatement::ExpStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

StatementPtr ExpStatement::clone() {
  ExpStatementPtr stmt(new ExpStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ExpStatement::analyzeAutoload(AnalysisResultPtr ar) {
  if (!m_exp->is(Expression::KindOfAssignmentExpression)) return;

  AssignmentExpressionPtr assign =
    dynamic_pointer_cast<AssignmentExpression>(m_exp);
  if (!assign->getVariable()->is(Expression::KindOfArrayElementExpression)) {
    return;
  }
  if (!assign->getValue()->is(Expression::KindOfScalarExpression) ||
      !dynamic_pointer_cast<ScalarExpression>
      (assign->getValue())->isLiteralString()) {
    return;
  }

  string file = assign->getValue()->getLiteralString();
  if (file.empty()) return;

  string s = assign->getText();
  string path = Option::GetAutoloadRoot(s);
  if (path.empty()) return;

  if (path[path.length() - 1] != '/' && file[0] != '/') {
    file = path + '/' + file;
  } else {
    file = path + file;
  }
  ScalarExpressionPtr exp
    (new ScalarExpression(assign->getValue()->getLocation(),
                          Expression::KindOfScalarExpression,
                          T_STRING, file, true));
  IncludeExpressionPtr include
    (new IncludeExpression(assign->getLocation(),
                           Expression::KindOfIncludeExpression,
                           exp, T_INCLUDE_ONCE));
  include->setDocumentRoot(); // autoload always starts from document root
  include->onParse(ar);
  m_exp = include;
}

void ExpStatement::analyzeShortCircuit(AnalysisResultPtr ar) {
  if (!m_exp->is(Expression::KindOfBinaryOpExpression)) return;

  BinaryOpExpressionPtr exp = dynamic_pointer_cast<BinaryOpExpression>(m_exp);
  if (exp->isShortCircuitOperator()) {
    FunctionCallPtr call = dynamic_pointer_cast<FunctionCall>(exp->getExp2());
    if (call) {
      call->setAllowVoidReturn();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ExpStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr ExpStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int ExpStatement::getKidCount() const {
  return 1;
}

void ExpStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr ExpStatement::preOptimize(AnalysisResultPtr ar) {
  if (ar->getPhase() != AnalysisResult::AnalyzeInclude) {
    m_exp = m_exp->unneeded(ar);
  }
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr ExpStatement::postOptimize(AnalysisResultPtr ar) {
  m_exp = m_exp->unneeded(ar);
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void ExpStatement::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, NEW_TYPE(Any), false);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ExpStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_exp->outputPHP(cg, ar);
  cg.printf(";\n");
}

void ExpStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (hasEffect() || Option::KeepStatementsWithNoEffect) {
    m_exp->outputCPPBegin(cg, ar);
    m_exp->outputCPPUnneeded(cg, ar);
    cg.printf(";\n");
    m_exp->outputCPPEnd(cg, ar);
  } else {
    cg.printf(";\n");
  }
}
