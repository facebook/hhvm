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

#include <lib/expression/expression_list.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/array_pair_expression.h>
#include <lib/analysis/function_scope.h>
#include <cpp/base/array/array_element.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ExpressionList::ExpressionList
(EXPRESSION_CONSTRUCTOR_PARAMETERS)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES), m_outputCount(-1),
    m_controlOrder(0), m_tempStart(0), m_arrayElements(false) {
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
    if (!m_exps[i]->isScalar()) return false;
  }
  return true;
}

bool ExpressionList::hasEffect() const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]->hasEffect()) return true;
  }
  return false;
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
  vector<ArrayElement*> elems;
  elems.reserve(128); // normally scalar arrays are large in size
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ArrayPairExpressionPtr exp =
      dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
    ExpressionPtr name = exp->getName();
    ExpressionPtr val = exp->getValue();
    if (!name) {
      Variant v;
      bool ret = val->getScalarValue(v);
      if (!ret) ASSERT(false);
      ArrayElement *elem = NEW(ArrayElement)(v);
      elems.push_back(elem);
    } else {
      Variant n;
      Variant v;
      bool ret1 = name->getScalarValue(n);
      bool ret2 = val->getScalarValue(v);
      if (!(ret1 && ret2)) ASSERT(false);
      ArrayElement *elem = NEW(ArrayElement)(n, v);
      elems.push_back(elem);
    }
  }
  value = ArrayData::Create(elems);
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

void ExpressionList::controlOrder(int withObj /* = 0 */) {
  if (!Option::ControlEvalOrder) return;
  m_controlOrder = withObj ? 1 + withObj : 1;
}

bool ExpressionList::controllingOrder() const {
  return m_controlOrder &&
    ((m_controlOrder >= 2 ? 1 : 0) + getOutputCount()) > 1 &&
    (m_controlOrder == 3 || hasEffect());
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ExpressionPtr &ExpressionList::operator[](int index) {
  ASSERT(index >= 0 && index < getCount());
  return m_exps[index];
}

void ExpressionList::analyzeProgram(AnalysisResultPtr ar) {
  analyzeProgramStart(ar);
  analyzeProgramEnd(ar);
}

void ExpressionList::analyzeProgramStart(AnalysisResultPtr ar) {
  bool analyzeOrder = ar->getPhase() == AnalysisResult::AnalyzeFinal &&
    controllingOrder();
  if (analyzeOrder) {
    FunctionScopePtr func = ar->getFunctionScope();
    ASSERT(func);
    m_tempStart = func->requireCallTemps(m_exps.size() +
                                         (m_controlOrder >= 2 ? 1 : 0));
  }

  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]) m_exps[i]->analyzeProgram(ar);
  }
}

void ExpressionList::analyzeProgramEnd(AnalysisResultPtr ar) {
  bool analyzeOrder = ar->getPhase() == AnalysisResult::AnalyzeFinal &&
    controllingOrder();
  if (analyzeOrder) {
    FunctionScopePtr func = ar->getFunctionScope();
    ASSERT(func);
    func->endRequireCallTemps(m_tempStart);
  }
}

ExpressionPtr ExpressionList::preOptimize(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ar->preOptimize(m_exps[i]);
  }
  return ExpressionPtr();
}

ExpressionPtr ExpressionList::postOptimize(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ar->postOptimize(m_exps[i]);
  }
  return ExpressionPtr();
}

TypePtr ExpressionList::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]) m_exps[i]->inferAndCheck(ar, type, coerce);
  }
  return type;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ExpressionList::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (i > 0) cg.printf(", ");
    if (m_exps[i]) m_exps[i]->outputPHP(cg, ar);
  }
}

void ExpressionList::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_arrayElements) {
    cg.printf("ArrayInit(%d).", m_exps.size());
  }
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    cg.setItemIndex(i);
    if (i > 0) cg.printf(m_arrayElements ? "." : ", ");
    if (m_exps[i]) {
      if (m_arrayElements) {
        ArrayPairExpressionPtr ap =
          dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
        cg.printf("set(%d, ", i);
        m_exps[i]->outputCPP(cg, ar);
        cg.printf(")");
      } else {
        m_exps[i]->outputCPP(cg, ar);
      }
    }
  }
  if (m_arrayElements) {
    cg.printf(".create()");
  }
}

bool ExpressionList::outputCPPTooManyArgsPre(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             const std::string &name) {
  if (m_outputCount >= 0) {
    cg.printf("invoke_too_many_args(\"%s\", (", name.c_str());
    int count = 0;
    for (unsigned int i = m_outputCount; i < m_exps.size(); i++) {
      ExpressionPtr exp = m_exps[i];
      if (exp && exp->hasEffect()) {
        if (count > 0) cg.printf(", ");
        exp->outputCPP(cg, ar);
        count++;
      }
    }
    if (count > 0) cg.printf(", ");
    cg.printf("%d), (", m_exps.size() - m_outputCount);
    return true;
  }
  return false;
}

void ExpressionList::outputCPPTooManyArgsPost(CodeGenerator &cg,
                                              AnalysisResultPtr ar,
                                              bool voidReturn) {
  ASSERT(m_outputCount >= 0);
  if (voidReturn) cg.printf(", null");
  cg.printf("))");
}

int ExpressionList::outputCPPControlledEvalOrderPre(CodeGenerator &cg,
                                                     AnalysisResultPtr ar,
                                                     ExpressionPtr obj /* = ExpressionPtr() */) {
  if (!controllingOrder()) return -1;
  uint oc = getOutputCount();
  cg.printf("(");
  int tempStart = tempOffset();
  if (obj && !obj->isScalar()) {
    cg.printf("assignCallTemp(%s%d, ", Option::EvalOrderTempPrefix,
              m_tempStart);
    obj->outputCPP(cg, ar);
    cg.printf("),");
  }
  for (unsigned int i = 0; i < oc; i++) {
    if (m_exps[i] && !m_exps[i]->isScalar()) {
      cg.printf("assignCallTemp(%s%d, ", Option::EvalOrderTempPrefix,
                tempStart + i);
      if (m_arrayElements) {
        ArrayPairExpressionPtr ap =
          dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
        ap->getValue()->outputCPP(cg, ar);
      } else {
        m_exps[i]->outputCPP(cg, ar);
      }
      cg.printf("),");
    }
  }
  return m_tempStart;
}
void ExpressionList::outputCPPControlledEvalOrderPost(CodeGenerator &cg,
                                                      AnalysisResultPtr ar) {
  if (!controllingOrder()) return;
  cg.printf(")");
}
