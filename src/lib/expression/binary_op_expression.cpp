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

#include <lib/expression/binary_op_expression.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/constant_expression.h>
#include <cpp/base/type_string.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/comparisons.h>
#include <lib/expression/expression_list.h>
#include <lib/expression/simple_function_call.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BinaryOpExpression::BinaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr exp1, ExpressionPtr exp2, int op)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp1(exp1), m_exp2(exp2), m_op(op), m_effect(false) {
  switch (m_op) {
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case T_DIV_EQUAL:
  case T_CONCAT_EQUAL:
  case T_MOD_EQUAL:
  case T_AND_EQUAL:
  case T_OR_EQUAL:
  case T_XOR_EQUAL:
  case T_SL_EQUAL:
  case T_SR_EQUAL:
    m_effect = true;
    m_exp1->setContext(Expression::LValue);
    if (m_exp1->is(Expression::KindOfObjectPropertyExpression)) {
      m_exp1->setContext(Expression::NoLValueWrapper);
    }
    // The binary op expression itself must always be handled in
    // a by-value fashion, even when it is passed to a function that
    // expects an argument by reference.
    setContext(Expression::NoRefWrapper);
    break;
  case T_INSTANCEOF:
    //m_exp1->setContext(Expression::ObjectContext);
    // Fall through
  default:
    m_effect = (m_exp1->hasEffect() || m_exp2->hasEffect());
    break;
  }
}

ExpressionPtr BinaryOpExpression::clone() {
  BinaryOpExpressionPtr exp(new BinaryOpExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exp1 = Clone(m_exp1);
  exp->m_exp2 = Clone(m_exp2);
  return exp;
}

bool BinaryOpExpression::isLiteralString() const {
  if (m_op == '.') {
    return m_exp1->isLiteralString() && m_exp2->isLiteralString();
  }
  return false;
}

std::string BinaryOpExpression::getLiteralString() const {
  if (m_op == '.') {
    return m_exp1->getLiteralString() + m_exp2->getLiteralString();
  }
  return "";
}

bool BinaryOpExpression::isShortCircuitOperator() const {
  switch (m_op) {
  case T_BOOLEAN_OR:
  case T_BOOLEAN_AND:
  case T_LOGICAL_OR:
  case T_LOGICAL_AND:
    return true;
  default:
    break;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void BinaryOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_op == T_INSTANCEOF && m_exp2->is(Expression::KindOfScalarExpression)) {
    ScalarExpressionPtr s = dynamic_pointer_cast<ScalarExpression>(m_exp2);
    addUserClass(ar, s->getString());
  }
  m_exp1->analyzeProgram(ar);
  m_exp2->analyzeProgram(ar);
}

ExpressionPtr BinaryOpExpression::simplifyLogical(AnalysisResultPtr ar) {
  if (m_exp1->is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr con =
      dynamic_pointer_cast<ConstantExpression>(m_exp1);
    if (con->isBoolean()) {
      if (con->getBooleanValue()) {
        if (ar->getPhase() >= AnalysisResult::PostOptimize) {
          // true && v (true AND v) => v
          ASSERT(m_exp2->getType()->is(Type::KindOfBoolean));
          if (m_op == T_BOOLEAN_AND || m_op == T_LOGICAL_AND) return m_exp2;
        }
        // true || v (true OR v) => true
        if (m_op == T_BOOLEAN_OR || m_op == T_LOGICAL_OR) {
          return CONSTANT("true");
        }
      } else {
        if (ar->getPhase() >= AnalysisResult::PostOptimize) {
          ASSERT(m_exp2->getType()->is(Type::KindOfBoolean));
          // false || v (false OR v) => v
          if (m_op == T_BOOLEAN_OR || m_op == T_LOGICAL_OR) return m_exp2;
        }
        // false && v (false AND v) => false
        if (m_op == T_BOOLEAN_AND || m_op == T_LOGICAL_AND) {
          return CONSTANT("false");
        }
      }
    }
  }
  if (m_exp2->is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr con =
      dynamic_pointer_cast<ConstantExpression>(m_exp2);
    if (con->isBoolean()) {
      if (con->getBooleanValue()) {
        if (ar->getPhase() >= AnalysisResult::PostOptimize) {
          ASSERT(m_exp1->getType()->is(Type::KindOfBoolean));
          // v && true (v AND true) => v
          if (m_op == T_BOOLEAN_AND || m_op == T_LOGICAL_AND) return m_exp1;
        }
        // v || true (v OR true) => true when v does not have effect
        if (m_op == T_BOOLEAN_OR || m_op == T_LOGICAL_OR) {
          if (!m_exp1->hasEffect()) return CONSTANT("true");
        }
      } else {
        if (ar->getPhase() >= AnalysisResult::PostOptimize) {
          ASSERT(m_exp1->getType()->is(Type::KindOfBoolean));
          // v || false (v OR false) => v
          if (m_op == T_BOOLEAN_OR || m_op == T_LOGICAL_OR) return m_exp1;
        }
        // v && false (v AND false) => false when v does not have effect
        if (m_op == T_BOOLEAN_AND || m_op == T_LOGICAL_AND) {
          if (!m_exp1->hasEffect()) return CONSTANT("false");
        }
      }
    }
  }
  return ExpressionPtr();
}

ConstructPtr BinaryOpExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp1;
    case 1:
      return m_exp2;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int BinaryOpExpression::getKidCount() const {
  return 2;
}

int BinaryOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp1 = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_exp2 = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr BinaryOpExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp1);
  ar->preOptimize(m_exp2);
  if (!m_exp2->isScalar()) {
    if (!m_exp1->isScalar()) return ExpressionPtr();
  }
  ExpressionPtr optExp;
  try {
    optExp = foldConst(ar);
  } catch (Exception &e) {
    // cpp/base threw an exception, perhaps bad operands
  }
  if (optExp) return optExp;
  if (isShortCircuitOperator()) return simplifyLogical(ar);
  return ExpressionPtr();
}

