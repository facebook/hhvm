(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
open Reordered_argument_collections
open Option.Monad_infix

(*****************************************************************************)
(* Main initialization *)
(*****************************************************************************)

let () = Printexc.record_backtrace true

let force_break_recheck_loop_for_test_ref = ref false

let force_break_recheck_loop_for_test x =
  force_break_recheck_loop_for_test_ref := x

module MainInit : sig
  val go :
    genv ->
    ServerArgs.options ->
    (unit -> env) ->
    (* init function to run while we have init lock *)
    env
end = struct
  (* This code is only executed when the options --check is NOT present *)
  let go genv options init_fun =
    let root = ServerArgs.root options in
    let t = Unix.gettimeofday () in
    let pid = Unix.getpid () in
    begin
      match ProcFS.first_cgroup_for_pid pid with
      | Ok cgroup ->
        Hh_logger.log "Server Pid: %d" pid;
        Hh_logger.log "Server cGroup: %s" cgroup
      | _ -> ()
    end;
    Hh_logger.log "Initializing Server (This might take some time)";

    (* note: we only run periodical tasks on the root, not extras *)
    let env = init_fun () in
    Hh_logger.log "Server is partially ready";
    ServerIdle.init genv root;
    let t' = Unix.gettimeofday () in
    Hh_logger.log "Took %f seconds." (t' -. t);
    HackEventLogger.server_is_partially_ready ();

    env
end

module Program = struct
  let preinit () =
    (* Warning: Global references inited in this function, should
         be 'restored' in the workers, because they are not 'forked'
         anymore. See `ServerWorker.{save/restore}_state`. *)
    Sys_utils.set_signal
      Sys.sigusr1
      (Sys.Signal_handle Typing.debug_print_last_pos);
    Sys_utils.set_signal
      Sys.sigusr2
      (Sys.Signal_handle
         (fun _ ->
           Hh_logger.log "Got sigusr2 signal. Going to shut down.";
           Exit.exit
             ~msg:
               "Hh_server received a stop signal. This can happen from a large rebase/update"
             Exit_status.Server_shutting_down_due_to_sigusr2))

  let run_once_and_exit
      genv
      env
      (save_state_result : SaveStateServiceTypes.save_state_result option) =
    let recheck_stats =
      Option.map
        ~f:ServerEnv.RecheckLoopStats.to_user_telemetry
        env.ServerEnv.last_recheck_loop_stats_for_actual_work
    in
    ServerError.print_error_list
      stdout
      ~stale_msg:None
      ~output_json:(ServerArgs.json_mode genv.options)
      ~error_list:
        (List.map (Errors.get_error_list env.errorl) ~f:User_error.to_absolute)
      ~save_state_result
      ~recheck_stats;

    WorkerController.force_quit_all ();

    let has_errors = not (Errors.is_empty env.errorl) in
    let is_saving_state_and_ignoring_errors =
      ServerArgs.gen_saved_ignore_type_errors genv.options
      && Option.is_some (ServerArgs.save_filename genv.options)
    in
    let error_code =
      if has_errors then
        if Option.is_some (ServerArgs.write_symbol_info genv.options) then
          32
        else if not is_saving_state_and_ignoring_errors then
          1
        else
          0
      else
        0
    in
    exit error_code

  (* filter and relativize updated file paths *)
  let exit_if_critical_update genv ~(raw_updates : SSet.t) : unit =
    let hhconfig_in_updates =
      SSet.mem
        raw_updates
        (Relative_path.to_absolute ServerConfig.repo_config_path)
    in
    if hhconfig_in_updates then begin
      let (new_config, _) = ServerConfig.load ~silent:false genv.options in
      if not (ServerConfig.is_compatible genv.config new_config) then (
        Hh_logger.log
          "%s changed in an incompatible way; please restart %s.\n"
          (Relative_path.suffix ServerConfig.repo_config_path)
          GlobalConfig.program_name;

        (* TODO: Notify the server monitor directly about this. *)
        Exit.exit Exit_status.Hhconfig_changed
      )
    end;
    let package_config_in_updates =
      SSet.mem
        raw_updates
        (Relative_path.to_absolute PackageConfig.repo_config_path)
    in
    if package_config_in_updates then begin
      Hh_logger.log
        "%s changed; please restart %s.\n"
        (Relative_path.suffix PackageConfig.repo_config_path)
        GlobalConfig.program_name;
      Exit.exit Exit_status.Package_config_changed
    end
end

let finalize_init init_env typecheck_telemetry init_telemetry =
  (* rest is just logging/telemetry *)
  let t' = Unix.gettimeofday () in
  let hash_telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.duration ~start_time:init_env.init_start_t
    |> Telemetry.object_
         ~key:"init"
         ~value:(ServerEnv.Init_telemetry.get init_telemetry)
    |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
    |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    |> Telemetry.int_
         ~key:"heap_size"
         ~value:(SharedMem.SMTelemetry.heap_size ())
  in
  HackEventLogger.server_is_ready telemetry;
  Hh_logger.log
    "SERVER_IS_READY. Took %f seconds to init. Telemetry:\n%s"
    (t' -. init_env.init_start_t)
    (Telemetry.to_string telemetry);
  ()

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

(** Query for changed files. This is a hard to understand method...
[let (env, changes, new_clock, may_be_stale, telemetry) = query_notifier genv env query_kind start_time].

CARE! [query_kind] is hard to understand...
* [`Sync] -- a watchman sync query, i.e. guarantees to have picked up all updates
  up to the moment it was invoked. Watchman does this by writing a dummy file
  and waiting until the OS notifies about it; the OS guarantees to send notifications in order.
* [`Async] -- picks up changes that watchman has pushed over the subscription, but we don't
  do a sync, so therefore there might be changes on disk that watchman will tell us about
  in future.
* [`Skip] -- CARE! Despite its name, this also behaves much like [`Async].

The return value [may_be_stale] indicates that the most recent watchman event that got pushed
was not a "sync" to the dummy file. This will never be true for [`Sync] query kind (which deliberately
waits for the sync), but it might be true or false for [`Async] and [`Skip]. The caller can
use this as a hint that we don't know whether there are more disk changes.
Personally, I've never actually seen it be true. It's surfaced to the user in clientCheckStatus.ml
with the message "this may be stale, probably due to watchman being unresponsive". *)
let query_notifier
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (query_kind : [> `Async | `Skip | `Sync ])
    (start_time : float) :
    ServerEnv.env
    * Relative_path.Set.t
    * Watchman.clock option
    * bool
    * Telemetry.t =
  let telemetry =
    Telemetry.create () |> Telemetry.duration ~key:"start" ~start_time
  in
  let (env, (raw_updates, clock)) =
    match query_kind with
    | `Sync ->
      ( env,
        (try ServerNotifier.get_changes_sync genv.notifier with
        | Watchman.Timeout -> (ServerNotifier.Unavailable, None)) )
    | `Async ->
      ( { env with last_notifier_check_time = start_time },
        ServerNotifier.get_changes_async genv.notifier )
    | `Skip -> (env, (ServerNotifier.AsyncChanges SSet.empty, None))
  in
  let telemetry = Telemetry.duration telemetry ~key:"notified" ~start_time in
  let unpack_updates = function
    | ServerNotifier.Unavailable -> (true, SSet.empty)
    | ServerNotifier.StateEnter _ -> (true, SSet.empty)
    | ServerNotifier.StateLeave _ -> (true, SSet.empty)
    | ServerNotifier.AsyncChanges updates -> (true, updates)
    | ServerNotifier.SyncChanges updates -> (false, updates)
  in
  let (updates_stale, raw_updates) = unpack_updates raw_updates in
  let rec pump_async_updates acc acc_clock =
    match ServerNotifier.async_reader_opt genv.notifier with
    | Some reader when Buffered_line_reader.is_readable reader ->
      let (changes, clock) = ServerNotifier.get_changes_async genv.notifier in
      let (_, raw_updates) = unpack_updates changes in
      pump_async_updates (SSet.union acc raw_updates) clock
    | _ -> (acc, acc_clock)
  in
  let (raw_updates, clock) = pump_async_updates raw_updates clock in
  let telemetry = Telemetry.duration telemetry ~key:"pumped" ~start_time in
  Program.exit_if_critical_update genv ~raw_updates;
  let updates =
    FindUtils.post_watchman_filter_from_fully_qualified_raw_updates
      ~root:(ServerArgs.root genv.options)
      ~raw_updates
  in
  (* CARE! For streaming-errors to work in clientCheckStatus.ml, the test
     which hh_server uses to determine "is there a non-empty set of changes which prompt me
     to start a new check" must be at least as strict as the one in clientCheckStatus.
     They're identical, in fact, because they both use the same watchman filter
     (FilesToIgnore.watchman_server_expression_terms) and the same post-watchman-filter. *)
  let telemetry =
    telemetry
    |> Telemetry.duration ~key:"processed" ~start_time
    |> Telemetry.int_ ~key:"raw_updates" ~value:(SSet.cardinal raw_updates)
    |> Telemetry.int_ ~key:"updates" ~value:(Relative_path.Set.cardinal updates)
  in
  if not @@ Relative_path.Set.is_empty updates then
    HackEventLogger.notifier_returned start_time (SSet.cardinal raw_updates);
  (env, updates, clock, updates_stale, telemetry)

let update_stats_after_recheck :
    RecheckLoopStats.t ->
    ServerTypeCheck.CheckStats.t ->
    check_kind:ServerTypeCheck.CheckKind.t ->
    telemetry:Telemetry.t ->
    start_time:seconds_since_epoch ->
    RecheckLoopStats.t =
 fun {
       RecheckLoopStats.total_changed_files_count;
       per_batch_telemetry;
       total_rechecked_count;
       updates_stale;
       recheck_id;
       last_iteration_start_time = _;
       duration;
       time_first_result = _;
       any_full_checks;
     }
     {
       ServerTypeCheck.CheckStats.total_rechecked_count =
         total_rechecked_count_in_iteration;
       reparse_count;
       time_first_result;
     }
     ~check_kind
     ~telemetry
     ~start_time ->
  {
    RecheckLoopStats.total_changed_files_count =
      total_changed_files_count + reparse_count;
    per_batch_telemetry = telemetry :: per_batch_telemetry;
    total_rechecked_count =
      total_rechecked_count + total_rechecked_count_in_iteration;
    updates_stale;
    recheck_id;
    last_iteration_start_time = start_time;
    duration = duration +. (Unix.gettimeofday () -. start_time);
    time_first_result;
    any_full_checks =
      any_full_checks || ServerTypeCheck.CheckKind.is_full_check check_kind;
  }

(* This function loops until it has processed all outstanding changes.
 *
 * One reason for doing this is so that, if a client makes a request,
 * then we can process all outstanding changes prior to handling that request.
 * That way the client will get an up-to-date answer.
 *
 * Another reason is to get meaningful logging in case of watchman events.
 * When a rebase occurs, Watchman/dfind takes a while to give us the full list
 * of updates, and it often comes in batches. To get an accurate measurement
 * of rebase time, we use the heuristic that any changes that come in
 * right after one rechecking round finishes to be part of the same
 * rebase, and we don't log the recheck_end event until the update list
 * is no longer getting populated.
 *
 * The above doesn't apply in presence of interruptions / cancellations -
 * it's possible for client to request current recheck to be stopped.
 *)
let rec recheck_until_no_changes_left stats genv env select_outcome :
    RecheckLoopStats.t * env =
  let start_time = Unix.gettimeofday () in
  (* this is telemetry for the current batch, i.e. iteration: *)
  let telemetry =
    Telemetry.create () |> Telemetry.float_ ~key:"start_time" ~value:start_time
  in

  (* When a new client connects, we use the synchronous notifier.
      This is to get synchronous file system changes when invoking
      hh_client in terminal.

     CARE! The [`Skip] option doesn't in fact skip. It will still
     retrieve queued-up watchman updates.

      NB: This also uses synchronous notify on establishing a persistent
      connection. This is harmless, but could maybe be filtered away. *)
  let query_kind =
    match select_outcome with
    | ClientProvider.Select_new _ -> `Sync
    | ClientProvider.Select_nothing
    | ClientProvider.Select_exception _
    | ClientProvider.Not_selecting_hg_updating ->
      if Float.(start_time - env.last_notifier_check_time > 0.5) then
        `Async
      else
        `Skip
    | ClientProvider.Select_persistent ->
      (* Do not aggressively process any disk changes when there are pending persistent
         client requests - some of them might be edits, and we don't want to
         do analysis on mid-edit state of the world. (Nevertheless, [`Skip] still may
         pick up updates...) *)
      `Skip
  in
  let (env, updates, clock, updates_stale, query_telemetry) =
    query_notifier genv env query_kind start_time
  in
  (* The response from [query_notifier] is tricky to unpick...
     * For [`Sync | `Async] it will always return [clock=Some] and updates may be empty or not.
     * For [`Skip] it might return [clock=Some] and updates empty or not; or it might
       return [clock=None] with updates empty. *)
  let telemetry =
    telemetry
    |> Telemetry.object_ ~key:"query" ~value:query_telemetry
    |> Telemetry.duration ~key:"query_done" ~start_time
  in
  let stats = { stats with RecheckLoopStats.updates_stale } in
  let is_idle =
    (match select_outcome with
    | ClientProvider.Select_persistent -> false
    | _ -> true)
    && (* "average person types [...] between 190 and 200 characters per minute"
        * 60/200 = 0.3 *)
    Float.(start_time - env.last_command_time > 0.3)
  in
  (* saving any file is our trigger to start full recheck *)
  let env =
    if Option.is_some clock && not (Option.equal String.equal clock env.clock)
    then begin
      Hh_logger.log "Recheck at watchclock %s" (ServerEnv.show_clock clock);
      { env with clock }
    end else
      env
  in
  let env =
    if Relative_path.Set.is_empty updates then
      env
    else
      let disk_needs_parsing =
        Relative_path.Set.union updates env.disk_needs_parsing
      in
      match env.full_recheck_on_file_changes with
      | Paused _ ->
        let () = Hh_logger.log "Skipping full check due to `hh --pause`" in
        { env with disk_needs_parsing; full_check_status = Full_check_needed }
      | _ ->
        { env with disk_needs_parsing; full_check_status = Full_check_started }
  in
  let telemetry = Telemetry.duration telemetry ~key:"got_updates" ~start_time in

  (* If the client went away (e.g. user pressed Ctrl+C while waiting for the typecheck),
     let's clean up now. *)
  let env =
    match env.nonpersistent_client_pending_command_needs_full_check with
    | Some (_command, _reason, client)
      when is_full_check_needed env.full_check_status
           && Float.(start_time - env.last_command_time > 5.0) -> begin
      try
        ClientProvider.ping client;
        env
      with
      | ClientProvider.Client_went_away ->
        ClientProvider.shutdown_client client;
        {
          env with
          nonpersistent_client_pending_command_needs_full_check = None;
        }
    end
    | _ -> env
  in

  (* If a typecheck had been suspended due to IDE edits, then we want to resume it eventually...
     To recap: if an IDE edit comes in, it takes us time to suspend the current typecheck,
     handle the edit, then resume the current typecheck. If we did this every edit then we'd
     get sluggish perf. We have two "debounce" mechanisms to avoid resuming too eagerly:
     * If we handled any IDE action, then we won't accept any further CLI clients until
       the IDE tells us it has no further pending work which it does by sending IDE_IDLE.
       The code to deny further clients is when [ServerMain.serve_one_iteration] calls
       [ClientProvider.sleep_and_check]. The code to reset upon IDE_IDLE is in [ServerRpc.handle]
       when it receives IDE_IDLE.
     * If we handled an IDE edit action, then we won't resume any typechecking work until either
       5.0s has elapsed or there was a disk change. The code to suspend typechecking work is when
       [ServerCommand.handle] returns [Needs_writes {recheck_restart_is_needed=false}] and
       its caller [ServerMain.persistent_client_interrupt_handler] sets [env.full_check_status=Full_check_needed].
       This has effect because [ServerMain.recheck_until_no_changes_left] is in charge of deciding
       whether a check is needed, and it decides "no" unless [ServerEnv.is_full_check_started].
       The code to resume typechecking work is right here! We'll restart only after 5.0s.
     * These mechanisms notwithstanding, we'll still start full recheck immediately
       upon any file save. *)
  let env =
    if
      is_full_check_needed env.full_check_status
      && Float.(start_time > env.last_command_time + 5.0)
    then begin
      Hh_logger.log "Restarting full check after 5.0s";
      { env with full_check_status = Full_check_started }
    end else
      env
  in
  (* Same as above, but for persistent clients *)
  let env =
    match env.persistent_client_pending_command_needs_full_check with
    | Some (_command, reason) when is_full_check_needed env.full_check_status ->
      Hh_logger.log "Restarting full check due to %s" reason;
      { env with full_check_status = Full_check_started }
    | _ -> env
  in
  let telemetry =
    Telemetry.duration telemetry ~key:"sorted_out_client" ~start_time
  in
  (* We have some new, or previously un-processed updates *)
  let full_check =
    ServerEnv.is_full_check_started env.full_check_status
    (* Prioritize building search index over full rechecks. *)
    && (Queue.is_empty SearchServiceRunner.SearchServiceRunner.queue
       (* Unless there is something actively waiting for this *)
       || Option.is_some
            env.nonpersistent_client_pending_command_needs_full_check)
  in
  let lazy_check =
    (not @@ Relative_path.Set.is_empty env.ide_needs_parsing) && is_idle
  in
  let telemetry =
    telemetry
    |> Telemetry.bool_ ~key:"full_check" ~value:full_check
    |> Telemetry.bool_ ~key:"lazy_check" ~value:lazy_check
    |> Telemetry.duration ~key:"figured_check_kind" ~start_time
  in
  if (not full_check) && not lazy_check then
    let telemetry =
      Telemetry.string_ telemetry ~key:"check_kind" ~value:"None"
    in
    let stats =
      {
        stats with
        RecheckLoopStats.per_batch_telemetry =
          telemetry :: stats.RecheckLoopStats.per_batch_telemetry;
      }
    in
    (stats, env)
  else
    let check_kind =
      if lazy_check then
        ServerTypeCheck.CheckKind.Lazy
      else
        ServerTypeCheck.CheckKind.Full
    in
    let check_kind_str = ServerTypeCheck.CheckKind.to_string check_kind in
    let env = { env with can_interrupt = not lazy_check } in
    let needed_full_init = env.init_env.why_needed_full_check in
    let old_errorl = Errors.get_error_list env.errorl in

    (* HERE'S WHERE WE DO THE HEAVY WORK! **)
    let telemetry =
      telemetry
      |> Telemetry.string_ ~key:"check_kind" ~value:check_kind_str
      |> Telemetry.duration ~key:"type_check_start" ~start_time
    in
    let (env, check_stats, type_check_telemetry) =
      CgroupProfiler.step_group check_kind_str ~log:(not lazy_check)
      @@ ServerTypeCheck.type_check genv env check_kind start_time
    in
    let telemetry =
      telemetry
      |> Telemetry.object_ ~key:"type_check" ~value:type_check_telemetry
      |> Telemetry.duration ~key:"type_check_end" ~start_time
    in

    (* END OF HEAVY WORK *)

    (* Final telemetry and cleanup... *)
    let env = { env with can_interrupt = true } in
    begin
      match (needed_full_init, env.init_env.why_needed_full_check) with
      | (Some needed_full_init, None) ->
        finalize_init env.init_env telemetry needed_full_init
      | _ -> ()
    end;
    ServerStamp.touch_stamp_errors old_errorl (Errors.get_error_list env.errorl);
    let telemetry =
      Telemetry.duration telemetry ~key:"finalized_and_touched" ~start_time
    in
    let stats =
      update_stats_after_recheck
        stats
        check_stats
        ~check_kind
        ~start_time
        ~telemetry
    in
    (* Avoid batching ide rechecks with disk rechecks - there might be
     * other ide edits to process first and we want to give the main loop
     * a chance to process them first.
     * Similarly, if a recheck was interrupted because of arrival of command
     * that needs writes, break the recheck loop to give that command chance
     * to be handled in main loop.
     * Finally, tests have ability to opt-out of batching completely. *)
    if
      lazy_check
      || Option.is_some env.pending_command_needs_writes
      || !force_break_recheck_loop_for_test_ref
    then
      (stats, env)
    else
      recheck_until_no_changes_left stats genv env select_outcome

let new_serve_iteration_id () = Random_id.short_string ()

(* This is safe to run only in the main loop, when workers are not doing
 * anything. *)
let main_loop_command_handler client_kind client result =
  match result with
  | ServerUtils.Done env -> env
  | ServerUtils.Needs_full_recheck { env; finish_command_handling; reason } ->
  begin
    match client_kind with
    | `Non_persistent ->
      (* We should not accept any new clients until this is cleared *)
      assert (
        Option.is_none env.nonpersistent_client_pending_command_needs_full_check);
      {
        env with
        nonpersistent_client_pending_command_needs_full_check =
          Some (finish_command_handling, reason, client);
      }
    | `Persistent ->
      (* Persistent client will not send any further commands until previous one
       * is handled. *)
      assert (
        Option.is_none env.persistent_client_pending_command_needs_full_check);
      {
        env with
        persistent_client_pending_command_needs_full_check =
          Some (finish_command_handling, reason);
      }
  end
  | ServerUtils.Needs_writes
      {
        env;
        finish_command_handling;
        recheck_restart_is_needed = _;
        reason = _;
      } ->
    finish_command_handling env

let generate_and_update_recheck_id env =
  let recheck_id = new_serve_iteration_id () in
  let env =
    {
      env with
      ServerEnv.init_env =
        { env.ServerEnv.init_env with ServerEnv.recheck_id = Some recheck_id };
    }
  in
  (env, recheck_id)

let idle_if_no_client env waiting_client =
  match waiting_client with
  | ClientProvider.Select_nothing
  | ClientProvider.Select_exception _
  | ClientProvider.Not_selecting_hg_updating ->
    let {
      RecheckLoopStats.per_batch_telemetry;
      total_changed_files_count;
      total_rechecked_count;
      _;
    } =
      env.last_recheck_loop_stats
    in
    (* Ugly hack: We want GC_SHAREDMEM_RAN to record the last rechecked
     * count so that we can figure out if the largest reclamations
     * correspond to massive rebases. However, the logging call is done in
     * the SharedMem module, which doesn't know anything about Server stuff.
     * So we wrap the call here. *)
    HackEventLogger.with_rechecked_stats
      ~update_batch_count:(List.length per_batch_telemetry)
      ~total_changed_files:total_changed_files_count
      ~total_rechecked:total_rechecked_count
      (fun () -> SharedMem.GC.collect `aggressive);
    let t = Unix.gettimeofday () in
    if Float.(t -. env.last_idle_job_time > 0.5) then
      let env = ServerIdle.go env in
      { env with last_idle_job_time = t }
    else
      env
  | ClientProvider.Select_new _
  | ClientProvider.Select_persistent ->
    env

(** Push diagnostics (typechecker errors in the IDE are called diagnostics) to IDE.
    Return a reason why nothing was pushed and optionally the timestamp of the push. *)
let push_diagnostics env : env * string * seconds_since_epoch option =
  let (diagnostic_pusher, time_errors_pushed) =
    Diagnostic_pusher.push_whats_left env.diagnostic_pusher
  in
  let env = { env with diagnostic_pusher } in
  (env, "pushed any leftover", time_errors_pushed)

let log_recheck_end (stats : ServerEnv.RecheckLoopStats.t) ~errors ~diag_reason
    =
  let telemetry =
    ServerEnv.RecheckLoopStats.to_user_telemetry stats
    |> Telemetry.string_ ~key:"diag_reason" ~value:diag_reason
    |> Telemetry.object_ ~key:"errors" ~value:(Errors.as_telemetry errors)
  in
  let {
    RecheckLoopStats.duration;
    total_changed_files_count;
    total_rechecked_count;
    per_batch_telemetry;
    any_full_checks;
    recheck_id;
    updates_stale = _;
    last_iteration_start_time = _;
    time_first_result = _;
  } =
    stats
  in
  HackEventLogger.recheck_end
    ~last_recheck_duration:duration
    ~update_batch_count:(List.length per_batch_telemetry - 1)
    ~total_changed_files:total_changed_files_count
    ~total_rechecked:total_rechecked_count
    (Option.some_if any_full_checks telemetry);
  Hh_logger.log
    "RECHECK_END (recheck_id %s):\n%s"
    recheck_id
    (Telemetry.to_string telemetry);
  ()

let exit_if_parent_dead () =
  (* Cross-platform compatible way; parent PID becomes 1 when parent dies. *)
  if Unix.getppid () = 1 then (
    Hh_logger.log "Server's parent has died; exiting.\n";
    Exit.exit Exit_status.Lost_parent_monitor
  )

let serve_one_iteration genv env client_provider =
  let (env, recheck_id) = generate_and_update_recheck_id env in
  exit_if_parent_dead ();
  let acceptable_new_client_kind =
    let has_default_client_pending =
      Option.is_some env.nonpersistent_client_pending_command_needs_full_check
    in
    let use_tracker_v2 =
      genv.local_config.ServerLocalConfig.use_server_revision_tracker_v2
    in
    let can_accept_clients =
      not @@ ServerRevisionTracker.is_hg_updating use_tracker_v2
    in
    match (can_accept_clients, has_default_client_pending) with
    (* If we are already blocked on some client, do not accept more of them.
     * Other clients (that connect through priority pipe, or persistent clients)
     * can still be handled - unless we are in hg.update state, where we want to
     * stop accepting any new clients, with the exception of forced ones. *)
    | (true, true) -> Some `Priority
    | (true, false) -> Some `Any
    | (false, true) -> None
    | (false, false) -> Some `Force_dormant_start_only
  in
  let selected_client =
    match acceptable_new_client_kind with
    | None -> ClientProvider.Not_selecting_hg_updating
    | Some client_kind ->
      ClientProvider.sleep_and_check
        client_provider
        (Ide_info_store.get_client ())
        ~ide_idle:env.ide_idle
        ~idle_gc_slice:genv.local_config.ServerLocalConfig.idle_gc_slice
        client_kind
  in

  (* ServerProgress: By updating status now at the start of the serve_one_iteration,
   * it means there's no obligation on the "doing work" part of the previous
   * iteration to clean up its own status-reporting once done.
   *
   * Caveat: that's not quite true, since ClientProvider.sleep_and_check will
   * wait up to 0.1s if there are no pending requests. So theoretically we
   * won't update our status for up to 0.1s after the previous work is done.
   * That doesn't really matter, since (1) if there are no pending requests
   * then no client will even ask for status, and (2) it's worth it to
   * keep the code clean and simple.
   *
   * By the same token, we will be writing "ready" once every 0.1s to the status file.
   * Think of it as a heartbeat!
   *
   * Note: the message here might soon be replaced. If we discover disk changes
   * that prompt a typecheck, then typechecking sends its own status updates.
   * And if the selected_client was a request, then once we discover the nature
   * of that request then ServerCommand.handle will send its own status updates too.
   *)
  begin
    match selected_client with
    | ClientProvider.(Select_nothing | Select_exception _) when not env.ide_idle
      ->
      ServerProgress.write ~include_in_logs:false "hh_client:active"
    | ClientProvider.(Select_nothing | Select_exception _) ->
      (* There's some subtle IDE behavior, described in [ServerCommand.handle]
         and [ServerMain.recheck_until_no_changes_left]... If an EDIT was received
         over the persistent connection, then we won't resume typechecking
         until either a file-save comes in or 5.0s has elapsed. *)
      let (disposition, msg) =
        match env.full_check_status with
        | Full_check_needed -> (ServerProgress.DWorking, "will resume")
        | Full_check_started -> (ServerProgress.DWorking, "typechecking")
        | Full_check_done -> (ServerProgress.DReady, "ready")
      in
      ServerProgress.write ~include_in_logs:false ~disposition "%s" msg
    | ClientProvider.Not_selecting_hg_updating ->
      ServerProgress.write ~include_in_logs:false "hg-transaction"
    | ClientProvider.Select_new _
    | ClientProvider.Select_persistent ->
      ServerProgress.write ~include_in_logs:false "working"
  end;
  let env = idle_if_no_client env selected_client in
  let stage =
    if Option.is_some env.init_env.why_needed_full_check then
      `Init
    else
      `Recheck
  in
  HackEventLogger.with_id ~stage recheck_id @@ fun () ->
  (* We'll first do "recheck_until_no_changes_left" to handle all outstanding changes, so that
   * after that we'll be able to give an up-to-date answer to the client.
   * Except: this might be stopped early in some cases, e.g. IDE checks. *)
  let t_start_recheck = Unix.gettimeofday () in
  let (stats, env) =
    recheck_until_no_changes_left
      (RecheckLoopStats.empty ~recheck_id)
      genv
      env
      selected_client
  in
  let t_done_recheck = Unix.gettimeofday () in
  let (env, diag_reason, time_errors_pushed) = push_diagnostics env in
  let t_sent_diagnostics = Unix.gettimeofday () in
  let stats =
    ServerEnv.RecheckLoopStats.record_result_sent_ts stats time_errors_pushed
  in
  let did_work = stats.RecheckLoopStats.total_rechecked_count > 0 in
  let env =
    {
      env with
      last_recheck_loop_stats = stats;
      last_recheck_loop_stats_for_actual_work =
        (if did_work then
          Some stats
        else
          env.last_recheck_loop_stats_for_actual_work);
    }
  in

  if did_work then log_recheck_end stats ~errors:env.errorl ~diag_reason;

  let env =
    match selected_client with
    | ClientProvider.Select_persistent
    | ClientProvider.Select_nothing
    | ClientProvider.Select_exception _
    | ClientProvider.Not_selecting_hg_updating ->
      env
    | ClientProvider.Select_new { ClientProvider.client; m2s_sequence_number }
      -> begin
      try
        Hh_logger.log
          "Serving new client obtained from monitor handoff #%d"
          m2s_sequence_number;
        (* client here is the new client (not the existing persistent client)
         * whose request we're going to handle. *)
        ClientProvider.track
          client
          ~key:Connection_tracker.Server_start_recheck
          ~time:t_start_recheck;
        ClientProvider.track
          client
          ~key:Connection_tracker.Server_done_recheck
          ~time:t_done_recheck;
        ClientProvider.track
          client
          ~key:Connection_tracker.Server_sent_diagnostics
          ~time:t_sent_diagnostics;
        let env =
          Client_command_handler.handle_client_command_or_persistent_connection
            genv
            env
            client
            `Non_persistent
          |> main_loop_command_handler `Non_persistent client
        in
        HackEventLogger.handled_connection t_start_recheck;
        env
      with
      | exn ->
        let e = Exception.wrap exn in
        HackEventLogger.handle_connection_exception "outer" e;
        Hh_logger.log
          "HANDLE_CONNECTION_EXCEPTION(outer) [ignoring request] %s"
          (Exception.to_string e);
        env
    end
  in
  let env =
    match Ide_info_store.get_client () with
    | None -> env
    | Some client ->
      (* Test whether at the beginning of this iteration of main loop
       * there was a request to read and handle.
       * If yes, we'll try to do it, but it's possible that we have ran a recheck
       * in-between those two events, and if this recheck was non-blocking, we
       * might have already handled this command there. Proceeding to
       * handle_connection would then block reading a request that is not there
       * anymore, so we need to call has_persistent_connection_request again. *)
      if ClientProvider.has_persistent_connection_request client then (
        HackEventLogger.got_persistent_client_channels t_start_recheck;
        try
          let env =
            Client_command_handler
            .handle_client_command_or_persistent_connection
              genv
              env
              client
              `Persistent
            |> main_loop_command_handler `Persistent client
          in
          HackEventLogger.handled_persistent_connection t_start_recheck;
          env
        with
        | exn ->
          let e = Exception.wrap exn in
          HackEventLogger.handle_persistent_connection_exception
            "outer"
            e
            ~is_fatal:true;
          Hh_logger.log
            "HANDLE_PERSISTENT_CONNECTION_EXCEPTION(outer) [ignoring request] %s"
            (Exception.to_string e);
          env
      ) else
        env
  in
  let env =
    match env.pending_command_needs_writes with
    | Some f -> { (f env) with pending_command_needs_writes = None }
    | None -> env
  in
  let env =
    match env.persistent_client_pending_command_needs_full_check with
    | Some (f, _reason) when is_full_check_done env.full_check_status ->
      { (f env) with persistent_client_pending_command_needs_full_check = None }
    | _ -> env
  in
  let env =
    match env.nonpersistent_client_pending_command_needs_full_check with
    | Some (f, _reason, _client) when is_full_check_done env.full_check_status
      ->
      {
        (f env) with
        nonpersistent_client_pending_command_needs_full_check = None;
      }
    | _ -> env
  in
  env

(** This synthesizes a [MultiThreadedCall.Cancel] in the event that we want
a typecheck cancelled due to files changing on disk. It constructs the
human-readable [user_message] and also [log_message] appropriately. *)
let cancel_due_to_watchman
    (updates : Relative_path.Set.t) (clock : Watchman.clock option) :
    MultiThreadedCall.interrupt_result =
  assert (not (Relative_path.Set.is_empty updates));
  let size = Relative_path.Set.cardinal updates in
  let examples =
    (List.take (Relative_path.Set.elements updates) 5
    |> List.map ~f:Relative_path.suffix)
    @
    if size > 5 then
      ["..."]
    else
      []
  in
  let timestamp = Unix.gettimeofday () in
  let tm = Unix.localtime timestamp in
  MultiThreadedCall.Cancel
    {
      MultiThreadedCall.user_message =
        Printf.sprintf
          "Files have changed on disk! [%02d:%02d:%02d] %s"
          tm.Unix.tm_hour
          tm.Unix.tm_min
          tm.Unix.tm_sec
          (List.hd_exn examples);
      log_message =
        Printf.sprintf
          "watchman interrupt handler at clock %s. %d files changed. [%s]"
          (ServerEnv.show_clock clock)
          (Relative_path.Set.cardinal updates)
          (String.concat examples ~sep:",");
      timestamp;
    }

let watchman_interrupt_handler genv : env MultiThreadedCall.interrupt_handler =
 fun env ->
  let start_time = Unix.gettimeofday () in
  let (env, updates, clock, updates_stale, _telemetry) =
    query_notifier genv env `Async start_time
  in
  (* Async updates can always be stale, so we don't care *)
  ignore updates_stale;
  let size = Relative_path.Set.cardinal updates in
  if size > 0 then (
    Hh_logger.log
      "Interrupted by Watchman message: %d files changed at watchclock %s"
      size
      (ServerEnv.show_clock clock);
    ( {
        env with
        disk_needs_parsing =
          Relative_path.Set.union env.disk_needs_parsing updates;
        clock;
      },
      cancel_due_to_watchman updates clock )
  ) else
    (env, MultiThreadedCall.Continue)

(** Handler for events on the priority socket, which is used priority commands which
    must be served immediately. *)
let priority_client_interrupt_handler genv client_provider :
    env MultiThreadedCall.interrupt_handler =
 fun env ->
  let t = Unix.gettimeofday () in
  Hh_logger.log "Handling message on priority socket.";
  (* For non-persistent clients that don't synchronize file contents, users
   * expect that a query they do immediately after saving a file will reflect
   * this file contents. Async notifications are not always fast enough to
   * guarantee it, so we need an additional sync query before accepting such
   * client *)
  let (env, updates, clock, _updates_stale, _telemetry) =
    query_notifier genv env `Sync t
  in
  let size = Relative_path.Set.cardinal updates in
  if size > 0 then (
    Hh_logger.log
      "Interrupted by Watchman sync query: %d files changed at watchclock %s"
      size
      (ServerEnv.show_clock clock);
    ( {
        env with
        disk_needs_parsing =
          Relative_path.Set.union env.disk_needs_parsing updates;
        clock;
      },
      cancel_due_to_watchman updates clock )
  ) else
    let idle_gc_slice = genv.local_config.ServerLocalConfig.idle_gc_slice in
    let use_tracker_v2 =
      genv.local_config.ServerLocalConfig.use_server_revision_tracker_v2
    in
    let select_outcome =
      if ServerRevisionTracker.is_hg_updating use_tracker_v2 then (
        Hh_logger.log "Won't handle client message: hg is updating.";
        ClientProvider.Not_selecting_hg_updating
      ) else
        ClientProvider.sleep_and_check
          client_provider
          (Ide_info_store.get_client ())
          ~ide_idle:env.ide_idle
          ~idle_gc_slice
          `Priority
    in
    let env =
      match select_outcome with
      | ClientProvider.Select_persistent ->
        failwith "should only be looking at new priority clients"
      | ClientProvider.Select_nothing ->
        (* This is possible because client might have gone away during
         * sleep_and_check. *)
        Hh_logger.log "Client went away.";
        env
      | ClientProvider.Select_exception e ->
        Hh_logger.log
          "Exception during client FD select: %s"
          (Exception.get_ctor_string e);
        env
      | ClientProvider.Not_selecting_hg_updating ->
        Hh_logger.log "hg is updating.";
        env
      | ClientProvider.Select_new { ClientProvider.client; m2s_sequence_number }
        ->
        Hh_logger.log
          "Serving new client obtained from monitor handoff #%d"
          m2s_sequence_number;
        (match
           Client_command_handler.handle_client_command_or_persistent_connection
             genv
             env
             client
             `Non_persistent
         with
        | ServerUtils.Needs_full_recheck { reason; _ } ->
          failwith
            ("unexpected command needing full recheck in priority channel: "
            ^ reason)
        | ServerUtils.Needs_writes { reason; _ } ->
          failwith
            ("unexpected command needing writes in priority channel: " ^ reason)
        | ServerUtils.Done env -> env)
    in

    (* Global rechecks in response to file changes can be paused.
       Here, we check if the user requested global rechecks to be paused during
       the current recheck (the one that we're in the middle of). The above call
       to `handle_connection` could have resulted in this state change if
       the RPC was `PAUSE true`.

       If the state did change to `Paused` during the current recheck,
       we should cancel the current recheck.

       Note that `PAUSE false`, which resumes global rechecks in response to
       file changes, requires a full recheck by policy - see ServerCommand's
       `rpc_command_needs_full_check`. Commands that require a full recheck
       do not use `priority pipe`, so they don't end up handled here.
       Such commands don't interrupt MultiWorker calls, by design.

       The effect of `PAUSE true` during a recheck is that the recheck will be
       canceled, while the result of `PAUSE false` is that the client will wait
       for the recheck to be finished. *)
    let decision =
      match (env.full_recheck_on_file_changes, env.init_env.recheck_id) with
      | ( Paused { paused_recheck_id = Some paused_recheck_id; _ },
          Some recheck_id )
        when String.equal paused_recheck_id recheck_id ->
        MultiThreadedCall.Cancel
          {
            MultiThreadedCall.user_message = "Pause via 'hh --pause'";
            log_message = "";
            timestamp = Unix.gettimeofday ();
          }
      | _ -> MultiThreadedCall.Continue
    in
    (env, decision)

let persistent_client_interrupt_handler genv :
    env MultiThreadedCall.interrupt_handler =
 fun env ->
  Hh_logger.info "Handling message on persistent client socket.";
  match Ide_info_store.get_client () with
  (* Several handlers can become ready simultaneously and one of them can remove
   * the persistent client before we get to it. *)
  | None -> (env, MultiThreadedCall.Continue)
  | Some client ->
    (match
       Client_command_handler.handle_client_command_or_persistent_connection
         genv
         env
         client
         `Persistent
     with
    | ServerUtils.Needs_full_recheck { env; finish_command_handling; reason } ->
      ServerProgress.write "typechecking";
      (* This should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one. *)
      assert (
        Option.is_none env.persistent_client_pending_command_needs_full_check);
      ( {
          env with
          persistent_client_pending_command_needs_full_check =
            Some (finish_command_handling, reason);
        },
        MultiThreadedCall.Continue )
    | ServerUtils.Needs_writes
        { env; finish_command_handling; recheck_restart_is_needed; reason } ->
      let full_check_status =
        match env.full_check_status with
        | Full_check_started when not recheck_restart_is_needed ->
          Full_check_needed
        | x -> x
      in
      (* this should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one *)
      assert (Option.is_none env.pending_command_needs_writes);
      ( {
          env with
          pending_command_needs_writes = Some finish_command_handling;
          full_check_status;
        },
        MultiThreadedCall.Cancel
          {
            MultiThreadedCall.user_message =
              Printf.sprintf
                "Interrupted [%s]\n(Sorry about this nuisance... we're working to fix it T92870399)"
                reason;
            log_message = "";
            timestamp = Unix.gettimeofday ();
          } )
    | ServerUtils.Done env ->
      ServerProgress.write "typechecking";
      (env, MultiThreadedCall.Continue))

let setup_interrupts env client_provider =
  {
    env with
    interrupt_handlers =
      (fun genv env ->
        let { ServerLocalConfig.interrupt_on_watchman; interrupt_on_client; _ }
            =
          genv.local_config
        in
        let handlers = [] in
        let handlers =
          let interrupt_on_watchman =
            interrupt_on_watchman && env.can_interrupt
          in
          match ServerNotifier.async_reader_opt genv.notifier with
          | Some reader when interrupt_on_watchman ->
            (Buffered_line_reader.get_fd reader, watchman_interrupt_handler genv)
            :: handlers
          | _ -> handlers
        in
        let handlers =
          let interrupt_on_client = interrupt_on_client && env.can_interrupt in
          match ClientProvider.priority_fd client_provider with
          | Some fd when interrupt_on_client ->
            (fd, priority_client_interrupt_handler genv client_provider)
            :: handlers
          | _ -> handlers
        in
        let handlers =
          match
            Ide_info_store.get_client () >>= ClientProvider.get_client_fd
          with
          | Some fd when interrupt_on_client ->
            (fd, persistent_client_interrupt_handler genv) :: handlers
          | _ -> handlers
        in
        handlers);
  }

let serve genv env in_fds =
  if genv.local_config.ServerLocalConfig.ide_parser_cache then
    Ide_parser_cache.enable ();
  (* During server lifetime dependency table can be not up-to-date. Because of
   * that, we ban access to it be default, forcing the code trying to read it to
   * take it into account, either by explcitely enabling reads (and being fine
   * with stale results), or declaring (in ServerCommand) that it requires full
   * check to be completed before being executed. *)
  let (_ : bool) =
    Typing_deps.allow_dependency_table_reads env.deps_mode false
  in
  let () = Errors.set_allow_errors_in_default_path false in
  MultiThreadedCall.on_exception (fun e -> ServerUtils.exit_on_exception e);
  let client_provider = ClientProvider.provider_from_file_descriptors in_fds in

  (* This is needed when typecheck_after_init option is disabled.
   * We're just filling it with placeholder telemetry values since
   * we don't much care about this scenario. *)
  let init_telemetry =
    ServerEnv.Init_telemetry.make
      ServerEnv.Init_telemetry.Init_typecheck_disabled_after_init
      (Telemetry.create ()
      |> Telemetry.string_
           ~key:"mode"
           ~value:"serve_due_to_disabled_typecheck_after_init")
  in
  let typecheck_telemetry = Telemetry.create () in
  if Option.is_none env.init_env.why_needed_full_check then
    finalize_init env.init_env typecheck_telemetry init_telemetry;

  let env = setup_interrupts env client_provider in
  let env = ref env in
  while true do
    let new_env = serve_one_iteration genv !env client_provider in
    env := new_env
  done

(* Rules for whether+how to load saved-state...
 * 1. If hh.conf lacks "use_mini_state = true", then don't load it.
 * 2. If hh_server --no-load, then don't load it.
 * 3. If hh_server --save-mini or -s, then save but don't load it.
 * 4. If "hh_server --with-mini-state", then load the one specified there!
 * 5. If hh.conf lacks "load_state_natively_v4", then don't load it
 * 6. Otherwise, load it normally!
 *)
let resolve_init_approach genv : ServerInit.init_approach * string =
  let nonce = genv.local_config.ServerLocalConfig.remote_nonce in
  match
    ( genv.local_config.ServerLocalConfig.remote_worker_key,
      genv.local_config.ServerLocalConfig.remote_check_id )
  with
  | (Some worker_key, Some check_id) ->
    let remote_init = ServerInit.{ worker_key; nonce; check_id } in
    (ServerInit.Remote_init remote_init, "Server_args_remote_worker")
  | (Some worker_key, None) ->
    let check_id = Random_id.short_string () in
    let remote_init = ServerInit.{ worker_key; nonce; check_id } in
    (ServerInit.Remote_init remote_init, "Server_args_remote_worker")
  | (None, Some check_id) ->
    failwith
      (Printf.sprintf
         "Remote check ID is specified (%s), but the remote worker ID is not"
         check_id)
  | (None, None) ->
    if
      Option.is_some (ServerArgs.save_naming_filename genv.options)
      && Option.is_none (ServerArgs.save_filename genv.options)
    then
      (ServerInit.Parse_only_init, "Server_args_saving_naming")
    else if ServerArgs.no_load genv.options then
      (ServerInit.Full_init, "Server_args_no_load")
    else if Option.is_some (ServerArgs.save_filename genv.options) then
      (ServerInit.Full_init, "Server_args_saving_state")
    else if
      (not genv.local_config.ServerLocalConfig.use_saved_state)
      && Option.is_none (ServerArgs.write_symbol_info genv.options)
    then
      (ServerInit.Full_init, "Local_config_saved_state_disabled")
    else if Option.is_some (ServerArgs.write_symbol_info genv.options) then
      match
        ( genv.local_config.ServerLocalConfig.use_saved_state_when_indexing,
          ServerArgs.with_saved_state genv.options )
      with
      | (false, None) ->
        (ServerInit.Write_symbol_info, "Server_args_writing_symbol_info")
      | (true, None) ->
        ( ServerInit.Write_symbol_info_with_state ServerInit.Load_state_natively,
          "Server_args_writing_symbol_info_load_native" )
      | (_, Some (ServerArgs.Saved_state_target_info target)) ->
        ( ServerInit.Write_symbol_info_with_state (ServerInit.Precomputed target),
          "Server_args_writing_symbol_info_precomputed" )
    else (
      match
        ( genv.local_config.ServerLocalConfig.load_state_natively,
          ServerArgs.with_saved_state genv.options )
      with
      | (_, Some (ServerArgs.Saved_state_target_info target)) ->
        ( ServerInit.Saved_state_init (ServerInit.Precomputed target),
          "Precomputed" )
      | (false, None) ->
        (ServerInit.Full_init, "No_native_loading_or_precomputed")
      | (true, None) ->
        (* Use native loading only if the config specifies a load script,
         * and the local config prefers native. *)
        ( ServerInit.Saved_state_init ServerInit.Load_state_natively,
          "Load_state_natively" )
    )

let program_init genv env =
  Hh_logger.log "Init id: %s" env.init_env.init_id;
  ServerProgress.write "initializing...";
  ServerProgress.enable_error_production
    genv.local_config.ServerLocalConfig.produce_streaming_errors;
  Exit.add_hook_upon_clean_exit (fun _finale_data ->
      ServerProgress.ErrorsWrite.unlink_at_server_stop ());
  let env =
    {
      env with
      init_env =
        { env.init_env with ci_info = Some (Ci_util.begin_get_info ()) };
    }
  in
  let (init_approach, approach_name) = resolve_init_approach genv in
  Hh_logger.log "Initing with approach: %s" approach_name;
  let (env, init_type, init_error, init_error_telemetry, saved_state_delta) =
    let (env, init_result) = ServerInit.init ~init_approach genv env in
    match init_approach with
    | ServerInit.Remote_init _ -> (env, "remote", None, None, None)
    | ServerInit.Write_symbol_info
    | ServerInit.Full_init ->
      (env, "fresh", None, None, None)
    | ServerInit.Parse_only_init -> (env, "parse-only", None, None, None)
    | ServerInit.Write_symbol_info_with_state _
    | ServerInit.Saved_state_init _ -> begin
      match init_result with
      | ServerInit.Load_state_succeeded saved_state_delta ->
        let init_type =
          match
            Naming_table.get_forward_naming_fallback_path env.naming_table
          with
          | None -> "state_load_blob"
          | Some _ -> "state_load_sqlite"
        in
        (env, init_type, None, None, saved_state_delta)
      | ServerInit.Load_state_failed (err, telemetry) ->
        (env, "state_load_failed", Some err, Some telemetry, None)
      | ServerInit.Load_state_declined reason ->
        (env, "state_load_declined", Some reason, None, None)
    end
  in
  let env =
    {
      env with
      init_env =
        {
          env.init_env with
          saved_state_delta;
          approach_name;
          init_error;
          init_type;
        };
    }
  in
  Hh_logger.log "Waiting for daemon(s) to be ready...";
  ServerProgress.write "wrapping up init...";
  ServerNotifier.wait_until_ready genv.notifier;
  ServerStamp.touch_stamp ();
  EventLogger.set_init_type init_type;
  let telemetry =
    ServerUtils.log_and_get_sharedmem_load_telemetry ()
    |> Telemetry.object_opt ~key:"init_error" ~value:init_error_telemetry
    |> Telemetry.json_
         ~key:"deps_mode"
         ~value:(Typing_deps_mode.to_opaque_json env.deps_mode)
  in
  HackEventLogger.init_lazy_end telemetry ~approach_name ~init_error ~init_type;
  env

let num_workers options local_config =
  (* The number of workers is set both in hh.conf and as an optional server argument.
     if the two numbers given in argument and in hh.conf are different, we always take the minimum
     of the two.
  *)
  let max_procs_opt =
    Option.merge
      ~f:(fun a b ->
        if Int.equal a b then
          a
        else (
          Hh_logger.log
            ("Warning: both an argument --max-procs and a local config "
            ^^ "for max workers are given. Choosing minimum of the two.");
          min a b
        ))
      (ServerArgs.max_procs options)
      local_config.ServerLocalConfig.max_workers
  in
  let nbr_procs = Sys_utils.nbr_procs in
  match max_procs_opt with
  | None -> nbr_procs
  | Some max_procs ->
    if max_procs <= nbr_procs then
      max_procs
    else (
      Hh_logger.log
        "Warning: max workers is higher than the number of processors. Ignoring.";
      nbr_procs
    )

(* The hardware we are running on are Intel Skylake and Haswell family
   processors with 80, 56, or 48 cores.  Turns out that there are only 1/2
   actual CPUs, the rest are hyperthreads. Using worker processes for
   hyperthreads is slower than using just the number of actual computation
   cores. *)
let modify_worker_count hack_worker_count =
  let n_procs = Sys_utils.nbr_procs in
  let workers =
    if hack_worker_count < n_procs then
      (* Already limited, use what we have *)
      hack_worker_count
    else
      (* Use half. *)
      max 1 (n_procs / 2)
  in
  workers

let setup_server ~informant_managed ~monitor_pid options config local_config =
  let num_workers = num_workers options local_config |> modify_worker_count in
  let handle =
    SharedMem.init ~num_workers (ServerConfig.sharedmem_config config)
  in
  let init_id = Random_id.short_string () in
  let pid = Unix.getpid () in

  (* There are three files which are used for IPC.
     1. server_finale_file - we unlink it now upon startup,
        and upon clean exit we'll write finale-date to it.
     2. server_receipt_to_monitor_file - we'll unlink it now upon startup,
        and upon clean exit we'll unlink it.
     3. server_progress_file - we write "starting up" to it now upon startup,
        and upon clean exit we'll write "shutting down" to it.
     In both case of clean exit and abrupt exit there'll be leftover files.
     We'll rely upon tmpclean to eventually clean them up. *)
  ServerProgress.set_root (ServerArgs.root options);
  let server_finale_file = ServerFiles.server_finale_file pid in
  let server_receipt_to_monitor_file =
    ServerFiles.server_receipt_to_monitor_file pid
  in
  (try Unix.unlink server_finale_file with
  | _ -> ());
  (try Unix.unlink server_receipt_to_monitor_file with
  | _ -> ());
  ServerProgress.write "startuping up";
  Exit.add_hook_upon_clean_exit (fun finale_data ->
      begin
        try Unix.unlink server_receipt_to_monitor_file with
        | _ -> ()
      end;
      begin
        try
          Sys_utils.with_umask 0o000 (fun () ->
              let oc = Stdlib.open_out_bin server_finale_file in
              Marshal.to_channel oc finale_data [];
              Stdlib.close_out oc)
        with
        | _ -> ()
      end;
      begin
        try ServerProgress.write "shutting down" with
        | _ -> ()
      end;
      ());

  Hh_logger.log "Version: %s" Hh_version.version;
  Hh_logger.log "Hostname: %s" (Unix.gethostname ());
  let root = ServerArgs.root options in

  let deps_mode =
    match ServerArgs.save_64bit options with
    | Some new_edges_dir ->
      let human_readable_dep_map_dir =
        ServerArgs.save_human_readable_64bit_dep_map options
      in
      Typing_deps_mode.SaveToDiskMode
        { graph = None; new_edges_dir; human_readable_dep_map_dir }
    | None -> Typing_deps_mode.InMemoryMode None
  in

  (* The OCaml default is 500, but we care about minimizing the memory
   * overhead *)
  let gc_control = Caml.Gc.get () in
  Caml.Gc.set { gc_control with Caml.Gc.max_overhead = 200 };
  let { ServerLocalConfig.cpu_priority; io_priority; enable_on_nfs; _ } =
    local_config
  in
  let hhconfig_version =
    config |> ServerConfig.version |> Config_file.version_to_string_opt
  in
  List.iter (ServerConfig.ignored_paths config) ~f:FilesToIgnore.ignore_path;
  let logging_init init_id ~is_worker =
    Hh_logger.Level.set_min_level local_config.ServerLocalConfig.min_log_level;
    Hh_logger.Level.set_categories local_config.ServerLocalConfig.log_categories;

    if not (Sys_utils.enable_telemetry ()) then
      EventLogger.init_fake ()
    else if is_worker then
      HackEventLogger.init_worker
        ~root
        ~hhconfig_version
        ~init_id
        ~custom_columns:(ServerArgs.custom_telemetry_data options)
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags local_config)
        ~rollout_group:local_config.ServerLocalConfig.rollout_group
        ~time:(Unix.gettimeofday ())
        ~per_file_profiling:local_config.ServerLocalConfig.per_file_profiling
    else
      HackEventLogger.init
        ~root
        ~hhconfig_version
        ~init_id
        ~custom_columns:(ServerArgs.custom_telemetry_data options)
        ~informant_managed
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags local_config)
        ~rollout_group:local_config.ServerLocalConfig.rollout_group
        ~time:(Unix.gettimeofday ())
        ~max_workers:num_workers
        ~per_file_profiling:local_config.ServerLocalConfig.per_file_profiling
  in
  logging_init init_id ~is_worker:false;
  HackEventLogger.init_start
    ~experiments_config_meta:
      local_config.ServerLocalConfig.experiments_config_meta
    (Memory_stats.get_host_hw_telemetry ());
  let root_s = Path.to_string root in
  let check_mode = ServerArgs.check_mode options in
  if (not check_mode) && Sys_utils.is_nfs root_s && not enable_on_nfs then (
    Hh_logger.log "Refusing to run on %s: root is on NFS!" root_s;
    HackEventLogger.nfs_root ();
    Exit.exit Exit_status.Nfs_root
  );

  if
    ServerConfig.warn_on_non_opt_build config && not Build_id.is_build_optimized
  then begin
    let msg =
      Printf.sprintf
        "hh_server binary was built in \"%s\" mode, "
        Build_id.build_mode
      ^ "is running with Rust version of parser enabled, "
      ^ "and this repository's .hhconfig specifies warn_on_non_opt_build option. "
      ^ "Parsing with non-opt build will take significantly longer"
    in
    if ServerArgs.allow_non_opt_build options then
      Hh_logger.log
        "Warning: %s. Initializing anyway due to --allow-non-opt-build option."
        msg
    else
      let msg =
        Printf.sprintf
          "Error: %s. Recompile the server in opt or dbgo mode, or pass --allow-non-opt-build to continue anyway."
          msg
      in
      Hh_logger.log "%s" msg;
      Exit.exit ~msg Exit_status.Server_non_opt_build_mode
  end;

  Program.preinit ();
  Sys_utils.set_priorities ~cpu_priority ~io_priority;

  (* this is to transform SIGPIPE in an exception. A SIGPIPE can happen when
   * someone C-c the client.
   *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  PidLog.init (ServerFiles.pids_file root);
  Option.iter monitor_pid ~f:(fun monitor_pid ->
      PidLog.log ~reason:"monitor" monitor_pid);
  PidLog.log ~reason:"main" (Unix.getpid ());

  (* Make a sub-init_id because we use it to name temporary files for piping to
     scuba logging processes. *)
  let worker_logging_init () =
    logging_init (init_id ^ "." ^ Random_id.short_string ()) ~is_worker:true
  in
  let gc_control = ServerConfig.gc_control config in
  let workers =
    ServerWorker.make
      ~longlived_workers:local_config.ServerLocalConfig.longlived_workers
      ~nbr_procs:num_workers
      gc_control
      handle
      ~logging_init:worker_logging_init
  in
  (workers, ServerEnvBuild.make_env config ~init_id ~deps_mode)

let run_once options config local_config =
  assert (ServerArgs.check_mode options);
  let (workers, env) =
    setup_server
      options
      config
      local_config
      ~informant_managed:false
      ~monitor_pid:None
  in
  let genv = ServerEnvBuild.make_genv options config local_config workers in

  (* The type-checking happens here *)
  let env = program_init genv env in
  (* All of saving state happens here *)
  let (env, save_state_results) =
    match ServerArgs.save_filename genv.options with
    | None -> (env, None)
    | Some filename -> (env, ServerInit.save_state genv env filename)
  in
  let _naming_table_rows_changed =
    match ServerArgs.save_naming_filename genv.options with
    | None -> None
    | Some filename ->
      Disk.mkdir_p (Filename.dirname filename);
      let save_result = Naming_table.save env.naming_table filename in
      Hh_logger.log
        "Inserted symbols into the naming table:\n%s"
        (Naming_sqlite.show_save_result save_result);
      if List.length save_result.Naming_sqlite.errors > 0 then begin
        Sys_utils.rm_dir_tree filename;
        failwith "Naming table state had errors - deleting output file!"
      end else
        Some save_result
  in
  (* Finish up by generating the output and the exit code *)
  match ServerArgs.concatenate_prefix genv.options with
  | Some prefix ->
    let prefix =
      Relative_path.from_root ~suffix:prefix |> Relative_path.to_absolute
    in
    let text = ServerConcatenateAll.go genv env [prefix] in
    print_endline text;
    Exit.exit Exit_status.No_error
  | _ ->
    Hh_logger.log "Running in check mode";
    Program.run_once_and_exit genv env save_state_results

(*
 * The server monitor will pass client connections to this process
 * via ic.
 *)
let daemon_main_exn ~informant_managed options monitor_pid in_fds =
  assert (not (ServerArgs.check_mode options));
  Folly.ensure_folly_init ();
  Printexc.record_backtrace true;

  let (config, local_config) = ServerConfig.load ~silent:false options in
  Option.iter local_config.ServerLocalConfig.memtrace_dir ~f:(fun dir ->
      Daemon.start_memtracing (Filename.concat dir "memtrace.server.ctf"));
  let (workers, env) =
    setup_server
      options
      config
      local_config
      ~informant_managed
      ~monitor_pid:(Some monitor_pid)
  in
  let genv = ServerEnvBuild.make_genv options config local_config workers in
  HackEventLogger.with_id ~stage:`Init env.init_env.init_id @@ fun () ->
  let env = MainInit.go genv options (fun () -> program_init genv env) in
  serve genv env in_fds

type params = {
  informant_managed: bool;
  state: ServerGlobalState.t;
  options: ServerArgs.options;
  monitor_pid: int;
  priority_in_fd: Unix.file_descr;
  force_dormant_start_only_in_fd: Unix.file_descr;
}

let daemon_main
    {
      informant_managed;
      state;
      options;
      monitor_pid;
      priority_in_fd;
      force_dormant_start_only_in_fd;
    }
    (default_ic, _default_oc) =
  (* Avoid leaking this fd further *)
  let () = Unix.set_close_on_exec priority_in_fd in
  let () = Unix.set_close_on_exec force_dormant_start_only_in_fd in
  let default_in_fd = Daemon.descr_of_in_channel default_ic in

  (* Restore the root directory and other global states from monitor *)
  ServerGlobalState.restore state ~worker_id:0;

  (match ServerArgs.custom_hhi_path options with
  | None ->
    (* Restore hhi files every time the server restarts
       in case the tmp folder changes *)
    ignore (Hhi.get_hhi_root ())
  | Some path ->
    if Disk.file_exists path && Disk.is_directory path then (
      Hh_logger.log "Custom hhi directory set to %s." path;
      Hhi.set_custom_hhi_root (Path.make path)
    ) else (
      Hh_logger.log "Custom hhi directory %s not found." path;
      Exit.exit Exit_status.Input_error
    ));

  ServerUtils.with_exit_on_exception @@ fun () ->
  daemon_main_exn
    ~informant_managed
    options
    monitor_pid
    (default_in_fd, priority_in_fd, force_dormant_start_only_in_fd)

let entry = Daemon.register_entry_point "ServerMain.daemon_main" daemon_main
