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

#include "hphp/compiler/expression/ordering.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

Ordering::Ordering
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr key, TokenID direction)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(Ordering)),
    m_key(key), m_direction(direction){
}

ExpressionPtr Ordering::clone() {
  OrderingPtr exp(new Ordering(*this));
  Expression::deepCopy(exp);
  exp->m_key = Clone(m_key);
  exp->m_direction = m_direction;
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void Ordering::analyzeProgram(AnalysisResultPtr ar) {
  m_key->analyzeProgram(ar);
}

ConstructPtr Ordering::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_key;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int Ordering::getKidCount() const {
  return 1;
}

void Ordering::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_key = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      break;
  }
}

TypePtr Ordering::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  m_key->inferAndCheck(ar, Type::Some, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void Ordering::outputCodeModel(CodeGenerator &cg) {
  int direction;
  switch (m_direction) {
  case T_ASCENDING:
    direction = 1;
    break;
  case T_DESCENDING:
    direction = 2;
    break;
  default:
    direction = 3;
    break;
  }
  auto propCount = direction > 0 ? 3 : 2;
  cg.printObjectHeader("Ordering", propCount);
  cg.printPropertyHeader("key");
  m_key->outputCodeModel(cg);
  if (propCount == 3) {
    cg.printPropertyHeader("direction");
    cg.printValue(direction);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void Ordering::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_key->outputPHP(cg, ar);
  switch (m_direction) {
  case T_ASCENDING:
    cg_printf(" ascending");
    break;
  case T_DESCENDING:
    cg_printf(" decending");
    break;
  default:
    break;
  }
}

