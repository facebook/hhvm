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

#ifndef incl_HPHP_LIST_ASSIGNMENT_H_
#define incl_HPHP_LIST_ASSIGNMENT_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/variable_table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ListAssignment);

class ListAssignment : public Expression {
public:
  enum RHSKind {
    Regular,
    Checked,
    Null
  };
  ListAssignment(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                 ExpressionListPtr variables, ExpressionPtr array,
                 bool rhsFirst = false);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  RHSKind getRHSKind() const { return m_rhsKind; }

  ExpressionListPtr getVariables() const { return m_variables; }
  ExpressionPtr getArray() const { return m_array; }
  bool isRhsFirst() { return m_rhsFirst; }
private:
  ExpressionListPtr m_variables;
  ExpressionPtr m_array;
  RHSKind m_rhsKind;
  bool m_rhsFirst;

  void setLValue();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_LIST_ASSIGNMENT_H_
