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

#include "hphp/compiler/analysis/select_rewriters.h"
#include "hphp/compiler/expression/join_clause.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/unary_op_expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void SelectRewriter::rewriteQuery(QueryExpressionPtr qe) {
  assert(qe != nullptr);
  this->rewriteExpressionList(qe->getClauses());
}

/**
 * Calls this->rewrite(e) for every element, e, of the given list.
 * Also sets up m_boundVars to contain the right set of query locals
 * before each call to rewrite.
 */
void SelectRewriter::rewriteExpressionList(ExpressionListPtr l) {
  assert(l != nullptr);
  int np = 0;
  int nc = l->getCount();
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
    this->rewrite(e);
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
}

/**
 * Dispatches the right rewriter method depending on the type of ep.
 */
void SelectRewriter::rewrite(ExpressionPtr ep) {
  assert(ep != nullptr);
  switch (ep->getKindOf()) {
    case Expression::KindOfQueryExpression: {
      auto qe = static_pointer_cast<QueryExpression>(ep);
      this->rewriteQuery(qe);
      break;
    }
    case Expression::KindOfExpressionList: {
      this->rewriteExpressionList(static_pointer_cast<ExpressionList>(ep));
      break;
    }
    case Expression::KindOfSelectClause: {
      this->rewriteSelect(static_pointer_cast<SelectClause>(ep));
      return;
    }
    default: {
      this->rewriteOther(ep);
      break;
    }
  }
}

/**
 * Recursively call rewrite on every child of ep.
 */
void SelectRewriter::rewriteOther(ExpressionPtr ep) {
  int nkids = ep->getKidCount();
  for (int i = 0; i < nkids; i++) {
    auto kid = dynamic_pointer_cast<Expression>(ep->getNthKid(i));
    if (kid != nullptr) this->rewrite(kid);
  }
}

/**
 * Rewrites sc as either a client side or server side clause, depending
 * on the value of m_serverSide.
 */
void SelectRewriter::rewriteSelect(SelectClausePtr sc) {
  assert(sc != nullptr);
  if (!m_serverSide) {
    m_clientSideSelectClause = static_pointer_cast<SelectClause>(sc->clone());
    this->rewriteClientSide(m_clientSideSelectClause);
    return;
  }

  // Server side rewrite: Extract all references to query only variables and
  // properties into a single tuple.
  auto selectExpr = sc->getExpression();
  ExpressionListPtr newList(
    new ExpressionList(selectExpr->getScope(), selectExpr->getLocation())
  );
  m_selectedColumns = newList;
  this->collectSelectedColumns(sc);
  SimpleFunctionCallPtr callTuple(
    new SimpleFunctionCall(selectExpr->getScope(), selectExpr->getLocation(),
      "tuple", false, newList, nullptr)
  );
  sc->setExpression(callTuple);
}

/**
 * Constructs an expression that is a reference __query_result_row__[i]
 * where i is the index of expr in m_selectedColumns.
 */
ExpressionPtr SelectRewriter::getResultColumn(ExpressionPtr expr) {
  assert(expr != nullptr);
  auto scope = expr->getScope();
  auto location = expr->getLocation();

  SimpleVariablePtr obj(
    new SimpleVariable(scope, location, "__query_result_row__")
  );
  ScalarExpressionPtr offset(
    new ScalarExpression(scope, location, m_columnIndex++)
  );
  ArrayElementExpressionPtr ae(
    new ArrayElementExpression(scope, location, obj, offset)
  );
  return ae;
}

