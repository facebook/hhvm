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

TypedValue* fh_assert_options(TypedValue* _rv, int what, TypedValue* value) asm("_ZN4HPHP16f_assert_optionsEiRKNS_7VariantE");

TypedValue* fh_assert(TypedValue* _rv, TypedValue* assertion) asm("_ZN4HPHP8f_assertERKNS_7VariantE");

long fh_dl(Value* library) asm("_ZN4HPHP4f_dlERKNS_6StringE");

bool fh_extension_loaded(Value* name) asm("_ZN4HPHP18f_extension_loadedERKNS_6StringE");

Value* fh_get_loaded_extensions(Value* _rv, bool zend_extensions) asm("_ZN4HPHP23f_get_loaded_extensionsEb");

Value* fh_get_extension_funcs(Value* _rv, Value* module_name) asm("_ZN4HPHP21f_get_extension_funcsERKNS_6StringE");

Value* fh_get_cfg_var(Value* _rv, Value* option) asm("_ZN4HPHP13f_get_cfg_varERKNS_6StringE");

Value* fh_get_current_user(Value* _rv) asm("_ZN4HPHP18f_get_current_userEv");

Value* fh_get_defined_constants(Value* _rv, TypedValue* categorize) asm("_ZN4HPHP23f_get_defined_constantsERKNS_7VariantE");

Value* fh_get_include_path(Value* _rv) asm("_ZN4HPHP18f_get_include_pathEv");

void fh_restore_include_path() asm("_ZN4HPHP22f_restore_include_pathEv");

Value* fh_set_include_path(Value* _rv, Value* new_include_path) asm("_ZN4HPHP18f_set_include_pathERKNS_6StringE");

Value* fh_get_included_files(Value* _rv) asm("_ZN4HPHP20f_get_included_filesEv");

Value* fh_inclued_get_data(Value* _rv) asm("_ZN4HPHP18f_inclued_get_dataEv");

long fh_get_magic_quotes_gpc() asm("_ZN4HPHP22f_get_magic_quotes_gpcEv");

long fh_get_magic_quotes_runtime() asm("_ZN4HPHP26f_get_magic_quotes_runtimeEv");

Value* fh_get_required_files(Value* _rv) asm("_ZN4HPHP20f_get_required_filesEv");

TypedValue* fh_getenv(TypedValue* _rv, Value* varname) asm("_ZN4HPHP8f_getenvERKNS_6StringE");

long fh_getlastmod() asm("_ZN4HPHP12f_getlastmodEv");

long fh_getmygid() asm("_ZN4HPHP10f_getmygidEv");

long fh_getmyinode() asm("_ZN4HPHP12f_getmyinodeEv");

long fh_getmypid() asm("_ZN4HPHP10f_getmypidEv");

long fh_getmyuid() asm("_ZN4HPHP10f_getmyuidEv");

Value* fh_getopt(Value* _rv, Value* options, TypedValue* longopts) asm("_ZN4HPHP8f_getoptERKNS_6StringERKNS_7VariantE");

Value* fh_getrusage(Value* _rv, int who) asm("_ZN4HPHP11f_getrusageEi");

bool fh_clock_getres(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP14f_clock_getresEiRKNS_14VRefParamValueES2_");

bool fh_clock_gettime(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP15f_clock_gettimeEiRKNS_14VRefParamValueES2_");

bool fh_clock_settime(int clk_id, long sec, long nsec) asm("_ZN4HPHP15f_clock_settimeEill");

long fh_cpu_get_count() asm("_ZN4HPHP15f_cpu_get_countEv");

Value* fh_cpu_get_model(Value* _rv) asm("_ZN4HPHP15f_cpu_get_modelEv");

Value* fh_ini_alter(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP11f_ini_alterERKNS_6StringES2_");

Value* fh_ini_get_all(Value* _rv, Value* extension) asm("_ZN4HPHP13f_ini_get_allERKNS_6StringE");

Value* fh_ini_get(Value* _rv, Value* varname) asm("_ZN4HPHP9f_ini_getERKNS_6StringE");

void fh_ini_restore(Value* varname) asm("_ZN4HPHP13f_ini_restoreERKNS_6StringE");

Value* fh_ini_set(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP9f_ini_setERKNS_6StringES2_");

long fh_memory_get_allocation() asm("_ZN4HPHP23f_memory_get_allocationEv");

long fh_memory_get_peak_usage(bool real_usage) asm("_ZN4HPHP23f_memory_get_peak_usageEb");

long fh_memory_get_usage(bool real_usage) asm("_ZN4HPHP18f_memory_get_usageEb");

Value* fh_php_ini_scanned_files(Value* _rv) asm("_ZN4HPHP23f_php_ini_scanned_filesEv");

Value* fh_php_logo_guid(Value* _rv) asm("_ZN4HPHP15f_php_logo_guidEv");

Value* fh_php_sapi_name(Value* _rv) asm("_ZN4HPHP15f_php_sapi_nameEv");

Value* fh_php_uname(Value* _rv, Value* mode) asm("_ZN4HPHP11f_php_unameERKNS_6StringE");

bool fh_phpcredits(int flag) asm("_ZN4HPHP12f_phpcreditsEi");

bool fh_phpinfo(int what) asm("_ZN4HPHP9f_phpinfoEi");

Value* fh_phpversion(Value* _rv, Value* extension) asm("_ZN4HPHP12f_phpversionERKNS_6StringE");

bool fh_putenv(Value* setting) asm("_ZN4HPHP8f_putenvERKNS_6StringE");

bool fh_set_magic_quotes_runtime(bool new_setting) asm("_ZN4HPHP26f_set_magic_quotes_runtimeEb");

void fh_set_time_limit(int seconds) asm("_ZN4HPHP16f_set_time_limitEi");

Value* fh_sys_get_temp_dir(Value* _rv) asm("_ZN4HPHP18f_sys_get_temp_dirEv");

Value* fh_zend_logo_guid(Value* _rv) asm("_ZN4HPHP16f_zend_logo_guidEv");

long fh_zend_thread_id() asm("_ZN4HPHP16f_zend_thread_idEv");

Value* fh_zend_version(Value* _rv) asm("_ZN4HPHP14f_zend_versionEv");

TypedValue* fh_version_compare(TypedValue* _rv, Value* version1, Value* version2, Value* sop) asm("_ZN4HPHP17f_version_compareERKNS_6StringES2_S2_");

bool fh_gc_enabled() asm("_ZN4HPHP12f_gc_enabledEv");

void fh_gc_enable() asm("_ZN4HPHP11f_gc_enableEv");

void fh_gc_disable() asm("_ZN4HPHP12f_gc_disableEv");

long fh_gc_collect_cycles() asm("_ZN4HPHP19f_gc_collect_cyclesEv");

} // namespace HPHP
