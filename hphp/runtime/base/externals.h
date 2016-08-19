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

#ifndef incl_HPHP_CPP_BASE_EXTERNALS_H_
#define incl_HPHP_CPP_BASE_EXTERNALS_H_

///////////////////////////////////////////////////////////////////////////////

/**
 * All external dependencies of runtime/base. This file may not include any
 * generated file based on user code, as this file is included by some
 * runtime/base .cpp files.
 */

#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * This file is included from inside an extern "C" block in some places, but
 * these functions have return types that are incompatible with C linkage.
 */
extern "C++" {

/**
 * Invoking an arbitrary user-defined function.
 */
Variant invoke(const char *function, const Variant& params, strhash_t hash = -1,
    bool tryInterp = true, bool fatal = true, bool useWeakTypes = false);

} // extern C++

///////////////////////////////////////////////////////////////////////////////
}

#endif