ExpressionPtr BinaryOpExpression::simplifyArithmetic(AnalysisResultPtr ar) {
  Variant v1;
  Variant v2;
  if (m_exp1->getScalarValue(v1)) {
    if (v1.isInteger()) {
      int64 ival1 = v1.toInt64();
      // 1 * $a => $a, 0 + $a => $a
      if ((ival1 == 1 && m_op == '*') || (ival1 == 0 && m_op == '+')) {
        TypePtr actType2 = m_exp2->getActualType();
        TypePtr expType2 = m_exp2->getExpectedType();
        if ((actType2->isInteger() || actType2->is(Type::KindOfDouble)) ||
            (expType2 && expType2->is(Type::KindOfNumeric) &&
             Type::IsCastNeeded(ar, actType2, expType2))) {
          return m_exp2;
        }
      }
    } else if (v1.isString()) {
      String sval1 = v1.toString();
      if ((sval1.empty() && m_op == '.')) {
        TypePtr actType2 = m_exp2->getActualType();
        TypePtr expType2 = m_exp2->getExpectedType();
        // '' . $a => $a
        if (actType2->is(Type::KindOfString) ||
            (expType2 && expType2->is(Type::KindOfString) &&
             Type::IsCastNeeded(ar, actType2, expType2))) {
          return m_exp2;
        }
      }
    }
  }
  if (m_exp2->getScalarValue(v2)) {
    if (v2.isInteger()) {
      int64 ival2 = v2.toInt64();
      // $a * 1 => $a, $a + 0 => $a
      if ((ival2 == 1 && m_op == '*') || (ival2 == 0 && m_op == '+')) {
        TypePtr actType1 = m_exp1->getActualType();
        TypePtr expType1 = m_exp1->getExpectedType();
        if ((actType1->isInteger() || actType1->is(Type::KindOfDouble)) ||
            (expType1 && expType1->is(Type::KindOfNumeric) &&
             Type::IsCastNeeded(ar, actType1, expType1))) {
          return m_exp1;
        }
      }
    } else if (v2.isString()) {
      String sval2 = v2.toString();
      if ((sval2.empty() && m_op == '.')) {
        TypePtr actType1 = m_exp1->getActualType();
        TypePtr expType1 = m_exp1->getExpectedType();
        // $a . '' => $a
        if (actType1->is(Type::KindOfString) ||
            (expType1 && expType1->is(Type::KindOfString) &&
             Type::IsCastNeeded(ar, actType1, expType1))) {
          return m_exp1;
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr BinaryOpExpression::makeConcatCall(AnalysisResultPtr ar,
                                                 int count,
                                                 ExpressionListPtr expList) {
  if (count <= MAX_CONCAT_ARGS) {
    if (count > 2) {
      char fname[20];
      snprintf(fname, sizeof(fname), "concat%d", count);
      SimpleFunctionCallPtr call =
        SimpleFunctionCallPtr(new SimpleFunctionCall(getLocation(),
          Expression::KindOfSimpleFunctionCall, fname, expList, NULL));
      call->setValid();
      call->setNoPrefix();
      call->setActualType(Type::String);
      return call;
    } else if (count == 2) {
      BinaryOpExpressionPtr result =
        BinaryOpExpressionPtr(new BinaryOpExpression(getLocation(),
          Expression::KindOfBinaryOpExpression,
          (*expList)[0], (*expList)[1], '.'));
      result->setActualType(Type::String);
      return result;
    } else {
      return (*expList)[0];
    }
  } else {
    ExpressionListPtr segList =
      ExpressionListPtr(new ExpressionList(getLocation(),
        Expression::KindOfExpressionList));
    for (int i = 0; i < MAX_CONCAT_ARGS; i++) {
      segList->addElement((*expList)[count - MAX_CONCAT_ARGS + i]);
    }
    for (int i = 0; i < MAX_CONCAT_ARGS; i++) {
      expList->removeElement(count - MAX_CONCAT_ARGS);
    }
    ExpressionPtr call2 = makeConcatCall(ar, MAX_CONCAT_ARGS, segList);
    ExpressionPtr call1 = makeConcatCall(ar, count - MAX_CONCAT_ARGS, expList);
    BinaryOpExpressionPtr result =
      BinaryOpExpressionPtr(new BinaryOpExpression(getLocation(),
        Expression::KindOfBinaryOpExpression,
        call1, call2, '.'));
    call1->setExpectedType(Type::String);
    call2->setExpectedType(Type::String);
    result->setActualType(Type::String);
    return result;
  }
}

ExpressionPtr BinaryOpExpression::mergeConcat(AnalysisResultPtr ar) {
  ExpressionPtr exp1 = m_exp1;
  ExpressionListPtr expList =
    ExpressionListPtr(new ExpressionList(getLocation(),
      Expression::KindOfExpressionList));
  expList->insertElement(m_exp2);
  do {
    if (!exp1->is(Expression::KindOfBinaryOpExpression)) break;
    BinaryOpExpressionPtr binOpExp =
      dynamic_pointer_cast<BinaryOpExpression>(exp1);
    if (binOpExp->m_op == '.') {
      expList->insertElement(binOpExp->m_exp2);
    } else {
      break;
    }
    exp1 = binOpExp->m_exp1;
  } while (true);
  expList->insertElement(exp1);
  int count = expList->getCount();
  ASSERT(count >= 2);
  if (count == 2) return ExpressionPtr();
  // work around g++ function arguments evaluation order (right to left),
  // cannot have two arguments with side effect.
  bool seenEffect = false;
  for (int i = 0; i < count; i++) {
    if ((*expList)[i]->hasEffect()) {
      if (seenEffect) return ExpressionPtr();
      seenEffect = true;
    }
  }
  ExpressionPtr result = makeConcatCall(ar, count, expList);
  result->setActualType(getActualType());
  result->setExpectedType(getExpectedType());
  return result;
}

ExpressionPtr BinaryOpExpression::postOptimize(AnalysisResultPtr ar) {
  ExpressionPtr optExp;
  if (m_op == '.') {
    optExp = mergeConcat(ar);
    if (optExp) return optExp;
  }
  ar->postOptimize(m_exp1);
  ar->postOptimize(m_exp2);
  optExp = simplifyArithmetic(ar);
  if (optExp) return optExp;
  if (isShortCircuitOperator()) return simplifyLogical(ar);
  return ExpressionPtr();
}

ExpressionPtr BinaryOpExpression::foldConst(AnalysisResultPtr ar) {
  ExpressionPtr optExp;
  Variant v1;
  Variant v2;

  if (!m_exp2->getScalarValue(v2)) return ExpressionPtr();

  if (m_exp1->isScalar()) {
    if (!m_exp1->getScalarValue(v1)) return ExpressionPtr();
    Variant result;
    switch (m_op) {
    case T_LOGICAL_XOR:
      result = logical_xor(v1, v2); break;
    case '|':
      result = bitwise_or(v1, v2); break;
    case '&':
      result = bitwise_and(v1, v2); break;
    case '^':
      result = bitwise_xor(v1, v2); break;
    case '.':
      result = concat(v1, v2); break;
    case T_IS_IDENTICAL:
      result = same(v1, v2); break;
    case T_IS_NOT_IDENTICAL:
      result = !same(v1, v2); break;
    case T_IS_EQUAL:
      result = equal(v1, v2); break;
    case T_IS_NOT_EQUAL:
      result = !equal(v1, v2); break;
    case '<':
      result = less(v1, v2); break;
    case T_IS_SMALLER_OR_EQUAL:
      result = not_more(v1, v2); break;
    case '>':
      result = more(v1, v2); break;
    case T_IS_GREATER_OR_EQUAL:
      result = not_less(v1, v2); break;
    case '+':
      result = plus(v1, v2); break;
    case '-':
      result = minus(v1, v2); break;
    case '*':
      result = multiply(v1, v2); break;
    case '/':
      if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
        return ExpressionPtr();
      }
      result = divide(v1, v2); break;
    case '%':
      if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
        return ExpressionPtr();
      }
      result = modulo(v1, v2); break;
    case T_SL:
      result = shift_left(v1, v2); break;
    case T_SR:
      result = shift_right(v1, v2); break;
    case T_BOOLEAN_OR:
      result = v1 || v2; break;
    case T_BOOLEAN_AND:
      result = v1 && v2; break;
    case T_LOGICAL_OR:
      result = v1 || v2; break;
    case T_LOGICAL_AND:
      result = v1 && v2; break;
    default:
      break;
    }
    if (result.isNull()) {
      return CONSTANT("null");
    } else if (result.isBoolean()) {
      return CONSTANT(result ? "true" : "false");
    } else if (result.isDouble() && !finite(result.getDouble())) {
      return ExpressionPtr();
    } else {
      return ScalarExpressionPtr
             (new ScalarExpression(getLocation(),
                                   Expression::KindOfScalarExpression,
                                   result));
    }
  } else {
    switch (m_op) {
    case T_LOGICAL_XOR:
    case '|':
    case '&':
    case '^':
    case '.':
    case '+':
    case '*':
    case T_BOOLEAN_OR:
    case T_BOOLEAN_AND:
    case T_LOGICAL_OR:
    case T_LOGICAL_AND:
      optExp = foldConstRightAssoc(ar);
      if (optExp) return optExp;
    default:
      break;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr
BinaryOpExpression::foldConstRightAssoc(AnalysisResultPtr ar) {
  ExpressionPtr optExp1;
  switch (m_op) {
  case '.':
  case '+':
  case '*':
    if (m_exp1->is(Expression::KindOfBinaryOpExpression)) {
      BinaryOpExpressionPtr binOpExp =
        dynamic_pointer_cast<BinaryOpExpression>(m_exp1);
      if (binOpExp->m_op == m_op) {
        // turn a Op b Op c, namely (a Op b) Op c into a Op (b Op c)
        ExpressionPtr aExp = binOpExp->m_exp1;
        ExpressionPtr bExp = binOpExp->m_exp2;
        ExpressionPtr cExp = m_exp2;
        BinaryOpExpressionPtr bcExp =
          BinaryOpExpressionPtr(new BinaryOpExpression(getLocation(),
            Expression::KindOfBinaryOpExpression,
            bExp, cExp, m_op));
        ExpressionPtr optExp = bcExp->foldConst(ar);
        if (optExp) {
          BinaryOpExpressionPtr a_bcExp
            (new BinaryOpExpression(getLocation(),
                                    Expression::KindOfBinaryOpExpression,
                                    aExp, optExp, m_op));
          optExp = a_bcExp->foldConstRightAssoc(ar);
          if (optExp) return optExp;
          else return a_bcExp;
        }
      }
    }
    break;
  default:
    break;
  }
  return ExpressionPtr();
}

TypePtr BinaryOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                       bool coerce) {
  TypePtr et1;          // expected m_exp1's type
  bool coerce1 = false; // whether m_exp1 needs to coerce to et1
  TypePtr et2;          // expected m_exp2's type
  bool coerce2 = false; // whether m_exp2 needs to coerce to et2
  TypePtr rt;           // return type

  switch (m_op) {
  case '+':
  case T_PLUS_EQUAL:
    if (coerce && Type::SameType(type, Type::Array)) {
      et1 = et2 = Type::Array;
      coerce1 = coerce2 = true;
      rt = Type::Array;
    } else {
      et1 = NEW_TYPE(PlusOperand);
      et2 = NEW_TYPE(PlusOperand);
      rt  = NEW_TYPE(PlusOperand);
    }
    break;
  case '-':
  case '*':
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case '/':
  case T_DIV_EQUAL:
    et1 = NEW_TYPE(Numeric);
    et2 = NEW_TYPE(Numeric);
    rt  = NEW_TYPE(Numeric);
    break;
  case '.':
    et1 = et2 = rt = Type::String;
    break;
  case T_CONCAT_EQUAL:
    et1 = et2 = Type::String;
    rt = Type::Variant;
    break;
  case '%':
    et1 = et2 = Type::Int64;
    rt  = NEW_TYPE(Numeric);
    break;
  case T_MOD_EQUAL:
    et1 = NEW_TYPE(Numeric);
    et2 = Type::Int64;
    rt  = NEW_TYPE(Numeric);
    break;
  case '|':
  case '&':
  case '^':
  case T_AND_EQUAL:
  case T_OR_EQUAL:
  case T_XOR_EQUAL:
    et1 = NEW_TYPE(Primitive);
    et2 = NEW_TYPE(Primitive);
    rt  = NEW_TYPE(Primitive);
    break;
  case T_SL:
  case T_SR:
  case T_SL_EQUAL:
  case T_SR_EQUAL:
    et1 = et2 = rt = Type::Int64;
    break;
  case T_BOOLEAN_OR:
  case T_BOOLEAN_AND:
  case T_LOGICAL_OR:
  case T_LOGICAL_AND:
  case T_LOGICAL_XOR:
    et1 = et2 = rt = Type::Boolean;
    break;
  case '<':
  case T_IS_SMALLER_OR_EQUAL:
  case '>':
  case T_IS_GREATER_OR_EQUAL:
  case T_IS_IDENTICAL:
  case T_IS_NOT_IDENTICAL:
  case T_IS_EQUAL:
  case T_IS_NOT_EQUAL:
    et1 = NEW_TYPE(Some);
    et2 = NEW_TYPE(Some);
    rt = Type::Boolean;
    break;

  case T_INSTANCEOF:
    et1 = Type::CreateType(Type::KindOfAny);
    et2 = Type::String;
    rt = Type::Boolean;
    break;
  default:
    ASSERT(false);
  }

  switch (m_op) {
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case T_DIV_EQUAL:
  case T_MOD_EQUAL:
  case T_AND_EQUAL:
  case T_OR_EQUAL:
  case T_XOR_EQUAL:
  case T_SL_EQUAL:
  case T_SR_EQUAL:
    {
      TypePtr ret = m_exp2->inferAndCheck(ar, et2, coerce2);
      m_exp1->inferAndCheck(ar, ret, true);
    }
    break;
  case T_CONCAT_EQUAL:
    {
      TypePtr ret = m_exp2->inferAndCheck(ar, et2, coerce2);
      m_exp1->inferAndCheck(ar, ret, true);
      TypePtr act1 = m_exp1->getActualType();
      if (act1 && act1->is(Type::KindOfString)) rt = Type::String;
    }
    break;
  case '+':
  case '-':
  case '*':
    {
      m_exp1->inferAndCheck(ar, et1, coerce1);
      m_exp2->inferAndCheck(ar, et2, coerce2);
      TypePtr act1 = m_exp1->getActualType();
      TypePtr act2 = m_exp2->getActualType();
      bool bothEffect = m_exp1->hasEffect() && m_exp2->hasEffect();

      TypePtr combined = Type::combinedPrimType(act1, act2);
      if (combined &&
          (combined->isPrimitive() || !rt->isPrimitive())) {
        if (combined->isPrimitive() && bothEffect) {
          combined = Type::GetType(Type::KindOfNumeric);
        }
        rt = combined;
      }
    }
    break;
  default:
    m_exp1->inferAndCheck(ar, et1, coerce1);
    m_exp2->inferAndCheck(ar, et2, coerce2);
    break;
  }
  return rt;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BinaryOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_exp1->outputPHP(cg, ar);

  switch (m_op) {
  case T_PLUS_EQUAL:          cg.printf(" += ");         break;
  case T_MINUS_EQUAL:         cg.printf(" -= ");         break;
  case T_MUL_EQUAL:           cg.printf(" *= ");         break;
  case T_DIV_EQUAL:           cg.printf(" /= ");         break;
  case T_CONCAT_EQUAL:        cg.printf(" .= ");         break;
  case T_MOD_EQUAL:           cg.printf(" %%= ");        break;
  case T_AND_EQUAL:           cg.printf(" &= ");         break;
  case T_OR_EQUAL:            cg.printf(" |= ");         break;
  case T_XOR_EQUAL:           cg.printf(" ^= ");         break;
  case T_SL_EQUAL:            cg.printf(" <<= ");        break;
  case T_SR_EQUAL:            cg.printf(" >>= ");        break;
  case T_BOOLEAN_OR:          cg.printf(" || ");         break;
  case T_BOOLEAN_AND:         cg.printf(" && ");         break;
  case T_LOGICAL_OR:          cg.printf(" or ");         break;
  case T_LOGICAL_AND:         cg.printf(" and ");        break;
  case T_LOGICAL_XOR:         cg.printf(" xor ");        break;
  case '|':                   cg.printf(" | ");          break;
  case '&':                   cg.printf(" & ");          break;
  case '^':                   cg.printf(" ^ ");          break;
  case '.':                   cg.printf(" . ");          break;
  case '+':                   cg.printf(" + ");          break;
  case '-':                   cg.printf(" - ");          break;
  case '*':                   cg.printf(" * ");          break;
  case '/':                   cg.printf(" / ");          break;
  case '%':                   cg.printf(" %% ");         break;
  case T_SL:                  cg.printf(" << ");         break;
  case T_SR:                  cg.printf(" >> ");         break;
  case T_IS_IDENTICAL:        cg.printf(" === ");        break;
  case T_IS_NOT_IDENTICAL:    cg.printf(" !== ");        break;
  case T_IS_EQUAL:            cg.printf(" == ");         break;
  case T_IS_NOT_EQUAL:        cg.printf(" != ");         break;
  case '<':                   cg.printf(" < ");          break;
  case T_IS_SMALLER_OR_EQUAL: cg.printf(" <= ");         break;
  case '>':                   cg.printf(" > ");          break;
  case T_IS_GREATER_OR_EQUAL: cg.printf(" >= ");         break;
  case T_INSTANCEOF:          cg.printf(" instanceof "); break;
  default:
    ASSERT(false);
  }

  m_exp2->outputPHP(cg, ar);
}

void BinaryOpExpression::outputCPPImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  bool bothEffect = m_exp1->hasEffect() && m_exp2->hasEffect();
  // Reverse exceptions
  if (bothEffect) {
    switch (m_op) {
    case T_PLUS_EQUAL:
    case T_MINUS_EQUAL:
    case T_MUL_EQUAL:
    case T_DIV_EQUAL:
    case T_MOD_EQUAL:
    case T_AND_EQUAL:
    case T_OR_EQUAL:
    case T_XOR_EQUAL:
    case T_SL_EQUAL:
    case T_SR_EQUAL:
    case T_BOOLEAN_OR:
    case T_BOOLEAN_AND:
    case T_LOGICAL_OR:
    case T_LOGICAL_AND:
    case T_INSTANCEOF:
      bothEffect = false;
    default:
      break;
    }
  }

  bool wrapped = true;
  switch (m_op) {
  case T_CONCAT_EQUAL:        cg.printf("concat_assign"); break;
  case T_LOGICAL_XOR:         cg.printf("logical_xor");   break;
  case '|':                   cg.printf("bitwise_or");    break;
  case '&':                   cg.printf("bitwise_and");   break;
  case '^':                   cg.printf("bitwise_xor");   break;
  case '.':                   cg.printf("concat");        break;
  case T_IS_IDENTICAL:        cg.printf("same");          break;
  case T_IS_NOT_IDENTICAL:    cg.printf("!same");         break;
  case T_IS_EQUAL:            cg.printf("equal");         break;
  case T_IS_NOT_EQUAL:        cg.printf("!equal");        break;
  case '<':                   cg.printf("less");          break;
  case T_IS_SMALLER_OR_EQUAL: cg.printf("not_more");      break;
  case '>':                   cg.printf("more");          break;
  case T_IS_GREATER_OR_EQUAL: cg.printf("not_less");      break;
  case '/':                   cg.printf("divide");        break;
  case '%':                   cg.printf("modulo");        break;
  case T_INSTANCEOF:          cg.printf("instanceOf");    break;
  default:
    if (bothEffect) {
      switch (m_op) {
      case '+':               cg.printf("plus");          break;
      case '-':               cg.printf("minus");         break;
      case '*':               cg.printf("multiply");      break;
      case T_SL:              cg.printf("shift_left");    break;
      case T_SR:              cg.printf("shift_right");   break;
      default:
        wrapped = false;
        break;
      }
    } else {
      wrapped = false;
    }
    break;
  }
  if (bothEffect) {
    cg.printf("_rev");
  }

  if (wrapped) cg.printf("(");

  ExpressionPtr first = m_exp1;
  ExpressionPtr second = m_exp2;
  if (bothEffect) {
    first = m_exp2;
    second = m_exp1;
  }

  // we could implement these functions natively on String and Array classes
  switch (m_op) {
  case '+':
  case '-':
  case '*':
  case '/': {
    TypePtr actualType = first->getActualType();

    if (actualType &&
        (actualType->is(Type::KindOfString) ||
         actualType->is(Type::KindOfArray))) {
      cg.printf("(Variant)(");
      first->outputCPP(cg, ar);
      cg.printf(")");
    } else {
      first->outputCPP(cg, ar);
    }
    break;
  }
  case T_SL:
  case T_SR:
    cg.printf("toInt64(");
    first->outputCPP(cg, ar);
    cg.printf(")");
    break;
  default:
    first->outputCPP(cg, ar);
    break;
  }

  switch (m_op) {
  case T_PLUS_EQUAL:          cg.printf(" += ");   break;
  case T_MINUS_EQUAL:         cg.printf(" -= ");   break;
  case T_MUL_EQUAL:           cg.printf(" *= ");   break;
  case T_DIV_EQUAL:           cg.printf(" /= ");   break;
  case T_MOD_EQUAL:           cg.printf(" %%= ");  break;
  case T_AND_EQUAL:           cg.printf(" &= ");   break;
  case T_OR_EQUAL:            cg.printf(" |= ");   break;
  case T_XOR_EQUAL:           cg.printf(" ^= ");   break;
  case T_SL_EQUAL:            cg.printf(" <<= ");  break;
  case T_SR_EQUAL:            cg.printf(" >>= ");  break;
  case T_BOOLEAN_OR:          cg.printf(" || ");   break;
  case T_BOOLEAN_AND:         cg.printf(" && ");   break;
  case T_LOGICAL_OR:          cg.printf(" || ");   break;
  case T_LOGICAL_AND:         cg.printf(" && ");   break;
  default:
    if (!bothEffect) {
      switch (m_op) {
      case '+':                   cg.printf(" + ");    break;
      case '-':                   cg.printf(" - ");    break;
      case '*':                   cg.printf(" * ");    break;
      case T_SL:                  cg.printf(" << ");   break;
      case T_SR:                  cg.printf(" >> ");   break;
      default:
        cg.printf(", ");
        break;
      }
    } else {
      cg.printf(", ");
    }
    break;
  }

  switch (m_op) {
  case '+':
  case '-':
  case '*':
  case '/': {
    TypePtr actualType = second->getActualType();

    if (actualType &&
        (actualType->is(Type::KindOfString) ||
         actualType->is(Type::KindOfArray))) {
      cg.printf("(Variant)(");
      second->outputCPP(cg, ar);
      cg.printf(")");
    } else {
      second->outputCPP(cg, ar);
    }
    break;
  }
  case T_INSTANCEOF:
    {
      if (second->is(Expression::KindOfScalarExpression)) {
        cg.printf("\"");
        second->outputCPP(cg, ar);
        cg.printf("\"");
      } else {
        second->outputCPP(cg, ar);
      }
      break;
    }
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
    {
      TypePtr t1 = first->getActualType();
      TypePtr t2 = second->getActualType();
      if (t1 && t2 && Type::IsCastNeeded(ar, t2, t1)) {
        t1->outputCPPCast(cg, ar);
        cg.printf("(");
        second->outputCPP(cg, ar);
        cg.printf(")");
      } else {
        second->outputCPP(cg, ar);
      }
      break;
    }
  default:
    second->outputCPP(cg, ar);
  }

  if (wrapped) cg.printf(")");
}
