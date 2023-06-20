<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int CLOCK_MONOTONIC;
const int CLOCK_PROCESS_CPUTIME_ID;
const int CLOCK_REALTIME;
const int CLOCK_THREAD_CPUTIME_ID;

<<__PHPStdLib>>
function dl(string $_library): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function extension_loaded(
  string $name,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_loaded_extensions(
  bool $zend_extensions = false,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_extension_funcs(
  string $module_name,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_cfg_var(string $_option): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_current_user(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_defined_constants(
  bool $categorize = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_include_path(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function restore_include_path(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function set_include_path(
  HH\FIXME\MISSING_PARAM_TYPE $new_include_path,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_included_files(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_required_files(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getenv(string $varname)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getlastmod(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getmygid(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getmyinode(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getmypid(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getmyuid(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getopt(
  string $options,
  HH\FIXME\MISSING_PARAM_TYPE $longopts = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getopt_with_optind(
  string $options,
  ?vec<string> $longopts,
  inout int $optind,
): dict<string, mixed>;
<<__PHPStdLib>>
function getrusage(int $who = 0): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clock_getres(
  int $clk_id,
  inout HH\FIXME\MISSING_PARAM_TYPE $sec,
  inout HH\FIXME\MISSING_PARAM_TYPE $nsec,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clock_gettime(
  int $clk_id,
  inout HH\FIXME\MISSING_PARAM_TYPE $sec,
  inout HH\FIXME\MISSING_PARAM_TYPE $nsec,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function clock_gettime_ns(int $clk_id)[leak_safe]: int;
<<__PHPStdLib>>
function cpu_get_count()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function cpu_get_model()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_memory_get_interval_peak_usage(bool $real_usage = false): int;
<<__PHPStdLib>>
function hphp_memory_start_interval(): bool;
<<__PHPStdLib>>
function hphp_memory_stop_interval(): bool;
<<__PHPStdLib>>
function ini_alter(
  string $varname,
  HH\FIXME\MISSING_PARAM_TYPE $newvalue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ini_get_all(string $extension = ""): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ini_get(string $varname)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ini_restore(string $varname): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ini_set(
  string $varname,
  HH\FIXME\MISSING_PARAM_TYPE $newvalue,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function memory_get_allocation()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function memory_get_peak_usage(
  bool $real_usage = false,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function memory_get_usage(
  bool $real_usage = false,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function php_ini_scanned_files(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function php_logo_guid(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function php_sapi_name()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function php_uname(string $mode = ""): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function phpcredits(
  HH\FIXME\MISSING_PARAM_TYPE $flag = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function phpinfo(int $what = 0): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function phpversion(string $extension = ""): mixed;
<<__PHPStdLib>>
function putenv(string $setting): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function set_time_limit(int $seconds): HH\FIXME\MISSING_RETURN_TYPE;
function set_pre_timeout_handler(
  int $seconds,
  (function(HH\Awaitable<mixed>): void) $callback,
): void;
<<__PHPStdLib>>
function sys_get_temp_dir(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function version_compare(
  string $version1,
  string $version2,
  string $sop = "",
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_enabled(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_enable(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_disable(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_collect_cycles()[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_mem_caches(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gc_check_heap(): HH\FIXME\MISSING_RETURN_TYPE;
function get_visited_files(): keyset<string>;
function record_visited_files(): void;
