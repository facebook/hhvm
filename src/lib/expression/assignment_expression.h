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

#ifndef __ASSIGNMENT_EXPRESSION_H__
#define __ASSIGNMENT_EXPRESSION_H__

#include <lib/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AssignmentExpression);

class AssignmentExpression : public Expression, public IParseHandler {
public:
  AssignmentExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                       ExpressionPtr variable, ExpressionPtr value,
                       bool ref);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

  static TypePtr inferTypesImpl(AnalysisResultPtr ar, TypePtr type,
                                bool coerce, ExpressionPtr variable,
                                ExpressionPtr value = ExpressionPtr());

  ExpressionPtr getVariable() { return m_variable;}
  ExpressionPtr getValue() { return m_value;}

private:
  ExpressionPtr makeIdCall(AnalysisResultPtr ar);

  ExpressionPtr m_variable;
  ExpressionPtr m_value;
  bool m_ref;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __ASSIGNMENT_EXPRESSION_H__
