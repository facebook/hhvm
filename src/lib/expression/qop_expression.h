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

#ifndef __QOP_EXPRESSION_H__
#define __QOP_EXPRESSION_H__

#include <lib/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(QOpExpression);

class QOpExpression : public Expression {
public:
  QOpExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                ExpressionPtr condition, ExpressionPtr expYes,
                ExpressionPtr expNo);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool hasEffect() const { return m_effect;}

private:
  ExpressionPtr m_condition;
  ExpressionPtr m_expYes;
  ExpressionPtr m_expNo;
  bool m_effect;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __QOP_EXPRESSION_H__
