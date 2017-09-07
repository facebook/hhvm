<?hh // decl /* -*- php -*- */
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
  const OPT_LIBKETAMA_HASH = 0;
  const bool GET_ERROR_RETURN_VALUE = false;
  const int LIBMEMCACHED_VERSION_HEX = 0;
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
  const HAVE_SESSION = true;

  public function __construct(?string $persistent_id = null): void;
  public function add(mixed $key, mixed $value, int $expiration = 0): bool;
  public function addByKey(string $server_key, string $key, mixed $value, int $expiration = 0): bool;
  public function addServer(string $host, int $port, int $weight = 0): bool;
  public function addServers(array<array<mixed>> $servers): bool;
  public function append(mixed $key, mixed $value): bool;
  public function appendByKey(string $server_key, string $key, string $value): bool;
  public function cas(float $cas_token, string $key, mixed $value, int $expiration = 0): bool;
  public function casByKey(float $cas_token, string $server_key, string $key, mixed $value, int $expiration = 0): bool;
  public function decrement(string $key, int $offset = 1, mixed $initial_value = false, int $expiry = 0): mixed;
  public function decrementByKey(string $server_key, string $key, int $offset = 1, mixed $initial_value = false, int $expiry = 0): mixed;
  public function delete(mixed $key, int $time = 0): bool;
  public function deleteMultiByKey(string $server_key, array $keys, int $time = 0): mixed;
  public function deleteMulti(array $keys, int $time = 0): mixed;
  public function deleteByKey(string $server_key, string $key, int $time = 0): bool;

  public function fetch(): mixed;
  public function fetchAll(): mixed;
  public function flush(int $delay = 0): bool;
  public function get(mixed $key, mixed $cache_cb = null, mixed &$cas_token = null): mixed;
  public function getAllKeys(): mixed;
  public function getByKey(string $server_key, string $key, mixed $cache_cb = null, mixed &$cas_token = null);
  public function getDelayed(mixed $keys, mixed $with_cas = false, mixed $value_cb = null): bool;
  public function getDelayedByKey(string $server_key, arraay $keys, bool $with_cas = false, ?callable $value_cb = null): bool;
  public function getMulti(mixed $keys, mixed &$cas_tokens = null, int $flags = 0): mixed;
  public function getMultiByKey(string $server_key, array $keys, mixed &$cas_tokens = null, int $flags = 0): mixed;
  public function getOption(int $option): mixed;
  public function getResultCode(): int;
  public function getResultMessage(): string;
  public function getServerByKey(string $server_key): mixed;
  public function getServerList(): array;
  public function resetServerList(): bool;
  public function getStats(): mixed;
  public function getVersion(): mixed;

  public function increment(string $key, int $offset = 1, mixed $initial_value = false, int $expiry = 0): mixed;
  public function incrementByKey(string $server_key, string $key, int $offset = 1, mxied $initial_value = false, int $expiry = 0): mixed;

  public function isPersistent(): bool;
  public function isPristine(): bool;

  public function prepend(mixed $key, mixed $value): bool;
  public function prependByKey(string $server_key, string $key, string $value): bool;

  public function quit(): bool;

  public function replace(mixed $key, mixed $value, int $expiration = 0): bool;
  public function replaceByKey(string $server_key, string $key, mixed $value, int $expiration = 0): bool;
  public function set(mixed $key, mixed $value, int $expiration = 0): bool;
  public function setByKey(string $server_key, string $key, mixed $value, int $expiration = 0): bool;
  public function setMulti(array<string, mixed> $items, int $expiration = 0): bool;
  public function setMultiByKey(string $server_key, array<string, mixed> $items, int $expiration = 0): bool;
  public function setOption(int $option, mixed $value): bool;
  public function setOptions(array<int, mixed> $options): bool;
  public function touch(string $key, int $expiration = 0): bool;
  public function touchByKey(string $server_key, string $key, int $expiration = 0): bool;
}
