/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/expression_list.h"

#include <set>
#include <vector>

#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/compiler/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ExpressionList::ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                               ListKind kind)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ExpressionList))
  , m_elems_kind(ElemsKind::None)
  , m_argUnpack(false)
  , m_kind(kind)
{}

ExpressionPtr ExpressionList::clone() {
  ExpressionListPtr exp(new ExpressionList(*this));
  Expression::deepCopy(exp);
  assert(exp->m_argUnpack == this->m_argUnpack);
  exp->m_exps.clear();
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    exp->m_exps.push_back(Clone(m_exps[i]));
  }
  return exp;
}

void ExpressionList::setContext(Context context) {
  Expression::setContext(context);
  if (m_kind == ListKindParam && (context & UnsetContext)) {
    for (unsigned int i = m_exps.size(); i--; ) {
      if (m_exps[i]) {
        m_exps[i]->setContext(UnsetContext);
        m_exps[i]->setContext(LValue);
        m_exps[i]->setContext(NoLValueWrapper);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ExpressionList::addElement(ExpressionPtr exp) {
  auto ap = dynamic_pointer_cast<ArrayPairExpression>(exp);
  if (ap) {
    if (m_elems_kind == ElemsKind::None) m_elems_kind = ElemsKind::ArrayPairs;
  } else {
    assert(m_elems_kind == ElemsKind::None);
  }
  m_exps.push_back(exp);
}

void ExpressionList::insertElement(ExpressionPtr exp, int index /* = 0 */) {
  m_exps.insert(m_exps.begin() + index, exp);
}

void ExpressionList::removeElement(int index) {
  m_exps.erase(m_exps.begin() + index, m_exps.begin() + index + 1);
}

void ExpressionList::clearElements() {
  m_exps.clear();
}

bool ExpressionList::isRefable(bool checkError /* = false */) const {
  if (m_kind == ListKindWrapped || m_kind == ListKindLeft) {
    // Its legal to ref a list...
    if (checkError) return true;
    // ...but we shouldnt apply ref() to it unless the corresponding
    // arg is refable
    int ix = m_kind == ListKindLeft ? 0 : m_exps.size() - 1;
    return m_exps[ix]->isRefable(false);
  }
  return false;
}

bool ExpressionList::isScalar() const {
  if (m_elems_kind != ElemsKind::None) {
    return isScalarArrayPairs();
  }
  if (m_kind == ListKindParam) {
    for (unsigned int i = m_exps.size(); i--; ) {
      if (m_exps[i] && !m_exps[i]->isScalar()) return false;
    }
    return true;
  }
  if (!hasEffect()) {
    ExpressionPtr v(listValue());
    return v ? v->isScalar() : false;
  }
  return false;
}

bool ExpressionList::isNoObjectInvolved() const {
  for (const auto& exp : m_exps) {
    if (!exp->isScalar()) return false;
  }
  return true;
}

bool ExpressionList::containsDynamicConstant(AnalysisResultPtr ar) const {
  for (const auto& exp : m_exps) {
    if (exp->containsDynamicConstant(ar)) return true;
  }
  return false;
}

bool ExpressionList::isScalarArrayPairs() const {
  if (m_elems_kind != ElemsKind::ArrayPairs &&
      m_elems_kind != ElemsKind::Collection) {
    return false;
  }
  for (const auto& ape : m_exps) {
    auto exp = dynamic_pointer_cast<ArrayPairExpression>(ape);
    if (!exp || !exp->isScalarArrayPair()) {
      return false;
    }
  }
  return true;
}

void ExpressionList::getStrings(std::vector<std::string> &strings) {
  for (const auto& exp : m_exps) {
    auto s = dynamic_pointer_cast<ScalarExpression>(exp);
    strings.push_back(s->getString());
  }
}

bool ExpressionList::flattenLiteralStrings(
  std::vector<ExpressionPtr>& literals
) const {
  for (auto e : m_exps) {
    if (e->is(Expression::KindOfArrayPairExpression)) {
      auto ap = dynamic_pointer_cast<ArrayPairExpression>(e);
      if (ap->getName()) return false;
      e = ap->getValue();
    }
    if (e->is(Expression::KindOfUnaryOpExpression)) {
      auto unary = dynamic_pointer_cast<UnaryOpExpression>(e);
      if (unary->getOp() == T_ARRAY) {
        auto el = dynamic_pointer_cast<ExpressionList>(unary->getExpression());
        if (!el->flattenLiteralStrings(literals)) {
          return false;
        }
      }
    }
    else if (e->isLiteralString()) {
      literals.push_back(e);
    } else {
      return false;
    }
  }
  return true;
}

bool ExpressionList::getScalarValue(Variant &value) {
  if (m_elems_kind != ElemsKind::None) {
    if (isScalarArrayPairs()) {
      ArrayInit init(m_exps.size(), ArrayInit::Mixed{});
      for (const auto ape : m_exps) {
        auto exp = dynamic_pointer_cast<ArrayPairExpression>(ape);
        auto name = exp->getName();
        auto val = exp->getValue();
        if (!name) {
          Variant v;
          bool ret = val->getScalarValue(v);
          if (!ret) assert(false);
          init.append(v);
        } else {
          Variant n;
          Variant v;
          bool ret1 = name->getScalarValue(n);
          bool ret2 = val->getScalarValue(v);
          if (!(ret1 && ret2)) return false;
          init.setUnknownKey(n, v);
        }
      }
      value = init.toVariant();
      return true;
    }
    return false;
  }
  if (m_kind != ListKindParam && !hasEffect()) {
    ExpressionPtr v(listValue());
    return v ? v->getScalarValue(value) : false;
  }
  return false;
}

void ExpressionList::stripConcat() {
  ExpressionList &el = *this;
  for (int i = 0; i < el.getCount(); ) {
    ExpressionPtr &e = el[i];
    if (e->is(Expression::KindOfUnaryOpExpression)) {
      auto u = static_pointer_cast<UnaryOpExpression>(e);
      if (u->getOp() == '(') {
        e = u->getExpression();
      }
    }
    if (e->is(Expression::KindOfBinaryOpExpression)) {
      auto b = static_pointer_cast<BinaryOpExpression>(e);
      if (b->getOp() == '.') {
        if (!b->getExp1()->isArray() && !b->getExp2()->isArray()) {
          e = b->getExp1();
          el.insertElement(b->getExp2(), i + 1);
          continue;
        }
      }
    }
    i++;
  }
}

void ExpressionList::markParam(int p) {
  ExpressionPtr param = (*this)[p];
  if (param->hasContext(Expression::InvokeArgument)) {
  } else if (!param->hasContext(Expression::RefParameter)) {
    param->setContext(Expression::InvokeArgument);
    param->setContext(Expression::RefValue);
  }
  param->setArgNum(p);
}

void ExpressionList::markParams() {
  for (int i = 0; i < getCount(); i++) {
    markParam(i);
  }
}

void ExpressionList::setCollectionElems() {
  m_elems_kind = ElemsKind::Collection;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ExpressionPtr &ExpressionList::operator[](int index) {
  assert(index >= 0 && index < getCount());
  return m_exps[index];
}

void ExpressionList::analyzeProgram(AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (m_exps[i]) m_exps[i]->analyzeProgram(ar);
  }
}

bool ExpressionList::kidUnused(int i) const {
  if (m_kind == ListKindParam) {
    return false;
  }

  if (isUnused()) return true;

  if (m_kind == ListKindLeft) {
    return i != 0;
  }

  return (i + 1u) != m_exps.size();
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
    assert(false);
  } else {
    m_exps[n] = dynamic_pointer_cast<Expression>(cp);
  }
}

ExpressionPtr ExpressionList::listValue() const {
  if (size_t i = m_exps.size()) {
    if (m_kind == ListKindComma || m_kind == ListKindWrapped) {
      return m_exps[i-1];
    } else if (m_kind == ListKindLeft) {
      return m_exps[0];
    }
  }
  return ExpressionPtr();
}

bool ExpressionList::isLiteralString() const {
  ExpressionPtr v(listValue());
  return v ? v->isLiteralString() : false;
}

std::string ExpressionList::getLiteralString() const {
  ExpressionPtr v(listValue());
  return v ? v->getLiteralString() : std::string("");
}

void ExpressionList::optimize(AnalysisResultConstPtr ar) {
  bool changed = false;
  size_t i = m_exps.size();
  if (m_kind != ListKindParam) {
    size_t skip = m_kind == ListKindLeft ? 0 : i - 1;
    while (i--) {
      if (i != skip) {
        ExpressionPtr &e = m_exps[i];
        if (!e || (e->getContainedEffects() == NoEffect)) {
          removeElement(i);
          changed = true;
        } else if (e->is(KindOfExpressionList)) {
          auto el = static_pointer_cast<ExpressionList>(e);
          removeElement(i);
          for (size_t j = el->getCount(); j--; ) {
            insertElement((*el)[j], i);
          }
          changed = true;
        } else if (e->getLocalEffects() == NoEffect) {
          e = e->unneeded();
          // changed already handled by unneeded
        }
      }
    }
    if (m_exps.size() == 1) {
      m_kind = ListKindWrapped;
    } else if (m_kind == ListKindLeft && m_exps[0]->isScalar()) {
      ExpressionPtr e = m_exps[0];
      removeElement(0);
      addElement(e);
      m_kind = ListKindWrapped;
    }
  }
  if (changed) {
    getScope()->addUpdates(BlockScope::UseKindCaller);
  }
}

ExpressionPtr ExpressionList::preOptimize(AnalysisResultConstPtr ar) {
  optimize(ar);
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ExpressionList::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    if (i > 0) cg_printf(", ");
    ExpressionPtr exp = m_exps[i];
    if (exp) {
      if (exp->hasContext(RefParameter)) {
        cg_printf("&");
      }
      exp->outputPHP(cg, ar);
    }
  }
}

unsigned int ExpressionList::checkLitstrKeys() const {
  assert(m_elems_kind == ElemsKind::ArrayPairs);
  std::unordered_set<std::string> keys;
  for (const auto exp : m_exps) {
    auto ap = dynamic_pointer_cast<ArrayPairExpression>(exp);
    auto name = ap->getName();
    if (!name) return 0;
    Variant value;
    bool ret = name->getScalarValue(value);
    if (!ret) return 0;
    if (!value.isString()) return 0;
    auto str = value.toString();
    if (str.isInteger()) return 0;
    keys.emplace(str.data(), str.size());
  }
  return keys.size();
}
