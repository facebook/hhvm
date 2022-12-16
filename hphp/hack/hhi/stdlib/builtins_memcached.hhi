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
class Memcached {
  const int OPT_COMPRESSION;
  const int OPT_SERIALIZER;
  const int SERIALIZER_PHP;
  const int SERIALIZER_IGBINARY;
  const int SERIALIZER_JSON;
  const int OPT_PREFIX_KEY;
  const int OPT_HASH;
  const int HASH_DEFAULT;
  const int HASH_MD5;
  const int HASH_CRC;
  const int HASH_FNV1_64;
  const int HASH_FNV1A_64;
  const int HASH_FNV1_32;
  const int HASH_FNV1A_32;
  const int HASH_HSIEH;
  const int HASH_MURMUR;
  const int OPT_DISTRIBUTION;
  const int DISTRIBUTION_MODULA;
  const int DISTRIBUTION_CONSISTENT;
  const int OPT_LIBKETAMA_COMPATIBLE;
  const int OPT_LIBKETAMA_HASH;
  const bool GET_ERROR_RETURN_VALUE;
  const int LIBMEMCACHED_VERSION_HEX;
  const int OPT_BUFFER_WRITES;
  const int OPT_BINARY_PROTOCOL;
  const int OPT_NO_BLOCK;
  const int OPT_TCP_NODELAY;
  const int OPT_SOCKET_SEND_SIZE;
  const int OPT_SOCKET_RECV_SIZE;
  const int OPT_CONNECT_TIMEOUT;
  const int OPT_RETRY_TIMEOUT;
  const int OPT_SEND_TIMEOUT;
  const int OPT_RECV_TIMEOUT;
  const int OPT_POLL_TIMEOUT;
  const int OPT_CACHE_LOOKUPS;
  const int OPT_SERVER_FAILURE_LIMIT;
  const int HAVE_IGBINARY;
  const int HAVE_JSON;
  const int GET_PRESERVE_ORDER;
  const int RES_SUCCESS;
  const int RES_FAILURE;
  const int RES_HOST_LOOKUP_FAILURE;
  const int RES_UNKNOWN_READ_FAILURE;
  const int RES_PROTOCOL_ERROR;
  const int RES_CLIENT_ERROR;
  const int RES_SERVER_ERROR;
  const int RES_WRITE_FAILURE;
  const int RES_DATA_EXISTS;
  const int RES_NOTSTORED;
  const int RES_NOTFOUND;
  const int RES_PARTIAL_READ;
  const int RES_SOME_ERRORS;
  const int RES_NO_SERVERS;
  const int RES_END;
  const int RES_ERRNO;
  const int RES_BUFFERED;
  const int RES_TIMEOUT;
  const int RES_BAD_KEY_PROVIDED;
  const int RES_CONNECTION_SOCKET_CREATE_FAILURE;
  const int RES_PAYLOAD_FAILURE;

  public function __construct(
    HH\FIXME\MISSING_PARAM_TYPE $persistent_id = null,
  );
  public function add(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addByKey(
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addServer(
    string $host,
    int $port,
    int $weight = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addServers(
    HH\FIXME\MISSING_PARAM_TYPE $servers,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function append(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function appendByKey(
    string $server_key,
    string $key,
    string $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function cas(
    float $cas_token,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function casByKey(
    float $cas_token,
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function decrement(
    string $key,
    int $offset = 1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function delete(
    string $key,
    int $time = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function deleteByKey(
    string $server_key,
    string $key,
    int $time = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function deleteMulti(varray<string> $keys, int $time = 0): mixed;
  public function deleteMultiByKey(
    string $server_key,
    varray<string> $keys,
    int $time = 0,
  ): mixed;
  public function fetch(): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchAll(): HH\FIXME\MISSING_RETURN_TYPE;
  public function flush(int $delay = 0): HH\FIXME\MISSING_RETURN_TYPE;
  public function get(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $cache_cb = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getByKey(
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $cache_cb = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getWithCasToken(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $cache_cb,
    inout $cas_token,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getByKeyWithCasToken(
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $cache_cb,
    inout $cas_token,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDelayed(
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    HH\FIXME\MISSING_PARAM_TYPE $with_cas = false,
    HH\FIXME\MISSING_PARAM_TYPE $value_cb = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDelayedByKey(
    string $server_key,
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    bool $with_cas = false,
    HH\FIXME\MISSING_PARAM_TYPE $value_cb = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMulti(
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    int $flags = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMultiByKey(
    string $server_key,
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    int $flags = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMultiWithCasTokens(
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    inout $cas_tokens,
    int $flags = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMultiByKeyWithCasTokens(
    string $server_key,
    HH\FIXME\MISSING_PARAM_TYPE $keys,
    inout $cas_tokens,
    int $flags = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getOption(int $option): HH\FIXME\MISSING_RETURN_TYPE;
  public function getResultCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getResultMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getServerByKey(
    string $server_key,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getServerList(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getStats(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getVersion(): HH\FIXME\MISSING_RETURN_TYPE;
  public function increment(
    string $key,
    int $offset = 1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function prepend(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function prependByKey(
    string $server_key,
    string $key,
    string $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function replace(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function replaceByKey(
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function set(
    HH\FIXME\MISSING_PARAM_TYPE $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setByKey(
    string $server_key,
    string $key,
    HH\FIXME\MISSING_PARAM_TYPE $value,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setMulti(
    HH\FIXME\MISSING_PARAM_TYPE $items,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setMultiByKey(
    string $server_key,
    HH\FIXME\MISSING_PARAM_TYPE $items,
    int $expiration = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setOption(
    int $option,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function touch(string $key, int $expiration = 0): bool;
  public function touchByKey(
    string $server_key,
    string $key,
    int $expiration = 0,
  ): bool;
}
