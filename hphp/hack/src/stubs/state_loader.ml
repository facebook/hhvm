type native_load_result = {
  saved_state_fn: string;
  corresponding_rev: Hg.rev;
  mergebase_rev: Hg.global_rev option;
  mergebase: Hg.hg_rev option Future.t;
  is_cached: bool;
  state_distance: int;
  deptable_fn: string;
  deptable_is_64bit: bool;
  dirty_files: (Relative_path.t list * Relative_path.t list) Future.t;
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
    ~config:_
    ~use_canary:_
    ?saved_state_handle:_
    ~config_hash:_
    ~use_prechecked_files:_ =
  raise Not_supported
