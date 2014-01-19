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

#include "hphp/compiler/analysis/alias_manager.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/function_scope.h"
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
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/expression.h"
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
#include "hphp/compiler/analysis/control_flow.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/data_flow.h"
#include "hphp/compiler/analysis/dictionary.h"
#include "hphp/compiler/analysis/expr_dict.h"
#include "hphp/compiler/analysis/live_dict.h"
#include "hphp/compiler/analysis/ref_dict.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/parser/hphp.tab.hpp"
#include "hphp/parser/location.h"
#include "hphp/util/util.h"

#define spc(T,p) static_pointer_cast<T>(p)
#define dpc(T,p) dynamic_pointer_cast<T>(p)

using namespace HPHP;
using std::string;

///////////////////////////////////////////////////////////////////////////////

AliasManager::AliasManager(int opt) :
    m_bucketList(0), m_nextID(1), m_changes(0), m_replaced(0),
    m_wildRefs(false), m_nrvoFix(0), m_inCall(0), m_inlineAsExpr(true),
    m_noAdd(false), m_preOpt(opt<0), m_postOpt(opt>0),
    m_cleared(false), m_inPseudoMain(false), m_genAttrs(false),
    m_hasDeadStore(false), m_hasChainRoot(false),
    m_hasTypeAssertions(false), m_graph(0), m_exprIdx(-1) {
}

AliasManager::~AliasManager() {
  delete m_graph;
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
    } else if (opt == "copyprop") {
      Option::CopyProp = val;
    } else if (opt == "string") {
      Option::StringLoopOpts = val;
    } else if (opt == "inline") {
      Option::AutoInline = val ? 1 : -1;
    } else if (opt == "cflow") {
      Option::ControlFlow = val;
    } else if (opt == "coalesce") {
      Option::VariableCoalescing = val;
    } else if (val && (opt == "all" || opt == "none")) {
      val = opt == "all";
      Option::EliminateDeadCode = val;
      Option::LocalCopyProp = val;
      Option::AutoInline = val ? 1 : -1;
      Option::ControlFlow = val;
      Option::CopyProp = val;
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
  if (!m_genAttrs) e->setCanonID(m_nextID++);
}

bool AliasManager::insertForDict(ExpressionPtr e) {
  ExpressionPtr c = getCanonical(e);
  if (c == e) return true;
  e->setCanonID(c->getCanonID());
  return false;
}

