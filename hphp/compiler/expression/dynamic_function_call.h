/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __DYNAMIC_FUNCTION_CALL_H__
#define __DYNAMIC_FUNCTION_CALL_H__

#include <compiler/expression/function_call.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DynamicFunctionCall);

class DynamicFunctionCall : public FunctionCall {
public:
  DynamicFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                      ExpressionPtr name, ExpressionListPtr params,
                      ExpressionPtr cls);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar, int state);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __DYNAMIC_FUNCTION_CALL_H__
