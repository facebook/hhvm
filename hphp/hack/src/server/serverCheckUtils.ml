(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerEnv
open ServerLocalConfig

let get_naming_table_fallback_path genv (deptable_fn: string option): string option =
  Hh_logger.log "Figuring out naming table SQLite path";
  match genv.local_config.naming_sqlite_path with
  | Some path ->
    Hh_logger.log "Found path: %s" path;
    Some path
  | None when genv.local_config.enable_naming_table_fallback ->
    Hh_logger.log "No path, using dep table";
    deptable_fn
  | None ->
    Hh_logger.log "Load from blob";
    None

let extend_fast fast naming_table additional_files =
  Relative_path.Set.fold additional_files ~init:fast ~f:begin fun x acc ->
    match Relative_path.Map.get fast x with
    | None ->
        (try
           let info = Naming_table.get_file_info_unsafe naming_table x in
           let info_names = FileInfo.simplify info in
           Relative_path.Map.add acc ~key:x ~data:info_names
         with Not_found ->
           acc)
    | Some _ -> acc
  end

let maybe_remote_type_check genv env fnl =
  let t = Unix.gettimeofday () in
  let (do_remote, _t) = Typing_remote_check_service.should_do_remote
    env.tcopt
    fnl t
  in
  if do_remote then begin
    let eden_threshold =
      genv.local_config.remote_worker_eden_checkout_threshold
    in
    let remote_errors = Typing_remote_check_service.go
      genv.workers
      env.tcopt
      ~naming_table:(Some env.naming_table)
      ~naming_sqlite_path:(get_naming_table_fallback_path genv None)
      ~eden_threshold
      fnl
    in
    Some remote_errors
  end
  else None

let maybe_remote_type_check_without_interrupt genv env fnl ~local =
  match maybe_remote_type_check genv env fnl with
  | Some remote_errors -> remote_errors
  | None -> local ()

let maybe_remote_type_check_with_interrupt genv env fnl ~local =
  (* TODO: remote type check should actually respond to interruption *)
  match maybe_remote_type_check genv env fnl with
  | Some remote_errors -> (remote_errors, env, [])
  | None -> local ()

let get_check_info env =
  let open ServerEnv in
  let init_id = env.init_env.init_id in
  let recheck_id = env.init_env.recheck_id in
  {Typing_check_service.
    init_id;
    recheck_id;
  }
