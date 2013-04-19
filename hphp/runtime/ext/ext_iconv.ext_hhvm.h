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

TypedValue* fh_iconv_mime_encode(TypedValue* _rv, Value* field_name, Value* field_value, TypedValue* preferences) asm("_ZN4HPHP19f_iconv_mime_encodeERKNS_6StringES2_RKNS_7VariantE");

TypedValue* fh_iconv_mime_decode(TypedValue* _rv, Value* encoded_string, int mode, Value* charset) asm("_ZN4HPHP19f_iconv_mime_decodeERKNS_6StringEiS2_");

TypedValue* fh_iconv_mime_decode_headers(TypedValue* _rv, Value* encoded_headers, int mode, Value* charset) asm("_ZN4HPHP27f_iconv_mime_decode_headersERKNS_6StringEiS2_");

TypedValue* fh_iconv_get_encoding(TypedValue* _rv, Value* type) asm("_ZN4HPHP20f_iconv_get_encodingERKNS_6StringE");

bool fh_iconv_set_encoding(Value* type, Value* charset) asm("_ZN4HPHP20f_iconv_set_encodingERKNS_6StringES2_");

TypedValue* fh_iconv(TypedValue* _rv, Value* in_charset, Value* out_charset, Value* str) asm("_ZN4HPHP7f_iconvERKNS_6StringES2_S2_");

TypedValue* fh_iconv_strlen(TypedValue* _rv, Value* str, Value* charset) asm("_ZN4HPHP14f_iconv_strlenERKNS_6StringES2_");

TypedValue* fh_iconv_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* charset) asm("_ZN4HPHP14f_iconv_strposERKNS_6StringES2_iS2_");

TypedValue* fh_iconv_strrpos(TypedValue* _rv, Value* haystack, Value* needle, Value* charset) asm("_ZN4HPHP15f_iconv_strrposERKNS_6StringES2_S2_");

TypedValue* fh_iconv_substr(TypedValue* _rv, Value* str, int offset, int length, Value* charset) asm("_ZN4HPHP14f_iconv_substrERKNS_6StringEiiS2_");

Value* fh_ob_iconv_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP18f_ob_iconv_handlerERKNS_6StringEi");

} // namespace HPHP
