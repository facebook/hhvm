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

#ifndef __DYNAMIC_VARIABLE_H__
#define __DYNAMIC_VARIABLE_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DynamicVariable);

class DynamicVariable : public Expression {
public:
  DynamicVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual int getLocalEffects() const { return NoEffect; }
  virtual bool isRefable(bool checkError = false) const { return true;}

  ExpressionPtr getSubExpression() const { return m_exp; }
private:
  ExpressionPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __DYNAMIC_VARIABLE_H__
