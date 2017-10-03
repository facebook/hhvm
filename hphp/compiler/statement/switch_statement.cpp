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

#include "hphp/compiler/statement/switch_statement.h"
#include <set>
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/comparisons.h"

using namespace HPHP;
using std::string;
using std::set;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SwitchStatement::SwitchStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, StatementListPtr cases)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(SwitchStatement)),
    m_exp(exp), m_cases(cases) {
}

StatementPtr SwitchStatement::clone() {
  SwitchStatementPtr stmt(new SwitchStatement(*this));
  stmt->m_cases = Clone(m_cases);
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

int SwitchStatement::getRecursiveCount() const {
  return 1 + (m_cases ? m_cases->getRecursiveCount() : 0);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ConstructPtr SwitchStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    case 1:
      return m_cases;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int SwitchStatement::getKidCount() const {
  return 2;
}

void SwitchStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_cases = dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SwitchStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("switch (");
  m_exp->outputPHP(cg, ar);
  cg_printf(") {\n");
  if (m_cases) m_cases->outputPHP(cg, ar);
  cg_printf("}\n");
}
