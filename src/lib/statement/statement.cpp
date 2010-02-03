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

#include <lib/statement/statement.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Statement::Statement(LocationPtr loc, KindOf kindOf)
  : Construct(loc), m_kindOf(kindOf) {
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void Statement::addElement(StatementPtr stmt) {
  ASSERT(false);
}

void Statement::insertElement(StatementPtr stmt, int index /* = 0 */) {
  ASSERT(false);
}
