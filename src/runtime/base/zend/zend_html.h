/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_ZEND_HTML_H__
#define __HPHP_ZEND_HTML_H__

#include <util/base.h>
#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * Major departures from Zend:
 *
 * 1. We are only supporting UTF-8 encoding. So, dropped charset parameters.
 *    Major reason for this is because the original get_next_char() bothers me,
 *    sacrificing performance for some character sets that people rarely used
 *    or that people shouldn't use. UTF-8 should really be the standard string
 *    format everywhere, and we ought to write coding specifilized for it to
 *    take full advantage of it: one example would be the new html encoding
 *    function that simply do *p one a time iterating through the strings to
 *    look for special characters for entity escaping.
 *
 * 2. HTML encoding function no longer encodes entities other than the basic
 *    ones. There is no need to encode them, since all browsers support UTF-8
 *    natively, and we are ok to send out UTF-8 encoding characters without
 *    turning them into printable ASCIIs. Basic entities are encoded for
 *    a different reason! In fact, I personally don't see why HTML spec has
 *    those extended list of entities, other than historical artifacts.
 *
 * 3. Double encoding parameter is not supported. That really sounds like
 *    a workaround of buggy coding. I don't find a legit use for that yet.
 */

char *string_html_encode(const char *input, int &len, bool encode_double_quote,
                         bool encode_single_quote);
char *string_html_decode(const char *input, int &len);
Array string_get_html_translation_table(int which, int quote_style);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_STRING_H__
