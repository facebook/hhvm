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

#include "hphp/compiler/analysis/ref_dict.h"
#include "hphp/compiler/analysis/alias_manager.h"
#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/simple_variable.h"

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"

#include "hphp/parser/hphp.tab.hpp"

using namespace HPHP;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

void RefDict::build(MethodStatementPtr m) {
  m_am.clear();
  Dictionary::build(m);
  static int rows[] =
    {
      // BitVecs for pass 1
      DataFlow::Referenced, DataFlow::Killed,
      DataFlow::PRefIn,     DataFlow::PRefOut,

      // BitVecs for pass 2
      DataFlow::Object,     DataFlow::NotObject,
      DataFlow::PObjIn,     DataFlow::PObjOut,
    };

  m_am.graph()->allocateDataFlow(size() + 1,
                                 sizeof(rows)/sizeof(rows[0]), rows);
  m_method_stmt = m;
}

void RefDict::visit(ExpressionPtr e) {
  if (!first_pass) {
    // only need to record expressions once
    return;
  }

  if (e->getScope()->inPseudoMain()) {
    // bail out for psuedomain
    return;
  }

  if (!e->is(Expression::KindOfSimpleVariable)) {
    // only need to record simple variables
    return;
  }

  SimpleVariablePtr ptr(static_pointer_cast<SimpleVariable>(e));

  if (ptr->isSuperGlobal() || ptr->isThis()) {
    // don't both recording for super globals or this
    return;
  }

  // Good to go
  if (m_am.insertForDict(e)) {
    record(e);
  }
}

void RefDict::beginBlock(ControlBlock *b) {
  // pass 1
  m_referenced = b->getRow(DataFlow::Referenced);
  m_killed     = b->getRow(DataFlow::Killed);

  // pass 2
  m_obj        = b->getRow(DataFlow::Object);
  m_noobj      = b->getRow(DataFlow::NotObject);

  always_assert(
    m_am.graph()->rowExists(DataFlow::Referenced) &&
    m_am.graph()->rowExists(DataFlow::Killed) &&
    m_am.graph()->rowExists(DataFlow::PRefIn) &&
    m_am.graph()->rowExists(DataFlow::PRefOut) &&
    m_am.graph()->rowExists(DataFlow::Object) &&
    m_am.graph()->rowExists(DataFlow::NotObject) &&
    m_am.graph()->rowExists(DataFlow::PObjIn) &&
    m_am.graph()->rowExists(DataFlow::PObjOut));

  if (!first_pass) {
    // set m_referenced to PRefIn
    BitOps::Bits *prefin = b->getRow(DataFlow::PRefIn);
    BitOps::bit_copy(size(), m_referenced, prefin);
  }
}

void RefDict::endBlock(ControlBlock *b) {}

void RefDict::updateParams() {
  ControlFlowGraph *g = m_am.graph();
  ControlBlock *b     = g->getDfBlock(1);

  BitOps::Bits *refbv = b->getRow(DataFlow::PRefIn);
  BitOps::Bits *objbv = b->getRow(DataFlow::PObjIn);

  for (int i = size(); i--; ) {
    if (ExpressionPtr e = get(i)) {
      always_assert(e->is(Expression::KindOfSimpleVariable));
      always_assert(((unsigned int)i) == e->getCanonID());
      Symbol *sym = static_pointer_cast<SimpleVariable>(e)->getSymbol();
      if (sym && (sym->isParameter() || sym->isClosureVar())) {
        TypePtr paramType;
        bool isRef;
        if (sym->isParameter()) {
          ExpressionListPtr methodParams = m_method_stmt->getParams();
          ExpressionPtr paramExprPtr =
              (*methodParams)[sym->getParameterIndex()];
          paramType = paramExprPtr->getType();
          isRef = m_method_stmt->isRef(sym->getParameterIndex());
        } else {
          assert(sym->isClosureVar());
          // can only assume it is a Variant for now
          paramType = Type::Variant;
          isRef = sym->isRefClosureVar();
        }
        if (first_pass) {
          if (isRef || sym->isCallTimeRef()) {
            BitOps::set_bit(i, refbv, true);
          }
        } else {
          if (paramType) {
            if (!paramType->isNoObjectInvolved()) {
              BitOps::set_bit(i, objbv, true);
            }
          } else {
            // no type information, so we must assume it holds an object
            BitOps::set_bit(i, objbv, true);
          }
        }
      }
    }
  }
}

