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

#ifndef incl_HPHP_ZEND_FUNCTIONS_H_
#define incl_HPHP_ZEND_FUNCTIONS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/util/hash.h"
#include "hphp/util/slice.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// zend logic: These are not string utilities, but zend's special language
// semantics.

/**
 * Testing whether a string is numeric or not.
 */
DataType is_numeric_string(const char *str, int length, int64_t *lval,
                           double *dval, int allow_errors = 0);

/**
 * Whether or not a string is a valid variable name.
 */
bool is_valid_var_name(const char *var_name, int len);

///////////////////////////////////////////////////////////////////////////////

/*
 * Adapted from ap_php_conv_10 for fast signed integer to string conversion.
 */
StringSlice conv_10(int64_t num, char* buf_end);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_FUNCTIONS_H_
