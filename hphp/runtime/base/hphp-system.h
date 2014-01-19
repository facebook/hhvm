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

#ifndef incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_
#define incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_

///////////////////////////////////////////////////////////////////////////////

/**
 * This is the file that's included at top of a code generated system file.
 */

#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/smart-object.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/simple-counter.h"
#include "hphp/util/shm-counter.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char* getHphpCompilerVersion();
const char* getHphpCompilerId();

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_
