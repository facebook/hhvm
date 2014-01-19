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

#ifndef incl_HPHP_INTL_CONVERT_H_
#define incl_HPHP_INTL_CONVERT_H_

#include "hphp/runtime/base/complex-types.h"
#include <unicode/ustring.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void intl_convert_utf8_to_utf16(UChar** target, int* target_len,
                                const char* src, int src_len,
                                UErrorCode* status);
void intl_convert_utf16_to_utf8(char** target, int* target_len,
                                const UChar* src, int src_len,
                                UErrorCode*  status);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_INTL_CONVERT_H_
