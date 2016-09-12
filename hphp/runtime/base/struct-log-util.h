/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef incl_HPHP_RUNTIME_BASE_STRUCT_LOG_UTIL_H_
#define incl_HPHP_RUNTIME_BASE_STRUCT_LOG_UTIL_H_

#include <folly/Range.h>

#include "hphp/util/struct-log.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Shared utilities for structured logging that use VM datatypes.
 */
namespace StructuredLog {
void logSerDes(const char* format,
               const char* op,
               const String& serialized,
               const Variant& value);
};

///////////////////////////////////////////////////////////////////////////////

}

#endif
