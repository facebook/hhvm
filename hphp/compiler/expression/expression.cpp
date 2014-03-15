/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#define DEC_EXPR_NAMES(x,t) #x
const char *Expression::Names[] = {
  DECLARE_EXPRESSION_TYPES(DEC_EXPR_NAMES)
};

#define DEC_EXPR_CLASSES(x,t) Expression::t
Expression::ExprClass Expression::Classes[] = {
  DECLARE_EXPRESSION_TYPES(DEC_EXPR_CLASSES)
};

Expression::Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS)
    : Construct(scope, loc), m_context(RValue), m_kindOf(kindOf),
      m_originalScopeSet(false), m_unused(false), m_canon_id(0), m_error(0),
      m_canonPtr() {
}

ExpressionPtr Expression::replaceValue(ExpressionPtr rep) {
  if (hasContext(Expression::RefValue) &&
      isRefable(true) && !rep->isRefable(true)) {
    /*
      An assignment isRefable, but the rhs may not be. Need this to
      prevent "bad pass by reference" errors.
    */
    ExpressionListPtr el(new ExpressionList(getScope(), getLocation(),
                                            ExpressionList::ListKindWrapped));
    el->addElement(rep);
    rep->clearContext(AssignmentRHS);
    rep = el;
  }
  if (rep->is(KindOfSimpleVariable) && !is(KindOfSimpleVariable)) {
    static_pointer_cast<SimpleVariable>(rep)->setAlwaysStash();
  }
  rep->copyContext(m_context & ~(DeadStore|AccessContext));
  if (TypePtr t1 = getType()) {
    if (TypePtr t2 = rep->getType()) {
      if (!Type::SameType(t1, t2)) {
        rep->setExpectedType(t1);
      }
    }
  }

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
  exp->m_actualType = m_actualType;
  exp->m_expectedType = m_expectedType;
  exp->m_implementedType = m_implementedType;
  exp->m_assertedType = m_assertedType;
  exp->m_canon_id = 0;
  exp->m_unused = false;
  exp->m_canonPtr.reset();
  exp->m_replacement.reset();
  exp->clearVisited();
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
  ExprClass cls = Classes[m_kindOf];
  if (cls == Update) {
    ExpressionPtr k = getStoreVariable();
    if (!k || !(k->hasContext(OprLValue))) cls = Expression::None;
  }
  return cls;
}

FileScopeRawPtr Expression::getUsedScalarScope(CodeGenerator& cg) {
  return cg.getLiteralScope() ?
    cg.getLiteralScope() : getFileScope();
}

bool Expression::getEffectiveScalar(Variant &v) {
  if (is(KindOfExpressionList)) {
    ExpressionRawPtr sub = static_cast<ExpressionList*>(this)->listValue();
    if (!sub) return false;
    return sub->getEffectiveScalar(v);
  }
  return getScalarValue(v);
}

void Expression::addElement(ExpressionPtr exp) {
  assert(false);
}

void Expression::insertElement(ExpressionPtr exp, int index /* = 0 */) {
  assert(false);
}

ExpressionPtr Expression::unneededHelper() {
  ExpressionListPtr elist = ExpressionListPtr
    (new ExpressionList(getScope(), getLocation(),
                        ExpressionList::ListKindWrapped));

  bool change = false;
  for (int i=0, n = getKidCount(); i < n; i++) {
    ExpressionPtr kid = getNthExpr(i);
    if (kid && kid->getContainedEffects()) {
      ExpressionPtr rep = kid->unneeded();
      if (rep != kid) change = true;
      if (rep->is(Expression::KindOfExpressionList)) {
        for (int j=0, m = rep->getKidCount(); j < m; j++) {
          elist->addElement(rep->getNthExpr(j));
        }
      } else {
        elist->addElement(rep);
      }
    }
  }

  if (change) {
    getScope()->addUpdates(BlockScope::UseKindCaller);
  }

  int n = elist->getCount();
  assert(n);
  if (n == 1) {
    return elist->getNthExpr(0);
  } else {
    return elist;
  }
}

