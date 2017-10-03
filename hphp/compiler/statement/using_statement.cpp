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

#include "hphp/compiler/statement/using_statement.h"

namespace HPHP {

UsingStatement::UsingStatement(
  STATEMENT_CONSTRUCTOR_PARAMETERS,
  bool isAsync, bool isWholeFunc, ExpressionListPtr expr, StatementListPtr body
) : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(UsingStatement))
  , m_isAsync(isAsync)
  , m_isWholeFunc(isWholeFunc)
  , m_expr(std::move(expr))
  , m_body(std::move(body))
{
}

StatementPtr UsingStatement::clone() {
  auto stmt = std::make_shared<UsingStatement>(*this);
  stmt->m_expr = Clone(m_expr);
  stmt->m_body = Clone(m_body);
  return stmt;
}

ConstructPtr UsingStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_expr;
    case 1:
      return m_body;
    default:
      break;
  }
  always_assert(false);
}

int UsingStatement::getKidCount() const {
  return 2;
}

void UsingStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_expr = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 1:
      m_body = dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      always_assert(false);
  }
}

void UsingStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("%susing ", m_isAsync ? "await " : "");

  if (!m_body) {
    m_expr->outputPHP(cg, ar);
    cg_printf(";\n");
    return;
  }

  cg_printf("(");
  m_expr->outputPHP(cg, ar);
  cg_indentBegin(") {\n");
  m_body->outputPHP(cg, ar);
  cg_indentEnd("}\n");
}

}