ExpressionPtr AliasManager::getCanonical(ExpressionPtr e) {
  unsigned val = (e->getCanonHash() % MaxBuckets);

  BucketMapEntry &em = m_bucketMap[val];
  em.link(m_bucketList);

  ExpressionPtr c = em.find(e);

  if (!c) {
    add(em, e);
    c = e;
    if (!m_genAttrs) e->setCanonPtr(ExpressionPtr());
  } else if (!m_genAttrs) {
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
  ExpressionPtr e(new ScalarExpression(BlockScopePtr(), LocationPtr(),
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
      e(new ScalarExpression(BlockScopePtr(), LocationPtr(),
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
            bool ok = (m_postOpt || !e->is(Expression::KindOfSimpleVariable));
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
            (sv->isNeededValid() ? sv->isNeeded() :
             !sv->getSymbol() || sv->getSymbol()->isNeeded())) {
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
                             ExpressionPtr &rep, int *flags /* = 0 */,
                             bool allowLval /* = false */) {
  BucketMapEntry &lvs = m_accessList;
  return findInterf0(rv, isLoad, rep, lvs.rbegin(), lvs.rend(),
      flags, allowLval);
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
      ExpressionPtr match(e->getNextCanonCsePtr());
      if (match && !match->isChainRoot()) {
        ExpressionPtr matchNext(match->getNextCanonCsePtr());
        if (!matchNext) {
          // make match a new CSE chain root
          match->setChainRoot();
          m_hasChainRoot = true;
        }
      }
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
      if (m_preOpt) ret = e->preOptimize(m_arp);
      if (m_postOpt) ret = e->postOptimize(m_arp);
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
    if (m_preOpt) ret = e->preOptimize(m_arp);
    if (m_postOpt) ret = e->postOptimize(m_arp);
    if (ret) {
      return canonicalizeRecurNonNull(ret);
    }
  }

  e->setVisited();
  e->clearLocalExprAltered();
  e->setCanonPtr(ExpressionPtr());
  e->clearChainRoot();
  e->setCanonID(0);

  switch (e->getKindOf()) {
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
    case Expression::KindOfListAssignment:
      markAllLocalExprAltered(e);
      add(m_accessList, e);
      break;

    case Expression::KindOfAssignmentExpression: {
      AssignmentExpressionPtr ae = spc(AssignmentExpression,e);
      if (e->hasContext(Expression::DeadStore)) {
        e->recomputeEffects();
        return ae->replaceValue(ae->getValue());
      }
      markAllLocalExprAltered(ae->getVariable());
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
                                  b->getScope(), b->getLocation(),
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
                  ExpressionPtr inc
                    (new ScalarExpression(u->getScope(), u->getLocation(),
                                          T_LNUMBER, string("1")));

                  val = ExpressionPtr(
                    new BinaryOpExpression(u->getScope(), u->getLocation(),
                                           val, inc,
                                           u->getOp() == T_INC ? '+' : '-'));

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
        markAllLocalExprAltered(e);
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
                              ExpressionListPtr el(
                                new ExpressionList(
                                  a->getScope(), a->getLocation(),
                                  ExpressionList::ListKindWrapped));
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
      bool doArrayCSE = Option::ArrayAccessIdempotent && m_postOpt;
      if (e->hasContext(Expression::AccessContext)) {
        always_assert(doAccessChains);
        if (e->getContext() & (Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter|
                               Expression::DeepReference|
                               Expression::UnsetContext)) {
          ExpressionPtr rep;
          int interf =
            findInterf(e, true, rep, nullptr, doArrayCSE);
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
          findInterf(e, true, rep, nullptr, doArrayCSE);
        if (!m_inPseudoMain && interf == DisjointAccess && !m_cleared &&
            e->is(Expression::KindOfSimpleVariable) &&
            !e->isThis()) {
          Symbol *s = spc(SimpleVariable, e)->getSymbol();
          if (s && !s->isParameter() && !s->isClosureVar()) {
            rep = e->makeConstant(m_arp, "null");
            Compiler::Error(Compiler::UseUndeclaredVariable, e);
            if (m_variables->getAttribute(VariableTable::ContainsCompact)) {
              rep = ExpressionPtr(
                new UnaryOpExpression(e->getScope(), e->getLocation(),
                                      rep, T_UNSET_CAST, true));
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
            if (doArrayCSE) setCanonPtrForArrayCSE(e, rep);
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
          canonicalizeNonNull(ExpressionPtr(
                                new BinaryOpExpression(
                                  bop->getScope(), bop->getLocation(),
                                  lhs, rhs, rop))));
      }
      if (rop) {
        markAllLocalExprAltered(bop);
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
                ExpressionPtr rhs(
                  (new BinaryOpExpression(e->getScope(), e->getLocation(),
                                          op0->clone(), op1->clone(), rop)));

                lhs = lhs->clone();
                lhs->clearContext(Expression::OprLValue);
                return e->replaceValue(
                  canonicalizeRecurNonNull(
                    ExpressionPtr(new AssignmentExpression(
                                    e->getScope(), e->getLocation(),
                                    lhs, rhs, false))));
              }
              alt = spc(AssignmentExpression,alt)->getVariable();
              break;
            }
            case Expression::KindOfBinaryOpExpression: {
              BinaryOpExpressionPtr b2(spc(BinaryOpExpression, alt));
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
                                      bop->getScope(), bop->getLocation(),
                                      op0->clone(), bop->getExp2(),
                                      rop));
                    ExpressionPtr b(new BinaryOpExpression(
                                      bop->getScope(), bop->getLocation(),
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
            markAllLocalExprAltered(e);
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
  case Statement::KindOfTraitRequireStatement:
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
        ExpressionPtr e(new ScalarExpression(BlockScopePtr(), LocationPtr(),
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
  if (m_preOpt) rep = s->preOptimize(m_arp);
  if (m_postOpt) rep = s->postOptimize(m_arp);
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
        m_inlineAsExpr = false;
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
    bool inlineOk = false;
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
      case Statement::KindOfExpStatement:
      case Statement::KindOfStatementList:
        inlineOk = true;
        break;
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
    if (!inlineOk) {
      m_inlineAsExpr = false;
    }
  } else {
    ExpressionPtr e = spc(Expression, cs);
    if (e->isNoRemove()) m_hasTypeAssertions = true;
    if (e->getContext() & Expression::DeadStore) m_hasDeadStore = true;
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
            } else if (m_graph) {
              int &id = m_gidMap["v:this"];
              if (!id) id = m_gidMap.size();
              e->setCanonID(id);
            }
          } else if (m_graph) {
            m_objMap[sym->getName()] = sv;
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
        if (m_graph) {
          const std::string &name = p->getClassName();
          if (!name.empty()) {
            int &id = m_gidMap[name];
            if (!id) id = m_gidMap.size();
            e->setCanonID(id);
          }
        }
        break;
      }
      case Expression::KindOfClosureExpression:
        // currently disable inlining for scopes with closure
        // expressions. TODO: revisit this later
        m_inlineAsExpr = false;
        break;
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
  func->setInlineSameContext(false);
  func->setNextLSB(false);

  int i, nkid = m->getKidCount(), cost = 0;
  for (i = 0; i < nkid; i++) {
    ConstructPtr cp(m->getNthKid(i));
    int c = collectAliasInfoRecur(cp, false);
    if (cp == m->getStmts()) cost = c;
  }

  if (m_inlineAsExpr) {
    if (cost > Option::AutoInline ||
        func->isVariableArgument() ||
        m_variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
        m_variables->getAttribute(VariableTable::ContainsExtract) ||
        m_variables->getAttribute(VariableTable::ContainsCompact) ||
        m_variables->getAttribute(VariableTable::ContainsGetDefinedVars)) {
      m_inlineAsExpr = false;
    }
  }
  func->setInlineAsExpr(m_inlineAsExpr);

  if (!func->nextLSB()) {
    if (func->usesLSB()) {
      func->clearUsesLSB();
      m_changes = true;
    }
  } else {
    func->setInlineSameContext(true);
  }
}

static void markAvailable(ExpressionRawPtr e) {
  if (e->is(Expression::KindOfSimpleVariable)) {
    SimpleVariableRawPtr sv(spc(SimpleVariable,e));
    sv->setGuarded();
    sv->setNonNull();
  } else {
    StaticClassName *scn = dynamic_cast<StaticClassName*>(e.get());
    always_assert(scn);
    scn->setPresent();
  }
}

class TypeAssertionInserter {
public:
  explicit TypeAssertionInserter(AnalysisResultConstPtr ar) :
    m_ar(ar), m_changed(false) {
    BuildAssertionMap();
  }
  bool walk(StatementPtr stmt) {
    m_changed = false;
    createTypeAssertions(stmt);
    return m_changed;
  }
private:

  #define CONSTRUCT_EXP(from) \
      (from)->getScope(), (from)->getLocation()
  #define CONSTRUCT_STMT(from) \
      (from)->getScope(), (from)->getLabelScope(), (from)->getLocation()

  AnalysisResultConstPtr m_ar;
  bool m_changed;

  typedef std::pair<int, TypePtr> PosType;
  typedef hphp_hash_map<string, PosType, string_hash>
    StringPosTypeMap;
  typedef hphp_hash_map<string, SimpleVariablePtr, string_hash>
    StringVarMap;

  static StringPosTypeMap s_type_assertion_map;
  static Mutex s_type_assertion_map_mutex;

  static void BuildAssertionMap();

  ExpressionPtr createTypeAssertionsForKids(ExpressionPtr parent,
                                            int startIdx,
                                            bool &passStmt,
                                            bool &negate) {
    ExpressionPtr rep;
    for (int i = startIdx; i < parent->getKidCount(); i++) {
      rep = createTypeAssertions(parent->getNthExpr(i), passStmt, negate);
    }
    return rep;
  }

  void replaceExpression(
      ConstructPtr parent, ExpressionPtr rep, ExpressionPtr old, int kid) {
    assert(parent);
    assert(parent->getKidCount() > kid);
    assert(parent->getNthKid(kid) == old);

    if (!rep) return;

    rep->setActualType(old->getActualType());
    rep->setExpectedType(old->getExpectedType());
    rep->setImplementedType(old->getImplementedType());
    old->replaceValue(rep);

    parent->setNthKid(kid, rep);
    m_changed = true;
  }

  void replaceExpression(ExpressionPtr parent, ExpressionPtr rep, int kid) {
    replaceExpression(parent, rep, parent->getNthExpr(kid), kid);
  }

  // returns the rep node for target, after the insertion
  ExpressionPtr insertTypeAssertion(ExpressionPtr assertion,
                                    ExpressionPtr target) {
    assert(assertion);
    assert(target);
    assert(assertion->is(Expression::KindOfSimpleVariable) ||
           assertion->is(Expression::KindOfExpressionList));
    assert(assertion->isNoRemove());

    if (ExpressionListPtr el = dpc(ExpressionList, target)) {
      if (el->getListKind() == ExpressionList::ListKindComma) {
        assert(assertion->is(Expression::KindOfSimpleVariable));
        el->insertElement(assertion, el->getCount() - 1);
        return ExpressionPtr();
      }
    }

    if (ExpressionListPtr el = dpc(ExpressionList, assertion)) {
      ExpressionListPtr el0 = spc(ExpressionList, el->clone());
      el0->insertElement(target, el0->getCount());
      return el0;
    }

    ExpressionListPtr el(
        new ExpressionList(
          CONSTRUCT_EXP(target),
          ExpressionList::ListKindComma));
    if (target->isUnused()) {
      el->setUnused(true);
    }
    el->addElement(assertion);
    el->addElement(target);
    el->setNoRemove(); // since it contains an assertion
    return el;
  }

  ExpressionPtr newTypeAssertion(ExpressionPtr base, TypePtr t) {
    assert(base->is(Expression::KindOfSimpleVariable));
    ExpressionPtr assertion(base->clone());
    assertion->setAssertedType(t);
    assertion->setNoRemove();
    return assertion;
  }

  ExpressionPtr newInstanceOfAssertion(
      ExpressionPtr obj, ExpressionPtr clsName) {
    SimpleVariablePtr sv(extractAssertableVariable(obj));
    ScalarExpressionPtr se(dpc(ScalarExpression, clsName));
    if (sv && se) {
      const string &s = se->getLiteralString();
      if (s.empty()) return ExpressionPtr();
      if (interface_supports_array(s) ||
          interface_supports_string(s) ||
          interface_supports_int(s) ||
          interface_supports_double(s))  {
        // This could be a primitive type, so don't assert anything
        return ExpressionPtr();
      }
      TypePtr o(Type::CreateObjectType(Util::toLower(s)));

      // don't do specific type assertions for unknown classes
      ClassScopePtr cscope(o->getClass(m_ar, sv->getScope()));
      if (!cscope) return newTypeAssertion(sv, Type::Object);

      // don't do type assertions for interfaces, since
      // user classes don't necessarily extend the interface class
      // in C++
      if (cscope->isInterface()) {
        return newTypeAssertion(sv, Type::Object);
      }

      // also don't do type assertions for derived by dynamic
      // classes, since these classes will be DynamicObjectData
      // instances at runtime (so we cannot emit a fast pointer cast
      // safely)
      if (cscope->derivedByDynamic()) {
        return newTypeAssertion(sv, Type::Object);
      }

      return newTypeAssertion(sv, o);
    }
    return ExpressionPtr();
  }

  SimpleVariablePtr extractAssertableVariable(ExpressionPtr e) {
    assert(e);
    switch (e->getKindOf()) {
    case Expression::KindOfSimpleVariable:
      return spc(SimpleVariable, e);
    case Expression::KindOfAssignmentExpression:
      {
        AssignmentExpressionPtr ae(spc(AssignmentExpression, e));
        return extractAssertableVariable(ae->getVariable());
      }
    default:
      break;
    }
    return SimpleVariablePtr();
  }

  ExpressionPtr orAssertions(
      SimpleVariablePtr lhs,
      SimpleVariablePtr rhs) {
    assert(lhs && lhs->isNoRemove() && lhs->getAssertedType());
    assert(rhs && rhs->isNoRemove() && rhs->getAssertedType());
    assert(lhs->getName() == rhs->getName());
    TypePtr u(
        Type::Union(m_ar, lhs->getAssertedType(), rhs->getAssertedType()));
    ExpressionPtr lhs0(lhs->clone());
    lhs0->setAssertedType(u);
    return lhs0;
  }

  ExpressionPtr andAssertions(
      SimpleVariablePtr lhs,
      SimpleVariablePtr rhs) {
    assert(lhs && lhs->isNoRemove() && lhs->getAssertedType());
    assert(rhs && rhs->isNoRemove() && rhs->getAssertedType());
    assert(lhs->getName() == rhs->getName());
    TypePtr i(
        Type::Intersection(
          m_ar, lhs->getAssertedType(), rhs->getAssertedType()));
    ExpressionPtr lhs0(lhs->clone());
    lhs0->setAssertedType(i);
    return lhs0;
  }

  ExpressionPtr orAssertions(ExpressionPtr lhs, ExpressionPtr rhs) {
    if (!lhs || !rhs) return ExpressionPtr();

    assert(lhs->is(Expression::KindOfSimpleVariable) ||
           lhs->is(Expression::KindOfExpressionList));
    assert(rhs->is(Expression::KindOfSimpleVariable) ||
           rhs->is(Expression::KindOfExpressionList));

    if (lhs->is(Expression::KindOfSimpleVariable) &&
        rhs->is(Expression::KindOfSimpleVariable) &&
        spc(SimpleVariable, lhs)->getName() ==
        spc(SimpleVariable, rhs)->getName()) {
      return orAssertions(
          spc(SimpleVariable, lhs),
          spc(SimpleVariable, rhs));
    }

    return ExpressionPtr();
  }

  bool isAllScalar(ExpressionListPtr ep, int startIdx) {
    assert(ep && ep->getListKind() == ExpressionList::ListKindParam);
    for (int i = startIdx; i < ep->getCount(); i++) {
      ExpressionPtr c((*ep)[i]);
      if (c && !c->isScalar()) return false;
    }
    return true;
  }

  ExpressionPtr createTypeAssertions(ExpressionPtr from,
                                     bool &passStmt,
                                     bool &negate) {
    passStmt = false;
    negate   = false;
    if (!from) return ExpressionPtr();
    int startIdx     = 0;
    bool useKidValue = true;
    switch (from->getKindOf()) {
      case Expression::KindOfExpressionList:
        {
          ExpressionListPtr p(spc(ExpressionList, from));
          switch (p->getListKind()) {
            case ExpressionList::ListKindComma:
              return createTypeAssertions(p->listValue(), passStmt, negate);
            default:
              break;
          }
        }
        break;
      case Expression::KindOfSimpleFunctionCall:
        {
          SimpleFunctionCallPtr p(spc(SimpleFunctionCall, from));
          ExpressionListPtr ep(p->getParams());
          StringPosTypeMap::const_iterator it(
             s_type_assertion_map.find(p->getName()));
          if (it != s_type_assertion_map.end()) {
            int pos = it->second.first;
            TypePtr t(it->second.second);
            if (ep->getCount() - 1 >= pos && isAllScalar(ep, pos + 1)) {
              ExpressionPtr a0((*ep)[pos]);
              if (a0) {
                SimpleVariablePtr sv(extractAssertableVariable(a0));
                if (sv) {
                  bool passStmt;
                  bool negate;
                  createTypeAssertionsForKids(from, 2, passStmt, negate);
                  return newTypeAssertion(sv, t);
                }
              }
            }
          } else if (p->getName() == "is_a" &&
                     (ep->getCount() >= 2 && isAllScalar(ep, 1))) {
            // special case is_a
            bool passStmt;
            bool negate;
            createTypeAssertionsForKids(from, 2, passStmt, negate);
            return newInstanceOfAssertion((*ep)[0], (*ep)[1]);
          }
          startIdx = 2; // params
        }
        break;
      case Expression::KindOfQOpExpression:
        {
          QOpExpressionPtr qop(spc(QOpExpression, from));
          bool passStmt;
          bool negateChild;
          ExpressionPtr lAfter(
            createTypeAssertions(
              qop->getCondition(),
              passStmt,
              negateChild));
          if (lAfter) {
            if (negateChild || qop->getYes()) {
              replaceExpression(
                  qop,
                  insertTypeAssertion(
                    lAfter,
                    !negateChild ? qop->getYes() : qop->getNo()),
                  !negateChild ? 1 : 2);
            }
          }
          startIdx = 1; // yes expr
          useKidValue = false;
        }
        break;
      case Expression::KindOfBinaryOpExpression:
        {
          BinaryOpExpressionPtr binop(spc(BinaryOpExpression, from));
          switch (binop->getOp()) {
            case T_LOGICAL_AND:
            case T_BOOLEAN_AND:
              {
                // we cannot push the LHS assertion into the expr
                // past the RHS assertion, since the RHS assertion could
                // have changed the LHS assertion. for example:
                //
                // if (is_array($x) && ($x = 5)) { stmt1; }
                //
                // All we can do is to push the LHS assertion into the beginning
                // of the RHS assertion, and then return the RHS assertion. We
                // would need more analysis to do more (eg copy prop of
                // assertions)
                bool passStmt;
                bool negateChild;
                ExpressionPtr lhsAfter(
                  createTypeAssertions(
                    binop->getExp1(),
                    passStmt,
                    negateChild));
                if (lhsAfter && !negateChild) {
                  replaceExpression(
                    binop,
                    insertTypeAssertion(lhsAfter, binop->getExp2()),
                    1);
                }
                ExpressionPtr rhsAfter(
                  createTypeAssertions(
                    binop->getExp2(),
                    passStmt,
                    negateChild));
                return negateChild ? ExpressionPtr() : rhsAfter;
              }
            case T_LOGICAL_OR:
            case T_BOOLEAN_OR:
              {
                bool passStmt;
                bool negateChild1;
                bool negateChild2;
                ExpressionPtr lhsAfter(
                  createTypeAssertions(
                    binop->getExp1(),
                    passStmt,
                    negateChild1));
                ExpressionPtr rhsAfter(
                  createTypeAssertions(
                    binop->getExp2(),
                    passStmt,
                    negateChild2));
                if (lhsAfter && rhsAfter && !negateChild1 && !negateChild2) {
                  return orAssertions(lhsAfter, rhsAfter);
                }
                return ExpressionPtr();
              }
            case T_INSTANCEOF:
              {
                ExpressionPtr r(newInstanceOfAssertion(
                    binop->getExp1(), binop->getExp2()));
                if (r) return r;
              }
          }
        }
        break;
      case Expression::KindOfUnaryOpExpression:
        {
          UnaryOpExpressionPtr unop(spc(UnaryOpExpression, from));
          switch (unop->getOp()) {
            case '!':
              {
                bool passStmt;
                ExpressionPtr after(
                  createTypeAssertions(
                    unop->getExpression(), passStmt, negate));
                negate = !negate;
                return after;
              }
            default:
              // do the default behavior
              break;
          }
        }
        break;
      case Expression::KindOfAssignmentExpression:
        {
          // handle $x = (type) $x explicitly,
          // since our current type inference will not be
          // able to deal with it optimally
          AssignmentExpressionPtr ap(spc(AssignmentExpression, from));
          SimpleVariablePtr sv(dpc(SimpleVariable, ap->getVariable()));
          UnaryOpExpressionPtr uop(dpc(UnaryOpExpression, ap->getValue()));
          if (sv && uop) {
            TypePtr castType(uop->getCastType());
            if (castType) {
              if (SimpleVariablePtr sv0 =
                  dpc(SimpleVariable, uop->getExpression())) {
                if (sv->getName() == sv0->getName()) {
                  passStmt = true;
                  return newTypeAssertion(sv, castType);
                }
              }
            }
          }
          useKidValue = false;
        }
        break;
      default:
        break;
    }
    ExpressionPtr result(
        createTypeAssertionsForKids(from, startIdx, passStmt, negate));
    return useKidValue ? result : ExpressionPtr();
  }

  void insertTypeAssertion(
      ExpressionPtr assertion,
      StatementPtr stmt,
      int idx = 0) {
    if (!stmt) return;

    m_changed = true;
    switch (stmt->getKindOf()) {
    case Statement::KindOfBlockStatement:
      {
        BlockStatementPtr blockPtr(spc(BlockStatement, stmt));
        insertTypeAssertion(assertion, blockPtr->getStmts());
      }
      break;
    case Statement::KindOfStatementList:
      {
        StatementListPtr slistPtr(spc(StatementList, stmt));
        slistPtr->insertElement(
          ExpStatementPtr(
            new ExpStatement(
              CONSTRUCT_STMT(slistPtr), assertion)),
          idx);
      }
      break;
    case Statement::KindOfExpStatement:
      {
        ExpStatementPtr es(spc(ExpStatement, stmt));
        ExpressionPtr rep(
            insertTypeAssertion(assertion, es->getExpression()));
        replaceExpression(es, rep, es->getExpression(), 0);
      }
      break;
    case Statement::KindOfReturnStatement:
      {
        ReturnStatementPtr rs(spc(ReturnStatement, stmt));
        if (rs->hasRetExp()) {
          ExpressionPtr rep(
              insertTypeAssertion(assertion, rs->getRetExp()));
          replaceExpression(rs, rep, rs->getRetExp(), 0);
        }
      }
      break;
    default:
      // this is something like a continue statement,
      // in which case we don't do anything
      break;
    }
  }

  ExpressionPtr createTypeAssertions(StatementPtr e) {
    if (!e) return ExpressionPtr();
    if (FunctionWalker::SkipRecurse(e)) return ExpressionPtr();

    ExpressionPtr loopCond;
    StatementPtr loopBody;
    std::vector<ExpressionPtr> needed;
    switch (e->getKindOf()) {
    case Statement::KindOfIfStatement:
      {
        IfStatementPtr ifStmt(spc(IfStatement, e));
        StatementListPtr branches(ifStmt->getIfBranches());
        if (branches) {
          for (int i = 0; i < branches->getCount(); i++) {
            IfBranchStatementPtr branch(
                dpc(IfBranchStatement, (*branches)[i]));
            assert(branch);
            if (branch->getCondition()) {
              bool passStmt;
              bool negate;
              ExpressionPtr after(
                createTypeAssertions(
                  branch->getCondition(), passStmt, negate));
              if (after) {
                if (!negate) {
                  insertTypeAssertion(after, branch->getStmt());
                } else {
                  // insert into the next branch:
                  //   * if a condition exists, put it in the condition.
                  //   * if there is no condition, put it in the body.
                  //   * if this is the last branch, create a branch after
                  //     with no condition, and put it in the body.

                  if (i < branches->getCount() - 1) {
                    IfBranchStatementPtr nextBranch(
                        dpc(IfBranchStatement, (*branches)[i + 1]));
                    assert(nextBranch);
                    if (nextBranch->getCondition()) {
                      ExpressionPtr old(nextBranch->getCondition());
                      replaceExpression(
                          nextBranch,
                          insertTypeAssertion(after, old),
                          old,
                          0);
                    } else {
                      // next branch must be the last branch
                      assert(i + 1 == branches->getCount() - 1);
                      insertTypeAssertion(after, nextBranch->getStmt());
                    }
                  } else {
                    ExpStatementPtr newExpStmt(
                      new ExpStatement(
                        CONSTRUCT_STMT(branch),
                        after));

                    IfBranchStatementPtr newBranch(
                      new IfBranchStatement(
                        CONSTRUCT_STMT(branch),
                        ExpressionPtr(), newExpStmt));

                    branches->addElement(newBranch);
                    break;
                  }
                }
              }
            }
          }
          for (int i = 0; i < branches->getCount(); i++) {
            IfBranchStatementPtr branch(
                dpc(IfBranchStatement, (*branches)[i]));
            assert(branch);
            if (branch->getStmt()) {
              createTypeAssertions(branch->getStmt());
            }
          }
        }
      }
      return ExpressionPtr();
    case Statement::KindOfForStatement: {
      ForStatementPtr fs(static_pointer_cast<ForStatement>(e));
      loopCond = fs->getCondExp();
      loopBody = fs->getBody();
      needed.push_back(fs->getInitExp());
      needed.push_back(fs->getIncExp());
      goto loop_stmt;
    }
    case Statement::KindOfWhileStatement: {
      WhileStatementPtr ws(static_pointer_cast<WhileStatement>(e));
      loopCond = ws->getCondExp();
      loopBody = ws->getBody();
      goto loop_stmt;
    }
    case Statement::KindOfDoStatement: {
      DoStatementPtr ds(static_pointer_cast<DoStatement>(e));
      loopCond = ds->getCondExp();
      loopBody = ds->getBody();
    }

loop_stmt:
      {
        if (loopCond) {
          bool passStmt;
          bool negate;
          ExpressionPtr after(createTypeAssertions(loopCond, passStmt, negate));
          if (after && !negate) {
            insertTypeAssertion(after, loopBody);
          }
        }
        for (auto it = needed.begin(); it != needed.end(); ++it) {
          ExpressionPtr k(dpc(Expression, *it));
          if (k) {
            bool passStmt;
            bool negate;
            createTypeAssertions(k, passStmt, negate);
          }
        }
        createTypeAssertions(loopBody);
      }
      return ExpressionPtr();
    case Statement::KindOfStatementList:
      {
        StatementListPtr slist(spc(StatementList, e));
        int i = 0;
        while (i < slist->getCount()) {
          ExpressionPtr next(createTypeAssertions((*slist)[i]));
          if (next) {
            insertTypeAssertion(next, slist, i + 1);
            i += 2; // skip over the inserted node
          } else {
            i++;
          }
        }
      }
      return ExpressionPtr();
    case Statement::KindOfExpStatement:
      {
        ExpStatementPtr es(dpc(ExpStatement, e));
        bool passStmt;
        bool negate;
        ExpressionPtr after(
            createTypeAssertions(es->getExpression(), passStmt, negate));
        return passStmt && !negate ? after : ExpressionPtr();
      }
    default:
      break;
    }
    for (int i = 0; i < e->getKidCount(); i++) {
      ConstructPtr kid(e->getNthKid(i));
      if (!kid) continue;
      if (StatementPtr s = dpc(Statement, kid)) {
        createTypeAssertions(s);
      } else if (ExpressionPtr e = dpc(Expression, kid)) {
        bool passStmt;
        bool negate;
        createTypeAssertions(e, passStmt, negate);
      } else {
        assert(false);
      }
    }
    return ExpressionPtr();
  }

};

TypeAssertionInserter::StringPosTypeMap
  TypeAssertionInserter::s_type_assertion_map;
Mutex
  TypeAssertionInserter::s_type_assertion_map_mutex;

void TypeAssertionInserter::BuildAssertionMap() {
  Lock lock(s_type_assertion_map_mutex);
  if (s_type_assertion_map.empty()) {
    s_type_assertion_map["is_array"]   = PosType(0, Type::Array);
    s_type_assertion_map["is_string"]  = PosType(0, Type::String);
    s_type_assertion_map["is_object"]  = PosType(0, Type::Object);

    s_type_assertion_map["is_int"]     = PosType(0, Type::Int64);
    s_type_assertion_map["is_integer"] = PosType(0, Type::Int64);
    s_type_assertion_map["is_long"]    = PosType(0, Type::Int64);

    s_type_assertion_map["is_double"]  = PosType(0, Type::Double);
    s_type_assertion_map["is_float"]   = PosType(0, Type::Double);
    s_type_assertion_map["is_real"]    = PosType(0, Type::Double);

    s_type_assertion_map["is_bool"]    = PosType(0, Type::Boolean);

    s_type_assertion_map["is_numeric"] = PosType(0, Type::Numeric);

    s_type_assertion_map["in_array"]   = PosType(1, Type::Array);

    // even though the PHP docs say that in PHP 5.3, this function
    // only accepts an array for the search, Zend PHP 5.3 still
    // accepts objects. Quite a bummer.
    s_type_assertion_map["array_key_exists"] =
      PosType(1,
          TypePtr(
            new Type(
              (Type::KindOf)(Type::KindOfArray|Type::KindOfObject))));
  }
}

class TypeAssertionRemover {
public:
  TypeAssertionRemover() : m_changed(false) {}
  bool walk(StatementPtr stmt) {
    m_changed = false;
    removeTypeAssertions(stmt);
    return m_changed;
  }
private:

  bool m_changed;

  void safeRepValue(ConstructPtr parent, int i,
      ExpressionPtr orig, ExpressionPtr rep) {
    if (orig == rep) return;
    m_changed = true;
    return parent->setNthKid(i, orig->replaceValue(rep));
  }

  void safeRepValue(ConstructPtr parent, int i,
      ConstructPtr orig, ConstructPtr rep) {
    if (orig == rep) return;
    m_changed = true;
    return parent->setNthKid(i, rep);
  }

  /**
   * Returns the replacement node for from
   */
  ConstructPtr removeTypeAssertions(ConstructPtr from) {
    if (!from) return ConstructPtr();
    if (StatementPtr s = dpc(Statement, from)) {
      return removeTypeAssertions(s);
    } else if (ExpressionPtr e = dpc(Expression, from)) {
      return removeTypeAssertions(e);
    } else {
      assert(false);
      return ConstructPtr();
    }
  }

  ExpressionPtr restoreExpType(ExpressionPtr orig, ExpressionPtr rep) {
    rep->setExpectedType(orig->getExpectedType());
    return rep;
  }

  ExpressionPtr removeTypeAssertions(ExpressionPtr from) {
    if (!from) return ExpressionPtr();
    switch (from->getKindOf()) {
    case Expression::KindOfExpressionList:
      {
        ExpressionListPtr p(spc(ExpressionList, from));
        if (p->getListKind() ==
            ExpressionList::ListKindComma) {
          for (int i = 0; i < p->getCount(); i++) {
            ExpressionPtr c((*p)[i]);
            ExpressionPtr rep(removeTypeAssertions(c));
            if (!rep) p->removeElement(i--);
            else      safeRepValue(p, i, c, rep);
          }
          if (p->isNoRemove()) {
            if (p->getCount() == 0) return ExpressionPtr();
            if (p->getCount() == 1) return restoreExpType(p, (*p)[0]);
          }
        }
      }
      break;
    case Expression::KindOfSimpleVariable:
      if (from->isNoRemove()) return ExpressionPtr();
      break;
    default:
      break;
    }
    for (int i = 0; i < from->getKidCount(); i++) {
      ExpressionPtr c(from->getNthExpr(i));
      ExpressionPtr rep(removeTypeAssertions(c));
      safeRepValue(from, i, c, rep);
    }
    return from;
  }

  StatementPtr removeTypeAssertions(StatementPtr from) {
    if (!from) return from;
    if (FunctionWalker::SkipRecurse(from)) return from;

    switch (from->getKindOf()) {
    case Statement::KindOfExpStatement:
      {
        ExpStatementPtr p(spc(ExpStatement, from));
        ExpressionPtr rep(removeTypeAssertions(p->getExpression()));
        if (!rep) return StatementPtr();
        else      safeRepValue(p, 0, p->getExpression(), rep);
      }
      break;
    case Statement::KindOfStatementList:
      {
        StatementListPtr p(spc(StatementList, from));
        for (int i = 0; i < p->getKidCount(); i++) {
          StatementPtr s((*p)[i]);
          StatementPtr rep(removeTypeAssertions(s));
          if (!rep) p->removeElement(i--);
          else      safeRepValue(from, i, s, rep);
        }
      }
      break;
    default:
      {
        for (int i = 0; i < from->getKidCount(); i++) {
          ConstructPtr c(from->getNthKid(i));
          ConstructPtr rep(removeTypeAssertions(c));
          safeRepValue(from, i, c, rep);
        }
      }
      break;
    }
    return from;
  }

};

static bool isNewResult(ExpressionPtr e) {
  if (!e) return false;
  if (e->is(Expression::KindOfNewObjectExpression)) return true;
  if (e->is(Expression::KindOfBinaryOpExpression)) {
    auto b = spc(BinaryOpExpression, e);
    if (b->getOp() == T_COLLECTION) {
      return true;
    }
  }
  if (e->is(Expression::KindOfAssignmentExpression)) {
    return isNewResult(spc(AssignmentExpression, e)->getValue());
  }
  if (e->is(Expression::KindOfExpressionList)) {
    return isNewResult(spc(ExpressionList, e)->listValue());
  }
  return false;
}

class ConstructTagger : public DataFlowWalker {
public:
  ConstructTagger(ControlFlowGraph *g,
                  std::map<std::string,int> &gidMap) :
      DataFlowWalker(g), m_gidMap(gidMap) {}

  void walk(MethodStatementPtr m) {
    DataFlowWalker::walk(*this);
    ControlBlock *b = m_graph.getDfBlock(1);
    std::map<std::string,int>::iterator it = m_gidMap.find("v:this");
    if (it != m_gidMap.end() && it->second) {
      b->setBit(DataFlow::PRefIn, it->second);
      b->setBit(DataFlow::PInitIn, it->second);
    }
    updateParamInfo(m->getParams(), Option::HardTypeHints);
    updateParamInfo(m->getFunctionScope()->getClosureVars(), false);
  }

  void updateParamInfo(ExpressionListPtr el, bool useDefaults) {
    if (!el) return;
    ControlBlock *b = m_graph.getDfBlock(1);
    for (int i = el->getCount(); i--; ) {
      ParameterExpressionPtr p =
        static_pointer_cast<ParameterExpression>((*el)[i]);
      std::map<std::string,int>::iterator it =
        m_gidMap.find("v:" + p->getName());
      if (it != m_gidMap.end() && it->second) {
        // NB: this is unsound if the user error handler swallows a
        // parameter typehint failure.  It's opt-in via compiler
        // options, though.
        if (useDefaults && p->hasTypeHint() && !p->defaultValue()) {
          b->setBit(DataFlow::AvailIn, it->second);
        }
        b->setBit(DataFlow::PRefIn, it->second);
        b->setBit(DataFlow::PInitIn, it->second);
      }
    }
  }

  void processAccess(ExpressionPtr e) {
    int id = e->getCanonID();
    if (id) {
      if (e->isThis() && e->getAssertedType()) {
        markAvailable(e);
      }
      if (m_block->getBit(DataFlow::Available, id)) {
        markAvailable(e);
        e->clearAnticipated();
      } else {
        m_block->setBit(DataFlow::Available, id);
        e->setAnticipated();
      }
    } else {
      bool set = true, maybeRef = true;
      SimpleVariablePtr sv;
      if (e->is(Expression::KindOfSimpleVariable)) {
        sv = spc(SimpleVariable, e);
        set = (e->isThis() && e->hasContext(Expression::ObjectContext));
        if (e->getAssertedType()) {
          markAvailable(sv);
          set = true;
        }
      } else {
        if (e->is(Expression::KindOfObjectMethodExpression)) {
          ExpressionPtr a(spc(ObjectMethodExpression, e)->getObject());
          while (true) {
            if (a->is(Expression::KindOfObjectPropertyExpression)) {
              a = spc(ObjectPropertyExpression, a)->getObject();
            } else if (a->is(Expression::KindOfArrayElementExpression)) {
              a = spc(ArrayElementExpression, a)->getVariable();
            } else if (a->is(Expression::KindOfAssignmentExpression)) {
              a = spc(AssignmentExpression, a)->getValue();
            } else if (a->is(Expression::KindOfExpressionList)) {
              a = spc(ExpressionList, a)->listValue();
            } else {
              break;
            }
          }
          if (a->is(Expression::KindOfSimpleVariable)) {
            sv = spc(SimpleVariable, a);
          }
        } else if (e->is(Expression::KindOfObjectPropertyExpression)) {
          ObjectPropertyExpressionPtr op(spc(ObjectPropertyExpression, e));
          if (op->getObject()->is(Expression::KindOfSimpleVariable)) {
            sv = spc(SimpleVariable, op->getObject());
            set = false;
          }
        } else if (e->is(Expression::KindOfAssignmentExpression)) {
          AssignmentExpressionPtr ae(spc(AssignmentExpression, e));
          if (ae->getVariable()->is(Expression::KindOfSimpleVariable)) {
            sv = spc(SimpleVariable, ae->getVariable());
            set = isNewResult(ae->getValue());
            if (!sv->couldBeAliased() &&
                (ae->getValue()->isScalar() ||
                 (ae->getValue()->getActualType() &&
                  ae->getValue()->getActualType()->getKindOf() <
                  Type::KindOfString))) {
              maybeRef = false;
            }
          }
        }
        if (sv && sv->isThis()) sv.reset();
      }
      if (sv) {
        id = m_gidMap["v:" + sv->getName()];
        if (id) {
          if (sv == e) {
            sv->clearAnticipated();
            if (m_block->getBit(DataFlow::Killed, id)) {
              sv->setKilled();
            } else {
              sv->clearKilled();
            }
            if (m_block->getBit(DataFlow::Referenced, id)) {
              sv->setRefCounted();
              sv->setKilled();
            }
            if (m_block->getBit(DataFlow::Inited, id)) {
              sv->setInited();
              sv->setKilled();
            }
            if (sv->hasAllContext(Expression::UnsetContext|
                                  Expression::LValue)) {
              m_block->setBit(DataFlow::Available, id, false);
              m_block->setBit(DataFlow::Altered, id, true);
              m_block->setBit(DataFlow::Killed, id, true);
              m_block->setBit(DataFlow::Referenced, id, false);
              m_block->setBit(DataFlow::Inited, id, false);
              return;
            }
            bool ref = sv->hasAnyContext(Expression::RefValue|
                                         Expression::InvokeArgument|
                                         Expression::DeepReference|
                                         Expression::DeepAssignmentLHS|
                                         Expression::DeepOprLValue);

            bool mod =
              sv->hasAllContext(Expression::Declaration) ||
              sv->hasAnyContext(Expression::AssignmentLHS|
                                Expression::OprLValue) ||
              (!ref && sv->hasContext(Expression::LValue));

            if (mod) {
              m_block->setBit(DataFlow::Available, id, false);
              m_block->setBit(DataFlow::Altered, id, true);
            }
            if (mod || ref) {
              m_block->setBit(DataFlow::Referenced, id, true);
              m_block->setBit(DataFlow::Inited, id, true);
              bool kill = true;

              if (!mod) {
                if (TypePtr act = sv->getActualType()) {
                  if (sv->hasContext(Expression::ObjectContext)) {
                    if (act->is(Type::KindOfObject)) kill = false;
                  } else if (sv->hasContext(Expression::AccessContext)) {
                    if (act->is(Type::KindOfArray)) kill = false;
                  }
                }
              }
              if (kill) {
                m_block->setBit(DataFlow::Killed, id, true);
                return;
              }
            }

            if (!sv->couldBeAliased()) {
              if (m_block->getBit(DataFlow::Available, id)) {
                markAvailable(sv);
              } else {
                if (!m_block->getBit(DataFlow::Altered, id)) {
                  sv->setAnticipated();
                }
                if (set && !sv->hasAnyContext(Expression::ExistContext|
                                              Expression::UnsetContext)) {
                  m_block->setBit(DataFlow::Available, id, true);
                }
              }
            }
          } else {
            if (!maybeRef) {
              assert(m_block->getBit(DataFlow::Killed, id));
              m_block->setBit(DataFlow::Referenced, id, false);
            }
            if (set && !sv->hasAnyContext(Expression::ExistContext|
                                          Expression::UnsetContext)) {
              m_block->setBit(DataFlow::Available, id, true);
            }
          }
        }
      }
    }
  }

  int after(ConstructRawPtr cp) {
    if (ReturnStatementPtr rs = dynamic_pointer_cast<ReturnStatement>(cp)) {
      int id = m_gidMap["v:this"];
      if (id && m_block->getBit(DataFlow::Available, id)) {
        cp->setGuarded();
      }
    }
    return DataFlowWalker::after(cp);
  }
private:
  std::map<std::string,int> &m_gidMap;
};

class ConstructMarker : public DataFlowWalker {
public:
  ConstructMarker(ControlFlowGraph *g, std::map<std::string,int> &gidMap) :
      DataFlowWalker(g), m_gidMap(gidMap),
      m_top(g->getMethod()->getStmts()) {}

  void walk() {
    DataFlowWalker::walk(*this);
  }

  void processAccess(ExpressionPtr e) {
    int id = e->getCanonID();
    if (!id && e->is(Expression::KindOfSimpleVariable) &&
        (e->isAnticipated() || !e->isKilled())) {
      id = m_gidMap["v:" + static_pointer_cast<SimpleVariable>(e)->getName()];
    }
    if (id) {
      if (!m_block->getDfn()) return;
      if (e->isAnticipated() && m_block->getBit(DataFlow::AvailIn, id)) {
        markAvailable(e);
      }
      if (e->is(Expression::KindOfSimpleVariable) && !e->isKilled()) {
        if (m_block->getBit(DataFlow::PRefIn, id)) {
          e->setRefCounted();
        } else {
          e->clearRefCounted();
        }
        if (m_block->getBit(DataFlow::PInitIn, id)) {
          e->setInited();
        } else {
          e->clearInited();
        }
      }
    }
  }

  int after(ConstructRawPtr cp) {
    if (cp == m_top ||
        static_pointer_cast<Statement>(cp)->is(
          Statement::KindOfReturnStatement)) {
      if (m_block->getDfn()) {
        int id = m_gidMap["v:this"];
        if (id && m_block->getBit(DataFlow::AvailOut, id)) {
          cp->setGuarded();
        }
      }
    }

    if (auto rs = dynamic_pointer_cast<ReturnStatement>(cp)) {
      std::vector<std::string> lnames;
      VariableTableConstPtr vars = cp->getFunctionScope()->getVariables();
      vars->getLocalVariableNames(lnames);
      for (auto& l : lnames) {
        int id = m_gidMap["v:" + l];
        if (id && !m_block->getBit(DataFlow::PInitOut, id)) {
          rs->addNonRefcounted(l);
        } else {
          auto sym = vars->getSymbol(l);
          auto dt = vars->getFinalType(l)->getDataType();
          if (!sym->isStatic() && dt != KindOfUnknown &&
              !IS_REFCOUNTED_TYPE(dt)) {
            rs->addNonRefcounted(l);
          }
        }
      }
    }

    return DataFlowWalker::after(cp);
  }
private:
  std::map<std::string,int> &m_gidMap;
  ConstructPtr m_top;
};

class Propagater : public ControlFlowGraphWalker {
public:
  Propagater(ControlFlowGraph *g, ExprDict &d, bool doTypeProp) :
      ControlFlowGraphWalker(g), m_dict(d),
      m_changed(false), m_doTypeProp(doTypeProp) {}

  bool walk() { ControlFlowGraphWalker::walk(*this); return m_changed; }
  int afterEach(ConstructRawPtr p, int i, ConstructPtr kid) {
    if (ExpressionRawPtr e = dynamic_pointer_cast<Expression>(kid)) {
      if (e->isTypeAssertion()) return WalkContinue; // nothing to do

      bool safeForProp =
          e->isAnticipated() &&
          !(e->getContext() & (Expression::LValue|
                               Expression::OprLValue|
                               Expression::AssignmentLHS|
                               Expression::RefValue|
                               Expression::UnsetContext|
                               Expression::DeepReference));
      if (safeForProp) {
        if (ExpressionPtr rep = m_dict.propagate(e)) {
          m_changed = true;
          rep = e->replaceValue(rep->clone());
          p->setNthKid(i, rep);
          return WalkContinue;
        }
      }

      if (m_doTypeProp && e->isAnticipated()) {
        TypePtr t = m_dict.propagateType(e);
        if (!t) return WalkContinue;
        if (safeForProp ||
            // $x[0] = ... where $x is not referenced
            // should be allowed to grab the type assertion
            // if it is an array assertion
            (t->is(Type::KindOfArray) &&
              e->is(Expression::KindOfSimpleVariable) &&
              e->hasContext(Expression::AccessContext) &&
              !spc(SimpleVariable, e)->couldBeAliased())) {
          e->setAssertedType(t);
        }
      }
    }

    return WalkContinue;
  }

  void beforeBlock(ControlBlock *b) {
    m_dict.beforePropagate(b);
  }
private:
  ExprDict &m_dict;
  bool m_changed;
  bool m_doTypeProp;
};

void AliasManager::doFinal(MethodStatementPtr m) {
  FunctionScopeRawPtr func = m->getFunctionScope();
  if (func->isRefReturn()) {
    m_nrvoFix = -1;
  } else if (m_nrvoFix > 0) {
    Symbol *sym = m_variables->getSymbol(m_returnVar);
    if (sym && !sym->isParameter() &&
        (m_wildRefs || sym->isReferenced() ||
         ((sym->isGlobal() || sym->isStatic()) &&
          m_variables->needLocalCopy(sym)))) {
      // do nothing
    } else {
      m_nrvoFix = -1;
    }
  }

  func->setNRVOFix(m_nrvoFix > 0);

  if (Option::StringLoopOpts && !m_wildRefs) {
    stringOptsRecur(m->getStmts());
  }
}

void AliasManager::performReferencedAndNeededAnalysis(MethodStatementPtr m) {
  always_assert(m_graph != nullptr);

  // bail out for pseudomain context
  if (m->getScope()->inPseudoMain()) return;

  RefDict rd(*this);
  rd.build(m);
  AttributeTagger<RefDict> rt(m_graph, rd);
  RefDictWalker rdw(m_graph);

  // referenced analysis
  rd.updateParams();
  rt.walk();
  DataFlow::ComputePartialReferenced(*m_graph);
  rdw.walk();

  rd.togglePass();
  rdw.togglePass();

  // needed analysis
  rd.updateParams();
  rt.walk();
  DataFlow::ComputePartialNeeded(*m_graph);
  rdw.walk();
}

int AliasManager::copyProp(MethodStatementPtr m) {
  if (m_graph == nullptr) createCFG(m);

  performReferencedAndNeededAnalysis(m);

  ExprDict ed(*this);
  m_genAttrs = true;
  ed.build(m);
  AttributeTagger<ExprDict> at(m_graph, ed);
  at.walk();
  m_genAttrs = false;

  DataFlow::ComputeAvailable(*m_graph);
  DataFlow::ComputeAnticipated(*m_graph);

  if (Option::DumpAst) m_graph->dump(m_arp);

  Propagater prop(m_graph, ed, m_preOpt);
  bool ret = prop.walk();
  deleteCFG();
  return ret;
}

void AliasManager::deleteCFG() {
  assert(m_graph != nullptr);
  delete m_graph;
  m_graph = nullptr;
}

void AliasManager::createCFG(MethodStatementPtr m) {
  assert(m_graph == nullptr);
  m_graph = ControlFlowGraph::buildControlFlow(m);
}

void AliasManager::insertTypeAssertions(AnalysisResultConstPtr ar,
                                        MethodStatementPtr m) {
  TypeAssertionInserter i(ar);
  i.walk(m->getStmts());

  if (Option::ControlFlow && Option::DumpAst) {
    if (m_graph != nullptr) deleteCFG();
    createCFG(m);
    printf("-------- BEGIN INSERTED -----------\n");
    m_graph->dump(m_arp);
    printf("-------- END   INSERTED -----------\n");
    deleteCFG();
  }
}

void AliasManager::removeTypeAssertions(AnalysisResultConstPtr ar,
                                        MethodStatementPtr m) {
  TypeAssertionRemover r;
  r.walk(m->getStmts());

  if (Option::ControlFlow && Option::DumpAst) {
    if (m_graph != nullptr) deleteCFG();
    createCFG(m);
    printf("-------- BEGIN REMOVED -----------\n");
    m_graph->dump(m_arp);
    printf("-------- END   REMOVED -----------\n");
    deleteCFG();
  }
}

int AliasManager::optimize(AnalysisResultConstPtr ar, MethodStatementPtr m) {
  m_hasTypeAssertions = false;
  gatherInfo(ar, m);

  bool runCanon = Option::LocalCopyProp || Option::EliminateDeadCode;

  if (runCanon && m_preOpt) {
    if (m_hasTypeAssertions) removeTypeAssertions(ar, m);
    insertTypeAssertions(ar, m);
  }

  if (runCanon && m_postOpt && m_hasTypeAssertions) {
    removeTypeAssertions(ar, m);
  }

  if (!m_hasDeadStore && Option::CopyProp) {
    if (copyProp(m)) return 1;
  }

  if (m_graph != nullptr) deleteCFG();

  m_hasChainRoot = false;

  if (runCanon) {
    for (int i = 0, nkid = m->getKidCount(); i < nkid; i++) {
      if (i) {
        clear();
        m_cleared = false;
      }
      canonicalizeKid(m, m->getNthKid(i), i);
      killLocals();
    }

    if (m_hasChainRoot && m_postOpt) {
      // need to do possible invalidation for label statements
      invalidateChainRoots(m->getStmts());
    }
  }

  if (!m_replaced && !m_changes && m_postOpt && !Option::ControlFlow) {
    doFinal(m);
  }

  return m_replaced ? -1 : m_changes ? 1 : 0;
}

void AliasManager::finalSetup(AnalysisResultConstPtr ar, MethodStatementPtr m) {
  if (Option::ControlFlow) {
    m_graph = ControlFlowGraph::buildControlFlow(m);
  }

  gatherInfo(ar, m);
  if (m_graph) {
    if (!m_inPseudoMain &&
        !m_variables->getAttribute(VariableTable::ContainsLDynamicVariable)) {
      for (std::map<string,SimpleVariablePtr>::iterator it =
             m_objMap.begin(), end = m_objMap.end(); it != end; ++it) {
        SimpleVariablePtr sv = it->second;
        const Symbol *sym = sv->getSymbol();
        int &id = m_gidMap["v:"+sym->getName()];
        assert(!id);
        id = m_gidMap.size();
      }
    }

    {
      static int rows[] = {
        DataFlow::Available, DataFlow::Altered,
        DataFlow::Inited, DataFlow::Referenced, DataFlow::Killed,
        DataFlow::AvailIn, DataFlow::AvailOut,
        DataFlow::PInitIn, DataFlow::PInitOut,
        DataFlow::PRefIn, DataFlow::PRefOut
      };
      m_graph->allocateDataFlow(m_gidMap.size()+1,
                                sizeof(rows)/sizeof(rows[0]), rows);
    }

    ConstructTagger ct(m_graph, m_gidMap);
    ct.walk(m);

    DataFlow::ComputeAvailable(*m_graph);
    DataFlow::ComputePartialReferenced(*m_graph);
    DataFlow::ComputePartialInited(*m_graph);

    ConstructMarker cm(m_graph, m_gidMap);
    cm.walk();

    if (Option::VariableCoalescing &&
        !m_inPseudoMain &&
        !m_variables->getAttribute(VariableTable::ContainsDynamicVariable)) {

      performReferencedAndNeededAnalysis(m);

      LiveDict ld(*this);
      m_genAttrs = true;
      ld.build(m);
      AttributeTagger<LiveDict> lt(m_graph, ld);
      lt.walk();
      ld.updateParams();
      m_genAttrs = false;
      DataFlow::ComputePartialAvailable(*m_graph);
      DataFlow::ComputePartialAnticipated(*m_graph);
      DataFlow::ComputeUsed(*m_graph);
      DataFlow::ComputePartialDying(*m_graph);

      if (Option::DumpAst) m_graph->dump(ar);

      ld.buildConflicts();
      if (ld.color(Type::Variant) ||
          ld.color(Type::String) ||
          ld.color(Type::Numeric) ||
          ld.color(Type::Primitive)) {
        ld.coalesce(m);
      }
      ld.shrinkWrap();
    }

    if (Option::DumpAst) m_graph->dump(ar);

    delete m_graph;
    m_graph = 0;
  }
  doFinal(m);
}

void AliasManager::invalidateChainRoots(StatementPtr s) {
  if (!s) return;
  if (FunctionWalker::SkipRecurse(s)) return;
  switch (s->getKindOf()) {
  case Statement::KindOfStatementList:
    {
      StatementListPtr slist(spc(StatementList, s));
      // start from the last statement -
      // if we find a child stmt with
      // a reachable label, we must invalidate
      // all statements before
      bool disable = false;
      for (int i = s->getKidCount(); i--; ) {
        StatementPtr kid((*slist)[i]);
        invalidateChainRoots(kid);
        if (!disable && kid->hasReachableLabel()) {
          disable = true;
        }
        if (disable) disableCSE(kid);
      }
    }
    break;
  case Statement::KindOfDoStatement:
    {
      // need to disable CSE in the top level body
      // otherwise we'd have to declare all CSE temps like:
      //   declare_temps;
      //   do { ... } while (cond);
      DoStatementPtr ds(static_pointer_cast<DoStatement>(s));
      disableCSE(ds->getBody());
    }
    // fall through
  default:
    for (int i = s->getKidCount(); i--; ) {
      invalidateChainRoots(s->getNthStmt(i));
    }
    break;
  }
}

void AliasManager::nullSafeDisableCSE(StatementPtr parent, ExpressionPtr kid) {
  assert(parent);
  if (!kid) return;
  kid->disableCSE();
}

void AliasManager::disableCSE(StatementPtr s) {
  if (!s) return;
  switch (s->getKindOf()) {
  case Statement::KindOfIfBranchStatement: {
    IfBranchStatementPtr is(static_pointer_cast<IfBranchStatement>(s));
    nullSafeDisableCSE(s, is->getCondition());
    break;
  }
  case Statement::KindOfSwitchStatement: {
    SwitchStatementPtr ss(static_pointer_cast<SwitchStatement>(s));
    nullSafeDisableCSE(s, ss->getExp());
    break;
  }
  case Statement::KindOfForStatement: {
    ForStatementPtr fs(static_pointer_cast<ForStatement>(s));
    nullSafeDisableCSE(s, fs->getInitExp());
    nullSafeDisableCSE(s, fs->getCondExp());
    nullSafeDisableCSE(s, fs->getIncExp());
    break;
  }
  case Statement::KindOfForEachStatement: {
    ForEachStatementPtr fs(static_pointer_cast<ForEachStatement>(s));
    nullSafeDisableCSE(s, fs->getArrayExp());
    nullSafeDisableCSE(s, fs->getNameExp());
    nullSafeDisableCSE(s, fs->getValueExp());
    break;
  }
  case Statement::KindOfWhileStatement: {
    WhileStatementPtr ws(static_pointer_cast<WhileStatement>(s));
    nullSafeDisableCSE(s, ws->getCondExp());
    break;
  }
  case Statement::KindOfDoStatement: {
    DoStatementPtr ds(static_pointer_cast<DoStatement>(s));
    nullSafeDisableCSE(s, ds->getCondExp());
    break;
  }
  default:
    for (int i = s->getKidCount(); i--; ) {
      ConstructPtr c(s->getNthKid(i));
      if (StatementPtr skid = dpc(Statement, c)) {
        disableCSE(skid);
      } else {
        nullSafeDisableCSE(s, dpc(Expression, c));
      }
    }
    break;
  }
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
  always_assert(sz);
  LoopInfo &li1 = m_loopInfo.back();
  always_assert(li1.m_stmt == s);
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
          if (!s->couldBeAliased()) {
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

  // Dont handle nested functions
  // they will be dealt with by another
  // top level call
  if (FunctionWalker::SkipRecurse(s)) return;

  bool pop = false;
  Statement::KindOf stype = s->getKindOf();
  switch (stype) {
  case Statement::KindOfSwitchStatement:
    if (m_loopInfo.size()) {
      pushStringScope(s);
      pop = true;
    }
    break;

  case Statement::KindOfForStatement: {
    ForStatementPtr fs = spc(ForStatement, s);
    stringOptsRecur(fs->getInitExp(), true);
    pushStringScope(s);
    stringOptsRecur(fs->getCondExp(), false);
    stringOptsRecur(fs->getBody());
    stringOptsRecur(fs->getIncExp(), true);
    popStringScope(s);
    return;
  }
  case Statement::KindOfWhileStatement: {
    WhileStatementPtr ws = spc(WhileStatement, s);
    pushStringScope(s);
    stringOptsRecur(ws->getCondExp(), false);
    stringOptsRecur(ws->getBody());
    popStringScope(s);
    return;
  }
  case Statement::KindOfDoStatement: {
    DoStatementPtr ds = spc(DoStatement, s);
    pushStringScope(s);
    stringOptsRecur(ds->getBody());
    stringOptsRecur(ds->getCondExp(), false);
    popStringScope(s);
    return;
  }
  case Statement::KindOfForEachStatement: {
    ForEachStatementPtr fs = spc(ForEachStatement, s);
    stringOptsRecur(fs->getArrayExp(), false);
    stringOptsRecur(fs->getNameExp(), false);
    stringOptsRecur(fs->getValueExp(), false);
    pushStringScope(s);
    stringOptsRecur(fs->getBody());
    popStringScope(s);
    return;
  }
  case Statement::KindOfExpStatement:
    stringOptsRecur(spc(ExpStatement,s)->getExpression(), true);
    return;

  case Statement::KindOfBreakStatement:
    {
      BreakStatementPtr b = spc(BreakStatement, s);
      int64_t depth = b->getDepth();
      if (depth != 1) {
        int64_t s = m_loopInfo.size() - 1;
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

void AliasManager::markAllLocalExprAltered(ExpressionPtr e) {
  if (!m_postOpt) return;
  assert(isInExpression());
  assert(m_exprIdx <= (int)m_accessList.size());
  e->setLocalExprAltered();
  ExpressionPtrList::reverse_iterator it(m_accessList.rbegin());
  int curIdx = m_accessList.size() - 1;
  bool found = m_exprBeginStack.empty();
  for (; curIdx >= m_exprIdx; --curIdx, ++it) {
    ExpressionPtr p(*it);
    if (!found && p == m_exprBeginStack.back()) {
      found = true;
    }
    if (found) {
      bool isLoad; int depth, effects;
      int interf = checkAnyInterf(e, p, isLoad, depth, effects);
      if (interf == InterfAccess ||
          interf == SameAccess) {
        p->setLocalExprAltered();
      }
    }
  }
}