ExpressionPtr Expression::unneeded() {
  if (getLocalEffects() || is(KindOfScalarExpression) || isNoRemove()) {
    return static_pointer_cast<Expression>(shared_from_this());
  }
  if (!getContainedEffects()) {
    getScope()->addUpdates(BlockScope::UseKindCaller);
    return ScalarExpressionPtr
      (new ScalarExpression(getScope(), getLocation(),
                            T_LNUMBER, string("0")));
  }

  return unneededHelper();
}

///////////////////////////////////////////////////////////////////////////////

bool Expression::IsIdentifier(const string &value) {
  if (value.empty()) {
    return false;
  }
  unsigned char ch = value[0];
  if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
      ch < '\x7f' && ch != '_') {
    return false;
  }
  for (unsigned int i = 1; i < value.size(); i++) {
    unsigned char ch = value[i];
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

TypePtr Expression::getType() {
  if (m_expectedType) return m_expectedType;
  if (m_actualType) return m_actualType;
  return Type::Any;
}

TypePtr Expression::getGenType() {
  if (m_expectedType) return m_expectedType;
  if (m_implementedType) return m_implementedType;
  if (m_actualType) return m_actualType;
  return Type::Any;
}

TypePtr Expression::getCPPType() {
  if (m_implementedType) return m_implementedType;
  if (m_actualType) return m_actualType;
  return Type::Variant;
}

TypePtr Expression::propagateTypes(AnalysisResultConstPtr ar, TypePtr inType) {
  ExpressionPtr e = getCanonTypeInfPtr();
  TypePtr ret = inType;

  while (e) {
    if (e->getAssertedType() && !getAssertedType()) {
      setAssertedType(e->getAssertedType());
    }
    TypePtr inferred = Type::Inferred(ar, ret, e->m_actualType);
    if (!inferred) {
      break;
    }
    ret = inferred;
    e = e->getCanonTypeInfPtr();
  }

  return ret;
}

void Expression::analyzeProgram(AnalysisResultPtr ar) {
}

BlockScopeRawPtr Expression::getOriginalScope() {
  if (!m_originalScopeSet) {
    m_originalScopeSet = true;
    m_originalScope = getScope();
  }
  return m_originalScope;
}

void Expression::setOriginalScope(BlockScopeRawPtr scope) {
  m_originalScope = scope;
  m_originalScopeSet = true;
}

ClassScopeRawPtr Expression::getOriginalClass() {
  BlockScopeRawPtr scope = getOriginalScope();
  return scope ? scope->getContainingClass() : ClassScopeRawPtr();
}

FunctionScopeRawPtr Expression::getOriginalFunction() {
  BlockScopeRawPtr scope = getOriginalScope();
  return scope ? scope->getContainingFunction() : FunctionScopeRawPtr();
}

void Expression::resetTypes() {
  m_actualType     .reset();
  m_expectedType   .reset();
  m_implementedType.reset();
}

TypePtr Expression::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());
  assert(type);
  resetTypes();
  TypePtr actualType = inferTypes(ar, type, coerce);
  if (type->is(Type::KindOfSome) || type->is(Type::KindOfAny)) {
    m_actualType = actualType;
    m_expectedType.reset();
    return actualType;
  }
  return checkTypesImpl(ar, type, actualType, coerce);
}

TypePtr Expression::checkTypesImpl(AnalysisResultConstPtr ar,
                                   TypePtr expectedType,
                                   TypePtr actualType, bool coerce) {
  TypePtr ret;
  actualType = propagateTypes(ar, actualType);
  assert(actualType);
  if (coerce) {
    ret = Type::Coerce(ar, expectedType, actualType);
    setTypes(ar, actualType, expectedType);
  } else {
    ret = Type::Intersection(ar, actualType, expectedType);
    setTypes(ar, actualType, ret);
  }
  assert(ret);
  return ret;
}

