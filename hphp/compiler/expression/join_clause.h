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

#ifndef incl_HPHP_JOIN_CLAUSE_H_
#define incl_HPHP_JOIN_CLAUSE_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(JoinClause);

class JoinClause : public Expression {
public:
  JoinClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
      const std::string &var, ExpressionPtr coll, ExpressionPtr left,
      ExpressionPtr right, const std::string &group);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  std::string getVar() const { return m_var; }
  ExpressionPtr getColl() { return m_coll; }
  ExpressionPtr getLeft() { return m_left; }
  ExpressionPtr getRight() { return m_right; }
  std::string getGroup() { return m_group; }
private:
  std::string m_var;
  ExpressionPtr m_coll;
  ExpressionPtr m_left;
  ExpressionPtr m_right;
  std::string m_group;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_JOIN_CLAUSE_H_
