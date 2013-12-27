/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/compiler/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

inline void UnaryOpExpression::ctorInit() {
  switch (m_op) {
  case T_INC:
  case T_DEC:
    m_localEffects = AssignEffect;
    m_exp->setContext(Expression::OprLValue);
    m_exp->setContext(Expression::DeepOprLValue);
    // this is hacky, what we need is LValueWrapper
    if (!m_exp->is(Expression::KindOfSimpleVariable)) {
      m_exp->setContext(Expression::LValue);
      m_exp->clearContext(Expression::NoLValueWrapper);
    }
    break;
  case T_PRINT:
    m_localEffects = IOEffect;
    break;
  case T_EXIT:
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:
  case T_EVAL:
  case T_CLONE:
    m_localEffects = UnknownEffect;
    break;
  case T_ISSET:
  case T_EMPTY:
    setExistContext();
    break;
  case T_UNSET:
    m_localEffects = AssignEffect;
    m_exp->setContext(UnsetContext);
    break;
  case T_CLASS:
  case T_FUNCTION:
    m_localEffects = CreateEffect;
    break;
  case T_ARRAY:
  default:
    break;
  }
}

void UnaryOpExpression::setDefinedScope(BlockScopeRawPtr scope) {
  always_assert(m_op == T_CLASS || m_op == T_FUNCTION);
  m_definedScope = scope;
}

UnaryOpExpression::UnaryOpExpression
(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS, ExpressionPtr exp, int op, bool front)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_exp(exp), m_op(op), m_front(front) {
  ctorInit();
}

UnaryOpExpression::UnaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op, bool front)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(UnaryOpExpression)),
    m_exp(exp), m_op(op), m_front(front) {
  ctorInit();
}

ExpressionPtr UnaryOpExpression::clone() {
  UnaryOpExpressionPtr exp(new UnaryOpExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exp = Clone(m_exp);
  return exp;
}

bool UnaryOpExpression::isTemporary() const {
  switch (m_op) {
  case '!':
  case '+':
  case '-':
  case '~':
  case T_ARRAY:
    return true;
  }
  return false;
}

bool UnaryOpExpression::isScalar() const {
  switch (m_op) {
  case '!':
  case '+':
  case '-':
  case '~':
  case '@':
    return m_exp->isScalar();
  case T_ARRAY:
    return (!m_exp || m_exp->isScalar());
  default:
    break;
  }
  return false;
}

bool UnaryOpExpression::isCast() const {
  switch (m_op) {
  case T_INT_CAST:
  case T_DOUBLE_CAST:
  case T_STRING_CAST:
  case T_ARRAY_CAST:
  case T_OBJECT_CAST:
  case T_BOOL_CAST:
    return true;
  default: break;
  }
  return false;
}

TypePtr UnaryOpExpression::getCastType() const {
  switch (m_op) {
  case T_INT_CAST:    return Type::Int64;
  case T_DOUBLE_CAST: return Type::Double;
  case T_STRING_CAST: return Type::String;
  case T_ARRAY_CAST:  return Type::Array;
  case T_OBJECT_CAST: return Type::Object;
  case T_BOOL_CAST:   return Type::Boolean;
  default: break;
  }
  return TypePtr();
}

bool UnaryOpExpression::isRefable(bool checkError /*= false */) const {
  if (m_op == T_INC || m_op == T_DEC) {
    return m_exp->isRefable(checkError);
  }
  return false;
}

bool UnaryOpExpression::isThis() const {
  return false;
}

bool UnaryOpExpression::containsDynamicConstant(AnalysisResultPtr ar) const {
  switch (m_op) {
  case '+':
  case '-':
  case T_ARRAY:
    return m_exp && m_exp->containsDynamicConstant(ar);
  default:
    break;
  }
  return false;
}

bool UnaryOpExpression::getScalarValue(Variant &value) {
  if (m_exp) {
    if (m_op == T_ARRAY) {
      return m_exp->getScalarValue(value);
    }
    Variant t;
    return m_exp->getScalarValue(t) &&
      preCompute(t, value);
  }
  if (m_op != T_ARRAY) return false;
  value = Array::Create();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void UnaryOpExpression::onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
  if (m_op == T_EVAL) {
    ConstructPtr self = shared_from_this();
    Compiler::Error(Compiler::UseEvaluation, self);
    scope->setAttribute(FileScope::ContainsLDynamicVariable);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void UnaryOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_exp) m_exp->analyzeProgram(ar);
  if ((m_op == T_CLASS || m_op == T_FUNCTION) &&
      ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    ar->link(getFileScope(), m_definedScope->getContainingFile());
  }
}

bool UnaryOpExpression::preCompute(CVarRef value, Variant &result) {
  bool ret = true;
  try {
    g_context->setThrowAllErrors(true);
    switch(m_op) {
      case '!':
        result = (!toBoolean(value)); break;
      case '+':
        cellSet(cellAdd(make_tv<KindOfInt64>(0), *value.asCell()),
                *result.asCell());
        break;
      case '-':
        cellSet(cellSub(make_tv<KindOfInt64>(0), *value.asCell()),
                *result.asCell());
        break;
      case '~':
        tvSet(*value.asCell(), *result.asTypedValue());
        cellBitNot(*result.asCell());
        break;
      case '@':
        result = value;
        break;
      case T_INT_CAST:
        result = value.toInt64();
        break;
      case T_DOUBLE_CAST:
        result = toDouble(value);
        break;
      case T_STRING_CAST:
        result = toString(value);
        break;
      case T_BOOL_CAST:
        result = toBoolean(value);
        break;
      case T_EMPTY:
        result = !toBoolean(value);
        break;
      case T_ISSET:
        result = is_not_null(value);
        break;
      case T_INC:
      case T_DEC:
        assert(false);
      default:
        ret = false;
        break;
    }
  } catch (...) {
    ret = false;
  }
  g_context->setThrowAllErrors(false);
  return ret;
}

ConstructPtr UnaryOpExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int UnaryOpExpression::getKidCount() const {
  return 1;
}

void UnaryOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

bool UnaryOpExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  UnaryOpExpressionPtr u =
    static_pointer_cast<UnaryOpExpression>(e);

  return m_op == u->m_op &&
    m_front == u->m_front;
}

