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
#pragma once

#include "hphp/runtime/vm/as-base.h"
#include "hphp/runtime/vm/fcall-args-flags.h"
#include "hphp/runtime/vm/hhbc-shared.h"

#include "rust/cxx.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This header contains routines linked into HackC.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Convert an fcall flag `to a string of space-separated flag names.
 */
rust::String fcall_flags_to_string_ffi(FCallArgsFlags);
}
