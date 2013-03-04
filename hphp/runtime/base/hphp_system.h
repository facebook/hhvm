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

#ifndef __CPP_BASE_HPHP_SYSTEM_H__
#define __CPP_BASE_HPHP_SYSTEM_H__

///////////////////////////////////////////////////////////////////////////////

/**
 * This is the file that's included at top of a code generated system file.
 */

#include <runtime/base/macros.h>
#include <runtime/base/zend/zend_functions.h>
#include <util/exception.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/util/smart_object.h>
#include <runtime/base/list_assignment.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/variable_table.h>
#include <runtime/base/string_util.h>
#include <util/util.h>
#include <runtime/base/file/plain_file.h>
#include <runtime/base/class_info.h>
#include <runtime/base/externals.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/util/simple_counter.h>
#include <util/shm_counter.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char* getHphpCompilerVersion();
const char* getHphpCompilerId();
HphpBinary::Type getHphpBinaryType();

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#define DECLARE_SYSTEM_GLOBALS(sg)                      \
  SystemGlobals *sg ATTRIBUTE_UNUSED =       \
    get_global_variables();

#define DECLARE_GLOBAL_VARIABLES(g)                     \
  SystemGlobals *g ATTRIBUTE_UNUSED =      \
    get_global_variables();

///////////////////////////////////////////////////////////////////////////////

#endif // __CPP_BASE_HPHP_SYSTEM_H__
