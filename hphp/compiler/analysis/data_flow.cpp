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

#include "hphp/compiler/analysis/data_flow.h"

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/list_assignment.h"

using namespace HPHP;
using std::pair;

///////////////////////////////////////////////////////////////////////////////

const char *DataFlow::GetName(int i) {
#define DECLARE_DF_NAME(x,v) #x
  static const char *names[] = {
    DECLARE_DATA_FLOW(DECLARE_DF_NAME)
  };
  if (i >= 0 && i < NumBVs) return names[i];
  return "Unknown";
}

int DataFlow::GetInit(int i) {
#define DECLARE_DF_INIT(x,v) v
  static int inits[] = {
    DECLARE_DATA_FLOW(DECLARE_DF_INIT)
  };
  if (i >= 0 && i < NumBVs) return inits[i];
  return 0;
}

template <typename T>
void DataFlow::ComputeForwards(T func, const ControlFlowGraph &g,
                               int lAttr, int altAttr,
                               int inAttr, int outAttr) {
  int num = g.getNumBlocks();
  bool changed;
  BitOps::Bits *tmp1 = g.getTempBits(0);
  bool hasAltered = g.rowExists(altAttr);
  size_t width = g.bitWidth();

  do {
    changed = false;

    for (int i = 1; i <= num; i++) {
      ControlBlock *b = g.getDfBlock(i);
      std::pair<in_edge_iterator, in_edge_iterator> vi = in_edges(b, g);
      BitOps::Bits *ain = b->getRow(inAttr);

      if (vi.first != vi.second) {
        ControlBlock *p = source(*vi.first, g);
        if (++vi.first != vi.second) {
          if (!changed) BitOps::bit_copy(width, tmp1, ain);
          func(width, ain,
               p->getRow(outAttr),
               source(*vi.first, g)->getRow(outAttr));
          while (++vi.first != vi.second) {
            p = source(*vi.first, g);
            func(width, ain, ain, p->getRow(outAttr));
          }
          if (!changed) changed = !BitOps::bit_equal(width, tmp1, ain);
        } else {
          if (!changed) {
            changed = !BitOps::bit_equal(width, ain, p->getRow(outAttr));
          }
          BitOps::bit_copy(width, ain, p->getRow(outAttr));
        }
      }

      BitOps::Bits *aout = b->getRow(outAttr);
      if (!changed) BitOps::bit_copy(width, tmp1, aout);
      BitOps::Bits *avl = b->getRow(lAttr);
      if (hasAltered) {
        BitOps::Bits *alt = b->getRow(altAttr);
        BitOps::bit_andc_or(width, aout, ain, alt, avl);
      } else {
        BitOps::bit_or(width, aout, ain, avl);
      }
      if (!changed) changed = !BitOps::bit_equal(width, tmp1, aout);
    }
  } while (changed);
}

template <typename T>
void DataFlow::ComputeBackwards(T func, const ControlFlowGraph &g,
                                int lAttr, int altAttr,
                                int inAttr, int outAttr) {
  int num = g.getNumBlocks();
  bool changed;
  BitOps::Bits *tmp1 = g.getTempBits(0);
  bool hasAltered = g.rowExists(altAttr);
  size_t width = g.bitWidth();

  do {
    changed = false;

    for (int i = num; i ; i--) {
      ControlBlock *b = g.getDfBlock(i);
      std::pair<out_edge_iterator, out_edge_iterator> vi = out_edges(b, g);
      BitOps::Bits *aout = b->getRow(outAttr);

      if (vi.first != vi.second) {
        ControlBlock *s = target(*vi.first, g);
        if (++vi.first != vi.second) {
          if (!changed) BitOps::bit_copy(width, tmp1, aout);
          func(width, aout,
               s->getRow(inAttr),
               target(*vi.first, g)->getRow(inAttr));
          while (++vi.first != vi.second) {
            s = target(*vi.first, g);
            func(width, aout, aout, s->getRow(inAttr));
          }
          if (!changed) changed = !BitOps::bit_equal(width, tmp1, aout);
        } else {
          if (!changed) {
            changed = !BitOps::bit_equal(width, aout, s->getRow(inAttr));
          }
          BitOps::bit_copy(width, aout, s->getRow(inAttr));
        }
      }

      BitOps::Bits *ain = b->getRow(inAttr);
      if (!changed) BitOps::bit_copy(width, tmp1, ain);
      BitOps::Bits *ant = b->getRow(lAttr);
      if (hasAltered) {
        BitOps::Bits *alt = b->getRow(altAttr);
        BitOps::bit_andc_or(width, ain, aout, alt, ant);
      } else {
        BitOps::bit_or(width, ain, aout, ant);
      }
      if (!changed) changed = !BitOps::bit_equal(width, tmp1, ain);
    }
  } while (changed);
}

