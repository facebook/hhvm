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

#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/class_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TraitAliasStatement::TraitAliasStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                                         ScalarExpressionPtr expTraitName,
                                         ScalarExpressionPtr expMethodName,
                                         ModifierExpressionPtr expModifiers,
                                         ScalarExpressionPtr expNewMethodName)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TraitAliasStatement)),
      m_traitName(expTraitName), m_methodName(expMethodName),
      m_modifiers(expModifiers), m_newMethodName(expNewMethodName) {
}

StatementPtr TraitAliasStatement::clone() {
  TraitAliasStatementPtr new_stmt(new TraitAliasStatement(*this));
  new_stmt->m_traitName = Clone(m_traitName);
  new_stmt->m_methodName = Clone(m_methodName);
  new_stmt->m_modifiers = Clone(m_modifiers);
  new_stmt->m_newMethodName = Clone(m_newMethodName);
  return new_stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void TraitAliasStatement::analyzeProgram(AnalysisResultPtr ar) {
  // Analyze children
  m_traitName->analyzeProgram(ar);
  m_methodName->analyzeProgram(ar);
  m_modifiers->analyzeProgram(ar);
  m_newMethodName->analyzeProgram(ar);
}

ConstructPtr TraitAliasStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_traitName;
    case 1:
      return m_methodName;
    case 2:
      return m_modifiers;
    case 3:
      return m_newMethodName;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int TraitAliasStatement::getKidCount() const {
  return 4;
}

void TraitAliasStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_traitName = dynamic_pointer_cast<ScalarExpression>(cp);
      break;
    case 1:
      m_methodName = dynamic_pointer_cast<ScalarExpression>(cp);
      break;
    case 2:
      m_modifiers = dynamic_pointer_cast<ModifierExpression>(cp);
      break;
    case 3:
      m_newMethodName = dynamic_pointer_cast<ScalarExpression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void TraitAliasStatement::outputCodeModel(CodeGenerator &cg) {
  auto propCount = 3;
  auto traitName = m_traitName->getString();
  if (!traitName.empty()) propCount++;
  cg.printObjectHeader("TraitAliasStatement", propCount);
  if (!traitName.empty()) {
    cg.printPropertyHeader("traitName");
    cg.printValue(traitName);
  }
  cg.printPropertyHeader("methodName1");
  m_methodName->outputCodeModel(cg);
  cg.printPropertyHeader("modifiers");
  m_modifiers->outputCodeModel(cg);
  cg.printPropertyHeader("methodName2");
  m_newMethodName->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void TraitAliasStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_traitName->getString().empty()) {
    m_traitName->outputPHP(cg, ar);
    cg_printf("::");
  }
  m_methodName->outputPHP(cg, ar);
  cg_printf(" as ");
  m_modifiers->outputPHP(cg, ar);
  m_newMethodName->outputPHP(cg, ar);
  cg_printf(";\n");
}
