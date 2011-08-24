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

#include <runtime/eval/ast/inc_op_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/eval/ast/variable_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

IncOpExpression::IncOpExpression(EXPRESSION_ARGS, LvalExpressionPtr exp,
                                 bool inc, bool front)
  : Expression(KindOfIncOpExpression, EXPRESSION_PASS),
   m_exp(exp), m_inc(inc), m_front(front) {}

Variant IncOpExpression::evalUncommon(
  LvalExpression *lval, Variant &left, CVarRef right,
  bool inc, const Location &self, const Location *other) {
  if (RuntimeOption::EnableStrict) {
    if (!VariableExpression::CheckCompatibleAssignment(left, right)) {
      // inline SET_LINE
      if (!set_line(self.line0, self.char0, self.line1, self.char1)) {
        return Variant::lvalBlackHole();
      }
      throw_strict(TypeVariableChangeException(
                   location_to_string(other)),
                   StrictMode::StrictHardCore);
    }
  }
  // inline SET_LINE
  if (!set_line(self.line0, self.char0, self.line1, self.char1)) {
    return Variant::lvalBlackHole();
  }
  return lval->setOpVariant(left, inc ? (int)T_INC : (int)T_DEC, right);
}

Expression *IncOpExpression::optimize(VariableEnvironment &env) {
  if (!m_exp->isKindOf(Expression::KindOfVariableExpression)) return NULL;
  VariableExpression *var = static_cast<VariableExpression *>(m_exp.get());
  if (m_inc) {
    if (m_front) {
      return new IncVariableExpression(VariableExpressionPtr(var), loc());
    } else {
      return new VariableIncExpression(VariableExpressionPtr(var), loc());
    }
  }
  if (m_front) {
    return new DecVariableExpression(VariableExpressionPtr(var), loc());
  } else {
    return new VariableDecExpression(VariableExpressionPtr(var), loc());
  }
}

Variant IncOpExpression::eval(VariableEnvironment &env) const {
  if (m_exp->isKindOf(Expression::KindOfVariableExpression)) {
    Variant &lhs = m_exp->lval(env);
    Variant::TypedValueAccessor acc = lhs.getTypedAccessor();
    DataType t = Variant::GetAccessorType(acc);
    if (t == HPHP::KindOfInt64) {
      int64 &v = *Variant::GetInt64Data(acc);
      return m_inc ? (m_front ? ++v : v++) : (m_front ? --v : v--);
    }
    CVarRef rhs = m_front ? null : Variant(0);
    return evalUncommon(m_exp.get(), lhs, rhs, m_inc, m_loc, m_exp->loc());
  }
  SET_LINE;
  return m_exp->setOp(env, m_inc ? T_INC : T_DEC, m_front ? null : Variant(0));
}

Variant IncOpExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  if (m_front) {
    return strongBind(eval(env));
  } else {
    return strongBind(Expression::refval(env, strict));
  }
}

void IncOpExpression::dump(std::ostream &out) const {
  if (m_front) {
    out << (m_inc ? "++" : "--");
  }
  m_exp->dump(out);
  if (!m_front) {
    out << (m_inc ? "++" : "--");
  }
}

IncVariableExpression::IncVariableExpression(VariableExpressionPtr var,
  const Location *loc)
  : Expression(KindOfIncVariableExpression, loc), m_var(var) {}
VariableIncExpression::VariableIncExpression(VariableExpressionPtr var,
  const Location *loc)
  : Expression(KindOfIncVariableExpression, loc), m_var(var) {}
DecVariableExpression::DecVariableExpression(VariableExpressionPtr var,
  const Location *loc)
  : Expression(KindOfDecVariableExpression, loc), m_var(var) {}
VariableDecExpression::VariableDecExpression(VariableExpressionPtr var,
  const Location *loc)
  : Expression(KindOfDecVariableExpression, loc), m_var(var) {}

Variant IncVariableExpression::eval(VariableEnvironment &env) const {
  Variant &lhs = m_var->getRef(env);
  Variant::TypedValueAccessor acc = lhs.getTypedAccessor();
  DataType t = Variant::GetAccessorType(acc);
  if (t == HPHP::KindOfInt64) {
    int64 &v = *Variant::GetInt64Data(acc);
    return ++v;
  }
  return IncOpExpression::evalUncommon(m_var.get(), lhs, null, true,
                                       m_loc, m_var->loc());
}

Variant VariableIncExpression::eval(VariableEnvironment &env) const {
  Variant &lhs = m_var->getRef(env);
  Variant::TypedValueAccessor acc = lhs.getTypedAccessor();
  DataType t = Variant::GetAccessorType(acc);
  if (t == HPHP::KindOfInt64) {
    int64 &v = *Variant::GetInt64Data(acc);
    return v++;
  }
  return IncOpExpression::evalUncommon(m_var.get(), lhs, 0, true,
                                       m_loc, m_var->loc());
}

Variant DecVariableExpression::eval(VariableEnvironment &env) const {
  Variant &lhs = m_var->getRef(env);
  Variant::TypedValueAccessor acc = lhs.getTypedAccessor();
  DataType t = Variant::GetAccessorType(acc);
  if (t == HPHP::KindOfInt64) {
    int64 &v = *Variant::GetInt64Data(acc);
    return --v;
  }
  return IncOpExpression::evalUncommon(m_var.get(), lhs, null, false,
                                       m_loc, m_var->loc());
}

Variant VariableDecExpression::eval(VariableEnvironment &env) const {
  Variant &lhs = m_var->getRef(env);
  Variant::TypedValueAccessor acc = lhs.getTypedAccessor();
  DataType t = Variant::GetAccessorType(acc);
  if (t == HPHP::KindOfInt64) {
    int64 &v = *Variant::GetInt64Data(acc);
    return v--;
  }
  return IncOpExpression::evalUncommon(m_var.get(), lhs, 0, false,
                                       m_loc, m_var->loc());
}

Variant IncVariableExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}
Variant VariableIncExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(Expression::refval(env, strict));
}
Variant DecVariableExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(eval(env));
}
Variant VariableDecExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(Expression::refval(env, strict));
}

void IncVariableExpression::dump(std::ostream &out) const {
  out << "++";
  m_var->dump(out);
}
void VariableIncExpression::dump(std::ostream &out) const {
  m_var->dump(out);
  out << "++";
}
void DecVariableExpression::dump(std::ostream &out) const {
  out << "--";
  m_var->dump(out);
}
void VariableDecExpression::dump(std::ostream &out) const {
  m_var->dump(out);
  out << "--";
}

///////////////////////////////////////////////////////////////////////////////
}

}

