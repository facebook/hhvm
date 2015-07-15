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

#ifndef incl_HPHP_ENCAPS_LIST_EXPRESSION_H_
#define incl_HPHP_ENCAPS_LIST_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/analysis/analysis_result.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(EncapsListExpression);

class EncapsListExpression : public Expression {
public:
  EncapsListExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                       int type, ExpressionListPtr expList);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;

  int getLocalEffects() const override {
    return m_type == '`' ? UnknownEffect : NoEffect;
  }
   int getType() { return m_type;}
  ExpressionListPtr getExpressions() { return m_exps; }
  void stripConcat();
private:
  int m_type;
  ExpressionListPtr m_exps;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ENCAPS_LIST_EXPRESSION_H_