void RefDict::updateAccess(ExpressionPtr e) {
  always_assert(!e->getScope()->inPseudoMain());

  int eid     = e->getCanonID();
  int context = e->getContext();

  if (first_pass) {
    if (!e->is(Expression::KindOfSimpleVariable) &&
        !e->is(Expression::KindOfDynamicVariable)) return;

    e->clearAvailable();
    e->clearReferencedValid();
    e->clearReferenced();

    SimpleVariablePtr ptr(dynamic_pointer_cast<SimpleVariable>(e));
    if (ptr && (ptr->isSuperGlobal() || ptr->isThis())) return;

    if (e->is(Expression::KindOfSimpleVariable)) {
      if (BitOps::get_bit(eid, m_referenced)) {
        e->setReferenced();
      } else if (!BitOps::get_bit(eid, m_killed)) {
        // use as a temp place holder
        e->setAvailable();
      }
    }
  }

  // let the first pass information propagate for both passes, since
  // we need it in both contexts
  if (context & Expression::RefAssignmentLHS ||
      context & Expression::RefValue ||
      context & Expression::RefParameter ||
      ((context & Expression::Declaration) == Expression::Declaration)) {
    if (e->is(Expression::KindOfSimpleVariable)) {
      BitOps::set_bit(eid, m_referenced, true);
      BitOps::set_bit(eid, m_killed, false);
    } else {
      // for dynamic variables, we must assume the worst
      BitOps::set(size(), m_referenced, -1);
      BitOps::set(size(), m_killed, 0);
    }
  } else if (e->is(Expression::KindOfSimpleVariable) &&
             context & Expression::LValue &&
             context & Expression::UnsetContext) {
    BitOps::set_bit(eid, m_referenced, false);
    BitOps::set_bit(eid, m_killed, true);
  }

  if (first_pass) return;

  // now we're on the second pass

  if (context & Expression::AssignmentLHS ||
      context & Expression::OprLValue) {
    // we dealt with this node as a store expression
    return;
  }

  int cls = e->getExprClass();

  bool isRhsNeeded = false;
  bool canKill     = false;

  ExpressionPtr lhs;
  ExpressionPtr rhs;

  if (cls & Expression::Store) {
    // we care about two cases here
    switch (e->getKindOf()) {
      case Expression::KindOfAssignmentExpression:
        // $x = ...
        {
          AssignmentExpressionPtr assign(
              static_pointer_cast<AssignmentExpression>(e));
          lhs = assign->getVariable();
          rhs = assign->getValue();
          isRhsNeeded = Expression::CheckNeededRHS(rhs);
          canKill = true;
        }
        break;
      case Expression::KindOfBinaryOpExpression:
        // $x += ...
        {
          BinaryOpExpressionPtr binop(
              static_pointer_cast<BinaryOpExpression>(e));
          if (binop->getOp() == T_PLUS_EQUAL) {
            lhs = binop->getExp1();
            rhs = binop->getExp2();
            isRhsNeeded = Expression::CheckNeededRHS(rhs);
          }
        }
        break;
      default:
        break;
    }
  }

  bool isLhsSimpleVar = false;
  bool isLhsDynamic   = false;
  bool isRefd         = false;
  if (lhs) {
    isLhsSimpleVar = lhs->is(Expression::KindOfSimpleVariable);

    // TODO: can a variable only be simple or dynamic?
    // If so, this is un-necessary
    isLhsDynamic   = lhs->is(Expression::KindOfDynamicVariable);

    if (isLhsSimpleVar) {
      // clean up the LHS AST
      lhs->clearAvailable();
      lhs->clearNeeded();
      lhs->clearNeededValid();

      if (BitOps::get_bit(lhs->getCanonID(), m_obj)) {
        lhs->setNeeded();
        lhs->setNeededValid();
      } else if (!BitOps::get_bit(lhs->getCanonID(), m_noobj)) {
        lhs->setAvailable();
      }
    }

    if (lhs->isReferencedValid() && lhs->isReferenced() && isRhsNeeded) {
      // could we possibly have modified another referenced variable?
      isRefd = true;
    }

    if (isLhsSimpleVar && isRhsNeeded) {
      // we see this case:
      // $x = new ...
      // so we mark $x as being needed
      BitOps::set_bit(lhs->getCanonID(), m_obj, true);
      BitOps::set_bit(lhs->getCanonID(), m_noobj, false);
    } else if (isLhsSimpleVar && canKill && !isRhsNeeded) {
      // we saw an assignment that was of the form
      // $x = <primitive>
      // we can now set $x to be not an object
      BitOps::set_bit(lhs->getCanonID(), m_obj, false);
      BitOps::set_bit(lhs->getCanonID(), m_noobj, true);
    }
  }

  if (isLhsDynamic && isRhsNeeded) {
    // in this case, we must set EVERY variable to contain an object
    BitOps::set(size(), m_obj, -1 /* true for each bit */);
    BitOps::set(size(), m_noobj, 0 /* false for each bit */);

    // we're done, since it can be no worse (no more conservative) than this
    return;
  }

  // do we see a load which could cause the value of this expr to be changed?
  // for example:
  // function foo(&$x) { $x = 10; }
  // $x = 30;
  // foo($x); /* <-- this is what we care about */
  if ((cls & (Expression::Load|Expression::Call)) &&
      (context & (Expression::RefValue|Expression::DeepReference))) {
    isRefd = true;
  }

  // we want to propagate this information to other simple vars we see
  if (e->is(Expression::KindOfSimpleVariable)) {
    // clean up the AST
    e->clearAvailable();
    e->clearNeeded();
    e->clearNeededValid();

    SimpleVariablePtr svp(static_pointer_cast<SimpleVariable>(e));
    if (svp->isSuperGlobal() || svp->isThis()) return;

    // update the AST to *before* the modification
    if (BitOps::get_bit(eid, m_obj)) {
      e->setNeeded();
      e->setNeededValid();
    } else if (!BitOps::get_bit(eid, m_noobj)) {
      // use as a temp place holder
      e->setAvailable();
    }

    if (context & Expression::LValue &&
        context & Expression::UnsetContext) {
      always_assert(!isRefd);
      // unset($x);
      BitOps::set_bit(eid, m_obj, false);
      BitOps::set_bit(eid, m_noobj, true);
    } else if (isRefd ||
        ((context & Expression::Declaration) == Expression::Declaration)) {
      // if a simple variable has isRefd, then we need to mark it
      // as potentially containing an object.
      // also, if the simple variable is in global context
      // then we also mark it as potentially containing an object
      BitOps::set_bit(eid, m_obj, true);
      BitOps::set_bit(eid, m_noobj, false);
    }
  }

  if (isRefd) {
    // do a scan for every simple variable referenced value
    // in the dictionary and mark it as potentially
    // containing an object (in the bit vector)
    for (int i = size(); i--; ) {
      if (ExpressionPtr e = get(i)) {
        always_assert(e->is(Expression::KindOfSimpleVariable));
        always_assert(((unsigned int)i) == e->getCanonID());
        if (BitOps::get_bit(i, m_referenced)) {
          BitOps::set_bit(i, m_obj, true);
          BitOps::set_bit(i, m_noobj, false);
        }
      }
    }
  }
}

int RefDictWalker::after(ConstructRawPtr cp) {
  if (SimpleVariableRawPtr s =
      dynamic_pointer_cast<SimpleVariable>(cp)) {
    if (int id = s->getCanonID()) {
      if (first_pass) {
        if (s->isAvailable() && m_block->getBit(DataFlow::PRefIn, id)) {
          s->setReferenced();
        }
        s->setReferencedValid();
        always_assert(s->isReferencedValid());
      } else {
        if (s->isAvailable() && m_block->getBit(DataFlow::PObjIn, id)) {
          s->setNeeded();
        }
        s->setNeededValid();
        always_assert(s->isNeededValid());
      }
    }
  }
  return WalkContinue;
}
