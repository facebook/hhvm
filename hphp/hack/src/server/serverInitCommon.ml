(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Hh_prelude
module Bucket = Hack_bucket
open ServerEnv

let directory_walk
    ?hhi_filter ~(telemetry_label : string) (genv : ServerEnv.genv) :
    Relative_path.t list Bucket.next * float =
  Server_progress.write "indexing";
  let t = Unix.gettimeofday () in
  let get_next =
    ServerUtils.make_next
      ?hhi_filter
      ~indexer:(genv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.config)
  in
  HackEventLogger.indexing_end ~desc:telemetry_label t;
  let t = Hh_logger.log_duration ("indexing " ^ telemetry_label) t in
  (get_next, t)

let parse_files_and_update_forward_naming_table
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    ~(get_next : Relative_path.t list Bucket.next)
    ?(count : int option)
    (t : float)
    ~(trace : bool)
    ~(cache_decls : bool)
    ~(telemetry_label : string)
    ~(cgroup_steps : CgroupProfiler.step_group)
    ~(worker_call : MultiWorker.call_wrapper) : ServerEnv.env * float =
  CgroupProfiler.step_start_end cgroup_steps telemetry_label
  @@ fun _cgroup_step ->
  begin
    match count with
    | None -> Server_progress.write "parsing"
    | Some c -> Server_progress.write "parsing %d files" c
  end;
  let ctx = Provider_utils.ctx_from_server_env env in
  let defs_per_file =
    Direct_decl_service.go
      ctx
      ~worker_call
      genv.workers
      ~get_next
      ~trace
      ~cache_decls
  in
  let naming_table = Naming_table.update_many env.naming_table defs_per_file in
  let hs = SharedMem.SMTelemetry.heap_size () in
  Stats.(stats.init_parsing_heap_size <- hs);

  (* The true count of how many files we parsed is wrapped up in the get_next closure.
     But our caller provides us 'count' option in cases where it knows the number in
     advance, e.g. during init. We'll log that for now. In future it'd be nice to
     log the actual number parsed. *)
  HackEventLogger.parsing_end_for_init
    t
    hs
    ~parsed_count:count
    ~desc:telemetry_label;
  let env = { env with naming_table } in
  (env, Hh_logger.log_duration ("Parsing " ^ telemetry_label) t)

let update_reverse_naming_table_from_env_and_get_duplicate_name_errors
    (env : ServerEnv.env)
    (t : float)
    ~(telemetry_label : string)
    ~(cgroup_steps : CgroupProfiler.step_group) : ServerEnv.env * float =
  CgroupProfiler.step_start_end cgroup_steps telemetry_label
  @@ fun _cgroup_step ->
  Server_progress.with_message "resolving symbol references" @@ fun () ->
  let ctx = Provider_utils.ctx_from_server_env env in
  let count = ref 0 in
  let env =
    Naming_table.fold
      env.naming_table
      ~f:(fun k fi env ->
        count := !count + 1;
        let failed_naming =
          Naming_global.ndecl_file_and_get_conflict_files ctx k fi.FileInfo.ids
        in
        {
          env with
          failed_naming =
            Relative_path.Set.union env.failed_naming failed_naming;
        })
      ~init:env
  in
  HackEventLogger.global_naming_end
    ~count:!count
    ~desc:telemetry_label
    ~heap_size:(SharedMem.SMTelemetry.heap_size ())
    ~start_t:t;
  (env, Hh_logger.log_duration ("Naming " ^ telemetry_label) t)

let validate_no_errors (errors : Errors.t) : unit =
  let witness_opt =
    Errors.fold_errors errors ~init:None ~f:(fun path error _acc ->
        Some (path, error))
  in
  match witness_opt with
  | None -> ()
  | Some (path, error) ->
    let error = User_error.to_absolute error |> Errors.to_string in
    Hh_logger.log "Unexpected error during init: %s" error;
    HackEventLogger.invariant_violation_bug
      "unexpected error during init"
      ~path
      ~data:error;
    ()

let log_type_check_end
    env
    genv
    ~start_t
    ~total_rechecked_count
    ~desc
    ~init_telemetry
    ~typecheck_telemetry : unit =
  let hash_telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in

  let telemetry =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"init"
         ~value:(ServerEnv.Init_telemetry.get init_telemetry)
    |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
    |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    |> Telemetry.object_
         ~key:"errors"
         ~value:(Errors.as_telemetry_summary env.errorl)
    |> Telemetry.object_
         ~key:"repo_states"
         ~value:(Watchman.RepoStates.get_as_telemetry ())
  in
  HackEventLogger.type_check_end
    (Some telemetry)
    ~heap_size:(SharedMem.SMTelemetry.heap_size ())
    ~started_count:total_rechecked_count
    ~total_rechecked_count
    ~desc
    ~experiments:genv.local_config.ServerLocalConfig.experiments
    ~start_t

let defer_or_do_type_check
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (files_to_check : Relative_path.t list)
    (init_telemetry : Init_telemetry.t)
    (t : float)
    ~(telemetry_label : string)
    ~(cgroup_steps : CgroupProfiler.step_group) : ServerEnv.env * float =
  (* No type checking in AI mode *)
  if Option.is_some (ServerArgs.ai_mode genv.options) then
    (env, t)
  else if
    ServerArgs.check_mode genv.options
    || Option.is_some (ServerArgs.save_filename genv.options)
  then (
    (* Prechecked files are not supported in check/saving-state modes, we
     * should always recheck everything necessary up-front. *)
    assert (
      match env.prechecked_files with
      | Prechecked_files_disabled -> true
      | _ -> false);
    (* Streaming errors aren't supported for these niche cases: for simplicity, the only
       code that sets up and tears down streaming errors is in [ServerTypeCheck.type_check].
       Our current code calls into typing_check_service.ml without having done that set up,
       and so we will override whatever was set before and disable it now. *)
    Hh_logger.log "Streaming errors disabled for eager init";
    Server_progress.enable_error_production false;
    Server_progress.write "typechecking";

    let count = List.length files_to_check in
    let logstring =
      Printf.sprintf "Filter %d files [%s]" count telemetry_label
    in
    Hh_logger.log "Begin %s" logstring;
    let files_to_check =
      if
        not
          genv.ServerEnv.local_config
            .ServerLocalConfig.enable_type_check_filter_files
      then
        files_to_check
      else
        let files_to_check_set = Relative_path.Set.of_list files_to_check in
        let filtered_check =
          ServerCheckUtils.user_filter_type_check_files
            ~to_recheck:files_to_check_set
            ~reparsed:Relative_path.Set.empty
        in
        Relative_path.Set.elements filtered_check
    in
    let (_new_t : float) = Hh_logger.log_duration logstring t in
    let total_rechecked_count = List.length files_to_check in
    let logstring =
      Printf.sprintf "type-check %d files" total_rechecked_count
    in
    Hh_logger.log "Begin %s" logstring;
    let {
      Typing_check_service.errors = errorl;
      telemetry = typecheck_telemetry;
      _;
    } =
      let longlived_workers =
        genv.local_config.ServerLocalConfig.longlived_workers
      in
      let hh_distc_fanout_threshold =
        let use_distc = genv.local_config.ServerLocalConfig.use_distc in
        Option.some_if
          use_distc
          genv.local_config.ServerLocalConfig.hh_distc_fanout_threshold
      in
      let root = Some (ServerArgs.root genv.ServerEnv.options) in
      let ctx = Provider_utils.ctx_from_server_env env in
      CgroupProfiler.step_start_end cgroup_steps telemetry_label @@ fun () ->
      Typing_check_service.go
        ctx
        genv.workers
        (Telemetry.create ())
        files_to_check
        ~root
        ~longlived_workers
        ~hh_distc_fanout_threshold
        ~check_info:
          (ServerCheckUtils.get_check_info
             ~check_reason:(ServerEnv.Init_telemetry.get_reason init_telemetry)
             ~log_errors:true
             genv
             env)
    in
    let env = { env with errorl = Errors.merge errorl env.errorl } in
    log_type_check_end
      env
      genv
      ~start_t:t
      ~total_rechecked_count
      ~desc:telemetry_label
      ~init_telemetry
      ~typecheck_telemetry;
    (env, Hh_logger.log_duration logstring t)
  ) else
    let needs_recheck =
      List.fold files_to_check ~init:Relative_path.Set.empty ~f:(fun acc fn ->
          Relative_path.Set.add acc fn)
    in
    let env =
      {
        env with
        needs_recheck = Relative_path.Set.union env.needs_recheck needs_recheck;
        (* eagerly start rechecking after init *)
        full_check_status = Full_check_started;
        init_env =
          { env.init_env with why_needed_full_check = Some init_telemetry };
      }
    in
    (env, t)
