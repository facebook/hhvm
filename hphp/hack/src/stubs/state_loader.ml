type native_load_result = {
  saved_state_fn : string;
  corresponding_rev : Hg.rev;
  is_cached : bool;
  state_distance : int;
  deptable_fn : string;
  dirty_files : (string list) Future.t;
}

type mini_state_handle = {
  mini_state_for_rev : Hg.rev;
  mini_state_everstore_handle : string;
}

exception Not_supported

let mk_state_future ~use_canary:_ ?mini_state_handle:_ ~config_hash:_ ~tiny:_ _ =
  raise Not_supported
