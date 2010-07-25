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

#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/list_assignment.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/expression.h>
#include <compiler/statement/statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/catch_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/break_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/loop_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/echo_statement.h>
#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/parser/hphp.tab.hpp>
#include <util/util.h>

#define spc(T,p) boost::static_pointer_cast<T>(p)
#define dpc(T,p) boost::dynamic_pointer_cast<T>(p)

using namespace HPHP;
using std::string;

///////////////////////////////////////////////////////////////////////////////

AliasManager::AliasManager() :
    m_nextID(1), m_changes(0), m_replaced(0),
    m_wildRefs(0), m_nrvoFix(0), m_inlineAsExpr(true),
    m_noAdd(false) {
}

bool AliasManager::parseOptimizations(const std::string &optimizations,
                                      std::string &errs)
{
  size_t pos = 0;
  while ((pos = optimizations.find_first_not_of(" ,", pos)) !=
         string::npos) {
    size_t end = optimizations.find_first_of(" ,", pos);
    string opt = optimizations.substr(pos, end - pos);
    bool val = true;
    if (opt.substr(0, 3) == "no-") {
      val = false;
      opt = opt.substr(3);
    }

    if (opt == "deadcode") {
      Option::EliminateDeadCode = val;
    } else if (opt == "localcopy") {
      Option::LocalCopyProp = val;
    } else if (opt == "string") {
      Option::StringLoopOpts = val;
    } else if (opt == "inline") {
      Option::AutoInline = val;
    } else if (val && (opt == "all" || opt == "none")) {
      val = opt == "all";
      Option::EliminateDeadCode = val;
      Option::LocalCopyProp = val;
      Option::AutoInline = val;
    } else {
      errs = "Unknown optimization: " + opt;
      return false;
    }

    if (end == string::npos) {
      break;
    }
    pos = end;
  }

  return true;
}

ExpressionPtr BucketMapEntry::find(ExpressionPtr e) {
  ExpressionPtrList::iterator it = m_exprs.begin();
  while (it != m_exprs.end()) {
    ExpressionPtr c = *it;
    if (e->canonCompare(c)) {
      return c;
    }
    ++it;
  }

  return ExpressionPtr();
}

void BucketMapEntry::add(ExpressionPtr e) {
  m_exprs.push_back(e);
  m_num++;
}

void BucketMapEntry::clear() {
  m_stack.resize(0);
  m_exprs.resize(0);
  m_num = 0;
}

void BucketMapEntry::beginScope() {
  m_stack.push_back(m_num);
}

void BucketMapEntry::endScope() {
  resetScope();
  if (m_stack.size()) {
    m_stack.pop_back();
  }
}

void BucketMapEntry::resetScope() {
  if (!m_stack.size()) {
    m_exprs.empty();
    m_num = 0;
  } else {
    m_num = m_stack.back();
    m_exprs.resize(m_num);
  }
}

bool BucketMapEntry::isSubLast(ExpressionPtr e) {
  ExpressionPtrList::reverse_iterator it = rbegin(), end = rend();
  while (it != end) {
    ExpressionPtr t = *it++;
    if (t == e) return true;
    if (t->hasSubExpr(e)) return true;
    if (t->is(Expression::KindOfSimpleVariable)) continue;
    if (!t->is(Expression::KindOfScalarExpression)) break;
    ScalarExpressionPtr se = spc(ScalarExpression, t);
    const std::string &s = se->getString();
    if (s != "begin" && s != "end") break;
  }
  return false;
}

void BucketMapEntry::stash(size_t from, ExpressionPtrList &to) {
  ExpressionPtrList::iterator it = m_exprs.begin(), end = m_exprs.end();
  m_num = from;
  while (from--) {
    assert(it != end);
    ++it;
  }
  while (it != end) {
    ExpressionPtr t = *it;

    to.insert(to.end(), t);
    it = m_exprs.erase(it);
  }
}

void BucketMapEntry::import(ExpressionPtrList &from) {
  m_num += from.size();
  m_exprs.splice(m_exprs.end(), from);
}

void AliasManager::add(BucketMapEntry &em, ExpressionPtr e) {
  if (!m_noAdd) em.add(e);
  e->setCanonID(m_nextID++);
}

ExpressionPtr AliasManager::getCanonical(ExpressionPtr e) {
  unsigned val = (e->getCanonHash() % MaxBuckets) + 1;

  BucketMapEntry &em = m_bucketMap[val];

  ExpressionPtr c = em.find(e);

  if (!c) {
    add(em, e);
    c = e;
    e->setCanonPtr(ExpressionPtr());
  } else {
    e->setCanonID(c->getCanonID());
    e->setCanonPtr(c);
  }

  return c;
}

void AliasManager::clearHelper(BucketMap::value_type &it) {
  it.second.clear();
}

void AliasManager::clear() {
  m_bucketMap.clear();
  m_stack.resize(0);

  //  std::for_each(m_bucketMap.begin(), m_bucketMap.end(),
  //              clearHelper);
}

void AliasManager::beginScopeHelper(BucketMap::value_type &it) {
  it.second.beginScope();
}

void AliasManager::beginScope() {
  if (m_noAdd) return;
  ExpressionPtr e(new ScalarExpression(LocationPtr(),
                                       Expression::KindOfScalarExpression,
                                       T_STRING, string("begin")));
  m_bucketMap[0].add(e);
  m_stack.push_back(m_bucketMap[0].size());
  std::for_each(m_bucketMap.begin(), m_bucketMap.end(),
                beginScopeHelper);
}

void AliasManager::mergeScope() {
  if (m_noAdd) return;
  if (m_stack.size()) {
    CondStackElem &cs = m_stack.back();
    BucketMapEntry &bm = m_bucketMap[0];
    bm.stash(cs.m_size, cs.m_exprs);
  } else {
    clear();
  }
}

void AliasManager::endScopeHelper(BucketMap::value_type &it) {
  it.second.endScope();
}

