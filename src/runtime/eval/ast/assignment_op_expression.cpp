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

#include <runtime/eval/ast/assignment_op_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/eval/ast/variable_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

AssignmentOpExpression::AssignmentOpExpression(EXPRESSION_ARGS, int op,
                                               LvalExpressionPtr lhs,
                                               ExpressionPtr rhs)
  : Expression(KindOfAssignmentOpExpression, EXPRESSION_PASS),
  m_op(op), m_lhs(lhs), m_rhs(rhs) {}

Expression *AssignmentOpExpression::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_lhs);
  Eval::optimize(env, m_rhs);
  if (m_lhs->isKindOf(Expression::KindOfVariableExpression)) {
    switch (m_op) {
    case '=':
      return new VariableAssignmentExpression(m_lhs, m_rhs, loc());
    case T_PLUS_EQUAL:
      return new VariablePlusEqualExpression(m_lhs, m_rhs, loc());
    case T_MINUS_EQUAL:
      return new VariableMinusEqualExpression(m_lhs, m_rhs, loc());
    case T_MUL_EQUAL:
      return new VariableMulEqualExpression(m_lhs, m_rhs, loc());
    case T_AND_EQUAL:
      return new VariableAndEqualExpression(m_lhs, m_rhs, loc());
    case T_OR_EQUAL:
      return new VariableOrEqualExpression(m_lhs, m_rhs, loc());
    case T_XOR_EQUAL:
      return new VariableXorEqualExpression(m_lhs, m_rhs, loc());
    case T_SL_EQUAL:
      return new VariableSLEqualExpression(m_lhs, m_rhs, loc());
    case T_SR_EQUAL:
      return new VariableSREqualExpression(m_lhs, m_rhs, loc());
    default:
      break;
    }
  }
  return NULL;
}

Variant AssignmentOpExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  if (m_op == '=') return m_lhs->set(env, rhs);
  return m_lhs->setOp(env, m_op, rhs);
}

void AssignmentOpExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " ";
  switch (m_op) {
  case '=':            out << "=";   break;
  case T_PLUS_EQUAL:   out << "+=";  break;
  case T_MINUS_EQUAL:  out << "-=";  break;
  case T_MUL_EQUAL:    out << "*=";  break;
  case T_DIV_EQUAL:    out << "/=";  break;
  case T_CONCAT_EQUAL: out << ".=";  break;
  case T_MOD_EQUAL:    out << "%=";  break;
  case T_AND_EQUAL:    out << "&=";  break;
  case T_OR_EQUAL:     out << "|=";  break;
  case T_XOR_EQUAL:    out << "^=";  break;
  case T_SL_EQUAL:     out << "<<="; break;
  case T_SR_EQUAL:     out << ">>="; break;
  default:
    ASSERT(false);
  }
  out << " ";
  m_rhs->dump(out);
}

Variant AssignmentOpExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

VariableAssignmentExpression::VariableAssignmentExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableAssignmentExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableAssignmentExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  return lhs.assignVal(rhs);
}

Variant VariableAssignmentExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableAssignmentExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " = ";
  m_rhs->dump(out);
}

VariablePlusEqualExpression::VariablePlusEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariablePlusEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

static Variant VariableAssignmentOpHelper(
  LvalExpression *lhsExp, Variant &lhs, Variant &rhs, int op) {
  if (RuntimeOption::EnableStrict) {
    if (!VariableExpression::CheckCompatibleAssignment(lhs, rhs)) {
      throw_strict(TypeVariableChangeException(
                   location_to_string(lhsExp->loc())),
                   StrictMode::StrictHardCore);
    }
  }
  return lhsExp->LvalExpression::setOpVariant(lhs, op, rhs);
}

Variant VariablePlusEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv += rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_PLUS_EQUAL);
}

Variant VariablePlusEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariablePlusEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " += ";
  m_rhs->dump(out);
}

VariableMinusEqualExpression::VariableMinusEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableMinusEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableMinusEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv -= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_MINUS_EQUAL);
}

Variant VariableMinusEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableMinusEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " -= ";
  m_rhs->dump(out);
}

VariableMulEqualExpression::VariableMulEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableMulEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableMulEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv *= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_MUL_EQUAL);
}

Variant VariableMulEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableMulEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " *= ";
  m_rhs->dump(out);
}

VariableAndEqualExpression::VariableAndEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableAndEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableAndEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv &= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_AND_EQUAL);
}

Variant VariableAndEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableAndEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " &= ";
  m_rhs->dump(out);
}

VariableOrEqualExpression::VariableOrEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableOrEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableOrEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv |= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_OR_EQUAL);
}

Variant VariableOrEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableOrEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " |= ";
  m_rhs->dump(out);
}

VariableXorEqualExpression::VariableXorEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableXorEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableXorEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv ^= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_XOR_EQUAL);
}

Variant VariableXorEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableXorEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " ^= ";
  m_rhs->dump(out);
}

VariableSLEqualExpression::VariableSLEqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableSLEqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableSLEqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv <<= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_SL_EQUAL);
}

Variant VariableSLEqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableSLEqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " <<= ";
  m_rhs->dump(out);
}

VariableSREqualExpression::VariableSREqualExpression(
  LvalExpressionPtr lhs, ExpressionPtr rhs, const Location *loc)
  : Expression(KindOfVariableSREqualExpression, loc),
  m_lhs(lhs), m_rhs(rhs) {}

Variant VariableSREqualExpression::eval(VariableEnvironment &env) const {
  Variant rhs(m_rhs->eval(env));
  Variant &lhs = VariableExpression::GetVariableByRef(env, m_lhs.get());
  Variant::TypedValueAccessor lhsAcc = lhs.getTypedAccessor();
  DataType lhsType = Variant::GetAccessorType(lhsAcc);
  Variant::TypedValueAccessor rhsAcc = rhs.getTypedAccessor();
  DataType rhsType = Variant::GetAccessorType(rhsAcc);
  if (lhsType == HPHP::KindOfInt64 && rhsType == HPHP::KindOfInt64) {
    int64 &lv = *Variant::GetInt64Data(lhsAcc);
    int64 rv = Variant::GetInt64(rhsAcc);
    return lv >>= rv;
  }
  return VariableAssignmentOpHelper(m_lhs.get(), lhs, rhs, T_SR_EQUAL);
}

Variant VariableSREqualExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}

void VariableSREqualExpression::dump(std::ostream &out) const {
  m_lhs->dump(out);
  out << " >>= ";
  m_rhs->dump(out);
}

}
}

