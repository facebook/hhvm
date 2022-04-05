/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/module.h"

#include "hphp/runtime/base/string-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

//////////////////////////////////////////////////////////////////////

void Module::prettyPrint(std::ostream& out) const {
  out << "Module " << name->data();
  if (attrs & AttrUnique)     out << " (unique)";
  if (attrs & AttrPersistent) out << " (persistent)";
  out << std::endl;
}

}
