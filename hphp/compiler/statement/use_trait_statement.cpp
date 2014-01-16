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

#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/class_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UseTraitStatement::UseTraitStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp,
 StatementListPtr stmt)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(UseTraitStatement)),
      m_exp(exp), m_stmt(stmt) {
}

StatementPtr UseTraitStatement::clone() {
  UseTraitStatementPtr new_stmt(new UseTraitStatement(*this));
  new_stmt->m_exp = Clone(m_exp);
  new_stmt->m_stmt = Clone(m_stmt);
  return new_stmt;
}


///////////////////////////////////////////////////////////////////////////////
// parser functions

void UseTraitStatement::onParseRecur(AnalysisResultConstPtr ar,
                                     ClassScopePtr scope) {
  if (scope->isInterface()) {
    parseTimeFatal(Compiler::InvalidTraitStatement,
                   "Interfaces cannot use traits");
  }
  vector<string> usedTraits;
  getUsedTraitNames(usedTraits);
  for (auto &t : usedTraits) {
    ar->parseOnDemandByClass(Util::toLower(t));
  }
  scope->addUsedTraits(usedTraits);
}


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void UseTraitStatement::analyzeProgram(AnalysisResultPtr ar) {
  // Analyze children
  for (int i = 0; i < m_exp->getCount(); i++) {
    (*m_exp)[i]->analyzeProgram(ar);
  }
  if (m_stmt) m_stmt->analyzeProgram(ar);

  // At the final pass, collect info & insert trait into target class
  if (ar->getPhase() != AnalysisResult::AnalyzeAll) return;
}

ConstructPtr UseTraitStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    case 1:
      return m_stmt;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int UseTraitStatement::getKidCount() const {
  return 2;
}

void UseTraitStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 1:
      m_stmt = dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void UseTraitStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////

void UseTraitStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("UseTraitStatement", 3);
  cg.printPropertyHeader("expressions");
  cg.printExpressionVector(m_exp);
  cg.printPropertyHeader("block");
  cg.printAsBlock(m_stmt);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UseTraitStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("use ");
  m_exp->outputPHP(cg, ar);
  cg_printf(" {\n");
  m_stmt->outputPHP(cg, ar);
  cg_printf("}\n");
}
