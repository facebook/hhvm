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

#include <cpp/eval/ast/expression.h>
#include <cpp/eval/ast/lval_expression.h>
#include <cpp/eval/parser/hphp.tab.hpp>
#include <cpp/eval/bytecode/bytecode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

Expression::Expression(EXPRESSION_ARGS) : Construct(CONSTRUCT_PASS) {}

Variant Expression::evalVector(const std::vector<ExpressionPtr> &v,
                               VariableEnvironment &env) {
  Variant res;
  for (std::vector<ExpressionPtr>::const_iterator it = v.begin();
       it != v.end(); ++it) {
    res = (*it)->eval(env);
  }
  return res;
}

Variant Expression::refval(VariableEnvironment &env) const {
  return ref(eval(env));
}

bool Expression::exist(VariableEnvironment &env, int op) const {
  if (op == T_ISSET) {
    return HPHP::isset(eval(env));
  }
  ASSERT(op == T_EMPTY);
  return HPHP::empty(eval(env));
}

const LvalExpression *Expression::toLval() const {
  return NULL;
}

bool Expression::isRefParam() const {
  return false;
}

void Expression::byteCodeEval(ByteCodeProgram &code) const {
  throw FatalErrorException("Cannot compile %s:%d", m_loc.file, m_loc.line1);
}
void Expression::byteCodeRefval(ByteCodeProgram &code) const {
  throw FatalErrorException("Cannot compile %s:%d", m_loc.file, m_loc.line1);

}

void Expression::byteCodeEvalVector(const std::vector<ExpressionPtr> &v,
                                    ByteCodeProgram &code) {
  uint i;
  for (i = 0; i < v.size() - 1; ++i) {
    v[i]->byteCodeEval(code);
    code.add(ByteCode::Discard);
  }
  v[i]->byteCodeEval(code);
}

///////////////////////////////////////////////////////////////////////////////
}
}
