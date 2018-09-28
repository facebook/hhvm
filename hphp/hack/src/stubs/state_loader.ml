type native_load_result = {
  saved_state_fn : string;
  corresponding_rev : Hg.rev;
  mergebase_rev : Hg.svn_rev option;
  is_cached : bool;
  state_distance : int;
  deptable_fn : string;
  dirty_files : (string list * string list) Future.t;
}

type mini_state_handle = {
  mini_state_for_rev : Hg.rev;
  mini_state_everstore_handle : string;
  watchman_mergebase : ServerMonitorUtils.watchman_mergebase option;
}

type error = unit

let error_string _ = ""

let cached_state
  ?mini_state_handle:_
  ~config_hash:_
  ~rev:_ = None

exception Not_supported

let fetch_mini_state
  ~cache_limit:_
  ~config:_
  ~config_hash:_
  _ = raise Not_supported

let mk_state_future
  ~config:_
  ~use_canary:_
  ?mini_state_handle:_
  ~config_hash:_
  ~use_prechecked_files:_
  = raise Not_supported
