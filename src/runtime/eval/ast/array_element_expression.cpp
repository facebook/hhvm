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

#include <runtime/eval/ast/array_element_expression.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/scalar_value_expression.h>
#include <runtime/eval/ast/temp_expression.h>
#include <runtime/eval/ast/static_member_expression.h>
#include <runtime/eval/ast/name.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {

///////////////////////////////////////////////////////////////////////////////

#define I(exp) (static_cast<TempExpression *>(exp.get())->getExp())
#define VC(exp) (VariableExpression::GetVariableByRefCheck(env, exp))
#define VE(exp) (VariableExpression::GetVariableByRef(env, exp))
#define S(exp) (ScalarValueExpression::GetScalarValueByRef(exp))
#define D(exp) (exp->eval(env))
#define DE(exp) (exp->evalExist(env))


void ArrayElementExpression::setArrayElementKindOf() {
  if (m_arr->isKindOf(Expression::KindOfVariableExpression)) {
    if (!m_idx) {
      m_arrayElementKindOf = VZN;
    } else {
      ASSERT(m_idx->isKindOf(KindOfTempExpression));
      Expression *idxExp = I(m_idx);
      switch (idxExp->getKindOf()) {
      case KindOfVariableExpression:
        m_arrayElementKindOf = VVN;
        break;
      case KindOfScalarValueExpression:
        m_arrayElementKindOf = VSN;
        break;
      default:
        m_arrayElementKindOf = VDR;
        break;
      }
    }
  } else {
    if (!m_idx) {
      m_arrayElementKindOf = DZN;
    } else {
      ASSERT(m_idx->isKindOf(KindOfTempExpression));
      Expression *idxExp = I(m_idx);
      switch (idxExp->getKindOf()) {
      case KindOfVariableExpression:
        m_arrayElementKindOf = DVN;
        break;
      case KindOfScalarValueExpression:
        m_arrayElementKindOf = DSN;
        break;
      default:
        m_arrayElementKindOf = DDN;
        break;
      }
    }
  }
}

ArrayElementExpression::ArrayElementExpression(EXPRESSION_ARGS,
                                               ExpressionPtr arr,
                                               ExpressionPtr idx)
  : LvalExpression(KindOfArrayElementExpression, EXPRESSION_PASS),
  m_arr(arr), m_idx(idx) {
  setArrayElementKindOf();
}

Expression *ArrayElementExpression::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_arr);
  Eval::optimize(env, m_idx);
  setArrayElementKindOf();
  return NULL;
}

Variant ArrayElementExpression::eval(VariableEnvironment &env) const {
  Variant a, i;
  Variant *ap, *ip;
  switch (m_arrayElementKindOf) {
  case VZN:
  case DZN:
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  case VVN:
    ap = &VC(m_arr.get());
    ip = &VC(I(m_idx));
    break;
  case VSN:
    ap = &VC(m_arr.get());
    ip = &S(I(m_idx));
    break;
  case DVN:
    ap = new(&a) Variant(D(m_arr));
    ip = &VC(I(m_idx));
    break;
  case DSN:
    ap = new(&a) Variant(D(m_arr));
    ip = &S(I(m_idx));
    break;
  case DDN:
    ap = new(&a) Variant(D(m_arr));
    ip = new(&i) Variant(D(m_idx));
    break;
  case VDR: // reverse
    ip = new(&i) Variant(D(m_idx));
    ap = &VC(m_arr.get());
    break;
  default:
    assert(false);
  }
  SET_LINE;
  return ap->rvalAt(*ip, AccessFlags::Error);
}

