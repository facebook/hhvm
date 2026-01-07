<?hh

// If you update this file, make sure `ext_watcher.php` is synced as well.

namespace HH {

  type WatcherOptions = shape(
    ?'repo_root' => string,
    ?'include_paths' => vec<string>,
    ?'include_extensions' => vec<string>,
    ?'exclude_paths' => vec<string>,
    ?'exclude_extensions' => vec<string>,
    ?'relative_root' => string,
    ?'fields' => keyset<string>,
    ?'socket_path' => string,
  );

  type WatcherFileResult = shape(
    ?'sha1hex' => string,
  );

  type WatcherResult = shape(
    'clock' => string,
    'files' => dict<string, WatcherFileResult>,
    'is_fresh_instance' => bool
  );

  function watcher_query(
    WatcherOptions $options,
    ?string $clock = null
  ): Awaitable<WatcherResult>;

  function watcher_get_clock(
    ?WatcherOptions $options = null,
  ): Awaitable<string>;

  function ext_watcher_version(): int;

} // namespace HH
