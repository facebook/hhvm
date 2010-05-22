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

#ifndef __CLASS_CONSTANT_H__
#define __CLASS_CONSTANT_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassConstant);

class ClassConstant : public Statement, public IParseHandler {
public:
  ClassConstant(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

private:
  ExpressionListPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_CONSTANT_H__
