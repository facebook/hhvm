/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/func-util.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

uint32_t closure_num_use_vars(borrowed_ptr<const php::Func> f) {
  // Properties on the closure object are either use vars, or storage
  // for static locals.  The first N are the use vars.
  return f->cls->properties.size() - f->staticLocals.size();
}

//////////////////////////////////////////////////////////////////////

}}