Variant ArrayElementExpression::evalExist(VariableEnvironment &env) const {
  Variant a, i;
  Variant *ap, *ip;
  switch (m_arrayElementKindOf) {
  case VZN:
  case DZN:
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in isset/empty context");
  case VVN:
    ap = &VE(m_arr.get());
    ip = &VC(I(m_idx));
    break;
  case VSN:
    ap = &VE(m_arr.get());
    ip = &S(I(m_idx));
    break;
  case DVN:
    ap = new(&a) Variant(DE(m_arr));
    ip = &VC(I(m_idx));
    break;
  case DSN:
    ap = new(&a) Variant(DE(m_arr));
    ip = &S(I(m_idx));
    break;
  case DDN:
    ap = new(&a) Variant(DE(m_arr));
    ip = new(&i) Variant(D(m_idx));
    break;
  case VDR: // reverse
    ip = new(&i) Variant(D(m_idx));
    ap = &VE(m_arr.get());
    break;
  default:
    assert(false);
  }
  SET_LINE;
  return ap->rvalAt(*ip);
}

bool ArrayElementExpression::exist(VariableEnvironment &env, int op) const {
  Variant a, i;
  Variant *ap, *ip;
  switch (m_arrayElementKindOf) {
  case VZN:
  case DZN:
    SET_LINE;
    throw InvalidOperandException("Cannot use [] in read context");
  case VVN:
    ap = &VE(m_arr.get());
    ip = &VC(I(m_idx));
    break;
  case VSN:
    ap = &VE(m_arr.get());
    ip = &S(I(m_idx));
    break;
  case DVN:
    ap = new(&a) Variant(DE(m_arr));
    ip = &VC(I(m_idx));
    break;
  case DSN:
    ap = new(&a) Variant(DE(m_arr));
    ip = &S(I(m_idx));
    break;
  case DDN:
    ap = new(&a) Variant(DE(m_arr));
    ip = new(&i) Variant(D(m_idx));
    break;
  case VDR: // reverse
    ip = new(&i) Variant(D(m_idx));
    ap = &VE(m_arr.get());
    break;
  default:
    assert(false);
  }
  SET_LINE;
  if (op == T_ISSET) {
    return HPHP::isset(*ap, *ip);
  }
  ASSERT(op == T_EMPTY);
  return HPHP::empty(*ap, *ip);
}

Variant &ArrayElementExpression::lval(VariableEnvironment &env) const {
  const LvalExpression *larr = m_arr->toLval();
  ASSERT(larr);
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant i;
  Variant *ap, *ip;
  switch (m_arrayElementKindOf) {
  case VZN:
    ap = &VE(m_arr.get());
    ip = NULL;
    break;
  case DZN:
    ap = &larr->lval(env);
    ip = NULL;
    break;
  case VVN:
    ap = &VE(m_arr.get());
    ip = &VC(I(m_idx));
    break;
  case VSN:
    ap = &VE(m_arr.get());
    ip = &S(I(m_idx));
    break;
  case DVN:
    ap = &larr->lval(env);
    ip = &VC(I(m_idx));
    break;
  case DSN:
    ap = &larr->lval(env);
    ip = &S(I(m_idx));
    break;
  case DDN:
    ap = &larr->lval(env);
    ip = new(&i) Variant(D(m_idx));
    break;
  case VDR: // no reverse
    ap = &VE(m_arr.get());
    ip = new(&i) Variant(D(m_idx));
    break;
  default:
    assert(false);
  }
  if (ip) {
    if (UNLIKELY(!ip->canBeValidKey())) {
      SET_LINE;
    }
    return ap->lvalAt(*ip);
  } else {
    return ap->lvalAt();
  }
}

bool ArrayElementExpression::weakLval(VariableEnvironment &env,
                                      Variant *&v) const {
  if (!m_idx) {
    SET_LINE;
    throw InvalidOperandException("Cannot unset array append");
  }
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant *arr;
  bool ok = larr->weakLval(env, arr);
  Variant i;
  Variant *ip;
  switch (m_arrayElementKindOf) {
  case VVN: case DVN:
    ip = &VC(I(m_idx));
    break;
  case VSN: case DSN:
    ip = &S(I(m_idx));
    break;
  case DDN: case VDR:
    ip = new(&i) Variant(D(m_idx));
    break;
  default:
    assert(false);
  }
  if (!ok || !arr->is(KindOfArray)) {
    return false;
  }
  Array &a(arr->toArrRef());
  SET_LINE;
  if (a.exists(*ip)) {
    v = &a.lvalAt(*ip);
    return true;
  }
  return false;
}

