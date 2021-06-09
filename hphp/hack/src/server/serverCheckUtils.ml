(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
open ServerLocalConfig
include ServerRemoteUtils

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
         with Naming_table.File_info_not_found -> acc)
      | Some _ -> acc)

let extend_fast_batch genv fast naming_table additional_files bucket_size =
  Hh_logger.log "Extending FAST in batch mode...";
  let additional_files = Relative_path.Set.elements additional_files in
  let get_one acc x =
    try
      let info = Naming_table.get_file_info_unsafe naming_table x in
      let info_names = FileInfo.simplify info in
      Relative_path.Map.add acc ~key:x ~data:info_names
    with Naming_table.File_info_not_found -> acc
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

let get_check_info genv env : Typing_service_types.check_info =
  {
    Typing_service_types.init_id = env.init_env.init_id;
    recheck_id = env.init_env.recheck_id;
    profile_log = ServerArgs.profile_log genv.options;
    profile_decling = genv.local_config.ServerLocalConfig.profile_decling;
    profile_type_check_twice =
      genv.local_config.ServerLocalConfig.profile_type_check_twice;
    profile_type_check_duration_threshold =
      genv.local_config.ServerLocalConfig.profile_type_check_duration_threshold;
    profile_type_check_memory_threshold_mb =
      genv.local_config.ServerLocalConfig.profile_type_check_memory_threshold_mb;
  }

type user_filter =
  | UserFilterInclude of Str.regexp list
  | UserFilterExclude of Str.regexp list

let user_filter_of_json (json : Hh_json.json) : user_filter =
  let open Hh_json_helpers in
  let json = Some json in
  let type_ = Jget.string_exn json "type" in
  let regexes = Jget.string_array_exn json "regexes" in
  let regexes =
    List.map regexes ~f:(fun re ->
        try Str.regexp re
        with Failure explanation ->
          raise
          @@ Failure
               (Printf.sprintf
                  "Could not parse regex \"%s\": %s"
                  re
                  explanation))
  in
  if String.equal type_ "include" then
    UserFilterInclude regexes
  else if String.equal type_ "exclude" then
    UserFilterExclude regexes
  else
    raise @@ Failure (Printf.sprintf "Unknown filter type: '%s'" type_)

let user_filter_should_type_check
    (user_filter : user_filter) (path : Relative_path.t) : bool =
  let suffix = Relative_path.suffix path in
  let matches_any regexes =
    List.exists regexes ~f:(fun re ->
        (try Str.search_forward re suffix 0 >= 0 with Caml.Not_found -> false))
  in
  match user_filter with
  | UserFilterInclude regexes -> matches_any regexes
  | UserFilterExclude regexes -> not (matches_any regexes)

let user_filters_should_type_check
    (user_filters : user_filter list) (path : Relative_path.t) : bool =
  List.for_all user_filters ~f:(fun f -> user_filter_should_type_check f path)

let user_filter_type_check_files ~to_recheck ~reparsed ~is_ide_file =
  Hh_logger.log "Filtering files to type check using user-defined predicates";
  let config_file_path =
    Sys_utils.expanduser "~/.hack_type_check_files_filter"
  in
  let read_config_file_once () : user_filter list =
    let contents = Sys_utils.cat config_file_path in
    let json = Hh_json.json_of_string contents in
    let filters = Hh_json.get_array_exn json in
    List.map filters ~f:user_filter_of_json
  in
  let rec read_config_file () : user_filter list =
    Hh_logger.log "Reading in config file at %s" config_file_path;
    try read_config_file_once ()
    with e ->
      ServerProgress.send_progress
        ~include_in_logs:false
        "error while applying user file filter, see logs to continue";
      let e = Exception.wrap e in
      Printf.fprintf stderr "%s" (Exception.to_string e);
      Hh_logger.log
        "An exception occurred while reading %s, retrying in 5s..."
        config_file_path;
      Sys_utils.sleep ~seconds:5.;
      read_config_file ()
  in
  let filters = read_config_file () in
  let to_recheck_original_count = Relative_path.Set.cardinal to_recheck in
  let to_recheck =
    Relative_path.Set.filter to_recheck ~f:(fun path ->
        Relative_path.Set.mem reparsed path
        || is_ide_file path
        || user_filters_should_type_check filters path)
  in
  let passed_filter_count = Relative_path.Set.cardinal to_recheck in
  Hh_logger.log
    "Filtered files to recheck from %d to %d"
    to_recheck_original_count
    passed_filter_count;
  to_recheck
