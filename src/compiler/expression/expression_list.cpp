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

#include <compiler/expression/expression_list.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/analysis/function_scope.h>
#include <runtime/base/array/array_init.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ExpressionList::ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS, ListKind kind)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES), m_outputCount(-1),
    m_arrayElements(false), m_kind(kind) {
}

ExpressionPtr ExpressionList::clone() {
  ExpressionListPtr exp(new ExpressionList(*this));
  Expression::deepCopy(exp);
  exp->m_exps.clear();
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    exp->m_exps.push_back(Clone(m_exps[i]));
  }
  return exp;
}

void ExpressionList::toLower() {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ScalarExpressionPtr s = dynamic_pointer_cast<ScalarExpression>(m_exps[i]);
    s->toLower();
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ExpressionList::addElement(ExpressionPtr exp) {
  ArrayPairExpressionPtr ap = dynamic_pointer_cast<ArrayPairExpression>(exp);
  if (ap) {
    m_arrayElements = true;
  } else {
    ASSERT(!m_arrayElements);
  }
  m_exps.push_back(exp);
}

void ExpressionList::insertElement(ExpressionPtr exp, int index /* = 0 */) {
  m_exps.insert(m_exps.begin() + index, exp);
}

void ExpressionList::removeElement(int index) {
  m_exps.erase(m_exps.begin() + index, m_exps.begin() + index + 1);
}

bool ExpressionList::isScalar() const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i] && !m_exps[i]->isScalar()) return false;
  }
  return true;
}

unsigned int ExpressionList::getScalarCount() const {
  unsigned int count = 0;
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]->isScalar()) count++;
  }
  return count;
}

bool ExpressionList::isNoObjectInvolved() const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    TypePtr t = m_exps[i]->getActualType();
    if (t == NULL || !t->isNoObjectInvolved())
      return false;
  }
  return true;
}

bool ExpressionList::containsDynamicConstant(AnalysisResultPtr ar) const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]->containsDynamicConstant(ar)) return true;
  }
  return false;
}

bool ExpressionList::isScalarArrayPairs() const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ArrayPairExpressionPtr exp =
      dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
    if (!exp || !exp->isScalarArrayPair()) {
      return false;
    }
  }
  return true;
}

void ExpressionList::getStrings(std::vector<std::string> &strings) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ScalarExpressionPtr s = dynamic_pointer_cast<ScalarExpression>(m_exps[i]);
    strings.push_back(s->getString());
  }
}

bool ExpressionList::getScalarValue(Variant &value) {
  if (!isScalarArrayPairs()) return false;
  ArrayInit init(m_exps.size());
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ArrayPairExpressionPtr exp =
      dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
    ExpressionPtr name = exp->getName();
    ExpressionPtr val = exp->getValue();
    if (!name) {
      Variant v;
      bool ret = val->getScalarValue(v);
      if (!ret) ASSERT(false);
      init.set(i, v);
    } else {
      Variant n;
      Variant v;
      bool ret1 = name->getScalarValue(n);
      bool ret2 = val->getScalarValue(v);
      if (!(ret1 && ret2)) ASSERT(false);
      init.set(i, n, v);
    }
  }
  value = Array(init.create());
  return true;
}

void ExpressionList::setOutputCount(int count) {
  ASSERT(count >= 0 && count <= (int)m_exps.size());
  m_outputCount = count;
}

int ExpressionList::getOutputCount() const {
  return m_outputCount < 0 ? m_exps.size() : m_outputCount;
}

void ExpressionList::resetOutputCount() {
  m_outputCount = -1;
}

void ExpressionList::markParam(int p, bool noRefWrapper) {
  ExpressionPtr param = (*this)[p];
  if (param->hasContext(Expression::InvokeArgument)) {
    if (noRefWrapper) {
      param->setContext(Expression::NoRefWrapper);
    } else {
      param->clearContext(Expression::NoRefWrapper);
    }
  } else if (!param->hasContext(Expression::RefValue)) {
    param->setContext(Expression::InvokeArgument);
    param->setContext(Expression::RefValue);
    if (noRefWrapper) {
      param->setContext(Expression::NoRefWrapper);
    }
  }
}

void ExpressionList::markParams(bool noRefWrapper) {
  for (int i = 0; i < getCount(); i++) {
    markParam(i, noRefWrapper);
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ExpressionPtr &ExpressionList::operator[](int index) {
  ASSERT(index >= 0 && index < getCount());
  return m_exps[index];
}

void ExpressionList::analyzeProgram(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]) m_exps[i]->analyzeProgram(ar);
  }
}

ConstructPtr ExpressionList::getNthKid(int n) const {
  if (n < (int)m_exps.size()) {
    return m_exps[n];
  }
  return ConstructPtr();
}

int ExpressionList::getKidCount() const {
  return m_exps.size();
}

void ExpressionList::setNthKid(int n, ConstructPtr cp) {
  int m = m_exps.size();
  if (n >= m) {
    ASSERT(false);
  } else {
    m_exps[n] = boost::dynamic_pointer_cast<Expression>(cp);
  }
}

bool ExpressionList::optimize(AnalysisResultPtr ar) {
  bool changed = false;
  size_t i = m_exps.size();
  if (m_kind != ListKindParam) {
    if (i--) {
      while (i--) {
        ExpressionPtr &e = m_exps[i];
        if (!e || e->getContainedEffects() == NoEffect) {
          ar->incOptCounter();
          removeElement(i);
          changed = true;
        } else if (e->getLocalEffects() == NoEffect) {
          e = e->unneeded(ar);
          changed = true;
        }
      }
    }
    return changed;
  }

  if (hasContext(UnsetContext) &&
      ar->getPhase() >= AnalysisResult::PostOptimize) {
    while (i--) {
      ExpressionPtr &e = m_exps[i];
      if (e->is(Expression::KindOfSimpleVariable)) {
        SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(e);
        const std::string &name = var->getName();
        VariableTablePtr variables = ar->getScope()->getVariables();
        if (variables->checkUnused(name)) {
          removeElement(i);
          changed = true;
        }
      }
    }
  }
  return changed;
}

