/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_
#define incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_

///////////////////////////////////////////////////////////////////////////////

/**
 * This is the file that's included at top of a code generated system file.
 */

#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/zend_functions.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/smart_object.h"
#include "hphp/runtime/base/list_assignment.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/string_util.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/plain_file.h"
#include "hphp/runtime/base/class_info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array_init.h"
#include "hphp/runtime/base/array_iterator.h"
#include "hphp/runtime/base/string_buffer.h"
#include "hphp/runtime/base/simple_counter.h"
#include "hphp/util/shm_counter.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char* getHphpCompilerVersion();
const char* getHphpCompilerId();

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_CPP_BASE_HPHP_SYSTEM_H_
