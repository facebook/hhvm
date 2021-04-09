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

let hh_log_heap () =
  let to_g n = Float.of_int n /. (1024.0 *. 1024.0 *. 1024.0) in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %fG" (to_g hs)

let no_incremental_check (options : ServerArgs.options) : bool =
  let in_check_mode = ServerArgs.check_mode options in
  let full_init = Option.is_none (SharedMem.loaded_dep_table_filename ()) in
  in_check_mode && full_init

let indexing ?hhi_filter (genv : ServerEnv.genv) :
    Relative_path.t list Bucket.next * float =
  ServerProgress.send_progress_to_monitor_w_timeout "indexing";
  let t = Unix.gettimeofday () in
  let get_next =
    ServerFiles.make_next
      ?hhi_filter
      ~indexer:(genv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.config)
  in
  HackEventLogger.indexing_end t;
  let t = Hh_logger.log_duration "indexing" t in
  (get_next, t)

let parsing
    ~(lazy_parse : bool)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    ~(get_next : Relative_path.t list Bucket.next)
    ?(count : int option)
    (t : float)
    ~(trace : bool)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
  @@ fun () ->
  begin
    match count with
    | None -> ServerProgress.send_progress_to_monitor_w_timeout "%s" "parsing"
    | Some c ->
      ServerProgress.send_progress_to_monitor_w_timeout "parsing %d files" c
  end;
  let quick = lazy_parse in
  let ctx = Provider_utils.ctx_from_server_env env in
  let (fast, errorl, _) =
    Parsing_service.go
      ctx
      ~quick
      ~show_all_errors:true
      genv.workers
      Relative_path.Set.empty
      ~get_next
      ~trace
      env.popt
  in
  let naming_table = Naming_table.update_many env.naming_table fast in
  hh_log_heap ();
  let hs = SharedMem.heap_size () in
  Stats.(stats.init_parsing_heap_size <- hs);

  (* The true count of how many files we parsed is wrapped up in the get_next closure.
  But our caller provides us 'count' option in cases where it knows the number in
  advance, e.g. during init. We'll log that for now. In future it'd be nice to
  log the actual number parsed. *)
  HackEventLogger.parsing_end_for_init t hs ~parsed_count:count;
  let env =
    { env with naming_table; errorl = Errors.merge errorl env.errorl }
  in
  (env, Hh_logger.log_duration "Parsing" t)

let update_files
    (genv : ServerEnv.genv)
    (naming_table : Naming_table.t)
    (ctx : Provider_context.t)
    (t : float)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : float =
  if no_incremental_check genv.options then
    t
  else (
    Hh_logger.log "Updating file dependencies...";
    CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
    @@ fun () ->
    (let deps_mode = Provider_context.get_deps_mode ctx in
     Naming_table.iter naming_table (Typing_deps.Files.update_file deps_mode));
    HackEventLogger.updating_deps_end t;
    Hh_logger.log_duration "Updated file dependencies" t
  )

let naming
    (env : ServerEnv.env)
    (t : float)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
  @@ fun () ->
  ServerProgress.send_progress_to_monitor_w_timeout
    "resolving symbol references";
  let ctx = Provider_utils.ctx_from_server_env env in
  let env =
    Naming_table.fold
      env.naming_table
      ~f:
        begin
          fun k v env ->
          let (errorl, failed_naming) =
            Naming_global.ndecl_file_error_if_already_bound ctx k v
          in
          {
            env with
            errorl = Errors.merge errorl env.errorl;
            failed_naming =
              Relative_path.Set.union env.failed_naming failed_naming;
          }
        end
      ~init:env
  in
  hh_log_heap ();
  HackEventLogger.global_naming_end t (SharedMem.heap_size ());
  (env, Hh_logger.log_duration "Naming" t)

