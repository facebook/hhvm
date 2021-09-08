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

let indexing ?hhi_filter ~(profile_label : string) (genv : ServerEnv.genv) :
    Relative_path.t list Bucket.next * float =
  ServerProgress.send_progress "indexing";
  let t = Unix.gettimeofday () in
  let get_next =
    ServerUtils.make_next
      ?hhi_filter
      ~indexer:(genv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.config)
  in
  HackEventLogger.indexing_end ~desc:profile_label t;
  let t = Hh_logger.log_duration ("indexing " ^ profile_label) t in
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
    | None -> ServerProgress.send_progress "%s" "parsing"
    | Some c -> ServerProgress.send_progress "parsing %d files" c
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
  HackEventLogger.parsing_end_for_init
    t
    hs
    ~parsed_count:count
    ~desc:profile_label;
  let env =
    { env with naming_table; errorl = Errors.merge errorl env.errorl }
  in
  (env, Hh_logger.log_duration ("Parsing " ^ profile_label) t)

let update_files
    ?(warn_on_naming_costly_iter : bool option)
    (genv : ServerEnv.genv)
    (naming_table : Naming_table.t)
    (ctx : Provider_context.t)
    (t : float)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : float =
  if no_incremental_check genv.options then
    t
  else (
    Hh_logger.log "Updating dep->filename [%s]... " profile_label;
    let deps_mode = Provider_context.get_deps_mode ctx in
    let count = ref 0 in
    CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
    @@ fun () ->
    Naming_table.iter
      ?warn_on_naming_costly_iter
      naming_table
      ~f:(fun path fi ->
        Typing_deps.Files.update_file deps_mode path fi;
        count := !count + 1);
    HackEventLogger.updating_deps_end
      ~count:!count
      ~desc:profile_label
      ~start_t:t;
    Hh_logger.log_duration "Updated dep->filename" t
  )

let naming
    (env : ServerEnv.env)
    (t : float)
    ~(profile_label : string)
    ~(profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  CgroupProfiler.collect_cgroup_stats ~profiling ~stage:profile_label
  @@ fun () ->
  ServerProgress.send_progress "resolving symbol references";
  let ctx = Provider_utils.ctx_from_server_env env in
  let count = ref 0 in
  let env =
    Naming_table.fold
      env.naming_table
      ~f:(fun k v env ->
        count := !count + 1;
        let (errorl, failed_naming) =
          Naming_global.ndecl_file_error_if_already_bound ctx k v
        in
        {
          env with
          errorl = Errors.merge errorl env.errorl;
          failed_naming =
            Relative_path.Set.union env.failed_naming failed_naming;
        })
      ~init:env
  in
  hh_log_heap ();
  HackEventLogger.global_naming_end
    ~count:!count
    ~desc:profile_label
    ~heap_size:(SharedMem.heap_size ())
    ~start_t:t;
  (env, Hh_logger.log_duration ("Naming " ^ profile_label) t)

let log_type_check_end
    env
    genv
    ~start_t
    ~count
    ~desc
    ~init_telemetry
    ~typecheck_telemetry
    ~adhoc_profiling : unit =
  let hash_telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.object_ ~key:"init" ~value:init_telemetry
    |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
    |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    |> Telemetry.object_ ~key:"errors" ~value:(Errors.as_telemetry env.errorl)
    |> Telemetry.object_
         ~key:"repo_states"
         ~value:(Watchman.RepoStates.get_as_telemetry ())
  in
  HackEventLogger.type_check_end
    (Some telemetry)
    ~adhoc_profiling:(Adhoc_profiler.CallTree.to_string adhoc_profiling)
    ~heap_size:(SharedMem.heap_size ())
    ~started_count:count
    ~count
    ~desc
    ~experiments:genv.local_config.ServerLocalConfig.experiments
    ~start_t

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
  then (
    (* Prechecked files are not supported in check/saving-state modes, we
     * should always recheck everything necessary up-front. *)
    assert (
      match env.prechecked_files with
      | Prechecked_files_disabled -> true
      | _ -> false);

    let count = List.length files_to_check in
    let logstring = Printf.sprintf "Filter %d files [%s]" count profile_label in
    Hh_logger.log "Begin %s" logstring;

    let (_new_t : float) = Hh_logger.log_duration logstring t in
    let count = List.length files_to_check in
    let logstring = Printf.sprintf "type-check %d files" count in
    Hh_logger.log "Begin %s" logstring;
    let {
      Typing_check_service.errors = errorl;
      delegate_state;
      telemetry = typecheck_telemetry;
      adhoc_profiling;
      _;
    } =
      let memory_cap =
        genv.local_config.ServerLocalConfig.max_typechecker_worker_memory_mb
      in
      let longlived_workers =
        genv.local_config.ServerLocalConfig.longlived_workers
      in
      let remote_execution = env.ServerEnv.remote_execution in
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
        ~remote_execution
        ~check_info:(ServerCheckUtils.get_check_info genv env)
    in
    hh_log_heap ();
    let env =
      {
        env with
        typing_service = { env.typing_service with delegate_state };
        errorl = Errors.merge errorl env.errorl;
      }
    in
    log_type_check_end
      env
      genv
      ~start_t:t
      ~count
      ~desc:profile_label
      ~init_telemetry
      ~typecheck_telemetry
      ~adhoc_profiling;
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
