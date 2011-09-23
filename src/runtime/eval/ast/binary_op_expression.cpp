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
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/binary_op_expression.h>
#include <runtime/eval/ast/scalar_value_expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void BinaryOpExpression::setOperandKindOf() {
  switch (m_exp1->getKindOf()) {
  case KindOfVariableExpression:
    switch (m_exp2->getKindOf()) {
    case KindOfVariableExpression:
      m_operandKindOf = VVN;
      break;
    case KindOfScalarValueExpression:
      m_operandKindOf = VSN;
      break;
    default:
      m_operandKindOf = VDR;
      break;
    }
    break;
  case KindOfScalarValueExpression:
    switch (m_exp2->getKindOf()) {
    case KindOfVariableExpression:
      m_operandKindOf = SVN;
      break;
    case KindOfScalarValueExpression:
      m_operandKindOf = SSN;
      break;
    default:
      m_operandKindOf = SDN;
      break;
    }
    break;
  default:
    switch (m_exp2->getKindOf()) {
    case KindOfVariableExpression:
      m_operandKindOf = DVN;
      break;
    case KindOfScalarValueExpression:
      m_operandKindOf = DSN;
      break;
    default:
      m_operandKindOf = DDN;
      break;
    }
    break;
  }
}

BinaryOpExpression::BinaryOpExpression(EXPRESSION_ARGS, ExpressionPtr exp1,
                                       int op, ExpressionPtr exp2)
  : Expression(KindOfBinaryOpExpression, EXPRESSION_PASS),
  m_exp1(exp1), m_exp2(exp2), m_op(op) {
  setOperandKindOf();
}

Expression *BinaryOpExpression::optimize(VariableEnvironment &env) {
  Variant v;
  Eval::optimize(env, m_exp1);
  Eval::optimize(env, m_exp2);
  if (evalScalar(env, v)) {
    return new ScalarValueExpression(v, loc());
  }
  if (m_exp1->isKindOf(KindOfBinaryOpExpression)) {
    BinaryOpExpression *binOpExp =
      static_cast<BinaryOpExpression *>(m_exp1.get());
    if (binOpExp->m_op == '.') {
      VectorConcatExpression *vce =
        new VectorConcatExpression(binOpExp->m_exp1, loc());
      vce->addElement(binOpExp->m_exp2);
      vce->addElement(m_exp2);
      return vce;
    }
  } else if (m_exp1->isKindOf(KindOfVectorConcatExpression)) {
    VectorConcatExpression *vce =
      new VectorConcatExpression(
        static_cast<VectorConcatExpression *>(m_exp1.get()));
    vce->addElement(m_exp2);
    return vce;
  }
  setOperandKindOf();
  return NULL;
}