void DataFlow::ComputeAvailable(const ControlFlowGraph &g) {
  DataFlow::ComputeForwards(
    BitOps::bit_and, g,
    DataFlow::Available, DataFlow::Altered,
    DataFlow::AvailIn, DataFlow::AvailOut);
}

void DataFlow::ComputeAnticipated(const ControlFlowGraph &g) {
  DataFlow::ComputeBackwards(
    BitOps::bit_and, g,
    DataFlow::Anticipated, DataFlow::Altered,
    DataFlow::AntIn, DataFlow::AntOut);
}

void DataFlow::ComputePartialAvailable(const ControlFlowGraph &g) {
  DataFlow::ComputeForwards(
    BitOps::bit_or, g,
    DataFlow::Available, DataFlow::Altered,
    DataFlow::PAvailIn, DataFlow::PAvailOut);
}

void DataFlow::ComputePartialAnticipated(const ControlFlowGraph &g) {
  DataFlow::ComputeBackwards(
    BitOps::bit_or, g,
    DataFlow::Anticipated, DataFlow::Altered,
    DataFlow::PAntIn, DataFlow::PAntOut);
}

void DataFlow::ComputePartialReferenced(const ControlFlowGraph &g) {
  DataFlow::ComputeForwards(
    BitOps::bit_or, g,
    DataFlow::Referenced, DataFlow::Killed,
    DataFlow::PRefIn, DataFlow::PRefOut);
}

void DataFlow::ComputePartialNeeded(const ControlFlowGraph &g) {
  DataFlow::ComputeForwards(
    BitOps::bit_or, g,
    DataFlow::Object, DataFlow::NotObject,
    DataFlow::PObjIn, DataFlow::PObjOut);
}

void DataFlow::ComputePartialInited(const ControlFlowGraph &g) {
  DataFlow::ComputeForwards(
    BitOps::bit_or, g,
    DataFlow::Inited, DataFlow::Killed,
    DataFlow::PInitIn, DataFlow::PInitOut);
}

void DataFlow::ComputeUsed(const ControlFlowGraph &g) {
  int num = g.getNumBlocks();
  size_t width = g.bitWidth();

  for (int i = num; i ; i--) {
    ControlBlock *b = g.getDfBlock(i);
    BitOps::Bits *ant = b->getRow(Anticipated);
    BitOps::Bits *alt = b->getRow(Altered);
    BitOps::Bits *avl = b->getRow(Available);
    BitOps::bit_or_or(width, b->getRow(Used), ant, alt, avl);
  }
}

void DataFlow::ComputePartialDying(const ControlFlowGraph &g) {
  DataFlow::ComputeBackwards(
    BitOps::bit_or, g,
    DataFlow::Dying, DataFlow::Used,
    DataFlow::PDieIn, DataFlow::PDieOut);
}

///////////////////////////////////////////////////////////////////////////////
// DataFlowWalker

int DataFlowWalker::after(ConstructRawPtr cp) {
  if (ExpressionRawPtr e = dynamic_pointer_cast<Expression>(cp)) {
    switch (e->getKindOf()) {
      case Expression::KindOfSimpleVariable:
        if (!static_pointer_cast<SimpleVariable>(
              e)->getAlwaysStash()) {
          return WalkContinue;
        }
        break;
      case Expression::KindOfBinaryOpExpression:
        if (static_pointer_cast<BinaryOpExpression>(
              e)->isShortCircuitOperator()) {
          break;
        }
        goto process_vars;
      case Expression::KindOfExpressionList:
      case Expression::KindOfObjectMethodExpression:
      case Expression::KindOfDynamicFunctionCall:
      case Expression::KindOfSimpleFunctionCall:
      case Expression::KindOfNewObjectExpression:
      case Expression::KindOfQOpExpression:
        break;
      default: process_vars: {
        for (int i = 0, nk = e->getKidCount(); i < nk; i++) {
          ExpressionPtr k = e->getNthExpr(i);
          if (k && k->is(Expression::KindOfSimpleVariable) &&
              !static_pointer_cast<SimpleVariable>(
                k)->getAlwaysStash()) {
            process(k);
          }
        }
        break;
      }
    }
    process(e);
  }
  return WalkContinue;
}

