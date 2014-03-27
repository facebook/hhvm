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

#include "hphp/compiler/expression/ordering.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/compiler/code_model_enums.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

Ordering::Ordering
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr key, std::string direction)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(Ordering)),
    m_key(key) {
  if (direction == "") {
    m_direction = PHP_NOT_SPECIFIED;
  } else if (strcasecmp(direction.c_str(), "ascending") == 0) {
    m_direction = PHP_ASCENDING;
  } else {
    assert(strcasecmp(direction.c_str(), "descending") == 0);
    m_direction = PHP_DESCENDING;
  }
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
  cg.printObjectHeader("Ordering", 3);
  cg.printPropertyHeader("expression");
  m_key->outputCodeModel(cg);
  cg.printPropertyHeader("order");
  cg.printValue(m_direction);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void Ordering::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_key->outputPHP(cg, ar);
  switch (m_direction) {
  case PHP_ASCENDING:
    cg_printf(" ascending");
    break;
  case PHP_DESCENDING:
    cg_printf(" decending");
    break;
  default:
    break;
  }
}

