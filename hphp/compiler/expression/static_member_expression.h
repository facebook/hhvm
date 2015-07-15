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

#ifndef incl_HPHP_STATIC_MEMBER_EXPRESSION_H_
#define incl_HPHP_STATIC_MEMBER_EXPRESSION_H_

#include "hphp/compiler/expression/static_class_name.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StaticMemberExpression);

class StaticMemberExpression : public Expression, public StaticClassName {
public:
  StaticMemberExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                         ExpressionPtr classExp, ExpressionPtr exp);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;
  int getLocalEffects() const override { return NoEffect; }
  bool isRefable(bool checkError = false) const override { return true;}

  ExpressionPtr getExp() { return m_exp; }

private:
  bool findMember(AnalysisResultPtr ar, std::string &name, Symbol *&sym);

  ExpressionPtr    m_exp;
  bool             m_valid;
  ClassScopeRawPtr m_resolvedClass;
  bool             m_dynamicClass;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_STATIC_MEMBER_EXPRESSION_H_
