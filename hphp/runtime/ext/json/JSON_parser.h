/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_JSON_PARSER_H_
#define incl_HPHP_JSON_PARSER_H_

#include <cstdint>

namespace HPHP {

struct StringBuffer;
struct Variant;
struct IMarker;

void utf16_to_utf8(StringBuffer& buf, unsigned short utf16);
bool JSON_parser(Variant& z, const char *p, int length,
                 bool assoc, int depth, int64_t options);
void json_parser_init(); // called at request-init
void json_parser_scan(IMarker&);

enum json_error_codes {
  JSON_ERROR_NONE = 0,
  JSON_ERROR_DEPTH,
  JSON_ERROR_STATE_MISMATCH,
  JSON_ERROR_CTRL_CHAR,
  JSON_ERROR_SYNTAX,
  JSON_ERROR_UTF8,
  JSON_ERROR_RECURSION,
  JSON_ERROR_INF_OR_NAN,
  JSON_ERROR_UNSUPPORTED_TYPE
};

json_error_codes json_get_last_error_code();
const char* json_get_last_error_msg();
void json_set_last_error_code(json_error_codes ec);

}

#endif // incl_HPHP_JSON_PARSER_H_
