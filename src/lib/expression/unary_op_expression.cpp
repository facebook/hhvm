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

#include <lib/expression/unary_op_expression.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/analysis/dependency_graph.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/file_scope.h>
#include <lib/statement/statement_list.h>
#include <lib/option.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/function_scope.h>
#include <lib/expression/simple_variable.h>
#include <lib/analysis/variable_table.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/constant_expression.h>
#include <cpp/base/builtin_functions.h>
#include <lib/parser/parser.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UnaryOpExpression::UnaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op, bool front)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp), m_op(op), m_front(front), m_effect(false), m_arrayId(-1) {
  switch (m_op) {
  case T_INC:
  case T_DEC:
    m_effect = true;
    // this is hacky, what we need is LValueWrapper
    if (!m_exp->is(Expression::KindOfSimpleVariable)) {
      m_exp->setContext(Expression::LValue);
      m_exp->clearContext(Expression::NoLValueWrapper);
    }
    break;
  case '@':
  case '(':
    m_effect = m_exp->hasEffect();
    break;
  case T_EXIT:
  case T_PRINT:
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:
  case T_EVAL:
    m_effect = true;
    break;
  case T_ISSET:
    break;
  case T_ARRAY:
    {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (exps) exps->controlOrder();
    }
  default:
    break;
  }
}

