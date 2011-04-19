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

#include <runtime/eval/ast/closure_expression.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ClosureExpression::ClosureExpression(EXPRESSION_ARGS,
                                     FunctionStatementPtr func,
                                     const vector<ParameterPtr> &vars)
    : Expression(EXPRESSION_PASS), m_func(func), m_vars(vars) {
  func->setClosure(this);
}

Variant ClosureExpression::eval(VariableEnvironment &env) const {
  m_func->eval(env);
  Array vars;
  for (unsigned int i = 0; i < m_vars.size(); i++) {
    ParameterPtr param = m_vars[i];
    String name = param->getName();
    if (param->isRef()) {
      vars.append(ref(env.get(name)));
    } else {
      vars.append(env.get(name));
    }
  }
  return create_object("Closure", CREATE_VECTOR2(m_func->name(), vars));
}

void ClosureExpression::dump(std::ostream &out) const {
  m_func->dumpHeader(out);
  if (!m_vars.empty()) {
    out << " use (";
    dumpVector(out, m_vars);
    out << ")";
  }
  m_func->dumpBody(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

