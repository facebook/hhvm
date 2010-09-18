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

#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/parser/hphp.tab.hpp>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/encaps_list_expression.h>
#include <runtime/base/builtin_functions.h>
#include <compiler/parser/parser.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UnaryOpExpression::UnaryOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp, int op, bool front)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_exp(exp), m_op(op), m_front(front),
    m_silencer(-1), m_localEffects(0) {
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
  case T_UNSET:
    m_localEffects = AssignEffect;
    m_exp->setContext(UnsetContext);
    break;
  case T_ARRAY:
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
  case '(':
    return m_exp->isScalar();
  case T_ARRAY:
    return (!m_exp || m_exp->isScalar());
  default:
    break;
  }
  return false;
}

bool UnaryOpExpression::isRefable(bool checkError /*= false */) const {
  if (m_op == '(' || m_op == T_INC || m_op == T_DEC) {
    return m_exp->isRefable(checkError);
  }
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

void UnaryOpExpression::onParse(AnalysisResultPtr ar) {
  if (m_op == T_EVAL) {
    ConstructPtr self = shared_from_this();
    ar->getCodeError()->record(self, CodeError::UseEvaluation, self);
    ar->getFileScope()->setAttribute(FileScope::ContainsLDynamicVariable);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

int UnaryOpExpression::getLocalEffects() const {
  return m_localEffects;
}

void UnaryOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal && m_op == '@') {
    StatementPtr stmt = ar->getStatementForSilencer();
    ASSERT(stmt);
    m_silencer = stmt->requireSilencers(1);
  }
  if (ar->isFirstPass()) {
    ConstructPtr self = shared_from_this();
    if (m_op == T_INCLUDE || m_op == T_REQUIRE) {
      ar->getCodeError()->record(self, CodeError::UseInclude, self);
    }
  }
  if (m_exp) m_exp->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal && m_op == '@') {
    StatementPtr stmt = ar->getStatementForSilencer();
    ASSERT(stmt);
    stmt->endRequireSilencers(m_silencer);
  }
}

bool UnaryOpExpression::preCompute(CVarRef value, Variant &result) {
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
  case '@':
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
      ASSERT(false);
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
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

bool UnaryOpExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  UnaryOpExpressionPtr u =
    static_pointer_cast<UnaryOpExpression>(e);

  return m_op == u->m_op &&
    m_front == u->m_front &&
    m_silencer == u->m_silencer;
}

ExpressionPtr UnaryOpExpression::preOptimize(AnalysisResultPtr ar) {
  Variant value;
  Variant result;

  ar->preOptimize(m_exp);
  if (m_exp && ar->getPhase() >= AnalysisResult::FirstPreOptimize) {
    if (m_op == '(') {
      return m_exp;
    }
    if (m_op == T_UNSET) {
      if (m_exp->isScalar() ||
          (m_exp->is(KindOfExpressionList) &&
           static_pointer_cast<ExpressionList>(m_exp)->getCount() == 0)) {
        return CONSTANT("null");
      }
      return ExpressionPtr();
    }
  }

  if (m_op != T_ARRAY &&
      m_exp &&
      m_exp->isScalar() &&
      m_exp->getScalarValue(value) &&
      preCompute(value, result)) {
    return replaceValue(MakeScalarExpression(ar, getLocation(), result));
  }
  return ExpressionPtr();
}

