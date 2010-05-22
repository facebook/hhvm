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

#ifndef __ENCAPS_LIST_EXPRESSION_H__
#define __ENCAPS_LIST_EXPRESSION_H__

#include <compiler/expression/expression.h>
#include <compiler/analysis/analysis_result.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(EncapsListExpression);

class EncapsListExpression : public Expression {
public:
  EncapsListExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                       int type, ExpressionListPtr expList);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual int getLocalEffects() const { return m_type == '`' ? UnknownEffect : NoEffect; }
  virtual bool canonCompare(ExpressionPtr e) const;
private:
  int m_type;
  ExpressionListPtr m_exps;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __ENCAPS_LIST_EXPRESSION_H__
