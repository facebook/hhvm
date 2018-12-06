<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int CLOCK_MONOTONIC = 1;
const int CLOCK_PROCESS_CPUTIME_ID = 2;
const int CLOCK_REALTIME = 0;
const int CLOCK_THREAD_CPUTIME_ID = 3;

<<__PHPStdLib>>
function assert_options($what, $value = null);
<<__PHPStdLib>>
function assert($assertion);
<<__PHPStdLib>>
function dl($library);
<<__PHPStdLib>>
function extension_loaded($name);
<<__PHPStdLib>>
function get_loaded_extensions($zend_extensions = false);
<<__PHPStdLib>>
function get_extension_funcs($module_name);
<<__PHPStdLib>>
function get_cfg_var($option);
<<__PHPStdLib>>
function get_current_user();
<<__PHPStdLib>>
function get_defined_constants($categorize = null);
<<__PHPStdLib>>
function get_include_path();
<<__PHPStdLib>>
function restore_include_path();
<<__PHPStdLib>>
function set_include_path($new_include_path);
<<__PHPStdLib>>
function get_included_files();
<<__PHPStdLib>>
function get_magic_quotes_gpc();
<<__PHPStdLib>>
function get_magic_quotes_runtime();
<<__PHPStdLib>>
function get_required_files();
<<__PHPStdLib>>
function getenv($varname);
<<__PHPStdLib>>
function getlastmod();
<<__PHPStdLib>>
function getmygid();
<<__PHPStdLib>>
function getmyinode();
<<__PHPStdLib>>
function getmypid();
<<__PHPStdLib>>
function getmyuid();
<<__PHPStdLib>>
function getopt($options, $longopts = null);
<<__PHPStdLib>>
function getrusage($who = 0);
<<__PHPStdLib>>
function clock_getres($clk_id, &$sec, &$nsec);
<<__PHPStdLib>>
function clock_gettime($clk_id, &$sec, &$nsec);
<<__PHPStdLib>>
function clock_gettime_ns(int $clk_id): int;
<<__PHPStdLib>>
function cpu_get_count();
<<__PHPStdLib>>
function cpu_get_model();
<<__PHPStdLib>>
function hphp_memory_get_interval_peak_usage(bool $real_usage = false): int;
<<__PHPStdLib>>
function hphp_memory_start_interval(): bool;
<<__PHPStdLib>>
function hphp_memory_stop_interval(): bool;
<<__PHPStdLib>>
function ini_alter($varname, $newvalue);
<<__PHPStdLib>>
function ini_get_all($extension = null);
<<__PHPStdLib>>
function ini_get($varname);
<<__PHPStdLib>>
function ini_restore($varname);
<<__PHPStdLib>>
function ini_set($varname, $newvalue);
<<__PHPStdLib>>
function memory_get_allocation();
<<__PHPStdLib>>
function memory_get_peak_usage($real_usage = false);
<<__PHPStdLib>>
function memory_get_usage($real_usage = false);
<<__PHPStdLib>>
function php_ini_scanned_files();
<<__PHPStdLib>>
function php_logo_guid();
<<__PHPStdLib>>
function php_sapi_name();
<<__PHPStdLib>>
function php_uname($mode = null);
<<__PHPStdLib>>
function phpcredits($flag = 0);
<<__PHPStdLib>>
function phpinfo($what = 0);
<<__PHPStdLib>>
function phpversion($extension = null);
<<__PHPStdLib>>
function putenv($setting);
<<__PHPStdLib>>
function set_magic_quotes_runtime($new_setting);
<<__PHPStdLib>>
function set_time_limit($seconds);
<<__PHPStdLib>>
function sys_get_temp_dir();
<<__PHPStdLib, __Rx>>
function version_compare($version1, $version2, $sop = null);
<<__PHPStdLib>>
function gc_enabled();
<<__PHPStdLib>>
function gc_enable();
<<__PHPStdLib>>
function gc_disable();
<<__PHPStdLib>>
function gc_collect_cycles();
<<__PHPStdLib>>
function gc_mem_caches();
<<__PHPStdLib>>
function gc_check_heap();
<<__PHPStdLib>>
function zend_logo_guid();
<<__PHPStdLib>>
function zend_thread_id();
<<__PHPStdLib>>
function zend_version();
