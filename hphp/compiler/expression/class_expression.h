/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CLASS_EXPRESSION_H_
#define incl_HPHP_CLASS_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/function_call.h"
#include "hphp/parser/parser.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassExpression;
using ClassExpressionPtr = std::shared_ptr<ClassExpression>;

class ClassStatement;
using ClassStatementPtr = std::shared_ptr<ClassStatement>;

class ClassExpression : public FunctionCall {
public:
  ClassExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                  ClassStatementPtr cls,
                  ExpressionListPtr params);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;

  ClassStatementPtr getClass() { return m_cls; }

private:
  ClassStatementPtr m_cls;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_EXPRESSION_H_
