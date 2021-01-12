type dirty_files = {
  master_changes: Relative_path.t list;
  local_changes: Relative_path.t list;
}

type hot_decls_paths = {
  legacy_hot_decls_path: string;
  shallow_hot_decls_path: string;
}

type native_load_result = {
  naming_table_path: string;
  corresponding_rev: Hg.rev;
  mergebase_rev: Hg.global_rev option;
  mergebase: Hg.hg_rev option Future.t;
  is_cached: bool;
  state_distance: int;
  deptable_fn: string;
  deptable_is_64bit: bool;
  dirty_files: dirty_files Future.t;
  hot_decls_paths: hot_decls_paths;
  errors_path: string;
}

type saved_state_handle = {
  saved_state_for_rev: Hg.rev;
  saved_state_everstore_handle: string;
  watchman_mergebase: ServerMonitorUtils.watchman_mergebase option;
}

type error = unit

type verbose_error = {
  message: string;
  stack: Utils.callstack;
  auto_retry: bool;
  environment: string option;
}
[@@deriving show]

let error_string_verbose _ =
  {
    message = "";
    auto_retry = false;
    stack = Utils.Callstack "";
    environment = None;
  }

let cached_state ~load_64bit:_ ?saved_state_handle:_ ~config_hash:_ ~rev:_ =
  None

exception Not_supported

let fetch_saved_state ~load_64bit:_ ~cache_limit:_ ~config:_ ~config_hash:_ _ =
  raise Not_supported

let mk_state_future
    ~config:_ ?saved_state_handle:_ ~config_hash:_ ~use_prechecked_files:_ =
  raise Not_supported
