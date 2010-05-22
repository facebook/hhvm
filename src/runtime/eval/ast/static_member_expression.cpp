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

#include <runtime/eval/ast/static_member_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

StaticMemberExpression::StaticMemberExpression(EXPRESSION_ARGS,
    const NamePtr &cls, const NamePtr &variable)
  : LvalExpression(EXPRESSION_PASS), m_class(cls),
    m_variable(variable) {}

Variant &StaticMemberExpression::lval(VariableEnvironment &env) const {
  String cls(m_class->get(env));
  String variable(m_variable->get(env));
  Variant *lv = get_static_property_lv(cls.data(), variable.data());
  if (lv) {
    return *lv;
  }
  SET_LINE;
  raise_error("Static member %s::$%s does not exist", cls.data(),
              variable.data());
  return Variant::lvalInvalid();
}

void StaticMemberExpression::dump() const {
  m_class->dump();
  printf("::");
  m_variable->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

