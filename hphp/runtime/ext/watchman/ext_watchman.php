<?hh

/* A Watchman (https://facebook.github.io/watchman/) API for HHVM.
 *
 * Example usage:
 *
 *  print(watchman_run('["clock", "/home/jbower/"]')->join());
 *  // => {"clock":"c:1459617179:3235334:1:388831","version":"4.5.0"}
 *
 */

namespace HH {

/*
 * Asynchronously run a one-shot Watchman query. A JSON encoded response string
 * is returned after a successful join. See the Watchman docs for details of
 * queries and responses.
 *
 * Any errors will be reported via an exception from this call or on join.
 */
<<__Native>>
function watchman_run(
  string $json_query,
  ?string $socket_name = null,
): Awaitable<string>;

type WatchmanResult = shape(
  'version' => string,
  ?'buildinfo' => string,
  ?'capabilities' => dict<string, bool>,
  ?'clock' => mixed,
  ?'config' => shape(
    ?'content_hash_max_items' => bool,
    ?'content_hash_warming' => bool,
    ?'enforce_root_files' => bool,
    ?'fsevents_latency' => num,
    ?'fsevents_try_resync' => bool,
    ?'gc_age_seconds' => int,
    ?'gc_interval_seconds' => int,
    ?'hint_num_dirs' => int,
    ?'hint_num_files_per_dir' => int,
    ?'idle_reap_age_seconds' => int,
    ?'illegal_fstypes' => vec<string>,
    ?'illegal_fstypes_advice' => string,
    ?'ignore_dirs' => vec<string>,
    ?'ignore_vcs' => vec<string>,
    ?'prefer_split_fsevents_watcher' => bool,
    ?'root_files' => vec<string>,
    ?'root_restrict_files' => vec<string>,
    ?'settle' => int,
    ?'suppress_recrawl_warnings' => bool,
    ...
  ),
  ?'debug' => shape(
    ...
  ),
  ?'dropped' => vec<string>,
  ?'error' => string,
  ?'files' => vec<shape(
    ?'cclock' => string,
    ?'content.sha1hex' => mixed,
    ?'ctime' => int,
    ?'dev' => int,
    ?'exists' => bool,
    ?'gid' => int,
    ?'ino' => int,
    ?'mode' => int,
    ?'mtime' => int,
    ?'name' => string,
    ?'nlink' => int,
    ?'oclock' => string,
    ?'size' => int,
    ?'uid' => int,
    ...
  )>,
  ?'is_fresh_instance' => bool,
  ?'metadata' => mixed,
  ?'named_pipe' => string,
  ?'no_sync_needed' => vec<string>,
  ?'root' => string,
  ?'roots' => vec<string>,
  ?'sockname' => string,
  ?'subscription' => string,
  ?'state-enter' => string,
  ?'synced' => vec<string>,
  ?'unix_domain' => string,
  ?'warning' => string,
  ...
);

/**
 * This should be bumped with every non-backwards compatible API change.
 * 1 => first version
 */
function ext_watchman_version(): int {
  return (int)\phpversion("watchman");
}

} // namespace HH
