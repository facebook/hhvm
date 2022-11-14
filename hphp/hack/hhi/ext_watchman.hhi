<?hh

namespace HH {

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

  function watchman_query(
    mixed $json_query,
    ?string $socket_name = null,
  ): Awaitable<WatchmanResult>;

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