bool SelectRewriter::isServerSideExpression(ExpressionPtr ep) {
  assert(ep != nullptr);
  switch (ep->getKindOf()) {
    case Expression::KindOfSimpleVariable: {
      return this->isTableName(static_pointer_cast<SimpleVariable>(ep));
    }
    case Expression::KindOfObjectPropertyExpression: {
      auto ope = static_pointer_cast<ObjectPropertyExpression>(ep);
      auto obj = ope->getObject();
      auto prop = ope->getProperty();
      return this->isTableName(obj) && this->isColumnName(prop);
    }
    case Expression::KindOfSimpleFunctionCall: {
      auto sfc = static_pointer_cast<SimpleFunctionCall>(ep);
      if (sfc->hadBackslash() ||
        (sfc->getClass() != nullptr && !sfc->getClassName().empty())) {
        return false;
      }
      auto func_name = sfc->getName();
      return func_name.compare(0, 2, "q_") == 0;
    }
    case Expression::KindOfUnaryOpExpression: {
      auto uop = static_pointer_cast<UnaryOpExpression>(ep);
      if (!uop->getFront()) return false;
      switch (uop->getOp()) {
        case '+':
        case '-':
        case '!':
        case '~': {
          auto expr = uop->getExpression();
          return this->isServerSideExpression(expr);
        }
        default:
          return false;
      }
    }
    case Expression::KindOfBinaryOpExpression: {
      auto bop = static_pointer_cast<BinaryOpExpression>(ep);
      switch (bop->getOp()) {
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
        case T_LOGICAL_XOR:
        case '.': {
          auto expr1 = bop->getExp1();
          auto expr2 = bop->getExp2();
          return this->isServerSideExpression(expr1) &&
            this->isServerSideExpression(expr2);
        }
        default:
          return false;
      }
    }
    case Expression::KindOfQOpExpression: {
      auto qop = static_pointer_cast<QOpExpression>(ep);
      auto condition = qop->getCondition();
      auto yes = qop->getYes();
      auto no = qop->getNo();
      return this->isServerSideExpression(condition) &&
        this->isServerSideExpression(no) &&
        (yes == nullptr || this->isServerSideExpression(yes));
    }
    default:
      return false;
  }
}

/**
 * Rewrites the expression by replacing all subexpressions that are to be
 * evaluated by the query server, with expressions of the form
 * __query_result_row__[i] where i is the index of ep in m_selectedColumns.
 */
void SelectRewriter::rewriteClientSide(ExpressionPtr ep) {
  assert(ep != nullptr);
  if (ep->getKindOf() == Expression::KindOfSelectClause) {
    m_columnIndex = 0;
  }
  int nkid = ep->getKidCount();
  for (int i = 0; i < nkid; i++) {
    auto kid = dynamic_pointer_cast<Expression>(ep->getNthKid(i));
    if (kid == nullptr) continue;
    if (this->isServerSideExpression(kid)) {
      ep->setNthKid(i, this->getResultColumn(kid));
    } else {
      this->rewriteClientSide(kid);
    }
  }
}

/**
 * Traverses the expression and adds every occurrence of a subexpression
 * that are to be evaluated by the query server to the m_selectedColumns list.
 */
void SelectRewriter::collectSelectedColumns(ExpressionPtr ep) {
  assert(ep != nullptr);
  int nkid = ep->getKidCount();
  for (int i = 0; i < nkid; i++) {
    auto kid = dynamic_pointer_cast<Expression>(ep->getNthKid(i));
    if (kid == nullptr) continue;
    if (this->isServerSideExpression(kid)) {
      m_selectedColumns->addElement(kid);
    } else {
      this->collectSelectedColumns(kid);
    }
  }
}

/**
 * Returns true if ep is a simple variable whose name matches that of
 * a query local introduced by a query clause.
 */
bool SelectRewriter::isTableName(ExpressionPtr ep) {
  assert(ep != nullptr);
  if (ep->getKindOf() != Expression::KindOfSimpleVariable) return false;
  auto sv = static_pointer_cast<SimpleVariable>(ep);
  auto varName = sv->getName();
  for (auto &boundVar : m_boundVars) {
    if (varName == boundVar) return true;
  }
  return false;
}

/**
 * Returns true if ep is scalar expression that contains an identifier
 * (as opposed to a literal string).
 */
bool SelectRewriter::isColumnName(ExpressionPtr ep) {
  assert(ep != nullptr);
  if (ep->getKindOf() != Expression::KindOfScalarExpression) return false;
  auto scalar = static_pointer_cast<ScalarExpression>(ep);
  return !scalar->isLiteralString();
}

}
