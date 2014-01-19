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

#include "hphp/runtime/vm/debug/gdb-jit.h"

/* __jit_debug_regiser_code() needs to be defined in a separate file
 * from the one it is called from. Otherwise gcc notices that it is empty,
 * and optimizes away the call. This prevents gdb from trapping updates to
 * the DWARF files emitted by HHVM */

void __attribute__((noinline)) __jit_debug_register_code() { };
