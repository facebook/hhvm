/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/compiler/analysis/class_scope.h"

#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/unary_op_expression.h"

#include "hphp/compiler/statement/loop_statement.h"

#include "hphp/parser/hphp.tab.hpp"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/zend-string.h"

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
    break;
  case T_COLLECTION: {
    std::string s = m_exp1->getLiteralString();
    static_pointer_cast<ExpressionList>(m_exp2)->setCollectionElems();
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

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

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

ExpressionPtr BinaryOpExpression::preOptimize(AnalysisResultConstRawPtr ar) {
  if (!m_exp2->isScalar()) {
    if (!m_exp1->isScalar()) {
      if (m_exp1->is(KindOfBinaryOpExpression)) {
        auto b = dynamic_pointer_cast<BinaryOpExpression>(m_exp1);
        if (b->m_op == m_op && b->m_exp1->isScalar()) {
          return foldRightAssoc(ar);
        }
      }
      return ExpressionPtr();
    }
  }
  ExpressionPtr optExp;
  try {
    optExp = foldConst(ar);
  } catch (Exception& e) {
    // runtime/base threw an exception, perhaps bad operands
  }
  if (optExp) optExp = replaceValue(optExp);
  return optExp;
}

static ExpressionPtr makeIsNull(AnalysisResultConstRawPtr ar,
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
ExpressionPtr BinaryOpExpression::foldConst(AnalysisResultConstRawPtr ar) {
  ExpressionPtr optExp;
  Variant v1;
  Variant v2;

  if (RuntimeOption::EvalDisableHphpcOpts) return ExpressionPtr();

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
            auto binOpExp = dynamic_pointer_cast<BinaryOpExpression>(m_exp2);
            if (binOpExp->m_op == m_op && binOpExp->m_exp1->isScalar()) {
              auto aExp = m_exp1;
              auto bExp = binOpExp->m_exp1;
              auto cExp = binOpExp->m_exp2;
              if (aExp->isArray() || bExp->isArray() || cExp->isArray()) {
                break;
              }
              m_exp1 = binOpExp = Clone(binOpExp);
              m_exp2 = cExp;
              binOpExp->m_exp1 = aExp;
              binOpExp->m_exp2 = bExp;
              if (auto optExp = binOpExp->foldConst(ar)) {
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
      ThrowAllErrorsSetter taes;
      auto scalar1 = dynamic_pointer_cast<ScalarExpression>(m_exp1);
      auto scalar2 = dynamic_pointer_cast<ScalarExpression>(m_exp2);
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
          *result.toCell() = cellBitOr(*v1.toCell(), *v2.toCell());
          break;
        case '&':
          *result.toCell() = cellBitAnd(*v1.toCell(), *v2.toCell());
          break;
        case '^':
          *result.toCell() = cellBitXor(*v1.toCell(), *v2.toCell());
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
          result = cellLessOrEqual(*v1.toCell(), *v2.toCell());
          break;
        case '>':
          result = more(v1, v2);
          break;
        case T_IS_GREATER_OR_EQUAL:
          result = cellGreaterOrEqual(*v1.toCell(), *v2.toCell());
          break;
        case T_SPACESHIP:
          result = cellCompare(*v1.toCell(), *v2.toCell());
          break;
        case '+':
          *result.toCell() = add(*v1.toCell(), *v2.toCell());
          break;
        case '-':
          *result.toCell() = sub(*v1.toCell(), *v2.toCell());
          break;
        case '*':
          *result.toCell() = mul(*v1.toCell(), *v2.toCell());
          break;
        case '/':
          if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
            return ExpressionPtr();
          }
          *result.toCell() = cellDiv(*v1.toCell(), *v2.toCell());
          break;
        case '%':
          if ((v2.isIntVal() && v2.toInt64() == 0) || v2.toDouble() == 0.0) {
            return ExpressionPtr();
          }
          *result.toCell() = cellMod(*v1.toCell(), *v2.toCell());
          break;
        case T_SL: {
          int64_t shift = v2.toInt64();
          if (!RuntimeOption::PHP7_IntSemantics) {
            result = static_cast<int64_t>(
              static_cast<uint64_t>(v1.toInt64()) << (shift & 63));
          } else if (shift >= 64) {
            result = 0;
          } else if (shift < 0) {
            // This raises an error, and so can't be folded.
            return ExpressionPtr();
          } else {
            result = static_cast<int64_t>(
              static_cast<uint64_t>(v1.toInt64()) << (shift & 63));
          }
          break;
        }
        case T_SR: {
          int64_t shift = v2.toInt64();
          if (!RuntimeOption::PHP7_IntSemantics) {
            result = v1.toInt64() >> (shift & 63);
          } else if (shift >= 64) {
            result = v1.toInt64() >= 0 ? 0 : -1;
          } else if (shift < 0) {
            // This raises an error, and so can't be folded.
            return ExpressionPtr();
          } else {
            result = v1.toInt64() >> (shift & 63);
          }
          break;
        }
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
            if (v1.isVecArray() &&
                interface_supports_vec(v2.getStringData())) {
              result = true;
              break;
            }
            if (v1.isDict() &&
                interface_supports_dict(v2.getStringData())) {
              result = true;
              break;
            }
            if (v1.isKeyset() &&
                interface_supports_keyset(v2.getStringData())) {
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
BinaryOpExpression::foldRightAssoc(AnalysisResultConstRawPtr ar) {
  ExpressionPtr optExp1;

  if (RuntimeOption::EvalDisableHphpcOpts) return ExpressionPtr();

  switch (m_op) {
  case '.':
  case '+':
  case '*':
    if (m_exp1->is(Expression::KindOfBinaryOpExpression)) {
      auto binOpExp = dynamic_pointer_cast<BinaryOpExpression>(m_exp1);
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
  case T_SPACESHIP:           cg_printf(" <=> ");        break;
  case T_INSTANCEOF:          cg_printf(" instanceof "); break;
  case T_PIPE:                cg_printf(" |> ");         break;
  case T_COLLECTION: {
    auto el = static_pointer_cast<ExpressionList>(m_exp2);
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
