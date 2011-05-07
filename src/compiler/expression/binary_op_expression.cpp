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

#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <util/parser/hphp.tab.hpp>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_string.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/encaps_list_expression.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/statement/loop_statement.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BinaryOpExpression::BinaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr exp1, ExpressionPtr exp2, int op)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp1(exp1), m_exp2(exp2), m_op(op), m_assign(false), m_canThrow(false) {
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
    m_assign = true;
    m_exp1->setContext(Expression::LValue);
    m_exp1->setContext(Expression::OprLValue);
    m_exp1->setContext(Expression::DeepOprLValue);
    if (m_exp1->is(Expression::KindOfObjectPropertyExpression)) {
      m_exp1->setContext(Expression::NoLValueWrapper);
    }
    break;
  case T_INSTANCEOF:
    //m_exp1->setContext(Expression::ObjectContext);
    // Fall through
  default:
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

bool BinaryOpExpression::isTemporary() const {
  switch (m_op) {
  case '+':
  case '-':
  case '*':
  case '/':
  case T_SL:
  case T_SR:
  case T_BOOLEAN_OR:
  case T_BOOLEAN_AND:
  case T_LOGICAL_OR:
  case T_LOGICAL_AND:
  case T_INSTANCEOF:
    return true;
  }
  return false;
}

bool BinaryOpExpression::isRefable(bool checkError /* = false */) const {
  return checkError && m_assign;
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

ExpressionPtr BinaryOpExpression::unneededHelper() {
  if (!isShortCircuitOperator() || !m_exp2->getContainedEffects()) {
    return Expression::unneededHelper();
  }

  m_exp2 = m_exp2->unneeded();
  m_exp2->setExpectedType(Type::Boolean);
  return static_pointer_cast<Expression>(shared_from_this());
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

int BinaryOpExpression::getLocalEffects() const {
  int effect = NoEffect;
  m_canThrow = false;
  switch (m_op) {
  case '/':
  case '%':
  case T_DIV_EQUAL:
  case T_MOD_EQUAL: {
    Variant v2;
    if (!m_exp2->getScalarValue(v2) || v2.equal(0)) {
      effect = CanThrow;
      m_canThrow = true;
    }
    break;
  }
  default:
    break;
  }
  if (m_assign) effect |= AssignEffect;
  return effect;
}

void BinaryOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal &&
      m_op == T_INSTANCEOF && m_exp2->is(Expression::KindOfScalarExpression)) {
    ScalarExpressionPtr s = dynamic_pointer_cast<ScalarExpression>(m_exp2);
    addUserClass(ar, s->getString());
  }
  m_exp1->analyzeProgram(ar);
  m_exp2->analyzeProgram(ar);
}

ExpressionPtr BinaryOpExpression::simplifyLogical(AnalysisResultConstPtr ar) {
  try {
    ExpressionPtr rep = foldConst(ar);
    if (rep) return replaceValue(rep);
  } catch (Exception e) {
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
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int BinaryOpExpression::getKidCount() const {
  return 2;
}

void BinaryOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
  case 0:
    m_exp1 = boost::dynamic_pointer_cast<Expression>(cp);
    break;
  case 1:
    m_exp2 = boost::dynamic_pointer_cast<Expression>(cp);
    break;
  default:
    ASSERT(false);
    break;
  }
}

bool BinaryOpExpression::canonCompare(ExpressionPtr e) const {
  return Expression::canonCompare(e) &&
    getOp() == static_cast<BinaryOpExpression*>(e.get())->getOp();
}

ExpressionPtr BinaryOpExpression::preOptimize(AnalysisResultConstPtr ar) {
  if (!m_exp2->isScalar()) {
    if (!m_exp1->isScalar()) {
      if (m_exp1->is(KindOfBinaryOpExpression)) {
        BinaryOpExpressionPtr b(
          dynamic_pointer_cast<BinaryOpExpression>(m_exp1));
        if (b->m_op == m_op && b->m_exp1->isScalar()) {
          return foldRightAssoc(ar);
        }
      }
      return ExpressionPtr();
    }
  } else if (m_canThrow && !(getLocalEffects() & CanThrow)) {
    recomputeEffects();
  }
  ExpressionPtr optExp;
  try {
    optExp = foldConst(ar);
  } catch (Exception &e) {
    // runtime/base threw an exception, perhaps bad operands
  }
  if (!optExp) {
    if (isShortCircuitOperator()) optExp = simplifyLogical(ar);
  }
  if (optExp) optExp = replaceValue(optExp);
  return optExp;
}

