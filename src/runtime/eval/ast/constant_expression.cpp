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

#include <runtime/eval/ast/constant_expression.h>
#include <runtime/base/externals.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ConstantExpression::ConstantExpression(EXPRESSION_ARGS,
                                       const string &constant)
  : Expression(EXPRESSION_PASS), m_constant(constant) {}

Variant ConstantExpression::eval(VariableEnvironment &env) const {
  return get_constant(m_constant);
}

void ConstantExpression::dump() const {
  printf("%s", m_constant.c_str());
}

///////////////////////////////////////////////////////////////////////////////
}
}

