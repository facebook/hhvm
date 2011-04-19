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

#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/expression/expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/list_assignment.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/expression.h>
#include <compiler/expression/include_expression.h>
#include <compiler/expression/closure_expression.h>
#include <compiler/statement/statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/catch_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/break_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/loop_statement.h>
#include <compiler/statement/foreach_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/echo_statement.h>
#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/control_flow.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/data_flow.h>
#include <compiler/analysis/dictionary.h>
#include <compiler/analysis/expr_dict.h>
#include <compiler/analysis/live_dict.h>

#include <util/parser/hphp.tab.hpp>
#include <util/util.h>

#define spc(T,p) boost::static_pointer_cast<T>(p)
#define dpc(T,p) boost::dynamic_pointer_cast<T>(p)

using namespace HPHP;
using std::string;

///////////////////////////////////////////////////////////////////////////////

AliasManager::AliasManager(int opt) :
    m_bucketList(0), m_nextID(1), m_changes(0), m_replaced(0),
    m_wildRefs(false), m_nrvoFix(0), m_inlineAsExpr(true),
    m_noAdd(false), m_preOpt(opt<0), m_postOpt(opt>0),
    m_cleared(false), m_inPseudoMain(false), m_genAttrs(false),
    m_hasDeadStore(false), m_graph(0) {
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
      Option::AutoInline = val;
    } else if (opt == "cflow") {
      Option::ControlFlow = val;
    } else if (opt == "coalesce") {
      Option::VariableCoalescing = val;
    } else if (val && (opt == "all" || opt == "none")) {
      val = opt == "all";
      Option::EliminateDeadCode = val;
      Option::LocalCopyProp = val;
      Option::AutoInline = val;
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
    assert(m_exprs.size() >= m_num);
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
                                       Expression::KindOfScalarExpression,
                                       T_STRING, string("begin")));
  m_accessList.add(e);
  m_stack.push_back(m_accessList.size());
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
                             Expression::KindOfScalarExpression,
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

int AliasManager::testAccesses(ExpressionPtr e1, ExpressionPtr e2) {
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

      return canonCompare(e1, e2) ?
        SameAccess : InterfAccess;

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
          IncludeExpressionPtr inc(spc(IncludeExpression, e2));
          if (!inc->getPrivateScope()) {
            return InterfAccess;
          }
          goto def;
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
        ASSERT(false);
      }
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfIncludeExpression:
      if (k1 == Expression::KindOfSimpleVariable) {
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
    assert(false);
  }
}

void AliasManager::cleanRefs(ExpressionPtr e,
                             ExpressionPtrList::reverse_iterator it,
                             ExpressionPtrList::reverse_iterator &end,
                             int depth) {
  if (e->is(Expression::KindOfAssignmentExpression) ||
      e->is(Expression::KindOfBinaryOpExpression) ||
      e->is(Expression::KindOfUnaryOpExpression)) {
    ExpressionPtr var = e->getNthExpr(0);
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
               (p->getContext() & Expression::Declaration) ==
               Expression::Declaration)) {
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
      } else if (!eIsLoad) {
        cleanRefs(e, it, end, depth);
        m_accessList.erase(it, end);
        continue;
      }
    }
    ++it;
  }
}

