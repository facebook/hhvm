<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function memcache_connect($host, $port = 0, $timeout = 0, $timeoutms = 0) { }
<<__PHPStdLib>>
function memcache_pconnect($host, $port = 0, $timeout = 0, $timeoutms = 0) { }
<<__PHPStdLib>>
function memcache_add($memcache, $key, $var, $flag = 0, $expire = 0) { }
<<__PHPStdLib>>
function memcache_set($memcache, $key, $var, $flag = 0, $expire = 0) { }
<<__PHPStdLib>>
function memcache_replace($memcache, $key, $var, $flag = 0, $expire = 0) { }
<<__PHPStdLib>>
function memcache_get($memcache, $key) { }
<<__PHPStdLib>>
function memcache_delete($memcache, $key, $expire = 0) { }
<<__PHPStdLib>>
function memcache_increment($memcache, $key, $offset = 1) { }
<<__PHPStdLib>>
function memcache_decrement($memcache, $key, $offset = 1) { }
<<__PHPStdLib>>
function memcache_close($memcache) { }
<<__PHPStdLib>>
function memcache_debug($onoff) { }
<<__PHPStdLib>>
function memcache_get_version($memcache) { }
<<__PHPStdLib>>
function memcache_flush($memcache, $timestamp = 0) { }
<<__PHPStdLib>>
function memcache_setoptimeout($memcache, int $timeoutms) { }
<<__PHPStdLib>>
function memcache_get_server_status($memcache, string $host, int $port = 0) { }
<<__PHPStdLib>>
function memcache_set_compress_threshold($memcache, $threshold, $min_savings = 0.2) { }
<<__PHPStdLib>>
function memcache_get_stats($memcache, $type = null, $slabid = 0, $limit = 100) { }
<<__PHPStdLib>>
function memcache_get_extended_stats($memcache, $type = null, $slabid = 0, $limit = 100) { }
<<__PHPStdLib>>
function memcache_set_server_params($memcache, $host, $port = 11211, $timeout = 0, $retry_interval = 0, $status = true, $failure_callback = null) { }
<<__PHPStdLib>>
function memcache_add_server($memcache, $host, $port = 11211, $persistent = false, $weight = 0, $timeout = 0, $retry_interval = 0, $status = true, $failure_callback = null, $timeoutms = 0) { }
class Memcache {
  public function __construct();
  public function connect(string $host, int $port = 0, int $timeout = 0, int $timeoutms = 0);
  public function pconnect($host, $port = 0, $timeout = 0, $timeoutms = 0);
  public function add(string $key, $var, int $flag = 0, int $expire = 0);
  public function set(string $key, $var, int $flag = 0, int $expire = 0);
  public function replace(string $key, $var, int $flag = 0, int $expire = 0);
  public function get($key);
  public function delete(string $key, int $expire = 0);
  public function increment(string $key, int $offset = 1);
  public function decrement(string $key, int $offset = 1);
  public function getversion();
  public function flush(int $expire = 0);
  public function setoptimeout($timeoutms);
  public function close();
  public function getserverstatus(string $host, int $port = 0);
  public function setcompressthreshold(int $threshold, float $min_savings = 0.2);
  public function getstats(string $type = "", int $slabid = 0, int $limit = 100);
  public function getextendedstats(string $type = "", int $slabid = 0, int $limit = 100);
  public function setserverparams(string $host, int $port = 11211, int $timeout = 0, int $retry_interval = 0, bool $status = true, $failure_callback = null);
  public function addserver(string $host, int $port = 11211, bool $persistent = false, int $weight = 0, int $timeout = 0, int $retry_interval = 0, bool $status = true, $failure_callback = null, int $timeoutms = 0);
}