let is_path_in_range (path : string) (range : ServerArgs.files_to_check_range) :
    bool =
  (* TODO: not sure how to include the prefix - should we just have these as strings? *)
  let is_from_prefix_incl =
    match range.ServerArgs.from_prefix_incl with
    | None -> true
    | Some (from_prefix_incl : Relative_path.t) ->
      String.(path >= Relative_path.suffix from_prefix_incl)
  in
  let is_to_prefix_excl =
    match range.ServerArgs.to_prefix_excl with
    | None -> true
    | Some (to_prefix_excl : Relative_path.t) ->
      String.(path < Relative_path.suffix to_prefix_excl)
  in
  is_from_prefix_incl && is_to_prefix_excl

let does_path_match_spec path (spec : ServerArgs.files_to_check_spec) : bool =
  match spec with
  | ServerArgs.Range range -> is_path_in_range path range
  | ServerArgs.Prefix prefix ->
    String_utils.string_starts_with path (Relative_path.suffix prefix)

let does_path_match_specs path (specs : ServerArgs.files_to_check_spec list) :
    bool =
  match List.find specs ~f:(fun spec -> does_path_match_spec path spec) with
  | None -> false
  | Some _ -> true

let filter_filenames_by_spec
    (fnl : Relative_path.t list) (spec : ServerArgs.save_state_spec_info) :
    Relative_path.t list =
  let filtered_filenames =
    List.filter fnl ~f:(fun (fn : Relative_path.t) ->
        (* TODO: not sure how to include the prefix *)
        does_path_match_specs
          (Relative_path.suffix fn)
          spec.ServerArgs.files_to_check)
  in
  Hh_logger.log "Filtered filenames to %d" (List.length filtered_filenames);
  filtered_filenames

let type_check
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (files_to_check : Relative_path.t list)
    (init_telemetry : Telemetry.t)
    (t : float)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  (* No type checking in AI mode *)
  if Option.is_some (ServerArgs.ai_mode genv.options) then
    (env, t)
  else if
    ServerArgs.check_mode genv.options
    || Option.is_some (ServerArgs.save_filename genv.options)
    || Option.is_some (ServerArgs.save_with_spec genv.options)
  then (
    (* Prechecked files are not supported in check/saving-state modes, we
     * should always recheck everything necessary up-front. *)
    assert (
      match env.prechecked_files with
      | Prechecked_files_disabled -> true
      | _ -> false );

    let count = List.length files_to_check in
    let logstring = Printf.sprintf "Filter %d files" count in
    Hh_logger.log "Begin %s" logstring;

    let (files_to_check : Relative_path.t list) =
      match ServerArgs.save_with_spec genv.options with
      | None -> files_to_check
      | Some (spec : ServerArgs.save_state_spec_info) ->
        filter_filenames_by_spec files_to_check spec
    in
    let (_new_t : float) = Hh_logger.log_duration logstring t in
    let count = List.length files_to_check in
    let logstring = Printf.sprintf "type-check %d files" count in
    Hh_logger.log "Begin %s" logstring;
    let ( (errorl : Errors.t),
          (delegate_state : Typing_service_delegate.state),
          (typecheck_telemetry : Telemetry.t) ) =
      let memory_cap =
        genv.local_config.ServerLocalConfig.max_typechecker_worker_memory_mb
      in
      let longlived_workers =
        genv.local_config.ServerLocalConfig.longlived_workers
      in
      let ctx = Provider_utils.ctx_from_server_env env in
      CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
      @@ fun () ->
      Typing_check_service.go
        ctx
        genv.workers
        env.typing_service.delegate_state
        (Telemetry.create ())
        Relative_path.Set.empty
        files_to_check
        ~memory_cap
        ~longlived_workers
        ~check_info:(ServerCheckUtils.get_check_info genv env)
    in
    hh_log_heap ();
    let hash_telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.object_ ~key:"init" ~value:init_telemetry
      |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
      |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    in
    HackEventLogger.type_check_end
      (Some telemetry)
      ~heap_size:(SharedMem.heap_size ())
      ~started_count:count
      ~count
      ~experiments:genv.local_config.ServerLocalConfig.experiments
      ~start_t:t;
    let env =
      {
        env with
        typing_service = { env.typing_service with delegate_state };
        errorl = Errors.merge errorl env.errorl;
      }
    in
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
          { env.init_env with why_needed_full_init = Some init_telemetry };
      }
    in
    (env, t)
