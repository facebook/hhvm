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

#include <compiler/analysis/ast_walker.h>
#include <compiler/statement/statement.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

int FunctionWalker::before(ConstructRawPtr cp) {
  if (StatementRawPtr s = dynamic_pointer_cast<Statement>(cp)) {
    Statement::KindOf stype = s->getKindOf();
    switch (stype) {
      case Statement::KindOfFunctionStatement:
      case Statement::KindOfMethodStatement:
      case Statement::KindOfClassStatement:
      case Statement::KindOfInterfaceStatement:
        return WalkSkip;
      default:
        break;
    }
  }
  return WalkContinue;
}
