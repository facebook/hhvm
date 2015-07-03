<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the 'hack' directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Redis {
  const int REDIS_NOT_FOUND = 0;
  const int REDIS_STRING = 1;
  const int REDIS_SET = 2;
  const int REDIS_LIST = 3;
  const int REDIS_ZSET = 4;
  const int REDIS_HASH = 5;
  const int ATOMIC = 0;
  const int MULTI = 1;
  const int PIPELINE = 2;
  const int OPT_SERIALIZER = 1;
  const int OPT_PREFIX = 2;
  const int OPT_READ_TIMEOUT = 3;
  const int SERIALIZER_NONE = 0;
  const int SERIALIZER_PHP = 1;
  const int OPT_SCAN = 4;
  const int SCAN_RETRY = 1;
  const int SCAN_NORETRY = 0;
  const string AFTER = 'after';
  const string BEFORE = 'before';
  public function __construct() {}
  public function connect($host, $port = 6379, $timeout = 0.0) {}
  public function psetex($key, $ttl, $value) {}
  public function sScan($key, $iterator, $pattern = '', $count = 0) {}
  public function scan($iterator, $pattern = '', $count = 0) {}
  public function zScan($key, $iterator, $pattern = '', $count = 0) {}
  public function hScan($key, $iterator, $pattern = '', $count = 0) {}
  public function client($command, $arg = '') {}
  public function slowlog($command) {}
  public function open($host, $port = 6379, $timeout = 0.0) {}
  public function pconnect($host, $port = 6379, $timeout = 0.0) {}
  public function popen($host, $port = 6379, $timeout = 0.0) {}
  public function close() {}
  public function setOption($name, $value) {}
  public function getOption($name) {}
  public function ping() {}
  public function get($key) {}
  public function set($key, $value, $timeout = 0) {}
  public function setex($key, $ttl, $value) {}
  public function setnx($key, $value) {}
  public function del($key1, $key2 = null, $key3 = null) {}
  public function delete($key1, $key2 = null, $key3 = null) {}
  public function multi() {}
  public function exec() {}
  public function discard() {}
  public function watch($key) {}
  public function unwatch() {}
  public function subscribe($channels, $callback) {}
  public function psubscribe($patterns, $callback) {}
  public function publish($channel, $message) {}
  public function exists($key) {}
  public function incr($key) {}
  public function incrByFloat($key, $increment) {}
  public function incrBy($key, $value) {}
  public function decr($key) {}
  public function decrBy($key, $value) {}
  public function getMultiple(array $keys) {}
  public function lPush($key, $value1, $value2 = null, $valueN = null) {}
  public function rPush($key, $value1, $value2 = null, $valueN = null) {}
  public function lPushx($key, $value) {}
  public function rPushx($key, $value) {}
  public function lPop($key) {}
  public function rPop($key) {}
  public function blPop(array $keys) {}
  public function brPop(array $keys) {}
  public function lLen($key) {}
  public function lSize($key) {}
  public function lIndex($key, $index) {}
  public function lGet($key, $index) {}
  public function lSet($key, $index, $value) {}
  public function lRange($key, $start, $end) {}
  public function lGetRange($key, $start, $end) {}
  public function lTrim($key, $start, $stop) {}
  public function listTrim($key, $start, $stop) {}
  public function lRem($key, $value, $count) {}
  public function lRemove($key, $value, $count) {}
  public function lInsert($key, $position, $pivot, $value) {}
  public function sAdd($key, $value1, $value2 = null, $valueN = null) {}
  public function sRem($key, $member1, $member2 = null, $memberN = null) {}
  public function sRemove($key, $member1, $member2 = null, $memberN = null) {}
  public function sMove($srcKey, $dstKey, $member) {}
  public function sIsMember($key, $value) {}
  public function sContains($key, $value) {}
  public function sCard($key) {}
  public function sPop($key) {}
  public function sRandMember($key) {}
  public function sInter($key1, $key2, $keyN = null) {}
  public function sInterStore($dstKey, $key1, $key2, $keyN = null) {}
  public function sUnion($key1, $key2, $keyN = null) {}
  public function sUnionStore($dstKey, $key1, $key2, $keyN = null) {}
  public function sDiff($key1, $key2, $keyN = null) {}
  public function sDiffStore($dstKey, $key1, $key2, $keyN = null) {}
  public function sMembers($key) {}
  public function sGetMembers($key) {}
  public function getSet($key, $value) {}
  public function randomKey() {}
  public function select($dbindex) {}
  public function move($key, $dbindex) {}
  public function rename($srcKey, $dstKey) {}
  public function renameKey($srcKey, $dstKey) {}
  public function renameNx($srcKey, $dstKey) {}
  public function expire($key, $ttl) {}
  public function pExpire($key, $ttl) {}
  public function setTimeout($key, $ttl) {}
  public function expireAt($key, $timestamp) {}
  public function pExpireAt($key, $timestamp) {}
  public function keys($pattern) {}
  public function getKeys($pattern) {}
  public function dbSize() {}
  public function auth($password) {}
  public function bgrewriteaof() {}
  public function slaveof($host = '127.0.0.1', $port = 6379) {}
  public function object($string = '', $key = '') {}
  public function save() {}
  public function bgsave() {}
  public function lastSave() {}
  public function type($key) {}
  public function append($key, $value) {}
  public function getRange($key, $start, $end) {}
  public function substr($key, $start, $end) {}
  public function setRange($key, $offset, $value) {}
  public function strlen($key) {}
  public function getBit($key, $offset) {}
  public function setBit($key, $offset, $value) {}
  public function bitCount($key) {}
  public function bitOp($operation, $retKey, $key1, $key2, $key3 = null) {}
  public function flushDB() {}
  public function flushAll() {}
  public function sort($key, $option = null) {}
  public function info($option = null) {}
  public function resetStat() {}
  public function ttl($key) {}
  public function pttl($key) {}
  public function persist($key) {}
  public function mset(array $array) {}
  public function mget(array $array) {}
  public function msetnx(array $array) {}
  public function rpoplpush($srcKey, $dstKey) {}
  public function brpoplpush($srcKey, $dstKey, $timeout) {}
  public function zAdd($key, $score1, $value1, $score2 = null, $value2 = null, $scoreN = null, $valueN = null) {}
  public function zRange($key, $start, $end, $withscores = null) {}
  public function zRem($key, $member1, $member2 = null, $memberN = null) {}
  public function zDelete($key, $member1, $member2 = null, $memberN = null) {}
  public function zRevRange($key, $start, $end, $withscore = null) {}
  public function zRangeByScore($key, $start, $end, array $options = array()) {}
  public function zRevRangeByScore($key, $start, $end, array $options = array()) {}
  public function zCount($key, $start, $end) {}
  public function zRemRangeByScore($key, $start, $end) {}
  public function zDeleteRangeByScore($key, $start, $end) {}
  public function zRemRangeByRank($key, $start, $end) {}
  public function zDeleteRangeByRank($key, $start, $end) {}
  public function zCard($key) {}
  public function zSize($key) {}
  public function zScore($key, $member) {}
  public function zRank($key, $member) {}
  public function zRevRank($key, $member) {}
  public function zIncrBy($key, $value, $member) {}
  public function zUnion($Output, $ZSetKeys, array $Weights = [], $aggregateFunction = 'SUM') {}
  public function zInter($Output, $ZSetKeys, array $Weights = [], $aggregateFunction = 'SUM') {}
  public function hSet($key, $hashKey, $value) {}
  public function hSetNx($key, $hashKey, $value) {}
  public function hGet($key, $hashKey) {}
  public function hLen($key) {}
  public function hDel($key, $hashKey1, $hashKey2 = null, $hashKeyN = null) {}
  public function hKeys($key) {}
  public function hVals($key) {}
  public function hGetAll($key) {}
  public function hExists($key, $hashKey) {}
  public function hIncrBy($key, $hashKey, $value) {}
  public function hIncrByFloat($key, $field, $increment) {}
  public function hMset($key, $hashKeys) {}
  public function hMGet($key, $hashKeys) {}
  public function config($operation, $key, $value) {}
  public function evaluate($script, $args = array(), $numKeys = 0) {}
  public function evalSha($scriptSha, $args = array(), $numKeys = 0) {}
  public function evaluateSha($scriptSha, $args = array(), $numKeys = 0) {}
  public function script($command, $script) {}
  public function getLastError() {}
  public function clearLastError() {}
  public function _prefix($value) {}
  public function _unserialize($value) {}
  public function dump($key) {}
  public function restore($key, $ttl, $value) {}
  public function migrate($host, $port, $key, $db, $timeout) {}
  public function time() {}
}

class RedisException extends RuntimeException {
}

class RedisArray {
  public function __call($function_name, $arguments) {}
  public function __construct($name = '', array $hosts = [], array $opts = []) {}
  public function _distributor() {}
  public function _function() {}
  public function _hosts() {}
  public function _instance() {}
  public function _rehash() {}
  public function _target() {}
  public function bgsave() {}
  public function del() {}
  public function delete() {}
  public function discard() {}
  public function exec() {}
  public function flushall() {}
  public function flushdb() {}
  public function getMultiple() {}
  public function getOption() {}
  public function info() {}
  public function keys() {}
  public function mget() {}
  public function mset() {}
  public function multi() {}
  public function ping() {}
  public function save() {}
  public function select() {}
  public function setOption() {}
  public function unwatch() {}
}
