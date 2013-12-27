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

#include "hphp/compiler/expression/simple_query_clause.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SimpleQueryClause::SimpleQueryClause
(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS,
 const std::string &identifier, ExpressionPtr expression)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES) {
  m_identifier = identifier;
  m_expression = expression;
}

ExpressionPtr SimpleQueryClause::clone() {
  assert(false);
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleQueryClause::analyzeProgram(AnalysisResultPtr ar) {
  m_expression->analyzeProgram(ar);
}

ConstructPtr SimpleQueryClause::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_expression;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int SimpleQueryClause::getKidCount() const {
  return 1;
}

void SimpleQueryClause::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_expression = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

TypePtr SimpleQueryClause::inferTypes(AnalysisResultPtr ar,
  TypePtr type, bool coerce) {
  m_expression->inferAndCheck(ar, Type::Some, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void SimpleQueryClause::outputCodeModel(CodeGenerator &cg) {
  switch (this->getKindOf()) {
    case Expression::KindOfFromClause:
      cg.printObjectHeader("FromClause", 3);
      cg.printPropertyHeader("identifier");
      cg.printValue(m_identifier);
      cg.printPropertyHeader("collection");
      break;
    case Expression::KindOfLetClause:
      cg.printObjectHeader("LetClause", 3);
      cg.printPropertyHeader("identifier");
      cg.printValue(m_identifier);
      cg.printPropertyHeader("expression");
      break;
    case Expression::KindOfIntoClause:
      cg.printObjectHeader("IntoClause", 3);
      cg.printPropertyHeader("identifier");
      cg.printValue(m_identifier);
      cg.printPropertyHeader("query");
      break;
    case Expression::KindOfWhereClause:
      cg.printObjectHeader("WhereClause", 2);
      cg.printPropertyHeader("condition");
      break;
    case Expression::KindOfSelectClause:
      cg.printObjectHeader("SelectClause", 2);
      cg.printPropertyHeader("expression");
      break;
    default:
      assert(false);
      break;
  }
  m_expression->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleQueryClause::outputPHP(
  CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (this->getKindOf()) {
    case Expression::KindOfFromClause:
      cg_printf("from %s in ", m_identifier.c_str());
      break;
    case Expression::KindOfLetClause:
      cg_printf("let %s in ", m_identifier.c_str());
      break;
    case Expression::KindOfIntoClause:
      cg_printf("into %s ", m_identifier.c_str());
      break;
    case Expression::KindOfWhereClause:
      cg_printf("where ");
      break;
    case Expression::KindOfSelectClause:
      cg_printf("select ");
      break;
    default:
      assert(false);
      break;
  }
  m_expression->outputPHP(cg, ar);
}

