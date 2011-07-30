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
  : Expression(KindOfBinaryOpExpression, EXPRESSION_PASS),
  m_exp1(exp1), m_exp2(exp2), m_op(op) {
  m_reverseOrder = m_exp1->isKindOf(Expression::KindOfVariableExpression);
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
      Variant::TypedValueAccessor acc1 = v1.getTypedAccessor();
      Variant::TypedValueAccessor acc2 = v2.getTypedAccessor();
      DataType t1 = Variant::GetAccessorType(acc1);
      DataType t2 = Variant::GetAccessorType(acc2);
      ASSERT(t1 != KindOfUninit && t2 != KindOfUninit &&
             t1 != KindOfVariant && t2 != KindOfVariant);
      bool sameType = (t1 == t2) ||
        (t1 == KindOfInt64 && t2 == KindOfInt32 ||
         t1 == KindOfInt32 && t2 == KindOfInt64 ||
         t1 == KindOfString && t2 == KindOfStaticString ||
         t1 == KindOfStaticString && t2 == KindOfString);
      switch (m_op) {
      case T_LOGICAL_XOR:
        SET_LINE;
        return logical_xor(v1, v2);
      case '|':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) | Variant::GetInt64(acc2);
          }
        }
        SET_LINE;
        return bitwise_or(v1, v2);
      case '&':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) & Variant::GetInt64(acc2);
          }
        }
        SET_LINE;
        return bitwise_and(v1, v2);
      case '^':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) ^ Variant::GetInt64(acc2);
          }
        }
        SET_LINE;
        return bitwise_xor(v1, v2);
      case '.':
        SET_LINE;
        return concat(v1, v2);
      case '+':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) + Variant::GetInt64(acc2);
          } else if (LIKELY(t1 == KindOfDouble)) {
            return Variant::GetDouble(acc1) + Variant::GetDouble(acc2);
          }
        }
        SET_LINE;
        return v1 + v2;
      case '-':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) - Variant::GetInt64(acc2);
          } else if (LIKELY(t1 == KindOfDouble)) {
            return Variant::GetDouble(acc1) - Variant::GetDouble(acc2);
          }
        }
        SET_LINE;
        return v1 - v2;
      case '*':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) * Variant::GetInt64(acc2);
          } else if (LIKELY(t1 == KindOfDouble)) {
            return Variant::GetDouble(acc1) * Variant::GetDouble(acc2);
          }
        }
        SET_LINE;
        return v1 * v2;
      case '/':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            int64 lval = Variant::GetInt64(acc1);
            int64 lval2 = Variant::GetInt64(acc2);
            if (UNLIKELY(lval2 == 0)) {
              SET_LINE;
              raise_warning("Division by zero");
              return false;
            }
            if (lval % lval2 == 0) {
              return lval / lval2;
            } else {
              return (double)lval / lval2;
            }
          } else if (LIKELY(t1 == KindOfDouble)) {
            double dval = Variant::GetDouble(acc1);
            double dval2 = Variant::GetDouble(acc2);
            if (UNLIKELY(dval2 == 0)) {
              SET_LINE;
              raise_warning("Division by zero");
              return false;
            }
            return dval / dval2;
          }
        }
        SET_LINE;
        return divide(v1, v2);
      case '%':
        SET_LINE;
        return modulo(v1, v2);
      case T_SL:
        if (sameType) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) << Variant::GetInt64(acc2);
          }
        }
        SET_LINE;
        return v1.toInt64() << v2.toInt64();
      case T_SR:
        if (sameType) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) >> Variant::GetInt64(acc2);
          }
        }
        SET_LINE;
        return v1.toInt64() >> v2.toInt64();
      case T_IS_IDENTICAL:
        if (sameType) {
          if (t1 == KindOfInt64 || t1 == KindOfInt32) {
            return Variant::GetInt64(acc1) == Variant::GetInt64(acc2);
          }
          if (t1 == KindOfString || t1 == KindOfStaticString) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return (s1 == s2 || s1->same(s2));
          }
        } else if (LIKELY(!IS_REFCOUNTED_TYPE(t1) && !IS_REFCOUNTED_TYPE(t2))
                   || (t1 != KindOfNull && t2 != KindOfNull)) {
          ASSERT(!same(v1, v2));
          return false;
        }
        SET_LINE;
        return same(v1, v2);
      case T_IS_NOT_IDENTICAL:
        if (LIKELY(sameType)) {
          if (t1 == KindOfInt64 || t1 == KindOfInt32) {
            return Variant::GetInt64(acc1) != Variant::GetInt64(acc2);
          }
          if (t1 == KindOfString || t1 == KindOfStaticString) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return !(s1 == s2 || s1->same(s2));
          }
        } else if (LIKELY(!IS_REFCOUNTED_TYPE(t1) && !IS_REFCOUNTED_TYPE(t2))
                   || (t1 != KindOfNull && t2 != KindOfNull)) {
          ASSERT(!same(v1, v2));
          return true;
        }
        SET_LINE;
        return !same(v1, v2);
      case T_IS_EQUAL:
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) == Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return (s1 == s2 || s1->compare(s2) == 0);
          }
        }
        SET_LINE;
        return equal(v1, v2);
      case T_IS_NOT_EQUAL:
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) != Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return !(s1 == s2 || s1->compare(s2) == 0);
          }
        }
        SET_LINE;
        return !equal(v1, v2);
      case '<':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) < Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return s1->compare(s2) < 0;
          }
        }
        SET_LINE;
        return less(v1, v2);
      case T_IS_SMALLER_OR_EQUAL:
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) <= Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return s1->compare(s2) <= 0;
          }
        }
        SET_LINE;
        return not_more(v1, v2);
      case '>':
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) > Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return s1->compare(s2) > 0;
          }
        }
        SET_LINE;
        return more(v1, v2);
      case T_IS_GREATER_OR_EQUAL:
        if (LIKELY(sameType)) {
          if (LIKELY(t1 == KindOfInt64 || t1 == KindOfInt32)) {
            return Variant::GetInt64(acc1) >= Variant::GetInt64(acc2);
          }
          if (LIKELY(t1 == KindOfString || t1 == KindOfStaticString)) {
            const StringData *s1 = Variant::GetStringData(acc1);
            const StringData *s2 = Variant::GetStringData(acc2);
            return s1->compare(s2) >= 0;
          }
        }
        SET_LINE;
        return not_less(v1, v2);
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

