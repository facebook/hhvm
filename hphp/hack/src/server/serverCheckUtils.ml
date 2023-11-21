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

let get_naming_table_fallback_path genv : string option =
  match genv.local_config.naming_sqlite_path with
  | Some path as p ->
    Hh_logger.log "Loading naming table path from config: %s" path;
    p
  | None ->
    Hh_logger.log "No path set in config, using the loaded naming table";
    None

(** [extend_defs_per_file_sequential defs_per_file naming_table additional_files]
  extends [defs_per_file] file-to-defs table with definitions in [additional_files]
  by querying [naming_table] for their definitions.
  Does so sequentially file by file. *)
let extend_defs_per_file_sequential defs_per_file naming_table additional_files
    =
  Hh_logger.log "Extending file-to-defs table sequentially (in memory)...";
  Relative_path.Set.fold
    additional_files
    ~init:defs_per_file
    ~f:(fun path acc ->
      match Relative_path.Map.find_opt defs_per_file path with
      | None ->
        (try
           let info = Naming_table.get_file_info_unsafe naming_table path in
           let info_names = FileInfo.simplify info in
           Relative_path.Map.add acc ~key:path ~data:info_names
         with
        | Naming_table.File_info_not_found -> acc)
      | Some _ -> acc)

(** [extend_defs_per_file_batch genv defs_per_file naming_table additional_files bucket_size]
  extends [defs_per_file] file-to-defs table with definitions in [additional_files]
  by querying [naming_table] for their definitions.
  Does so in batches of size [bucket_size]. *)
let extend_defs_per_file_batch
    genv defs_per_file naming_table additional_files bucket_size =
  Hh_logger.log "Extending file-to-defs table in batch mode...";
  let additional_files = Relative_path.Set.elements additional_files in
  let get_one acc x =
    try
      let info = Naming_table.get_file_info_unsafe naming_table x in
      let info_names = FileInfo.simplify info in
      Relative_path.Map.add acc ~key:x ~data:info_names
    with
    | Naming_table.File_info_not_found -> acc
  in
  let job (acc : FileInfo.names Relative_path.Map.t) additional_files =
    Core.(
      let result = List.fold_left additional_files ~f:get_one ~init:acc in
      result)
  in
  let next =
    MultiWorker.next ~max_size:bucket_size genv.workers additional_files
  in
  let neutral = Relative_path.Map.empty in
  let merge = Relative_path.Map.union in
  let extended_defs_per_file =
    MultiWorker.call genv.workers ~job ~neutral ~merge ~next
  in
  Relative_path.Map.union defs_per_file extended_defs_per_file

(** [extend_defs_per_file genv defs_per_file naming_table additional_files]
  extends [defs_per_file] file-to-defs table with definitions in [additional_files]
  by querying [naming_table] for their definitions.
  Does so either sequentially or in batches depending on how many files to add. *)
let extend_defs_per_file
    genv
    (defs_per_file : FileInfo.names Relative_path.Map.t)
    (naming_table : Naming_table.t)
    (additional_files : Relative_path.Set.t) :
    FileInfo.names Relative_path.Map.t =
  let additional_count = Relative_path.Set.cardinal additional_files in
  if additional_count = 0 then
    defs_per_file
  else (
    Hh_logger.log
      "Extending the file-to-defs table by %d additional files"
      additional_count;
    let t = Unix.gettimeofday () in
    let bucket_size =
      genv.local_config.ServerLocalConfig.extend_defs_per_file_bucket_size
    in
    let extended_defs_per_file =
      match Naming_table.get_forward_naming_fallback_path naming_table with
      | Some _sqlite_path when additional_count >= bucket_size ->
        extend_defs_per_file_batch
          genv
          defs_per_file
          naming_table
          additional_files
          bucket_size
      | _ ->
        extend_defs_per_file_sequential
          defs_per_file
          naming_table
          additional_files
    in
    let _t = Hh_logger.log_duration "Extended file-to-defs table" t in
    extended_defs_per_file
  )

let get_check_info ~check_reason ~log_errors (genv : genv) env :
    Typing_service_types.check_info =
  {
    Typing_service_types.init_id = env.init_env.init_id;
    check_reason;
    log_errors;
    recheck_id = env.init_env.recheck_id;
    use_max_typechecker_worker_memory_for_decl_deferral =
      genv.local_config
        .ServerLocalConfig.use_max_typechecker_worker_memory_for_decl_deferral;
    per_file_profiling = genv.local_config.ServerLocalConfig.per_file_profiling;
    memtrace_dir = genv.local_config.ServerLocalConfig.memtrace_dir;
  }

type user_filter =
  | UserFilterInclude of Str.regexp list
  | UserFilterExclude of Str.regexp list
  | UserFilterFiles of SSet.t

let user_filter_of_json (json : Hh_json.json) : user_filter =
  let open Hh_json_helpers in
  let json = Some json in
  let type_ = Jget.string_exn json "type" in
  if String.equal type_ "include" then
    let regexes = Jget.string_array_exn json "regexes" in
    let regexes =
      List.map regexes ~f:(fun re ->
          try Str.regexp re with
          | Failure explanation ->
            raise
            @@ Failure
                 (Printf.sprintf
                    "Could not parse regex \"%s\": %s"
                    re
                    explanation))
    in
    UserFilterInclude regexes
  else if String.equal type_ "exclude" then
    let regexes = Jget.string_array_exn json "regexes" in
    let regexes =
      List.map regexes ~f:(fun re ->
          try Str.regexp re with
          | Failure explanation ->
            raise
            @@ Failure
                 (Printf.sprintf
                    "Could not parse regex \"%s\": %s"
                    re
                    explanation))
    in
    UserFilterExclude regexes
  else if String.equal type_ "specific_files" then
    let file_list = Jget.string_array_exn json "files" in
    UserFilterFiles (SSet.of_list file_list)
  else
    raise @@ Failure (Printf.sprintf "Unknown filter type: '%s'" type_)

let user_filter_should_type_check
    (user_filter : user_filter) (path : Relative_path.t) : bool =
  let suffix = Relative_path.suffix path in
  let matches_any regexes =
    List.exists regexes ~f:(fun re ->
        try Str.search_forward re suffix 0 >= 0 with
        | Stdlib.Not_found -> false)
  in
  match user_filter with
  | UserFilterInclude regexes -> matches_any regexes
  | UserFilterExclude regexes -> not (matches_any regexes)
  | UserFilterFiles files -> SSet.mem (Relative_path.suffix path) files

let user_filters_should_type_check
    (user_filters : user_filter list) (path : Relative_path.t) : bool =
  List.for_all user_filters ~f:(fun f -> user_filter_should_type_check f path)

let user_filter_type_check_files ~to_recheck ~reparsed =
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
    try read_config_file_once () with
    | e ->
      ServerProgress.write
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
        || user_filters_should_type_check filters path)
  in
  let passed_filter_count = Relative_path.Set.cardinal to_recheck in
  Hh_logger.log
    "Filtered files to recheck from %d to %d"
    to_recheck_original_count
    passed_filter_count;
  to_recheck
