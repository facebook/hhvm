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

#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/tv-val.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

// Converts a TypedValue `source' to its uncounted form, so that it can be
// shared across requests. We call it after doing a raw copy of the array
// elements without manipulating refcounts. That's safe, because an uncounted
// value will never hold references to refcounted values.
void ConvertTvToUncounted(tv_lval source, DataWalker::PointerMap* seen);

// The analogue of decRefAndRelease for an uncounted value.
void ReleaseUncountedTv(tv_lval lval);

//////////////////////////////////////////////////////////////////////////////

}
