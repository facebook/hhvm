<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
class Memcached {
  const OPT_COMPRESSION = 0;
  const OPT_SERIALIZER = 0;
  const SERIALIZER_PHP = 0;
  const SERIALIZER_IGBINARY = 0;
  const SERIALIZER_JSON = 0;
  const OPT_PREFIX_KEY = 0;
  const OPT_HASH = 0;
  const HASH_DEFAULT = 0;
  const HASH_MD5 = 0;
  const HASH_CRC = 0;
  const HASH_FNV1_64 = 0;
  const HASH_FNV1A_64 = 0;
  const HASH_FNV1_32 = 0;
  const HASH_FNV1A_32 = 0;
  const HASH_HSIEH = 0;
  const HASH_MURMUR = 0;
  const OPT_DISTRIBUTION = 0;
  const DISTRIBUTION_MODULA = 0;
  const DISTRIBUTION_CONSISTENT = 0;
  const OPT_LIBKETAMA_COMPATIBLE = 0;
  const OPT_BUFFER_WRITES = 0;
  const OPT_BINARY_PROTOCOL = 0;
  const OPT_NO_BLOCK = 0;
  const OPT_TCP_NODELAY = 0;
  const OPT_SOCKET_SEND_SIZE = 0;
  const OPT_SOCKET_RECV_SIZE = 0;
  const OPT_CONNECT_TIMEOUT = 0;
  const OPT_RETRY_TIMEOUT = 0;
  const OPT_SEND_TIMEOUT = 0;
  const OPT_RECV_TIMEOUT = 0;
  const OPT_POLL_TIMEOUT = 0;
  const OPT_CACHE_LOOKUPS = 0;
  const OPT_SERVER_FAILURE_LIMIT = 0;
  const HAVE_IGBINARY = 0;
  const HAVE_JSON = 0;
  const GET_PRESERVE_ORDER = 0;
  const RES_SUCCESS = 0;
  const RES_FAILURE = 0;
  const RES_HOST_LOOKUP_FAILURE = 0;
  const RES_UNKNOWN_READ_FAILURE = 0;
  const RES_PROTOCOL_ERROR = 0;
  const RES_CLIENT_ERROR = 0;
  const RES_SERVER_ERROR = 0;
  const RES_WRITE_FAILURE = 0;
  const RES_DATA_EXISTS = 0;
  const RES_NOTSTORED = 0;
  const RES_NOTFOUND = 0;
  const RES_PARTIAL_READ = 0;
  const RES_SOME_ERRORS = 0;
  const RES_NO_SERVERS = 0;
  const RES_END = 0;
  const RES_ERRNO = 0;
  const RES_BUFFERED = 0;
  const RES_TIMEOUT = 0;
  const RES_BAD_KEY_PROVIDED = 0;
  const RES_CONNECTION_SOCKET_CREATE_FAILURE = 0;
  const RES_PAYLOAD_FAILURE = 0;
  public function __construct($persistent_id = null) { }
  public function add($key, $value, $expiration = 0) { }
  public function addByKey($server_key, $key, $value, $expiration = 0) { }
  public function addServer($host, $port, $weight = 0) { }
  public function addServers($servers) { }
  public function append($key, $value) { }
  public function appendByKey($server_key, $key, $value) { }
  public function cas($cas_token, $key, $value, $expiration = 0) { }
  public function casByKey($cas_token, $server_key, $key, $value, $expiration = 0) { }
  public function decrement($key, $offset = 1) { }
  public function delete($key, $time = 0) { }
  public function deleteByKey($server_key, $key, $time = 0) { }
  public function fetch() { }
  public function fetchAll() { }
  public function flush($delay = 0) { }
  public function get($key, $cache_cb = null_variant, &$cas_token = null_variant) { }
  public function getByKey($server_key, $key, $cache_cb = null_variant, &$cas_token = null_variant) { }
  public function getDelayed($keys, $with_cas = false, $value_cb = null_variant) { }
  public function getDelayedByKey($server_key, $keys, $with_cas = false, $value_cb = null_variant) { }
  public function getMulti($keys, &$cas_tokens = null_variant, $flags = 0) { }
  public function getMultiByKey($server_key, $keys, &$cas_tokens = null_variant, $flags = 0) { }
  public function getOption($option) { }
  public function getResultCode() { }
  public function getResultMessage() { }
  public function getServerByKey($server_key) { }
  public function getServerList() { }
  public function getStats() { }
  public function getVersion() { }
  public function increment($key, $offset = 1) { }
  public function prepend($key, $value) { }
  public function prependByKey($server_key, $key, $value) { }
  public function replace($key, $value, $expiration = 0) { }
  public function replaceByKey($server_key, $key, $value, $expiration = 0) { }
  public function set($key, $value, $expiration = 0) { }
  public function setByKey($server_key, $key, $value, $expiration = 0) { }
  public function setMulti($items, $expiration = 0) { }
  public function setMultiByKey($server_key, $items, $expiration = 0) { }
  public function setOption($option, $value) { }
}
