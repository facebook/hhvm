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
#ifndef incl_HPHP_VM_BACKUP_GC_H_
#define incl_HPHP_VM_BACKUP_GC_H_

#include <string>

namespace HPHP { namespace VM {

//////////////////////////////////////////////////////////////////////

/*
 * Collect cyclic garbage hanging out in the heap.  Expensive
 * function, but potentially useful for long-running scripts.
 *
 * Returns: a string containing information about what was collected.
 * (The format of this string is subject to change; it's intended to
 * be usable as programmer-visible information.)
 */
std::string gc_collect_cycles();

/*
 * Detect cyclic garbage and dump it as GML to filename.  Intended to
 * allow introspection of the user heap so application-level code can
 * be changed to avoid cyclic garbage if desired.
 */
void gc_detect_cycles(const std::string& filename);

//////////////////////////////////////////////////////////////////////

}}

#endif
