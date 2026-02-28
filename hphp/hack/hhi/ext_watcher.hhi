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
    ?'sync_timeout' => int,
    ?'defer_vcs' => bool,
    ?'empty_on_fresh_instance' => bool,
  );

  // More detailed description of the fields are here: https://www.internalfb.com/intern/staticdocs/watchman/docs/cmd/query#available-fields.
  type WatcherFileResult = shape(
    ?'content.sha1hex' => string,
    ?'exists' => bool,
    ?'cclock' => string,
    ?'oclock' => string,
    ?'ctime' => int,
    ?'ctime_ms' => int,
    ?'ctime_us' => int,
    ?'ctime_ns' => int,
    ?'ctime_f' => float,
    ?'mtime' => int,
    ?'mtime_ms' => int,
    ?'mtime_us' => int,
    ?'mtime_ns' => int,
    ?'mtime_f' => float,
    ?'size' => int,
    ?'mode' => int,
    ?'uid' => int,
    ?'gid' => int,
    ?'ino' => int,
    ?'dev' => int,
    ?'nlink' => int,
    ?'new' => bool,
    ?'type' => string,
    ?'symlink_target' => string,
  );

  type WatcherResult = shape(
    'clock' => string,
    'files' => dict<string, WatcherFileResult>,
    'is_fresh_instance' => bool
  );

  function watcher_query(
    WatcherOptions $options,
    ?string $clock = null
  ): Awaitable<string>;

  function watcher_get_clock(
    ?WatcherOptions $options = null,
  ): Awaitable<string>;

  function ext_watcher_version(): int;

} // namespace HH
