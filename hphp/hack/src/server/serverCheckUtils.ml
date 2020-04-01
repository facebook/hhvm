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
open ServerLocalConfig.RemoteTypeCheck

let should_do_remote
    (opts : TypecheckerOptions.t) (fnl : Relative_path.Set.t) ~(t : float) :
    bool * float =
  let remote_type_check = TypecheckerOptions.remote_type_check opts in
  let remote_type_check_threshold =
    TypecheckerOptions.remote_type_check_threshold opts
  in
  let file_count = Relative_path.Set.cardinal fnl in
  let do_remote =
    match (remote_type_check, remote_type_check_threshold) with
    | (true, Some remote_type_check_threshold)
      when file_count >= remote_type_check_threshold ->
      Hh_logger.log
        "Going to schedule work because file count %d >= threshold %d"
        file_count
        remote_type_check_threshold;
      true
    | _ -> false
  in
  ( do_remote,
    Hh_logger.log_duration
      (Printf.sprintf
         "Calculated whether should do remote type checking (%B)"
         do_remote)
      t )

let start_typing_delegate genv env : env =
  let version_specifier = genv.local_config.remote_version_specifier in
  let transport_channel = genv.local_config.remote_transport_channel in
  let {
    declaration_threshold = defer_class_declaration_threshold;
    num_workers;
    max_batch_size;
    min_batch_size;
    worker_min_log_level;
    _;
  } =
    genv.local_config.remote_type_check
  in
  let root = Relative_path.path_of_prefix Relative_path.Root in
  {
    env with
    typing_service =
      {
        env.typing_service with
        delegate_state =
          Typing_service_delegate.start
            Typing_service_types.
              {
                defer_class_declaration_threshold;
                init_id = env.init_env.init_id;
                mergebase = env.init_env.mergebase;
                num_workers;
                recheck_id =
                  Option.value
                    env.init_env.recheck_id
                    ~default:env.init_env.init_id;
                root;
                server =
                  ServerApi.make_local_server_api
                    env.naming_table
                    ~root
                    ~ignore_hh_version:
                      (ServerArgs.ignore_hh_version genv.options);
                version_specifier;
                worker_min_log_level;
                remote_mode = JobRunner.Remote;
                transport_channel;
              }
            (* TODO: use env.typing_service.delegate_state when cancellation
                    implementation is finished *)
            (Typing_service_delegate.create ~max_batch_size ~min_batch_size ())
            ~recheck_id:env.init_env.recheck_id;
      };
  }

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
      match Relative_path.Map.find_opt fast x with
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

let get_check_info genv env =
  ServerEnv.(
    let init_id = env.init_env.init_id in
    let recheck_id = env.init_env.recheck_id in
    let profile_log = ServerArgs.profile_log genv.options in
    let profile_type_check_twice =
      genv.local_config.ServerLocalConfig.profile_type_check_twice
    in
    let profile_type_check_duration_threshold =
      genv.local_config.ServerLocalConfig.profile_type_check_duration_threshold
    in
    {
      Typing_check_service.init_id;
      recheck_id;
      profile_log;
      profile_type_check_twice;
      profile_type_check_duration_threshold;
    })
