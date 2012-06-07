/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <compiler/expression/user_attribute.h>

using namespace HPHP;
using namespace boost;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UserAttribute::UserAttribute
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string& name,
 ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(UserAttribute)),
    m_name(name), m_exp(exp) {
  if (m_exp) {
    ASSERT(m_exp->isScalar());
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

TypePtr UserAttribute::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  ASSERT(false);
  return TypePtr();
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

void UserAttribute::outputCPPImpl(CodeGenerator &cg,
                                  AnalysisResultPtr ar) {
}

