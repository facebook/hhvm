<?hh

// If you update this, make sure to update the corresponding `ext_watcher.hhi` file.

namespace HH {

type WatcherOptions = shape(
  ?'repo_root' => string,
  ?'include_paths' => vec<string>,
  ?'include_extensions' => vec<string>,
  ?'exclude_paths' => vec<string>,
  ?'exclude_extensions' => vec<string>,
  ?'relative_root' => string,

  // The fields of the file result set that the query should include.
  // Allowed values:  sha1hex
  ?'fields' => keyset<string>,

  // Watchman specific
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

// This will return a JSON-encoded string of the result.
<<__Native>>
function watcher_query(
  /* WatcherOptions */ mixed $options,
  /* ?string */ mixed $clock = null
): Awaitable<string>;

<<__Native>>
function watcher_get_clock(
  /* WatcherOptions */ ?mixed $options = null,
): Awaitable<string>;

/**
 * This should be bumped with every non-backwards compatible API change.
 * 1 => first version
 */
function ext_watcher_version(): int {
  return (int)\phpversion("watch");
}

} // namespace HH
