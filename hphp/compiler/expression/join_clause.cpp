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

#include "hphp/compiler/expression/join_clause.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

JoinClause::JoinClause
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
    const std::string &var, ExpressionPtr coll, ExpressionPtr left,
    ExpressionPtr right, const std::string &group)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(JoinClause)),
    m_var(var), m_coll(coll), m_left(left),
    m_right(right), m_group(group) {
}

ExpressionPtr JoinClause::clone() {
  JoinClausePtr exp(new JoinClause(*this));
  Expression::deepCopy(exp);
  exp->m_var = m_var;
  exp->m_coll = Clone(m_coll);
  if (m_left != nullptr) {
    exp->m_left = Clone(m_left);
  }
  if (m_right != nullptr) {
    exp->m_right = Clone(m_right);
  }
  exp->m_group = m_group;
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void JoinClause::analyzeProgram(AnalysisResultPtr ar) {
  m_coll->analyzeProgram(ar);
  if (m_left != nullptr) m_left->analyzeProgram(ar);
  if (m_right != nullptr) m_right->analyzeProgram(ar);
}

ConstructPtr JoinClause::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_coll;
    case 1:
      return m_left;
    case 2:
      return m_right;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int JoinClause::getKidCount() const {
  int count = 1;
  if (m_left != nullptr) count++;
  if (m_right != nullptr) count++;
  return count;
}

void JoinClause::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_coll = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_left = dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_right = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

TypePtr JoinClause::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  m_coll->inferAndCheck(ar, Type::Some, false);
  if (m_left != nullptr) m_left->inferAndCheck(ar, Type::Some, false);
  if (m_right != nullptr) m_right->inferAndCheck(ar, Type::Some, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void JoinClause::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 2;
  if (!m_var.empty()) numProps++;
  if (m_left != nullptr) numProps++;
  if (m_right != nullptr) numProps++;
  if (!m_group.empty()) numProps++;
  cg.printObjectHeader("JoinClause", numProps);
  if (!m_var.empty()) {
    cg.printPropertyHeader("identifier");
    cg.printValue(m_var);
  }
  cg.printPropertyHeader("collection");
  m_coll->outputCodeModel(cg);
  if (m_left != nullptr) {
    cg.printPropertyHeader("left");
    m_left->outputCodeModel(cg);
  }
  if (m_right != nullptr) {
    cg.printPropertyHeader("right");
    m_right->outputCodeModel(cg);
  }
  if (!m_group.empty()) {
    cg.printPropertyHeader("group");
    cg.printValue(m_group);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void JoinClause::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_var.empty()) {
    cg_printf("join ");
  } else {
    cg_printf("join %s in ", m_var.c_str());
  }
  m_coll->outputPHP(cg, ar);
  if (m_left != nullptr) {
    cg_printf(" on ");
    m_left->outputPHP(cg, ar);
  }
  if (m_right != nullptr) {
    cg_printf(" equals ");
    m_right->outputPHP(cg, ar);
  }
  if (!m_group.empty()) {
    cg_printf(" into %s", m_group.c_str());
  }
}

