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
#ifndef incl_HPHP_FUNC_UTIL_H_
#define incl_HPHP_FUNC_UTIL_H_

#include <cstdint>

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {
namespace php { struct Func; }

//////////////////////////////////////////////////////////////////////

/*
 * Return the number of use-vars for a closure body.
 *
 * Pre: f->isClosureBody
 */
uint32_t closure_num_use_vars(borrowed_ptr<const php::Func>);

//////////////////////////////////////////////////////////////////////

}}

#endif