ExpressionPtr UnaryOpExpression::clone() {
  UnaryOpExpressionPtr exp(new UnaryOpExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exp = Clone(m_exp);
  return exp;
}

bool UnaryOpExpression::isScalar() const {
  switch (m_op) {
  case '!':
  case '+':
  case '-':
  case T_INC:
  case T_DEC:
  case '~':
  case '@':
  case '(':
    return m_exp->isScalar();
  case T_ARRAY:
    return (!m_exp || m_exp->isScalar());
  default:
    break;
  }
  return false;
}

bool UnaryOpExpression::isRefable() const {
  if (m_op == '(') return m_exp->isRefable();
  return false;
}

bool UnaryOpExpression::isThis() const {
  if (m_op == '(') return m_exp->isThis();
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
  if (m_op != T_ARRAY) return false;
  if (m_exp) return (m_exp->getScalarValue(value));
  value = Array::Create();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void UnaryOpExpression::onParse(AnalysisResultPtr ar) {
  if (m_op == T_EVAL) {
    ConstructPtr self = shared_from_this();
    ar->getCodeError()->record(self, CodeError::UseEvaluation, self);
    ar->getFileScope()->setAttribute(FileScope::ContainsLDynamicVariable);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void UnaryOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->isFirstPass()) {
    ConstructPtr self = shared_from_this();
    if (m_op == T_INCLUDE || m_op == T_REQUIRE) {
      ar->getCodeError()->record(self, CodeError::UseInclude, self);
    }
  }
  if (m_exp) m_exp->analyzeProgram(ar);
}

bool UnaryOpExpression::preCompute(AnalysisResultPtr ar, CVarRef value,
                                   Variant &result) {
  switch(m_op) {
  case '!':
    result = (!toBoolean(value)); break;
  case '+':
    result = value.unary_plus(); break;
  case '-':
    result = value.negate(); break;
  case '~':
    result = ~value; break;
  case '(':
    result = value; break;
  case T_INT_CAST:
    result = value.toInt64(); break;
  case T_DOUBLE_CAST:
    result = toDouble(value); break;
  case T_STRING_CAST:
    result = toString(value); break;
  case T_BOOL_CAST:
    result = toBoolean(value); break;
  case T_INC:
  case T_DEC:
    ASSERT(false);
  default:
    return false;
  }
  return true;
}

ConstructPtr UnaryOpExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int UnaryOpExpression::getKidCount() const {
  return 1;
}

int UnaryOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr UnaryOpExpression::preOptimize(AnalysisResultPtr ar) {
  Variant value;
  Variant result;
  bool hasResult;

  ar->preOptimize(m_exp);

  if (!m_exp || !m_exp->isScalar()) return ExpressionPtr();
  if (!m_exp->getScalarValue(value)) return ExpressionPtr();
  hasResult = preCompute(ar, value, result);
  if (hasResult) {
    if (result.isNull()) {
      return CONSTANT("null");
    } else if (result.isBoolean()) {
      return CONSTANT(result ? "true" : "false");
    } else if (result.isDouble() && !finite(result.getDouble())) {
      return ExpressionPtr();
    } else if (result.isArray()) {
      return ExpressionPtr();
    } else {
      return ScalarExpressionPtr
               (new ScalarExpression(m_exp->getLocation(),
                                     Expression::KindOfScalarExpression,
                                     result));
    }
  }
  return ExpressionPtr();
}

ExpressionPtr UnaryOpExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return ExpressionPtr();
}

TypePtr UnaryOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                      bool coerce) {
  TypePtr et; // expected m_exp's type
  TypePtr rt; // return type

  switch (m_op) {
  case '!':             et = rt = Type::Boolean;                         break;
  case '+':
  case '-':             et = NEW_TYPE(Numeric); rt = NEW_TYPE(Numeric);  break;
  case T_INC:
  case T_DEC:
  case '~':             et = rt = NEW_TYPE(Primitive);                   break;
  case T_CLONE:         et = rt = NEW_TYPE(Object);                      break;
  case '@':
  case '(':             et = rt = type;                                  break;
  case T_INT_CAST:      et = NEW_TYPE(Some);      rt = Type::Int64;      break;
  case T_DOUBLE_CAST:   et = NEW_TYPE(Some);      rt = Type::Double;     break;
  case T_STRING_CAST:   et = NEW_TYPE(Some);      rt = Type::String;     break;
  case T_ARRAY:
  case T_ARRAY_CAST:    et = NEW_TYPE(Some);      rt = Type::Array;      break;
  case T_OBJECT_CAST:   et = NEW_TYPE(Some);      rt = NEW_TYPE(Object); break;
  case T_BOOL_CAST:     et = NEW_TYPE(Some);      rt = Type::Boolean;    break;
  case T_UNSET_CAST:    et = NEW_TYPE(Some);      rt = Type::Variant;    break;
  case T_EXIT:          et = NEW_TYPE(Primitive); rt = Type::Variant;    break;
  case T_PRINT:         et = Type::String;        rt = Type::Boolean;    break;
  case T_ISSET:
    et = Type::Variant;
    rt = Type::Boolean;
    if (m_exp && m_exp->is(Expression::KindOfExpressionList)) {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      for (int i = 0; i < exps->getCount(); i++) {
        (*exps)[i]->setContext(Expression::IssetContext);
      }
    }
    break;
  case T_EMPTY:         et = NEW_TYPE(Some);      rt = Type::Boolean;    break;
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:  et = Type::String;        rt = Type::Boolean;    break;
  case T_EVAL:
    et = Type::String;
    rt = NEW_TYPE(Any);
    ar->getScope()->getVariables()->forceVariants(ar);
    break;
  case T_FILE:          et = rt = Type::String;                          break;
  default:
    ASSERT(false);
  }

  bool insideScalarArray = ar->getInsideScalarArray();

  if (m_op == T_ARRAY &&
      (getContext() & (RefValue|LValue)) == 0) {
    if (m_exp) {
      ExpressionListPtr pairs = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (pairs && pairs->isScalarArrayPairs()) {
        m_arrayId = ar->registerScalarArray(m_exp);
        ar->setInsideScalarArray(true);
      }
    } else {
      m_arrayId = ar->registerScalarArray(m_exp); // empty array
    }
  }

  if (m_exp) {
    bool ecoerce = false; // expected type needs m_exp to coerce to
    switch (m_op) {
    case T_CLONE:
    case T_ISSET:
      ecoerce = true;
    default:
      break;
    }

    TypePtr expType = m_exp->inferAndCheck(ar, et, ecoerce);
    if (Type::SameType(expType, Type::String) &&
        (m_op == T_INC || m_op == T_DEC)) {
      rt = expType = m_exp->inferAndCheck(ar, Type::Variant, true);
    }

    switch (m_op) {
    case '@':
    case '(':
      rt = expType;
      break;
    case '+':
    case '-':
      if (Type::SameType(expType, Type::Int64) ||
          Type::SameType(expType, Type::Double)) {
        rt = expType;
      }
      break;
    case T_INC:
    case T_DEC:
      if (m_exp->is(Expression::KindOfSimpleVariable)) {
        FunctionScopePtr func =
          dynamic_pointer_cast<FunctionScope>(ar->getScope());
        if (func) {
          SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(m_exp);
          VariableTablePtr variables = ar->getScope()->getVariables();
          const std::string &name = var->getName();
          if (variables->isParameter(name)) {
            variables->addLvalParam(name);
          }
        }
      }
      // fall through
    case '~':
      if (Type::SameType(expType, Type::Int64) ||
          Type::SameType(expType, Type::Double) ||
          Type::SameType(expType, Type::String)) {
        rt = expType;
      }
      break;
    default:
      break;
    }
  }

  ar->setInsideScalarArray(insideScalarArray);

  return rt;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UnaryOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_front) {
    switch (m_op) {
    case T_CLONE:         cg.printf("clone ");        break;
    case T_INC:           cg.printf("++");            break;
    case T_DEC:           cg.printf("--");            break;
    case '+':             cg.printf("+");             break;
    case '-':             cg.printf("-");             break;
    case '!':             cg.printf("!");             break;
    case '~':             cg.printf("~");             break;
    case '(':             cg.printf("(");             break;
    case T_INT_CAST:      cg.printf("(int)");         break;
    case T_DOUBLE_CAST:   cg.printf("(real)");        break;
    case T_STRING_CAST:   cg.printf("(string)");      break;
    case T_ARRAY_CAST:    cg.printf("(array)");       break;
    case T_OBJECT_CAST:   cg.printf("(object)");      break;
    case T_BOOL_CAST:     cg.printf("(bool)");        break;
    case T_UNSET_CAST:    cg.printf("(unset)");       break;
    case T_EXIT:          cg.printf("exit(");         break;
    case '@':             cg.printf("@");             break;
    case T_ARRAY:         cg.printf("array(");        break;
    case T_PRINT:         cg.printf("print ");        break;
    case T_ISSET:         cg.printf("isset(");        break;
    case T_EMPTY:         cg.printf("empty(");        break;
    case T_INCLUDE:       cg.printf("include ");      break;
    case T_INCLUDE_ONCE:  cg.printf("include_once "); break;
    case T_EVAL:          cg.printf("eval(");         break;
    case T_REQUIRE:       cg.printf("require ");      break;
    case T_REQUIRE_ONCE:  cg.printf("require_once "); break;
    case T_FILE:          cg.printf("__FILE__");      break;
    default:
      ASSERT(false);
    }
  }

  if (m_exp) m_exp->outputPHP(cg, ar);

  if (m_front) {
    switch (m_op) {
    case '(':
    case T_EXIT:
    case T_ARRAY:
    case T_ISSET:
    case T_EMPTY:
    case T_EVAL:          cg.printf(")");  break;
    default:
      break;
    }
  } else {
    switch (m_op) {
    case T_INC:           cg.printf("++"); break;
    case T_DEC:           cg.printf("--"); break;
    default:
      ASSERT(false);
    }
  }
}