int DataFlowWalker::afterEach(ConstructRawPtr cur, int i, ConstructRawPtr kid) {
  if (ExpressionRawPtr k = dynamic_pointer_cast<Expression>(kid)) {
    if (k->is(Expression::KindOfSimpleVariable) &&
        !static_pointer_cast<SimpleVariable>(k)->getAlwaysStash()) {
      if (ExpressionRawPtr e = dynamic_pointer_cast<Expression>(cur)) {
        switch (e->getKindOf()) {
          case Expression::KindOfBinaryOpExpression:
            if (!static_pointer_cast<BinaryOpExpression>(
                  e)->isShortCircuitOperator()) {
              return WalkContinue;
            }
            break;
          case Expression::KindOfExpressionList:
          case Expression::KindOfObjectMethodExpression:
          case Expression::KindOfDynamicFunctionCall:
          case Expression::KindOfSimpleFunctionCall:
          case Expression::KindOfNewObjectExpression:
          case Expression::KindOfQOpExpression:
            break;
          default:
            return WalkContinue;
        }
      }
      process(k);
    }
  }
  return WalkContinue;
}

void DataFlowWalker::processAccessChain(ExpressionPtr e) {
  if (!e) return;
  if (!e->is(Expression::KindOfObjectPropertyExpression) &&
      !e->is(Expression::KindOfArrayElementExpression)) {
    return;
  }
  for (int i = 0, n = e->getKidCount(); i < n; ++i) {
    ExpressionPtr kid(e->getNthExpr(i));
    if (kid && kid->hasContext(Expression::AccessContext)) {
      processAccessChain(kid);
      process(kid, true);
      break;
    }
  }
}

void DataFlowWalker::processAccessChainLA(ListAssignmentPtr la) {
  ExpressionList &lhs = *la->getVariables().get();
  for (int i = lhs.getCount(); i--; ) {
    ExpressionPtr ep = lhs[i];
    if (ep) {
      if (ep->is(Expression::KindOfListAssignment)) {
        processAccessChainLA(static_pointer_cast<ListAssignment>(ep));
      } else {
        processAccessChain(ep);
        processAccess(ep);
      }
    }
  }
}

void DataFlowWalker::process(ExpressionPtr e, bool doAccessChains) {
  if ((e->getContext() & (Expression::AssignmentLHS|Expression::OprLValue)) ||
      (!doAccessChains && e->hasContext(Expression::AccessContext))) {
    return;
  }

  switch (e->getKindOf()) {
    case Expression::KindOfListAssignment:
      processAccessChainLA(static_pointer_cast<ListAssignment>(e));
      processAccess(e);
      break;
    case Expression::KindOfArrayElementExpression:
    case Expression::KindOfObjectPropertyExpression:
      if (!e->hasContext(Expression::AccessContext)) {
        processAccessChain(e);
      }
      // fall through
    case Expression::KindOfObjectMethodExpression:
    case Expression::KindOfDynamicFunctionCall:
    case Expression::KindOfSimpleFunctionCall:
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
    case Expression::KindOfSimpleVariable:
    case Expression::KindOfDynamicVariable:
    case Expression::KindOfStaticMemberExpression:
    case Expression::KindOfConstantExpression:
      processAccess(e);
      break;
    case Expression::KindOfAssignmentExpression:
    case Expression::KindOfBinaryOpExpression:
    case Expression::KindOfUnaryOpExpression: {
      ExpressionPtr var = e->getStoreVariable();
      if (var && var->getContext() & (Expression::AssignmentLHS|
                                      Expression::OprLValue)) {
        processAccessChain(var);
        processAccess(var);
      }
      // fall through
    }
    default:
      processAccess(e);
      break;
  }
}
