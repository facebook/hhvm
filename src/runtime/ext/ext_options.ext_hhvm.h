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

/*
HPHP::Variant HPHP::f_assert_options(int, HPHP::Variant const&)
_ZN4HPHP16f_assert_optionsEiRKNS_7VariantE

(return value) => rax
_rv => rdi
what => rsi
value => rdx
*/

TypedValue* fh_assert_options(TypedValue* _rv, int what, TypedValue* value) asm("_ZN4HPHP16f_assert_optionsEiRKNS_7VariantE");

/*
HPHP::Variant HPHP::f_assert(HPHP::Variant const&)
_ZN4HPHP8f_assertERKNS_7VariantE

(return value) => rax
_rv => rdi
assertion => rsi
*/

TypedValue* fh_assert(TypedValue* _rv, TypedValue* assertion) asm("_ZN4HPHP8f_assertERKNS_7VariantE");

/*
long long HPHP::f_dl(HPHP::String const&)
_ZN4HPHP4f_dlERKNS_6StringE

(return value) => rax
library => rdi
*/

long long fh_dl(Value* library) asm("_ZN4HPHP4f_dlERKNS_6StringE");

/*
bool HPHP::f_extension_loaded(HPHP::String const&)
_ZN4HPHP18f_extension_loadedERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_extension_loaded(Value* name) asm("_ZN4HPHP18f_extension_loadedERKNS_6StringE");

/*
HPHP::Array HPHP::f_get_loaded_extensions(bool)
_ZN4HPHP23f_get_loaded_extensionsEb

(return value) => rax
_rv => rdi
zend_extensions => rsi
*/

Value* fh_get_loaded_extensions(Value* _rv, bool zend_extensions) asm("_ZN4HPHP23f_get_loaded_extensionsEb");

/*
HPHP::Array HPHP::f_get_extension_funcs(HPHP::String const&)
_ZN4HPHP21f_get_extension_funcsERKNS_6StringE

(return value) => rax
_rv => rdi
module_name => rsi
*/

Value* fh_get_extension_funcs(Value* _rv, Value* module_name) asm("_ZN4HPHP21f_get_extension_funcsERKNS_6StringE");

/*
HPHP::String HPHP::f_get_cfg_var(HPHP::String const&)
_ZN4HPHP13f_get_cfg_varERKNS_6StringE

(return value) => rax
_rv => rdi
option => rsi
*/

Value* fh_get_cfg_var(Value* _rv, Value* option) asm("_ZN4HPHP13f_get_cfg_varERKNS_6StringE");

/*
HPHP::String HPHP::f_get_current_user()
_ZN4HPHP18f_get_current_userEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_current_user(Value* _rv) asm("_ZN4HPHP18f_get_current_userEv");

/*
HPHP::Array HPHP::f_get_defined_constants(HPHP::Variant const&)
_ZN4HPHP23f_get_defined_constantsERKNS_7VariantE

(return value) => rax
_rv => rdi
categorize => rsi
*/

Value* fh_get_defined_constants(Value* _rv, TypedValue* categorize) asm("_ZN4HPHP23f_get_defined_constantsERKNS_7VariantE");

/*
HPHP::String HPHP::f_get_include_path()
_ZN4HPHP18f_get_include_pathEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_include_path(Value* _rv) asm("_ZN4HPHP18f_get_include_pathEv");

/*
void HPHP::f_restore_include_path()
_ZN4HPHP22f_restore_include_pathEv

*/

void fh_restore_include_path() asm("_ZN4HPHP22f_restore_include_pathEv");

/*
HPHP::String HPHP::f_set_include_path(HPHP::String const&)
_ZN4HPHP18f_set_include_pathERKNS_6StringE

(return value) => rax
_rv => rdi
new_include_path => rsi
*/

Value* fh_set_include_path(Value* _rv, Value* new_include_path) asm("_ZN4HPHP18f_set_include_pathERKNS_6StringE");

/*
HPHP::Array HPHP::f_get_included_files()
_ZN4HPHP20f_get_included_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_included_files(Value* _rv) asm("_ZN4HPHP20f_get_included_filesEv");

/*
HPHP::Array HPHP::f_inclued_get_data()
_ZN4HPHP18f_inclued_get_dataEv

(return value) => rax
_rv => rdi
*/

Value* fh_inclued_get_data(Value* _rv) asm("_ZN4HPHP18f_inclued_get_dataEv");

/*
long long HPHP::f_get_magic_quotes_gpc()
_ZN4HPHP22f_get_magic_quotes_gpcEv

(return value) => rax
*/

long long fh_get_magic_quotes_gpc() asm("_ZN4HPHP22f_get_magic_quotes_gpcEv");

/*
long long HPHP::f_get_magic_quotes_runtime()
_ZN4HPHP26f_get_magic_quotes_runtimeEv

(return value) => rax
*/

long long fh_get_magic_quotes_runtime() asm("_ZN4HPHP26f_get_magic_quotes_runtimeEv");

