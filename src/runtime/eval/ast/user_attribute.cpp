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

#include <runtime/eval/ast/user_attribute.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

UserAttribute::UserAttribute(EXPRESSION_ARGS,
                             const string &name,
                             ExpressionPtr exp)
  : Expression(KindOfUserAttribute, EXPRESSION_PASS),
  m_name(name), m_exp(exp) {
}

Expression *UserAttribute::optimize(VariableEnvironment &env) {
  return NULL;
}

Variant UserAttribute::eval(VariableEnvironment &env) const {
  return null;
}

bool UserAttribute::evalStaticScalar(VariableEnvironment &env,
                                           Variant &r) const {
  return false;
}

void UserAttribute::dump(std::ostream &out) const {
  out << m_name;
}

///////////////////////////////////////////////////////////////////////////////
}
}

