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

#include "hphp/runtime/vm/debug/gdb-jit.h"
#include "hphp/util/portability.h"

/* __jit_debug_register_code() needs to be defined in a separate file
 * from the one it is called from. Otherwise gcc notices that it is empty,
 * and optimizes away the call. This prevents gdb from trapping updates to
 * the DWARF files emitted by HHVM */

KEEP_SECTION NEVER_INLINE
void __jit_debug_register_code() {};

/*
 * With identical-code-folding enabled in the linker,
 * __jit_debug_register_code tends to get folded with a bunch of other
 * functions, some of which are called a lot.  This results in things
 * going extremely slowly when run under gdb, because gdb spends all
 * its time rescanning its symbol tables.
 *
 * By adding KEEP_SECTION, both these functions end up in the same
 * section, and by doing something reasonably silly in this one, we
 * can ensure that no code folding takes place.
 */
KEEP_SECTION NEVER_INLINE
long jit_debug_register_code_uniquifier(int a, char* b, long* c) {
  return a + *b * *c;
};
