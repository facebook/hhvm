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

#include "hphp/compiler/expression/expression.h"
#include <vector>
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/util/text-util.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/function_call.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/util/hash.h"
#include "hphp/runtime/base/array-iterator.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

#define DEC_EXPR_NAMES(x,t) #x,
const char *Expression::Names[] = {
  DECLARE_EXPRESSION_TYPES(DEC_EXPR_NAMES)
};
#undef DEC_EXPR_NAMES

const char* Expression::nameOfKind(Construct::KindOf kind) {
  assert(kind > Construct::KindOfExpression);
  auto const idx = static_cast<int32_t>(kind) -
    static_cast<int32_t>(Construct::KindOfExpression);
  assert(idx > 0);
  return Names[idx];
}

#define DEC_EXPR_CLASSES(x,t) Expression::t,
Expression::ExprClass Expression::Classes[] = {
  DECLARE_EXPRESSION_TYPES(DEC_EXPR_CLASSES)
};
#undef DEC_EXPR_CLASSES

Expression::Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS)
    : Construct(scope, r, kindOf), m_context(RValue),
      m_unused(false), m_error(0) {
}

ExpressionPtr Expression::replaceValue(ExpressionPtr rep, bool noWarn) {
  if (hasContext(Expression::RefValue) &&
      isRefable(true) && !rep->isRefable(true)) {
    /*
      An assignment isRefable, but the rhs may not be. Need this to
      prevent "bad pass by reference" errors.
    */
    auto el = std::make_shared<ExpressionList>(
      getScope(), getRange(), noWarn ?
      ExpressionList::ListKindWrappedNoWarn : ExpressionList::ListKindWrapped);
    el->addElement(rep);
    rep->clearContext(AssignmentRHS);
    rep = el;
  }
  if (rep->is(KindOfSimpleVariable) && !is(KindOfSimpleVariable)) {
    static_pointer_cast<SimpleVariable>(rep)->setAlwaysStash();
  }
  if (isUnpack()) rep->setIsUnpack();
  rep->copyContext(m_context & ~(DeadStore|AccessContext));

  if (rep->getScope() != getScope()) {
    rep->resetScope(getScope());
  }

  return rep;
}

void Expression::copyContext(int contexts) {
  unsigned val = contexts;
  while (val) {
    unsigned next = val & (val - 1);
    unsigned low = val ^ next; // lowest set bit
    setContext((Context)low);
    val = next;
  }
}

void Expression::clearContext() {
  unsigned val = m_context;
  while (val) {
    unsigned next = val & (val - 1);
    unsigned low = val ^ next; // lowest set bit
    clearContext((Context)low);
    val = next;
  }
}

void Expression::setArgNum(int n) {
  m_argNum = n;
  int kc = getKidCount();
  for (int i=0; i < kc; i++) {
    ExpressionPtr kid = getNthExpr(i);
    if (kid) {
      kid->setArgNum(n);
    }
  }
}

void Expression::deepCopy(ExpressionPtr exp) {
  exp->m_unused = false;
};

bool Expression::hasSubExpr(ExpressionPtr sub) const {
  if (this == sub.get()) return true;
  for (int i = getKidCount(); i--; ) {
    ExpressionPtr kid = getNthExpr(i);
    if (kid && kid->hasSubExpr(sub)) return true;
  }
  return false;
}

Expression::ExprClass Expression::getExprClass() const {
  assert(m_kindOf > Construct::KindOfExpression);
  auto const idx = static_cast<int32_t>(m_kindOf) -
    static_cast<int32_t>(Construct::KindOfExpression);
  assert(idx > 0);
  ExprClass cls = Classes[idx];
  if (cls == Update) {
    ExpressionPtr k = getStoreVariable();
    if (!k || !(k->hasContext(OprLValue))) cls = Expression::None;
  }
  return cls;
}

bool Expression::getEffectiveScalar(Variant &v) {
  if (is(KindOfExpressionList)) {
    ExpressionRawPtr sub = static_cast<ExpressionList*>(this)->listValue();
    if (!sub) return false;
    return sub->getEffectiveScalar(v);
  }
  return getScalarValue(v);
}

void Expression::addElement(ExpressionPtr /*exp*/) {
  assert(false);
}

void Expression::insertElement(ExpressionPtr /*exp*/, int /*index*/ /* = 0 */) {
  assert(false);
}

///////////////////////////////////////////////////////////////////////////////

bool Expression::IsIdentifier(const std::string &value) {
  if (value.empty()) {
    return false;
  }
  unsigned char ch = value[0];
  if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
      ch < '\x7f' && ch != '_') {
    return false;
  }
  for (unsigned int i = 1; i < value.size(); i++) {
    ch = value[i];
    if (((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
         (ch < '0' || ch > '9') && ch < '\x7f' && ch != '_')) {
      if (ch == '\\' && i < value.size() - 1 && value[i+1] != '\\') {
        continue;
      }
      return false;
    }
  }
  return true;
}

ExpressionPtr
Expression::MakeConstant(AnalysisResultConstRawPtr /*ar*/, BlockScopePtr scope,
                         const Location::Range& r, const std::string& value) {
  auto exp = std::make_shared<ConstantExpression>(scope, r, value, false);
  if (value == "true" || value == "false") {
  } else if (value == "null") {
  } else {
    assert(false);
  }
  return exp;
}

bool Expression::isArray() const {
  if (is(KindOfUnaryOpExpression)) {
    return static_cast<const UnaryOpExpression*>(this)->getOp() == T_ARRAY;
  }
  return false;
}

bool Expression::isCollection() const {
  if (is(KindOfBinaryOpExpression)) {
    return
      static_cast<const BinaryOpExpression*>(this)->getOp() == T_COLLECTION;
  }
  return false;
}

ExpressionPtr Expression::MakeScalarExpression(AnalysisResultConstRawPtr ar,
                                               BlockScopePtr scope,
                                               const Location::Range& r,
                                               const Variant& value) {
  if (value.isArray()) {
    auto el = std::make_shared<ExpressionList>(
      scope, r, ExpressionList::ListKindParam);

    if (value.isVecArray() || value.isKeyset()) {
      for (ArrayIter iter(value.toArray()); iter; ++iter) {
        ExpressionPtr v(MakeScalarExpression(ar, scope, r, iter.second()));
        if (!v) return ExpressionPtr();
        el->addElement(v);
      }
    } else {
      for (ArrayIter iter(value.toArray()); iter; ++iter) {
        ExpressionPtr k(MakeScalarExpression(ar, scope, r, iter.first()));
        ExpressionPtr v(MakeScalarExpression(ar, scope, r, iter.second()));
        if (!k || !v) return ExpressionPtr();
        auto ap = std::make_shared<ArrayPairExpression>(scope, r, k, v, false);
        el->addElement(ap);
      }
    }

    if (!el->getCount()) el.reset();

    auto const token = [&]{
      if (value.isPHPArray()) return T_ARRAY;
      if (value.isVecArray()) return T_VEC;
      if (value.isDict()) return T_DICT;
      if (value.isKeyset()) return T_KEYSET;
      always_assert(false);
    }();
    return std::make_shared<UnaryOpExpression>(scope, r, el, token, true);
  } else if (value.isNull()) {
    return MakeConstant(ar, scope, r, "null");
  } else if (value.isBoolean()) {
    return MakeConstant(ar, scope, r, value.toBoolean() ? "true" : "false");
  } else {
    return std::make_shared<ScalarExpression>(scope, r, value);
  }
}
