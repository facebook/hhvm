type native_load_result = {
  saved_state_fn : string;
  corresponding_base_rev : string;
  is_cached : bool;
  state_distance : int;
  deptable_fn : string;
  dirty_files : (string list) Future.t;
}

exception Not_supported

let mk_state_future _ _ =
  raise Not_supported
