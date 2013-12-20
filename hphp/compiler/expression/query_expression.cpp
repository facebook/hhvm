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

#include "hphp/compiler/expression/query_expression.h"
#include "hphp/compiler/expression/simple_query_clause.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

QueryOrderby::QueryOrderby(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES) {
  m_expressions = nullptr;
}

ExpressionPtr QueryOrderby::clone() {
  assert(false);
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void QueryOrderby::analyzeProgram(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_expressions->getCount(); i++) {
    (*m_expressions)[i]->analyzeProgram(ar);
  }
}

ConstructPtr QueryOrderby::getNthKid(int n) const {
  if (n < (int)m_expressions->getCount()) {
    return (*m_expressions)[n];
  }
  return ConstructPtr();
}

int QueryOrderby::getKidCount() const {
  return m_expressions->getCount();
}

void QueryOrderby::setNthKid(int n, ConstructPtr cp) {
  int m = m_expressions->getCount();
  if (n >= m) {
    assert(false);
  } else {
    (*m_expressions)[n] = dynamic_pointer_cast<Expression>(cp);
  }
}

TypePtr QueryOrderby::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  for (unsigned int i = 0; i < m_expressions->getCount(); i++) {
    if (ExpressionPtr e = (*m_expressions)[i]) {
      e->inferAndCheck(ar, Type::Some, false);
    }
  }
  return Type::Object;
}

ExpressionPtr QueryExpression::getCollection() const {
  assert(m_expressions->getCount() > 0);
  assert((*m_expressions)[0]->getKindOf() == Expression::KindOfFromClause);
  auto fromClause = static_pointer_cast<FromClause>((*m_expressions)[0]);
  return fromClause->getExpression();
}

///////////////////////////////////////////////////////////////////////////////

void QueryOrderby::outputCodeModel(CodeGenerator &cg) {
  if (this->getKindOf() == Expression::KindOfOrderbyClause) {
    cg.printObjectHeader("OrderbyClause", 2);
  } else {
    cg.printObjectHeader("QueryExpression", 2);
  }
  cg.printPropertyHeader("clauses");
  m_expressions->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void QueryOrderby::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (this->getKindOf() == Expression::KindOfOrderbyClause) {
    cg_printf("orderby ");
  }
  for (unsigned int i = 0; i < m_expressions->getCount(); i++) {
    if (ExpressionPtr e = (*m_expressions)[i]) {
      e->outputPHP(cg, ar);
      if (i > 0) cg_printf(" ");
    }
  }

}

