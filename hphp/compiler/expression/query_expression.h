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

#ifndef incl_HPHP_QUERY_EXPRESSION_H_
#define incl_HPHP_QUERY_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/runtime/base/static-string-table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(QueryOrderby);

class QueryOrderby : public Expression {
public:
  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

protected:
  explicit QueryOrderby(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS);

  ExpressionListPtr m_expressions;
};

DECLARE_BOOST_TYPES(QueryExpression);

class QueryExpression : public QueryOrderby {
public:
  QueryExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                  ExpressionPtr head, ExpressionPtr body);
  QueryExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                  ExpressionListPtr clauses);

  virtual ExpressionPtr clone() {
    QueryExpressionPtr exp(new QueryExpression(*this));
    Expression::deepCopy(exp);
    exp->m_expressions = Clone(m_expressions);
    exp->m_queryargs = Clone(m_queryargs);
    exp->m_querystr = m_querystr;
    return exp;
  }

  ExpressionListPtr getClauses() const { return m_expressions; }
  ExpressionListPtr getQueryArguments();
  StringData* getQueryString();
private:
  void serializeQueryExpression();

  ExpressionListPtr m_queryargs;
  StringData* m_querystr;
};

DECLARE_BOOST_TYPES(OrderbyClause);

class OrderbyClause : public QueryOrderby {
public:
  OrderbyClause(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr orderings)
  : QueryOrderby(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(OrderbyClause)) {
    assert(orderings && orderings->is(Expression::KindOfExpressionList));
    m_expressions = static_pointer_cast<ExpressionList>(orderings);
  }

  virtual ExpressionPtr clone() {
    OrderbyClausePtr exp(new OrderbyClause(*this));
    Expression::deepCopy(exp);
    exp->m_expressions = Clone(m_expressions);
    return exp;
  }

  ExpressionListPtr getOrderings() const { return m_expressions; }
};
///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_QUERY_EXPRESSION_H_