/*
HPHP::Array HPHP::f_get_required_files()
_ZN4HPHP20f_get_required_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_required_files(Value* _rv) asm("_ZN4HPHP20f_get_required_filesEv");

/*
HPHP::Variant HPHP::f_getenv(HPHP::String const&)
_ZN4HPHP8f_getenvERKNS_6StringE

(return value) => rax
_rv => rdi
varname => rsi
*/

TypedValue* fh_getenv(TypedValue* _rv, Value* varname) asm("_ZN4HPHP8f_getenvERKNS_6StringE");

/*
long long HPHP::f_getlastmod()
_ZN4HPHP12f_getlastmodEv

(return value) => rax
*/

long long fh_getlastmod() asm("_ZN4HPHP12f_getlastmodEv");

/*
long long HPHP::f_getmygid()
_ZN4HPHP10f_getmygidEv

(return value) => rax
*/

long long fh_getmygid() asm("_ZN4HPHP10f_getmygidEv");

/*
long long HPHP::f_getmyinode()
_ZN4HPHP12f_getmyinodeEv

(return value) => rax
*/

long long fh_getmyinode() asm("_ZN4HPHP12f_getmyinodeEv");

/*
long long HPHP::f_getmypid()
_ZN4HPHP10f_getmypidEv

(return value) => rax
*/

long long fh_getmypid() asm("_ZN4HPHP10f_getmypidEv");

/*
long long HPHP::f_getmyuid()
_ZN4HPHP10f_getmyuidEv

(return value) => rax
*/

long long fh_getmyuid() asm("_ZN4HPHP10f_getmyuidEv");

/*
HPHP::Array HPHP::f_getopt(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_getoptERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
options => rsi
longopts => rdx
*/

Value* fh_getopt(Value* _rv, Value* options, TypedValue* longopts) asm("_ZN4HPHP8f_getoptERKNS_6StringERKNS_7VariantE");

/*
HPHP::Array HPHP::f_getrusage(int)
_ZN4HPHP11f_getrusageEi

(return value) => rax
_rv => rdi
who => rsi
*/

Value* fh_getrusage(Value* _rv, int who) asm("_ZN4HPHP11f_getrusageEi");

/*
bool HPHP::f_clock_getres(int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_clock_getresEiRKNS_14VRefParamValueES2_

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_getres(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP14f_clock_getresEiRKNS_14VRefParamValueES2_");

/*
bool HPHP::f_clock_gettime(int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP15f_clock_gettimeEiRKNS_14VRefParamValueES2_

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_gettime(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP15f_clock_gettimeEiRKNS_14VRefParamValueES2_");

/*
bool HPHP::f_clock_settime(int, long long, long long)
_ZN4HPHP15f_clock_settimeEixx

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_settime(int clk_id, long long sec, long long nsec) asm("_ZN4HPHP15f_clock_settimeEixx");

/*
HPHP::String HPHP::f_ini_alter(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_ini_alterERKNS_6StringES2_

(return value) => rax
_rv => rdi
varname => rsi
newvalue => rdx
*/

