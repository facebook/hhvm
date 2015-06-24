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

#ifndef incl_HPHP_ASSIGNMENT_EXPRESSION_H_
#define incl_HPHP_ASSIGNMENT_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AssignmentExpression);
struct TypedValue;

class AssignmentExpression : public Expression, public IParseHandler {
public:
  AssignmentExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                       ExpressionPtr variable, ExpressionPtr value,
                       bool ref, bool rhsFirst = false);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;

  // implementing IParseHandler
  void onParseRecur(AnalysisResultConstPtr ar, FileScopeRawPtr fs,
                    ClassScopePtr scope) override;

  bool isRefable(bool checkError = false) const override {
    if (checkError) return true;
    return m_value->isRefable() &&
           (m_value->getContext() & Expression::RefValue);
  }

  ExpressionPtr getVariable() { return m_variable;}
  ExpressionPtr getStoreVariable() const override { return m_variable; }
  ExpressionPtr getValue() { return m_value;}
  void setVariable(ExpressionPtr v) { m_variable = v; }
  void setValue(ExpressionPtr v) { m_value = v; }
  bool isRhsFirst() { return m_rhsFirst; }
  int getLocalEffects() const override;

  // $GLOBALS[<literal-string>] = <scalar>;
  bool isSimpleGlobalAssign(StringData **name, TypedValue *tv) const;
private:
  ExpressionPtr optimize(AnalysisResultConstPtr ar);

  ExpressionPtr m_variable;
  ExpressionPtr m_value;
  bool m_ref;
  bool m_rhsFirst;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ASSIGNMENT_EXPRESSION_H_
