/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/vm/runtime.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BinaryOpExpression::BinaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr exp1, ExpressionPtr exp2, int op)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(BinaryOpExpression)),
    m_exp1(exp1), m_exp2(exp2), m_op(op), m_assign(false), m_canThrow(false) {
  switch (m_op) {
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case T_DIV_EQUAL:
  case T_POW_EQUAL:
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
  case T_COLLECTION: {
    std::string s = m_exp1->getLiteralString();
    ExpressionListPtr el = static_pointer_cast<ExpressionList>(m_exp2);
    el->setCollectionElems();
    break;
  }
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

bool BinaryOpExpression::containsDynamicConstant(AnalysisResultPtr ar) const {
  switch (m_op) {
  case T_COLLECTION:
    return m_exp2->containsDynamicConstant(ar);
  default:
    break;
  }
  return false;
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

bool BinaryOpExpression::isLogicalOrOperator() const {
  switch (m_op) {
  case T_BOOLEAN_OR:
  case T_LOGICAL_OR:
    return true;
  default:
    break;
  }
  return false;
}

ExpressionPtr BinaryOpExpression::unneededHelper() {
  bool shortCircuit = isShortCircuitOperator();
  if (!m_exp2->getContainedEffects() ||
      (!shortCircuit && !m_exp1->getContainedEffects())) {
    return Expression::unneededHelper();
  }

  if (shortCircuit) {
    m_exp2 = m_exp2->unneeded();
  }
  return static_pointer_cast<Expression>(shared_from_this());
}

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
    if (!m_exp2->getScalarValue(v2) || equal(v2, 0)) {
      effect = CanThrow;
      m_canThrow = true;
    }
    break;
  }
  case '.': {
    // Conservatively assume that a concat with any non-scalar argument can
    // invoke arbitrary side-effects. This is because it may involve invoking an
    // object's __toString() method, which can have side-effects.
    if (!m_exp1->isScalar() || !m_exp2->isScalar()) {
      effect = UnknownEffect;
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
  m_exp1->analyzeProgram(ar);
  m_exp2->analyzeProgram(ar);
}

ConstructPtr BinaryOpExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp1;
    case 1:
      return m_exp2;
    default:
      assert(false);
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
    m_exp1 = dynamic_pointer_cast<Expression>(cp);
    break;
  case 1:
    m_exp2 = dynamic_pointer_cast<Expression>(cp);
    break;
  default:
    assert(false);
    break;
  }
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
  if (optExp) optExp = replaceValue(optExp);
  return optExp;
}

static ExpressionPtr makeIsNull(AnalysisResultConstPtr ar,
                                const Location::Range& r, ExpressionPtr exp,
                                bool invert) {
  /* Replace "$x === null" with an is_null call; this requires slightly
   * less work at runtime. */
  auto expList = std::make_shared<ExpressionList>(exp->getScope(), r);
  expList->insertElement(exp);

  auto call =
    std::make_shared<SimpleFunctionCall>(
      exp->getScope(), r, "is_null", false, expList, ExpressionPtr());

  call->setValid();
  call->setupScopes(ar);

  if (!invert) return call;

  return std::make_shared<UnaryOpExpression>(
    exp->getScope(), r, call, '!', true);
}

// foldConst() is callable from the parse phase as well as the analysis phase.
// We take advantage of this during the parse phase to reduce very simple
// expressions down to a single scalar and keep the parse tree smaller,
// especially in cases of long chains of binary operators. However, we limit
// the effectivness of this during parse to ensure that we eliminate only
// very simple scalars that don't require analysis in later phases. For now,
// that's just simply scalar values.
ExpressionPtr BinaryOpExpression::foldConst(AnalysisResultConstPtr ar) {
  ExpressionPtr optExp;
  Variant v1;
  Variant v2;

  if (!m_exp2->getScalarValue(v2)) {
    if ((ar->getPhase() != AnalysisResult::ParseAllFiles) &&
        m_exp1->isScalar() && m_exp1->getScalarValue(v1)) {
      switch (m_op) {
        case T_IS_IDENTICAL:
        case T_IS_NOT_IDENTICAL:
          if (v1.isNull()) {
            return makeIsNull(ar, getRange(), m_exp2,
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
                getScope(), getRange(),
                rep, T_BOOL_CAST, true));
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
              if (aExp->isArray() || bExp->isArray() || cExp->isArray()) {
                break;
              }
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
      ScalarExpressionPtr scalar1 =
        dynamic_pointer_cast<ScalarExpression>(m_exp1);
      ScalarExpressionPtr scalar2 =
        dynamic_pointer_cast<ScalarExpression>(m_exp2);
      // Some data, like the values of __CLASS__ and friends, are not available
      // while we're still in the initial parse phase.
      if (ar->getPhase() == AnalysisResult::ParseAllFiles) {
        if ((scalar1 && scalar1->needsTranslation()) ||
            (scalar2 && scalar2->needsTranslation())) {
          return ExpressionPtr();
        }
      }
      if ((!Option::WholeProgram || !Option::ParseTimeOpts) && getScope()) {
        // In the VM, don't optimize __CLASS__ if within a trait, since
        // __CLASS__ is not resolved yet.
        auto cs = getClassScope();
        if (cs && cs->isTrait()) {
          if ((scalar1 && scalar1->getType() == T_CLASS_C) ||
              (scalar2 && scalar2->getType() == T_CLASS_C)) {
            return ExpressionPtr();
          }
        }
      }
      Variant result;
      auto add = RuntimeOption::IntsOverflowToInts ? cellAdd : cellAddO;
      auto sub = RuntimeOption::IntsOverflowToInts ? cellSub : cellSubO;
      auto mul = RuntimeOption::IntsOverflowToInts ? cellMul : cellMulO;

      switch (m_op) {
        case T_LOGICAL_XOR:
          result = static_cast<bool>(v1.toBoolean() ^ v2.toBoolean());
          break;
        case '|':
          *result.asCell() = cellBitOr(*v1.asCell(), *v2.asCell());
          break;
        case '&':
          *result.asCell() = cellBitAnd(*v1.asCell(), *v2.asCell());
          break;
        case '^':
          *result.asCell() = cellBitXor(*v1.asCell(), *v2.asCell());
          break;
        case '.':
          if (v1.isArray() || v2.isArray()) {
            return ExpressionPtr();
          }
          result = concat(v1.toString(), v2.toString());
          break;
        case T_IS_IDENTICAL:
          result = same(v1, v2);
          break;
        case T_IS_NOT_IDENTICAL:
          result = !same(v1, v2);
          break;
        case T_IS_EQUAL:
          result = equal(v1, v2);
          break;
        case T_IS_NOT_EQUAL:
          result = !equal(v1, v2);
          break;
        case '<':
          result = less(v1, v2);
          break;
        case T_IS_SMALLER_OR_EQUAL:
          result = cellLessOrEqual(*v1.asCell(), *v2.asCell());
          break;
        case '>':
          result = more(v1, v2);
          break;
        case T_IS_GREATER_OR_EQUAL:
          result = cellGreaterOrEqual(*v1.asCell(), *v2.asCell());
          break;
        case '+':
          *result.asCell() = add(*v1.asCell(), *v2.asCell());
          break;
        case '-':
          *result.asCell() = sub(*v1.asCell(), *v2.asCell());
          break;
        case '*':
          *result.asCell() = mul(*v1.asCell(), *v2.asCell());
          break;
        case '/':
          if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
            return ExpressionPtr();
          }
          *result.asCell() = cellDiv(*v1.asCell(), *v2.asCell());
          break;
        case '%':
          if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
            return ExpressionPtr();
          }
          *result.asCell() = cellMod(*v1.asCell(), *v2.asCell());
          break;
        case T_SL:
          result = v1.toInt64() << v2.toInt64();
          break;
        case T_SR:
          result = v1.toInt64() >> v2.toInt64();
          break;
        case T_BOOLEAN_OR:
          result = v1.toBoolean() || v2.toBoolean(); break;
        case T_BOOLEAN_AND:
          result = v1.toBoolean() && v2.toBoolean(); break;
        case T_LOGICAL_OR:
          result = v1.toBoolean() || v2.toBoolean(); break;
        case T_LOGICAL_AND:
          result = v1.toBoolean() && v2.toBoolean(); break;
        case T_INSTANCEOF: {
          if (v2.isString()) {
            if (v1.isArray() &&
                interface_supports_array(v2.getStringData())) {
              result = true;
              break;
            }
            if (v1.isString() &&
                interface_supports_string(v2.getStringData())) {
              result = true;
              break;
            }
            if (v1.isInteger() &&
                interface_supports_int(v2.getStringData())) {
              result = true;
              break;
            }
            if (v1.isDouble() &&
                interface_supports_double(v2.getStringData())) {
              result = true;
              break;
            }
          }
          result = false;
          break;
        }
        default:
          return ExpressionPtr();
      }
      return makeScalarExpression(ar, result);
    } catch (...) {
    }
  } else if (ar->getPhase() != AnalysisResult::ParseAllFiles) {
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
            getScope(), getRange(),
            rep, T_BOOL_CAST, true));
        if (!useFirst) {
          ExpressionListPtr l(
            new ExpressionList(
              getScope(), getRange(),
              ExpressionList::ListKindComma));
          l->addElement(m_exp1);
          l->addElement(rep);
          rep = l;
        }
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
          return makeIsNull(ar, getRange(), m_exp1,
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

///////////////////////////////////////////////////////////////////////////////

void BinaryOpExpression::outputCodeModel(CodeGenerator &cg) {
  if (m_op == T_COLLECTION) {
    cg.printObjectHeader("CollectionInitializerExpression", 3);
    cg.printPropertyHeader("class");
    cg.printTypeExpression(m_exp1);
    cg.printPropertyHeader("arguments");
    cg.printExpressionVector(static_pointer_cast<ExpressionList>(m_exp2));
    cg.printPropertyHeader("sourceLocation");
    cg.printLocation(this);
    cg.printObjectFooter();
    return;
  }

  cg.printObjectHeader("BinaryOpExpression", 4);
  cg.printPropertyHeader("expression1");
  m_exp1->outputCodeModel(cg);
  cg.printPropertyHeader("expression2");
  if (m_op == T_INSTANCEOF) {
    cg.printTypeExpression(m_exp2);
  } else {
    m_exp2->outputCodeModel(cg);
  }
  cg.printPropertyHeader("operation");

  int op = 0;
  switch (m_op) {
    case T_PLUS_EQUAL: op = PHP_PLUS_ASSIGN; break;
    case T_MINUS_EQUAL: op = PHP_MINUS_ASSIGN; break;
    case T_MUL_EQUAL: op = PHP_MULTIPLY_ASSIGN; break;
    case T_DIV_EQUAL: op = PHP_DIVIDE_ASSIGN; break;
    case T_CONCAT_EQUAL: op = PHP_CONCAT_ASSIGN; break;
    case T_MOD_EQUAL:  op = PHP_MODULUS_ASSIGN;  break;
    case T_AND_EQUAL: op = PHP_AND_ASSIGN; break;
    case T_OR_EQUAL: op = PHP_OR_ASSIGN;  break;
    case T_XOR_EQUAL: op = PHP_XOR_ASSIGN; break;
    case T_SL_EQUAL: op = PHP_SHIFT_LEFT_ASSIGN; break;
    case T_SR_EQUAL: op = PHP_SHIFT_RIGHT_ASSIGN; break;
    case T_BOOLEAN_OR: op = PHP_BOOLEAN_OR;  break;
    case T_BOOLEAN_AND: op = PHP_BOOLEAN_AND; break;
    case T_LOGICAL_OR: op = PHP_LOGICAL_OR; break;
    case T_LOGICAL_AND: op = PHP_LOGICAL_AND;  break;
    case T_LOGICAL_XOR: op = PHP_LOGICAL_XOR; break;
    case '|': op = PHP_OR; break;
    case '&': op = PHP_AND;  break;
    case '^': op = PHP_XOR; break;
    case '.': op = PHP_CONCAT; break;
    case '+': op = PHP_PLUS; break;
    case '-': op = PHP_MINUS; break;
    case '*': op = PHP_MULTIPLY; break;
    case '/': op = PHP_DIVIDE; break;
    case '%': op = PHP_MODULUS; break;
    case T_SL: op = PHP_SHIFT_LEFT; break;
    case T_SR: op = PHP_SHIFT_RIGHT; break;
    case T_IS_IDENTICAL: op = PHP_IS_IDENTICAL; break;
    case T_IS_NOT_IDENTICAL: op = PHP_IS_NOT_IDENTICAL; break;
    case T_IS_EQUAL: op = PHP_IS_EQUAL; break;
    case T_IS_NOT_EQUAL: op = PHP_IS_NOT_EQUAL; break;
    case '<': op = PHP_IS_SMALLER; break;
    case T_IS_SMALLER_OR_EQUAL: op = PHP_IS_SMALLER_OR_EQUAL; break;
    case '>': op = PHP_IS_GREATER; break;
    case T_IS_GREATER_OR_EQUAL: op = PHP_IS_GREATER_OR_EQUAL;  break;
    case T_INSTANCEOF: op = PHP_INSTANCEOF;  break;
    default:
      assert(false);
  }

  cg.printValue(op);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
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
  case T_POW_EQUAL:           cg_printf(" **= ");        break;
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
  case T_POW:                 cg_printf(" ** ");         break;
  case T_IS_IDENTICAL:        cg_printf(" === ");        break;
  case T_IS_NOT_IDENTICAL:    cg_printf(" !== ");        break;
  case T_IS_EQUAL:            cg_printf(" == ");         break;
  case T_IS_NOT_EQUAL:        cg_printf(" != ");         break;
  case '<':                   cg_printf(" < ");          break;
  case T_IS_SMALLER_OR_EQUAL: cg_printf(" <= ");         break;
  case '>':                   cg_printf(" > ");          break;
  case T_IS_GREATER_OR_EQUAL: cg_printf(" >= ");         break;
  case T_INSTANCEOF:          cg_printf(" instanceof "); break;
  case T_COLLECTION: {
    ExpressionListPtr el = static_pointer_cast<ExpressionList>(m_exp2);
    if (el->getCount() == 0) {
      cg_printf(" {}");
    } else {
      cg_printf(" { ");
      el->outputPHP(cg, ar);
      cg_printf(" }");
    }
    return;
  }
  default:
    assert(false);
  }

  m_exp2->outputPHP(cg, ar);
}
