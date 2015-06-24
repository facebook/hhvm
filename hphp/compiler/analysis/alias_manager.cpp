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

#include "hphp/compiler/analysis/alias_manager.h"
#include <map>
#include <utility>
#include <vector>

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/ast_walker.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/list_assignment.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/expression/await_expression.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/catch_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/statement/if_statement.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/statement/for_statement.h"
#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/echo_statement.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/global_statement.h"
#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/analysis/variable_table.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/parser/hphp.tab.hpp"
#include "hphp/parser/location.h"
#include "hphp/util/text-util.h"

#define spc(T,p) static_pointer_cast<T>(p)
#define dpc(T,p) dynamic_pointer_cast<T>(p)

using namespace HPHP;
using std::string;

///////////////////////////////////////////////////////////////////////////////

AliasManager::AliasManager() :
    m_bucketList(0), m_nextID(1), m_changes(0), m_replaced(0),
    m_wildRefs(false), m_inCall(0),
    m_noAdd(false),
    m_cleared(false), m_inPseudoMain(false),
    m_exprIdx(-1) {
}

AliasManager::~AliasManager() {
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
    } else if (opt == "inline") {
      Option::AutoInline = val ? 1 : -1;
    } else if (opt == "coalesce") {
      Option::VariableCoalescing = val;
    } else if (val && (opt == "all" || opt == "none")) {
      val = opt == "all";
      Option::EliminateDeadCode = val;
      Option::LocalCopyProp = val;
      Option::AutoInline = val ? 1 : -1;
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
    always_assert(m_exprs.size() >= m_num);
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
    always_assert(it != end);
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
  unsigned val = (e->getCanonHash() % MaxBuckets);

  BucketMapEntry &em = m_bucketMap[val];
  em.link(m_bucketList);

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

void AliasManager::clear() {
  m_accessList.clear();
  m_bucketMap.clear();
  m_bucketList = 0;
  m_stack.resize(0);
  m_cleared = true;
}

void AliasManager::beginScope() {
  if (m_noAdd) return;
  ExpressionPtr e(new ScalarExpression(BlockScopePtr(), Location::Range(),
                                       T_STRING, string("begin")));
  m_accessList.add(e);
  m_stack.push_back(CondStackElem(m_accessList.size()));
  m_accessList.beginScope();
  if (BucketMapEntry *tail = m_bucketList) {
    BucketMapEntry *bm = tail;
    do {
      bm->beginScope();
    } while ((bm = bm->next()) != tail);
  }
}

void AliasManager::mergeScope() {
  if (m_noAdd) return;
  if (m_stack.size()) {
    CondStackElem &cs = m_stack.back();
    BucketMapEntry &bm = m_accessList;
    bm.stash(cs.m_size, cs.m_exprs);
  } else {
    clear();
  }
}

void AliasManager::endScope() {
  if (m_noAdd) return;
  mergeScope();

  m_accessList.endScope();
  if (BucketMapEntry *tail = m_bucketList) {
    BucketMapEntry *bm = tail;
    do {
      bm->endScope();
    } while ((bm = bm->next()) != tail);
  }

  if (m_stack.size()) {
    CondStackElem &cs = m_stack.back();
    BucketMapEntry &bm = m_accessList;
    bm.import(cs.m_exprs);
    ExpressionPtr
      e(new ScalarExpression(BlockScopePtr(), Location::Range(),
                             T_STRING, string("end")));
    bm.add(e);
    m_stack.pop_back();
  }
}

void AliasManager::resetScope() {
  if (m_noAdd) return;
  mergeScope();
  m_accessList.resetScope();
  if (BucketMapEntry *tail = m_bucketList) {
    BucketMapEntry *bm = tail;
    do {
      bm->resetScope();
    } while ((bm = bm->next()) != tail);
  }
}

void AliasManager::dumpAccessChain() {
  BucketMapEntry &lvs = m_accessList;
  ExpressionPtrList::reverse_iterator it = lvs.rbegin(), end = lvs.rend();

  if (isInExpression()) {
    std::cout << "m_exprIdx: " << m_exprIdx << std::endl;
    std::cout << "parent: "; m_exprParent->dumpNode(0, m_arp);
  } else {
    std::cout << "Not in expression" << std::endl;
  }

  while (it != end) {
    ExpressionPtr e = *it;
    e->dump(0, m_arp);
    ++it;
  }
}

bool AliasManager::couldBeAliased(SimpleVariablePtr sv) {
  if (m_wildRefs) return true;
  if (!sv->couldBeAliased()) return false;
  if (m_inPseudoMain) return true;
  int context = sv->getContext();
  if ((context & Expression::Declaration) ==
      Expression::Declaration /* global declaration */ ||
      (context & Expression::RefAssignmentLHS) ||
      (context & (Expression::LValue|Expression::UnsetContext)) ==
      (Expression::LValue|Expression::UnsetContext)) {
    /*
      All of these are effectively reference assignments, so the only
      thing that interferes is an exact match.
    */
    return false;
  }
  return true;
}

static bool canonCompare(ExpressionPtr e1, ExpressionPtr e2) {
  return
    e1->hasContext(Expression::ExistContext) ==
    e2->hasContext(Expression::ExistContext) &&
    e1->canonCompare(e2);
}

static bool compareArrays(ArrayElementExpressionPtr e1,
                          ArrayElementExpressionPtr e2) {
  if (!e1->getOffset() || !e2->getOffset() ||
      e1->getOffset()->getCanonID() != e2->getOffset()->getCanonID()) {
    return false;
  }
  always_assert(e1->getOffset()->getCanonID() != 0);

  if (e1->getVariable()->getCanonID() == e2->getVariable()->getCanonID() &&
      e1->getVariable()->getCanonID() != 0) {
    return true;
  }

  if (e1->getVariable()->getCanonLVal() == e2->getVariable() ||
      e2->getVariable()->getCanonLVal() == e1->getVariable()) {
    return true;
  }

  return false;
}

int AliasManager::testAccesses(ExpressionPtr e1, ExpressionPtr e2,
    bool forLval /* = false */) {
  Expression::KindOf k1 = e1->getKindOf(), k2 = e2->getKindOf();
  while (true) {
    switch (k1) {
    case Expression::KindOfConstantExpression:
      if (canonCompare(e1, e2)) return SameAccess;
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

      if (canonCompare(e1, e2)) return SameAccess;

      if (k2 != Expression::KindOfArrayElementExpression) {
        if (forLval && !e1->hasContext(Expression::LValue)) {
          return DisjointAccess;
        }
        return InterfAccess;
      }

      {
        ArrayElementExpressionPtr p1(spc(ArrayElementExpression, e1));
        ArrayElementExpressionPtr p2(spc(ArrayElementExpression, e2));
        if (compareArrays(p1, p2)) return SameAccess;

        if (!forLval)             return InterfAccess;

        // forLval alias checking

        // e1 and e2 are both array elements, where e2 is testing
        // against e1 (e1 is ahead in the chain)

        if (!p2->getVariable()->is(Expression::KindOfSimpleVariable)) {
          return InterfAccess;
        }
        SimpleVariablePtr sv2(spc(SimpleVariable, p2->getVariable()));
        if (couldBeAliased(sv2)) return InterfAccess;

        // e2 looks like ($a[...]) now, and $a is not referenced
        // so the only way e1 would interfere (in an lvalue sense)
        // is if e1 looked also like ($a[...])

        if (!p1->getVariable()->is(Expression::KindOfSimpleVariable)) {
          if (p1->getVariable()->is(Expression::KindOfDynamicVariable)) {
            return InterfAccess;
          }
          return DisjointAccess;
        }
        SimpleVariablePtr sv1(spc(SimpleVariable, p1->getVariable()));

        // make sure the variables refer to the same thing by chasing
        // the canon ptr
        ExpressionPtr p(sv2->getCanonLVal());
        while (p && p != sv1) p = p->getCanonLVal();
        if (!p) return DisjointAccess;

        // make sure the offsets refer to the same *value* by comparing
        // canon ids
        if (!p1->getOffset() || !p2->getOffset()) return InterfAccess;
        return p1->getOffset()->getCanonID() == p2->getOffset()->getCanonID() ?
          SameAccess : InterfAccess;
      }

    case Expression::KindOfStaticMemberExpression:
      if (k2 == Expression::KindOfSimpleVariable ||
          k2 == Expression::KindOfConstantExpression) {
        break;
      }
      return canonCompare(e1, e2) ?
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

      return canonCompare(e1, e2) ?
        SameAccess : InterfAccess;

    case Expression::KindOfSimpleVariable:
      {
        if (k2 == Expression::KindOfConstantExpression) {
          return DisjointAccess;
        }
        SimpleVariablePtr sv1 = spc(SimpleVariable, e1);
        switch (k2) {
        case Expression::KindOfSimpleVariable:
          {
            SimpleVariablePtr sv2 = spc(SimpleVariable, e2);
            if (sv1->getName() == sv2->getName()) {
              if (sv1->SimpleVariable::isThis() &&
                  sv1->hasContext(Expression::ObjectContext) !=
                  sv2->hasContext(Expression::ObjectContext) &&
                  m_variables->getAttribute(
                    VariableTable::ContainsLDynamicVariable)) {
                /*
                 * $this not in object context may not be the same as
                 * $this in object context if there have been writes
                 * through a variable table
                 */
                return InterfAccess;
              }
              return SameAccess;
            }

            if (couldBeAliased(sv1) && couldBeAliased(sv2)) {
              return InterfAccess;
            }
          }
          return DisjointAccess;

        case Expression::KindOfDynamicVariable:
          return InterfAccess;

        case Expression::KindOfArrayElementExpression:
          if (couldBeAliased(sv1)) {
            return InterfAccess;
          }
          return DisjointAccess;

        case Expression::KindOfSimpleFunctionCall: {
          SimpleFunctionCallPtr call(spc(SimpleFunctionCall, e2));
          if (call->readsLocals() || call->writesLocals()) {
            return InterfAccess;
          }
          goto def;
        }
        case Expression::KindOfIncludeExpression: {
          return InterfAccess;
        }
        case Expression::KindOfStaticMemberExpression:
        case Expression::KindOfObjectPropertyExpression:
        default: def:
          if (couldBeAliased(sv1)) {
            return InterfAccess;
          }
          return DisjointAccess;
        }
        // mustnt get here (we would loop forever).
        assert(false);
      }
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfIncludeExpression:
      if (k2 == Expression::KindOfSimpleVariable) {
        break;
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

static void updateDepthAndFlags(ExpressionPtr e, int &depth, int &flags) {
  ScalarExpressionPtr se = spc(ScalarExpression, e);
  const std::string &s = se->getString();
  if (s == "begin") {
    depth--;
  } else if (s == "end") {
    depth++;
  } else if (s == "io") {
    flags |= Expression::IOEffect;
  } else {
    not_reached();
  }
}

void AliasManager::cleanRefs(ExpressionPtr e,
                             ExpressionPtrList::reverse_iterator it,
                             ExpressionPtrList::reverse_iterator &end,
                             int depth) {
  if (e->is(Expression::KindOfUnaryOpExpression) &&
      e->getLocalEffects() == Expression::UnknownEffect) {
    return;
  }
  if (e->is(Expression::KindOfAssignmentExpression) ||
      e->is(Expression::KindOfBinaryOpExpression) ||
      e->is(Expression::KindOfUnaryOpExpression)) {
    ExpressionPtr var = e->getStoreVariable();
    if (var->is(Expression::KindOfSimpleVariable) &&
        !(var->getContext() & Expression::RefAssignmentLHS)) {
      SimpleVariablePtr sv(spc(SimpleVariable, var));
      if (sv->getSymbol() && sv->getSymbol()->isReferenced()) {
        /*
          Suppose we have:
          $t = &$q;
          $t = 0;
          return $q;

          The $q from the return interferes with "$t = 0;", so we
          remove "$t = 0" from the list (meaning that we /wont/ kill it).
          But $q does not interfere with "$t = &$q". So when we remove
          "$t = 0", we also have to remove any reference assignments
          to $t.
        */
        int effects = 0;
        bool pIsLoad = false;
        ++it;
        while (it != end) {
          ExpressionPtr p = *it;
          if (p->is(Expression::KindOfScalarExpression)) {
            checkInterf(var, p, pIsLoad, depth, effects);
            if (depth < 0) return;
          } else if (p->is(Expression::KindOfAssignmentExpression) ||
                     (p->is(Expression::KindOfSimpleVariable) &&
                      (p->hasAllContext(Expression::Declaration) ||
                       p->hasAllContext(Expression::LValue |
                                        Expression::UnsetContext)))) {
            if (checkInterf(var, p, pIsLoad, depth, effects) == SameAccess) {
              m_accessList.erase(it, end);
              continue;
            }
          }
          ++it;
        }
      }
    }
  }
}

void AliasManager::cleanInterf(ExpressionPtr load,
                               ExpressionPtrList::reverse_iterator it,
                               ExpressionPtrList::reverse_iterator &end,
                               int depth) {
  while (it != end) {
    ExpressionPtr e = *it;
    bool eIsLoad = false;
    int effects = 0;
    int a = checkInterf(load, e, eIsLoad, depth, effects);
    if (a != DisjointAccess) {
      if (a == NotAccess) {
        if (depth < 0) return;
      } else if (!eIsLoad && !dpc(FunctionCall, e)) {
        cleanRefs(e, it, end, depth);
        m_accessList.erase(it, end);
        continue;
      }
    }
    ++it;
  }
}

bool AliasManager::okToKill(ExpressionPtr ep, bool killRef) {
  if (ep && ep->isNoRemove()) return false;
  if (ep && ep->is(Expression::KindOfSimpleVariable)) {
    return spc(SimpleVariable, ep)->canKill(killRef);
  }
  return false;
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
  case T_POW_EQUAL: return T_POW;
  default: return 0;
  }
}

void AliasManager::killLocals() {
  BucketMapEntry &lvs = m_accessList;
  ExpressionPtrList::reverse_iterator it = lvs.rbegin(), end = lvs.rend();
  int effects = 0;
  int depth = 0;
  int emask = (Expression::IOEffect |
               Expression::CanThrow |
               Expression::AccessorEffect |
               Expression::OtherEffect);

  while (it != end) {
    ExpressionPtr e = *it;
    switch (e->getKindOf()) {
      case Expression::KindOfScalarExpression:
        updateDepthAndFlags(e, depth, effects);
        if (depth < 0) {
          it = end;
          continue;
        }
        break;

      case Expression::KindOfListAssignment:
        if (!(effects & emask)) {
          ListAssignmentPtr la = spc(ListAssignment, e);
          ExpressionList &lhs = *la->getVariables().get();
          for (int i = lhs.getCount(); i--; ) {
            if (okToKill(lhs[i], false)) {
              lhs[i].reset();
            }
          }
        }
        goto kill_it;

      case Expression::KindOfAssignmentExpression:
        if (!(effects & emask)) {
          AssignmentExpressionPtr ae(spc(AssignmentExpression, e));
          if (okToKill(ae->getVariable(),
                       ae->getValue()->getContext() & Expression::RefValue)) {
            e->setContext(Expression::DeadStore);
            m_replaced++;
          }
        }
        cleanRefs(e, it, end, depth);
        goto kill_it;

      case Expression::KindOfBinaryOpExpression:
        cleanInterf(spc(BinaryOpExpression, e)->getExp1(), ++it, end, depth);
        continue;

      case Expression::KindOfUnaryOpExpression:
        if (e->getLocalEffects() == Expression::UnknownEffect) goto kill_it;
        cleanInterf(spc(UnaryOpExpression, e)->getExpression(),
                    ++it, end, depth);
        continue;

      case Expression::KindOfSimpleVariable:
      case Expression::KindOfObjectPropertyExpression:
      case Expression::KindOfDynamicVariable:
      case Expression::KindOfArrayElementExpression:
      case Expression::KindOfStaticMemberExpression:
        if (e->hasAllContext(Expression::LValue | Expression::UnsetContext) ||
            e->hasAllContext(Expression::Declaration)) {
          if (!(effects & emask) && okToKill(e, true)) {
            bool ok = (!e->is(Expression::KindOfSimpleVariable));
            if (!ok) {
              Symbol *sym = spc(SimpleVariable,e)->getSymbol();
              ok =
                !sym ||
                (sym->isGlobal() && !e->hasContext(Expression::UnsetContext)) ||
                (!sym->isNeeded() && !sym->isUsed());
            }
            if (ok) {
              e->setReplacement(e->makeConstant(m_arp, "null"));
              m_replaced++;
            }
          } else {
            effects |= Expression::UnknownEffect;
          }
          goto kill_it;
        }
        cleanInterf(e, ++it, end, depth);
        continue;

      default: kill_it:
        lvs.erase(it, end);
        effects |= e->getContainedEffects();
        continue;
    }

    effects |= e->getContainedEffects();
    ++it;
  }
}

int AliasManager::checkInterf(ExpressionPtr rv, ExpressionPtr e,
                              bool &isLoad, int &depth, int &effects,
                              bool forLval /* = false */) {
  isLoad = true;
  switch (e->getKindOf()) {
    case Expression::KindOfScalarExpression:
    {
      updateDepthAndFlags(e, depth, effects);
      return NotAccess;
    }

    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
      isLoad = false;
      return testAccesses(rv, e, forLval);

    case Expression::KindOfListAssignment: {
      isLoad = false;
      ListAssignmentPtr la = spc(ListAssignment, e);
      ExpressionList &lhs = *la->getVariables().get();
      for (int i = lhs.getCount(); i--; ) {
        ExpressionPtr ep = lhs[i];
        if (ep) {
          if (ep->is(Expression::KindOfListAssignment)) {
            if (checkInterf(rv, ep, isLoad, depth, effects, forLval) !=
                DisjointAccess) {
              return InterfAccess;
            }
          } else if (testAccesses(ep, rv, forLval) != DisjointAccess) {
            return InterfAccess;
          }
        }
      }
      break;
    }

    case Expression::KindOfObjectPropertyExpression:
    case Expression::KindOfConstantExpression:
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfStaticMemberExpression: {
      int m = e->getContext() &
                 (Expression::LValue|
                  Expression::Declaration|
                  Expression::UnsetContext);
      isLoad = !(m & Expression::LValue) || m == Expression::LValue;
      return testAccesses(e, rv, forLval);
    }

    case Expression::KindOfUnaryOpExpression:
      if (e->getLocalEffects() == Expression::UnknownEffect) {
        isLoad = false;
        return InterfAccess;
      }
      // fall through
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfBinaryOpExpression: {
      isLoad = false;
      ExpressionPtr var = e->getStoreVariable();
      int access = testAccesses(var, rv, forLval);
      if (access == SameAccess) {
        // An assignment to something that might be visible from
        // another scope, and that might contain an object, could
        // end up with some other value (due to a destructor running)
        // than the rhs.
        if (var->getKindOf() != Expression::KindOfSimpleVariable) {
          return InterfAccess;
        }
        SimpleVariablePtr sv = static_pointer_cast<SimpleVariable>(var);
        if (sv->couldBeAliased() &&
            (!sv->getSymbol() || sv->getSymbol()->isNeeded())) {
          return InterfAccess;
        }
      }
      return access;
    }

    default:
      not_reached();
  }

  return DisjointAccess;
}

int AliasManager::checkAnyInterf(ExpressionPtr e1, ExpressionPtr e2,
                                 bool &isLoad, int &depth, int &effects,
                                 bool forLval /* = false */) {
  switch (e1->getKindOf()) {
    case Expression::KindOfListAssignment: {
      ListAssignmentPtr la = spc(ListAssignment, e1);
      ExpressionList &lhs = *la->getVariables().get();
      for (int i = lhs.getCount(); i--; ) {
        ExpressionPtr ep = lhs[i];
        if (ep && checkAnyInterf(ep, e2, isLoad, depth, effects, forLval) !=
            DisjointAccess) {
          isLoad = false;
          return InterfAccess;
        }
      }
      return DisjointAccess;
    }
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfBinaryOpExpression:
    case Expression::KindOfUnaryOpExpression:
      if (e1->getLocalEffects() == Expression::UnknownEffect) {
        isLoad = false;
        return InterfAccess;
      }
      e1 = e1->getStoreVariable();
      if (!e1 || !e1->hasContext(Expression::OprLValue)) return DisjointAccess;
      break;
    default:
      break;
  }

  return checkInterf(e1, e2, isLoad, depth, effects, forLval);
}

int AliasManager::findInterf0(
  ExpressionPtr rv, bool isLoad,
  ExpressionPtr &rep,
  ExpressionPtrList::reverse_iterator begin,
  ExpressionPtrList::reverse_iterator end,
  int *flags /* = 0 */,
  bool allowLval /* = false */, bool forLval /* = false */,
  int depth /* = 0 */, int min_depth /* = 0 */,
  int max_depth /* = 0 */) {

  rep.reset();
  ExpressionPtrList::reverse_iterator it = begin;

  bool unset_simple = !isLoad &&
    rv->hasAllContext(Expression::UnsetContext | Expression::LValue) &&
    rv->is(Expression::KindOfSimpleVariable) &&
    (!m_inPseudoMain || spc(SimpleVariable, rv)->isHidden());

  bool hasStash = false;
  ExpressionPtrList::reverse_iterator stash;
  int stash_depth = 0, stash_min_depth = 0, stash_max_depth = 0;

  for (; it != end; ++it) {
    ExpressionPtr e = *it;
    if (e->getContext() & Expression::DeadStore) continue;
    bool eIsLoad = false;
    int effects = 0;
    int a = checkInterf(rv, e, eIsLoad, depth, effects, forLval);
    if (a != DisjointAccess) {
      if (a == NotAccess) {
        if (effects & Expression::IOEffect) {
          int effect = rv->getLocalEffects();
          if (effect & (Expression::IOEffect|
                        Expression::CanThrow|
                        Expression::AccessorEffect|
                        Expression::OtherEffect)) {
            return InterfAccess;
          }
        } else if (depth < min_depth) {
          min_depth = depth;
        } else if (depth > max_depth) {
          max_depth = depth;
        }
      } else {
        if (a == InterfAccess &&
            rv->is(Expression::KindOfArrayElementExpression) &&
            e->hasContext(Expression::AccessContext)) {
          ExpressionPtr t = rv;
          do {
            t = spc(ArrayElementExpression, t)->getVariable();
            if (t == e) break;
          } while (t->is(Expression::KindOfArrayElementExpression));
          if (t == e) continue;
        }
        if (unset_simple) {
          if (a == InterfAccess) {
            if (!rep) {
              rep = e;
            }
            continue;
          } else if (a == SameAccess && rep) {
            if (!e->is(Expression::KindOfSimpleVariable) ||
                !e->hasAllContext(Expression::UnsetContext |
                                  Expression::LValue)) {
              return InterfAccess;
            }
          }
        }
        if (a == SameAccess) {
          if (flags) {
            *flags = 0;
            if (depth > min_depth) *flags |= NoCopyProp;
            if (min_depth < 0) *flags |= NoDeadStore;
          } else if (isLoad) {
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
        if (a != SameAccess && eIsLoad && isLoad && isReadOnlyAccess(e)) {
          if (!hasStash && !forLval && allowLval) {
            // stash the state away so we can pick up here if we need to
            hasStash = true;
            stash = it;
            stash_depth = depth;
            stash_min_depth = min_depth;
            stash_max_depth = max_depth;
          }
          continue;
        }
        rep = e;
        if (!forLval && allowLval && a != SameAccess) {
          if (hasStash) {
            return findInterf0(
                rv, isLoad, rep,
                stash, end, flags, allowLval, true,
                stash_depth, stash_min_depth,
                stash_max_depth);
          }
          forLval = true;
          // try this node again
          --it;
          continue;
        }
        if (a == SameAccess && forLval) a = SameLValueAccess;
        return a;
      }
    }
  }

  if (hasStash) {
    assert(allowLval);
    return findInterf0(
        rv, isLoad, rep,
        stash, end, flags, allowLval, true,
        stash_depth, stash_min_depth,
        stash_max_depth);
  }

  return DisjointAccess;
}

int AliasManager::findInterf(ExpressionPtr rv, bool isLoad,
                             ExpressionPtr &rep, int *flags /* = 0 */) {
  BucketMapEntry &lvs = m_accessList;
  return findInterf0(rv, isLoad, rep, lvs.rbegin(), lvs.rend(),
      flags, false);
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
  while (true) {
    e2 = e2->getCanonPtr();
    if (!e2) break;
    if (e2 == e1) return true;
  }
  return false;
}

void AliasManager::setCanonPtrForArrayCSE(
    ExpressionPtr e,
    ExpressionPtr rep) {
  assert(e);
  assert(rep);
  if (e->is(Expression::KindOfArrayElementExpression)) {
    // e is an array access in rvalue context,
    // need to switch on rep
    ExpressionPtr rep0;
    switch (rep->getKindOf()) {
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfBinaryOpExpression:
    case Expression::KindOfUnaryOpExpression:
      rep0 = rep->getStoreVariable();
      break;
    case Expression::KindOfListAssignment:
      // TODO: IMPLEMENT
      assert(false);
      break;
    default:
      rep0 = rep;
      break;
    }
    if (rep0 && rep0->is(Expression::KindOfArrayElementExpression)) {
      e->setCanonPtr(rep0);
    }
  }
}

void AliasManager::processAccessChain(ExpressionPtr e) {
  if (!e) return;
  if (!e->is(Expression::KindOfObjectPropertyExpression) &&
      !e->is(Expression::KindOfArrayElementExpression)) {
    return;
  }
  for (int i = 0, n = e->getKidCount(); i < n; ++i) {
    ExpressionPtr kid(e->getNthExpr(i));
    if (kid && kid->hasContext(Expression::AccessContext)) {
      processAccessChain(kid);
      ExpressionPtr rep(canonicalizeNode(kid, true));
      if (rep) {
        e->setNthKid(i, rep);
        e->recomputeEffects();
        setChanged();
      }
      break;
    }
  }
}

void AliasManager::processAccessChainLA(ListAssignmentPtr la) {
  ExpressionList &lhs = *la->getVariables().get();
  for (int i = lhs.getCount(); i--; ) {
    ExpressionPtr ep = lhs[i];
    if (ep) {
      if (ep->is(Expression::KindOfListAssignment)) {
        processAccessChainLA(spc(ListAssignment, ep));
      } else {
        processAccessChain(ep);
        add(m_accessList, ep);
      }
    }
  }
}

ExpressionPtr AliasManager::canonicalizeNode(
  ExpressionPtr e, bool doAccessChains /* = false */) {
  if (e->isVisited()) return ExpressionPtr();
  if ((e->getContext() & (Expression::AssignmentLHS|
                          Expression::OprLValue)) ||
      (!doAccessChains && e->hasContext(Expression::AccessContext))) {
    ExpressionPtr ret;
    if (!m_noAdd) {
      ret = e->preOptimize(m_arp);
      if (ret) {
        return canonicalizeRecurNonNull(ret);
      }
    }
    e->setCanonPtr(ExpressionPtr());
    e->setCanonID(0);
    return ExpressionPtr();
  }

  ExpressionPtr var;
  switch (e->getKindOf()) {
    case Expression::KindOfAssignmentExpression: {
    case Expression::KindOfBinaryOpExpression:
    case Expression::KindOfUnaryOpExpression:
      ExpressionPtr var = e->getStoreVariable();
      if (var && var->getContext() & (Expression::AssignmentLHS|
                                      Expression::OprLValue)) {
        processAccessChain(var);
      }
      break;
    }
    case Expression::KindOfListAssignment:
      processAccessChainLA(spc(ListAssignment,e));
      break;
    default:
      break;
  }

  ExpressionPtr ret;
  if (!m_noAdd && !doAccessChains) {
    ret = e->preOptimize(m_arp);
    if (ret) {
      return canonicalizeRecurNonNull(ret);
    }
  }

  e->setVisited();
  e->clearLocalExprAltered();
  e->setCanonPtr(ExpressionPtr());
  e->setCanonID(0);

  switch (e->getKindOf()) {
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
    case Expression::KindOfListAssignment:
      add(m_accessList, e);
      break;

    case Expression::KindOfAssignmentExpression: {
      AssignmentExpressionPtr ae = spc(AssignmentExpression,e);
      if (e->hasContext(Expression::DeadStore)) {
        e->recomputeEffects();
        return ae->replaceValue(ae->getValue());
      }
      ExpressionPtr rep;
      int interf = findInterf(ae->getVariable(), false, rep);
      if (interf == SameAccess) {
        switch (rep->getKindOf()) {
          case Expression::KindOfAssignmentExpression: {
            AssignmentExpressionPtr a = spc(AssignmentExpression, rep);
            rep = a->getVariable();
            if (Option::EliminateDeadCode) {
              ExpressionPtr value = a->getValue();
              if (value->getContext() & Expression::RefValue) {
                break;
              }
              if (!Expression::CheckNeeded(a->getVariable(), value) ||
                  m_accessList.isSubLast(a) ||
                  (value->isTemporary() && m_expr->hasSubExpr(a))) {
                a->setReplacement(value);
                m_replaced++;
              }
            }
            break;
          }
          case Expression::KindOfBinaryOpExpression: {
            BinaryOpExpressionPtr b = spc(BinaryOpExpression, rep);
            if (Option::EliminateDeadCode) {
              bool ok = b->getActualType() &&
                b->getActualType()->isNoObjectInvolved();
              if (!ok) {
                switch (b->getOp()) {
                  case T_PLUS_EQUAL:
                    // could be Array
                    break;
                  case T_MINUS_EQUAL:
                  case T_MUL_EQUAL:
                  case T_POW_EQUAL:
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
                                  b->getScope(), b->getRange(),
                                  lhs, rhs, getOpForAssignmentOp(b->getOp()))));
                m_replaced++;
              }
            }
            rep = b->getExp1();
            break;
          }
          case Expression::KindOfUnaryOpExpression: {
            UnaryOpExpressionPtr u = spc(UnaryOpExpression, rep);
            assert(u->getOp() == T_INC || u->getOp() == T_DEC);
            if (Option::EliminateDeadCode) {
              if (u->getActualType() && u->getActualType()->isInteger()) {
                ExpressionPtr val = u->getExpression()->clone();
                val->clearContext();
                if (u->getFront()) {
                  auto inc = std::make_shared<ScalarExpression>(
                    u->getScope(), u->getRange(), T_LNUMBER, string("1"));

                  val = std::make_shared<BinaryOpExpression>(
                    u->getScope(), u->getRange(), val, inc,
                    u->getOp() == T_INC ? '+' : '-');
                }

                u->setReplacement(val);
                m_replaced++;
              }
              rep = u->getExpression();
            }
            break;
          }
          default:
            break;
        }
        always_assert(rep->getKindOf() == ae->getVariable()->getKindOf());
        ae->getVariable()->setCanonID(rep->getCanonID());
      } else {
        ae->getVariable()->setCanonID(m_nextID++);
      }
      add(m_accessList, e);
      break;
    }

    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfObjectPropertyExpression:
      if (!e->hasContext(Expression::AccessContext)) {
        processAccessChain(e);
      }
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
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
                return canonicalizeRecurNonNull(e->makeConstant(m_arp, "null"));
              }
            } else {
              switch (rep->getKindOf()) {
                case Expression::KindOfAssignmentExpression:
                {
                  /*
                    Handling unset of a variable which hasnt been
                    used since it was last assigned
                  */
                  if (!e->is(Expression::KindOfSimpleVariable)) {
                    break;
                  }

                  AssignmentExpressionPtr a = spc(AssignmentExpression, rep);

                  always_assert(
                    a->getVariable()->is(Expression::KindOfSimpleVariable));
                  SimpleVariablePtr variable =
                    spc(SimpleVariable, a->getVariable());
                  if (variable->couldBeAliased()) {
                    break;
                  }

                  ExpressionPtr value = a->getValue();
                  if (value->getContext() & Expression::RefValue) {
                    break;
                  }
                  if (!Expression::CheckNeeded(a->getVariable(), value) ||
                      m_accessList.isSubLast(a) ||
                      (value->isTemporary() && m_expr->hasSubExpr(a))) {
                    rep->setReplacement(value);
                    m_replaced++;
                  } else {
                    ExpressionPtr last = value;
                    while (last->is(Expression::KindOfAssignmentExpression)) {
                      last = spc(AssignmentExpression, last)->getValue();
                    }
                    if (!last->isScalar()) {
                      ExpressionPtr cur = value;
                      while (cur) {
                        ExpressionPtr rhs = cur;
                        ExpressionPtr next;
                        if (cur->is(Expression::KindOfAssignmentExpression)) {
                          rhs = spc(AssignmentExpression, cur)->getVariable();
                          next = spc(AssignmentExpression, cur)->getValue();
                        } else {
                          next.reset();
                        }
                        if (!rhs->hasEffect()) {
                          m_noAdd = true;
                          ExpressionPtr v = rhs->clone();
                          v->clearContext();
                          v = canonicalizeRecurNonNull(v);
                          m_noAdd = false;
                          while (v->getCanonPtr() && v->getCanonPtr() != last) {
                            v = v->getCanonPtr();
                          }
                          if (v->getCanonPtr()) {
                            if (a->isUnused() && rhs == value) {
                              value = value->replaceValue(
                                canonicalizeRecurNonNull(
                                  value->makeConstant(m_arp, "null")));
                              a->setValue(value);
                              a->recomputeEffects();
                              setChanged();
                            } else {
                              auto el = std::make_shared<ExpressionList>(
                                a->getScope(), a->getRange(),
                                ExpressionList::ListKindWrapped);
                              a = spc(AssignmentExpression, a->clone());
                              el->addElement(a);
                              el->addElement(a->getValue());
                              a->setValue(value->makeConstant(m_arp, "null"));
                              rep->setReplacement(el);
                              m_replaced++;
                            }
                            break;
                          }
                        }
                        cur = next;
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
        add(m_accessList, e);
        break;
      }
      // Fall through
    case Expression::KindOfConstantExpression: {
      /*
        Expressions with AccessContext need to be taken care of
        by processAccessChain(), which adds the entire access chain
        to the access list after all its side effects have taken place.
        In the case of assignment, or binary op assignment, its added
        after the rhs has been fully processed too.
      */
      if (e->hasContext(Expression::AccessContext)) {
        always_assert(doAccessChains);
        if (e->getContext() & (Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter|
                               Expression::DeepReference|
                               Expression::UnsetContext)) {
          ExpressionPtr rep;
          int interf =
            findInterf(e, true, rep, nullptr);
          add(m_accessList, e);
          if (interf == SameAccess) {
            if (e->getKindOf() == rep->getKindOf()) {
              // e->setCanonID(rep->getCanonID());
            } else if (rep->is(Expression::KindOfBinaryOpExpression) ||
                       rep->is(Expression::KindOfUnaryOpExpression)) {
              break;
            } else if (rep->is(Expression::KindOfAssignmentExpression)) {
              //e->setCanonID(
              //  spc(AssignmentExpression, rep)->getVariable()->getCanonID());
              do {
                rep = spc(AssignmentExpression, rep)->getValue();
              } while (rep->is(Expression::KindOfAssignmentExpression));
            }
            e->setCanonPtr(rep);
          } else if (interf == SameLValueAccess) {
            setCanonPtrForArrayCSE(e, rep);
          } else if (interf == InterfAccess && rep &&
                     rep->is(Expression::KindOfAssignmentExpression) &&
                     (e->is(Expression::KindOfSimpleVariable) ||
                      e->is(Expression::KindOfArrayElementExpression))) {
            ExpressionPtr val = rep;
            do {
              val = spc(AssignmentExpression, val)->getValue();
            } while (val->is(Expression::KindOfAssignmentExpression));
            if (!val->isScalar()) break;
            ExpressionPtr var = spc(AssignmentExpression, rep)->getVariable();
            while (var->is(Expression::KindOfArrayElementExpression)) {
              var = spc(ArrayElementExpression, var)->getVariable();
              if (var->getKindOf() == e->getKindOf()) {
                if (var->is(Expression::KindOfSimpleVariable)) {
                  if (canonCompare(var, e)) {
                    e->setCanonPtr(var);
                    break;
                  }
                } else {
                  ArrayElementExpressionPtr a1(
                    spc(ArrayElementExpression, e));
                  ArrayElementExpressionPtr a2(
                    spc(ArrayElementExpression, var));
                  if (compareArrays(a1, a2)) {
                    e->setCanonPtr(var);
                    break;
                  }
                }

              }
            }
          }
          break;
        }
      }
      if (!(e->getContext() & (Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter|
                               Expression::DeepReference|
                               Expression::UnsetContext))) {
        ExpressionPtr rep;
        int interf =
          findInterf(e, true, rep, nullptr);
        if (!m_inPseudoMain && interf == DisjointAccess && !m_cleared &&
            e->is(Expression::KindOfSimpleVariable) &&
            !e->isThis()) {
          Symbol *s = spc(SimpleVariable, e)->getSymbol();
          if (s && !s->isParameter() && !s->isClosureVar()) {
            rep = e->makeConstant(m_arp, "null");
            Compiler::Error(Compiler::UseUndeclaredVariable, e);
            if (m_variables->getAttribute(VariableTable::ContainsCompact)) {
              rep = std::make_shared<UnaryOpExpression>(
                e->getScope(), e->getRange(), rep, T_UNSET_CAST, true);
            }
            return e->replaceValue(canonicalizeRecurNonNull(rep));
          }
        }
        if (interf == SameAccess) {
          if (rep->getKindOf() == e->getKindOf()) {
            if (rep->hasContext(Expression::UnsetContext) &&
                rep->hasContext(Expression::LValue) &&
                !rep->is(Expression::KindOfConstantExpression)) {
              return e->replaceValue(
                canonicalizeRecurNonNull(e->makeConstant(m_arp, "null")));
            }
            add(m_accessList, e);
            e->setCanonID(rep->getCanonID());
            e->setCanonPtr(rep);
            return ExpressionPtr();
          }
          if (Option::LocalCopyProp &&
              rep->getKindOf() == Expression::KindOfAssignmentExpression) {
            AssignmentExpressionPtr ae = spc(AssignmentExpression,rep);
            ExpressionPtr cur = ae->getValue(), last = cur;
            while (last->is(Expression::KindOfAssignmentExpression)) {
              last = spc(AssignmentExpression, last)->getValue();
            }
            if (last->isScalar()) {
              last = last->clone();
              getCanonical(last);
              return last;
            }
            while (cur) {
              ExpressionPtr rhs = cur;
              ExpressionPtr next;
              if (cur->is(Expression::KindOfAssignmentExpression)) {
                rhs = spc(AssignmentExpression, cur)->getVariable();
                next = spc(AssignmentExpression, cur)->getValue();
              } else {
                next.reset();
              }
              if (rhs->is(Expression::KindOfSimpleVariable)) {
                rhs = rhs->clone();
                rhs->clearContext();
                ExpressionPtr orig;
                int i = findInterf(rhs, true, orig);
                if (i == SameAccess &&
                    (sameExpr(cur, orig) || (next && sameExpr(next, orig)))) {
                  e->recomputeEffects();
                  return e->replaceValue(canonicalizeRecurNonNull(rhs));
                }
              }
              cur = next;
            }
            if (!m_inCall &&
                !last->is(Expression::KindOfAwaitExpression) &&
                !last->is(Expression::KindOfYieldExpression) &&
                ae->isUnused() && m_accessList.isLast(ae) &&
                !e->hasAnyContext(Expression::AccessContext |
                                  Expression::ObjectContext |
                                  Expression::ExistContext |
                                  Expression::UnsetContext)) {
              rep = ae->clone();
              ae->setContext(Expression::DeadStore);
              ae->setValue(ae->makeConstant(m_arp, "null"));
              ae->setVariable(ae->makeConstant(m_arp, "null"));
              e->recomputeEffects();
              m_replaced++;
              return e->replaceValue(canonicalizeRecurNonNull(rep));
            }
            e->setCanonPtr(last);
          }
        } else if (interf == SameLValueAccess) {
          setCanonPtrForArrayCSE(e, rep);
        }
      }
      add(m_accessList, e);
      break;
    }

    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr bop = spc(BinaryOpExpression, e);
      int rop = getOpForAssignmentOp(bop->getOp());
      if (bop->hasContext(Expression::DeadStore)) {
        always_assert(rop);
        ExpressionPtr rhs = bop->getExp2();
        ExpressionPtr lhs = bop->getExp1();
        lhs->clearContext();
        e->recomputeEffects();
        return bop->replaceValue(
          canonicalizeNonNull(std::make_shared<BinaryOpExpression>(
                                bop->getScope(), bop->getRange(),
                                lhs, rhs, rop)));
      }
      if (rop) {
        ExpressionPtr lhs = bop->getExp1();
        ExpressionPtr alt;
        int flags = 0;
        int interf = findInterf(lhs, true, alt, &flags);
        if (interf == SameAccess && !(flags & NoCopyProp)) {
          switch (alt->getKindOf()) {
            case Expression::KindOfAssignmentExpression: {
              ExpressionPtr op0 = spc(AssignmentExpression,alt)->getValue();
              if (op0->isScalar()) {
                ExpressionPtr op1 = bop->getExp2();
                ExpressionPtr rhs = std::make_shared<BinaryOpExpression>(
                  e->getScope(), e->getRange(),
                  op0->clone(), op1->clone(), rop);

                lhs = lhs->clone();
                lhs->clearContext(Expression::OprLValue);
                return e->replaceValue(
                  canonicalizeRecurNonNull(
                    std::make_shared<AssignmentExpression>(
                      e->getScope(), e->getRange(), lhs, rhs, false)));
              }
              alt = spc(AssignmentExpression,alt)->getVariable();
              break;
            }
            case Expression::KindOfBinaryOpExpression: {
              auto b2(spc(BinaryOpExpression, alt));
              if (!(flags & NoDeadStore) && b2->getOp() == bop->getOp()) {
                switch (rop) {
                  case '-':
                    rop = '+';
                    break;
                  case '+':
                  case '*':
                  case '.':
                  case '&':
                  case '|':
                  case '^':
                    break;
                  default:
                    rop = 0;
                    break;
                }
                if (rop) {
                  ExpressionPtr op0 = b2->getExp2();
                  bool ok = op0->isScalar();
                  if (!ok && !op0->hasEffect()) {
                    m_noAdd = true;
                    ExpressionPtr v = op0->clone();
                    v->clearContext();
                    v = canonicalizeRecurNonNull(v);
                    m_noAdd = false;
                    while (v->getCanonPtr() && v->getCanonPtr() != op0) {
                      v = v->getCanonPtr();
                    }
                    ok = (v->getCanonPtr() != nullptr);
                  }
                  if (ok) {
                    b2->setContext(Expression::DeadStore);
                    ExpressionPtr r(new BinaryOpExpression(
                                      bop->getScope(), bop->getRange(),
                                      op0->clone(), bop->getExp2(),
                                      rop));
                    ExpressionPtr b(new BinaryOpExpression(
                                      bop->getScope(), bop->getRange(),
                                      lhs, r, bop->getOp()));
                    return e->replaceValue(canonicalizeRecurNonNull(b));
                  }
                }
              }
              alt = spc(BinaryOpExpression,alt)->getExp1();
              break;
            }
            case Expression::KindOfUnaryOpExpression:
              alt = spc(UnaryOpExpression,alt)->getExpression();
              break;
            default:
              break;
          }
          always_assert(alt->getKindOf() == lhs->getKindOf());
          lhs->setCanonID(alt->getCanonID());
        } else {
          lhs->setCanonID(m_nextID++);
        }
        add(m_accessList, e);
      } else {
        getCanonical(e);
      }
      break;
    }

    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr uop = spc(UnaryOpExpression, e);
      switch (uop->getOp()) {
        case T_INC:
        case T_DEC:
          {
            ExpressionPtr alt;
            int interf = findInterf(uop->getExpression(), true, alt);
            if (interf == SameAccess) {
              switch (alt->getKindOf()) {
                case Expression::KindOfAssignmentExpression:
                case Expression::KindOfBinaryOpExpression:
                case Expression::KindOfUnaryOpExpression:
                  alt = alt->getStoreVariable();
                  break;
                default:
                  break;
              }
              always_assert(alt->getKindOf() ==
                            uop->getExpression()->getKindOf());
              uop->getExpression()->setCanonID(alt->getCanonID());
            } else {
              uop->getExpression()->setCanonID(m_nextID++);
            }
          }
          add(m_accessList, e);
          break;
        default:
          if (uop->getLocalEffects() == Expression::UnknownEffect) {
            add(m_accessList, e);
          } else {
            getCanonical(e);
          }
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

void AliasManager::canonicalizeKid(ConstructPtr c, ExpressionPtr kid, int i) {
  assert(c->getNthKid(i) == kid);

  if (kid) {
    StatementPtr sp(dpc(Statement, c));
    if (sp) beginInExpression(sp, kid);
    kid = canonicalizeRecur(kid);
    if (kid) {
      c->setNthKid(i, kid);
      c->recomputeEffects();
      setChanged();
    }
    if (sp) {
      endInExpression(sp);
      ExpressionPtr kid0(dpc(Expression, c->getNthKid(i)));
      assert(kid0);
      kid0->computeLocalExprAltered();
    }
  }
}

int AliasManager::canonicalizeKid(ConstructPtr c, ConstructPtr kid, int i) {
  assert(c->getNthKid(i) == kid);

  int ret = FallThrough;
  if (kid) {
    ExpressionPtr e = dpc(Expression, kid);
    if (e) {
      canonicalizeKid(c, e, i);
    } else {
      StatementPtr s = dpc(Statement, kid);
      s = canonicalizeRecur(s, ret);
      if (s) {
        c->setNthKid(i, s);
        c->recomputeEffects();
        setChanged();
      }
    }
  }
  return ret;
}

ExpressionPtr AliasManager::canonicalizeRecur(ExpressionPtr e) {
  if (e->isVisited()) return ExpressionPtr();
  if (ExpressionPtr rep = e->fetchReplacement()) {
    if (e->getContainedEffects() != rep->getContainedEffects()) {
      e->recomputeEffects();
    }
    return canonicalizeRecurNonNull(e->replaceValue(rep));
  }

  bool delayVars = true;
  bool pushStack = false;
  bool inCall = false;
  bool setInCall = true;

  switch (e->getKindOf()) {
    case Expression::KindOfQOpExpression: {
      QOpExpressionPtr qe(spc(QOpExpression, e));
      canonicalizeKid(e, qe->getCondition(), 0);
      beginScope();
      if (ExpressionPtr e1 = qe->getYes()) {
        canonicalizeKid(e, e1, 1);
        resetScope();
      }
      canonicalizeKid(e, qe->getNo(), 2);
      endScope();
      return canonicalizeNode(e);
    }
    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr binop(spc(BinaryOpExpression, e));
      if (binop->isShortCircuitOperator()) {
        canonicalizeKid(e, binop->getExp1(), 0);
        beginScope();
        canonicalizeKid(e, binop->getExp2(), 1);
        endScope();
        return canonicalizeNode(e);
      }
      break;
    }
    case Expression::KindOfExpressionList:
      delayVars = false;
      break;

    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfDynamicFunctionCall:
      inCall = setInCall;
    case Expression::KindOfObjectMethodExpression:
      delayVars = false;
      // fall through
      pushStack = m_accessList.size() > 0;
      break;

    default:
      break;
  }

  ExpressionPtr aBack;
  if (pushStack) {
    aBack = m_accessList.back();
    assert(aBack);
  }

  int n = e->getKidCount();
  if (n < 2) delayVars = false;
  if (e->is(Expression::KindOfAssignmentExpression)) delayVars = false;

  m_inCall += inCall;
  for (int j = delayVars ? 0 : 1; j < 2; j++) {
    for (int i = 0; i < n; i++) {
      if (ExpressionPtr kid = e->getNthExpr(i)) {
        /*
          php doesnt evaluate simple variables at the point they are seen
          except in function parameters and function "addresses".
          So in a case like:
          $a + ($a = 5)
          the first $a is evaluated after the assignment. But in:
          ($a + 1) + ($a = 5)
          the first $a is evaluated before the assignment.
          To deal with this, if we're not dealing with a parameter list, and
          there's more than one child node, we do two passes, and process the
          simple variables on the second pass.
        */
        if (delayVars) {
          bool stash = !kid->is(Expression::KindOfSimpleVariable) ||
            spc(SimpleVariable, kid)->getAlwaysStash();
          if (stash == j) continue;
        }
        canonicalizeKid(e, kid, i);
      }
    }
  }

  if (pushStack) m_exprBeginStack.push_back(aBack);
  ExpressionPtr ret(canonicalizeNode(e));
  if (pushStack) m_exprBeginStack.pop_back();
  m_inCall -= inCall;
  return ret;
}

StatementPtr AliasManager::canonicalizeRecur(StatementPtr s, int &ret) {
  ret = FallThrough;
  if (!s || s->isVisited()) return StatementPtr();

  // Dont handle nested functions
  // they will be dealt with by another
  // top level call to optimize
  if (FunctionWalker::SkipRecurse(s)) return StatementPtr();

  Statement::KindOf stype = s->getKindOf();
  int start = 0;
  int nkid = s->getKidCount();

  switch (stype) {
  case Statement::KindOfUseTraitStatement:
  case Statement::KindOfClassRequireStatement:
  case Statement::KindOfTraitPrecStatement:
  case Statement::KindOfTraitAliasStatement:
    return StatementPtr();
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
    // No special action, just execute
    // and fall through
    break;

  case Statement::KindOfIfStatement: {
    IfStatementPtr is = spc(IfStatement, s);
    StatementListPtr iflist = is->getIfBranches();
    if (iflist) {
      for (int i = 0, n = iflist->getKidCount(); i < n; i++) {
        IfBranchStatementPtr ifstmt = spc(IfBranchStatement, (*iflist)[i]);
        canonicalizeKid(ifstmt, ifstmt->getCondition(), 0);
        if (!i) beginScope();
        beginScope();
        canonicalizeKid(ifstmt, ifstmt->getStmt(), 1);
        endScope();
        if (i+1 < n) resetScope();
      }
      endScope();
    }
    ret = FallThrough;
    start = nkid;
    break;
  }

  case Statement::KindOfIfBranchStatement:
    always_assert(false);
    break;

  case Statement::KindOfForStatement: {
    ForStatementPtr fs(spc(ForStatement, s));
    canonicalizeKid(s, fs->getInitExp(), 0);
    clear();
    canonicalizeKid(s, fs->getCondExp(), 1);
    canonicalizeKid(s, fs->getBody(), 2);
    clear();
    canonicalizeKid(s, fs->getIncExp(), 3);
    ret = Converge;
    start = nkid;
    break;
  }
  case Statement::KindOfWhileStatement:
  case Statement::KindOfDoStatement:
  case Statement::KindOfForEachStatement:
    clear();
    ret = Converge;
    break;

  case Statement::KindOfSwitchStatement: {
    SwitchStatementPtr ss(spc(SwitchStatement, s));
    canonicalizeKid(s, ss->getExp(), 0);
    clear();
    start = 1;
    ret = Converge;
    break;
  }
  case Statement::KindOfCaseStatement:
  case Statement::KindOfLabelStatement:
    clear();
    break;

  case Statement::KindOfReturnStatement: {
    ReturnStatementPtr rs(spc(ReturnStatement, s));
    canonicalizeKid(s, rs->getRetExp(), 0);
    killLocals();
    ret = FallThrough;
    start = nkid;
    break;
  }
  case Statement::KindOfBreakStatement:
  case Statement::KindOfContinueStatement:
  case Statement::KindOfGotoStatement:
    ret = Branch;
    break;

  case Statement::KindOfTryStatement: {
    TryStatementPtr trs(spc(TryStatement, s));
    beginScope();
    canonicalizeKid(s, trs->getBody(), 0);
    endScope();
    clear();
    canonicalizeKid(s, trs->getCatches(), 1);
    if (trs->getFinally()) {
      clear();
      canonicalizeKid(s, trs->getFinally(), 2);
    }
    ret = Converge;
    start = nkid;
    break;
  }

  case Statement::KindOfFinallyStatement: {
    break;
  }

  case Statement::KindOfTypedefStatement:
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
        ExpressionPtr kid(exprs->getNthExpr(i));
        beginInExpression(es, kid);
        canonicalizeKid(exprs, kid, i);
        endInExpression(es);
        kid->computeLocalExprAltered();
        ExpressionPtr e(new ScalarExpression(BlockScopePtr(), Location::Range(),
                                             T_STRING, string("io")));
        add(m_accessList, e);
      }
      ret = FallThrough;
      start = nkid;
      break;
    }

  default:
    not_reached();
  }

  for (int i = start; i < nkid; i++) {
    ConstructPtr cp = s->getNthKid(i);
    if (!cp) {
      continue;
    }
    int action = canonicalizeKid(s, cp, i);
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
  }

  s->setVisited();
  StatementPtr rep;
  rep = s->preOptimize(m_arp);
  if (rep) {
    s = canonicalizeRecur(rep, ret);
    return s ? s : rep;
  }
  return StatementPtr();
}

int AliasManager::collectAliasInfoRecur(ConstructPtr cs, bool unused) {
  if (!cs) {
    return 0;
  }

  cs->clearVisited();

  StatementPtr s = dpc(Statement, cs);
  if (s) {
    switch (s->getKindOf()) {
      case Statement::KindOfFunctionStatement:
      case Statement::KindOfMethodStatement:
      case Statement::KindOfClassStatement:
      case Statement::KindOfInterfaceStatement:
        return 0;
      default:
        break;
    }
  }

  int nkid = cs->getKidCount();
  int kidCost = 0;
  for (int i = 0; i < nkid; i++) {
    ConstructPtr kid = cs->getNthKid(i);
    if (kid) {
      kidCost += collectAliasInfoRecur(kid, cs->kidUnused(i));
    }
  }

  if (s) {
    Statement::KindOf skind = s->getKindOf();
    switch (skind) {
      case Statement::KindOfGlobalStatement:
      case Statement::KindOfStaticStatement:
      {
        ExpressionListPtr vars = (skind == Statement::KindOfGlobalStatement)
          ? spc(GlobalStatement, s)->getVars()
          : spc(StaticStatement, s)->getVars();

        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr e = (*vars)[i];
          if (AssignmentExpressionPtr ae = dpc(AssignmentExpression, e)) {
            e = ae->getVariable();
          }
          if (SimpleVariablePtr sv = dpc(SimpleVariable, e)) {
            if (Symbol *sym = sv->getSymbol()) {
              sym->setReferenced();
              sym->setReseated();
              if (skind == Statement::KindOfGlobalStatement) {
                sym->setGlobal();
              } else if (!m_variables->isPseudoMainTable()) {
                m_variables->addStaticVariable(sym, m_arp);
              }
            }
          }
        }
        break;
      }
      case Statement::KindOfCatchStatement:
        {
          CatchStatementPtr c(spc(CatchStatement, s));
          if (c->getVariable()->isThis()) {
            // Since catching $this results in re-assignment to $this, forcing
            // a variable table is the easiest way to force v_this to exist as
            // a local variable in the current scope.  While this is clearly
            // not optimal, I don't believe there is a compelling reason to
            // optimize for this case.
            m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
          }
        }
        break;
      case Statement::KindOfForEachStatement: {
        ForEachStatementPtr fs(static_pointer_cast<ForEachStatement>(s));
        SimpleVariablePtr name = dpc(SimpleVariable, fs->getNameExp());
        if (name) {
          if (Symbol *sym = name->getSymbol()) {
            sym->setNeeded();
          }
        }
        SimpleVariablePtr value = dpc(SimpleVariable, fs->getValueExp());
        if (value) {
          if (Symbol *sym = value->getSymbol()) {
            sym->setNeeded();
          }
        }
        break;
      }
      default:
        break;
    }
  } else {
    ExpressionPtr e = spc(Expression, cs);
    e->setCanonID(0);
    if (!kidCost) {
      if (!nkid) {
        if (e->is(Expression::KindOfSimpleVariable)) {
          if (!e->isThis()) {
            Symbol *sym = spc(SimpleVariable, e)->getSymbol();
            if (!sym || !sym->isParameter()) kidCost = 1;
          }
        } else if (!e->isScalar()) {
          kidCost = 1;
        }
      } else if (e->getLocalEffects() & ~Expression::AccessorEffect) {
        kidCost = 1;
      }
    } else if (!e->is(Expression::KindOfExpressionList)) {
      ++kidCost;
    }
    e->setUnused(unused);
    int context = e->getContext();
    int ekind = e->getKindOf();
    switch (ekind) {
      case Expression::KindOfAssignmentExpression:
      {
        AssignmentExpressionPtr ae = spc(AssignmentExpression, e);
        ExpressionPtr var = ae->getVariable();
        ExpressionPtr val = ae->getValue();
        if (var->is(Expression::KindOfSimpleVariable)) {
          if (val->getContext() & Expression::RefValue) {
            if (Symbol *sym = spc(SimpleVariable, var)->getSymbol()) {
              sym->setReferenced();
              sym->setReseated();
              sym->setUsed();
            }
          } else {
            Expression::CheckNeeded(var, val);
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
            if (Symbol *sym = spc(SimpleVariable, v)->getSymbol()) {
              sym->setNeeded();
            }
          }
        }
      }
      break;
      case Expression::KindOfSimpleVariable:
      {
        SimpleVariablePtr sv(spc(SimpleVariable, e));
        if (Symbol *sym = sv->getSymbol()) {
          bool ref = (context & (Expression::RefValue|
                                 Expression::RefAssignmentLHS)) ||
            sym->isRefClosureVar();
          if (ref) {
            sym->setReferenced();
          }
          bool unset = ((context & Expression::UnsetContext) &&
            (context & Expression::LValue));
          if (sv->isThis()) {
            sv->getFunctionScope()->setContainsThis();
            if (!e->hasContext(Expression::ObjectContext)) {
              sv->getFunctionScope()->setContainsBareThis(true, ref || unset);
            }
          }
          if (unset) {
            sym->setReseated();
          }
          if (!(context & (Expression::AssignmentLHS |
                           Expression::UnsetContext))) {
            sym->setUsed();
          }
        }
      }
      break;
      case Expression::KindOfDynamicVariable:
        m_variables->setAttribute(VariableTable::ContainsDynamicVariable);
        if (context & (Expression::RefValue|
                       Expression::LValue)) {
          m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
          if (context & Expression::RefValue) {
            m_wildRefs = true;
          }
        }
        break;
      case Expression::KindOfIncludeExpression:
      {
        IncludeExpressionPtr inc(spc(IncludeExpression, e));
        m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
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
          if (Symbol *sym = spc(SimpleVariable, e)->getSymbol()) {
            if (context & Expression::RefValue) {
              sym->setReferenced();
            }
            sym->setUsed(); // need this for UnsetContext
          }
        }
      }
      break;
      case Expression::KindOfObjectPropertyExpression:
      {
        ExpressionPtr obj = spc(ObjectPropertyExpression, e)->getObject();
        if (obj->is(Expression::KindOfSimpleVariable)) {
          if (Symbol *sym = spc(SimpleVariable, obj)->getSymbol()) {
            if (context & Expression::RefValue) {
              sym->setReferenced();
            }
            sym->setUsed(); // need this for UnsetContext
          }
        }
      }
      break;
      case Expression::KindOfSimpleFunctionCall:
      {
        SimpleFunctionCallPtr sfc(spc(SimpleFunctionCall, e));
        sfc->updateVtFlags();
      }
      case Expression::KindOfDynamicFunctionCall:
      case Expression::KindOfClassConstantExpression:
      case Expression::KindOfStaticMemberExpression:
      case Expression::KindOfNewObjectExpression: {
        StaticClassName *p = dynamic_cast<StaticClassName*>(e.get());
        always_assert(p);
        bool useLSB = false;
        if (p->isStatic()) {
          useLSB = true;
        } else if (ekind == Expression::KindOfDynamicFunctionCall) {
          if ((p->getClassName().empty() && !p->getClass()) ||
              p->isParent() || p->isSelf()) {
            useLSB = true;
          }
        }
        if (useLSB) {
          m_scope->getContainingFunction()->setNextLSB(true);
        }
        break;
      }
      case Expression::KindOfUnaryOpExpression:
        if (Option::EnableEval > Option::NoEval && spc(UnaryOpExpression, e)->
            getOp() == T_EVAL) {
          m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
        }
        break;

      case Expression::KindOfBinaryOpExpression: {
        BinaryOpExpressionPtr b(spc(BinaryOpExpression, e));
        if (b->getOp() == T_INSTANCEOF) {
          ExpressionPtr s = b->getExp2();
          if (s->is(Expression::KindOfScalarExpression)) {
            ScalarExpressionPtr scalar(spc(ScalarExpression, s));
            if (!scalar->isQuoted() && scalar->getString() == "static") {
              m_scope->getContainingFunction()->setNextLSB(true);
            }
          }
        } else if (unused && b->isShortCircuitOperator()) {
          b->getExp2()->setUnused(true);
        }
        break;
      }
      case Expression::KindOfQOpExpression: {
        QOpExpressionPtr q(spc(QOpExpression, e));
        if (unused) {
          if (ExpressionPtr t1 = q->getYes()) t1->setUnused(true);
          q->getNo()->setUnused(true);
        }
        break;
      }
      default:
        break;
    }
  }
  return kidCost;
}

void AliasManager::gatherInfo(AnalysisResultConstPtr ar, MethodStatementPtr m) {
  m_arp = ar;
  FunctionScopeRawPtr func = m->getFunctionScope();
  m_scope = func;
  m_variables = func->getVariables();
  m_variables->clearUsed();
  m_inPseudoMain = func->inPseudoMain();

  if (ExpressionListPtr pPtr = m->getParams()) {
    ExpressionList &params = *pPtr;
    for (int i = params.getCount(); i--; ) {
      ParameterExpressionPtr p = spc(ParameterExpression, params[i]);
      if (Symbol *sym = m_variables->getSymbol(p->getName())) {
        if (sym->isCallTimeRef() || p->isRef()) {
          sym->setReferenced();
        }
      }
    }
  }
  if (ExpressionListPtr useVars = func->getClosureVars()) {
    for (int i = 0; i < useVars->getCount(); i++) {
      ParameterExpressionPtr p = dpc(ParameterExpression, (*useVars)[i]);
      assert(p);
      if (Symbol *sym = m_variables->getSymbol(p->getName())) {
        if (p->isRef()) {
          sym->setReferenced();
          sym->setUsed();
        }
      }
    }
  }

  if (!m_inPseudoMain) {
    m_variables->clearAttribute(VariableTable::ContainsLDynamicVariable);
    m_variables->clearAttribute(VariableTable::ContainsDynamicVariable);
    m_variables->clearAttribute(VariableTable::ContainsCompact);
    m_variables->clearAttribute(VariableTable::ContainsExtract);
  }

  func->setContainsThis(false);
  func->setContainsBareThis(false);
  func->setNextLSB(false);

  int i, nkid = m->getKidCount();
  for (i = 0; i < nkid; i++) {
    ConstructPtr cp(m->getNthKid(i));
    collectAliasInfoRecur(cp, false);
  }

  if (!func->nextLSB()) {
    if (func->usesLSB()) {
      func->clearUsesLSB();
      m_changes = true;
    }
  }
}

int AliasManager::optimize(AnalysisResultConstPtr ar, MethodStatementPtr m) {
  gatherInfo(ar, m);

  bool runCanon = Option::LocalCopyProp || Option::EliminateDeadCode;

  if (runCanon) {
    for (int i = 0, nkid = m->getKidCount(); i < nkid; i++) {
      if (i) {
        clear();
        m_cleared = false;
      }
      canonicalizeKid(m, m->getNthKid(i), i);
      killLocals();
    }
  }

  return m_replaced ? -1 : m_changes ? 1 : 0;
}

void AliasManager::beginInExpression(StatementPtr parent, ExpressionPtr kid) {
  assert(parent);
  if (m_exprIdx == -1) {
    assert(!m_exprParent);
    m_exprIdx    = m_accessList.size();
    m_exprParent = parent;
    m_expr       = kid;
  }
}

void AliasManager::endInExpression(StatementPtr requestor) {
  assert(requestor);
  if (m_exprIdx != -1) {
    assert(m_exprIdx >= 0 && m_exprParent);
    if (requestor == m_exprParent) {
      m_exprIdx = -1;
      m_exprParent.reset();
      m_expr.reset();
    }
  }
}