void UnaryOpExpression::outputCPPImpl(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  if (m_arrayId != -1) {
    if (cg.getOutput() == CodeGenerator::SystemCPP) {
      cg.printf("SystemScalarArrays::%s[%d]",
                Option::SystemScalarArrayName, m_arrayId);
    } else {
      cg.printf("ScalarArrays::%s[%d]",
                Option::ScalarArrayName, m_arrayId);
    }
    return;
  }

  if ((m_op == T_ISSET || m_op == T_EMPTY) && m_exp) {
    if (m_exp->is(Expression::KindOfExpressionList)) {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (exps->getCount() > 1) {
        cg.printf("(");
      }
      for (int i = 0; i < exps->getCount(); i++) {
        if (i > 0) cg.printf(" && ");
        (*exps)[i]->outputCPPExistTest(cg, ar, m_op);
      }
      if (exps->getCount() > 1) {
        cg.printf(")");
      }
    } else {
      m_exp->outputCPPExistTest(cg, ar, m_op);
    }
    return;
  }

  if (m_front) {
    if (m_op == T_ARRAY) {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (exps) exps->outputCPPControlledEvalOrderPre(cg, ar);
    }
    switch (m_op) {
    case T_CLONE:         cg.printf("f_clone(");   break;
    case T_INC:           cg.printf("++");         break;
    case T_DEC:           cg.printf("--");         break;
    case '+':             cg.printf("+");          break;
    case '-':             cg.printf("negate(");    break;
    case '!':             cg.printf("!(");         break;
    case '~':             cg.printf("~");          break;
    case '(':             cg.printf("(");          break;
    case T_INT_CAST:      cg.printf("toInt64(");   break;
    case T_DOUBLE_CAST:   cg.printf("toDouble(");  break;
    case T_STRING_CAST:   cg.printf("toString(");  break;
    case T_ARRAY_CAST:    cg.printf("toArray(");   break;
    case T_OBJECT_CAST:   cg.printf("toObject(");  break;
    case T_BOOL_CAST:     cg.printf("toBoolean("); break;
    case T_UNSET_CAST:    cg.printf("unset(");     break;
    case T_EXIT:          cg.printf("f_exit(");    break;
    case T_ARRAY:
      if (ar->getInsideScalarArray()) {
        cg.printf("StaticArray(");
      } else {
        cg.printf("Array(");
      }
      break;
    case T_PRINT:         cg.printf("print(");     break;
    case T_EVAL:
      if (Option::EnableEval > Option::NoEval) {
        cg.printf("eval(%s, Object(%s), ",
                  ar->getScope()->inPseudoMain() ?
                  "get_variable_table()" : "variables",
                  ar->getClassScope() ? "this" : "");
      } else {
        cg.printf("f_eval(");
      }
      break;
    case T_INCLUDE:
    case T_INCLUDE_ONCE:
    case T_REQUIRE:
    case T_REQUIRE_ONCE:  cg.printf("f_include("); break;
    case '@':
      cg.printf("(silenceInc(), silenceDec(");
      break;
    case T_FILE:
      cg.printf("get_source_filename(\"%s\")", getLocation()->file);
      break;
      break;
    default:
      ASSERT(false);
    }
  }


  if (m_exp) {
    switch (m_op) {
    case '+':
    case '-':
      if (m_exp->getActualType() &&
          (m_exp->getActualType()->is(Type::KindOfString) ||
           m_exp->getActualType()->is(Type::KindOfArray))) {
        cg.printf("(Variant)(");
        m_exp->outputCPP(cg, ar);
        cg.printf(")");
      } else {
        m_exp->outputCPP(cg, ar);
      }
      break;
    case '@':
      // Void needs to return something to silenceDec
      if (!m_exp->getActualType()) {
        cg.printf("(");
        m_exp->outputCPP(cg, ar);
        cg.printf(",null)");
      } else {
        m_exp->outputCPP(cg, ar);
      }
      break;
    default:
      m_exp->outputCPP(cg, ar);
      break;
    }
  }

  if (m_front) {
    switch (m_op) {
    case T_ARRAY:
      {
        ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
        if (exps) exps->outputCPPControlledEvalOrderPost(cg, ar);
        cg.printf(")");
        break;
      }
    case T_CLONE:
    case '!':
    case '(':
    case '-':
    case T_INT_CAST:
    case T_DOUBLE_CAST:
    case T_STRING_CAST:
    case T_ARRAY_CAST:
    case T_OBJECT_CAST:
    case T_BOOL_CAST:
    case T_UNSET_CAST:
    case T_EXIT:
    case T_PRINT:
    case T_EVAL:
    case T_INCLUDE:
    case T_INCLUDE_ONCE:
    case T_REQUIRE:
    case T_REQUIRE_ONCE:
      cg.printf(")");
      break;
    case '@':
      cg.printf("))");
      break;
    default:
      break;
    }
  } else {
    switch (m_op) {
    case T_INC:           cg.printf("++"); break;
    case T_DEC:           cg.printf("--"); break;
    default:
      ASSERT(false);
    }
  }
}
