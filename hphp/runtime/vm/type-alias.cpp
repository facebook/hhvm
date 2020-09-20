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

#include "hphp/runtime/vm/type-alias.h"

#include "hphp/runtime/vm/unit.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool TypeAlias::compat(const PreTypeAlias& alias) const {
  return (alias.type == AnnotType::Mixed && type == AnnotType::Mixed) ||
         (alias.type == type && alias.nullable == nullable &&
          Class::lookup(alias.value) == klass &&
          Unit::lookupRecordDesc(alias.value) == rec);
}

///////////////////////////////////////////////////////////////////////////////
}
