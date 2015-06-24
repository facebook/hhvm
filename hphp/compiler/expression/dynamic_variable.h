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

#ifndef incl_HPHP_DYNAMIC_VARIABLE_H_
#define incl_HPHP_DYNAMIC_VARIABLE_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DynamicVariable);

class DynamicVariable : public Expression {
public:
  DynamicVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  int getLocalEffects() const override { return NoEffect; }
  bool isRefable(bool checkError = false) const override { return true;}

  ExpressionPtr getSubExpression() const { return m_exp; }
private:
  ExpressionPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DYNAMIC_VARIABLE_H_
