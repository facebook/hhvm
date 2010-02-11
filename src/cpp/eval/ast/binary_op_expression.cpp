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

#include <cpp/eval/ast/binary_op_expression.h>
#include <cpp/eval/parser/hphp.tab.hpp>
#include <cpp/eval/bytecode/bytecode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

BinaryOpExpression::BinaryOpExpression(EXPRESSION_ARGS, ExpressionPtr exp1,
                                       int op, ExpressionPtr exp2)
  : Expression(EXPRESSION_PASS), m_exp1(exp1), m_exp2(exp2), m_op(op) {}

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
      Variant v1 = m_exp1->eval(env);
      Variant v2 = m_exp2->eval(env);
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

void BinaryOpExpression::dump() const {
  m_exp1->dump();
  const char* op = "<bad op>";
  switch (m_op) {
  case T_LOGICAL_OR: op = "OR"; break;
  case T_BOOLEAN_OR: op = "||"; break;
  case T_LOGICAL_AND: op = "AND"; break;
  case T_BOOLEAN_AND: op = "&&"; break;
  case T_LOGICAL_XOR: op = "XOR"; break;
  case '|': op = "|"; break;
  case '&': op = "&"; break;
  case '^': op = "^"; break;
  case '.': op = "."; break;
  case '+': op = "+"; break;
  case '-': op = "-"; break;
  case '*': op = "*"; break;
  case '/': op = "/"; break;
  case '%': op = "%"; break;
  case T_SL: op = "<<"; break;
  case T_SR: op = ">>"; break;
  case T_IS_IDENTICAL: op = "==="; break;
  case T_IS_NOT_IDENTICAL: op = "!=="; break;
  case T_IS_EQUAL: op = "=="; break;
  case T_IS_NOT_EQUAL: op = "!="; break;
  case '<': op = "<"; break;
  case T_IS_SMALLER_OR_EQUAL: op = "<="; break;
  case '>': op = ">"; break;
  case T_IS_GREATER_OR_EQUAL: op = ">="; break;
  default:
    ASSERT(false);
  }
  printf(" %s ", op);
  m_exp2->dump();
}

void BinaryOpExpression::byteCodeEval(ByteCodeProgram &code) const {
  ByteCode::Operation op = ByteCode::Nop;
  switch (m_op) {
  case T_LOGICAL_XOR: op = ByteCode::LogXor; break;
  case '|': op = ByteCode::BitOr; break;
  case '&': op = ByteCode::BitAnd; break;
  case '^': op = ByteCode::BitXor; break;
  case '.': op = ByteCode::Concat; break;
  case '+': op = ByteCode::Add; break;
  case '-': op = ByteCode::Sub; break;
  case '*': op = ByteCode::Mul; break;
  case '/': op = ByteCode::Div; break;
  case '%': op = ByteCode::Mod; break;
  case T_SL: op = ByteCode::Sl; break;
  case T_SR: op = ByteCode::Sr; break;
  case T_IS_IDENTICAL: op = ByteCode::Same; break;
  case T_IS_NOT_IDENTICAL: op = ByteCode::NotSame; break;
  case T_IS_EQUAL: op = ByteCode::Equal; break;
  case T_IS_NOT_EQUAL: op = ByteCode::NotEqual; break;
  case '<': op = ByteCode::LT; break;
  case T_IS_SMALLER_OR_EQUAL: op = ByteCode::LEQ; break;
  case '>': op = ByteCode::GT; break;
  case T_IS_GREATER_OR_EQUAL: op = ByteCode::GEQ; break;
  default:
    Expression::byteCodeEval(code);
  }
  m_exp1->byteCodeEval(code);
  m_exp2->byteCodeEval(code);
  code.add(op);
}

///////////////////////////////////////////////////////////////////////////////
}

}