void Expression::setTypes(AnalysisResultConstPtr ar, TypePtr actualType,
                          TypePtr expectedType) {
  assert(actualType);
  assert(expectedType);

  m_actualType = actualType;
  if (!expectedType->is(Type::KindOfAny) &&
      !expectedType->is(Type::KindOfSome)) {
    // store the expected type if it is not Any nor Some,
    // regardless of the actual type
    m_expectedType = expectedType;
  } else {
    m_expectedType.reset();
  }

  // This is a special case where Type::KindOfObject means any object.
  if (m_expectedType && m_expectedType->is(Type::KindOfObject) &&
      !m_expectedType->isSpecificObject() &&
      m_actualType->isSpecificObject()) {
    m_expectedType.reset();
  }

  if (m_actualType->isSpecificObject()) {
    std::const_pointer_cast<AnalysisResult>(ar)->
      addClassDependency(getFileScope(), m_actualType->getName());
  }
}

void Expression::setDynamicByIdentifier(AnalysisResultPtr ar,
                                        const std::string &value) {
  string id = toLower(value);
  size_t c = id.find("::");
  FunctionScopePtr fi;
  ClassScopePtr ci;
  if (c != 0 && c != string::npos && c+2 < id.size()) {
    string cl = id.substr(0, c);
    string fn = id.substr(c+2);
    if (IsIdentifier(cl) && IsIdentifier(fn)) {
      ci = ar->findClass(cl);
      if (ci) {
        fi = ci->findFunction(ar, fn, false);
        if (fi) fi->setDynamic();
      }
    }
  } else if (IsIdentifier(id)) {
    fi = ar->findFunction(id);
    if (fi) fi->setDynamic();
    ClassScopePtr ci = ar->findClass(id, AnalysisResult::MethodName);
    if (ci) {
      fi = ci->findFunction(ar, id, false);
      if (fi) fi->setDynamic();
    }
  }
}

bool Expression::CheckNeededRHS(ExpressionPtr value) {
  bool needed = true;
  always_assert(value);
  while (value->is(KindOfAssignmentExpression)) {
    value = dynamic_pointer_cast<AssignmentExpression>(value)->getValue();
  }
  if (value->isScalar()) {
    needed = false;
  } else {
    TypePtr type = value->getType();
    if (type && (type->is(Type::KindOfSome) || type->is(Type::KindOfAny))) {
      type = value->getActualType();
    }
    if (type && type->isNoObjectInvolved()) needed = false;
  }
  return needed;
}

bool Expression::CheckNeeded(ExpressionPtr variable, ExpressionPtr value) {
  // if the value may involve object, consider the variable as "needed"
  // so that objects are not destructed prematurely.
  bool needed = true;
  if (value) needed = CheckNeededRHS(value);
  if (variable->is(Expression::KindOfSimpleVariable)) {
    SimpleVariablePtr var =
      dynamic_pointer_cast<SimpleVariable>(variable);
    const std::string &name = var->getName();
    VariableTablePtr variables = var->getScope()->getVariables();
    if (needed) {
      variables->addNeeded(name);
    } else {
      needed = variables->isNeeded(name);
    }
  }
  return needed;
}

bool Expression::CheckVarNR(ExpressionPtr value,
                            TypePtr expectedType /* = TypePtr */) {
  if (!expectedType) expectedType = value->getExpectedType();
  return (!value->hasContext(Expression::RefValue) &&
          expectedType && expectedType->is(Type::KindOfVariant) &&
          (value->getCPPType()->is(Type::KindOfArray) ||
           value->getCPPType()->is(Type::KindOfString) ||
           value->getCPPType()->is(Type::KindOfObject) ||
           value->getCPPType()->isPrimitive() ||
           value->isScalar()));
}

