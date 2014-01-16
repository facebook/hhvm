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

#include "hphp/compiler/expression/group_clause.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

GroupClause::GroupClause
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
    ExpressionPtr coll, ExpressionPtr key)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(JoinClause)),
    m_coll(coll), m_key(key) {
}

ExpressionPtr GroupClause::clone() {
  GroupClausePtr exp(new GroupClause(*this));
  Expression::deepCopy(exp);
  exp->m_coll = Clone(m_coll);
  exp->m_key = Clone(m_key);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void GroupClause::analyzeProgram(AnalysisResultPtr ar) {
  m_coll->analyzeProgram(ar);
  m_key->analyzeProgram(ar);
}

ConstructPtr GroupClause::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_coll;
    case 1:
      return m_key;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int GroupClause::getKidCount() const {
  return 2;
}

void GroupClause::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_coll = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_key = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

TypePtr GroupClause::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                bool coerce) {
  m_coll->inferAndCheck(ar, Type::Some, false);
  m_key->inferAndCheck(ar, Type::Some, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void GroupClause::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("GroupClause", 3);
  cg.printPropertyHeader("collection");
  m_coll->outputCodeModel(cg);
  cg.printPropertyHeader("key");
  m_key->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void GroupClause::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("group ");
  m_coll->outputPHP(cg, ar);
  cg_printf(" by ");
  m_key->outputPHP(cg, ar);
}