void AliasManager::endScope() {
  if (m_noAdd) return;
  mergeScope();

  std::for_each(m_bucketMap.begin(), m_bucketMap.end(),
                endScopeHelper);

  if (m_stack.size()) {
    CondStackElem &cs = m_stack.back();
    BucketMapEntry &bm = m_bucketMap[0];
    bm.import(cs.m_exprs);
    ExpressionPtr
      e(new ScalarExpression(LocationPtr(),
                             Expression::KindOfScalarExpression,
                             T_STRING, string("end")));
    bm.add(e);
    m_stack.pop_back();
  }
}

void AliasManager::resetScopeHelper(BucketMap::value_type &it) {
  it.second.resetScope();
}

void AliasManager::resetScope() {
  if (m_noAdd) return;
  mergeScope();
  std::for_each(m_bucketMap.begin(), m_bucketMap.end(),
                resetScopeHelper);
}

int AliasManager::testAccesses(ExpressionPtr e1, ExpressionPtr e2) {
  Expression::KindOf k1 = e1->getKindOf(), k2 = e2->getKindOf();
  while (true) {
    switch (k1) {
    case Expression::KindOfConstantExpression:
      if (e1->canonCompare(e2)) return SameAccess;
      switch (k2) {
      case Expression::KindOfObjectMethodExpression:
      case Expression::KindOfDynamicFunctionCall:
      case Expression::KindOfSimpleFunctionCall:
      case Expression::KindOfNewObjectExpression:
        return InterfAccess;
      default:
        return DisjointAccess;
      }
      break;

    case Expression::KindOfArrayElementExpression:
      if (k2 == Expression::KindOfSimpleVariable ||
          k2 == Expression::KindOfDynamicVariable ||
          k2 == Expression::KindOfConstantExpression) {
        break;
      }

      return e1->canonCompare(e2) ?
        SameAccess : InterfAccess;

    case Expression::KindOfStaticMemberExpression:
      if (k2 == Expression::KindOfSimpleVariable ||
          k2 == Expression::KindOfConstantExpression) {
        break;
      }
      return e1->canonCompare(e2) ?
        SameAccess : InterfAccess;

    case Expression::KindOfObjectPropertyExpression:
      if (k2 == Expression::KindOfSimpleVariable ||
          k2 == Expression::KindOfConstantExpression) {
        break;
      }
      return InterfAccess;

    case Expression::KindOfDynamicVariable:
      if (k2 == Expression::KindOfSimpleVariable ||
          k2 == Expression::KindOfConstantExpression) {
        break;
      }

      return e1->canonCompare(e2) ?
        SameAccess : InterfAccess;

    case Expression::KindOfSimpleVariable:
      {
        if (k2 == Expression::KindOfConstantExpression) {
          return DisjointAccess;
        }
        SimpleVariablePtr sv1 = spc(SimpleVariable, e1);
        AliasInfo &ai1 = m_aliasInfo[sv1->getName()];
        switch (k2) {
        case Expression::KindOfSimpleVariable:
          {
            SimpleVariablePtr sv2 = spc(SimpleVariable, e2);
            if (sv1->getName() == sv2->getName()) {
              return SameAccess;
            }
            AliasInfo &ai2 = m_aliasInfo[sv2->getName()];

            if (ai1.getIsRefTo() || ai1.getIsGlobal()) {
              return m_wildRefs || ai2.getIsGlobal() || ai2.checkRefLevel(0) ?
                InterfAccess : DisjointAccess;
            }

            if (ai2.getIsRefTo() || ai2.getIsGlobal()) {
              return m_wildRefs || ai1.getIsGlobal() || ai1.checkRefLevel(0) ?
                InterfAccess : DisjointAccess;
            }
          }
          return DisjointAccess;

        case Expression::KindOfDynamicVariable:
          return InterfAccess;

        case Expression::KindOfArrayElementExpression:
          if (ai1.getIsRefTo() || ai1.getIsGlobal() ||
              m_wildRefs || ai1.checkRefLevel(0)) {
            return InterfAccess;
          } else {
            // $a = "foo"; $a[0] = "x";
            ExpressionPtr var = spc(ArrayElementExpression, e2)->getVariable();
            if (e1->canonCompare(var)) {
              return InterfAccess;
            }
          }
          return DisjointAccess;

        case Expression::KindOfStaticMemberExpression:
        case Expression::KindOfObjectPropertyExpression:
        default:
          if (ai1.getIsRefTo() || ai1.getIsGlobal() || m_wildRefs) {
            return InterfAccess;
          }
          return DisjointAccess;
        }
        // mustnt get here (we would loop forever).
        ASSERT(false);
      }

    default:
      return InterfAccess;
    }

    ExpressionPtr t = e1;
    e1 = e2;
    e2 = t;
    k1 = k2;
    k2 = e2->getKindOf();
  }
}

static bool isReadOnlyAccess(ExpressionPtr e) {
  if (e->getContext() & (Expression::UnsetContext|
                         Expression::RefValue|
                         Expression::RefParameter|
                         Expression::LValue)) {
    return false;
  }
  switch (e->getKindOf()) {
  case Expression::KindOfConstantExpression:
  case Expression::KindOfSimpleVariable:
  case Expression::KindOfArrayElementExpression:
  case Expression::KindOfDynamicVariable:
    return true;
  default:
    return false;
  }
}

