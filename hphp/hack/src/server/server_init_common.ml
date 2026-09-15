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
open Server_env

let directory_walk
    ?hhi_filter ~(telemetry_label : string) (genv : Server_env.genv) :
    Relative_path.t list Bucket.next * float =
  Server_progress.write "indexing";
  let t = Unix.gettimeofday () in
  let get_next =
    Server_utils.make_next
      ?hhi_filter
      ~indexer:(genv.indexer Find_utils.file_filter)
      ~extra_roots:(Server_config.extra_paths genv.config)
  in
  Hack_event_logger.indexing_end ~desc:telemetry_label t;
  let t = Hh_logger.log_duration ("indexing " ^ telemetry_label) t in
  (get_next, t)

(** This parses files with the direct decl parser and uses the result to update
  the naming table in the provided env. *)
let parse_files_and_update_forward_naming_table
    (genv : Server_env.genv)
    (env : Server_env.env)
    ~(get_next : Relative_path.t list Bucket.next)
    ?(count : int option)
    (t : float)
    ~(trace : bool)
    ~(decl_mode : Direct_decl_service.direct_decl_mode)
    ~(telemetry_label : string)
    ~(cgroup_steps : Cgroup_profiler.step_group)
    ~(worker_call : Multi_worker.call_wrapper) : Server_env.env * float =
  Cgroup_profiler.step_start_end cgroup_steps telemetry_label
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
      ~decl_mode
  in
  let naming_table = Naming_table.update_many env.naming_table defs_per_file in
  let hs = Shared_mem.SMTelemetry.heap_size () in
  Stats.(stats.init_parsing_heap_size <- hs);

  (* The true count of how many files we parsed is wrapped up in the get_next closure.
     But our caller provides us 'count' option in cases where it knows the number in
     advance, e.g. during init. We'll log that for now. In future it'd be nice to
     log the actual number parsed. *)
  Hack_event_logger.parsing_end_for_init
    t
    hs
    ~parsed_count:count
    ~desc:telemetry_label;
  let env = { env with naming_table } in
  (env, Hh_logger.log_duration ("Parsing " ^ telemetry_label) t)

let update_reverse_naming_table_from_env_and_get_duplicate_name_errors
    (env : Server_env.env)
    (t : float)
    ~(telemetry_label : string)
    ~(cgroup_steps : Cgroup_profiler.step_group) : Server_env.env * float =
  Cgroup_profiler.step_start_end cgroup_steps telemetry_label
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
          Naming_global.ndecl_file_and_get_conflict_files ctx k fi.File_info.ids
        in
        {
          env with
          failed_naming =
            Relative_path.Set.union env.failed_naming failed_naming;
        })
      ~init:env
  in
  Hack_event_logger.global_naming_end
    ~count:!count
    ~desc:telemetry_label
    ~heap_size:(Shared_mem.SMTelemetry.heap_size ())
    ~start_t:t;
  (env, Hh_logger.log_duration ("Naming " ^ telemetry_label) t)

let validate_no_errors (errors : Diagnostics.t) : unit =
  let witness_opt =
    Diagnostics.fold_errors errors ~init:None ~f:(fun path error _acc ->
        Some (path, error))
  in
  match witness_opt with
  | None -> ()
  | Some (path, error) ->
    let error = User_diagnostic.to_absolute error |> Diagnostics.to_string in
    Hh_logger.log "Unexpected error during init: %s" error;
    Hack_event_logger.invariant_violation_bug
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
  let hash_telemetry = Server_utils.log_and_get_sharedmem_load_telemetry () in

  let telemetry =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"init"
         ~value:(Server_env.Init_telemetry.get init_telemetry)
    |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
    |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    |> Telemetry.object_
         ~key:"errors"
         ~value:(Diagnostics.as_telemetry_summary env.diagnostics)
    |> Telemetry.object_
         ~key:"repo_states"
         ~value:(Server_notifier.get_repo_states_telemetry genv.notifier)
  in
  Hack_event_logger.type_check_end
    (Some telemetry)
    ~heap_size:(Shared_mem.SMTelemetry.heap_size ())
    ~started_count:total_rechecked_count
    ~total_rechecked_count
    ~desc
    ~experiments:genv.local_config.Server_local_config.experiments
    ~start_t

let defer_or_do_type_check
    (genv : Server_env.genv)
    (env : Server_env.env)
    (files_to_check : Relative_path.t list)
    (init_telemetry : Init_telemetry.t)
    (t : float)
    ~(telemetry_label : string)
    ~(cgroup_steps : Cgroup_profiler.step_group) : Server_env.env * float =
  if Server_args.check_mode genv.options then (
    (* Prechecked files are not supported in check mode, we
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
          genv.Server_env.local_config
            .Server_local_config.enable_type_check_filter_files
      then
        files_to_check
      else
        let files_to_check_set = Relative_path.Set.of_list files_to_check in
        let filtered_check =
          Server_check_utils.user_filter_type_check_files
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
      Typing_check_service.diagnostics = errorl;
      telemetry = typecheck_telemetry;
      _;
    } =
      let longlived_workers =
        genv.local_config.Server_local_config.longlived_workers
      in
      let hh_distc_config =
        let use_distc = genv.local_config.Server_local_config.use_distc in
        Option.some_if
          use_distc
          Typing_check_service.
            {
              enable_fanout_aware_distc =
                genv.Server_env.local_config
                  .Server_local_config.enable_fanout_aware_distc;
              fanout_threshold =
                genv.Server_env.local_config
                  .Server_local_config.hh_distc_fanout_threshold;
              fanout_full_init_threshold =
                genv.Server_env.local_config
                  .Server_local_config.hh_distc_fanout_full_init_threshold;
            }
      in
      let root = Server_args.root genv.Server_env.options in
      let ctx = Provider_utils.ctx_from_server_env env in
      Cgroup_profiler.step_start_end cgroup_steps telemetry_label @@ fun () ->
      Typing_check_service.go
        ctx
        genv.workers
        (Telemetry.create ())
        files_to_check
        ~root:(Some root)
        ~longlived_workers
        ~hh_distc_config
        ~check_info:
          (Server_check_utils.get_check_info
             ~check_reason:(Server_env.Init_telemetry.get_reason init_telemetry)
             ~log_errors:true
             ~discard_warnings:(Server_env.discard_warnings env)
             genv
             env)
        ~warnings_saved_state:Server_env.(env.init_env.mergebase_warning_hashes)
    in
    let env =
      { env with diagnostics = Diagnostics.merge errorl env.diagnostics }
    in
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