ExpressionPtr UnaryOpExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  if (m_op == T_PRINT && m_exp->is(KindOfEncapsListExpression) &&
      !m_exp->hasEffect()) {
    EncapsListExpressionPtr e = static_pointer_cast<EncapsListExpression>
      (m_exp);
    e->stripConcat();
  }

  if (m_op == T_UNSET_CAST && !hasEffect()) {
    return CONSTANT("null");
  } else if (m_op == T_UNSET && m_exp->is(KindOfExpressionList) &&
             !static_pointer_cast<ExpressionList>(m_exp)->getCount()) {
    return CONSTANT("null");
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
  case '(':             et = rt = type;                              break;
  case T_INT_CAST:      et = rt = Type::Int64;                       break;
  case T_DOUBLE_CAST:   et = rt = Type::Double;                      break;
  case T_STRING_CAST:   et = rt = Type::String;                      break;
  case T_ARRAY:         et = Type::Some;      rt = Type::Array;      break;
  case T_ARRAY_CAST:    et = rt = Type::Array;                       break;
  case T_OBJECT_CAST:   et = rt = Type::Object;                      break;
  case T_BOOL_CAST:     et = rt = Type::Boolean;                     break;
  case T_UNSET_CAST:    et = Type::Some;      rt = Type::Variant;    break;
  case T_UNSET:         et = Type::Some;      rt = Type::Variant;    break;
  case T_EXIT:          et = Type::Primitive; rt = Type::Variant;    break;
  case T_PRINT:         et = Type::String;    rt = Type::Boolean;    break;
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
    ar->getScope()->getVariables()->forceVariants(ar);
    break;
  case T_FILE:          et = rt = Type::String;                      break;
  default:
    ASSERT(false);
  }

  if (m_exp) {
    bool ecoerce = false; // expected type needs m_exp to coerce to
    switch (m_op) {
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
    case '(':
      /*
        Need to make sure that type conversion happens at the right point.
        Without this, types are inferred differently:
        Consider (a+b)+1 and (a+b)."foo".
        In the first one, the "(" expression ends up with actualType int, in
        the second it ends up with actualType string. This is bad for cse,
        and type propagation (we rely on the fact that the same expression,
        with the same arguments, must have the same actual type).
      */
      m_exp->setExpectedType(TypePtr());
      rt = m_exp->getType();
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

  return rt;
}

ExpressionPtr UnaryOpExpression::unneededHelper(AnalysisResultPtr ar) {
  if ((m_op != '@' && m_op != T_ISSET && m_op != T_EMPTY) ||
      !m_exp->getContainedEffects()) {
    return Expression::unneededHelper(ar);
  }

  if (m_op == '@') {
    m_exp = m_exp->unneeded(ar);
  }

  return static_pointer_cast<Expression>(shared_from_this());
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
    case '(':             cg_printf("(");             break;
    case T_INT_CAST:      cg_printf("(int)");         break;
    case T_DOUBLE_CAST:   cg_printf("(real)");        break;
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
    default:
      ASSERT(false);
    }
  }

  if (m_exp) m_exp->outputPHP(cg, ar);

  if (m_front) {
    switch (m_op) {
    case '(':
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
      ASSERT(false);
    }
  }
}

void UnaryOpExpression::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                       int state) {
  if (hasCPPTemp() || m_op == T_FILE) return;
  if (m_exp && !getLocalEffects() &&
      m_op != '@' && m_op != T_ISSET && m_op != T_EMPTY &&
      !m_exp->is(KindOfExpressionList)) {
    m_exp->preOutputStash(cg, ar, state);
  } else {
    Expression::preOutputStash(cg, ar, state);
  }
}

