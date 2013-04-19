/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

TypedValue* fh_base64_decode(TypedValue* _rv, Value* data, bool strict) asm("_ZN4HPHP15f_base64_decodeERKNS_6StringEb");

Value* fh_base64_encode(Value* _rv, Value* data) asm("_ZN4HPHP15f_base64_encodeERKNS_6StringE");

TypedValue* fh_get_headers(TypedValue* _rv, Value* url, int format) asm("_ZN4HPHP13f_get_headersERKNS_6StringEi");

Value* fh_get_meta_tags(Value* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP15f_get_meta_tagsERKNS_6StringEb");

TypedValue* fh_http_build_query(TypedValue* _rv, TypedValue* formdata, Value* numeric_prefix, Value* arg_separator) asm("_ZN4HPHP18f_http_build_queryERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_parse_url(TypedValue* _rv, Value* url, int component) asm("_ZN4HPHP11f_parse_urlERKNS_6StringEi");

Value* fh_rawurldecode(Value* _rv, Value* str) asm("_ZN4HPHP14f_rawurldecodeERKNS_6StringE");

Value* fh_rawurlencode(Value* _rv, Value* str) asm("_ZN4HPHP14f_rawurlencodeERKNS_6StringE");

Value* fh_urldecode(Value* _rv, Value* str) asm("_ZN4HPHP11f_urldecodeERKNS_6StringE");

Value* fh_urlencode(Value* _rv, Value* str) asm("_ZN4HPHP11f_urlencodeERKNS_6StringE");

} // namespace HPHP