ExpressionPtr ExpressionList::preOptimize(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ar->preOptimize(m_exps[i]);
  }
  return optimize(ar) ? static_pointer_cast<Expression>(shared_from_this())
                      : ExpressionPtr();
}

ExpressionPtr ExpressionList::postOptimize(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ar->postOptimize(m_exps[i]);
  }
  return optimize(ar) ? static_pointer_cast<Expression>(shared_from_this())
                      : ExpressionPtr();
}

TypePtr ExpressionList::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  size_t size = m_exps.size();
  bool commaList = size && (m_kind == ListKindComma ||
                            m_kind == ListKindWrapped);

  TypePtr tmp = commaList ? NEW_TYPE(Some) : type;
  for (size_t i = 0; i < size; i++) {
    TypePtr t = i + 1 < size ? tmp : type;
    bool c = coerce && !(commaList && i + 1 < size);
    if (m_exps[i]) m_exps[i]->inferAndCheck(ar, t, c);
  }
  return type;
}

bool ExpressionList::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  ExpressionListPtr l =
    static_pointer_cast<ExpressionList>(e);

  return m_arrayElements == l->m_arrayElements &&
    m_kind == l->m_kind;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ExpressionList::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (i > 0) cg.printf(", ");
    if (m_exps[i]) m_exps[i]->outputPHP(cg, ar);
  }
}

void ExpressionList::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                    int state) {
  return;
}

bool ExpressionList::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int state) {
  if (m_kind == ListKindParam && !m_arrayElements) {
    return Expression::preOutputCPP(cg, ar, state|StashKidVars);
  }

  bool inExpression = ar->inExpression();
  ar->setInExpression(false);
  bool ret = false;
  if (m_arrayElements) {
    ret = Expression::preOutputCPP(cg, ar, state);
  } else {
    for (unsigned int i = 0; i < m_exps.size(); i++) {
      if (m_exps[i]->preOutputCPP(cg, ar, 0)) {
        ret = true;
        break;
      }
    }
  }

  if (!inExpression) return ret;

  ar->setInExpression(true);
  if (!ret) return false;

  ar->wrapExpressionBegin(cg);
  if (m_arrayElements) {
    setCPPTemp(genCPPTemp(cg, ar));
    outputCPPInternal(cg, ar, true, true);
  } else {
    for (unsigned int i = 0, n = m_exps.size(); i < n; i++) {
      ExpressionPtr e = m_exps[i];
      e->preOutputCPP(cg, ar, state);
      if (i < n - 1) {
        if (e->outputCPPUnneeded(cg, ar)) {
          cg.printf(";\n");
        }
        e->setCPPTemp("/**/");
      }
    }
  }
  return true;
}

void ExpressionList::outputCPPInternal(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       bool needed, bool pre) {
  bool needsComma = false;
  if (m_arrayElements) {
    bool isVector = true;
    for (unsigned int i = 0; i < m_exps.size(); i++) {
      ArrayPairExpressionPtr ap =
        dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
      if (ap->getName()) {
        isVector = false;
        break;
      }
    }
    cg.printf("ArrayInit");
    if (pre) {
      cg.printf(" %s", m_cppTemp.c_str());
    }
    cg.printf("(%d, %s)", m_exps.size(), isVector ? "true" : "false");
    if (pre) cg.printf(";\n");
    needsComma = true;
  }

  unsigned i = 0, s = m_exps.size();
  for ( ; i < s; i++) {
    cg.setItemIndex(i);
    if (ExpressionPtr exp = m_exps[i]) {
      if (pre) {
        exp->preOutputCPP(cg, ar, 0);
        cg.printf("%s", m_cppTemp.c_str());
      }
      if (needsComma) cg.printf(m_arrayElements ? "." : ", ");
      if (m_arrayElements) {
        ArrayPairExpressionPtr ap =
          dynamic_pointer_cast<ArrayPairExpression>(exp);
        if (ap->isRef()) {
          cg.printf("setRef(%d, ", i);
          // The value itself shouldn't be wrapped with ref() any more.
          ap->getValue()->setContext(NoRefWrapper);
        } else {
          cg.printf("set(%d, ", i);
        }
        exp->outputCPP(cg, ar);
        cg.printf(")");
        if (pre) {
          cg.printf(";\n");
        }
        needsComma = true;
      } else if (m_kind != ListKindParam && (i + 1 < s || !needed)) {
        needsComma = exp->outputCPPUnneeded(cg, ar);
      } else {
        exp->outputCPP(cg, ar);
        needsComma = true;
      }
    }
  }
  if (i && !needsComma) {
    cg.printf("id(0)");
  }
  if (m_arrayElements && !pre) {
    cg.printf(".create()");
  }
}

void ExpressionList::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_kind == ListKindWrapped) cg.printf("(");
  outputCPPInternal(cg, ar, true, false);
  if (m_kind == ListKindWrapped) cg.printf(")");
}

bool ExpressionList::outputCPPUnneeded(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  bool inExpression = ar->inExpression();
  bool wrapped = false;
  if (!inExpression) {
    ar->setInExpression(true);
    wrapped = preOutputCPP(cg, ar, 0);
  }

  outputCPPInternal(cg, ar, false, false);

  if (!inExpression) {
    if (wrapped) cg.printf(";\n");
    ar->wrapExpressionEnd(cg);
    ar->setInExpression(inExpression);
  }
  return true;
}