bool UnaryOpExpression::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                     int state) {

  if (m_op == T_ISSET && m_exp && m_exp->is(Expression::KindOfExpressionList)) {
    ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
    int count = exps->getCount();
    if (count > 1) {
      bool fix_e1 = (*exps)[0]->preOutputCPP(cg, ar, 0);
      bool inExpression = ar->inExpression();
      ar->setInExpression(false);
      bool fix_en = false;
      for (int i = 1; i < count; i++) {
        if ((*exps)[i]->preOutputCPP(cg, ar, 0)) {
          fix_en = true;
          break;
        }
      }
      ar->setInExpression(inExpression);
      if (inExpression && fix_en) {
        ar->wrapExpressionBegin(cg);
        std::string tmp = genCPPTemp(cg, ar);
        cg_printf("bool %s = (", tmp.c_str());
        (*exps)[0]->outputCPPExistTest(cg, ar, m_op);
        cg_printf(");\n");
        for (int i = 1; i < count; i++) {
          cg_indentBegin("if (%s) {\n", tmp.c_str());
          ExpressionPtr e = (*exps)[i];
          e->preOutputCPP(cg, ar, 0);
          cg_printf("%s = (", tmp.c_str());
          e->outputCPPExistTest(cg, ar, m_op);
          cg_printf(");\n");
        }
        for (int i = 1; i < count; i++) {
          cg_indentEnd("}\n");
        }
        m_cppTemp = tmp;
      } else if (state & FixOrder) {
        preOutputStash(cg, ar, state);
        fix_e1 = true;
      }
      return fix_e1 || fix_en;
    }
  }

  if (m_op == '@') {
    if (isUnused()) m_exp->setUnused(true);
    bool inExpression = ar->inExpression();
    bool doit = state & FixOrder;
    if (!doit) {
      ar->setInExpression(false);
      if (m_exp->preOutputCPP(cg, ar, state)) {
        doit = true;
      }
      ar->setInExpression(inExpression);
    }
    if (doit && inExpression) {
      cg_printf("%s%d.enable();\n", Option::SilencerPrefix, m_silencer);
      m_exp->preOutputCPP(cg, ar, 0);
      int s = m_silencer;
      m_silencer = -1;
      this->preOutputStash(cg, ar, state | FixOrder);
      m_silencer = s;
      cg_printf("%s%d.disable();\n", Option::SilencerPrefix, m_silencer);
    }
    return doit;
  } else if (m_op == T_PRINT && m_exp && !m_exp->hasEffect()) {
    ExpressionPtrVec ev;
    bool hasVoid = false, hasLit = false;
    if (BinaryOpExpression::getConcatList(ev, m_exp, hasVoid, hasLit) > 1 ||
        hasVoid || hasLit) {

      if (!ar->inExpression()) return true;
      ar->wrapExpressionBegin(cg);
      for (int i = 0, s = ev.size(); i < s; i++) {
        ExpressionPtr e = ev[i];
        e->preOutputCPP(cg, ar, 0);
        cg_printf("print(");
        e->outputCPP(cg, ar);
        cg_printf(");\n");
      }
      m_cppTemp = "1";
      return true;
    }
  }

  return Expression::preOutputCPP(cg, ar, state);
}

bool UnaryOpExpression::outputCPPImplOpEqual(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  if (m_exp->is(Expression::KindOfObjectPropertyExpression)) {
    ObjectPropertyExpressionPtr var(
      dynamic_pointer_cast<ObjectPropertyExpression>(m_exp));
    if (var->isValid()) return false;
    var->outputCPPObject(cg, ar);
    cg_printf("o_assign_op<%s,%d>(",
              isUnused() ? "void" : "Variant", m_op);
    var->outputCPPProperty(cg, ar);
    cg_printf(", %s, %s)",
              isUnused() || m_front ? "null_variant" : "Variant(0)",
              ar->getClassScope() ? "s_class_name" : "empty_string");
    return true;
  }
  return false;
}