bool AliasManager::okToKill(ExpressionPtr ep, bool killRef) {
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
        if (!(effects & emask) &&
            getOpForAssignmentOp(spc(BinaryOpExpression, e)->getOp())) {
          if (okToKill(spc(BinaryOpExpression, e)->getExp1(), false)) {
            e->setContext(Expression::DeadStore);
            m_replaced++;
            ++it;
            continue;
          }
        }
        cleanInterf(spc(BinaryOpExpression, e)->getExp1(), ++it, end, depth);
        continue;

      case Expression::KindOfUnaryOpExpression:
        cleanInterf(spc(UnaryOpExpression, e)->getExpression(),
                    ++it, end, depth);
        continue;

      case Expression::KindOfSimpleVariable:
      case Expression::KindOfObjectPropertyExpression:
      case Expression::KindOfDynamicVariable:
      case Expression::KindOfArrayElementExpression:
      case Expression::KindOfStaticMemberExpression:
        if ((e->getContext() &
             (Expression::LValue | Expression::UnsetContext)) ==
            (Expression::LValue | Expression::UnsetContext) ||
            (e->getContext() & Expression::Declaration) ==
            Expression::Declaration) {
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

int AliasManager::checkInterf(ExpressionPtr rv, ExpressionPtr e, bool &isLoad,
                              int &depth, int &effects) {
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
      return testAccesses(rv, e);

    case Expression::KindOfListAssignment: {
      isLoad = false;
      ListAssignmentPtr la = spc(ListAssignment, e);
      ExpressionList &lhs = *la->getVariables().get();
      for (int i = lhs.getCount(); i--; ) {
        ExpressionPtr ep = lhs[i];
        if (ep) {
          if (ep->is(Expression::KindOfListAssignment)) {
            if (checkInterf(rv, ep, isLoad, depth, effects) !=
                DisjointAccess) {
              return InterfAccess;
            }
          } else if (testAccesses(ep, rv) != DisjointAccess) {
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
      return testAccesses(e, rv);
    }

    case Expression::KindOfUnaryOpExpression:
      isLoad = false;
      return testAccesses(spc(UnaryOpExpression,e)->getExpression(), rv);
    case Expression::KindOfBinaryOpExpression:
      isLoad = false;
      return testAccesses(spc(BinaryOpExpression,e)->getExp1(), rv);
    case Expression::KindOfAssignmentExpression:
      isLoad = false;
      return testAccesses(spc(AssignmentExpression,e)->getVariable(), rv);

    default:
      assert(false);
  }

  return DisjointAccess;
}

int AliasManager::checkAnyInterf(ExpressionPtr e1, ExpressionPtr e2,
                                 bool &isLoad, int &depth, int &effects) {
  switch (e1->getKindOf()) {
    case Expression::KindOfListAssignment: {
      ListAssignmentPtr la = spc(ListAssignment, e1);
      ExpressionList &lhs = *la->getVariables().get();
      for (int i = lhs.getCount(); i--; ) {
        ExpressionPtr ep = lhs[i];
        if (ep && checkAnyInterf(ep, e2, isLoad, depth, effects) !=
            DisjointAccess) {
          isLoad = false;
          return InterfAccess;
        }
      }
      return DisjointAccess;
    }
    case Expression::KindOfAssignmentExpression:
      e1 = spc(AssignmentExpression, e1)->getVariable();
      break;
    case Expression::KindOfUnaryOpExpression:
    case Expression::KindOfBinaryOpExpression:
      e1 = e1->getNthExpr(0);
      if (!e1 || !e1->hasContext(Expression::OprLValue)) return DisjointAccess;
      break;
    default:
      break;
  }

  return checkInterf(e1, e2, isLoad, depth, effects);
}

int AliasManager::findInterf(ExpressionPtr rv, bool isLoad,
                             ExpressionPtr &rep, int *flags /* = 0 */) {
  BucketMapEntry &lvs = m_accessList;

  rep.reset();
  ExpressionPtrList::reverse_iterator it = lvs.rbegin(), end = lvs.rend();

  bool unset_simple = !isLoad && !m_inPseudoMain &&
    rv->hasContext(Expression::UnsetContext) &&
    rv->hasContext(Expression::LValue) &&
    rv->is(Expression::KindOfSimpleVariable);

  int depth = 0, min_depth = 0, max_depth = 0;
  for (; it != end; ++it) {
    ExpressionPtr e = *it;
    if (e->getContext() & Expression::DeadStore) continue;
    bool eIsLoad = false;
    int effects = 0;
    int a = checkInterf(rv, e, eIsLoad, depth, effects);
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
        if (unset_simple) {
          if (a == InterfAccess) {
            if (!rep) {
              rep = e;
            }
            continue;
          } else if (a == SameAccess && rep) {
            if (!e->is(Expression::KindOfSimpleVariable) ||
                !e->hasContext(Expression::UnsetContext) ||
                !e->hasContext(Expression::LValue)) {
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
          continue;
        }
        rep = e;
        return a;
      }
    }
  }

  return DisjointAccess;
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
    e->setCanonPtr(ExpressionPtr());
    e->setCanonID(0);
    return ExpressionPtr();
  }

  switch (e->getKindOf()) {
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfBinaryOpExpression:
    case Expression::KindOfUnaryOpExpression: {
      ExpressionPtr var = e->getNthExpr(0);
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
  if (!m_noAdd) {
    if (m_preOpt) ret = e->preOptimize(m_arp);
    if (m_postOpt) ret = e->postOptimize(m_arp);
    if (ret) {
      return canonicalizeRecurNonNull(ret);
    }
  }

  e->setVisited();
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
                  m_accessList.isSubLast(a)) {
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
                                  Expression::KindOfBinaryOpExpression,
                                  lhs, rhs, getOpForAssignmentOp(b->getOp()))));
                m_replaced++;
              }
            }
            rep = b->getExp1();
            break;
          }
          case Expression::KindOfUnaryOpExpression: {
            UnaryOpExpressionPtr u = spc(UnaryOpExpression, rep);
            if (Option::EliminateDeadCode) {
              if (u->getActualType() && u->getActualType()->isInteger()) {
                ExpressionPtr val = u->getExpression()->clone();
                val->clearContext();
                if (u->getFront()) {
                  ExpressionPtr inc
                    (new ScalarExpression(u->getScope(), u->getLocation(),
                                          Expression::KindOfScalarExpression,
                                          T_LNUMBER, string("1")));

                  val = ExpressionPtr(
                    new BinaryOpExpression(u->getScope(), u->getLocation(),
                                           Expression::KindOfBinaryOpExpression,
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
        assert(rep->getKindOf() == ae->getVariable()->getKindOf());
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
                  if (!e->is(Expression::KindOfSimpleVariable) ||
                      spc(SimpleVariable, e)->couldBeAliased()) {
                    break;
                  }

                  AssignmentExpressionPtr a = spc(AssignmentExpression, rep);
                  ExpressionPtr value = a->getValue();
                  if (value->getContext() & Expression::RefValue) {
                    break;
                  }
                  if (!Expression::CheckNeeded(a->getVariable(), value) ||
                      m_accessList.isSubLast(a)) {
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
                              a->setNthKid(1, value);
                              a->recomputeEffects();
                              setChanged();
                            } else {
                              ExpressionListPtr el(
                                new ExpressionList(
                                  a->getScope(), a->getLocation(),
                                  Expression::KindOfExpressionList,
                                  ExpressionList::ListKindWrapped));
                              a = spc(AssignmentExpression, a->clone());
                              el->addElement(a);
                              el->addElement(a->getValue());
                              a->setNthKid(1, value->makeConstant(m_arp, "null"));
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
        assert(doAccessChains);
        if (e->getContext() & (Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter|
                               Expression::DeepReference|
                               Expression::UnsetContext)) {
          ExpressionPtr rep;
          int interf = findInterf(e, true, rep);
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
        int interf = findInterf(e, true, rep);
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
                                      Expression::KindOfUnaryOpExpression,
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
                    (sameExpr(cur, orig) || next && sameExpr(next, orig))) {
                  e->recomputeEffects();
                  return e->replaceValue(canonicalizeRecurNonNull(rhs));
                }
              }
              cur = next;
            }
            if (ae->isUnused() && m_accessList.isLast(ae)) {
              rep = ae->clone();
              ae->setContext(Expression::DeadStore);
              ae->setNthKid(1, ae->makeConstant(m_arp, "null"));
              ae->setNthKid(0, ae->makeConstant(m_arp, "null"));
              e->recomputeEffects();
              m_replaced++;
              return e->replaceValue(canonicalizeRecurNonNull(rep));
            }
            e->setCanonPtr(last);
          }
        }
      }
      add(m_accessList, e);
      break;
    }

    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr bop = spc(BinaryOpExpression, e);
      int rop = getOpForAssignmentOp(bop->getOp());
      if (bop->hasContext(Expression::DeadStore)) {
        assert(rop);
        ExpressionPtr rhs = bop->getExp2();
        ExpressionPtr lhs = bop->getExp1();
        lhs->clearContext();
        e->recomputeEffects();
        return bop->replaceValue(
          canonicalizeNonNull(ExpressionPtr(
                                new BinaryOpExpression(
                                  bop->getScope(), bop->getLocation(),
                                  Expression::KindOfBinaryOpExpression,
                                  lhs, rhs, rop))));
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
                ExpressionPtr rhs(
                  (new BinaryOpExpression(e->getScope(), e->getLocation(),
                                          Expression::KindOfBinaryOpExpression,
                                          op0->clone(), op1->clone(), rop)));

                lhs = lhs->clone();
                lhs->clearContext(Expression::OprLValue);
                return e->replaceValue(
                  canonicalizeRecurNonNull(
                    ExpressionPtr(new AssignmentExpression(
                                    e->getScope(), e->getLocation(),
                                    Expression::KindOfAssignmentExpression,
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
                    ok = v->getCanonPtr();
                  }
                  if (ok) {
                    b2->setContext(Expression::DeadStore);
                    ExpressionPtr r(new BinaryOpExpression(
                                      bop->getScope(), bop->getLocation(),
                                      Expression::KindOfBinaryOpExpression,
                                      op0->clone(), bop->getExp2(),
                                      rop));
                    ExpressionPtr b(new BinaryOpExpression(
                                      bop->getScope(), bop->getLocation(),
                                      Expression::KindOfBinaryOpExpression,
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
          assert(alt->getKindOf() == lhs->getKindOf());
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
        case T_DEC: {
          ExpressionPtr alt;
          int interf = findInterf(uop->getExpression(), true, alt);
          if (interf == SameAccess) {
            switch (alt->getKindOf()) {
              case Expression::KindOfAssignmentExpression:
                alt = spc(AssignmentExpression,alt)->getVariable();
                break;
              case Expression::KindOfBinaryOpExpression:
                alt = spc(BinaryOpExpression,alt)->getExp1();
                break;
              case Expression::KindOfUnaryOpExpression:
                alt = spc(UnaryOpExpression,alt)->getExpression();
                break;
              default:
                break;
            }
            assert(alt->getKindOf() == uop->getExpression()->getKindOf());
            uop->getExpression()->setCanonID(alt->getCanonID());
          } else {
            uop->getExpression()->setCanonID(m_nextID++);
          }
        }
          add(m_accessList, e);
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

void AliasManager::canonicalizeKid(ConstructPtr c, ExpressionPtr kid, int i) {
  if (kid) {
    kid = canonicalizeRecur(kid);
    if (kid) {
      c->setNthKid(i, kid);
      c->recomputeEffects();
      setChanged();
    }
  }
}

int AliasManager::canonicalizeKid(ConstructPtr c, ConstructPtr kid, int i) {
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

  switch (e->getKindOf()) {
    case Expression::KindOfQOpExpression:
      canonicalizeKid(e, e->getNthExpr(0), 0);
      beginScope();
      if (ExpressionPtr e1 = e->getNthExpr(1)) {
        canonicalizeKid(e, e1, 1);
        resetScope();
      }
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

    case Expression::KindOfExpressionList:
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
      delayVars = false;
      break;

    default:
      break;
  }

  int n = e->getKidCount();
  if (n < 2) delayVars = false;

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

  return canonicalizeNode(e);
}

StatementPtr AliasManager::canonicalizeRecur(StatementPtr s, int &ret) {
  ret = FallThrough;
  if (!s || s->isVisited()) return StatementPtr();

  Statement::KindOf stype = s->getKindOf();
  int start = 0;
  int nkid = s->getKidCount();

  switch (stype) {
  case Statement::KindOfFunctionStatement:
  case Statement::KindOfMethodStatement:
  case Statement::KindOfClassStatement:
  case Statement::KindOfInterfaceStatement:
    // Dont handle nested functions
    // they will be dealt with by another
    // top level call to optimize
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
          canonicalizeKid(ifstmt, ifstmt->getNthKid(1), 1);
          endScope();
          if (i+1 < n) resetScope();
        }
        endScope();
      }
      ret = FallThrough;
      start = nkid;
    }
    break;

  case Statement::KindOfIfBranchStatement:
    assert(0);
    break;

  case Statement::KindOfForStatement:
    canonicalizeKid(s, spc(Expression,s->getNthKid(0)), 0);
    clear();
    canonicalizeKid(s, spc(Expression,s->getNthKid(1)), 1);
    canonicalizeKid(s, s->getNthKid(2), 2);
    clear();
    canonicalizeKid(s, spc(Expression,s->getNthKid(3)), 3);
    ret = Converge;
    start = nkid;
    break;

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
  case Statement::KindOfLabelStatement:
    clear();
    break;

  case Statement::KindOfReturnStatement:
  {
    canonicalizeKid(s, spc(Expression,s->getNthKid(0)), 0);
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
        ExpressionPtr e(new ScalarExpression(BlockScopePtr(), LocationPtr(),
                                             Expression::KindOfScalarExpression,
                                             T_STRING, string("io")));
        add(m_accessList, e);
      }
      ret = FallThrough;
      start = nkid;
      break;
    }
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
        ExpressionListPtr vars = dpc(ExpressionList, s->getNthKid(0));
        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr e = (*vars)[i];
          if (AssignmentExpressionPtr ae = dpc(AssignmentExpression, e)) {
            e = ae->getVariable();
          }
          if (SimpleVariablePtr sv = dpc(SimpleVariable, e)) {
            if (Symbol *sym = sv->getSymbol()) {
              sym->setReferenced();
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
        SimpleVariablePtr name =
          dpc(SimpleVariable, s->getNthKid(ForEachStatement::NameExpr));
        if (name) {
          Symbol *sym = name->getSymbol();
          sym->setNeeded();
        }
        SimpleVariablePtr value =
          dpc(SimpleVariable, s->getNthKid(ForEachStatement::ValueExpr));
        if (value) {
          Symbol *sym = value->getSymbol();
          sym->setNeeded();
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
    switch (e->getKindOf()) {
    case Expression::KindOfAssignmentExpression:
      {
        AssignmentExpressionPtr ae = spc(AssignmentExpression, e);
        ExpressionPtr var = ae->getVariable();
        ExpressionPtr val = ae->getValue();
        if (var->is(Expression::KindOfSimpleVariable)) {
          if (val->getContext() & Expression::RefValue) {
            if (Symbol *sym = spc(SimpleVariable, var)->getSymbol()) {
              sym->setReferenced();
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
          if (sv->isThis()) {
            sv->getFunctionScope()->setContainsThis();
            if (!e->hasContext(Expression::ObjectContext)) {
              sv->getFunctionScope()->setContainsBareThis();
            } else if (m_graph) {
              int &id = m_gidMap["v:this"];
              if (!id) id = m_gidMap.size();
              e->setCanonID(id);
            }
          }
          if ((context & (Expression::RefValue|Expression::RefAssignmentLHS)) ||
              sym->isRefClosureVar()) {
            sym->setReferenced();
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
        if (!inc->getPrivateScope()) {
          m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
        }
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
        e = spc(ObjectPropertyExpression, e)->getObject();
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
    case Expression::KindOfSimpleFunctionCall:
      spc(SimpleFunctionCall, e)->updateVtFlags();
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfClassConstantExpression:
    case Expression::KindOfStaticMemberExpression:
    case Expression::KindOfNewObjectExpression:
      if (m_graph) {
        StaticClassName *p = dynamic_cast<StaticClassName*>(e.get());
        assert(p);
        const std::string &name = p->getClassName();
        if (!name.empty()) {
          int &id = m_gidMap[name];
          if (!id) id = m_gidMap.size();
          e->setCanonID(id);
        }
      }
      break;

    case Expression::KindOfUnaryOpExpression:
      if (Option::EnableEval > Option::NoEval && spc(UnaryOpExpression, e)->
          getOp() == T_EVAL) {
        m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
      }
      break;
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

  if (!m_inPseudoMain) {
    m_variables->clearAttribute(VariableTable::ContainsLDynamicVariable);
    m_variables->clearAttribute(VariableTable::ContainsDynamicVariable);
    m_variables->clearAttribute(VariableTable::ContainsCompact);
    m_variables->clearAttribute(VariableTable::ContainsExtract);
  }

  func->setContainsThis(false);
  func->setContainsBareThis(false);
  func->setInlineSameContext(false);

  int i, nkid = m->getKidCount(), cost = 0;
  for (i = 0; i < nkid; i++) {
    ConstructPtr cp(m->getNthKid(i));
    int c = collectAliasInfoRecur(cp, false);
    if (cp == m->getStmts()) cost = c;
  }

  if (func->containsThis() && !m->getClassScope()) {
    func->setContainsThis(false);
    func->setContainsBareThis(false);
  }

  if (m_inlineAsExpr) {
    if (!Option::AutoInline ||
        cost > 1 ||
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

static void markAvailable(ExpressionRawPtr e) {
  if (e->isThis()) {
    SimpleVariableRawPtr sv(dpc(SimpleVariable,e));
    assert(sv);
    sv->setGuardedThis();
  } else {
    StaticClassName *scn = dynamic_cast<StaticClassName*>(e.get());
    assert(scn);
    scn->setPresent();
  }
}

class ConstructTagger : public ControlFlowGraphWalker {
public:
  ConstructTagger(ControlFlowGraph *g) : ControlFlowGraphWalker(g) {}

  void walk() { ControlFlowGraphWalker::walk(*this); }
  int after(ConstructRawPtr cp) {
    if (ExpressionRawPtr e = boost::dynamic_pointer_cast<Expression>(cp)) {
      if (int id = e->getCanonID()) {
        if (m_block->getBit(DataFlow::Available, id)) {
          markAvailable(e);
        } else {
          m_block->setBit(DataFlow::Available, id);
        }
      }
    }
    return WalkContinue;
  }
};

class ConstructMarker : public ControlFlowGraphWalker {
public:
  ConstructMarker(ControlFlowGraph *g) : ControlFlowGraphWalker(g) {}

  void walk() { ControlFlowGraphWalker::walk(*this); }
  int after(ConstructRawPtr cp) {
    if (ExpressionRawPtr e = boost::dynamic_pointer_cast<Expression>(cp)) {
      if (int id = e->getCanonID()) {
        if (m_block->getBit(DataFlow::AvailIn, id)) {
          markAvailable(e);
        }
      }
    }
    return WalkContinue;
  }
};

class Propagater : public ControlFlowGraphWalker {
public:
  Propagater(ControlFlowGraph *g, ExprDict &d) :
      ControlFlowGraphWalker(g), m_dict(d), m_changed(false) {}

  bool walk() { ControlFlowGraphWalker::walk(*this); return m_changed; }
  int afterEach(ConstructRawPtr p, int i, ConstructPtr kid) {
    if (ExpressionRawPtr e = boost::dynamic_pointer_cast<Expression>(kid)) {
      if (e->isAnticipated() &&
          !(e->getContext() & (Expression::LValue|
                               Expression::OprLValue|
                               Expression::AssignmentLHS|
                               Expression::RefValue|
                               Expression::UnsetContext|
                               Expression::DeepReference))) {
        if (ExpressionPtr rep = m_dict.propagate(e)) {
          m_changed = true;
          rep = e->replaceValue(rep->clone());
          p->setNthKid(i, rep);
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

int AliasManager::copyProp(MethodStatementPtr m) {
  m_graph = ControlFlowGraph::buildControlFlow(m);
  ExprDict ed(*this);
  m_genAttrs = true;
  ed.build(m);
  AttributeTagger<ExprDict> at(m_graph, ed);
  at.walk();
  m_genAttrs = false;

  DataFlow::ComputeAvailable(*m_graph);
  DataFlow::ComputeAnticipated(*m_graph);

  if (Option::DumpAst) m_graph->dump(m_arp);

  Propagater prop(m_graph, ed);
  bool ret = prop.walk();

  delete m_graph;
  m_graph = 0;

  return ret;
}

int AliasManager::optimize(AnalysisResultConstPtr ar, MethodStatementPtr m) {
  gatherInfo(ar, m);

  if (!m_hasDeadStore && Option::CopyProp) {
    if (copyProp(m)) return 1;
  }

  if (Option::LocalCopyProp || Option::EliminateDeadCode) {
    for (int i = 0, nkid = m->getKidCount(); i < nkid; i++) {
      if (i) {
        clear();
        m_cleared = false;
      }
      canonicalizeKid(m, m->getNthKid(i), i);
    }

    killLocals();
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
    {
      static int rows[] =
        { DataFlow::Available, DataFlow::AvailIn, DataFlow::AvailOut };
      m_graph->allocateDataFlow(m_gidMap.size()+1,
                                sizeof(rows)/sizeof(rows[0]), rows);
    }

    ConstructTagger ct(m_graph);
    ct.walk();

    DataFlow::ComputeAvailable(*m_graph);

    ConstructMarker cm(m_graph);
    cm.walk();

    if (Option::VariableCoalescing &&
        !m_inPseudoMain &&
        !m_variables->getAttribute(VariableTable::ContainsDynamicVariable)) {
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
