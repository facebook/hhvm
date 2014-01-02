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

#include "hphp/compiler/expression/expression_list.h"
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
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ExpressionList)),
    m_arrayElements(false), m_collectionType(0), m_kind(kind) {
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

void ExpressionList::setContext(Context context) {
  Expression::setContext(context);
  if (m_kind == ListKindParam && context & UnsetContext) {
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
  ArrayPairExpressionPtr ap = dynamic_pointer_cast<ArrayPairExpression>(exp);
  if (ap) {
    m_arrayElements = true;
  } else {
    assert(!m_arrayElements);
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
  if (m_arrayElements) {
    return isScalarArrayPairs();
  }

  if (m_kind == ListKindParam) {
    for (unsigned int i = m_exps.size(); i--; ) {
      if (m_exps[i] && !m_exps[i]->isScalar()) return false;
    }
    return true;
  } else if (!hasEffect()) {
    ExpressionPtr v(listValue());
    return v ? v->isScalar() : false;
  }
  return false;
}

bool ExpressionList::isNoObjectInvolved() const {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    TypePtr t = m_exps[i]->getActualType();
    if (t == nullptr || !t->isNoObjectInvolved())
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
  if (!m_arrayElements || m_collectionType) return false;
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

void ExpressionList::getOriginalStrings(std::vector<std::string> &strings) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ScalarExpressionPtr s = dynamic_pointer_cast<ScalarExpression>(m_exps[i]);
    strings.push_back(s->getOriginalString());
  }
}

bool
ExpressionList::flattenLiteralStrings(vector<ExpressionPtr> &literals) const {
  for (unsigned i = 0; i < m_exps.size(); i++) {
    ExpressionPtr e = m_exps[i];
    if (e->is(Expression::KindOfArrayPairExpression)) {
      ArrayPairExpressionPtr ap = dynamic_pointer_cast<ArrayPairExpression>(e);
      if (ap->getName()) return false;
      e = ap->getValue();
    }
    if (e->is(Expression::KindOfUnaryOpExpression)) {
      UnaryOpExpressionPtr unary = dynamic_pointer_cast<UnaryOpExpression>(e);
      if (unary->getOp() == T_ARRAY) {
        ExpressionListPtr el =
          dynamic_pointer_cast<ExpressionList>(unary->getExpression());
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
  if (m_arrayElements) {
    if (isScalarArrayPairs()) {
      ArrayInit init(m_exps.size());
      for (unsigned int i = 0; i < m_exps.size(); i++) {
        ArrayPairExpressionPtr exp =
          dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
        ExpressionPtr name = exp->getName();
        ExpressionPtr val = exp->getValue();
        if (!name) {
          Variant v;
          bool ret = val->getScalarValue(v);
          if (!ret) assert(false);
          init.set(v);
        } else {
          Variant n;
          Variant v;
          bool ret1 = name->getScalarValue(n);
          bool ret2 = val->getScalarValue(v);
          if (!(ret1 && ret2)) return false;
          init.set(n, v);
        }
      }
      value = Array(init.create());
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
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(e));
      if (u->getOp() == '(') {
        e = u->getExpression();
      }
    }
    if (e->is(Expression::KindOfBinaryOpExpression)) {
      BinaryOpExpressionPtr b
        (static_pointer_cast<BinaryOpExpression>(e));
      if (b->getOp() == '.') {
        if(!b->getExp1()->isArray() && !b->getExp2()->isArray()) {
          e = b->getExp1();
          el.insertElement(b->getExp2(), i + 1);
          continue;
        }
      }
    }
    i++;
  }
}

void ExpressionList::markParam(int p, bool noRefWrapper) {
  ExpressionPtr param = (*this)[p];
  if (param->hasContext(Expression::InvokeArgument)) {
    if (noRefWrapper) {
      param->setContext(Expression::NoRefWrapper);
    } else {
      param->clearContext(Expression::NoRefWrapper);
    }
  } else if (!param->hasContext(Expression::RefParameter)) {
    param->setContext(Expression::InvokeArgument);
    param->setContext(Expression::RefValue);
    if (noRefWrapper) {
      param->setContext(Expression::NoRefWrapper);
    }
  }
  param->setArgNum(p);
}

void ExpressionList::markParams(bool noRefWrapper) {
  for (int i = 0; i < getCount(); i++) {
    markParam(i, noRefWrapper);
  }
}

void ExpressionList::setCollectionType(int cType) {
  m_arrayElements = true;
  m_collectionType = cType;
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

string ExpressionList::getLiteralString() const {
  ExpressionPtr v(listValue());
  return v ? v->getLiteralString() : string("");
}

void ExpressionList::optimize(AnalysisResultConstPtr ar) {
  bool changed = false;
  size_t i = m_exps.size();
  if (m_kind != ListKindParam) {
    size_t skip = m_kind == ListKindLeft ? 0 : i - 1;
    while (i--) {
      if (i != skip) {
        ExpressionPtr &e = m_exps[i];
        if (!e || (e->getContainedEffects() == NoEffect && !e->isNoRemove())) {
          removeElement(i);
          changed = true;
        } else if (e->is(KindOfExpressionList)) {
          ExpressionListPtr el(static_pointer_cast<ExpressionList>(e));
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
      // don't convert an exp-list with type assertions to
      // a ListKindWrapped
      if (!isNoRemove()) {
        m_kind = ListKindWrapped;
      }
    } else if (m_kind == ListKindLeft && m_exps[0]->isScalar()) {
      ExpressionPtr e = m_exps[0];
      removeElement(0);
      addElement(e);
      m_kind = ListKindWrapped;
    }
  } else {
    bool isUnset = hasContext(UnsetContext) &&
      ar->getPhase() >= AnalysisResult::PostOptimize;
    int isGlobal = -1;
    while (i--) {
      ExpressionPtr &e = m_exps[i];
      if (isUnset) {
        if (e->is(Expression::KindOfSimpleVariable)) {
          SimpleVariablePtr var = dynamic_pointer_cast<SimpleVariable>(e);
          if (var->checkUnused()) {
            const std::string &name = var->getName();
            VariableTablePtr variables = getScope()->getVariables();
            if (!variables->isNeeded(name)) {
              removeElement(i);
              changed = true;
            }
          }
        }
      } else {
        bool global = e && (e->getContext() & Declaration) == Declaration;
        if (isGlobal < 0) {
          isGlobal = global;
        } else {
          always_assert(isGlobal == global);
        }
        if (isGlobal && e->isScalar()) {
          removeElement(i);
          changed = true;
        }
      }
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

ExpressionPtr ExpressionList::postOptimize(AnalysisResultConstPtr ar) {
  optimize(ar);
  return ExpressionPtr();
}

TypePtr ExpressionList::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                   bool coerce) {
  size_t size = m_exps.size();
  bool commaList = size && (m_kind != ListKindParam);
  size_t ix = m_kind == ListKindLeft ? 0 : size - 1;
  TypePtr tmp = commaList ? Type::Some : type;
  TypePtr ret = type;
  for (size_t i = 0; i < size; i++) {
    TypePtr t = i != ix ? tmp : type;
    bool c = coerce && (!commaList || i == ix);
    if (ExpressionPtr e = m_exps[i]) {
      e->inferAndCheck(ar, t, c);
      if (commaList && i == ix) {
        e->setExpectedType(TypePtr());
        ret = e->getActualType();
        if (e->getImplementedType()) {
          m_implementedType = e->getImplementedType();
        }
        if (!ret) ret = Type::Variant;
      }
    }
  }
  return ret;
}

bool ExpressionList::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  ExpressionListPtr l =
    static_pointer_cast<ExpressionList>(e);

  return m_arrayElements == l->m_arrayElements &&
         m_collectionType == l->m_collectionType &&
         m_kind == l->m_kind;
}

///////////////////////////////////////////////////////////////////////////////

void ExpressionList::outputCodeModel(CodeGenerator &cg) {
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ExpressionPtr exp = m_exps[i];
    if (exp) {
      cg.printExpression(exp, exp->hasContext(RefParameter));
    } else {
      cg.printNull();
    }
  }
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
  assert(m_arrayElements && !m_collectionType);
  std::set<string> keys;
  for (unsigned int i = 0; i < m_exps.size(); i++) {
    ArrayPairExpressionPtr ap =
      dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
    ExpressionPtr name = ap->getName();
    if (!name) return 0;
    Variant value;
    bool ret = name->getScalarValue(value);
    if (!ret) return 0;
    if (!value.isString()) return 0;
    String str = value.toString();
    if (str->isInteger()) return 0;
    string s(str.data(), str.size());
    keys.insert(s);
  }
  return keys.size();
}

