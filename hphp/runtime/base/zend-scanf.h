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

#ifndef incl_HPHP_ZEND_SCANF_H_
#define incl_HPHP_ZEND_SCANF_H_

#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * How PHP scans a string. Called by ext/ext_string.cpp.
 */
int string_sscanf(const char *string, const char *format, int numVars,
                  Variant &return_value);

#define SCAN_SUCCESS     0
#define SCAN_ERROR_EOF  -1      // indicates premature termination of scan
                                // can be caused by bad parameters or format
                                // string.
#define SCAN_ERROR_INVALID_FORMAT     (SCAN_ERROR_EOF - 1)
#define SCAN_ERROR_VAR_PASSED_BYVAL   (SCAN_ERROR_INVALID_FORMAT - 1)
#define SCAN_ERROR_WRONG_PARAM_COUNT  (SCAN_ERROR_VAR_PASSED_BYVAL - 1)

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_SCANF_H_
