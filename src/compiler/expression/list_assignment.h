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

#ifndef __LIST_ASSIGNMENT_H__
#define __LIST_ASSIGNMENT_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ListAssignment);

class ListAssignment : public Expression {
public:
  ListAssignment(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                 ExpressionListPtr variables, ExpressionPtr array);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  ExpressionListPtr getVariables() const { return m_variables; }
private:
  ExpressionListPtr m_variables;
  ExpressionPtr m_array;

  void setLValue();
  void outputCPPAssignment(CodeGenerator &cg, AnalysisResultPtr ar,
      const std::string &arrTmp);

  void preOutputVariables(CodeGenerator &cg, AnalysisResultPtr ar, int state);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar, int state);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LIST_ASSIGNMENT_H__