Variant ArrayElementExpression::refval(VariableEnvironment &env,
                                       int strict /* = 2 */) const {
  if (m_idx) {
    Variant arr(m_arr->refval(env, strict));
    Variant i;
    Variant *ip;
    switch (m_arrayElementKindOf) {
    case VVN: case DVN:
      ip = &VC(I(m_idx));
      break;
    case VSN: case DSN:
      ip = &S(I(m_idx));
      break;
    case DDN: case VDR:
      ip = new(&i) Variant(D(m_idx));
      break;
    default:
      assert(false);
    }
    SET_LINE;
    return strongBind(arr.refvalAt(*ip));
  } else {
    SET_LINE;
    return strongBind(lval(env));
  }
}

Variant ArrayElementExpression::set(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant &arr = larr->lval(env);
  if (m_idx) {
    Variant i;
    Variant *ip;
    switch (m_arrayElementKindOf) {
    case VVN: case DVN:
      ip = &VC(I(m_idx));
      break;
    case VSN: case DSN:
      ip = &S(I(m_idx));
      break;
    case DDN: case VDR:
      ip = new(&i) Variant(D(m_idx));
      break;
    default:
      assert(false);
    }
    SET_LINE;
    return arr.set(*ip, val);
  } else {
    SET_LINE;
    return arr.append(val);
  }
}

Variant ArrayElementExpression::setRef(VariableEnvironment &env, CVarRef val)
  const {
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant &arr = larr->lval(env);
  if (m_idx) {
    Variant i;
    Variant *ip;
    switch (m_arrayElementKindOf) {
    case VVN: case DVN:
      ip = &VC(I(m_idx));
      break;
    case VSN: case DSN:
      ip = &S(I(m_idx));
      break;
    case DDN: case VDR:
      ip = new(&i) Variant(D(m_idx));
      break;
    default:
      assert(false);
    }
    SET_LINE;
    return arr.setRef(*ip, val);
  } else {
    SET_LINE;
    return arr.appendRef(val);
  }
}

void ArrayElementExpression::unset(VariableEnvironment &env) const {
  if (!m_idx) {
    throw InvalidOperandException("Cannot unset array append");
  }
  const LvalExpression *larr = m_arr->toLval();
  if (!larr) {
    throw InvalidOperandException("Cannot take l-value with function return");
  }
  Variant *arr;
  if (larr->weakLval(env, arr)) {
    Variant i;
    Variant *ip;
    switch (m_arrayElementKindOf) {
    case VVN: case DVN:
      ip = &VC(I(m_idx));
      break;
    case VSN: case DSN:
      ip = &S(I(m_idx));
      break;
    case DDN: case VDR:
      ip = new(&i) Variant(D(m_idx));
      break;
    default:
      assert(false);
    }
    SET_LINE_VOID;
    arr->weakRemove(*ip);
  } else {
    m_idx->eval(env);
  }
}

void ArrayElementExpression::sinkStaticMember(Parser *parser,
                                              const NamePtr &className) {
  ArrayElementExpressionPtr arr = m_arr->unsafe_cast<ArrayElementExpression>();
  if (arr) {
    arr->sinkStaticMember(parser, className);
    return;
  }

  VariableExpressionPtr var = m_arr->unsafe_cast<VariableExpression>();
  if (var) {
    m_arr =
      StaticMemberExpressionPtr(new StaticMemberExpression(parser,
                                                           className,
                                                           var->getName()));
    setArrayElementKindOf();
    return;
  }
  ASSERT(false);
}

void ArrayElementExpression::dump(std::ostream &out) const {
  m_arr->dump(out);
  out << "[";
  if (m_idx) {
    m_idx->dump(out);
  }
  out << "]";
}

///////////////////////////////////////////////////////////////////////////////
}
}