int AliasManager::findInterf(ExpressionPtr rv, bool isLoad,
                             ExpressionPtr &rep) {
  BucketMapEntry lvs = m_bucketMap[0];

  rep = ExpressionPtr();
  ExpressionPtrList::reverse_iterator it = lvs.rbegin(), end = lvs.rend();
  int a;

  int depth = 0, min_depth = 0, max_depth = 0;
  while (it != end) {
    ExpressionPtr e = *it++;
    switch (e->getKindOf()) {
    case Expression::KindOfScalarExpression:
      {
        ScalarExpressionPtr se = spc(ScalarExpression, e);
        const std::string &s = se->getString();
        if (s == "begin") {
          depth--;
          if (depth < min_depth) min_depth = depth;
        } else if (s == "end") {
          depth++;
          if (depth > max_depth) max_depth = depth;
        } else if (s == "io") {
          int effect = rv->getLocalEffects();
          if (effect & (Expression::IOEffect|
                        Expression::CanThrow|
                        Expression::AccessorEffect)) {
            return InterfAccess;
          }
        } else {
          assert(false);
        }
      }
      break;

    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
      a = testAccesses(rv, e);
      if (a == DisjointAccess) {
        continue;
      }
      return a;

    case Expression::KindOfListAssignment: {
      ListAssignmentPtr la = spc(ListAssignment, e);
      ExpressionList &lhs = *la->getVariables().get();
      for (int i = lhs.getCount(); i--; ) {
        ExpressionPtr ep = lhs[i];
        if (ep && testAccesses(ep, rv) != DisjointAccess) {
          return InterfAccess;
        }
      }
      break;
    }

    case Expression::KindOfObjectPropertyExpression:
    case Expression::KindOfConstantExpression:
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfStaticMemberExpression:
      a = testAccesses(e, rv);
      if (a == DisjointAccess) {
        continue;
      }
      if (a == SameAccess) {
        if (isLoad) {
          // The value of an earlier load is available
          // if it dominates this one
          if (depth > min_depth) {
            a = InterfAccess;
          }
        } else {
          // The assignment definitely hits the load
          // if it post-dominates it.
          if (min_depth < 0) {
            a = InterfAccess;
          }
        }
      }
      if (a != SameAccess &&
          isLoad && isReadOnlyAccess(e)) {
        continue;
      }
      rep = e;
      return a;

    case Expression::KindOfUnaryOpExpression:
      a = testAccesses(spc(UnaryOpExpression,e)->getExpression(), rv);
      goto handle_assign;
    case Expression::KindOfBinaryOpExpression:
      a = testAccesses(spc(BinaryOpExpression,e)->getExp1(), rv);
      goto handle_assign;
    case Expression::KindOfAssignmentExpression:
      a = testAccesses(spc(AssignmentExpression,e)->getVariable(), rv);
      goto handle_assign;

    handle_assign:
      if (a == DisjointAccess) {
        continue;
      }
      rep = e;
      if (a == SameAccess) {
        if (isLoad) {
          // we can propagate the value of an assignment
          // to a load, provided the assignment dominates
          // the load.
          if (depth > min_depth) {
            a = InterfAccess;
          }
        } else {
          // a later assignment kills an earlier one
          // provided the later one post-dominates the earlier
          if (min_depth < 0) {
            a = InterfAccess;
          }
        }
      }
      return a;

    default:
      break;
    }
  }
  return DisjointAccess;
}

static int getOpForAssignmentOp(int op) {
  switch (op) {
  case T_PLUS_EQUAL: return '+';
  case T_MINUS_EQUAL: return '-';
  case T_MUL_EQUAL: return '*';
  case T_DIV_EQUAL: return '/';
  case T_CONCAT_EQUAL: return '.';
  case T_MOD_EQUAL: return '%';
  case T_AND_EQUAL: return '&';
  case T_OR_EQUAL: return '|';
  case T_XOR_EQUAL: return '^';
  case T_SL_EQUAL: return T_SL;
  case T_SR_EQUAL: return T_SR;
  default: return 0;
  }
}

ExpressionPtr AliasManager::canonicalizeNonNull(ExpressionPtr e) {
  ExpressionPtr r = canonicalizeNode(e);
  return r ? r : e;
}

ExpressionPtr AliasManager::canonicalizeRecurNonNull(ExpressionPtr e) {
  ExpressionPtr r = canonicalizeRecur(e);
  return r ? r : e;
}

static bool sameExpr(ExpressionPtr e1, ExpressionPtr e2) {
  if (e1 == e2) return true;
  if (e1->getCanonID() != e2->getCanonID()) return false;
  while (e2->getCanonPtr()) {
    e2 = e2->getCanonPtr();
    if (e2 == e1) return true;
  }
  return false;
}

