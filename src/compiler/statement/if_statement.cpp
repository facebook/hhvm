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

#include <compiler/statement/if_statement.h>
#include <compiler/statement/if_branch_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/statement/block_statement.h>
#include <compiler/analysis/function_scope.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IfStatement::IfStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, StatementListPtr stmts)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_stmts(stmts) {
}

StatementPtr IfStatement::clone() {
  IfStatementPtr stmt(new IfStatement(*this));
  stmt->m_stmts = Clone(m_stmts);
  return stmt;
}

int IfStatement::getRecursiveCount() const {
  return m_stmts->getRecursiveCount();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void IfStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_stmts) m_stmts->analyzeProgram(ar);
}

bool IfStatement::hasDecl() const {
  return m_stmts && m_stmts->hasDecl();
}

bool IfStatement::hasRetExp() const {
  return m_stmts && m_stmts->hasRetExp();
}

ConstructPtr IfStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmts;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int IfStatement::getKidCount() const {
  return 1;
}

void IfStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmts = boost::dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr IfStatement::preOptimize(AnalysisResultConstPtr ar) {
  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return StatementPtr();
  }

  // we cannot optimize away the code inside if statement, because
  // there may be a goto that goes into if statement.
  if (hasReachableLabel()) {
    return StatementPtr();
  }

  bool changed = false;
  int i;
  int j;
  Variant value;
  bool hoist = false;
  for (i = 0; i < m_stmts->getCount(); i++) {
    IfBranchStatementPtr branch =
      dynamic_pointer_cast<IfBranchStatement>((*m_stmts)[i]);
    ExpressionPtr condition = branch->getCondition();
    if (!condition) {
      StatementPtr stmt = branch->getStmt();
      if (stmt) {
        if (!i &&
            ((getFunctionScope() && !getFunctionScope()->inPseudoMain()) ||
             !stmt->hasDecl())) {
          hoist = true;
          break;
        }
        if (stmt->is(KindOfIfStatement)) {
          StatementListPtr sub_stmts =
            dynamic_pointer_cast<IfStatement>(stmt)->m_stmts;
          m_stmts->removeElement(i);
          changed = true;
          for (j = 0; j < sub_stmts->getCount(); j++) {
            m_stmts->insertElement((*sub_stmts)[j], i++);
          }
        }
      }
      break;
    } else if (condition->getEffectiveScalar(value)) {
      if (value.toBoolean()) {
        hoist = !i &&
          ((getFunctionScope() && !getFunctionScope()->inPseudoMain()) ||
           !branch->hasDecl());
        break;
      } else if (!condition->hasEffect()) {
        m_stmts->removeElement(i--);
        changed = true;
      } else if (branch->getStmt()) {
        branch->clearStmt();
        changed = true;
      }
    }
  }

  if (!changed && i && i == m_stmts->getCount()) return StatementPtr();

  // either else branch or if (true) branch without further declarations

  i++;
  while (i < m_stmts->getCount()) {
    m_stmts->removeElement(i);
    changed = true;
  }

  // if there is only one branch left, return stmt.
  if (hoist) {
    IfBranchStatementPtr branch =
      dynamic_pointer_cast<IfBranchStatement>((*m_stmts)[0]);
    return branch->getStmt() ? branch->getStmt() : NULL_STATEMENT();
  } else if (m_stmts->getCount() == 0) {
    return NULL_STATEMENT();
  } else {
    return changed ? static_pointer_cast<Statement>(shared_from_this())
                   : StatementPtr();
  }
}

StatementPtr IfStatement::postOptimize(AnalysisResultConstPtr ar) {
  // we cannot optimize away the code inside if statement, because
  // there may be a goto that goes into if statement.
  if (hasReachableLabel()) {
    return StatementPtr();
  }

  bool changed = false;
  for (int i = 0; i < m_stmts->getCount(); i++) {
    IfBranchStatementPtr branch =
      dynamic_pointer_cast<IfBranchStatement>((*m_stmts)[i]);
    ExpressionPtr condition = branch->getCondition();
    if (!branch->getStmt() || !branch->getStmt()->hasImpl()) {
      if (!condition ||
          (i == m_stmts->getCount() - 1 &&
           !condition->hasEffect())) {
        // remove else branch without C++ implementation.
        m_stmts->removeElement(i);
        changed = true;
      } else if (condition->is(Expression::KindOfConstantExpression)) {
        ConstantExpressionPtr exp =
          dynamic_pointer_cast<ConstantExpression>(condition);
        // Remove if (false) branch without C++ implementation.
        // if (true) branch without C++ implementation is kept unless
        // it is the last branch. In general we cannot let a if (true)
        // branch short-circuit the rest branches which if removed may
        // cause g++ to complain unreferenced variables.
        if (exp->isBoolean()) {
          if (!exp->getBooleanValue() ||
              (exp->getBooleanValue() && i == m_stmts->getCount() - 1)) {
            m_stmts->removeElement(i);
            changed = true;
            i--;
          }
        }
      }
    }
  }
  if (m_stmts->getCount() == 0) {
    return NULL_STATEMENT();
  } else {
    return changed ? static_pointer_cast<Statement>(shared_from_this())
                   : StatementPtr();
  }
}

void IfStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmts) m_stmts->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void IfStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (int i = 0; i < m_stmts->getCount(); i++) {
    if (i > 0) cg_printf("else");
    (*m_stmts)[i]->outputPHP(cg, ar);
  }
}

void IfStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  int indent = 0;
  for (int i = 0; i < m_stmts->getCount(); i++) {
    if (i > 0) cg_printf("else ");
    indent += dynamic_pointer_cast<IfBranchStatement>((*m_stmts)[i])->
      outputCPPIfBranch(cg, ar);
  }
  while (indent--) cg_indentEnd("}\n");
}
