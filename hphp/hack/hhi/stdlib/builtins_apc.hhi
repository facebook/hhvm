<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function apc_add(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  int $ttl = 0,
  int $bump_ttl = 0,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_add_with_pure_sleep(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  int $ttl = 0,
  int $bump_ttl = 0,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function apc_store(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  int $ttl = 0,
  int $bump_ttl = 0,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_store_with_pure_sleep(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  int $ttl = 0,
  int $bump_ttl = 0,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function apc_fetch(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  inout ?bool $success,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_fetch_with_pure_wakeup(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  inout ?bool $success,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function apc_delete(
  HH\FIXME\MISSING_PARAM_TYPE $key,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_cache_info(
  string $cache_type = "",
  bool $limited = false,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_clear_cache(string $cache_id = "")[globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_sma_info(bool $limited = false)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_inc(
  string $key,
  int $step,
  inout ?bool $success,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_dec(
  string $key,
  int $step,
  inout ?bool $success,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_cas(
  string $key,
  int $old_cas,
  int $new_cas,
)[globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_exists(
  HH\FIXME\MISSING_PARAM_TYPE $key,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apc_extend_ttl(string $key, int $ttl)[globals]: bool;
<<__PHPStdLib>>
function apc_size(string $key)[read_globals]: ?int;

// The following are php std lib functions not supported by HHVM:
//
//function apc_compile_file($filename, $atomic = true, $cache_id = 0) { }
//function apc_define_constants($key, $constants, $case_sensitive = true, $cache_id = 0) { }
//function apc_load_constants($key, $case_sensitive = true, $cache_id = 0) { }
//function apc_filehits() { }
//function apc_delete_file($keys, $cache_id = 0) { }
//function apc_bin_dump($cache_id = 0, $filter = null) { }
//function apc_bin_load($data, $flags = 0, $cache_id = 0) { }
//function apc_bin_dumpfile($cache_id, $filter, $filename, $flags = 0, $context = null) { }
//function apc_bin_loadfile($filename, $context = null, $flags = 0, $cache_id = 0) { }