ExpressionPtr AliasManager::canonicalizeNode(ExpressionPtr e) {
  e->setCanonPtr(ExpressionPtr());
  e->setCanonID(0);

  switch (e->getKindOf()) {
  case Expression::KindOfObjectMethodExpression:
  case Expression::KindOfDynamicFunctionCall:
  case Expression::KindOfSimpleFunctionCall:
  case Expression::KindOfNewObjectExpression:
    add(m_bucketMap[0], e);
    break;

  case Expression::KindOfListAssignment:
    add(m_bucketMap[0], e);
    break;

  case Expression::KindOfAssignmentExpression: {
    AssignmentExpressionPtr ae = spc(AssignmentExpression,e);
    if (e->getContext() & Expression::DeadStore) {
      Construct::recomputeEffects();
      return ae->replaceValue(ae->getValue());
    }
    ExpressionPtr rep;
    int interf = findInterf(ae->getVariable(), false, rep);
    if (interf == SameAccess) {
      switch (rep->getKindOf()) {
      default:
        break;
      case Expression::KindOfAssignmentExpression:
        if (Option::EliminateDeadCode) {
          AssignmentExpressionPtr a = spc(AssignmentExpression, rep);
          ExpressionPtr value = a->getValue();
          if (value->getContext() & Expression::RefValue) {
            break;
          }
          if (!Expression::CheckNeeded(m_arp, a->getVariable(), value) ||
              m_bucketMap[0].isSubLast(a)) {
            rep->setReplacement(value);
            m_replaced++;
          }
        }
        break;
      case Expression::KindOfBinaryOpExpression:
        if (Option::EliminateDeadCode) {
          BinaryOpExpressionPtr b = spc(BinaryOpExpression, rep);
          bool ok = b->getActualType() &&
            b->getActualType()->isNoObjectInvolved();
          if (!ok) {
            switch (b->getOp()) {
            case T_PLUS_EQUAL:
              // could be Array
              break;
            case T_MINUS_EQUAL:
            case T_MUL_EQUAL:
            case T_DIV_EQUAL:
            case T_CONCAT_EQUAL:
            case T_MOD_EQUAL:
            case T_AND_EQUAL:
            case T_OR_EQUAL:
            case T_XOR_EQUAL:
            case T_SL_EQUAL:
            case T_SR_EQUAL:
              ok = true;
              break;
            }
          }
          if (ok) {
            ExpressionPtr rhs = b->getExp2()->clone();
            ExpressionPtr lhs = b->getExp1()->clone();
            lhs->clearContext();

            b->setReplacement(
              ExpressionPtr(new BinaryOpExpression(
                              b->getLocation(),
                              Expression::KindOfBinaryOpExpression,
                              lhs, rhs, getOpForAssignmentOp(b->getOp()))));
            m_replaced++;
          }
        }
        break;
      case Expression::KindOfUnaryOpExpression:
        if (Option::EliminateDeadCode) {
          UnaryOpExpressionPtr u = spc(UnaryOpExpression, rep);
          if (u->getActualType() && u->getActualType()->isInteger()) {
            ExpressionPtr val = u->getExpression()->clone();
            val->clearContext();
            if (u->getFront()) {
              ExpressionPtr inc
                (new ScalarExpression(u->getLocation(),
                                      Expression::KindOfScalarExpression,
                                      T_LNUMBER, string("1")));

              val = ExpressionPtr
                (new BinaryOpExpression(u->getLocation(),
                                        Expression::KindOfBinaryOpExpression,
                                        val, inc,
                                        u->getOp() == T_INC ? '+' : '-'));

            }

            u->setReplacement(val);
            m_replaced++;
          }
        }
        break;
      }
    }
    add(m_bucketMap[0], e);
    break;
  }

  case Expression::KindOfSimpleVariable:
  case Expression::KindOfDynamicVariable:
  case Expression::KindOfArrayElementExpression:
  case Expression::KindOfObjectPropertyExpression:
  case Expression::KindOfStaticMemberExpression:
    if (e->hasContext(Expression::UnsetContext) &&
        e->hasContext(Expression::LValue)) {
      if (Option::EliminateDeadCode) {
        ExpressionPtr rep;
        int interf = findInterf(e, false, rep);
        if (interf == SameAccess) {
          if (rep->getKindOf() == e->getKindOf()) {
            // if we hit a previous unset of the same thing
            // we can delete this one
            if (rep->hasContext(Expression::UnsetContext) &&
                rep->hasContext(Expression::LValue)) {
              return canonicalizeRecurNonNull(
                Expression::MakeConstant(m_arp, e->getLocation(), "null"));
            }
          } else {
            switch (rep->getKindOf()) {
            case Expression::KindOfAssignmentExpression:
              {
                if (!e->is(Expression::KindOfSimpleVariable)) break;
                const string &name = spc(SimpleVariable, e)->getName();
                AliasInfo &ai = m_aliasInfo[name];
                if (ai.getIsRefTo() || ai.getRefLevels()) {
                  break;
                }
                AssignmentExpressionPtr a = spc(AssignmentExpression, rep);
                ExpressionPtr value = a->getValue();
                if (value->getContext() & Expression::RefValue) {
                  break;
                }
                if (!Expression::CheckNeeded(m_arp, a->getVariable(), value) ||
                    m_bucketMap[0].isSubLast(a)) {
                  rep->setReplacement(value);
                  m_replaced++;
                } else {
                  ExpressionPtr rhs = value;
                  while (rhs->is(Expression::KindOfAssignmentExpression)) {
                    rhs = spc(AssignmentExpression, rhs)->getValue();
                  }
                  if (!rhs->hasEffect()) {
                    m_noAdd = true;
                    ExpressionPtr v = canonicalizeRecurNonNull(rhs->clone());
                    m_noAdd = false;
                    if (v->getCanonID() == rhs->getCanonID()) {
                      if (a->isUnused() && rhs == value) {
                        value = value->replaceValue(
                          canonicalizeRecurNonNull(
                            Expression::MakeConstant(
                              m_arp, value->getLocation(), "null")));
                        a->setNthKid(1, value);
                        m_changes++;
                      } else {
                        ExpressionListPtr el(
                          new ExpressionList(a->getLocation(),
                                             Expression::KindOfExpressionList,
                                             ExpressionList::ListKindWrapped));
                        a = spc(AssignmentExpression, a->clone());
                        el->addElement(a);
                        el->addElement(a->getValue());
                        a->setNthKid(
                          1, Expression::MakeConstant(m_arp,
                                                      value->getLocation(),
                                                      "null"));
                        rep->setReplacement(el);
                        m_replaced++;
                      }
                    }
                  }
                }
              }
            default:
              break;
            }
          }
        }
      }
      add(m_bucketMap[0], e);
      break;
    }
    // Fall through
  case Expression::KindOfConstantExpression:
    if (!(e->getContext() & (Expression::AssignmentLHS|
                             Expression::OprLValue))) {
      /*
        AssignmentLHS and OprLValue were already taken care of
        by the respective Assignment += ++ operators etc
        Dont add them to the chain again, or it will prevent dead
        store elimination
      */
      if (!(e->getContext() & (Expression::DeepAssignmentLHS|
                               Expression::DeepOprLValue|
                               Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter|
                               Expression::DeepReference|
                               Expression::UnsetContext))) {
        ExpressionPtr rep;
        int interf = findInterf(e, true, rep);
        if (interf == SameAccess) {
          if (rep->getKindOf() == e->getKindOf()) {
            add(m_bucketMap[0], e);
            e->setCanonID(rep->getCanonID());
            e->setCanonPtr(rep);
            return ExpressionPtr();
          }
          if (Option::LocalCopyProp &&
              rep->getKindOf() == Expression::KindOfAssignmentExpression) {
            AssignmentExpressionPtr ae = spc(AssignmentExpression,rep);
            ExpressionPtr rhs = ae->getValue();
            while (rhs->is(Expression::KindOfAssignmentExpression)) {
              rhs = spc(AssignmentExpression, rhs)->getValue();
            }
            if (rhs->isScalar()) {
              rhs = rhs->clone();
              getCanonical(rhs);
              return rhs;
            }
            if (rhs->is(Expression::KindOfSimpleVariable)) {
              ExpressionPtr orig;
              int i = findInterf(rhs, true, orig);
              if (i == SameAccess && sameExpr(rhs, orig)) {
                rhs = rhs->clone();
                return e->replaceValue(canonicalizeRecurNonNull(rhs));
              }
            }
            if (ae->isUnused() && m_bucketMap[0].isLast(ae)) {
              rep = ae->clone();
              ae->setContext(Expression::DeadStore);
              ae->setNthKid(1, Expression::MakeConstant(m_arp,
                                                          ae->getLocation(),
                                                          "null"));
              ae->setNthKid(0, Expression::MakeConstant(m_arp,
                                                        ae->getLocation(),
                                                        "null"));
              Expression::recomputeEffects();
              m_replaced++;
              return e->replaceValue(canonicalizeRecurNonNull(rep));
            }
            e->setCanonPtr(rhs);
          }
        }
      }
      add(m_bucketMap[0], e);
    } else {
      getCanonical(e);
    }
    break;

  case Expression::KindOfBinaryOpExpression: {
    BinaryOpExpressionPtr bop = spc(BinaryOpExpression, e);

    int rop = getOpForAssignmentOp(bop->getOp());
    if (rop) {
      ExpressionPtr lhs = bop->getExp1();
      ExpressionPtr alt;
      int interf = findInterf(lhs, true, alt);
      if (interf == SameAccess &&
          alt->is(Expression::KindOfAssignmentExpression)) {
        ExpressionPtr op0 = spc(AssignmentExpression,alt)->getValue();
        if (op0->isScalar()) {
          ExpressionPtr op1 = bop->getExp2();
          ExpressionPtr rhs(
              (new BinaryOpExpression(e->getLocation(),
                                      Expression::KindOfBinaryOpExpression,
                                      op0->clone(), op1->clone(), rop)));

          lhs = lhs->clone();
          lhs->clearContext(Expression::OprLValue);
          return e->replaceValue(
            canonicalizeRecurNonNull(
              ExpressionPtr(new AssignmentExpression(
                              e->getLocation(),
                              Expression::KindOfAssignmentExpression,
                              lhs, rhs, false))));
        }
      }

      add(m_bucketMap[0], e);
    } else {
      getCanonical(e);
    }
    break;
  }

  case Expression::KindOfUnaryOpExpression:
    {
      UnaryOpExpressionPtr uop = spc(UnaryOpExpression, e);
      switch (uop->getOp()) {
      case T_INC:
      case T_DEC:
        add(m_bucketMap[0], e);
        break;
      default:
        getCanonical(e);
        break;
      }
      break;
    }

  case Expression::KindOfExpressionList:
    if (e->hasContext(Expression::UnsetContext) &&
        spc(ExpressionList, e)->getListKind() ==
        ExpressionList::ListKindParam) {
      ExpressionListPtr el = spc(ExpressionList, e);
      for (int i = el->getCount(); i--; ) {
        if ((*el)[i]->isScalar()) {
          el->removeElement(i);
        }
      }
    }
    // Fall through
  default:
    getCanonical(e);
    break;
  }

  return ExpressionPtr();
}

