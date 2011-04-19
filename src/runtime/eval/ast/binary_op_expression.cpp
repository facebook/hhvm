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

#include <runtime/eval/ast/binary_op_expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

BinaryOpExpression::BinaryOpExpression(EXPRESSION_ARGS, ExpressionPtr exp1,
                                       int op, ExpressionPtr exp2)
  : Expression(EXPRESSION_PASS), m_exp1(exp1), m_exp2(exp2), m_op(op) {
  m_reverseOrder = m_exp1->is<VariableExpression>();
}

Variant BinaryOpExpression::eval(VariableEnvironment &env) const {
  switch (m_op) {
  case T_LOGICAL_OR:
  case T_BOOLEAN_OR: return m_exp1->eval(env) ||
      m_exp2->eval(env);
  case T_LOGICAL_AND:
  case T_BOOLEAN_AND: return m_exp1->eval(env) &&
      m_exp2->eval(env);
  default:
    {
      Variant v1, v2;
      if (m_reverseOrder) {
        v2 = m_exp2->eval(env);
        v1 = m_exp1->eval(env);
      } else {
        v1 = m_exp1->eval(env);
        v2 = m_exp2->eval(env);
      }
      SET_LINE;
      switch (m_op) {
      case T_LOGICAL_XOR:         return logical_xor(v1, v2);
      case '|':                   return bitwise_or(v1, v2);
      case '&':                   return bitwise_and(v1, v2);
      case '^':                   return bitwise_xor(v1, v2);
      case '.':                   return concat(v1, v2);
      case '+':                   return v1 + v2;
      case '-':                   return v1 - v2;
      case '*':                   return multiply(v1, v2);
      case '/':                   return divide(v1, v2);
      case '%':                   return modulo(v1, v2);
      case T_SL:                  return v1.toInt64() << v2.toInt64();
      case T_SR:                  return v1.toInt64() >> v2.toInt64();
      case T_IS_IDENTICAL:        return same(v1, v2);
      case T_IS_NOT_IDENTICAL:    return !same(v1, v2);
      case T_IS_EQUAL:            return equal(v1, v2);
      case T_IS_NOT_EQUAL:        return !equal(v1, v2);
      case '<':                   return less(v1, v2);
      case T_IS_SMALLER_OR_EQUAL: return not_more(v1, v2);
      case '>':                   return more(v1, v2);
      case T_IS_GREATER_OR_EQUAL: return not_less(v1, v2);
      default:
        ASSERT(false);
        return Variant();
      }
    }
  }
}

void BinaryOpExpression::dump(std::ostream &out) const {
  m_exp1->dump(out);
  out << " ";
  switch (m_op) {
  case T_LOGICAL_OR:          out << "or";  break;
  case T_BOOLEAN_OR:          out << "||";  break;
  case T_LOGICAL_AND:         out << "and"; break;
  case T_BOOLEAN_AND:         out << "&&";  break;
  case T_LOGICAL_XOR:         out << "xor"; break;
  case '|':                   out << "|";   break;
  case '&':                   out << "&";   break;
  case '^':                   out << "^";   break;
  case '.':                   out << ".";   break;
  case '+':                   out << "+";   break;
  case '-':                   out << "-";   break;
  case '*':                   out << "*";   break;
  case '/':                   out << "/";   break;
  case '%':                   out << "%";   break;
  case T_SL:                  out << "<<";  break;
  case T_SR:                  out << ">>";  break;
  case T_IS_IDENTICAL:        out << "==="; break;
  case T_IS_NOT_IDENTICAL:    out << "!=="; break;
  case T_IS_EQUAL:            out << "==";  break;
  case T_IS_NOT_EQUAL:        out << "!=";  break;
  case '<':                   out << "<";   break;
  case T_IS_SMALLER_OR_EQUAL: out << "<=";  break;
  case '>':                   out << ">";   break;
  case T_IS_GREATER_OR_EQUAL: out << ">=";  break;
  default:
    ASSERT(false);
  }
  out << " ";
  m_exp2->dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

