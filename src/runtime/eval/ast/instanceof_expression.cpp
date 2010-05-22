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

#include <runtime/eval/ast/instanceof_expression.h>
#include <runtime/eval/ast/name.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

InstanceOfExpression::InstanceOfExpression(EXPRESSION_ARGS, ExpressionPtr obj,
                                           NamePtr name)
  : Expression(EXPRESSION_PASS), m_obj(obj), m_name(name) {}

Variant InstanceOfExpression::eval(VariableEnvironment &env) const {
  Variant obj(m_obj->eval(env));
  String name(m_name->get(env));
  return instanceOf(obj, name);
}

void InstanceOfExpression::dump() const {
  printf("instanceof(");
  m_obj->dump();
  printf(", ");
  m_name->dump();
  printf(")");
}

///////////////////////////////////////////////////////////////////////////////
}
}

