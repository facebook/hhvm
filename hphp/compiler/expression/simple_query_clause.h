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

#ifndef incl_HPHP_SIMPLE_QUERY_CLAUSE_H_
#define incl_HPHP_SIMPLE_QUERY_CLAUSE_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleQueryClause);

class SimpleQueryClause : public Expression {
public:
  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  std::string getIdentifier() const { return m_identifier; }
  ExpressionPtr getExpression() { return m_expression; }
  void setExpression(ExpressionPtr value) { m_expression = value; }
protected:
  SimpleQueryClause(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS,
             const std::string &identifier, ExpressionPtr collection);

  std::string m_identifier;
  ExpressionPtr m_expression;
};

DECLARE_BOOST_TYPES(FromClause);

class FromClause : public SimpleQueryClause {
public:
  FromClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
             const std::string &identifier, ExpressionPtr collection)
    : SimpleQueryClause(
        EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(FromClause),
        identifier, collection)  {
  }

  ExpressionPtr clone() override {
    FromClausePtr exp(new FromClause(*this));
    Expression::deepCopy(exp);
    exp->m_identifier = m_identifier;
    exp->m_expression = Clone(m_expression);
    return exp;
  }

};

DECLARE_BOOST_TYPES(LetClause);

class LetClause : public SimpleQueryClause {
public:
  LetClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
            const std::string &identifier, ExpressionPtr expression)
    : SimpleQueryClause(
        EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(LetClause),
        identifier, expression)  {
  }

  ExpressionPtr clone() override {
    LetClausePtr exp(new LetClause(*this));
    Expression::deepCopy(exp);
    exp->m_identifier = m_identifier;
    exp->m_expression = Clone(m_expression);
    return exp;
  }

};

DECLARE_BOOST_TYPES(WhereClause);

class WhereClause : public SimpleQueryClause {
public:
  WhereClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
              ExpressionPtr condition)
    : SimpleQueryClause(
        EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(WhereClause),
        "", condition)  {
  }

  ExpressionPtr clone() override {
    WhereClausePtr exp(new WhereClause(*this));
    Expression::deepCopy(exp);
    exp->m_identifier = m_identifier;
    exp->m_expression = Clone(m_expression);
    return exp;
  }

};

DECLARE_BOOST_TYPES(SelectClause);

class SelectClause : public SimpleQueryClause {
public:
  SelectClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
               ExpressionPtr expression)
    : SimpleQueryClause(
        EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(SelectClause),
        "", expression)  {
  }

  ExpressionPtr clone() override {
    SelectClausePtr exp(new SelectClause(*this));
    Expression::deepCopy(exp);
    exp->m_identifier = m_identifier;
    exp->m_expression = Clone(m_expression);
    return exp;
  }

};

DECLARE_BOOST_TYPES(IntoClause);

class IntoClause : public SimpleQueryClause {
public:
  IntoClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
             const std::string &identifier, ExpressionPtr query)
    : SimpleQueryClause(
        EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(IntoClause),
        identifier, query)  {
  }

  ExpressionPtr clone() override {
    IntoClausePtr exp(new IntoClause(*this));
    Expression::deepCopy(exp);
    exp->m_identifier = m_identifier;
    exp->m_expression = Clone(m_expression);
    return exp;
  }

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SIMPLE_QUERY_CLAUSE_H_
