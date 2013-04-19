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

TypedValue* fh_simplexml_load_string(TypedValue* _rv, Value* data, Value* class_name, long options, Value* ns, bool is_prefix) asm("_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_lS2_b");

TypedValue* fh_simplexml_load_file(TypedValue* _rv, Value* filename, Value* class_name, long options, Value* ns, bool is_prefix) asm("_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_lS2_b");

TypedValue* fh_libxml_get_errors(TypedValue* _rv) asm("_ZN4HPHP19f_libxml_get_errorsEv");

TypedValue* fh_libxml_get_last_error(TypedValue* _rv) asm("_ZN4HPHP23f_libxml_get_last_errorEv");

void fh_libxml_clear_errors() asm("_ZN4HPHP21f_libxml_clear_errorsEv");

bool fh_libxml_use_internal_errors(TypedValue* use_errors) asm("_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE");

void fh_libxml_set_streams_context(Value* streams_context) asm("_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE");

bool fh_libxml_disable_entity_loader(bool disable) asm("_ZN4HPHP30f_libxml_disable_entity_loaderEb");

} // namespace HPHP
