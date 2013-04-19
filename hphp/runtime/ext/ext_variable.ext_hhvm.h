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

Value* fh_gettype(Value* _rv, TypedValue* v) asm("_ZN4HPHP9f_gettypeERKNS_7VariantE");

Value* fh_get_resource_type(Value* _rv, Value* handle) asm("_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE");

long fh_intval(TypedValue* v, long base) asm("_ZN4HPHP8f_intvalERKNS_7VariantEl");

double fh_doubleval(TypedValue* v) asm("_ZN4HPHP11f_doublevalERKNS_7VariantE");

double fh_floatval(TypedValue* v) asm("_ZN4HPHP10f_floatvalERKNS_7VariantE");

Value* fh_strval(Value* _rv, TypedValue* v) asm("_ZN4HPHP8f_strvalERKNS_7VariantE");

bool fh_settype(TypedValue* var, Value* type) asm("_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE");

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP9f_is_boolERKNS_7VariantE");

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP8f_is_intERKNS_7VariantE");

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP12f_is_integerERKNS_7VariantE");

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP9f_is_longERKNS_7VariantE");

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP11f_is_doubleERKNS_7VariantE");

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP10f_is_floatERKNS_7VariantE");

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP12f_is_numericERKNS_7VariantE");

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP9f_is_realERKNS_7VariantE");

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP11f_is_stringERKNS_7VariantE");

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP11f_is_scalarERKNS_7VariantE");

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP10f_is_arrayERKNS_7VariantE");

bool fh_is_object(TypedValue* var) asm("_ZN4HPHP11f_is_objectERKNS_7VariantE");

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP13f_is_resourceERKNS_7VariantE");

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP9f_is_nullERKNS_7VariantE");

TypedValue* fh_print_r(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP9f_print_rERKNS_7VariantEb");

TypedValue* fh_var_export(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP12f_var_exportERKNS_7VariantEb");

void fh_var_dump(int64_t _argc, TypedValue* expression, Value* _argv) asm("_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE");

void fh_debug_zval_dump(TypedValue* variable) asm("_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE");

TypedValue* fh_unserialize(TypedValue* _rv, Value* str, Value* class_whitelist) asm("_ZN4HPHP13f_unserializeERKNS_6StringERKNS_5ArrayE");

Value* fh_get_defined_vars(Value* _rv) asm("_ZN4HPHP18f_get_defined_varsEv");

bool fh_import_request_variables(Value* types, Value* prefix) asm("_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_");

long fh_extract(Value* var_array, int extract_type, Value* prefix) asm("_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE");

} // namespace HPHP