Variant BinaryOpExpression::eval(VariableEnvironment &env) const {
  switch (m_op) {
  case T_LOGICAL_OR:
  case T_BOOLEAN_OR: return m_exp1->eval(env) || m_exp2->eval(env);
  case T_LOGICAL_AND:
  case T_BOOLEAN_AND: return m_exp1->eval(env) && m_exp2->eval(env);
  default:
    break;
  }
  Variant r1, r2;
  Variant *v1p, *v2p;
  switch (m_operandKindOf) {
  case VVN:
    v1p = &VariableExpression::GetVariableByRefCheck(env, m_exp1.get());
    v2p = &VariableExpression::GetVariableByRefCheck(env, m_exp2.get());
    break;
  case VSN:
    v1p = &VariableExpression::GetVariableByRefCheck(env, m_exp1.get());
    v2p = &ScalarValueExpression::GetScalarValueByRef(m_exp2.get());
    break;
  case VDR: // reverse
    v2p = new(&r2) Variant(m_exp2->eval(env));
    v1p = &VariableExpression::GetVariableByRefCheck(env, m_exp1.get());
    break;
  case SVN:
    v1p = &ScalarValueExpression::GetScalarValueByRef(m_exp1.get());
    v2p = &VariableExpression::GetVariableByRefCheck(env, m_exp2.get());
    break;
  case SSN:
    v1p = &ScalarValueExpression::GetScalarValueByRef(m_exp1.get());
    v2p = &ScalarValueExpression::GetScalarValueByRef(m_exp2.get());
    break;
  case SDN:
    v1p = &ScalarValueExpression::GetScalarValueByRef(m_exp1.get());
    v2p = new(&r2) Variant(m_exp2->eval(env));
    break;
  case DVN:
    v1p = new(&r1) Variant(m_exp1->eval(env));
    v2p = &VariableExpression::GetVariableByRefCheck(env, m_exp2.get());
    break;
  case DSN:
    v1p = new(&r1) Variant(m_exp1->eval(env));
    v2p = &ScalarValueExpression::GetScalarValueByRef(m_exp2.get());
    break;
  case DDN:
    v1p = new(&r1) Variant(m_exp1->eval(env));
    v2p = new(&r2) Variant(m_exp2->eval(env));
    break;
  default:
    assert(false);
  }
  CVarRef v1 = *v1p;
  CVarRef v2 = *v2p;
  Variant::TypedValueAccessor acc1 = v1.getTypedAccessor();
  Variant::TypedValueAccessor acc2 = v2.getTypedAccessor();
  DataType t1 = Variant::GetAccessorType(acc1);
  DataType t2 = Variant::GetAccessorType(acc2);
  ASSERT(t1 != KindOfVariant && t2 != KindOfVariant);
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

bool BinaryOpExpression::evalScalar(
  VariableEnvironment &env, Variant &r) const {
  Variant v1;
  Variant v2;
  if (!m_exp1->evalScalar(env, v1) || !m_exp2->evalScalar(env, v2)) {
    r = null;
    return false;
  }
  bool isScalar = true;
  if (m_op == '/' || m_op == '%') {
    if ((v1.isArray() || v2.isArray()) ||
        (v2.isDouble() && v2.toDouble() == 0.0) ||
        (v2.isIntVal() && v2.toInt64() == 0.0)) {
      isScalar = false;
    } else {
      r = m_op == '/' ? divide(v1, v2) : modulo(v1, v2);
    }
  } else {
    r = eval(env);
  }
  return isScalar;
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

VectorConcatExpression::VectorConcatExpression(
  ExpressionPtr exp, const Location *loc) :
  Expression(KindOfVectorConcatExpression, loc) {
  m_exps.push_back(exp);
}

VectorConcatExpression::VectorConcatExpression(VectorConcatExpression *vce) :
  Expression(KindOfVectorConcatExpression, vce->loc()) {
  m_exps = vce->m_exps;
}

Variant VectorConcatExpression::eval(VariableEnvironment &env) const {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  unsigned int count = m_exps.size();
  ASSERT(count > 2);
  String *temps = new String[count];
  if (m_exps[0]->isKindOf(KindOfVariableExpression)) {
    temps[1] = m_exps[1]->eval(env);
    temps[0] = m_exps[0]->eval(env);
  } else {
    temps[0] = m_exps[0]->eval(env);
    temps[1] = m_exps[1]->eval(env);
  }
  int len = temps[0].size() + temps[1].size();
  for (unsigned int i = 2; i < count; i++) {
    temps[i] = m_exps[i]->eval(env);
    len += temps[i].size();
  }
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  char *p = buf;
  for (unsigned int i = 0; i < count; i++) {
    memcpy(p, temps[i].data(), temps[i].size());
    p += temps[i].size();
  }
  buf[len] = 0;
  delete[] temps;
  return String(buf, len, AttachString);
}

void VectorConcatExpression::addElement(ExpressionPtr exp) {
  // try to merge scalars
  ExpressionPtr &last = m_exps.back();
  DummyVariableEnvironment env;
  Variant v1;
  Variant v2;
  if (last->evalScalar(env, v1) && exp->evalScalar(env, v2)) {
    Variant r = concat(v1, v2);
    last = new ScalarValueExpression(r, last->loc());
    return;
  }
  m_exps.push_back(exp);
}

void VectorConcatExpression::dump(std::ostream &out) const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (i > 0) out << ".";
    m_exps[i]->dump(out);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

