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

#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/class_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TraitPrecStatement::TraitPrecStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                                       ScalarExpressionPtr expTraitName,
                                       ScalarExpressionPtr expMethodName,
                                       ExpressionListPtr expOtherTraitNames)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TraitPrecStatement)),
      m_traitName(expTraitName), m_methodName(expMethodName),
      m_otherTraitNames(expOtherTraitNames) {
}

StatementPtr TraitPrecStatement::clone() {
  TraitPrecStatementPtr new_stmt(new TraitPrecStatement(*this));
  new_stmt->m_traitName = Clone(m_traitName);
  new_stmt->m_methodName = Clone(m_methodName);
  new_stmt->m_otherTraitNames = Clone(m_otherTraitNames);
  return new_stmt;
}

void TraitPrecStatement::getOtherTraitNames(std::set<string> &namesSet) const {
  vector<string> namesVec;
  m_otherTraitNames->getStrings(namesVec);
  for (unsigned int i = 0; i < namesVec.size(); i++) {
    namesVec[i] = Util::toLower(namesVec[i]);
  }
  namesSet.clear();
  namesSet.insert(namesVec.begin(), namesVec.end());
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void TraitPrecStatement::analyzeProgram(AnalysisResultPtr ar) {
  // Analyze children
  m_traitName->analyzeProgram(ar);
  m_methodName->analyzeProgram(ar);
  for (int i = 0; i < m_otherTraitNames->getCount(); i++) {
    (*m_otherTraitNames)[i]->analyzeProgram(ar);
  }
}

ConstructPtr TraitPrecStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_traitName;
    case 1:
      return m_methodName;
    case 2:
      return m_otherTraitNames;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int TraitPrecStatement::getKidCount() const {
  return 3;
}

void TraitPrecStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_traitName = dynamic_pointer_cast<ScalarExpression>(cp);
      break;
    case 1:
      m_methodName = dynamic_pointer_cast<ScalarExpression>(cp);
      break;
    case 2:
      m_otherTraitNames = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void TraitPrecStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////

void TraitPrecStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("TraitInsteadStatement", 3);
  cg.printPropertyHeader("traitName");
  m_traitName->outputCodeModel(cg);
  cg.printPropertyHeader("methodName");
  m_methodName->outputCodeModel(cg);
  cg.printPropertyHeader("otherTraitNames");
  cg.printExpressionVector(m_otherTraitNames);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void TraitPrecStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_traitName->outputPHP(cg, ar);
  cg_printf("::");
  m_methodName->outputPHP(cg, ar);
  cg_printf(" insteadof ");
  for (int i = 0; i < m_otherTraitNames->getCount(); i++) {
    if (i > 0) cg_printf(", ");
    (*m_otherTraitNames)[i]->outputPHP(cg, ar);
  }
  cg_printf(";\n");
}
