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

#include <runtime/eval/ast/unary_op_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/ext/ext_misc.h>
#include <runtime/eval/eval.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

UnaryOpExpression::UnaryOpExpression(EXPRESSION_ARGS, ExpressionPtr exp,
                                     int op, bool front)
  : Expression(EXPRESSION_PASS), m_exp(exp), m_op(op), m_front(front) {}

Variant UnaryOpExpression::eval(VariableEnvironment &env) const {
  if (m_op == '@') {
    Silencer s;
    s.enable();
    return m_exp->eval(env);
  } else if (m_op == T_ISSET || m_op == T_EMPTY) {
    return m_exp->exist(env, m_op);
  }

  Variant exp(m_exp ? m_exp->eval(env) : null_variant);
  SET_LINE;
  switch (m_op) {
  case T_CLONE:       return f_clone(exp);
  case '+':           return +exp;
  case '-':           return negate(exp);
  case '!':           return !exp;
  case '~':           return ~exp;
  case T_INT_CAST:    return toInt64(exp);
  case T_DOUBLE_CAST: return toDouble(exp);
  case T_STRING_CAST: return toString(exp);
  case T_ARRAY_CAST:  return toArray(exp);
  case T_OBJECT_CAST: return toObject(exp);
  case T_BOOL_CAST:   return toBoolean(exp);
  case T_UNSET_CAST:  return unset(exp);
  case T_EXIT:        return f_exit(exp);
  case T_PRINT:       return print(exp.toString());
  case T_EVAL:        return HPHP::eval(&env, env.currentObject(), exp);
  default:
    ASSERT(false);
    return Variant();
  }
}

bool UnaryOpExpression::evalStaticScalar(VariableEnvironment &env, Variant &r)
  const {
  ASSERT(m_exp);
  Variant exp;
  if (!m_exp->evalStaticScalar(env, exp)) return false;
  switch (m_op) {
  case '+':           r = +exp;
  case '-':           r = negate(exp);
  default:
    ASSERT(false);
    r = null;
    return false;
  }
  return true;
}

Variant UnaryOpExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return ref(Expression::refval(env, strict));
}

void UnaryOpExpression::dump(std::ostream &out) const {
  if (m_front) {
    dumpOp(out);
  }
  if (m_exp) {
    m_exp->dump(out);
  }
  switch (m_op) {
    case T_EXIT:
    case T_EMPTY:
    case T_EVAL:
      out << ")";
  }
  if (!m_front) {
    dumpOp(out);
  }
}

void UnaryOpExpression::dumpOp(std::ostream &out) const {
  switch (m_op) {
  case '@':           out << "@";        break;
  case T_ISSET:       out << "isset ";   break;
  case T_CLONE:       out << "clone ";   break;
  case '+':           out << "+";        break;
  case '-':           out << "-";        break;
  case '!':           out << "!";        break;
  case '~':           out << "~";        break;
  case T_INT_CAST:    out << "(int)";    break;
  case T_DOUBLE_CAST: out << "(double)"; break;
  case T_STRING_CAST: out << "(string)"; break;
  case T_ARRAY_CAST:  out << "(array)";  break;
  case T_OBJECT_CAST: out << "(object)"; break;
  case T_BOOL_CAST:   out << "(bool)";   break;
  case T_UNSET_CAST:  out << "(unset)";  break;
  case T_EXIT:        out << "exit(";    break;
  case T_PRINT:       out << "print ";   break;
  case T_EMPTY:       out << "empty(";   break;
  case T_EVAL:        out << "eval(";    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
}

}

