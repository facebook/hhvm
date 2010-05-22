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

#ifndef __CLASS_VARIABLE_H__
#define __CLASS_VARIABLE_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassVariable);

class ClassVariable : public Statement, public IParseHandler {
public:
  ClassVariable(STATEMENT_CONSTRUCTOR_PARAMETERS,
                ModifierExpressionPtr modifiers,
                ExpressionListPtr declaration);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

private:
  ModifierExpressionPtr m_modifiers;
  ExpressionListPtr m_declaration;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_VARIABLE_H__
