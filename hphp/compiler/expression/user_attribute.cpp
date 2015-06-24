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

#include "hphp/compiler/expression/user_attribute.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/parser/scanner.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(UnaryOpExpression);

// constructors/destructors

UserAttribute::UserAttribute
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string& name,
 ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(UserAttribute)),
    m_name(name), m_exp(exp) {
  if (m_exp) {
    assert(m_exp->isScalar());
  }
}

ExpressionPtr UserAttribute::clone() {
  UserAttributePtr exp(new UserAttribute(*this));
  Expression::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void UserAttribute::analyzeProgram(AnalysisResultPtr ar) {
  // do nothing
}

///////////////////////////////////////////////////////////////////////////////

void UserAttribute::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("Attribute", m_exp != nullptr ? 3 : 2);
  cg.printPropertyHeader("attributeName");
  cg.printValue(m_name);
  if (m_exp != nullptr && m_exp->is(Expression::KindOfUnaryOpExpression)) {
    UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(m_exp));
    if (u->getOp() == T_ARRAY) {
      ExpressionPtr ex = u->getExpression();
      if (ex != nullptr) {
        if (ex->is(Expression::KindOfExpressionList)) {
          ExpressionListPtr el(static_pointer_cast<ExpressionList>(ex));
          cg.printPropertyHeader("expressions");
          cg.printExpressionVector(el);
        } else {
          assert(false);
        }
      } else {
        ExpressionListPtr el;
        cg.printPropertyHeader("expressions");
        cg.printExpressionVector(el);
      }
    } else {
      assert(false);
    }
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UserAttribute::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("%s", m_name.c_str());
  if (m_exp) {
    cg_printf("(");
    // XXX This is wrong, since it will print out "A(array(1,2))" instead
    // of "A(1,2)"... need to fix this
    m_exp->outputPHP(cg, ar);
    cg_printf(")");
  }
}

}