ExpressionPtr BinaryOpExpression::simplifyArithmetic(
  AnalysisResultConstPtr ar) {
  Variant v1;
  Variant v2;
  if (m_exp1->getScalarValue(v1)) {
    if (v1.isInteger()) {
      int64 ival1 = v1.toInt64();
      // 1 * $a => $a, 0 + $a => $a
      if ((ival1 == 1 && m_op == '*') || (ival1 == 0 && m_op == '+')) {
        TypePtr actType2 = m_exp2->getActualType();
        TypePtr expType2 = m_exp2->getExpectedType();
        if ((actType2 && actType2->mustBe(Type::KindOfNumeric)
                      && actType2->isExactType()) ||
            (expType2 && expType2->mustBe(Type::KindOfNumeric)
                      && Type::IsCastNeeded(ar, actType2, expType2))) {
          return m_exp2;
        }
      }
    } else if (v1.isString()) {
      String sval1 = v1.toString();
      if ((sval1.empty() && m_op == '.')) {
        TypePtr actType2 = m_exp2->getActualType();
        TypePtr expType2 = m_exp2->getExpectedType();
        // '' . $a => $a
        if ((actType2 && actType2->is(Type::KindOfString)) ||
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
        if ((actType1 && actType1->mustBe(Type::KindOfNumeric)
                      && actType1->isExactType()) ||
            (expType1 && expType1->mustBe(Type::KindOfNumeric)
                      && Type::IsCastNeeded(ar, actType1, expType1))) {
          return m_exp1;
        }
      }
    } else if (v2.isString()) {
      String sval2 = v2.toString();
      if ((sval2.empty() && m_op == '.')) {
        TypePtr actType1 = m_exp1->getActualType();
        TypePtr expType1 = m_exp1->getExpectedType();
        // $a . '' => $a
        if ((actType1 && actType1->is(Type::KindOfString)) ||
            (expType1 && expType1->is(Type::KindOfString) &&
             Type::IsCastNeeded(ar, actType1, expType1))) {
          return m_exp1;
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr BinaryOpExpression::postOptimize(AnalysisResultConstPtr ar) {
  ExpressionPtr optExp = simplifyArithmetic(ar);
  if (!optExp) {
    if (isShortCircuitOperator()) optExp = simplifyLogical(ar);
  }
  if (optExp) optExp = replaceValue(optExp);
  return optExp;
}

static ExpressionPtr makeIsNull(AnalysisResultConstPtr ar,
                                LocationPtr loc, ExpressionPtr exp,
                                bool invert) {
  /* Replace "$x === null" with an is_null call; this requires slightly
   * less work at runtime. */
  ExpressionListPtr expList =
    ExpressionListPtr(new ExpressionList(exp->getScope(), loc,
      Expression::KindOfExpressionList));
  expList->insertElement(exp);

  SimpleFunctionCallPtr call
    (new SimpleFunctionCall(exp->getScope(), loc,
                            Expression::KindOfSimpleFunctionCall,
                            "is_null", expList, ExpressionPtr()));

  call->setValid();
  call->setActualType(Type::Boolean);
  call->setupScopes(ar);

  ExpressionPtr result(call);
  if (invert) {
    result = ExpressionPtr(new UnaryOpExpression(
                             exp->getScope(), loc,
                             Expression::KindOfUnaryOpExpression,
                             result, '!', true));
  }

  return result;
}

ExpressionPtr BinaryOpExpression::foldConst(AnalysisResultConstPtr ar) {
  ExpressionPtr optExp;
  Variant v1;
  Variant v2;

  if (!m_exp2->getScalarValue(v2)) {
    if (m_exp1->isScalar() && m_exp1->getScalarValue(v1)) {
      switch (m_op) {
        case T_IS_IDENTICAL:
        case T_IS_NOT_IDENTICAL:
          if (v1.isNull()) {
            return makeIsNull(ar, getLocation(), m_exp2,
                              m_op == T_IS_NOT_IDENTICAL);
          }
          break;
        case T_LOGICAL_AND:
        case T_BOOLEAN_AND:
        case T_LOGICAL_OR:
        case T_BOOLEAN_OR: {
          ExpressionPtr rep =
            v1.toBoolean() == (m_op == T_LOGICAL_AND ||
                               m_op == T_BOOLEAN_AND) ? m_exp2 : m_exp1;
          rep = ExpressionPtr(
              new UnaryOpExpression(
                getScope(), getLocation(), KindOfUnaryOpExpression,
                rep, T_BOOL_CAST, true));
          rep->setActualType(Type::Boolean);
          return replaceValue(rep);
        }
        case '+':
        case '.':
        case '*':
        case '&':
        case '|':
        case '^':
          if (m_exp2->is(KindOfBinaryOpExpression)) {
            BinaryOpExpressionPtr binOpExp =
              dynamic_pointer_cast<BinaryOpExpression>(m_exp2);
            if (binOpExp->m_op == m_op && binOpExp->m_exp1->isScalar()) {
              ExpressionPtr aExp = m_exp1;
              ExpressionPtr bExp = binOpExp->m_exp1;
              ExpressionPtr cExp = binOpExp->m_exp2;
              m_exp1 = binOpExp = Clone(binOpExp);
              m_exp2 = cExp;
              binOpExp->m_exp1 = aExp;
              binOpExp->m_exp2 = bExp;
              if (ExpressionPtr optExp = binOpExp->foldConst(ar)) {
                m_exp1 = optExp;
              }
              return static_pointer_cast<Expression>(shared_from_this());
            }
          }
        break;
        default:
          break;
      }
    }

    return ExpressionPtr();
  }

  if (m_exp1->isScalar()) {
    if (!m_exp1->getScalarValue(v1)) return ExpressionPtr();
    try {
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
        case T_INSTANCEOF:
          result = false; break;
        default:
          return ExpressionPtr();
      }
      return makeScalarExpression(ar, result);
    } catch (...) {
    }
  } else {
    switch (m_op) {
      case T_LOGICAL_AND:
      case T_BOOLEAN_AND:
      case T_LOGICAL_OR:
      case T_BOOLEAN_OR: {
        bool useFirst = v2.toBoolean() == (m_op == T_LOGICAL_AND ||
                                           m_op == T_BOOLEAN_AND);
        ExpressionPtr rep = useFirst ? m_exp1 : m_exp2;
        rep = ExpressionPtr(
          new UnaryOpExpression(
            getScope(), getLocation(), KindOfUnaryOpExpression,
            rep, T_BOOL_CAST, true));
        rep->setActualType(Type::Boolean);
        if (!useFirst) {
          ExpressionListPtr l(
            new ExpressionList(
              getScope(), getLocation(), KindOfExpressionList,
              ExpressionList::ListKindComma));
          l->addElement(m_exp1);
          l->addElement(rep);
          l->setActualType(Type::Boolean);
          rep = l;
        }
        rep->setExpectedType(getExpectedType());
        return replaceValue(rep);
      }
      case T_LOGICAL_XOR:
      case '|':
      case '&':
      case '^':
      case '.':
      case '+':
      case '*':
        optExp = foldRightAssoc(ar);
        if (optExp) return optExp;
        break;
      case T_IS_IDENTICAL:
      case T_IS_NOT_IDENTICAL:
        if (v2.isNull()) {
          return makeIsNull(ar, getLocation(), m_exp1,
                            m_op == T_IS_NOT_IDENTICAL);
        }
        break;
      default:
        break;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr
BinaryOpExpression::foldRightAssoc(AnalysisResultConstPtr ar) {
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
        m_exp1 = aExp;
        m_exp2 = binOpExp = Clone(binOpExp);
        binOpExp->m_exp1 = bExp;
        binOpExp->m_exp2 = cExp;
        if (ExpressionPtr optExp = binOpExp->foldConst(ar)) {
          m_exp2 = optExp;
        }
        return static_pointer_cast<Expression>(shared_from_this());
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
      et1 = Type::PlusOperand;
      et2 = Type::PlusOperand;
      rt  = Type::PlusOperand;
    }
    break;
  case '-':
  case '*':
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case '/':
  case T_DIV_EQUAL:
    et1 = Type::Numeric;
    et2 = Type::Numeric;
    rt  = Type::Numeric;
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
    rt  = Type::Numeric;
    break;
  case T_MOD_EQUAL:
    et1 = Type::Numeric;
    et2 = Type::Int64;
    rt  = Type::Numeric;
    break;
  case '|':
  case '&':
  case '^':
  case T_AND_EQUAL:
  case T_OR_EQUAL:
  case T_XOR_EQUAL:
    et1 = Type::Primitive;
    et2 = Type::Primitive;
    rt  = Type::Primitive;
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
    et1 = Type::Some;
    et2 = Type::Some;
    rt = Type::Boolean;
    break;

  case T_INSTANCEOF:
    et1 = Type::Any;
    et2 = Type::String;
    rt = Type::Boolean;
    break;
  default:
    ASSERT(false);
  }

  switch (m_op) {
  case T_PLUS_EQUAL:
    {
      TypePtr rhs = m_exp2->inferAndCheck(ar, et2, coerce2);
      TypePtr lhs = m_exp1->inferAndCheck(ar, Type::Any, true);
      if (lhs) {
        if (lhs->mustBe(Type::KindOfArray)) {
          rt = Type::Array;
          break;
        }
        if (lhs->mustBe(Type::KindOfNumeric)) {
          if (!rhs->mustBe(lhs->getKindOf())) {
            rhs = Type::combinedArithmeticType(lhs, rhs);
            if (!rhs) rhs = Type::Numeric;
            lhs = m_exp1->inferAndCheck(ar, rhs, true);
          }
          rt = Type::Numeric;
          break;
        }
      }
      m_exp1->inferAndCheck(ar, rhs, true);
    }
    break;
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
      m_exp1->inferAndCheck(ar, Type::String, true);
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

      TypePtr combined = Type::combinedArithmeticType(act1, act2);
      if (combined && combined->isSubsetOf(rt)) {
        rt = combined;
      } else if (m_op == '+') {
        bool a1 = act1 && act1->is(Type::KindOfArray);
        bool a2 = act2 && act2->is(Type::KindOfArray);
        if (a1 || a2) {
          if (!a1 || !a2) {
            m_implementedType = Type::Variant;
          } else {
            m_exp1->setExpectedType(TypePtr());
            m_exp2->setExpectedType(TypePtr());
          }
          rt = Type::Array;
        }
      }
    }
    break;
  default:
    m_exp1->inferAndCheck(ar, et1, coerce1);
    m_exp2->inferAndCheck(ar, et2, coerce2);
    break;
  }

  m_exp1->fixExpectedType(ar);
  m_exp2->fixExpectedType(ar);

  return rt;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BinaryOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_exp1->outputPHP(cg, ar);

  switch (m_op) {
  case T_PLUS_EQUAL:          cg_printf(" += ");         break;
  case T_MINUS_EQUAL:         cg_printf(" -= ");         break;
  case T_MUL_EQUAL:           cg_printf(" *= ");         break;
  case T_DIV_EQUAL:           cg_printf(" /= ");         break;
  case T_CONCAT_EQUAL:        cg_printf(" .= ");         break;
  case T_MOD_EQUAL:           cg_printf(" %%= ");        break;
  case T_AND_EQUAL:           cg_printf(" &= ");         break;
  case T_OR_EQUAL:            cg_printf(" |= ");         break;
  case T_XOR_EQUAL:           cg_printf(" ^= ");         break;
  case T_SL_EQUAL:            cg_printf(" <<= ");        break;
  case T_SR_EQUAL:            cg_printf(" >>= ");        break;
  case T_BOOLEAN_OR:          cg_printf(" || ");         break;
  case T_BOOLEAN_AND:         cg_printf(" && ");         break;
  case T_LOGICAL_OR:          cg_printf(" or ");         break;
  case T_LOGICAL_AND:         cg_printf(" and ");        break;
  case T_LOGICAL_XOR:         cg_printf(" xor ");        break;
  case '|':                   cg_printf(" | ");          break;
  case '&':                   cg_printf(" & ");          break;
  case '^':                   cg_printf(" ^ ");          break;
  case '.':                   cg_printf(" . ");          break;
  case '+':                   cg_printf(" + ");          break;
  case '-':                   cg_printf(" - ");          break;
  case '*':                   cg_printf(" * ");          break;
  case '/':                   cg_printf(" / ");          break;
  case '%':                   cg_printf(" %% ");         break;
  case T_SL:                  cg_printf(" << ");         break;
  case T_SR:                  cg_printf(" >> ");         break;
  case T_IS_IDENTICAL:        cg_printf(" === ");        break;
  case T_IS_NOT_IDENTICAL:    cg_printf(" !== ");        break;
  case T_IS_EQUAL:            cg_printf(" == ");         break;
  case T_IS_NOT_EQUAL:        cg_printf(" != ");         break;
  case '<':                   cg_printf(" < ");          break;
  case T_IS_SMALLER_OR_EQUAL: cg_printf(" <= ");         break;
  case '>':                   cg_printf(" > ");          break;
  case T_IS_GREATER_OR_EQUAL: cg_printf(" >= ");         break;
  case T_INSTANCEOF:          cg_printf(" instanceof "); break;
  default:
    ASSERT(false);
  }

  m_exp2->outputPHP(cg, ar);
}

static bool castIfNeeded(TypePtr top, TypePtr arg,
                         CodeGenerator &cg, AnalysisResultPtr ar,
                         BlockScopeRawPtr scope) {
  if (top) {
    if (top->isPrimitive()) {
      if (!arg || !arg->isPrimitive()) {
        top->outputCPPCast(cg, ar, scope);
        cg_printf("(");
        return true;
      }
    } else if (top->is(Type::KindOfArray)) {
      if (arg && arg->isExactType() && !arg->is(Type::KindOfArray)) {
        cg_printf("((Variant)");
        return true;
      }
    } else if (top->mustBe(Type::KindOfNumeric)) {
      if (arg && arg->is(Type::KindOfArray)) {
        cg_printf("((Variant)");
        return true;
      }
    }
  }

  return false;
}

void BinaryOpExpression::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                        int state) {
  if (hasCPPTemp() || isScalar()) return;
  if (m_op == '.' && (state & FixOrder)) {
    if (m_exp1) m_exp1->preOutputStash(cg, ar, state|StashVars);
    if (m_exp2) m_exp2->preOutputStash(cg, ar, state|StashVars);
  } else {
    Expression::preOutputStash(cg, ar, state);
  }
}

static const char *stringBufferPrefix(CodeGenerator &cg, AnalysisResultPtr ar,
                                      ExpressionPtr var) {
  if (var->is(Expression::KindOfSimpleVariable)) {
    if (LoopStatementPtr loop = cg.getLoopStatement()) {
      SimpleVariablePtr sv = static_pointer_cast<SimpleVariable>(var);
      if (loop->checkStringBuf(sv->getName())) {
        return loop->getScope()->getVariables()->
          getVariablePrefix(sv->getName());
      }
    }
  }
  return 0;
}

static std::string stringBufferName(const char *temp, const char *prefix,
                                    const char *name)
{
  return std::string(temp) + "_sbuf_" + prefix + name;
}

int BinaryOpExpression::getConcatList(ExpressionPtrVec &ev, ExpressionPtr exp,
                                      bool &hasVoid) {
  if (!exp->hasCPPTemp()) {
    if (exp->is(Expression::KindOfUnaryOpExpression)) {
      UnaryOpExpressionPtr u = static_pointer_cast<UnaryOpExpression>(exp);
      if (u->getOp() == '(') {
        return getConcatList(ev, u->getExpression(), hasVoid);
      }
    } else if (exp->is(Expression::KindOfBinaryOpExpression)) {
      BinaryOpExpressionPtr b = static_pointer_cast<BinaryOpExpression>(exp);
      if (b->getOp() == '.') {
        return getConcatList(ev, b->getExp1(), hasVoid) +
          getConcatList(ev, b->getExp2(), hasVoid);
      }
    } else if (exp->is(Expression::KindOfEncapsListExpression)) {
      EncapsListExpressionPtr e =
        static_pointer_cast<EncapsListExpression>(exp);
      if (e->getType() != '`') {
        ExpressionListPtr el = e->getExpressions();
        int num = 0;
        for (int i = 0, s = el->getCount(); i < s; i++) {
          ExpressionPtr exp = (*el)[i];
          num += getConcatList(ev, exp, hasVoid);
        }
        return num;
      }
    }
  }

  ev.push_back(exp);
  bool isVoid = !exp->getActualType();
  hasVoid |= isVoid;
  return isVoid ? 0 : 1;
}

static void outputStringExpr(CodeGenerator &cg, AnalysisResultPtr ar,
                             ExpressionPtr exp, bool asLitStr) {
  if (asLitStr && exp->isLiteralString()) {
    const std::string &s = exp->getLiteralString();
    char *enc = string_cplus_escape(s.c_str(), s.size());
    cg_printf("\"%s\", %d", enc, s.size());
    free(enc);
    return;
  }

  bool close = false;
  if ((exp->hasContext(Expression::LValue) &&
       (!exp->getActualType()->is(Type::KindOfString) ||
        (exp->getImplementedType() &&
         !exp->getImplementedType()->is(Type::KindOfString))))
      ||
      !exp->getType()->is(Type::KindOfString)) {
    cg_printf("toString(");
    close = true;
  }
  exp->outputCPP(cg, ar);
  if (close) cg_printf(")");
}

static void outputStringBufExprs(ExpressionPtrVec &ev,
                                 CodeGenerator &cg, AnalysisResultPtr ar) {
  for (size_t i = 0; i < ev.size(); i++) {
    ExpressionPtr exp = ev[i];
    cg_printf(".add(");
    outputStringExpr(cg, ar, exp, true);
    cg_printf(")");
  }
}

bool BinaryOpExpression::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                      int state) {
  if (isOpEqual()) return Expression::preOutputCPP(cg, ar, state);
  bool effect2 = m_exp2->hasEffect();
  const char *prefix = 0;
  if (effect2 || m_exp1->hasEffect()) {
    ExpressionPtr self = static_pointer_cast<Expression>(shared_from_this());
    ExpressionPtrVec ev;
    bool hasVoid = false;
    int numConcat = 0;
    bool ok = false;
    if (m_op == '.') {
      numConcat = getConcatList(ev, self, hasVoid);
      ok = hasVoid ||
           (numConcat > MAX_CONCAT_ARGS &&
            (!Option::GenConcat ||
             cg.getOutput() == CodeGenerator::SystemCPP));
    } else if (effect2 && m_op == T_CONCAT_EQUAL) {
      prefix = stringBufferPrefix(cg, ar, m_exp1);
      ok = prefix;
      if (!ok) {
        if (m_exp1->is(KindOfSimpleVariable)) {
          ok = true;
          ev.push_back(m_exp1);
          numConcat++;
        }
      }
      numConcat += getConcatList(ev, m_exp2, hasVoid);
    }
    if (ok) {
      if (!cg.inExpression()) return true;

      cg.wrapExpressionBegin();
      std::string buf;
      if (prefix) {
        SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(m_exp1));
        buf = stringBufferName(Option::TempPrefix, prefix,
                               sv->getName().c_str());
        m_cppTemp = "/**/";
      } else if (numConcat) {
        buf = m_cppTemp = genCPPTemp(cg, ar);
        buf += "_buf";
        cg_printf("StringBuffer %s;\n", buf.c_str());
      } else {
        m_cppTemp = "\"\"";
      }

      for (size_t i = 0; i < ev.size(); i++) {
        ExpressionPtr exp = ev[i];
        bool is_void = !exp->getActualType();
        exp->preOutputCPP(cg, ar, 0);
        if (!is_void) {
          cg_printf("%s.append(", buf.c_str());
          outputStringExpr(cg, ar, exp, true);
          cg_printf(")");
        } else {
          exp->outputCPPUnneeded(cg, ar);
        }
        cg_printf(";\n");
      }

      if (numConcat && !prefix) {
        cg_printf("CStrRef %s(%s.detach());\n",
                  m_cppTemp.c_str(), buf.c_str());
        if (m_op == T_CONCAT_EQUAL) {
          m_exp1->outputCPP(cg, ar);
          cg_printf(" = %s;\n", m_cppTemp.c_str());
        }
      }
      return true;
    }
  }

  if (!isShortCircuitOperator()) {
    return Expression::preOutputCPP(cg, ar, state);
  }

  if (!effect2) {
    bool ret = m_exp1->preOutputCPP(cg, ar, 0);
    if (state & FixOrder) {
      ret = true;
      if (cg.inExpression()) {
        preOutputStash(cg, ar, state);
      }
    }
    return ret;
  }

  bool fix_e1 = m_exp1->preOutputCPP(cg, ar, 0);
  if (!cg.inExpression()) {
    return fix_e1 || m_exp2->preOutputCPP(cg, ar, 0);
  }

  cg.setInExpression(false);
  bool fix_e2 = m_exp2->preOutputCPP(cg, ar, 0);
  cg.setInExpression(true);

  if (fix_e2) {
    cg.wrapExpressionBegin();
    std::string tmp = genCPPTemp(cg, ar);
    cg_printf("bool %s = (", tmp.c_str());
    m_exp1->outputCPP(cg, ar);
    cg_printf(");\n");
    cg_indentBegin("if (%s%s) {\n",
                   m_op == T_LOGICAL_OR || m_op == T_BOOLEAN_OR ? "!" : "",
                   tmp.c_str());
    m_exp2->preOutputCPP(cg, ar, 0);
    cg_printf("%s = (", tmp.c_str());
    m_exp2->outputCPP(cg, ar);
    cg_printf(");\n");
    cg_indentEnd("}\n");
    m_cppTemp = tmp;
  } else if (state & FixOrder) {
    preOutputStash(cg, ar, state);
    fix_e1 = true;
  }
  return fix_e1 || fix_e2;
}

bool BinaryOpExpression::isOpEqual() {
  switch (m_op) {
  case T_CONCAT_EQUAL:
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
    return true;
  default:
    break;
  }
  return false;
}

bool BinaryOpExpression::outputCPPImplOpEqual(CodeGenerator &cg,
                                              AnalysisResultPtr ar) {
  if (m_exp1->is(Expression::KindOfArrayElementExpression)) {
    ArrayElementExpressionPtr exp =
      dynamic_pointer_cast<ArrayElementExpression>(m_exp1);
    if (exp->isSuperGlobal() || exp->isDynamicGlobal()) return false;
    if (TypePtr t = exp->getVariable()->getActualType()) {
      if (t->is(Type::KindOfArray) &&
          (!exp->getVariable()->getImplementedType() ||
           exp->getVariable()->getImplementedType()->is(Type::KindOfArray))) {
        return false;
      }
    }
    // turning $a['elem'] Op= $b into $a.setOpEqual('elem', $b);
    exp->getVariable()->outputCPP(cg, ar);
    if (exp->getOffset()) {
      cg_printf(".setOpEqual(%d, ", m_op);
      exp->getOffset()->outputCPP(cg, ar);
      cg_printf(", (");
    } else {
      cg_printf(".appendOpEqual(%d, (", m_op);
    }
    m_exp2->outputCPP(cg, ar);
    cg_printf(")");
    ExpressionPtr off = exp->getOffset();
    if (off) {
      ScalarExpressionPtr sc = dynamic_pointer_cast<ScalarExpression>(off);
      if (sc) {
        if (sc->isLiteralString()) {
          String s(sc->getLiteralString());
          int64 n;
          if (!s.get()->isStrictlyInteger(n)) {
            cg_printf(", true"); // skip toKey() at run time
          }
        }
      }
    }
    cg_printf(")");

    return true;
  }
  if (m_exp1->is(Expression::KindOfObjectPropertyExpression)) {
    ObjectPropertyExpressionPtr var(
      dynamic_pointer_cast<ObjectPropertyExpression>(m_exp1));
    if (var->isValid()) return false;
    var->outputCPPObject(cg, ar);
    cg_printf("o_assign_op<%s,%d>(",
              isUnused() ? "void" : "Variant", m_op);
    var->outputCPPProperty(cg, ar);
    cg_printf(", ");
    m_exp2->outputCPP(cg, ar);
    cg_printf("%s)", originalClassName(cg, true).c_str());
    return true;
  }

  return false;
}

void BinaryOpExpression::outputCPPImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {

  if (isOpEqual() && outputCPPImplOpEqual(cg, ar)) return;

  bool wrapped = true;
  switch (m_op) {
  case T_CONCAT_EQUAL:
    if (const char *prefix = stringBufferPrefix(cg, ar, m_exp1)) {
      SimpleVariablePtr sv = static_pointer_cast<SimpleVariable>(m_exp1);
      ExpressionPtrVec ev;
      bool hasVoid = false;
      getConcatList(ev, m_exp2, hasVoid);
      cg_printf("%s", stringBufferName(Option::TempPrefix, prefix,
                                       sv->getName().c_str()).c_str());
      outputStringBufExprs(ev, cg, ar);
      return;
    }
    cg_printf("concat_assign");
    break;
  case '.':
    {
      ExpressionPtr self = static_pointer_cast<Expression>(shared_from_this());
      ExpressionPtrVec ev;
      bool hasVoid = false;
      int num = getConcatList(ev, self, hasVoid);
      assert(!hasVoid);
      if ((num <= MAX_CONCAT_ARGS ||
           (Option::GenConcat &&
            cg.getOutput() != CodeGenerator::SystemCPP))) {
        assert(num >= 2);
        if (num == 2) {
          cg_printf("concat(");
        } else {
          if (num > MAX_CONCAT_ARGS) ar->m_concatLengths.insert(num);
          cg_printf("concat%d(", num);
        }
        for (size_t i = 0; i < ev.size(); i++) {
          ExpressionPtr exp = ev[i];
          if (i) cg_printf(", ");
          outputStringExpr(cg, ar, exp, false);
        }
        cg_printf(")");
      } else {
        cg_printf("StringBuffer()");
        outputStringBufExprs(ev, cg, ar);
        cg_printf(".detach()");
      }
    }
    return;
  case T_LOGICAL_XOR:         cg_printf("logical_xor");   break;
  case '|':                   cg_printf("bitwise_or");    break;
  case '&':                   cg_printf("bitwise_and");   break;
  case '^':                   cg_printf("bitwise_xor");   break;
  case T_IS_IDENTICAL:        cg_printf("same");          break;
  case T_IS_NOT_IDENTICAL:    cg_printf("!same");         break;
  case T_IS_EQUAL:            cg_printf("equal");         break;
  case T_IS_NOT_EQUAL:        cg_printf("!equal");        break;
  case '<':                   cg_printf("less");          break;
  case T_IS_SMALLER_OR_EQUAL: cg_printf("not_more");      break;
  case '>':                   cg_printf("more");          break;
  case T_IS_GREATER_OR_EQUAL: cg_printf("not_less");      break;
  case '/':                   cg_printf("divide");        break;
  case '%':                   cg_printf("modulo");        break;
  case T_INSTANCEOF:          cg_printf("instanceOf");    break;
  default:
    wrapped = !isUnused();
    break;
  }

  if (wrapped) cg_printf("(");

  ExpressionPtr first = m_exp1;
  ExpressionPtr second = m_exp2;

  // we could implement these functions natively on String and Array classes
  switch (m_op) {
  case '+':
  case '-':
  case '*':
  case '/':
    if (!first->outputCPPArithArg(cg, ar, m_op == '+')) {
      TypePtr argType = first->hasCPPTemp() ?
        first->getType() : first->getActualType();
      bool flag = castIfNeeded(getActualType(), argType, cg, ar, getScope());
      first->outputCPP(cg, ar);
      if (flag) {
        cg_printf(")");
      }
    }
    break;
  case T_SL:
  case T_SR:
    cg_printf("toInt64(");
    first->outputCPP(cg, ar);
    cg_printf(")");
    break;
  default:
    first->outputCPP(cg, ar);
    break;
  }

  switch (m_op) {
  case T_PLUS_EQUAL:          cg_printf(" += ");   break;
  case T_MINUS_EQUAL:         cg_printf(" -= ");   break;
  case T_MUL_EQUAL:           cg_printf(" *= ");   break;
  case T_DIV_EQUAL:           cg_printf(" /= ");   break;
  case T_MOD_EQUAL:           cg_printf(" %%= ");  break;
  case T_AND_EQUAL:           cg_printf(" &= ");   break;
  case T_OR_EQUAL:            cg_printf(" |= ");   break;
  case T_XOR_EQUAL:           cg_printf(" ^= ");   break;
  case T_SL_EQUAL:            cg_printf(" <<= ");  break;
  case T_SR_EQUAL:            cg_printf(" >>= ");  break;
  case T_BOOLEAN_OR:          cg_printf(" || ");   break;
  case T_BOOLEAN_AND:         cg_printf(" && ");   break;
  case T_LOGICAL_OR:          cg_printf(" || ");   break;
  case T_LOGICAL_AND:         cg_printf(" && ");   break;
  default:
    switch (m_op) {
    case '+':                   cg_printf(" + ");    break;
    case '-':                   cg_printf(" - ");    break;
    case '*':                   cg_printf(" * ");    break;
    case T_SL:                  cg_printf(" << ");   break;
    case T_SR:                  cg_printf(" >> ");   break;
    default:
      cg_printf(", ");
      break;
    }
    break;
  }

  switch (m_op) {
  case '+':
  case '-':
  case '*':
  case '/':
    if (!second->outputCPPArithArg(cg, ar, m_op == '+')) {
      TypePtr argType = second->hasCPPTemp() ?
        second->getType() : second->getActualType();
      bool flag = castIfNeeded(getActualType(), argType, cg, ar, getScope());
      second->outputCPP(cg, ar);
      if (flag) {
        cg_printf(")");
      }
    }
    break;
  case T_INSTANCEOF:
    {
      if (second->isScalar()) {
        ScalarExpressionPtr scalar =
          dynamic_pointer_cast<ScalarExpression>(second);
        bool notQuoted = scalar && !scalar->isQuoted();
        std::string s = second->getLiteralString();
        if (s == "static" && notQuoted) {
          cg_printf("FrameInjection::GetStaticClassName(fi.getThreadInfo())");
        } else if (s != "") {
          if (s == "self" && notQuoted) {
            ClassScopeRawPtr cls = getOriginalClass();
            if (cls) {
              s = cls->getOriginalName();
            }
          } else if (s == "parent" && notQuoted) {
            ClassScopeRawPtr cls = getOriginalClass();
            if (cls && !cls->getParent().empty()) {
              s = cls->getParent();
            }
          }
          cg_printString(s, ar, shared_from_this());
        } else {
          second->outputCPP(cg, ar);
        }
      } else {
        second->outputCPP(cg, ar);
      }
      break;
    }
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
    {
      TypePtr t1 = first->getCPPType();
      TypePtr t2 = second->getType();
      if (t1 && !t1->is(Type::KindOfArray) &&
          t2 && Type::IsCastNeeded(ar, t2, t1)) {
        t1->outputCPPCast(cg, ar, getScope());
        cg_printf("(");
        second->outputCPP(cg, ar);
        cg_printf(")");
      } else {
        second->outputCPP(cg, ar);
      }
      break;
    }
  case T_BOOLEAN_OR:
  case T_BOOLEAN_AND:
  case T_LOGICAL_AND:
  case T_LOGICAL_OR:
    if (isUnused()) {
      cg_printf("(");
      if (second->outputCPPUnneeded(cg, ar)) {
        cg_printf(",");
      }
      cg_printf("false)");
    } else {
      second->outputCPP(cg, ar);
    }
    break;
  default:
    second->outputCPP(cg, ar);
  }

  if (wrapped) cg_printf(")");
}
