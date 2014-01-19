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

#ifndef incl_HPHP_CLASS_CONSTANT_H_
#define incl_HPHP_CLASS_CONSTANT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassConstant);

class ClassConstant : public Statement, public IParseHandler {
public:
  ClassConstant(STATEMENT_CONSTRUCTOR_PARAMETERS, std::string typeConstraint,
		        ExpressionListPtr exp);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar);

  // implementing IParseHandler
  virtual void onParseRecur(AnalysisResultConstPtr ar, ClassScopePtr scope);

  std::string getTypeConstraint() const { return m_typeConstraint; }

  ExpressionListPtr getConList() { return m_exp; }
private:
  std::string m_typeConstraint;
  ExpressionListPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_CONSTANT_H_
