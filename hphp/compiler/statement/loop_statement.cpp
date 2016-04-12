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

#include "hphp/compiler/statement/loop_statement.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/parser/parser.h"

using namespace HPHP;

LoopStatement::LoopStatement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS) :
    Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES) {
}
