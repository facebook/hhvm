(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerEnv
open ServerLocalConfig

let get_naming_table_fallback_path genv (naming_table_fn : string option) :
    string option =
  Hh_logger.log "Figuring out naming table SQLite path...";
  match genv.local_config.naming_sqlite_path with
  | Some path ->
    Hh_logger.log "Naming table path from config: %s" path;
    Some path
  | None ->
    Hh_logger.log "No path set in config, using the loaded naming table";
    naming_table_fn

let extend_fast_sequential fast naming_table additional_files =
  Hh_logger.log "Extending FAST sequentially (in memory)...";
  Relative_path.Set.fold additional_files ~init:fast ~f:(fun x acc ->
      match Relative_path.Map.get fast x with
      | None ->
        (try
           let info = Naming_table.get_file_info_unsafe naming_table x in
           let info_names = FileInfo.simplify info in
           Relative_path.Map.add acc ~key:x ~data:info_names
         with Not_found_s _ -> acc)
      | Some _ -> acc)

let extend_fast_batch genv fast naming_table additional_files bucket_size =
  Hh_logger.log "Extending FAST in batch mode...";
  let additional_files = Relative_path.Set.elements additional_files in
  let get_one acc x =
    try
      let info = Naming_table.get_file_info_unsafe naming_table x in
      let info_names = FileInfo.simplify info in
      Relative_path.Map.add acc ~key:x ~data:info_names
    with Not_found_s _ -> acc
  in
  let job (acc : FileInfo.names Relative_path.Map.t) additional_files =
    Core_kernel.(
      let result = List.fold_left additional_files ~f:get_one ~init:acc in
      result)
  in
  let next =
    MultiWorker.next ~max_size:bucket_size genv.workers additional_files
  in
  let neutral = Relative_path.Map.empty in
  let merge = Relative_path.Map.union in
  let extended_fast =
    MultiWorker.call genv.workers ~job ~neutral ~merge ~next
  in
  Relative_path.Map.union fast extended_fast

let extend_fast
    genv
    (fast : FileInfo.names Relative_path.Map.t)
    (naming_table : Naming_table.t)
    (additional_files : Relative_path.Set.t) :
    FileInfo.names Relative_path.Map.t =
  let additional_count = Relative_path.Set.cardinal additional_files in
  if additional_count = 0 then
    fast
  else (
    Hh_logger.log "Extending the FAST by %d additional files" additional_count;
    let t = Unix.gettimeofday () in
    let bucket_size =
      genv.local_config.ServerLocalConfig.extend_fast_bucket_size
    in
    let extended_fast =
      match Naming_table.get_forward_naming_fallback_path naming_table with
      | Some _sqlite_path when additional_count >= bucket_size ->
        extend_fast_batch genv fast naming_table additional_files bucket_size
      | _ -> extend_fast_sequential fast naming_table additional_files
    in
    let _t = Hh_logger.log_duration "Extended FAST" t in
    extended_fast
  )

let global_typecheck_kind genv env =
  if genv.ServerEnv.options |> ServerArgs.remote then
    ServerCommandTypes.Remote_blocking "Forced remote type check"
  else if env.can_interrupt then
    ServerCommandTypes.Interruptible
  else
    ServerCommandTypes.Blocking

let set_up_remote_logging (env : ServerEnv.env) : unit =
  let send_progress (message : string) : unit =
    ServerBusyStatus.send
      env
      ServerCommandTypes.(Doing_global_typecheck (Remote_blocking message));
    ServerProgress.send_progress_to_monitor "%s" message
  in
  let send_percentage_progress
      ~(operation : string)
      ~(done_count : int)
      ~(total_count : int)
      ~(unit : string) : unit =
    let message =
      ServerProgress.make_percentage_progress_message
        ~operation
        ~done_count
        ~total_count
        ~unit
    in
    ServerBusyStatus.send
      env
      ServerCommandTypes.(Doing_global_typecheck (Remote_blocking message));
    ServerProgress.send_progress_to_monitor ~include_in_logs:false "%s" message
  in
  Typing_remote_check_service.set_up_logging
    ~send_progress
    ~send_percentage_progress

let maybe_remote_type_check genv env fnl =
  let t = Unix.gettimeofday () in
  let (do_remote, _t) =
    if genv.ServerEnv.options |> ServerArgs.remote then
      (true, t)
    else
      Typing_remote_check_service.should_do_remote env.tcopt fnl t
  in
  if do_remote then (
    set_up_remote_logging env;
    let eden_threshold =
      genv.local_config.remote_worker_eden_checkout_threshold
    in
    let remote_errors =
      Typing_remote_check_service.go
        genv.workers
        env.tcopt
        ~naming_table:(Some env.naming_table)
        ~naming_sqlite_path:(get_naming_table_fallback_path genv None)
        ~eden_threshold
        fnl
    in
    Some remote_errors
  ) else
    None

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
  ServerEnv.(
    let init_id = env.init_env.init_id in
    let recheck_id = env.init_env.recheck_id in
    { Typing_check_service.init_id; recheck_id })