TypePtr Expression::inferAssignmentTypes(AnalysisResultPtr ar, TypePtr type,
                                         bool coerce, ExpressionPtr variable,
                                         ExpressionPtr
                                         value /* =ExpressionPtr() */) {
  assert(type);
  TypePtr ret = type;
  if (value) {
    ret = value->inferAndCheck(ar, Type::Some, false);
    if (value->isLiteralNull()) {
      ret = Type::Null;
    }
    assert(ret);
  }

  BlockScopePtr scope = getScope();
  if (variable->is(Expression::KindOfConstantExpression)) {
    // ...as in ClassConstant statement
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(variable);
    BlockScope *defScope = nullptr;
    std::vector<std::string> bases;
    scope->getConstants()->check(getScope(), exp->getName(), ret,
                                 true, ar, variable,
                                 bases, defScope);
  }

  m_implementedType.reset();
  TypePtr vt = variable->inferAndCheck(ar, ret, true);
  if (!coerce && type->is(Type::KindOfAny)) {
    ret = vt;
  } else {
    TypePtr it = variable->getCPPType();
    if (!Type::SameType(it, ret)) {
      m_implementedType = it;
    }
  }

  if (value) {
    TypePtr vat(value->getActualType());
    TypePtr vet(value->getExpectedType());
    TypePtr vit(value->getImplementedType());
    if (vat && !vet && vit &&
        Type::IsMappedToVariant(vit) &&
        Type::HasFastCastMethod(vat)) {
      value->setExpectedType(vat);
    }
  }

  return ret;
}

ExpressionPtr Expression::MakeConstant(AnalysisResultConstPtr ar,
                                       BlockScopePtr scope,
                                       LocationPtr loc,
                                       const std::string &value) {
  ConstantExpressionPtr exp(new ConstantExpression(
                              scope, loc,
                              value, false));
  if (value == "true" || value == "false") {
    if (ar->getPhase() >= AnalysisResult::PostOptimize) {
      exp->m_actualType = Type::Boolean;
    }
  } else if (value == "null") {
    if (ar->getPhase() >= AnalysisResult::PostOptimize) {
      exp->m_actualType = Type::Variant;
    }
  } else {
    assert(false);
  }
  return exp;
}

void Expression::CheckPassByReference(AnalysisResultPtr ar,
                                      ExpressionPtr param) {
  if (param->hasContext(Expression::RefValue) &&
      !param->isRefable(true)) {
    param->setError(Expression::BadPassByRef);
    Compiler::Error(Compiler::BadPassByReference, param);
  }
}

unsigned Expression::getCanonHash() const {
  int64_t val = hash_int64(getKindOf());
  for (int i = getKidCount(); i--; ) {
    ExpressionPtr k = getNthExpr(i);
    if (k) {
      val = hash_int64(val ^ (((int64_t)k->getKindOf()<<32)+k->getCanonID()));
    }
  }

  return (unsigned)val ^ (unsigned)(val >> 32);
}

bool Expression::canonCompare(ExpressionPtr e) const {
  if (e->getKindOf() != getKindOf()) {
    return false;
  }

  int kk = getKidCount();
  if (kk != e->getKidCount()) {
    return false;
  }

  for (int i = kk; i--; ) {
    ExpressionPtr k1 = getNthExpr(i);
    ExpressionPtr k2 = e->getNthExpr(i);

    if (k1 != k2) {
      if (!k1 || !k2) {
        return false;
      }
      if (k1->getCanonID() != k2->getCanonID()) {
        return false;
      }
    }
  }

  return true;
}

bool Expression::equals(ExpressionPtr other) {
  if (!other) return false;

  // So that we can leverage canonCompare()
  setCanonID(0);
  other->setCanonID(0);

  if (other->getKindOf() != getKindOf()) {
    return false;
  }

  int nKids = getKidCount();
  if (nKids != other->getKidCount()) {
    return false;
  }

  for (int i = 0; i < nKids; i++) {
    ExpressionPtr thisKid = getNthExpr(i);
    ExpressionPtr otherKid = other->getNthExpr(i);

    if (!thisKid || !otherKid) {
      if (thisKid == otherKid) continue;
      return false;
    }
    if (!thisKid->equals(otherKid)) {
      return false;
    }
  }

  return canonCompare(other);
}

