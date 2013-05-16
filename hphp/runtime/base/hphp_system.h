/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/runtime/base/zend/zend_functions.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/string_offset.h"
#include "hphp/runtime/base/util/smart_object.h"
#include "hphp/runtime/base/list_assignment.h"
#include "hphp/runtime/base/resource_data.h"
#include "hphp/runtime/base/string_util.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/file/plain_file.h"
#include "hphp/runtime/base/class_info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/util/string_buffer.h"
#include "hphp/runtime/base/util/simple_counter.h"
#include "hphp/util/shm_counter.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char* getHphpCompilerVersion();
const char* getHphpCompilerId();

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#define DECLARE_SYSTEM_GLOBALS(sg)                      \
  SystemGlobals *sg ATTRIBUTE_UNUSED =       \
    get_global_variables();

#define DECLARE_GLOBAL_VARIABLES(g)                     \
  SystemGlobals *g ATTRIBUTE_UNUSED =      \
    get_global_variables();

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_
