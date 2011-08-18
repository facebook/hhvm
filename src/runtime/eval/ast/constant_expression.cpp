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

#include <runtime/eval/ast/constant_expression.h>
#include <runtime/base/externals.h>
#include <runtime/ext/ext_misc.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/name.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ConstantExpression::ConstantExpression(EXPRESSION_ARGS,
                                       const string &constant)
  : Expression(KindOfConstantExpression, EXPRESSION_PASS),
  m_constant(StringName::GetStaticName(constant)) {
  m_type = check_constant(m_constant);
  if (m_type == StaticBuiltinConstant) {
    m_value = get_builtin_constant(m_constant);
  }
}

Variant ConstantExpression::eval(VariableEnvironment &env) const {
  switch (m_type) {
  case StaticBuiltinConstant:
    return m_value;
  case StdioBuiltinConstant:
  case DynamicBuiltinConstant:
    return get_builtin_constant(m_constant);
  case NoneBuiltinConstant:
    break;
  default:
    assert(false);
    break;
  }
  const char *s = m_constant->data();
  if (LIKELY(s[0] != '\\')) {
    return RequestEvalState::findUserConstant(m_constant);
  }
  Variant ret;
  if (RequestEvalState::findConstant(s + 1, ret)) {
    return ret;
  }
  const char *r = s + m_constant->size() - 1;
  while (*r != '\\') r--;
  ASSERT(*r == '\\');
  return get_constant(r + 1);
}

bool ConstantExpression::evalStaticScalar(VariableEnvironment &env,
  Variant &r) const {
  if (f_defined(m_constant)) {
    r = get_constant(m_constant);
    return true;
  }
  return false;
}

void ConstantExpression::dump(std::ostream &out) const {
  out << m_constant->data();
}

///////////////////////////////////////////////////////////////////////////////
}
}

