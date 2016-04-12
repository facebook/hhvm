/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
  if (m_cases && m_exp->is(Expression::KindOfSimpleVariable)) {
    for (int i = m_cases->getCount(); i--; ) {
      auto c = dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
      if (c->getCondition() && c->getCondition()->hasEffect()) {
        m_exp->setContext(Expression::LValue);
        m_exp->setContext(Expression::NoLValueWrapper);
        break;
      }
    }
  }
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

void SwitchStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
  if (m_cases) m_cases->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll &&
      m_exp->is(Expression::KindOfSimpleVariable)) {
    auto exp = dynamic_pointer_cast<SimpleVariable>(m_exp);
    if (exp && exp->getSymbol() && exp->getSymbol()->isClassName()) {
      // Mark some classes as volatile since the name is used in switch
      for (int i = 0; i < m_cases->getCount(); i++) {
        auto stmt = dynamic_pointer_cast<CaseStatement>((*m_cases)[i]);
        assert(stmt);
        ExpressionPtr caseCond = stmt->getCondition();
        if (caseCond && caseCond->isScalar()) {
          auto name = dynamic_pointer_cast<ScalarExpression>(caseCond);
          if (name && name->isLiteralString()) {
            string className = name->getLiteralString();
            ClassScopePtr cls = ar->findClass(toLower(className));
            if (cls && cls->isUserClass()) {
              cls->setVolatile();
            }
          }
        }
      }
      // Also note this down as code error
      ConstructPtr self = shared_from_this();
      Compiler::Error(Compiler::ConditionalClassLoading, self);
    }
  }
}

bool SwitchStatement::hasDecl() const {
  return m_cases && m_cases->hasDecl();
}

bool SwitchStatement::hasRetExp() const {
  return m_cases && m_cases->hasRetExp();
}

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
