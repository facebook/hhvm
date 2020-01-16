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
function assert_options(int $what, $value = null);
<<__PHPStdLib>>
function assert($assertion);
<<__PHPStdLib>>
function dl(string $_library);
<<__PHPStdLib>>
function extension_loaded(string $name);
<<__PHPStdLib>>
function get_loaded_extensions(bool $zend_extensions = false);
<<__PHPStdLib>>
function get_extension_funcs(string $module_name);
<<__PHPStdLib>>
function get_cfg_var(string $_option);
<<__PHPStdLib>>
function get_current_user();
<<__PHPStdLib>>
function get_defined_constants(bool $categorize = false);
<<__PHPStdLib>>
function get_include_path();
<<__PHPStdLib>>
function restore_include_path();
<<__PHPStdLib>>
function set_include_path($new_include_path);
<<__PHPStdLib>>
function get_included_files();
<<__PHPStdLib>>
function get_required_files();
<<__PHPStdLib>>
function getenv(string $varname);
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
function getopt(string $options, $longopts = null);
<<__PHPStdLib>>
function getrusage(int $who = 0);
<<__PHPStdLib>>
function clock_getres(int $clk_id, inout $sec, inout $nsec);
<<__PHPStdLib>>
function clock_gettime(int $clk_id, inout $sec, inout $nsec);
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
function ini_alter(string $varname, $newvalue);
<<__PHPStdLib>>
function ini_get_all(string $extension = "");
<<__PHPStdLib>>
function ini_get(string $varname);
<<__PHPStdLib>>
function ini_restore(string $varname);
<<__PHPStdLib>>
function ini_set(string $varname, $newvalue);
<<__PHPStdLib>>
function memory_get_allocation();
<<__PHPStdLib>>
function memory_get_peak_usage(bool $real_usage = false);
<<__PHPStdLib>>
function memory_get_usage(bool $real_usage = false);
<<__PHPStdLib>>
function php_ini_scanned_files();
<<__PHPStdLib>>
function php_logo_guid();
<<__PHPStdLib>>
function php_sapi_name();
<<__PHPStdLib>>
function php_uname(string $mode = "");
<<__PHPStdLib>>
function phpcredits($flag = 0);
<<__PHPStdLib>>
function phpinfo(int $what = 0);
<<__PHPStdLib>>
function phpversion(string $extension = "");
<<__PHPStdLib>>
function putenv(string $setting);
<<__PHPStdLib>>
function set_time_limit(int $seconds);
function set_pre_timeout_handler(
  int $seconds, (function(HH\Awaitable<mixed>): void) $callback): void;
<<__PHPStdLib>>
function sys_get_temp_dir();
<<__PHPStdLib, __Rx>>
function version_compare(string $version1, string $version2, string $sop = "");
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