ExpressionPtr UnaryOpExpression::preOptimize(AnalysisResultConstPtr ar) {
  Variant value;
  Variant result;

  if (m_exp && ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    if (m_op == T_UNSET) {
      if (m_exp->isScalar() ||
          (m_exp->is(KindOfExpressionList) &&
           static_pointer_cast<ExpressionList>(m_exp)->getCount() == 0)) {
        recomputeEffects();
        return CONSTANT("null");
      }
      return ExpressionPtr();
    }
  }

  if (m_op == T_ISSET && m_exp->is(KindOfExpressionList) &&
      static_pointer_cast<ExpressionList>(m_exp)->getListKind() ==
      ExpressionList::ListKindParam) {
    ExpressionListPtr el(static_pointer_cast<ExpressionList>(m_exp));
    result = true;
    int i = 0, n = el->getCount();
    for (; i < n; i++) {
      ExpressionPtr e((*el)[i]);
      if (!e || !e->isScalar() || !e->getScalarValue(value)) break;
      if (value.isNull()) {
        result = false;
      }
    }
    if (i == n) {
      return replaceValue(makeScalarExpression(ar, result));
    }
  } else if (m_op != T_ARRAY &&
             m_exp &&
             m_exp->isScalar() &&
             m_exp->getScalarValue(value) &&
             preCompute(value, result)) {
    return replaceValue(makeScalarExpression(ar, result));
  } else if (m_op == T_BOOL_CAST) {
    switch (m_exp->getKindOf()) {
      default: break;
      case KindOfBinaryOpExpression: {
        int op = static_pointer_cast<BinaryOpExpression>(m_exp)->getOp();
        switch (op) {
          case T_LOGICAL_OR:
          case T_BOOLEAN_OR:
          case T_LOGICAL_AND:
          case T_BOOLEAN_AND:
          case T_LOGICAL_XOR:
          case T_INSTANCEOF:
          case '<':
          case T_IS_SMALLER_OR_EQUAL:
          case '>':
          case T_IS_GREATER_OR_EQUAL:
          case T_IS_IDENTICAL:
          case T_IS_NOT_IDENTICAL:
          case T_IS_EQUAL:
          case T_IS_NOT_EQUAL:
            return m_exp;
        }
        break;
      }
      case KindOfUnaryOpExpression: {
        int op = static_pointer_cast<UnaryOpExpression>(m_exp)->getOp();
        switch (op) {
          case T_BOOL_CAST:
          case '!':
          case T_ISSET:
          case T_EMPTY:
          case T_PRINT:
            return m_exp;
        }
        break;
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr UnaryOpExpression::postOptimize(AnalysisResultConstPtr ar) {
  if (m_op == T_PRINT && m_exp->is(KindOfEncapsListExpression) &&
      !m_exp->hasEffect()) {
    EncapsListExpressionPtr e = static_pointer_cast<EncapsListExpression>
      (m_exp);
    e->stripConcat();
  }

  if (m_op == T_UNSET_CAST && !hasEffect()) {
    if (!getScope()->getVariables()->
        getAttribute(VariableTable::ContainsCompact) ||
        !m_exp->isScalar()) {
      return CONSTANT("null");
    }
  } else if (m_op == T_UNSET && m_exp->is(KindOfExpressionList) &&
             !static_pointer_cast<ExpressionList>(m_exp)->getCount()) {
    recomputeEffects();
    return CONSTANT("null");
  } else if (m_op == T_BOOL_CAST) {
    if (m_exp->getActualType() &&
        m_exp->getActualType()->is(Type::KindOfBoolean)) {
      return replaceValue(m_exp);
    }
  } else if (m_op != T_ARRAY &&
             m_exp &&
             m_exp->isScalar()) {
    Variant value;
    Variant result;
    if (m_exp->getScalarValue(value) && preCompute(value, result)) {
      return replaceValue(makeScalarExpression(ar, result));
    }
  }
  return ExpressionPtr();
}

void UnaryOpExpression::setExistContext() {
  if (m_exp) {
    if (m_exp->is(Expression::KindOfExpressionList)) {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (exps->getListKind() == ExpressionList::ListKindParam) {
        for (int i = 0; i < exps->getCount(); i++) {
          (*exps)[i]->setContext(Expression::ExistContext);
        }
        return;
      }
    }

    m_exp->setContext(Expression::ExistContext);
  }
}

TypePtr UnaryOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  TypePtr et; // expected m_exp's type
  TypePtr rt; // return type

  switch (m_op) {
  case '!':             et = rt = Type::Boolean;                     break;
  case '+':
  case '-':             et = Type::Numeric; rt = Type::Numeric;      break;
  case T_INC:
  case T_DEC:
  case '~':             et = rt = Type::Primitive;                   break;
  case T_CLONE:         et = Type::Some;      rt = Type::Object;     break;
  case '@':             et = type;            rt = Type::Variant;    break;
  case T_INT_CAST:      et = rt = Type::Int64;                       break;
  case T_DOUBLE_CAST:   et = rt = Type::Double;                      break;
  case T_STRING_CAST:   et = rt = Type::String;                      break;
  case T_ARRAY:         et = Type::Some;      rt = Type::Array;      break;
  case T_ARRAY_CAST:    et = rt = Type::Array;                       break;
  case T_OBJECT_CAST:   et = rt = Type::Object;                      break;
  case T_BOOL_CAST:     et = rt = Type::Boolean;                     break;
  case T_UNSET_CAST:    et = Type::Some;      rt = Type::Variant;    break;
  case T_UNSET:         et = Type::Null;      rt = Type::Variant;    break;
  case T_EXIT:          et = Type::Primitive; rt = Type::Variant;    break;
  case T_PRINT:         et = Type::String;    rt = Type::Int64;      break;
  case T_ISSET:         et = Type::Variant;   rt = Type::Boolean;
    setExistContext();
    break;
  case T_EMPTY:         et = Type::Some;      rt = Type::Boolean;
    setExistContext();
    break;
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:  et = Type::String;    rt = Type::Variant;    break;
  case T_EVAL:
    et = Type::String;
    rt = Type::Any;
    getScope()->getVariables()->forceVariants(ar, VariableTable::AnyVars);
    break;
  case T_DIR:
  case T_FILE:          et = rt = Type::String;                      break;
  default:
    assert(false);
  }

  if (m_exp) {
    TypePtr expType = m_exp->inferAndCheck(ar, et, false);
    if (Type::SameType(expType, Type::String) &&
        (m_op == T_INC || m_op == T_DEC)) {
      rt = expType = m_exp->inferAndCheck(ar, Type::Variant, true);
    }

    switch (m_op) {
    case '+':
    case '-':
      if (Type::SameType(expType, Type::Int64) ||
          Type::SameType(expType, Type::Double)) {
        rt = expType;
      }
      break;
    case T_INC:
    case T_DEC:
    case '~':
      if (Type::SameType(expType, Type::Int64) ||
          Type::SameType(expType, Type::Double) ||
          Type::SameType(expType, Type::String)) {
        rt = expType;
      }
      break;
    case T_ISSET:
    case T_EMPTY:
      if (m_exp->is(Expression::KindOfExpressionList)) {
        ExpressionListPtr exps =
          dynamic_pointer_cast<ExpressionList>(m_exp);
        if (exps->getListKind() == ExpressionList::ListKindParam) {
          for (int i = 0; i < exps->getCount(); i++) {
            SetExpTypeForExistsContext(ar, (*exps)[i], m_op == T_EMPTY);
          }
        }
      } else {
        SetExpTypeForExistsContext(ar, m_exp, m_op == T_EMPTY);
      }
      break;
    default:
      break;
    }
  }

  return rt;
}

void UnaryOpExpression::SetExpTypeForExistsContext(AnalysisResultPtr ar,
                                                   ExpressionPtr e,
                                                   bool allowPrimitives) {
  if (!e) return;
  TypePtr at(e->getActualType());
  if (!allowPrimitives && at &&
      at->isExactType() && at->isPrimitive()) {
    at = e->inferAndCheck(ar, Type::Variant, true);
  }
  TypePtr it(e->getImplementedType());
  TypePtr et(e->getExpectedType());
  if (et && et->is(Type::KindOfVoid)) e->setExpectedType(TypePtr());
  if (at && (!it || Type::IsMappedToVariant(it)) &&
      ((allowPrimitives && Type::HasFastCastMethod(at)) ||
       (!allowPrimitives &&
        (at->is(Type::KindOfObject) ||
         at->is(Type::KindOfArray) ||
         at->is(Type::KindOfString))))) {
    e->setExpectedType(it ? at : TypePtr());
  }
}

ExpressionPtr UnaryOpExpression::unneededHelper() {
  if ((m_op != '@' && m_op != T_ISSET && m_op != T_EMPTY) ||
      !m_exp->getContainedEffects()) {
    return Expression::unneededHelper();
  }

  if (m_op == '@') {
    m_exp = m_exp->unneeded();
  }

  return static_pointer_cast<Expression>(shared_from_this());
}

///////////////////////////////////////////////////////////////////////////////

void UnaryOpExpression::outputCodeModel(CodeGenerator &cg) {
  auto numProps = m_exp == nullptr ? 2 : 3;
  switch (m_op) {
    case T_UNSET:
    case T_EXIT:
    case T_ARRAY:
    case T_ISSET:
    case T_EMPTY:
    case T_EVAL: {
      cg.printObjectHeader("SimpleFunctionCallExpression", numProps);
      std::string funcName;
      switch (m_op) {
        case T_UNSET: funcName = "unset"; break;
        case T_EXIT: funcName = "exit"; break;
        case T_ARRAY: funcName = "array"; break;
        case T_ISSET: funcName = "isset"; break;
        case T_EMPTY: funcName = "empty"; break;
        case T_EVAL: funcName = "eval"; break;
        default: break;
      }
      cg.printPropertyHeader("functionName");
      cg.printValue(funcName);
      if (m_exp != nullptr) {
        cg.printPropertyHeader("arguments");
        cg.printExpressionVector(m_exp);
      }
      cg.printPropertyHeader("sourceLocation");
      cg.printLocation(this->getLocation());
      cg.printObjectFooter();
      return;
    }
    default:
      break;
  }

  switch (m_op) {
    case T_FILE:
    case T_DIR:
    case T_CLASS:
    case T_FUNCTION: {
      cg.printObjectHeader("SimpleVariableExpression", 2);
      std::string varName;
      switch (m_op) {
        case T_FILE: varName = "__FILE__"; break;
        case T_DIR: varName = "__DIR__"; break;
        //case T_CLASS: varName = "class"; break;
        //case T_FUNCTION: varName = "function"; break;
        default:
          assert(false); //fishing expedition. Are these cases dead?
          break;
      }
      cg.printPropertyHeader("variableName");
      cg.printValue(varName);
      cg.printPropertyHeader("sourceLocation");
      cg.printLocation(this->getLocation());
      cg.printObjectFooter();
      return;
    }
    default:
      break;
  }

  cg.printObjectHeader("UnaryOpExpression", numProps);
  if (m_exp != nullptr) {
    cg.printPropertyHeader("expression");
    m_exp->outputCodeModel(cg);
  }
  cg.printPropertyHeader("operation");
  int op = 0;
  switch (m_op) {
    case T_CLONE: op = PHP_CLONE_OP; break;
    case T_INC:
      op = m_front ? PHP_PRE_INCREMENT_OP : PHP_POST_INCREMENT_OP;
      break;
    case T_DEC:
      op = m_front ? PHP_PRE_DECREMENT_OP : PHP_POST_DECREMENT_OP;
      break;
    case '+': op = PHP_PLUS_OP; break;
    case '-': op = PHP_MINUS_OP; break;
    case '!': op = PHP_NOT_OP;  break;
    case '~': op = PHP_BITWISE_NOT_OP; break;
    case T_INT_CAST: op = PHP_INT_CAST_OP; break;
    case T_DOUBLE_CAST: op = PHP_FLOAT_CAST_OP; break;
    case T_STRING_CAST: op = PHP_STRING_CAST_OP; break;
    case T_ARRAY_CAST: op = PHP_ARRAY_CAST_OP; break;
    case T_OBJECT_CAST: op = PHP_OBJECT_CAST_OP; break;
    case T_BOOL_CAST: op = PHP_BOOL_CAST_OP; break;
    case T_UNSET_CAST: op = PHP_UNSET_CAST_OP; break;
    case '@': op = PHP_ERROR_CONTROL_OP; break;
    case T_PRINT: op = PHP_PRINT_OP; break;
    case T_INCLUDE: op = PHP_INCLUDE_OP; break;
    case T_INCLUDE_ONCE: op = PHP_INCLUDE_ONCE_OP; break;
    case T_REQUIRE: op = PHP_REQUIRE_OP; break;
    case T_REQUIRE_ONCE: op = PHP_REQUIRE_ONCE_OP; break;
    default:
      assert(false);
  }
  cg.printValue(op);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UnaryOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_front) {
    switch (m_op) {
    case T_CLONE:         cg_printf("clone ");        break;
    case T_INC:           cg_printf("++");            break;
    case T_DEC:           cg_printf("--");            break;
    case '+':             cg_printf("+");             break;
    case '-':             cg_printf("-");             break;
    case '!':             cg_printf("!");             break;
    case '~':             cg_printf("~");             break;
    case T_INT_CAST:      cg_printf("(int)");         break;
    case T_DOUBLE_CAST:   cg_printf("(double)");      break;
    case T_STRING_CAST:   cg_printf("(string)");      break;
    case T_ARRAY_CAST:    cg_printf("(array)");       break;
    case T_OBJECT_CAST:   cg_printf("(object)");      break;
    case T_BOOL_CAST:     cg_printf("(bool)");        break;
    case T_UNSET:         cg_printf("unset(");        break;
    case T_UNSET_CAST:    cg_printf("(unset)");       break;
    case T_EXIT:          cg_printf("exit(");         break;
    case '@':             cg_printf("@");             break;
    case T_ARRAY:         cg_printf("array(");        break;
    case T_PRINT:         cg_printf("print ");        break;
    case T_ISSET:         cg_printf("isset(");        break;
    case T_EMPTY:         cg_printf("empty(");        break;
    case T_INCLUDE:       cg_printf("include ");      break;
    case T_INCLUDE_ONCE:  cg_printf("include_once "); break;
    case T_EVAL:          cg_printf("eval(");         break;
    case T_REQUIRE:       cg_printf("require ");      break;
    case T_REQUIRE_ONCE:  cg_printf("require_once "); break;
    case T_FILE:          cg_printf("__FILE__");      break;
    case T_DIR:           cg_printf("__DIR__");       break;
    case T_CLASS:         cg_printf("class ");        break;
    case T_FUNCTION:      cg_printf("function ");     break;
    default:
      assert(false);
    }
  }

  if (m_exp) m_exp->outputPHP(cg, ar);

  if (m_front) {
    switch (m_op) {
    case T_UNSET:
    case T_EXIT:
    case T_ARRAY:
    case T_ISSET:
    case T_EMPTY:
    case T_EVAL:          cg_printf(")");  break;
    default:
      break;
    }
  } else {
    switch (m_op) {
    case T_INC:           cg_printf("++"); break;
    case T_DEC:           cg_printf("--"); break;
    default:
      assert(false);
    }
  }
}