void AliasManager::canonicalizeKid(ConstructPtr e, ExpressionPtr kid, int i) {
  if (kid) {
    kid = canonicalizeRecur(kid);
    if (kid) {
      e->setNthKid(i, kid);
      m_changes++;
    }
  }
}

ExpressionPtr AliasManager::canonicalizeRecur(ExpressionPtr e) {
  if (ExpressionPtr rep = e->fetchReplacement()) {
    if (e->getContainedEffects() != rep->getContainedEffects()) {
      Expression::recomputeEffects();
    }
    return canonicalizeRecurNonNull(e->replaceValue(rep));
  }

  switch (e->getKindOf()) {
  case Expression::KindOfQOpExpression:
    canonicalizeKid(e, e->getNthExpr(0), 0);
    beginScope();
    canonicalizeKid(e, e->getNthExpr(1), 1);
    resetScope();
    canonicalizeKid(e, e->getNthExpr(2), 2);
    endScope();
    return canonicalizeNode(e);

  case Expression::KindOfBinaryOpExpression:
    if (spc(BinaryOpExpression,e)->isShortCircuitOperator()) {
      canonicalizeKid(e, e->getNthExpr(0), 0);
      beginScope();
      canonicalizeKid(e, e->getNthExpr(1), 1);
      endScope();
      return canonicalizeNode(e);
    }
    break;

  default:
    break;
  }

  for (int i = 0, n = e->getKidCount(); i < n; i++) {
    canonicalizeKid(e, e->getNthExpr(i), i);
  }

  return canonicalizeNode(e);
}