void UnaryOpExpression::outputCPPImpl(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  if ((m_op == T_INC || m_op == T_DEC) && outputCPPImplOpEqual(cg, ar)) {
    return;
  }
  if (m_op == T_ARRAY &&
      (getContext() & (RefValue|LValue)) == 0 &&
      !ar->getInsideScalarArray()) {
    int id = -1;
    int hash = -1;
    int index = -1;
    if (m_exp) {
      ExpressionListPtr pairs = dynamic_pointer_cast<ExpressionList>(m_exp);
      Variant v;
      if (pairs && pairs->isScalarArrayPairs() && pairs->getScalarValue(v)) {
        id = ar->registerScalarArray(m_exp, hash, index);
      }
    } else {
      id = ar->registerScalarArray(m_exp, hash, index); // empty array
    }
    if (id != -1) {
      ar->outputCPPScalarArrayId(cg, id, hash, index);
      return;
    }
  }

  if ((m_op == T_ISSET || m_op == T_EMPTY || m_op == T_UNSET) && m_exp) {
    if (m_exp->is(Expression::KindOfExpressionList)) {
      ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(m_exp);
      if (exps->getListKind() == ExpressionList::ListKindParam) {
        int count = exps->getCount();
        if (count > 1) {
          cg_printf("(");
        }
        for (int i = 0; i < count; i++) {
          if (m_op == T_UNSET) {
            if (i > 0) cg_printf(", ");
            (*exps)[i]->outputCPPUnset(cg, ar);
          } else {
            if (i > 0) cg_printf(" && ");
            (*exps)[i]->outputCPPExistTest(cg, ar, m_op);
          }
        }
        if (exps->getCount() > 1) {
          cg_printf(")");
        }
        return;
      }
    }
    if (m_op == T_UNSET) {
      m_exp->outputCPPUnset(cg, ar);
    } else {
      m_exp->outputCPPExistTest(cg, ar, m_op);
    }
    return;
  }

  if (m_front) {
    switch (m_op) {
    case T_CLONE:         cg_printf("f_clone(");   break;
    case T_INC:           cg_printf("++");         break;
    case T_DEC:           cg_printf("--");         break;
    case '+':             cg_printf("+");          break;
    case '-':             cg_printf("negate(");    break;
    case '!':             cg_printf("!(");         break;
    case '~':             cg_printf("~");          break;
    case '(':             cg_printf("(");          break;
    case T_INT_CAST:      cg_printf("(");          break;
    case T_DOUBLE_CAST:   cg_printf("(");          break;
    case T_STRING_CAST:   cg_printf("(");          break;
    case T_ARRAY_CAST:    cg_printf("(");          break;
    case T_OBJECT_CAST:   cg_printf("(");          break;
    case T_BOOL_CAST:     cg_printf("(");          break;
    case T_UNSET_CAST:
      if (m_exp->hasCPPTemp()) {
        cg_printf("(id(");
      } else {
        cg_printf("(");
      }
      break;
    case T_EXIT:          cg_printf("f_exit(");    break;
    case T_ARRAY:
      cg_printf("Array(");
      break;
    case T_PRINT:         cg_printf("print(");     break;
    case T_EVAL:
      if (Option::EnableEval > Option::NoEval) {
        bool instance;
        if (ar->getClassScope()) {
          FunctionScopePtr fs = ar->getFunctionScope();
          instance = fs && !fs->isStatic();
        } else {
          instance = false;
        }
        cg_printf("eval(%s, Object(%s), ",
                  ar->getScope()->inPseudoMain() ?
                  "get_variable_table()" : "variables",
                  instance ? "this" : "");
      } else {
        cg_printf("f_eval(");
      }
      break;
    case '@':
      if (m_silencer >= 0) {
        cg_printf("(%s%d.enable(),%s%d.disable(",
                  Option::SilencerPrefix, m_silencer,
                  Option::SilencerPrefix, m_silencer);
      }
      break;
    case T_FILE:
      cg_printf("get_source_filename(\"%s\")", getLocation()->file);
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
        cg_printf("(Variant)(");
        m_exp->outputCPP(cg, ar);
        cg_printf(")");
      } else {
        m_exp->outputCPP(cg, ar);
      }
      break;
    case '@':
      if (m_silencer < 0 && isUnused()) {
        m_exp->outputCPPUnneeded(cg, ar);
      } else if (!m_exp->hasCPPTemp() && !m_exp->getActualType()) {
        // Void needs to return something to silenceDec
        cg_printf("(");
        m_exp->outputCPP(cg, ar);
        cg_printf(",null)");
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
        if (!exps) {
          cg_printf("ArrayData::Create()");
        }
        cg_printf(")");
      }
      break;
    case T_UNSET_CAST:
      if (m_exp->hasCPPTemp()) {
        cg_printf("),null");
      } else {
        cg_printf(",null");
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
    case T_EXIT:
    case T_PRINT:
    case T_EVAL:
    case T_INCLUDE:
    case T_INCLUDE_ONCE:
    case T_REQUIRE:
    case T_REQUIRE_ONCE:
      cg_printf(")");
      break;
    case '@':
      if (m_silencer >= 0) {
        cg_printf("))");
      }
      break;
    default:
      break;
    }
  } else {
    switch (m_op) {
    case T_INC:           cg_printf("++"); break;
    case T_DEC:           cg_printf("--"); break;
    default:
      ASSERT(false);
    }
  }
}