ExpressionPtr Expression::getCanonTypeInfPtr() const {
  if (!m_canonPtr) return ExpressionPtr();
  if (!(m_context & (LValue|RefValue|UnsetContext|DeepReference))) {
    return m_canonPtr;
  }
  if (!hasAnyContext(AccessContext|ObjectContext) ||
      !m_canonPtr->getActualType()) {
    return ExpressionPtr();
  }
  switch (m_canonPtr->getActualType()->getKindOf()) {
  case Type::KindOfArray:
    {
      if (!hasContext(AccessContext)) break;
      if (m_canonPtr->getAssertedType()) return m_canonPtr;
      if (!is(Expression::KindOfSimpleVariable)) break;
      SimpleVariableConstPtr sv(
        static_pointer_cast<const SimpleVariable>(shared_from_this()));
      if (sv->couldBeAliased()) return ExpressionPtr();
      if (hasContext(LValue) &&
          !(m_context & (RefValue | UnsetContext | DeepReference))) {
        return m_canonPtr;
      }
    }
    break;
  case Type::KindOfObject:
    {
      if (!hasContext(ObjectContext)) break;
      if (m_canonPtr->getAssertedType()) return m_canonPtr;
      if (!is(Expression::KindOfSimpleVariable)) break;
      SimpleVariableConstPtr sv(
        static_pointer_cast<const SimpleVariable>(shared_from_this()));
      if (sv->couldBeAliased()) return ExpressionPtr();
      if (hasContext(LValue) &&
          !(m_context & (RefValue | UnsetContext | DeepReference))) {
        return m_canonPtr;
      }
    }
  default:
    break;
  }
  return ExpressionPtr();
}

ExpressionPtr Expression::fetchReplacement() {
  ExpressionPtr t = m_replacement;
  m_replacement.reset();
  return t;
}