int AliasManager::canonicalizeRecur(StatementPtr s) {
  if (!s) return FallThrough;

  Statement::KindOf stype = s->getKindOf();
  int ret = FallThrough;
  int start = 0;

  switch (stype) {
  case Statement::KindOfFunctionStatement:
  case Statement::KindOfMethodStatement:
  case Statement::KindOfClassStatement:
  case Statement::KindOfInterfaceStatement:
    // Dont handle nested functions
    // they will be dealt with by another
    // top level call to optimize
    return ret;
  case Statement::KindOfStaticStatement:
    clear();
    ret = Converge;
    break;
  case Statement::KindOfClassVariable:
    clear();
    ret = Branch;
  case Statement::KindOfClassConstant:
  case Statement::KindOfGlobalStatement:
  case Statement::KindOfUnsetStatement:
  case Statement::KindOfExpStatement:
  case Statement::KindOfStatementList:
  case Statement::KindOfBlockStatement:
  case Statement::KindOfTryStatement:
    // No special action, just execute
    // and fall through
    break;

  case Statement::KindOfIfStatement:
    {
      StatementPtr iflist = spc(Statement, s->getNthKid(0));
      if (iflist) {
        for (int i = 0, n = iflist->getKidCount(); i < n; i++) {
          StatementPtr ifstmt = spc(Statement, iflist->getNthKid(i));
          ExpressionPtr cond = spc(Expression, ifstmt->getNthKid(0));
          canonicalizeKid(ifstmt, cond, 0);
          if (!i) beginScope();
          beginScope();
          canonicalizeRecur(spc(Statement, ifstmt->getNthKid(1)));
          endScope();
          if (i+1 < n) resetScope();
        }
        endScope();
      }
      return FallThrough;
    }
    break;

  case Statement::KindOfIfBranchStatement:
    assert(0);
    break;

  case Statement::KindOfForStatement:
    canonicalizeKid(s, spc(Expression,s->getNthKid(0)), 0);
    clear();
    canonicalizeKid(s, spc(Expression,s->getNthKid(1)), 1);
    canonicalizeRecur(spc(Statement, s->getNthKid(2)));
    clear();
    canonicalizeKid(s, spc(Expression,s->getNthKid(3)), 3);
    return Converge;

  case Statement::KindOfWhileStatement:
  case Statement::KindOfDoStatement:
  case Statement::KindOfForEachStatement:
    clear();
    ret = Converge;
    break;

  case Statement::KindOfSwitchStatement:
    canonicalizeKid(s, spc(Expression,s->getNthKid(0)), 0);
    clear();
    start = 1;
    ret = Converge;
    break;

  case Statement::KindOfCaseStatement:
    clear();
    break;

  case Statement::KindOfBreakStatement:
  case Statement::KindOfContinueStatement:
  case Statement::KindOfReturnStatement:
    ret = Branch;
    break;

  case Statement::KindOfCatchStatement:
    clear();
    ret = Converge;
    break;

  case Statement::KindOfThrowStatement:
    ret = Branch;
    break;

  case Statement::KindOfEchoStatement:
    {
      EchoStatementPtr es(spc(EchoStatement, s));
      ExpressionListPtr exprs = es->getExpressionList();
      for (int i = 0; i < exprs->getCount(); i++) {
        canonicalizeKid(exprs, exprs->getNthExpr(i), i);
        ExpressionPtr e(new ScalarExpression(LocationPtr(),
                                             Expression::KindOfScalarExpression,
                                             T_STRING, string("io")));
        add(m_bucketMap[0], e);
      }
      return FallThrough;
    }
  }

  int nkid = s->getKidCount();
  for (int i = start; i < nkid; i++) {
    ConstructPtr cp = s->getNthKid(i);
    if (!cp) {
      continue;
    }
    if (StatementPtr skid = dpc(Statement, cp)) {
      int action = canonicalizeRecur(skid);
      switch (action) {
      case FallThrough:
      case CondBranch:
        break;
      case Branch:
        clear();
        break;
      case Converge:
        clear();
        break;
      }
    } else {
      canonicalizeKid(s, spc(Expression, cp), i);
    }
  }

  return ret;
}

void AliasManager::collectAliasInfoRecur(ConstructPtr cs, bool unused) {
  if (!cs) {
    return;
  }

  if (StatementPtr s = dpc(Statement, cs)) {
    switch (s->getKindOf()) {
      case Statement::KindOfFunctionStatement:
      case Statement::KindOfMethodStatement:
      case Statement::KindOfClassStatement:
      case Statement::KindOfInterfaceStatement:
        m_inlineAsExpr = false;
        return;
      default:
        break;
    }
  }

  int nkid = cs->getKidCount();

  for (int i = 0; i < nkid; i++) {
    ConstructPtr kid = cs->getNthKid(i);
    if (kid) {
      collectAliasInfoRecur(kid, cs->kidUnused(i));
    }
  }

  if (ExpressionPtr e = dpc(Expression, cs)) {
    e->setUnused(unused);
    int context = e->getContext();
    switch (e->getKindOf()) {
    case Expression::KindOfAssignmentExpression:
      {
        AssignmentExpressionPtr ae = spc(AssignmentExpression, e);
        ExpressionPtr var = ae->getVariable();
        ExpressionPtr val = ae->getValue();
        if (var->is(Expression::KindOfSimpleVariable)) {
          const std::string &name = spc(SimpleVariable, var)->getName();
          AliasInfo &ai = m_aliasInfo[name];
          if (val->getContext() & Expression::RefValue) {
            ai.setIsRefTo();
            m_variables->addUsed(name);
          } else {
            Expression::CheckNeeded(m_arp, var, val);
          }
        }
      }
      break;
    case Expression::KindOfListAssignment:
      {
        ListAssignmentPtr la = spc(ListAssignment, e);
        ExpressionListPtr vars = la->getVariables();
        for (int i = vars->getCount(); i--; ) {
          ExpressionPtr v = (*vars)[i];
          if (v && v->is(Expression::KindOfSimpleVariable)) {
            SimpleVariablePtr sv = spc(SimpleVariable, v);
            m_variables->addNeeded(sv->getName());
          }
        }
      }
      break;
    case Expression::KindOfSimpleVariable:
      {
        const std::string &name = spc(SimpleVariable, e)->getName();
        AliasInfo &ai = m_aliasInfo[name];
        if (context & Expression::RefValue) {
          ai.addRefLevel(0);
        }
        if (!(context & (Expression::AssignmentLHS |
                         Expression::UnsetContext))) {
          m_variables->addUsed(name);
        }
      }
      break;
    case Expression::KindOfDynamicVariable:
      if (context & Expression::RefValue) {
        m_wildRefs = true;
      }
      break;
    case Expression::KindOfArrayElementExpression:
      {
        int n = 1;
        while (n < 10 &&
               e->is(Expression::KindOfArrayElementExpression)) {
          e = spc(ArrayElementExpression, e)->getVariable();
        }
        if (e->is(Expression::KindOfSimpleVariable)) {
          const std::string &name = spc(SimpleVariable, e)->getName();
          if (context & Expression::RefValue) {
            AliasInfo &ai = m_aliasInfo[name];
            ai.addRefLevel(n);
          }
          m_variables->addUsed(name); // need this for UnsetContext
        }
      }
      break;
    case Expression::KindOfObjectPropertyExpression:
      {
        e = spc(ObjectPropertyExpression, e)->getObject();
        if (e->is(Expression::KindOfSimpleVariable)) {
          const std::string &name = spc(SimpleVariable, e)->getName();
          if (context & Expression::RefValue) {
            AliasInfo &ai = m_aliasInfo[name];
            ai.addRefLevel(1);
          }
          m_variables->addUsed(name); // need this for UnsetContext
        }
      }
      break;
    default:
      break;
    }
  } else {
    StatementPtr s = spc(Statement, cs);
    bool inlineOk = false;
    switch (s->getKindOf()) {
    case Statement::KindOfGlobalStatement:
    case Statement::KindOfStaticStatement:
      {
        ExpressionListPtr vars = dpc(ExpressionList, s->getNthKid(0));
        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr e = (*vars)[i];
          if (AssignmentExpressionPtr ae = dpc(AssignmentExpression, e)) {
            e = ae->getVariable();
          }
          if (SimpleVariablePtr sv = dpc(SimpleVariable, e)) {
            m_aliasInfo[sv->getName()].setIsGlobal();
          }
        }
        break;
      }
    case Statement::KindOfExpStatement:
    case Statement::KindOfStatementList:
      inlineOk = true;
      break;
    case Statement::KindOfCatchStatement:
      {
        const std::string &name = spc(CatchStatement, s)->getVariable();
        m_variables->addUsed(name);
        break;
      }
    case Statement::KindOfReturnStatement:
      inlineOk = true;
      if (m_nrvoFix >= 0) {
        ExpressionPtr e = spc(ReturnStatement, s)->getRetExp();
        if (!e || !e->is(Expression::KindOfSimpleVariable)) {
          m_nrvoFix = -1;
        } else {
          SimpleVariablePtr sv = spc(SimpleVariable, e);
          const std::string &n = sv->getName();
          if (!m_nrvoFix) {
            m_returnVar = n;
            m_nrvoFix = 1;
          } else if (m_returnVar != n) {
            m_nrvoFix = -1;
          }
        }
      }
    default:
      break;
    }
    if (!inlineOk) {
      m_inlineAsExpr = false;
    }
  }
}

