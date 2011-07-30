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

#include <runtime/eval/ast/function_call_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

FunctionCallExpression::FunctionCallExpression(EXPRESSION_ARGS,
                                               const std::vector<ExpressionPtr>
                                               &params)
  : Expression(KindOfFunctionCallExpression, EXPRESSION_PASS),
  m_params(params) {}

ArgArray *FunctionCallExpression::prepareArgArray(VariableEnvironment &env,
  const CallInfo* ci, unsigned int count) const {
  ArgArray *args = NEW(ArgArray)(count);
  ArgArray::Argument *argp = args->getStack();
  for (unsigned int i = 0; i < count; ++i, argp++) {
    if (ci->mustBeRef(i)) {
      argp->m_val.assignRef(m_params[i]->refval(env));
    } else if (ci->isRef(i)) {
      argp->m_val.assignRef(m_params[i]->refval(env, 0));
    } else {
      argp->m_val.assign(m_params[i]->eval(env));
    }
  }
  return args;
}

Array FunctionCallExpression::getParams(VariableEnvironment &env) const {
  Array params;
  for (std::vector<ExpressionPtr>::const_iterator it = m_params.begin();
       it != m_params.end(); ++it) {
    params.append(ref((*it)->refval(env, 0)));
  }
  return params;
}

void FunctionCallExpression::dumpParams(std::ostream &out) const {
  out << "(";
  dumpVector(out, m_params);
  out << ")";
}

bool FunctionCallExpression::exist(VariableEnvironment &env, int op) const {
  throw FatalErrorException(0, "Cannot call %s on a function return value",
                            op == T_ISSET ? "isset" : "empty");
}

Variant FunctionCallExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}


///////////////////////////////////////////////////////////////////////////////
}
}