Value* fh_ini_alter(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP11f_ini_alterERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_ini_get_all(HPHP::String const&)
_ZN4HPHP13f_ini_get_allERKNS_6StringE

(return value) => rax
_rv => rdi
extension => rsi
*/

Value* fh_ini_get_all(Value* _rv, Value* extension) asm("_ZN4HPHP13f_ini_get_allERKNS_6StringE");

/*
HPHP::String HPHP::f_ini_get(HPHP::String const&)
_ZN4HPHP9f_ini_getERKNS_6StringE

(return value) => rax
_rv => rdi
varname => rsi
*/

Value* fh_ini_get(Value* _rv, Value* varname) asm("_ZN4HPHP9f_ini_getERKNS_6StringE");

/*
void HPHP::f_ini_restore(HPHP::String const&)
_ZN4HPHP13f_ini_restoreERKNS_6StringE

varname => rdi
*/

void fh_ini_restore(Value* varname) asm("_ZN4HPHP13f_ini_restoreERKNS_6StringE");

/*
HPHP::String HPHP::f_ini_set(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_ini_setERKNS_6StringES2_

(return value) => rax
_rv => rdi
varname => rsi
newvalue => rdx
*/

Value* fh_ini_set(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP9f_ini_setERKNS_6StringES2_");

/*
long long HPHP::f_memory_get_allocation()
_ZN4HPHP23f_memory_get_allocationEv

(return value) => rax
*/

long long fh_memory_get_allocation() asm("_ZN4HPHP23f_memory_get_allocationEv");

/*
long long HPHP::f_memory_get_peak_usage(bool)
_ZN4HPHP23f_memory_get_peak_usageEb

(return value) => rax
real_usage => rdi
*/

long long fh_memory_get_peak_usage(bool real_usage) asm("_ZN4HPHP23f_memory_get_peak_usageEb");

/*
long long HPHP::f_memory_get_usage(bool)
_ZN4HPHP18f_memory_get_usageEb

(return value) => rax
real_usage => rdi
*/

long long fh_memory_get_usage(bool real_usage) asm("_ZN4HPHP18f_memory_get_usageEb");

/*
HPHP::String HPHP::f_php_ini_scanned_files()
_ZN4HPHP23f_php_ini_scanned_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_ini_scanned_files(Value* _rv) asm("_ZN4HPHP23f_php_ini_scanned_filesEv");

/*
HPHP::String HPHP::f_php_logo_guid()
_ZN4HPHP15f_php_logo_guidEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_logo_guid(Value* _rv) asm("_ZN4HPHP15f_php_logo_guidEv");

/*
HPHP::String HPHP::f_php_sapi_name()
_ZN4HPHP15f_php_sapi_nameEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_sapi_name(Value* _rv) asm("_ZN4HPHP15f_php_sapi_nameEv");

/*
HPHP::String HPHP::f_php_uname(HPHP::String const&)
_ZN4HPHP11f_php_unameERKNS_6StringE

(return value) => rax
_rv => rdi
mode => rsi
*/

Value* fh_php_uname(Value* _rv, Value* mode) asm("_ZN4HPHP11f_php_unameERKNS_6StringE");

/*
bool HPHP::f_phpcredits(int)
_ZN4HPHP12f_phpcreditsEi

(return value) => rax
flag => rdi
*/

bool fh_phpcredits(int flag) asm("_ZN4HPHP12f_phpcreditsEi");

/*
bool HPHP::f_phpinfo(int)
_ZN4HPHP9f_phpinfoEi

(return value) => rax
what => rdi
*/

bool fh_phpinfo(int what) asm("_ZN4HPHP9f_phpinfoEi");

/*
HPHP::String HPHP::f_phpversion(HPHP::String const&)
_ZN4HPHP12f_phpversionERKNS_6StringE

(return value) => rax
_rv => rdi
extension => rsi
*/

Value* fh_phpversion(Value* _rv, Value* extension) asm("_ZN4HPHP12f_phpversionERKNS_6StringE");

/*
bool HPHP::f_putenv(HPHP::String const&)
_ZN4HPHP8f_putenvERKNS_6StringE

(return value) => rax
setting => rdi
*/

bool fh_putenv(Value* setting) asm("_ZN4HPHP8f_putenvERKNS_6StringE");

/*
bool HPHP::f_set_magic_quotes_runtime(bool)
_ZN4HPHP26f_set_magic_quotes_runtimeEb

(return value) => rax
new_setting => rdi
*/

bool fh_set_magic_quotes_runtime(bool new_setting) asm("_ZN4HPHP26f_set_magic_quotes_runtimeEb");

/*
void HPHP::f_set_time_limit(int)
_ZN4HPHP16f_set_time_limitEi

seconds => rdi
*/

void fh_set_time_limit(int seconds) asm("_ZN4HPHP16f_set_time_limitEi");

/*
HPHP::String HPHP::f_sys_get_temp_dir()
_ZN4HPHP18f_sys_get_temp_dirEv

(return value) => rax
_rv => rdi
*/

Value* fh_sys_get_temp_dir(Value* _rv) asm("_ZN4HPHP18f_sys_get_temp_dirEv");

/*
HPHP::Variant HPHP::f_version_compare(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_version_compareERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
version1 => rsi
version2 => rdx
sop => rcx
*/

TypedValue* fh_version_compare(TypedValue* _rv, Value* version1, Value* version2, Value* sop) asm("_ZN4HPHP17f_version_compareERKNS_6StringES2_S2_");

/*
bool HPHP::f_gc_enabled()
_ZN4HPHP12f_gc_enabledEv

(return value) => rax
*/

bool fh_gc_enabled() asm("_ZN4HPHP12f_gc_enabledEv");

/*
void HPHP::f_gc_enable()
_ZN4HPHP11f_gc_enableEv

*/

void fh_gc_enable() asm("_ZN4HPHP11f_gc_enableEv");

/*
void HPHP::f_gc_disable()
_ZN4HPHP12f_gc_disableEv

*/

void fh_gc_disable() asm("_ZN4HPHP12f_gc_disableEv");

/*
long long HPHP::f_gc_collect_cycles()
_ZN4HPHP19f_gc_collect_cyclesEv

(return value) => rax
*/

long long fh_gc_collect_cycles() asm("_ZN4HPHP19f_gc_collect_cyclesEv");

/*
HPHP::String HPHP::f_zend_logo_guid()
_ZN4HPHP16f_zend_logo_guidEv

(return value) => rax
_rv => rdi
*/

Value* fh_zend_logo_guid(Value* _rv) asm("_ZN4HPHP16f_zend_logo_guidEv");

/*
long long HPHP::f_zend_thread_id()
_ZN4HPHP16f_zend_thread_idEv

(return value) => rax
*/

long long fh_zend_thread_id() asm("_ZN4HPHP16f_zend_thread_idEv");

/*
HPHP::String HPHP::f_zend_version()
_ZN4HPHP14f_zend_versionEv

(return value) => rax
_rv => rdi
*/

Value* fh_zend_version(Value* _rv) asm("_ZN4HPHP14f_zend_versionEv");


} // !HPHP

