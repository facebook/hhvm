/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EVAL_LIST_ASSIGNMENT_EXPRESSION_H__
#define __EVAL_LIST_ASSIGNMENT_EXPRESSION_H__

#include <runtime/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ListElement);
DECLARE_AST_PTR(ListAssignmentExpression);

class ListElement : public Construct {
public:
  ListElement(CONSTRUCT_ARGS);
  virtual void set(VariableEnvironment &env, CVarRef val) const = 0;
};

class LvalListElement : public ListElement {
public:
  LvalListElement(CONSTRUCT_ARGS, LvalExpressionPtr lval);
  virtual void set(VariableEnvironment &env, CVarRef val) const;
  virtual void dump() const;
private:
  LvalExpressionPtr m_lval;
};

class SubListElement : public ListElement {
public:
  SubListElement(CONSTRUCT_ARGS, const std::vector<ListElementPtr> &elems);
  virtual void set(VariableEnvironment &env, CVarRef val) const;
  virtual void dump() const;
private:
  std::vector<ListElementPtr> m_elems;
};

class ListAssignmentExpression : public Expression {
public:
  ListAssignmentExpression(EXPRESSION_ARGS, ListElementPtr lhs,
                           ExpressionPtr rhs);
  virtual Variant eval(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  ListElementPtr m_lhs;
  ExpressionPtr m_rhs;
};


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_LIST_ASSIGNMENT_EXPRESSION_H__ */
