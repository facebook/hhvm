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

#include "hphp/compiler/analysis/capture_extractor.h"
#include "hphp/compiler/expression/join_clause.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/parser/hphp.tab.hpp"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Rewrites the expression rooted in ep so that it is in a form
 * that a query processor can evaluate while referencing only
 * state that is contained in the query processor or supplied
 * to the query processor in the form of arguments to the query.
 * For instance, a reference to a local variable in the scope
 * containing the query expression will be rewritten into a
 * reference to a (synthetic) parameter of the query expression.
 * This is similar to the way lambda expressions capture variables
 * from their enclosing environment.
 * Note that rewriting implies allocating new objects.
 * The original construct is not mutated in any way.
 * If the original construct is already in the right form, it is
 * returned as is.
 */
ExpressionPtr CaptureExtractor::rewrite(ExpressionPtr ep) {
  assert(ep != nullptr);
  switch (ep->getKindOf()) {
    case Expression::KindOfQueryExpression: {
      return rewriteQuery(static_pointer_cast<QueryExpression>(ep));
    }
    case Expression::KindOfSelectClause: {
      // leave select clauses alone, another visitor deals with them.
      return ep;
    }
    case Expression::KindOfFromClause:
    case Expression::KindOfLetClause:
    case Expression::KindOfIntoClause:
    case Expression::KindOfWhereClause: {
      return rewriteSimpleClause(static_pointer_cast<SimpleQueryClause>(ep));
    }
    case Expression::KindOfGroupClause:
    case Expression::KindOfJoinClause:
    case Expression::KindOfOrderbyClause:
    case Expression::KindOfOrdering: {
      // leave these alone. they are query specific and not parameterizable.
      return ep;
    }
    case Expression::KindOfObjectPropertyExpression: {
      return rewriteObjectProperty(
        static_pointer_cast<ObjectPropertyExpression>(ep));
    }
    case Expression::KindOfSimpleFunctionCall: {
      return rewriteCall(static_pointer_cast<SimpleFunctionCall>(ep));
    }
    case Expression::KindOfScalarExpression: {
      // Leave scalars alone. If the query processor can't handle them
      // rewriting won't help.
      return ep;
    }
    case Expression::KindOfUnaryOpExpression: {
      return rewriteUnary(static_pointer_cast<UnaryOpExpression>(ep));
    }
    case Expression::KindOfBinaryOpExpression: {
      return rewriteBinary(static_pointer_cast<BinaryOpExpression>(ep));
    }
    case Expression::KindOfSimpleVariable: {
      return rewriteSimpleVariable(static_pointer_cast<SimpleVariable>(ep));
    }
    case Expression::KindOfExpressionList: {
      return rewriteExpressionList(static_pointer_cast<ExpressionList>(ep));
    }
    default: {
      // If we get here, the expression is not a candidate for evaluation
      // by the query processor, so just turn it into a query parameter.
      return newQueryParamRef(ep);
    }
  }
}

/**
 * Appends the given expression to end of the m_capturedExpressions list
 * and creates a new expression with the same scope and source location
 * that represents a reference to a query parameter. Query parameters do
 * not have a source code equivalent, but inform the query processor that
 * this expression represents the ith argument value, where i is zero based
 * and forms the last character of the special string @query_param_i.
 */
SimpleVariablePtr CaptureExtractor::newQueryParamRef(ExpressionPtr ae) {
  assert(ae != nullptr);
  char count = '0' + m_capturedExpressions.size();
  std::string pname = "@query_param_";
  pname.push_back(count);
  auto param =
    std::make_shared<SimpleVariable>(ae->getScope(), ae->getRange(), pname);
  m_capturedExpressions.push_back(ae);
  return param;
}

/**
 * If one or more of the arguments of the function call depend on query only
 * state (query local but not a query parameter reference), then rewrite
 * all of the arguments to be query local and return the rewritten
 * function call (the query processor has to either figure out a way to call
 * the function or must cause a runtime error when faced with the call).
 * Otherwise, rewrite the call as a query parameter reference.
 */
