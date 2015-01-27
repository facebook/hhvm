/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_OBJPROF_H_
#define incl_HPHP_EXT_OBJPROF_H_

#include "hphp/runtime/base/base-includes.h"
#include <semaphore.h>

/*
 *                       Project ObjProf
 * What is it?
 * Breakdown of allocated memory by object types.
 *
 * How does it work?
 * We turn on tracking of ObjectData* (instances) and then we
 * traverse them to measure their memory.
 *
 * How do I use it?
 * Calling the objprof_start will start tracking instances.
 * Calling the objprof_get_data extension will trigger the scan.
 */

namespace HPHP {
}

#endif // incl_HPHP_EXT_OBJPROF_H_
