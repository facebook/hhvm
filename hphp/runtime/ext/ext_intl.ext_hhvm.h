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

long fh_intl_get_error_code() asm("_ZN4HPHP21f_intl_get_error_codeEv");

Value* fh_intl_get_error_message(Value* _rv) asm("_ZN4HPHP24f_intl_get_error_messageEv");

Value* fh_intl_error_name(Value* _rv, long error_code) asm("_ZN4HPHP17f_intl_error_nameEl");

bool fh_intl_is_failure(long error_code) asm("_ZN4HPHP17f_intl_is_failureEl");

TypedValue* fh_collator_asort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP16f_collator_asortERKNS_7VariantERKNS_14VRefParamValueEl");

TypedValue* fh_collator_compare(TypedValue* _rv, TypedValue* obj, Value* str1, Value* str2) asm("_ZN4HPHP18f_collator_compareERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_collator_create(TypedValue* _rv, Value* locale) asm("_ZN4HPHP17f_collator_createERKNS_6StringE");

TypedValue* fh_collator_get_attribute(TypedValue* _rv, TypedValue* obj, long attr) asm("_ZN4HPHP24f_collator_get_attributeERKNS_7VariantEl");

TypedValue* fh_collator_get_error_code(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_collator_get_error_codeERKNS_7VariantE");

TypedValue* fh_collator_get_error_message(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP28f_collator_get_error_messageERKNS_7VariantE");

TypedValue* fh_collator_get_locale(TypedValue* _rv, TypedValue* obj, long type) asm("_ZN4HPHP21f_collator_get_localeERKNS_7VariantEl");

TypedValue* fh_collator_get_strength(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_collator_get_strengthERKNS_7VariantE");

TypedValue* fh_collator_set_attribute(TypedValue* _rv, TypedValue* obj, long attr, long val) asm("_ZN4HPHP24f_collator_set_attributeERKNS_7VariantEll");

TypedValue* fh_collator_set_strength(TypedValue* _rv, TypedValue* obj, long strength) asm("_ZN4HPHP23f_collator_set_strengthERKNS_7VariantEl");

TypedValue* fh_collator_sort_with_sort_keys(TypedValue* _rv, TypedValue* obj, TypedValue* arr) asm("_ZN4HPHP30f_collator_sort_with_sort_keysERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* fh_collator_sort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP15f_collator_sortERKNS_7VariantERKNS_14VRefParamValueEl");

TypedValue* fh_idn_to_ascii(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP14f_idn_to_asciiERKNS_6StringEllRKNS_14VRefParamValueE");

TypedValue* fh_idn_to_unicode(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP16f_idn_to_unicodeERKNS_6StringEllRKNS_14VRefParamValueE");

TypedValue* fh_idn_to_utf8(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP13f_idn_to_utf8ERKNS_6StringEllRKNS_14VRefParamValueE");

} // namespace HPHP
