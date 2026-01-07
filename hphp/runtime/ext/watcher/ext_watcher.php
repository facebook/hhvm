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
);

type WatcherFileResult = shape(
  ?'sha1hex' => string,
);

type WatcherResult = shape(
  'clock' => string,
  'files' => dict<string, WatcherFileResult>,
  'is_fresh_instance' => bool
);

<<__Native>>
function watcher_query(
  /* WatcherOptions */ mixed $options,
  /* ?string */ mixed $clock = null
): Awaitable<WatcherResult>; /* WatcherResult */

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