ExpressionPtr CaptureExtractor::rewriteCall(SimpleFunctionCallPtr sfc) {
  assert(sfc != nullptr);
  if (sfc->hadBackslash() || sfc->getClass() || sfc->hasStaticClass()) {
    return newQueryParamRef(sfc);
  }
  auto args = sfc->getParams();
  auto pc = args == nullptr ? 0 : args->getCount();
  bool isQueryCall = false;
  for (int i = 0; i < pc; i++) {
    auto arg = (*args)[i];
    assert(arg != nullptr);
    isQueryCall |= this->dependsOnQueryOnlyState(arg);
  }
  if (!isQueryCall) return newQueryParamRef(sfc);
  auto newArgs =
    std::make_shared<ExpressionList>(args->getScope(), args->getRange());
  bool noRewrites = true;
  for (int i = 0; i < pc; i++) {
    auto arg = (*args)[i];
    auto newArg = rewrite(arg);
    if (arg != newArg) noRewrites = false;
    newArgs->addElement(newArg);
  }
  if (noRewrites) return sfc;
  return std::make_shared<SimpleFunctionCall>(
    sfc->getScope(), sfc->getRange(),
    sfc->getOriginalName(), false, newArgs, ExpressionPtr());
}

/**
 * Traverses the expression tree rooted at e and returns true if
 * any node in the tree is a simple variable that references a
 * name in m_boundVars.
 */
bool CaptureExtractor::dependsOnQueryOnlyState(ExpressionPtr e) {
  assert(e != nullptr);
  if (e->getKindOf() == Expression::KindOfSimpleVariable) {
    auto sv = static_pointer_cast<SimpleVariable>(e);
    auto varName = sv->getName();
    for (auto &boundVar : m_boundVars) {
      if (varName == boundVar) return true;
    }
    return false;
  }
  auto numKids = e->getKidCount();
  for (int i = 0; i < numKids; i++) {
    auto ei = e->getNthExpr(i);
    if (ei == nullptr) return false; //Default param
    if (dependsOnQueryOnlyState(ei)) return true;
  }
  return false;
}

/**
 * If a simple variable refers to a name bound inside the query
 * then leave it alone. If not, rewrite it to be reference to
 * a query parameter.
 */
SimpleVariablePtr CaptureExtractor::rewriteSimpleVariable(
  SimpleVariablePtr sv) {
  assert(sv != nullptr);
  auto varName = sv->getName();
  for (auto &boundVar : m_boundVars) {
    if (varName == boundVar) return sv;
  }
  return newQueryParamRef(sv);
}

/**
* Query expressions introduce a local scope with names introduced
* by some of the clauses of the query expression. This needs
* special handling so that we can track variables local to the query.
*/
QueryExpressionPtr CaptureExtractor::rewriteQuery(QueryExpressionPtr qe) {
  assert(qe != nullptr);
  auto clauses = qe->getClauses();
  auto newClauses = rewriteExpressionList(clauses);
  if (clauses == newClauses) return qe;
  auto result =
    std::make_shared<QueryExpression>(qe->getScope(), qe->getRange(),
                                      newClauses);
  return result;
}

/**
 * Rewrites any expression query clauses in this list of clauses, taking care
 * to track variables local to the query.
 */
ExpressionListPtr CaptureExtractor::rewriteExpressionList(ExpressionListPtr l) {
  int np = 0;
  int nc = l->getCount();
  auto newList =
    std::make_shared<ExpressionList>(l->getScope(), l->getRange());
  bool noRewrites = true;
  for (int i = 0; i < nc; i++) {
    auto e = (*l)[i];
    assert(e != nullptr);
    auto kind = e->getKindOf();
    switch (kind) {
      case Expression::KindOfIntoClause: {
        // The into expression is in the scope of the into clause
        SimpleQueryClausePtr qcp(static_pointer_cast<SimpleQueryClause>(e));
        m_boundVars.push_back(qcp->getIdentifier());
        np++;
        break;
      }
      case Expression::KindOfJoinClause: {
        JoinClausePtr jcp(static_pointer_cast<JoinClause>(e));
        m_boundVars.push_back(jcp->getVar());
        np++;
        break;
      }
      default:
        break;
    }
    auto ne = rewrite(e);
    if (ne != e) noRewrites = false;
    newList->addElement(ne);
    // deal with clauses that introduce names for subsequent clauses
    switch (kind) {
      case Expression::KindOfFromClause:
      case Expression::KindOfLetClause: {
        SimpleQueryClausePtr qcp(static_pointer_cast<SimpleQueryClause>(e));
        m_boundVars.push_back(qcp->getIdentifier());
        np++;
        break;
      }
      case Expression::KindOfJoinClause: {
        JoinClausePtr jcp(static_pointer_cast<JoinClause>(e));
        auto groupId = jcp->getGroup();
        if (!groupId.empty()) {
          m_boundVars.push_back(groupId);
          np++;
        }
        break;
      }
      default:
        break;
    }
  }
  while (np-- > 0) m_boundVars.pop_back();
  if (noRewrites) return l;
  return newList;
}

