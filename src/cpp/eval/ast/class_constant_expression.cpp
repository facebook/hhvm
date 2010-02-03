/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/eval/ast/class_constant_expression.h>
#include <cpp/eval/runtime/eval_state.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ClassConstantExpression::ClassConstantExpression(EXPRESSION_ARGS,
                                                 const string &cls,
                                                 const string &constant)
  : Expression(EXPRESSION_PASS), m_class(cls),
    m_constant(constant) {}

Variant ClassConstantExpression::eval(VariableEnvironment &env) const {
  return get_class_constant(m_class.c_str(), m_constant.c_str());
}

void ClassConstantExpression::dump() const {
  printf("%s::%s", m_class.c_str(), m_constant.c_str());
}

///////////////////////////////////////////////////////////////////////////////
}
}

