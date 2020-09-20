<?hh

namespace HH {

function watchman_run(
  string $json_query,
  ?string $socket_name = null,
): Awaitable<string>;

function watchman_subscribe(
  string $json_query,
  string $path,
  string $sub_name,
  string $callback,
  ?string $socket_name = null,
): Awaitable<void>;

function watchman_check_sub(string $sub_name): bool;

function watchman_sync_sub(
  string $sub_name,
  int $timeout_ms = 0,
): Awaitable<bool>;

function watchman_unsubscribe(string $sub_name): Awaitable<string>;

function ext_watchman_version(): int;

} // namespace HH
