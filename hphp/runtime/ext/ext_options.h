/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_OPTIONS_H_
#define incl_HPHP_EXT_OPTIONS_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_assert_options(int what, CVarRef value = null_variant);
Variant f_assert(CVarRef assertion);
int64_t f_dl(CStrRef library);
bool f_extension_loaded(CStrRef name);
Array f_get_loaded_extensions(bool zend_extensions = false);
Array f_get_extension_funcs(CStrRef module_name);
Variant f_get_cfg_var(CStrRef option);
String f_get_current_user();
Array f_get_defined_constants(bool categorize = false);
String f_get_include_path();
void f_restore_include_path();
String f_set_include_path(CStrRef new_include_path);
Array f_get_included_files();
Array f_inclued_get_data();
int64_t f_get_magic_quotes_gpc();
int64_t f_get_magic_quotes_runtime();
Array f_get_required_files();
Variant f_getenv(CStrRef varname);
int64_t f_getlastmod();
int64_t f_getmygid();
int64_t f_getmyinode();
int64_t f_getmypid();
int64_t f_getmyuid();
Array f_getopt(CStrRef options, CVarRef longopts = null_variant);
Array f_getrusage(int who = 0);
bool f_clock_getres(int clk_id, VRefParam sec, VRefParam nsec);
bool f_clock_gettime(int clk_id, VRefParam sec, VRefParam nsec);
bool f_clock_settime(int clk_id, int64_t sec, int64_t nsec);
int64_t f_cpu_get_count();
String f_cpu_get_model();
String f_ini_alter(CStrRef varname, CStrRef newvalue);
Array f_ini_get_all(CStrRef extension = null_string);
String f_ini_get(CStrRef varname);
void f_ini_restore(CStrRef varname);
String f_ini_set(CStrRef varname, CStrRef newvalue);
int64_t f_memory_get_allocation();
int64_t f_memory_get_peak_usage(bool real_usage = false);
int64_t f_memory_get_usage(bool real_usage = false);
Variant f_php_ini_loaded_file();
String f_php_ini_scanned_files();
String f_php_logo_guid();
String f_php_sapi_name();
String f_php_uname(CStrRef mode = null_string);
bool f_phpcredits(int flag = 0);
bool f_phpinfo(int what = 0);
String f_phpversion(CStrRef extension = null_string);
bool f_putenv(CStrRef setting);
bool f_set_magic_quotes_runtime(bool new_setting);
void f_set_time_limit(int seconds);
String f_sys_get_temp_dir();
Variant f_version_compare(CStrRef version1, CStrRef version2, CStrRef sop = null_string);
bool f_gc_enabled();
void f_gc_enable();
void f_gc_disable();
int64_t f_gc_collect_cycles();
String f_zend_logo_guid();
int64_t f_zend_thread_id();
String f_zend_version();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_OPTIONS_H_
