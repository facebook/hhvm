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
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"

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
    new ExpressionList(selectExpr->getScope(), selectExpr->getRange())
  );
  m_selectedColumns = newList;
  this->collectSelectedColumns(sc);
  SimpleFunctionCallPtr callTuple(
    new SimpleFunctionCall(selectExpr->getScope(), selectExpr->getRange(),
      "tuple", false, newList, nullptr)
  );
  sc->setExpression(callTuple);
}

/**
 * Constructs an expression that is a reference to a property named
 * columnName that lives in an object that is passed as the actual
 * value of parameter __query_result_row__.
 */
ObjectPropertyExpressionPtr getResultColumn(
  ExpressionPtr expr, std::string columnName) {
  SimpleVariablePtr obj(
    new SimpleVariable(expr->getScope(),
        expr->getRange(), "__query_result_row__")
  );
  ScalarExpressionPtr propName(
    new ScalarExpression(expr->getScope(),
    expr->getRange(), columnName)
  );
  ObjectPropertyExpressionPtr result(
    new ObjectPropertyExpression(expr->getScope(),
        expr->getRange(), obj, propName, PropAccessType::Normal)
  );
  return result;
}

/**
 * Rewrites the expression by replacing all expression of the type
 * x or x->prop, where x is the name of a query local and prop is a simple
 * string, with expressions of the type __query_result_row__->x and
 * __query_result_row__->x::prop.
 */
void SelectRewriter::rewriteClientSide(ExpressionPtr ep) {
  assert(ep != nullptr);
  int nkid = ep->getKidCount();
  for (int i = 0; i < nkid; i++) {
    auto kid = dynamic_pointer_cast<Expression>(ep->getNthKid(i));
    if (kid == nullptr) continue;
    switch (kid->getKindOf()) {
      case Expression::KindOfSimpleVariable: {
        auto sv = static_pointer_cast<SimpleVariable>(kid);
        if (this->isTableName(sv)) {
          ep->setNthKid(i, getResultColumn(sv, sv->getName()));
        } else {
          this->rewriteClientSide(kid);
        }
        break;
      }
      case Expression::KindOfObjectPropertyExpression: {
        auto ope = static_pointer_cast<ObjectPropertyExpression>(kid);
        auto obj = ope->getObject();
        auto prop = ope->getProperty();
        if (this->isTableName(obj) && this->isColumnName(prop)) {
          auto svar = static_pointer_cast<SimpleVariable>(obj);
          auto scalar = static_pointer_cast<ScalarExpression>(prop);
          auto qualName = svar->getName() +
            "::" + scalar->getOriginalLiteralString();
          ep->setNthKid(i, getResultColumn(ope, qualName));
        } else {
          this->rewriteClientSide(kid);
        }
        break;
      }
      default: {
        this->rewriteClientSide(kid);
        break;
      }
    }
  }
}

/**
 * Traverses the expression and adds every occurrence of a subexpression of
 * form x or x->prop, where x is the name of a query local and prop is a simple
 * string, to the m_selectedColumns list.
 */
void SelectRewriter::collectSelectedColumns(ExpressionPtr ep) {
  assert(ep != nullptr);
  int nkid = ep->getKidCount();
  for (int i = 0; i < nkid; i++) {
    auto kid = dynamic_pointer_cast<Expression>(ep->getNthKid(i));
    if (kid == nullptr) continue;
    switch (kid->getKindOf()) {
      case Expression::KindOfSimpleVariable: {
        if (this->isTableName(kid)) {
          m_selectedColumns->addElement(kid);
        }
        break;
      }
      case Expression::KindOfObjectPropertyExpression: {
        auto ope = static_pointer_cast<ObjectPropertyExpression>(kid);
        if (this->isTableName(ope->getObject()) &&
            this->isColumnName(ope->getProperty())) {
          m_selectedColumns->addElement(ope);
        } else {
          this->collectSelectedColumns(kid);
        }
        break;
      }
      default: {
        this->collectSelectedColumns(kid);
        break;
      }
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
