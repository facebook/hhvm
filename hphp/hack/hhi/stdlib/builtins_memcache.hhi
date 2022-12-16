<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function memcache_connect(
  HH\FIXME\MISSING_PARAM_TYPE $host,
  HH\FIXME\MISSING_PARAM_TYPE $port = 0,
  HH\FIXME\MISSING_PARAM_TYPE $timeout = 0,
  HH\FIXME\MISSING_PARAM_TYPE $timeoutms = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_pconnect(
  HH\FIXME\MISSING_PARAM_TYPE $host,
  HH\FIXME\MISSING_PARAM_TYPE $port = 0,
  HH\FIXME\MISSING_PARAM_TYPE $timeout = 0,
  HH\FIXME\MISSING_PARAM_TYPE $timeoutms = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_add(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  HH\FIXME\MISSING_PARAM_TYPE $flag = 0,
  HH\FIXME\MISSING_PARAM_TYPE $expire = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_set(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  HH\FIXME\MISSING_PARAM_TYPE $flag = 0,
  HH\FIXME\MISSING_PARAM_TYPE $expire = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_replace(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  HH\FIXME\MISSING_PARAM_TYPE $flag = 0,
  HH\FIXME\MISSING_PARAM_TYPE $expire = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_get(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_delete(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $expire = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_increment(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $offset = 1,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_decrement(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $offset = 1,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_close(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_debug(
  HH\FIXME\MISSING_PARAM_TYPE $onoff,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_get_version(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_flush(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $timestamp = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_setoptimeout(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  int $timeoutms,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_get_server_status(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  string $host,
  int $port = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_set_compress_threshold(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $threshold,
  HH\FIXME\MISSING_PARAM_TYPE $min_savings = 0.2,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_get_stats(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
  HH\FIXME\MISSING_PARAM_TYPE $slabid = 0,
  HH\FIXME\MISSING_PARAM_TYPE $limit = 100,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_get_extended_stats(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
  HH\FIXME\MISSING_PARAM_TYPE $slabid = 0,
  HH\FIXME\MISSING_PARAM_TYPE $limit = 100,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_set_server_params(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $host,
  HH\FIXME\MISSING_PARAM_TYPE $port = 11211,
  HH\FIXME\MISSING_PARAM_TYPE $timeout = 0,
  HH\FIXME\MISSING_PARAM_TYPE $retry_interval = 0,
  HH\FIXME\MISSING_PARAM_TYPE $status = true,
  HH\FIXME\MISSING_PARAM_TYPE $failure_callback = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function memcache_add_server(
  HH\FIXME\MISSING_PARAM_TYPE $memcache,
  HH\FIXME\MISSING_PARAM_TYPE $host,
  HH\FIXME\MISSING_PARAM_TYPE $port = 11211,
  HH\FIXME\MISSING_PARAM_TYPE $persistent = false,
  HH\FIXME\MISSING_PARAM_TYPE $weight = 0,
  HH\FIXME\MISSING_PARAM_TYPE $timeout = 0,
  HH\FIXME\MISSING_PARAM_TYPE $retry_interval = 0,
  HH\FIXME\MISSING_PARAM_TYPE $status = true,
  HH\FIXME\MISSING_PARAM_TYPE $failure_callback = null,
  HH\FIXME\MISSING_PARAM_TYPE $timeoutms = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
class Memcache {
  public function __construct();
  public function connect(
    string $host,
    int $port = 0,
    int $timeout = 0,
    int $timeoutms = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function pconnect(
    HH\FIXME\MISSING_PARAM_TYPE $host,
    HH\FIXME\MISSING_PARAM_TYPE $port = 0,
    HH\FIXME\MISSING_PARAM_TYPE $timeout = 0,
    HH\FIXME\MISSING_PARAM_TYPE $timeoutms = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function add(
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $var,
    int $flag = 0,
    int $expire = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function set(
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $var,
    int $flag = 0,
    int $expire = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function replace(
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $var,
    int $flag = 0,
    int $expire = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function get(
    HH\FIXME\MISSING_PARAM_TYPE $key,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function delete(
    string $key,
    int $expire = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function increment(
    string $key,
    int $offset = 1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function decrement(
    string $key,
    int $offset = 1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getversion(): HH\FIXME\MISSING_RETURN_TYPE;
  public function flush(int $expire = 0): HH\FIXME\MISSING_RETURN_TYPE;
  public function setoptimeout(
    HH\FIXME\MISSING_PARAM_TYPE $timeoutms,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function close(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getserverstatus(
    string $host,
    int $port = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setcompressthreshold(
    int $threshold,
    float $min_savings = 0.2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getstats(
    string $type = "",
    int $slabid = 0,
    int $limit = 100,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getextendedstats(
    string $type = "",
    int $slabid = 0,
    int $limit = 100,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setserverparams(
    string $host,
    int $port = 11211,
    int $timeout = 0,
    int $retry_interval = 0,
    bool $status = true,
    HH\FIXME\MISSING_PARAM_TYPE $failure_callback = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addserver(
    string $host,
    int $port = 11211,
    bool $persistent = false,
    int $weight = 0,
    int $timeout = 0,
    int $retry_interval = 0,
    bool $status = true,
    HH\FIXME\MISSING_PARAM_TYPE $failure_callback = null,
    int $timeoutms = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}
