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

#include <runtime/eval/ast/class_constant_expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/eval.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ClassConstantExpression::ClassConstantExpression(EXPRESSION_ARGS,
    const NamePtr &cls, const string &constant)
  : Expression(EXPRESSION_PASS), m_class(cls), m_constant(constant) {}

Variant ClassConstantExpression::eval(VariableEnvironment &env) const {
  DECLARE_THREAD_INFO;
  check_recursion(info);
  String cls = m_class->get(env);
  return get_class_constant(cls.c_str(), m_constant.c_str());
}

bool ClassConstantExpression::evalStaticScalar(VariableEnvironment &env,
                                               Variant &r) const {
  String cls = m_class->get(env);
  const char *s = cls.c_str();
  const char *constant = m_constant.c_str();
  if (eval_get_class_constant_hook(r, s, constant)) return true;
  return false;
}

void ClassConstantExpression::dump(std::ostream &out) const {
  m_class->dump(out);
  out << "::" << m_constant;
}

///////////////////////////////////////////////////////////////////////////////
}
}

