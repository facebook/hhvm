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

#include <compiler/expression/expression_list.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/analysis/function_scope.h>
#include <runtime/base/array/array_init.h>
#include <compiler/parser/parser.h>

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

  for (unsigned int i = m_exps.size(); i--; ) {
    if (m_exps[i] && !m_exps[i]->isScalar()) return false;
  }

  return true;
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
  if (!m_arrayElements) return false;
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
  if (m_arrayElements && isScalarArrayPairs()) {
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
        init.set(v);
      } else {
        Variant n;
        Variant v;
        bool ret1 = name->getScalarValue(n);
        bool ret2 = val->getScalarValue(v);
        if (!(ret1 && ret2)) return ExpressionPtr();
        init.set(n, v);
      }
    }
    value = Array(init.create());
    return true;
  }
  if (m_kind != ListKindParam && !hasEffect()) {
    int i = m_exps.size();
    if (i) {
      return m_exps[m_kind == ListKindLeft ? 0 : i-1]->getScalarValue(value);
    }
  }
  return false;
}

void ExpressionList::stripConcat() {
  ExpressionList &el = *this;
  for (int i = 0, s = el.getCount(); i < s; ) {
    ExpressionPtr &e = el[i];
    if (e->is(Expression::KindOfUnaryOpExpression)) {
      UnaryOpExpressionPtr u(boost::static_pointer_cast<UnaryOpExpression>(e));
      if (u->getOp() == '(') {
        e = u->getExpression();
      }
    }
    if (e->is(Expression::KindOfBinaryOpExpression)) {
      BinaryOpExpressionPtr b
        (boost::static_pointer_cast<BinaryOpExpression>(e));
      if (b->getOp() == '.') {
        e = b->getExp1();
        el.insertElement(b->getExp2(), i + 1);
        continue;
      }
    }
    i++;
  }
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
  param->setArgNum(p);
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

bool ExpressionList::kidUnused(int i) const {
  if (m_kind == ListKindParam) {
    return false;
  }

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
    ASSERT(false);
  } else {
    m_exps[n] = boost::dynamic_pointer_cast<Expression>(cp);
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

void ExpressionList::optimize(AnalysisResultConstPtr ar) {
  bool changed = false;
  size_t i = m_exps.size();
  if (m_kind != ListKindParam) {
    size_t skip = m_kind == ListKindLeft ? 0 : i - 1;
    while (i--) {
      if (i != skip) {
        ExpressionPtr &e = m_exps[i];
        if (!e || e->getContainedEffects() == NoEffect) {
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
      m_kind = ListKindWrapped;
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
          assert(isGlobal == global);
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
        ret = e->getExpectedType();
        if (!ret) ret = e->getActualType();
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
    m_kind == l->m_kind;
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

void ExpressionList::preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                                    int state) {
  if (hasCPPTemp() || m_kind == ListKindParam || m_arrayElements) {
    return;
  }
  int n = m_exps.size();
  int i = m_kind == ListKindLeft ? 0 : n - 1;
  if (m_exps[i]->hasCPPTemp() &&
      Type::SameType(getType(), m_exps[i]->getType())) {
    setUnused(true);
    outputCPP(cg, ar);
    cg_printf(";\n");
    m_cppTemp = m_exps[i]->cppTemp();
  }
  return Expression::preOutputStash(cg, ar, state);
}

bool ExpressionList::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int state) {
  if (m_kind == ListKindParam && !m_arrayElements) {
    return Expression::preOutputCPP(cg, ar, state|StashKidVars);
  }

  unsigned n = m_exps.size();
  bool inExpression = cg.inExpression();
  if (!inExpression && (state & FixOrder)) {
    return true;
  }

  cg.setInExpression(false);
  bool ret = false;
  if (m_arrayElements) {
    /*
     * would like to do:
     *  ret = Expression::preOutputCPP(cg, ar, state);
     * but icc has problems with the generated code.
     */
    ret = hasEffect();
  } else if (n > 1 && m_kind == ListKindLeft) {
    ret = true;
  } else {
    for (unsigned int i = 0; i < n; i++) {
      if (m_exps[i]->preOutputCPP(cg, ar, 0)) {
        ret = true;
        break;
      }
    }
  }

  if (!inExpression) return ret;

  cg.setInExpression(true);
  if (!ret) {
    if (state & FixOrder) {
      preOutputStash(cg, ar, state);
      return true;
    }
    return false;
  }

  cg.wrapExpressionBegin();
  if (m_arrayElements) {
    setCPPTemp(genCPPTemp(cg, ar));
    outputCPPInternal(cg, ar, true, true);
  } else {
    unsigned ix = m_kind == ListKindLeft ? 0 : n - 1;
    for (unsigned int i = 0; i < n; i++) {
      ExpressionPtr e = m_exps[i];
      e->preOutputCPP(cg, ar, i == ix ? state : 0);
      if (i != ix) {
        if (e->outputCPPUnneeded(cg, ar)) {
          cg_printf(";\n");
        }
        e->setCPPTemp("/**/");
      } else if (e->hasCPPTemp() && Type::SameType(e->getType(), getType())) {
        setCPPTemp(e->cppTemp());
      } else if (!i && n > 1) {
        e->Expression::preOutputStash(cg, ar, state | FixOrder);
        if (!(state & FixOrder)) {
          cg_printf("id(%s);\n", e->cppTemp().c_str());
        }
        setCPPTemp(e->cppTemp());
      }
    }
  }
  return true;
}

unsigned int ExpressionList::checkLitstrKeys() const {
  ASSERT(m_arrayElements);
  set<string> keys;
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

bool ExpressionList::hasNonArrayCreateValue(
  bool arrayElements /* = true */, unsigned int start /* = 0 */) const {
  for (unsigned int i = start; i < m_exps.size(); i++) {
    ExpressionPtr value = m_exps[i];
    if (arrayElements) {
      ArrayPairExpressionPtr ap =
        dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
      value = ap->getValue();
    }
    ASSERT(value);
    if (value->hasContext(RefValue)) {
      return true;
    }
  }
  return false;
}

void ExpressionList::outputCPPUniqLitKeyArrayInit(
  CodeGenerator &cg, AnalysisResultPtr ar, bool litstrKeys, int64 num,
  bool arrayElements /* = true */, unsigned int start /* = 0 */) {
  if (arrayElements) ASSERT(m_arrayElements);
  unsigned int n =  m_exps.size();
  cg_printf("array_createv%c(%lu, ", litstrKeys ? 's' : 'i', num);
  for (unsigned int i = start; i < n; i++) {
    if (ExpressionPtr exp = m_exps[i]) {
      ExpressionPtr name;
      ExpressionPtr value = exp;
      if (arrayElements) {
        ArrayPairExpressionPtr ap =
          dynamic_pointer_cast<ArrayPairExpression>(m_exps[i]);
        name = ap->getName();
        value = ap->getValue();
      }
      cg_printf("toVPOD(");
      value->outputCPP(cg, ar);
      cg_printf(")");
      if (name) {
        ASSERT(litstrKeys);
        cg_printf(", ");
        cg_printf("toSPOD(");
        name->outputCPP(cg, ar);
        cg_printf(")");
      }
      if (i < n-1) {
        cg_printf(", ");
      } else {
        cg_printf(")");
      }
    }
  }
}

bool ExpressionList::outputCPPArrayCreate(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          bool isVector,
                                          bool pre) {
  ASSERT(pre == !m_cppTemp.empty());
  if (!Option::GenArrayCreate || cg.getOutput() == CodeGenerator::SystemCPP) {
    return false;
  }
  if (hasNonArrayCreateValue()) return false;

  unsigned int n = isVector ? m_exps.size() : 0;
  bool uniqLitstrKeys = false;
  if (isVector) {
    n = m_exps.size();
  } else {
    n = checkLitstrKeys();
    if (n > 0 && n == m_exps.size()) uniqLitstrKeys = true;
  }
  if (isVector) {
    if (ar->m_arrayIntegerKeyMaxSize < (int)n) ar->m_arrayIntegerKeyMaxSize = n;
  } else if (uniqLitstrKeys) {
    if (ar->m_arrayLitstrKeyMaxSize < (int)n) ar->m_arrayLitstrKeyMaxSize = n;
  } else {
    return false;
  }
  if (pre) {
    Expression::preOutputCPP(cg, ar, StashKidVars);
    cg_printf("ArrayInit %s(", m_cppTemp.c_str());
    outputCPPUniqLitKeyArrayInit(cg, ar, uniqLitstrKeys, n);
    cg_printf(");\n");
  } else {
    outputCPPUniqLitKeyArrayInit(cg, ar, uniqLitstrKeys, n);
  }
  return true;
}

bool ExpressionList::outputCPPInternal(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       bool needed, bool pre) {
  bool needsComma = false;
  bool anyOutput = false;
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
    if (outputCPPArrayCreate(cg, ar, isVector, pre)) return true;
    cg_printf("ArrayInit");
    if (pre) {
      cg_printf(" %s", m_cppTemp.c_str());
    }
    cg_printf("(%d, %s)", m_exps.size(), isVector ? "true" : "false");
    if (pre) cg_printf(";\n");
    needsComma = true;
    anyOutput = true;
  }

  bool trailingComma = false;
  unsigned i = 0, s = m_exps.size();
  unsigned ix = m_kind == ListKindLeft ? 0 : s - 1;
  bool uniqueStringKeys =
    m_arrayElements && (checkLitstrKeys() == s); // TODO integer keys as well
  for ( ; i < s; i++) {
    if (ExpressionPtr exp = m_exps[i]) {
      if (pre) {
        exp->preOutputCPP(cg, ar, 0);
        cg_printf("%s", m_cppTemp.c_str());
      }
      if (needsComma) {
        cg_printf(m_arrayElements ? "." : ", ");
        trailingComma = true;
      }
      if (m_arrayElements) {
        ArrayPairExpressionPtr ap =
          dynamic_pointer_cast<ArrayPairExpression>(exp);
        if (ap->isRef()) {
          cg_printf("setRef(");
          // The value itself shouldn't be wrapped with ref() any more.
          ap->getValue()->setContext(NoRefWrapper);
        } else {
          cg_printf(uniqueStringKeys ? "add(" : "set(");
        }
        exp->outputCPP(cg, ar);
        cg_printf(")");
        if (pre) {
          cg_printf(";\n");
        }
        needsComma = true;
      } else if (m_kind != ListKindParam && (i != ix || !needed)) {
        needsComma = exp->outputCPPUnneeded(cg, ar);
      } else {
        exp->outputCPP(cg, ar);
        needsComma = true;
      }
      if (needsComma) {
        trailingComma = false;
        anyOutput = true;
      }
    }
  }
  if (trailingComma) {
    cg_printf("id(0)");
  }
  if (m_arrayElements && !pre) {
    cg_printf(".create()");
    anyOutput = true;
  }
  return anyOutput;
}

void ExpressionList::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  bool wrapped = m_kind == ListKindWrapped ||
    (m_kind == ListKindComma && getCount() > 1);
  if (wrapped) cg_printf("(");
  outputCPPInternal(cg, ar, true, false);
  if (wrapped) cg_printf(")");
}

bool ExpressionList::outputCPPUnneeded(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  bool inExpression = cg.inExpression();
  bool wrapped = false;
  if (!inExpression) {
    cg.setInExpression(true);
    wrapped = preOutputCPP(cg, ar, 0);
  }

  bool ret = outputCPPInternal(cg, ar, false, false);

  if (!inExpression) {
    if (wrapped) cg_printf(";\n");
    cg.wrapExpressionEnd();
    cg.setInExpression(inExpression);
  }
  return ret || wrapped;
}