/*
 * If the expression of a simple query clause is query local, then
 * return the clause as is. Otherwise return a clone of the clause
 * with the expression rewritten to reference a query parameter.
 */
SimpleQueryClausePtr CaptureExtractor::rewriteSimpleClause(
    SimpleQueryClausePtr sc) {
  assert (sc != nullptr);
  auto expr = sc->getExpression();
  auto newExpr = rewrite(expr);
  if (expr == newExpr) return sc;
  auto rsc = static_pointer_cast<SimpleQueryClause>(sc->clone());
  rsc->setExpression(newExpr);
  return rsc;
}

/*
 * If the object expression is query local, that is, if it is a simple variable
 * referring to a name declared in a query clause, or itself a query local
 * object expression, then if keep this expression as is. If not, then
 * rewrite this expression into a query parameter reference.
 */
ExpressionPtr CaptureExtractor::rewriteObjectProperty(
  ObjectPropertyExpressionPtr ope) {
  assert(ope != nullptr);
  auto obj = ope->getObject();
  if (this->dependsOnQueryOnlyState(obj)) {
    auto prop = ope->getProperty();
    if (prop->getKindOf() == Expression::KindOfScalarExpression) {
      auto scalar = static_pointer_cast<ScalarExpression>(prop);
      const string &propName = scalar->getLiteralString();
      if (!propName.empty()) {
        return ope;
      }
    }
  }
  return newQueryParamRef(ope);
}

/**
 * If the unary operation is not PHP specific, but something a query
 * processor can handle (+ - ! ~), then rewrite the operand to something
 * the query processor can evaluate (such as a query parameter reference)
 * and rewrite the entire expression to use the rewritten operand.
 * If the rewritten operand is the same as the original operand, just
 * return the expression as is.
 */
ExpressionPtr CaptureExtractor::rewriteUnary(UnaryOpExpressionPtr ue) {
  assert (ue != nullptr);
  if (!ue->getFront()) return nullptr;
  switch (ue->getOp()) {
    case '+':
    case '-':
    case '!':
    case '~':
      break; // Could be something the query processor can handle
    default:
      return newQueryParamRef(ue);
  }
  auto expr = ue->getExpression();
  auto newExpr = rewrite(expr);
  if (expr == newExpr) return ue;
  return std::make_shared<UnaryOpExpression>(
    ue->getScope(), ue->getRange(), newExpr, ue->getOp(), true);
}

/**
 * If the binary operation is not PHP specific, but something a query
 * processor can handle (+ - * and so on), then rewrite the operands to
 * something the query processor can evaluate (such as a query parameter
 * references) and rewrite the entire expression to use the rewritten operands.
 * If the rewritten operands are the same as the original operands, just
 * return the expression as is.
 */
ExpressionPtr CaptureExtractor::rewriteBinary(BinaryOpExpressionPtr be) {
  assert(be != nullptr);
  switch (be->getOp()) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '&':
    case '|':
    case '^':
    case T_IS_IDENTICAL:
    case T_IS_EQUAL:
    case '>':
    case '<':
    case T_IS_GREATER_OR_EQUAL:
    case T_IS_SMALLER_OR_EQUAL:
    case T_IS_NOT_IDENTICAL:
    case T_IS_NOT_EQUAL:
    case T_BOOLEAN_OR:
    case T_BOOLEAN_AND:
    case T_LOGICAL_OR:
    case T_LOGICAL_AND:
    case '.':
      break; // Could be something the query processor can handle

    default:
      return newQueryParamRef(be);
  }
  auto expr1 = be->getExp1();
  auto expr2 = be->getExp2();
  auto newExpr1 = rewrite(expr1);
  auto newExpr2 = rewrite(expr2);
  if (expr1 == newExpr1 && expr2 == newExpr2) return be;
  return
    std::make_shared<BinaryOpExpression>(
      be->getScope(), be->getRange(), newExpr1, newExpr2, be->getOp());
}

}