int AliasManager::optimize(AnalysisResultPtr ar, MethodStatementPtr m) {
  m_arp = ar;

  m_variables = ar->getScope()->getVariables();
  if (!m_variables->isPseudoMainTable()) {
    m_variables->clearUsed();
  }

  if (ExpressionListPtr pPtr = m->getParams()) {
    ExpressionList &params = *pPtr;
    for (int i = params.getCount(); i--; ) {
      ParameterExpressionPtr p = spc(ParameterExpression, params[i]);
      AliasInfo &ai = m_aliasInfo[p->getName()];
      ai.setIsParam(true);
      if (p->isRef()) {
        ai.setIsRefTo();
      }
    }
  }

  collectAliasInfoRecur(m->getStmts(), false);
  FunctionScopePtr func = ar->getFunctionScope();
  if (func) {
    if (m_inlineAsExpr) {
      if (!Option::AutoInline ||
          func->isVariableArgument() ||
          m_variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
          m_variables->getAttribute(VariableTable::ContainsExtract) ||
          m_variables->getAttribute(VariableTable::ContainsCompact) ||
          m_variables->getAttribute(VariableTable::ContainsGetDefinedVars)) {
        m_inlineAsExpr = false;
      }
    }
    func->setInlineAsExpr(m_inlineAsExpr);
  }

  for (AliasInfoMap::iterator it = m_aliasInfo.begin(),
         end = m_aliasInfo.end(); it != end; ++it) {
    if (m_variables->isPseudoMainTable() ||
        m_variables->isGlobal(it->first) ||
        m_variables->isStatic(it->first)) {
      it->second.setIsGlobal();
    }
  }

  if (Option::LocalCopyProp || Option::EliminateDeadCode) {
    canonicalizeRecur(m->getStmts());
    if (m_replaced) return -1;
  }

  if (ar->getPhase() == AnalysisResult::PostOptimize) {
    if (func) {
      if (func->isRefReturn()) {
        m_nrvoFix = -1;
      } else if (m_nrvoFix > 0) {
        AliasInfo &ai = m_aliasInfo[m_returnVar];
        if (!ai.getIsParam() &&
            (m_wildRefs || ai.getRefLevels() ||
             (ai.getIsGlobal() && m_variables->needLocalCopy(m_returnVar)))) {
          // do nothing
        } else {
          m_nrvoFix = -1;
        }
      }

      func->setNRVOFix(m_nrvoFix > 0);
    }

    if (!m_changes && Option::StringLoopOpts && !m_wildRefs) {
      stringOptsRecur(m->getStmts());
    }
  }

  return m_changes ? 1 : 0;
}

AliasManager::LoopInfo::LoopInfo(StatementPtr s) :
  m_stmt(s), m_valid(!s->is(Statement::KindOfSwitchStatement)) {
}

void AliasManager::pushStringScope(StatementPtr s) {
  m_loopInfo.push_back(LoopInfo(s));
  if (LoopStatementPtr cur = dpc(LoopStatement,s)) {
    cur->clearStringBufs();
  }
}