void Expression::computeLocalExprAltered() {
  // if no kids, do nothing
  if (getKidCount() == 0) return;

  bool res = false;
  for (int i = 0; i < getKidCount(); i++) {
    ExpressionPtr k = getNthExpr(i);
    if (k) {
      k->computeLocalExprAltered();
      res |= k->isLocalExprAltered();
    }
  }
  if (res) {
    setLocalExprAltered();
  }
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

bool Expression::isUnquotedScalar() const {
  if (!is(KindOfScalarExpression)) return false;
  return !((ScalarExpression*)this)->isQuoted();
}

ExpressionPtr Expression::MakeScalarExpression(AnalysisResultConstPtr ar,
                                               BlockScopePtr scope,
                                               LocationPtr loc,
                                               const Variant& value) {
  if (value.isArray()) {
    ExpressionListPtr el(new ExpressionList(scope, loc,
                                            ExpressionList::ListKindParam));

    for (ArrayIter iter(value.toArray()); iter; ++iter) {
      ExpressionPtr k(MakeScalarExpression(ar, scope, loc, iter.first()));
      ExpressionPtr v(MakeScalarExpression(ar, scope, loc, iter.second()));
      if (!k || !v) return ExpressionPtr();
      ArrayPairExpressionPtr ap(
        new ArrayPairExpression(scope, loc, k, v, false));
      el->addElement(ap);
    }
    if (!el->getCount()) el.reset();
    return ExpressionPtr(
      new UnaryOpExpression(scope, loc, el, T_ARRAY, true));
  } else if (value.isNull()) {
    return MakeConstant(ar, scope, loc, "null");
  } else if (value.isBoolean()) {
    return MakeConstant(ar, scope, loc, value.toBoolean() ? "true" : "false");
  } else {
    return ScalarExpressionPtr
      (new ScalarExpression(scope, loc, value));
  }
}

///////////////////////////////////////////////////////////////////////////////

void Expression::collectCPPTemps(ExpressionPtrVec &collection) {
  if (isChainRoot()) {
    collection.push_back(static_pointer_cast<Expression>(shared_from_this()));
  } else {
    for (int i = 0; i < getKidCount(); i++) {
      ExpressionPtr kid = getNthExpr(i);
      if (kid) kid->collectCPPTemps(collection);
    }
  }
}

void Expression::disableCSE() {
  ExpressionPtrVec v;
  collectCPPTemps(v);
  ExpressionPtrVec::iterator it(v.begin());
  for (; it != v.end(); ++it) {
    ExpressionPtr p(*it);
    p->clearChainRoot();
  }
}

bool Expression::hasChainRoots() {
  ExpressionPtrVec v;
  collectCPPTemps(v);
  return !v.empty();
}

bool Expression::GetCseTempInfo(
    AnalysisResultPtr ar,
    ExpressionPtr p,
    TypePtr &t) {
  assert(p);
  switch (p->getKindOf()) {
  case Expression::KindOfArrayElementExpression:
    {
      ArrayElementExpressionPtr ap(
          static_pointer_cast<ArrayElementExpression>(p));
      ExpressionPtr var(ap->getVariable());

      TypePtr srcType, dstType;
      bool needsCast =
        var->getTypeCastPtrs(ar, srcType, dstType);

      TypePtr testType(needsCast ? dstType : srcType);
      if (testType) {
        t = testType;
        return !testType->is(Type::KindOfArray);
      }

      return true;
    }
    break;
  default:
    break;
  }
  return true;
}

ExpressionPtr Expression::getNextCanonCsePtr() const {

  bool dAccessCtx =
    hasContext(AccessContext);
  bool dLval =
    hasContext(LValue);
  bool dExistCtx =
    hasContext(ExistContext);
  bool dUnsetCtx =
    hasContext(UnsetContext);

  bool dGlobals = false;
  if (is(KindOfArrayElementExpression)) {
    ArrayElementExpressionConstPtr a(
        static_pointer_cast<const ArrayElementExpression>(
          shared_from_this()));
    dGlobals = a->isSuperGlobal() || a->isDynamicGlobal();
  }

  // see rules below - no hope to find CSE candidate
  if (dExistCtx || dUnsetCtx || dGlobals || (!dAccessCtx && dLval)) {
    return ExpressionPtr();
  }

  KindOf dKindOf = getKindOf();

  ExpressionPtr match;
  ExpressionPtr p(getCanonLVal());
  for (; p; p = p->getCanonLVal()) {
    // check if p is a suitable candidate for CSE of
    // downstream. the rules are:
    // A) rvals can always be CSE-ed regardless of access context,
    //    except for unset context, which it never can be CSE-ed for
    // B) lvals can only be CSE-ed if in AccessContext
    // C) rvals and lvals cannot be CSE-ed for each other
    // D) for now, ExistContext is not optimized
    // E) no CSE for $GLOBALS[...]
    // F) node types need to match

    bool pLval = p->hasContext(LValue);
    KindOf pKindOf = p->getKindOf();

    if (dKindOf != pKindOf) continue;

    if (dLval) {
      assert(dAccessCtx);
      bool pAccessCtx = p->hasContext(AccessContext);
      if (pLval && pAccessCtx) {
        // match found
        match = p;
        break;
      }
    } else {
      bool pExistCtx = p->hasContext(ExistContext);
      bool pUnsetCtx = p->hasContext(UnsetContext);
      if (!pLval && !pExistCtx && !pUnsetCtx) {
        // match found
        match = p;
        break;
      }
    }
  }

  return match;
}

ExpressionPtr Expression::getCanonCsePtr() const {
  ExpressionPtr next(getNextCanonCsePtr());
  if (next) {
    if (next->isChainRoot()) return next;
    return next->getCanonCsePtr();
  }
  return ExpressionPtr();
}

bool Expression::getTypeCastPtrs(
    AnalysisResultPtr ar, TypePtr &srcType, TypePtr &dstType) {
  srcType = m_actualType;
  dstType = m_expectedType;
  if (m_implementedType && srcType &&
      !Type::SameType(m_implementedType, srcType)) {
    srcType = m_implementedType;
  }
  if (!srcType && dstType && Type::IsCastNeeded(ar, Type::Variant, dstType)) {
    srcType = Type::Variant;
    return true;
  }
  return dstType && srcType && ((m_context & LValue) == 0) &&
      Type::IsCastNeeded(ar, srcType, dstType);
}
