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

#ifndef incl_HPHP_GROUP_CLAUSE_H_
#define incl_HPHP_GROUP_CLAUSE_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(GroupClause);

class GroupClause : public Expression {
public:
  GroupClause(EXPRESSION_CONSTRUCTOR_PARAMETERS,
    ExpressionPtr coll, ExpressionPtr key);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  ExpressionPtr getColl() { return m_coll; }
  ExpressionPtr getKey() { return m_key; }
private:
  ExpressionPtr m_coll;
  ExpressionPtr m_key;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_GROUP_CLAUSE_H_
