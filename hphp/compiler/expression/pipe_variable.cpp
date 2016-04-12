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

#include "hphp/compiler/expression/pipe_variable.h"

using namespace HPHP;

PipeVariable::PipeVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(PipeVariable))
{}

ExpressionPtr PipeVariable::clone() {
  auto const exp = std::make_shared<PipeVariable>(*this);
  Expression::deepCopy(exp);
  return exp;
}

void PipeVariable::analyzeProgram(AnalysisResultPtr ar) {}

void PipeVariable::outputPHP(CodeGenerator& cg, AnalysisResultPtr ar) {
  cg_printf("$$");
}