void AliasManager::popStringScope(StatementPtr s) {
  size_t sz = m_loopInfo.size();
  assert(sz);
  LoopInfo &li1 = m_loopInfo.back();
  assert(li1.m_stmt == s);
  if (li1.m_candidates.size() && li1.m_valid) {
    for (unsigned i = li1.m_inner.size(); i--; ) {
      if (LoopStatementPtr inner = dpc(LoopStatement, li1.m_inner[i])) {
        for (StringSet::iterator it = li1.m_candidates.begin(),
               end = li1.m_candidates.end(); it != end; ++it) {
          inner->removeStringBuf(*it);
        }
      }
    }

    if (LoopStatementPtr cur = dpc(LoopStatement, li1.m_stmt)) {
      for (StringSet::iterator it = li1.m_candidates.begin(),
             end = li1.m_candidates.end(); it != end; ++it) {
        cur->addStringBuf(*it);
      }
    }
  }

  if (sz > 1) {
    LoopInfo &li2 = m_loopInfo[sz-2];
    if (li1.m_candidates.size()) {
      for (StringSet::iterator it = li1.m_candidates.begin(),
             end = li1.m_candidates.end(); it != end; ++it) {
        if (li2.m_excluded.find(*it) == li2.m_excluded.end()) {
          li2.m_candidates.insert(*it);
        }
      }
      li2.m_inner.push_back(s);
    }
    for (StringSet::iterator it = li1.m_excluded.begin(),
           end = li1.m_excluded.end(); it != end; ++it) {
      li2.m_excluded.insert(*it);
      li2.m_candidates.erase(*it);
    }
    for (unsigned i = li1.m_inner.size(); i--; ) {
      if (LoopStatementPtr inner = dpc(LoopStatement, li1.m_inner[i])) {
        if (inner->numStringBufs()) {
          li2.m_inner.push_back(inner);
        }
      }
    }
  }

  m_loopInfo.pop_back();
}

void AliasManager::stringOptsRecur(ExpressionPtr e, bool ok) {
  if (!e) return;
  if (!m_loopInfo.size()) return;
  Expression::KindOf etype = e->getKindOf();
  switch (etype) {
  case Expression::KindOfBinaryOpExpression:
    {
      BinaryOpExpressionPtr b(spc(BinaryOpExpression,e));
      stringOptsRecur(b->getExp2(), false);
      if (ok && b->getOp() == T_CONCAT_EQUAL) {
        ExpressionPtr var = b->getExp1();
        if (var->is(Expression::KindOfSimpleVariable)) {
          SimpleVariablePtr s(spc(SimpleVariable,var));
          AliasInfo &ai = m_aliasInfo[s->getName()];
          if (!ai.getIsGlobal() &&
              !ai.getIsParam() &&
              !ai.getRefLevels() &&
              !ai.getIsRefTo()) {
            LoopInfo &li = m_loopInfo.back();
            if (li.m_excluded.find(s->getName()) ==
                li.m_excluded.end()) {
              li.m_candidates.insert(s->getName());
              return;
            }
          }
        }
      }
      stringOptsRecur(b->getExp1(), false);
    }
    return;
  case Expression::KindOfExpressionList:
    {
      ExpressionListPtr el(spc(ExpressionList,e));
      if (el->getListKind() != ExpressionList::ListKindParam) {
        int n = el->getCount();
        int chk = el->getListKind() == ExpressionList::ListKindLeft ?
          0 : n - 1;
        for (int i = 0; i < n; i++) {
          stringOptsRecur((*el)[i], i != chk || ok);
        }
        return;
      }
      break;
    }
  case Expression::KindOfSimpleVariable:
    {
      SimpleVariablePtr s(spc(SimpleVariable,e));
      LoopInfo &li = m_loopInfo.back();
      li.m_excluded.insert(s->getName());
      li.m_candidates.erase(s->getName());
      return;
    }
  default:
    break;
  }

  for (int i = 0, n = e->getKidCount(); i < n; i++) {
    stringOptsRecur(e->getNthExpr(i), false);
  }
}

void AliasManager::stringOptsRecur(StatementPtr s) {
  if (!s) return;

  bool pop = false;
  Statement::KindOf stype = s->getKindOf();
  switch (stype) {
  case Statement::KindOfFunctionStatement:
  case Statement::KindOfMethodStatement:
  case Statement::KindOfClassStatement:
  case Statement::KindOfInterfaceStatement:
    // Dont handle nested functions
    // they will be dealt with by another
    // top level call
    return;

  case Statement::KindOfSwitchStatement:
    if (m_loopInfo.size()) {
      pushStringScope(s);
      pop = true;
    }
    break;

  case Statement::KindOfForStatement:
    stringOptsRecur(spc(Expression,s->getNthKid(0)), true);
    pushStringScope(s);
    stringOptsRecur(spc(Expression,s->getNthKid(1)), false);
    stringOptsRecur(spc(Statement, s->getNthKid(2)));
    stringOptsRecur(spc(Expression,s->getNthKid(3)), true);
    popStringScope(s);
    return;

  case Statement::KindOfWhileStatement:
    pushStringScope(s);
    stringOptsRecur(spc(Expression,s->getNthKid(0)), false);
    stringOptsRecur(spc(Statement, s->getNthKid(1)));
    popStringScope(s);
    return;

  case Statement::KindOfDoStatement:
    pushStringScope(s);
    stringOptsRecur(spc(Statement, s->getNthKid(0)));
    stringOptsRecur(spc(Expression,s->getNthKid(1)), false);
    popStringScope(s);
    return;

  case Statement::KindOfForEachStatement:
    stringOptsRecur(spc(Expression,s->getNthKid(0)), false);
    stringOptsRecur(spc(Expression,s->getNthKid(1)), false);
    stringOptsRecur(spc(Expression,s->getNthKid(2)), false);
    pushStringScope(s);
    stringOptsRecur(spc(Statement, s->getNthKid(3)));
    popStringScope(s);
    return;

  case Statement::KindOfExpStatement:
    stringOptsRecur(spc(ExpStatement,s)->getExpression(), true);
    return;

  case Statement::KindOfBreakStatement:
    {
      BreakStatementPtr b = spc(BreakStatement, s);
      int64 depth = b->getDepth();
      if (depth != 1) {
        int64 s = m_loopInfo.size() - 1;
        if (s >= 0) {
          if (!depth || depth > s) depth = s;
          while (depth--) {
            m_loopInfo[s - depth].m_valid = false;
          }
        }
      }
    }
    break;

  default:
    break;
  }

  int nkid = s->getKidCount();
  for (int i = 0; i < nkid; i++) {
    ConstructPtr cp = s->getNthKid(i);
    if (!cp) {
      continue;
    }
    if (StatementPtr skid = dpc(Statement, cp)) {
      stringOptsRecur(skid);
    } else {
      stringOptsRecur(spc(Expression, cp), false);
    }
  }
  if (pop) {
    popStringScope(s);
  }
}
