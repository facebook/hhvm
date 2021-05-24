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
open String_utils
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
        ~f:ServerEnv.recheck_loop_stats_to_user_telemetry
        env.ServerEnv.last_recheck_loop_stats_for_actual_work
    in
    ServerError.print_error_list
      stdout
      ~stale_msg:None
      ~output_json:(ServerArgs.json_mode genv.options)
      ~error_list:
        (List.map (Errors.get_error_list env.errorl) Errors.to_absolute)
      ~save_state_result
      ~recheck_stats;

    WorkerController.force_quit_all ();

    (* as Warnings shouldn't break CI, don't change the exit status except for Errors *)
    let has_errors =
      List.exists
        ~f:(fun e ->
          match Errors.get_severity e with
          | Errors.Error -> true
          | _ -> false)
        (Errors.get_error_list env.errorl)
    in
    let is_saving_state_and_ignoring_errors =
      ServerArgs.gen_saved_ignore_type_errors genv.options
      && Option.is_some (ServerArgs.save_filename genv.options)
    in
    exit
      ( if has_errors && not is_saving_state_and_ignoring_errors then
        1
      else
        0 )

  (* filter and relativize updated file paths *)
  let process_updates genv updates =
    let root = Path.to_string @@ ServerArgs.root genv.options in
    (* Because of symlinks, we can have updates from files that aren't in
     * the .hhconfig directory *)
    let updates = SSet.filter updates (fun p -> string_starts_with p root) in
    let updates = Relative_path.(relativize_set Root updates) in
    let to_recheck =
      Relative_path.Set.filter updates (fun update ->
          FindUtils.file_filter (Relative_path.to_absolute update))
    in
    let config_in_updates =
      Relative_path.Set.mem updates ServerConfig.filename
    in
    ( if config_in_updates then
      let (new_config, _) =
        ServerConfig.(load ~silent:false filename genv.options)
      in
      if not (ServerConfig.is_compatible genv.config new_config) then (
        Hh_logger.log
          "%s changed in an incompatible way; please restart %s.\n"
          (Relative_path.suffix ServerConfig.filename)
          GlobalConfig.program_name;

        (* TODO: Notify the server monitor directly about this. *)
        Exit.exit Exit_status.Hhconfig_changed
      ) );
    to_recheck
end

let finalize_init init_env typecheck_telemetry init_telemetry =
  ServerProgress.send_warning None;
  (* rest is just logging/telemetry *)
  let t' = Unix.gettimeofday () in
  let heap_size = SharedMem.heap_size () in
  let hash_telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.duration ~start_time:init_env.init_start_t
    |> Telemetry.object_ ~key:"init" ~value:init_telemetry
    |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
    |> Telemetry.object_ ~key:"hash" ~value:hash_telemetry
    |> Telemetry.int_ ~key:"heap_size" ~value:heap_size
  in
  HackEventLogger.server_is_ready telemetry;
  Hh_logger.log
    "SERVER_IS_READY. Heap size: %d. Took %f seconds to init. Telemetry:\n%s"
    heap_size
    (t' -. init_env.init_start_t)
    (Telemetry.to_string telemetry);
  ()

let shutdown_persistent_client client env =
  ClientProvider.shutdown_client client;
  let env =
    {
      env with
      pending_command_needs_writes = None;
      persistent_client_pending_command_needs_full_check = None;
    }
  in
  ServerFileSync.clear_sync_data env

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

[@@@warning "-52"]

(* we have no alternative but to depend on Sys_error strings *)

let handle_connection_exception
    ~(env : ServerEnv.env) ~(client : ClientProvider.client) (e : Exception.t) :
    ServerEnv.env =
  match Exception.to_exn e with
  | ClientProvider.Client_went_away
  | ServerCommandTypes.Read_command_timeout ->
    ClientProvider.shutdown_client client;
    env
  (* Connection dropped off. Its unforunate that we don't actually know
   * which connection went bad (could be any write to any connection to
   * child processes/daemons), we just assume at this top-level that
   * since its not caught elsewhere, its the connection to the client.
   *
   * TODO: Make sure the pipe exception is really about this client.*)
  | Unix.Unix_error (Unix.EPIPE, _, _)
  | Sys_error "Broken pipe"
  | Sys_error "Connection reset by peer" ->
    Hh_logger.log "Client channel went bad. Shutting down client connection";
    ClientProvider.shutdown_client client;
    env
  | exn ->
    let e = Exception.wrap exn in
    HackEventLogger.handle_connection_exception "inner" e;
    Hh_logger.log
      "HANDLE_CONNECTION_EXCEPTION(inner) %s"
      (Exception.to_string e);
    ClientProvider.shutdown_client client;
    env

[@@@warning "+52"]

(* CARE! scope of suppression should be only handle_connection_exception *)

(* f represents a non-persistent command coming from client. If executing f
 * throws, we need to dispopose of this client (possibly recovering updated
 * environment from Nonfatal_rpc_exception). "return" is a constructor
 * wrapping the return value to make it match return type of f *)
let handle_connection_try return client env f =
  try f () with
  | ServerCommand.Nonfatal_rpc_exception (e, env) ->
    return (handle_connection_exception ~env ~client e)
  | exn ->
    let e = Exception.wrap exn in
    return (handle_connection_exception ~env ~client e)

let handle_connection_ genv env client =
  ClientProvider.track
    client
    ~key:Connection_tracker.Server_start_handle_connection;
  handle_connection_try (fun x -> ServerUtils.Done x) client env @@ fun () ->
  match ClientProvider.read_connection_type client with
  | ServerCommandTypes.Persistent ->
    let f env =
      let env =
        match env.persistent_client with
        | Some old_client ->
          ClientProvider.send_push_message_to_client
            old_client
            ServerCommandTypes.NEW_CLIENT_CONNECTED;
          shutdown_persistent_client old_client env
        | None -> env
      in
      ClientProvider.track client ~key:Connection_tracker.Server_start_handle;
      ClientProvider.send_response_to_client client ServerCommandTypes.Connected;
      let env =
        {
          env with
          persistent_client = Some (ClientProvider.make_persistent client);
        }
      in
      (* If the client connected in the middle of recheck, let them know it's
       * happening. *)
      if is_full_check_started env.full_check_status then
        ServerBusyStatus.send
          env
          (ServerCommandTypes.Doing_global_typecheck
             (ServerCheckUtils.global_typecheck_kind genv env));
      env
    in
    if
      Option.is_some env.persistent_client
      (* Cleaning up after existing client (in shutdown_persistent_client)
       * will attempt to write to shared memory *)
    then
      ServerUtils.Needs_writes (env, f, true, "Cleaning up persistent client")
    else
      ServerUtils.Done (f env)
  | ServerCommandTypes.Non_persistent -> ServerCommand.handle genv env client

let handle_persistent_connection_exception
    ~(client : ClientProvider.client) ~(is_fatal : bool) (e : Exception.t) :
    unit =
  let open Marshal_tools in
  let remote_e =
    {
      message = Exception.get_ctor_string e;
      stack = Exception.get_backtrace_string e |> Exception.clean_stack;
    }
  in
  let push =
    if is_fatal then
      ServerCommandTypes.FATAL_EXCEPTION remote_e
    else
      ServerCommandTypes.NONFATAL_EXCEPTION remote_e
  in
  begin
    try ClientProvider.send_push_message_to_client client push with _ -> ()
  end;
  HackEventLogger.handle_persistent_connection_exception "inner" ~is_fatal e;
  Hh_logger.error
    "HANDLE_PERSISTENT_CONNECTION_EXCEPTION(inner) %s"
    (Exception.to_string e);
  ()

(* Same as handle_connection_try, but for persistent clients *)
[@@@warning "-52"]

(* we have no alternative but to depend on Sys_error strings *)

let handle_persistent_connection_try return client env f =
  try f () with
  (* TODO: Make sure the pipe exception is really about this client. *)
  | Unix.Unix_error (Unix.EPIPE, _, _)
  | Sys_error "Connection reset by peer"
  | Sys_error "Broken pipe"
  | ServerCommandTypes.Read_command_timeout
  | ServerClientProvider.Client_went_away ->
    return
      env
      (shutdown_persistent_client client)
      ~needs_writes:(Some "Client_went_away")
  | ServerCommand.Nonfatal_rpc_exception (e, env) ->
    handle_persistent_connection_exception ~client ~is_fatal:false e;
    return env (fun env -> env) ~needs_writes:None
  | exn ->
    let e = Exception.wrap exn in
    handle_persistent_connection_exception ~client ~is_fatal:true e;
    let needs_writes = Some (Exception.to_string e) in
    return env (shutdown_persistent_client client) ~needs_writes

[@@@warning "+52"]

(* CARE! scope of suppression should be only handle_persistent_connection_try *)

let handle_persistent_connection_ genv env client =
  let return env f ~needs_writes =
    match needs_writes with
    | Some reason -> ServerUtils.Needs_writes (env, f, true, reason)
    | None -> ServerUtils.Done (f env)
  in
  handle_persistent_connection_try return client env @@ fun () ->
  let env = { env with ide_idle = false } in
  ServerCommand.handle genv env client

let handle_connection genv env client client_kind =
  ServerIdle.stamp_connection ();

  (* This "return" is guaranteed to be run as part of main loop, when workers
   * are not busy, so we can ignore whether it needs writes or not - it's always
   * safe for it to write. *)
  let return env f ~needs_writes:_ = f env in
  match client_kind with
  | `Persistent ->
    handle_persistent_connection_ genv env client
    |> ServerUtils.wrap (handle_persistent_connection_try return client)
  | `Non_persistent ->
    handle_connection_ genv env client
    |> ServerUtils.wrap (handle_connection_try (fun x -> x) client)

let query_notifier genv env query_kind start_time =
  let open ServerNotifierTypes in
  let telemetry =
    Telemetry.create () |> Telemetry.duration ~key:"start" ~start_time
  in
  let (env, raw_updates) =
    match query_kind with
    | `Sync ->
      ( env,
        begin
          try Notifier_synchronous_changes (genv.notifier ())
          with Watchman.Timeout -> Notifier_unavailable
        end )
    | `Async ->
      ( { env with last_notifier_check_time = start_time },
        genv.notifier_async () )
    | `Skip -> (env, Notifier_async_changes SSet.empty)
  in
  let telemetry = Telemetry.duration telemetry ~key:"notified" ~start_time in
  let unpack_updates = function
    | Notifier_unavailable -> (true, SSet.empty)
    | Notifier_state_enter _ -> (true, SSet.empty)
    | Notifier_state_leave _ -> (true, SSet.empty)
    | Notifier_async_changes updates -> (true, updates)
    | Notifier_synchronous_changes updates -> (false, updates)
  in
  let (updates_stale, raw_updates) = unpack_updates raw_updates in
  let rec pump_async_updates acc =
    match genv.notifier_async_reader () with
    | Some reader when Buffered_line_reader.is_readable reader ->
      let (_, raw_updates) = unpack_updates (genv.notifier_async ()) in
      pump_async_updates (SSet.union acc raw_updates)
    | _ -> acc
  in
  let raw_updates = pump_async_updates raw_updates in
  let telemetry = Telemetry.duration telemetry ~key:"pumped" ~start_time in
  let updates = Program.process_updates genv raw_updates in
  let telemetry =
    telemetry
    |> Telemetry.duration ~key:"processed" ~start_time
    |> Telemetry.int_ ~key:"raw_updates" ~value:(SSet.cardinal raw_updates)
    |> Telemetry.int_ ~key:"updates" ~value:(Relative_path.Set.cardinal updates)
  in
  if not @@ Relative_path.Set.is_empty updates then
    HackEventLogger.notifier_returned start_time (SSet.cardinal raw_updates);
  (env, updates, updates_stale, telemetry)

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
let rec recheck_until_no_changes_left acc genv env select_outcome =
  let start_time = Unix.gettimeofday () in
  (* this is telemetry for the current batch, i.e. iteration: *)
  let telemetry =
    Telemetry.create () |> Telemetry.float_ ~key:"start_time" ~value:start_time
  in

  (* When a new client connects, we use the synchronous notifier.
   * This is to get synchronous file system changes when invoking
   * hh_client in terminal.
   *
   * NB: This also uses synchronous notify on establishing a persistent
   * connection. This is harmless, but could maybe be filtered away. *)
  let query_kind =
    match select_outcome with
    | ClientProvider.Select_new _ -> `Sync
    | ClientProvider.Select_nothing ->
      if Float.(start_time - env.last_notifier_check_time > 0.5) then
        `Async
      else
        `Skip
    (* Do not process any disk changes when there are pending persistent
     * client requests - some of them might be edits, and we don't want to
     * do analysis on mid-edit state of the world *)
    | ClientProvider.Select_persistent -> `Skip
  in
  let (env, updates, updates_stale, query_telemetry) =
    query_notifier genv env query_kind start_time
  in
  let telemetry =
    telemetry
    |> Telemetry.object_ ~key:"query" ~value:query_telemetry
    |> Telemetry.duration ~key:"query_done" ~start_time
  in
  let acc = { acc with updates_stale } in
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
  let env =
    match env.nonpersistent_client_pending_command_needs_full_check with
    (* We need to auto-restart the recheck to make progress towards handling
     * this command... *)
    | Some (_command, reason, client)
      when is_full_check_needed env.full_check_status
           (*... but we don't want to get into a battle with IDE edits stopping
            * rechecks and us restarting them. We're going to heavily favor edits and
            * restart only after a longer period since last edit. Note that we'll still
            * start full recheck immediately after any file save. *)
           && Float.(start_time - env.last_command_time > 5.0) ->
      let still_there =
        try
          ClientProvider.ping client;
          true
        with ClientProvider.Client_went_away -> false
      in
      if still_there then (
        Hh_logger.log "Restarting full check due to %s" reason;
        { env with full_check_status = Full_check_started }
      ) else (
        ClientProvider.shutdown_client client;
        {
          env with
          nonpersistent_client_pending_command_needs_full_check = None;
        }
      )
    | _ -> env
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
    is_full_check_started env.full_check_status
    (* Prioritize building search index over full rechecks. *)
    && ( Queue.is_empty SearchServiceRunner.SearchServiceRunner.queue
       (* Unless there is something actively waiting for this *)
       || Option.is_some
            env.nonpersistent_client_pending_command_needs_full_check )
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
    let acc =
      { acc with per_batch_telemetry = telemetry :: acc.per_batch_telemetry }
    in
    (acc, env)
  else
    let check_kind =
      if lazy_check then
        ServerTypeCheck.Lazy_check
      else
        ServerTypeCheck.Full_check
    in
    let check_kind_str = ServerTypeCheck.check_kind_to_string check_kind in
    let env = { env with can_interrupt = not lazy_check } in
    let needed_full_init = env.init_env.why_needed_full_init in
    let old_errorl = Errors.get_error_list env.errorl in

    (* HERE'S WHERE WE DO THE HEAVY WORK! **)
    let telemetry =
      telemetry
      |> Telemetry.string_ ~key:"check_kind" ~value:check_kind_str
      |> Telemetry.duration ~key:"type_check_start" ~start_time
    in
    let (env, res, type_check_telemetry) =
      CgroupProfiler.profile_memory ~event:(`Recheck check_kind_str)
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
      match (needed_full_init, env.init_env.why_needed_full_init) with
      | (Some needed_full_init, None) ->
        finalize_init env.init_env telemetry needed_full_init
      | _ -> ()
    end;
    ServerStamp.touch_stamp_errors old_errorl (Errors.get_error_list env.errorl);
    let telemetry =
      Telemetry.duration telemetry ~key:"finalized_and_touched" ~start_time
    in
    let acc =
      {
        rechecked_count =
          acc.rechecked_count + res.ServerTypeCheck.reparse_count;
        per_batch_telemetry = telemetry :: acc.per_batch_telemetry;
        total_rechecked_count =
          acc.total_rechecked_count + res.ServerTypeCheck.total_rechecked_count;
        updates_stale = acc.updates_stale;
        recheck_id = acc.recheck_id;
        duration = acc.duration +. (Unix.gettimeofday () -. start_time);
        any_full_checks = acc.any_full_checks || not lazy_check;
      }
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
      (acc, env)
    else
      recheck_until_no_changes_left acc genv env select_outcome

let new_serve_iteration_id () = Random_id.short_string ()

(* This is safe to run only in the main loop, when workers are not doing
 * anything. *)
let main_loop_command_handler client_kind client result =
  match result with
  | ServerUtils.Done env -> env
  | ServerUtils.Needs_full_recheck (env, f, reason) ->
    begin
      match client_kind with
      | `Non_persistent ->
        (* We should not accept any new clients until this is cleared *)
        assert (
          Option.is_none
            env.nonpersistent_client_pending_command_needs_full_check );
        {
          env with
          nonpersistent_client_pending_command_needs_full_check =
            Some (f, reason, client);
        }
      | `Persistent ->
        (* Persistent client will not send any further commands until previous one
         * is handled. *)
        assert (
          Option.is_none env.persistent_client_pending_command_needs_full_check
        );
        {
          env with
          persistent_client_pending_command_needs_full_check = Some (f, reason);
        }
    end
  | ServerUtils.Needs_writes (env, f, _, _) -> f env

let has_pending_disk_changes genv =
  match genv.notifier_async_reader () with
  | Some reader when Buffered_line_reader.is_readable reader -> true
  | _ -> false

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

let serve_one_iteration genv env client_provider =
  let (env, recheck_id) = generate_and_update_recheck_id env in
  ServerMonitorUtils.exit_if_parent_dead ();
  let acceptable_new_client_kind =
    let has_default_client_pending =
      Option.is_some env.nonpersistent_client_pending_command_needs_full_check
    in
    let can_accept_clients = not @@ ServerRevisionTracker.is_hg_updating () in
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
    | None -> ClientProvider.Select_nothing
    | Some client_kind ->
      ClientProvider.sleep_and_check
        client_provider
        env.persistent_client
        ~ide_idle:env.ide_idle
        ~idle_gc_slice:genv.local_config.ServerLocalConfig.idle_gc_slice
        client_kind
  in

  (* We'll now update any waiting clients with our status.
   * (Or more precisely, we'll tell the monitor, so any waiting clients
   * will know when they poll the monitor.)
   *
   * By updating status now at the start of the serve_one_iteration,
   * it means there's no obligation on the "doing work" part of the previous
   * iteration to clean up its own status-reporting once done.
   * Caveat: that's not quite true, since ClientProvider.sleep_and_check will
   * wait up to 1s if there are no pending requests. So theoretically we
   * won't update our status for up to 1s after the previous work is done.
   * That doesn't really matter, since (1) if there are no pending requests
   * then no client will even ask for status, and (2) it's worth it to
   * keep the code clean and simple.
   *
   * Note: the message here might soon be replaced. If we discover disk changes
   * that prompt a typecheck, then typechecking sends its own status updates.
   * And if the selected_client was a request, then once we discover the nature
   * of that request then ServerCommand.handle will send its own status updates too.
   *)
  ServerProgress.send_progress
    ~include_in_logs:false
    "%s"
    (match selected_client with
    | ClientProvider.Select_nothing ->
      if env.ide_idle then
        "ready"
      else
        "HackIDE:active"
    | _ -> "working");
  let env =
    match selected_client with
    | ClientProvider.Select_nothing ->
      let last_stats = env.last_recheck_loop_stats in
      (* Ugly hack: We want GC_SHAREDMEM_RAN to record the last rechecked
       * count so that we can figure out if the largest reclamations
       * correspond to massive rebases. However, the logging call is done in
       * the SharedMem module, which doesn't know anything about Server stuff.
       * So we wrap the call here. *)
      HackEventLogger.with_rechecked_stats
        (List.length last_stats.per_batch_telemetry)
        last_stats.rechecked_count
        last_stats.total_rechecked_count
        (fun () -> SharedMem.collect `aggressive);
      let t = Unix.gettimeofday () in
      if Float.(t -. env.last_idle_job_time > 0.5) then
        let env = ServerIdle.go env in
        { env with last_idle_job_time = t }
      else
        env
    | _ -> env
  in
  let stage =
    if Option.is_some env.init_env.why_needed_full_init then
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
      (empty_recheck_loop_stats ~recheck_id)
      genv
      env
      selected_client
  in
  let t_done_recheck = Unix.gettimeofday () in
  let did_work = stats.total_rechecked_count > 0 in
  let env =
    {
      env with
      last_recheck_loop_stats = stats;
      last_recheck_loop_stats_for_actual_work =
        ( if did_work then
          Some stats
        else
          env.last_recheck_loop_stats_for_actual_work );
    }
  in
  (* push diagnostic changes to client, if necessary *)
  let (env, diag_reason) =
    match env.diag_subscribe with
    | None -> (env, "no diag subscriptions")
    | Some sub ->
      let client = Utils.unsafe_opt env.persistent_client in
      (* Should we hold off sending diagnostics to the client? *)
      if ClientProvider.client_has_message client then
        (env, "client has message")
      else if not @@ Relative_path.Set.is_empty env.ide_needs_parsing then
        (env, "ide_needs_parsing: processed edits but didn't recheck them yet")
      else if has_pending_disk_changes genv then
        (env, "has_pending_disk_changes")
      else
        let (sub, errors) = Diagnostic_subscription.pop_errors sub env.errorl in
        let env = { env with diag_subscribe = Some sub } in
        let id = Diagnostic_subscription.get_id sub in
        let res = ServerCommandTypes.DIAGNOSTIC (id, errors) in
        if SMap.is_empty errors then
          (env, "is_empty errors")
        else begin
          try
            ClientProvider.send_push_message_to_client client res;
            (env, "sent push message")
          with ClientProvider.Client_went_away ->
            (* Leaving cleanup of this condition to handled_connection function *)
            (env, "Client_went_away")
        end
  in
  let t_sent_diagnostics = Unix.gettimeofday () in

  if did_work then begin
    let telemetry =
      ServerEnv.recheck_loop_stats_to_user_telemetry stats
      |> Telemetry.string_ ~key:"diag_reason" ~value:diag_reason
    in
    HackEventLogger.recheck_end
      stats.duration
      (List.length stats.per_batch_telemetry - 1)
      stats.rechecked_count
      stats.total_rechecked_count
      (Option.some_if stats.any_full_checks telemetry);
    Hh_logger.log
      "RECHECK_END (recheck_id %s):\n%s"
      recheck_id
      (Telemetry.to_string telemetry);
    (* we're only interested in full check data *)
    CgroupProfiler.print_summary_memory_table ~event:(`Recheck "Full_check")
  end;

  let env =
    match selected_client with
    | ClientProvider.Select_persistent -> env
    | ClientProvider.Select_nothing -> env
    | ClientProvider.Select_new client ->
      begin
        try
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
            handle_connection genv env client `Non_persistent
            |> main_loop_command_handler `Non_persistent client
          in
          HackEventLogger.handled_connection t_start_recheck;
          env
        with exn ->
          let e = Exception.wrap exn in
          HackEventLogger.handle_connection_exception "outer" e;
          Hh_logger.log
            "HANDLE_CONNECTION_EXCEPTION(outer) [ignoring request] %s"
            (Exception.to_string e);
          env
      end
  in
  let has_persistent_connection_request =
    (* has_persistent_connection_request means that at the beginning of this
     * iteration of main loop there was a request to read and handle.
     * We'll now try to do it, but it's possible that we have ran a recheck
     * in-between  those two events, and if this recheck was non-blocking, we
     * might have already handled this command there. Proceeding to
     * handle_connection would then block reading a request that is not there
     * anymore, so we need to check and update has_persistent_connection_request
     * again. *)
    Option.value_map
      env.persistent_client
      ~f:ClientProvider.has_persistent_connection_request
      ~default:false
  in
  let env =
    if has_persistent_connection_request then (
      let client = Utils.unsafe_opt env.persistent_client in
      (* client here is the existing persistent client *)
      (* whose request we're going to handle.          *)
      HackEventLogger.got_persistent_client_channels t_start_recheck;
      try
        let env =
          handle_connection genv env client `Persistent
          |> main_loop_command_handler `Persistent client
        in
        HackEventLogger.handled_persistent_connection t_start_recheck;
        env
      with exn ->
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

let watchman_interrupt_handler genv env =
  let start_time = Unix.gettimeofday () in
  let (env, updates, updates_stale, _telemetry) =
    query_notifier genv env `Async start_time
  in
  (* Async updates can always be stale, so we don't care *)
  ignore updates_stale;
  let size = Relative_path.Set.cardinal updates in
  if size > 0 then (
    Hh_logger.log "Interrupted by Watchman message: %d files changed" size;
    ( {
        env with
        disk_needs_parsing =
          Relative_path.Set.union env.disk_needs_parsing updates;
      },
      MultiThreadedCall.Cancel )
  ) else
    (env, MultiThreadedCall.Continue)

let priority_client_interrupt_handler genv client_provider env =
  let t = Unix.gettimeofday () in
  (* For non-persistent clients that don't synchronize file contents, users
   * expect that a query they do immediately after saving a file will reflect
   * this file contents. Async notifications are not always fast enough to
   * quarantee it, so we need an additional sync query before accepting such
   * client *)
  let (env, updates, _updates_stale, _telemetry) =
    query_notifier genv env `Sync t
  in
  let size = Relative_path.Set.cardinal updates in
  if size > 0 then (
    Hh_logger.log "Interrupted by Watchman sync query: %d files changed" size;
    ( {
        env with
        disk_needs_parsing =
          Relative_path.Set.union env.disk_needs_parsing updates;
      },
      MultiThreadedCall.Cancel )
  ) else
    let idle_gc_slice = genv.local_config.ServerLocalConfig.idle_gc_slice in
    let select_outcome =
      if ServerRevisionTracker.is_hg_updating () then
        ClientProvider.Select_nothing
      else
        ClientProvider.sleep_and_check
          client_provider
          env.persistent_client
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
        env
      | ClientProvider.Select_new client ->
        (match handle_connection genv env client `Non_persistent with
        | ServerUtils.Needs_full_recheck (_, _, reason) ->
          failwith
            ( "unexpected command needing full recheck in priority channel: "
            ^ reason )
        | ServerUtils.Needs_writes (_, _, _, reason) ->
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
      | _ -> MultiThreadedCall.Continue
    in
    (env, decision)

let persistent_client_interrupt_handler genv env =
  match env.persistent_client with
  (* Several handlers can become ready simultaneously and one of them can remove
   * the persistent client before we get to it. *)
  | None -> (env, MultiThreadedCall.Continue)
  | Some client ->
    (match handle_connection genv env client `Persistent with
    | ServerUtils.Needs_full_recheck (env, f, reason) ->
      (* This should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one. *)
      assert (
        Option.is_none env.persistent_client_pending_command_needs_full_check );
      ( {
          env with
          persistent_client_pending_command_needs_full_check = Some (f, reason);
        },
        MultiThreadedCall.Continue )
    | ServerUtils.Needs_writes (env, f, should_restart_recheck, _) ->
      let full_check_status =
        match env.full_check_status with
        | Full_check_started when not should_restart_recheck ->
          Full_check_needed
        | x -> x
      in
      (* this should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one *)
      assert (Option.is_none env.pending_command_needs_writes);
      ( { env with pending_command_needs_writes = Some f; full_check_status },
        MultiThreadedCall.Cancel )
    | ServerUtils.Done env -> (env, MultiThreadedCall.Continue))

let setup_interrupts env client_provider =
  {
    env with
    interrupt_handlers =
      (fun genv env ->
        let { ServerLocalConfig.interrupt_on_watchman; interrupt_on_client; _ }
            =
          genv.local_config
        in
        let interrupt_on_watchman =
          interrupt_on_watchman && env.can_interrupt
        in
        let interrupt_on_client = interrupt_on_client && env.can_interrupt in
        let handlers =
          match genv.notifier_async_reader () with
          | Some reader when interrupt_on_watchman ->
            [
              ( Buffered_line_reader.get_fd reader,
                watchman_interrupt_handler genv );
            ]
          | _ -> []
        in
        let handlers =
          match ClientProvider.priority_fd client_provider with
          | Some fd when interrupt_on_client ->
            (fd, priority_client_interrupt_handler genv client_provider)
            :: handlers
          | _ -> handlers
        in
        let handlers =
          match env.persistent_client >>= ClientProvider.get_client_fd with
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
  MultiThreadedCall.on_exception (fun (e, stack) ->
      ServerUtils.exit_on_exception e ~stack);
  let client_provider = ClientProvider.provider_from_file_descriptors in_fds in

  (* This is needed when typecheck_after_init option is disabled.
   * We're just filling it with placeholder telemetry values since
   * we don't much care about this scenario. *)
  let init_telemetry =
    Telemetry.create ()
    |> Telemetry.string_
         ~key:"mode"
         ~value:"serve_due_to_disabled_typecheck_after_init"
  in
  let typecheck_telemetry = Telemetry.create () in
  if Option.is_none env.init_env.why_needed_full_init then
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
 * 4. If monitor previously got a saved-state failure and restarts, it might decide
 *    to use Informant_induced_mini_state_target, targeting the saved-state it had previously!
 * 5. If "hh_server --with-mini-state", then load the one specified there!
 * 6. If hh.conf lacks "load_state_natively_v4", then don't load it
 * 7. Otherwise, load it normally!
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
    else if not genv.local_config.ServerLocalConfig.use_saved_state then
      (ServerInit.Full_init, "Local_config_saved_state_disabled")
    else if ServerArgs.no_load genv.options then
      (ServerInit.Full_init, "Server_args_no_load")
    else if Option.is_some (ServerArgs.save_filename genv.options) then
      (ServerInit.Full_init, "Server_args_saving_state")
    else if Option.is_some (ServerArgs.write_symbol_info genv.options) then
      (ServerInit.Write_symbol_info, "Server_args_writing_symbol_info")
    else (
      match
        ( genv.local_config.ServerLocalConfig.load_state_natively,
          ServerArgs.with_saved_state genv.options )
      with
      | (_, Some (ServerArgs.Informant_induced_saved_state_target target)) ->
        ( ServerInit.Saved_state_init
            (ServerInit.Load_state_natively_with_target target),
          "Load_state_natively_with_target" )
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
  let env =
    {
      env with
      init_env =
        { env.init_env with ci_info = Some (Ci_util.begin_get_info ()) };
    }
  in
  let (init_approach, approach_name) = resolve_init_approach genv in
  Hh_logger.log "Initing with approach: %s" approach_name;
  let (env, init_type, init_error, init_error_stack, state_distance) =
    let (env, init_result) = ServerInit.init ~init_approach genv env in
    match init_approach with
    | ServerInit.Remote_init _ -> (env, "remote", None, None, None)
    | ServerInit.Write_symbol_info
    | ServerInit.Full_init ->
      (env, "fresh", None, None, None)
    | ServerInit.Parse_only_init -> (env, "parse-only", None, None, None)
    | ServerInit.Saved_state_init _ ->
      begin
        match init_result with
        | ServerInit.Load_state_succeeded distance ->
          let init_type =
            match
              Naming_table.get_forward_naming_fallback_path env.naming_table
            with
            | None -> "state_load_blob"
            | Some _ -> "state_load_sqlite"
          in
          (env, init_type, None, None, distance)
        | ServerInit.Load_state_failed (err, stack) ->
          (env, "state_load_failed", Some err, Some stack, None)
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
          state_distance;
          approach_name;
          init_error;
          init_type;
        };
    }
  in
  Hh_logger.log "Waiting for daemon(s) to be ready...";
  ServerProgress.send_progress "wrapping up init...";
  genv.wait_until_ready ();
  ServerStamp.touch_stamp ();
  let informant_use_xdb =
    genv.local_config.ServerLocalConfig.informant_use_xdb
  in
  let load_script_timeout =
    genv.local_config.ServerLocalConfig.load_state_script_timeout
  in
  EventLogger.set_init_type init_type;
  let telemetry = ServerUtils.log_and_get_sharedmem_load_telemetry () in
  HackEventLogger.init_lazy_end
    telemetry
    ~informant_use_xdb
    ~load_script_timeout
    ~state_distance
    ~approach_name
    ~init_error
    ~init_error_stack
    ~init_type;
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
            ( "Warning: both an argument --max-procs and a local config "
            ^^ "for max workers are given. Choosing minimum of the two." );
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
  let server_finale_file = ServerFiles.server_finale_file pid in
  let server_progress_file = ServerFiles.server_progress_file pid in
  let server_receipt_to_monitor_file =
    ServerFiles.server_receipt_to_monitor_file pid
  in
  Exit.prepare_server_specific_files
    ~server_finale_file
    ~server_progress_file
    ~server_receipt_to_monitor_file;
  ServerCommandTypesUtils.write_progress_file
    ~server_progress_file
    ~server_progress:
      {
        ServerCommandTypes.server_warning = None;
        server_progress = "starting up";
        server_timestamp = Unix.gettimeofday ();
      };
  Hh_logger.log "Version: %s" Hh_version.version;
  Hh_logger.log "Hostname: %s" (Unix.gethostname ());
  let root = ServerArgs.root options in
  ServerDynamicView.toggle := ServerArgs.dynamic_view options;

  let deps_mode =
    match ServerArgs.save_64bit options with
    | Some new_edges_dir ->
      Typing_deps_mode.SaveCustomMode { graph = None; new_edges_dir }
    | None -> Typing_deps_mode.SQLiteMode
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
    let profile_owner = local_config.ServerLocalConfig.profile_owner in
    let profile_desc = local_config.ServerLocalConfig.profile_desc in
    Hh_logger.Level.set_min_level local_config.ServerLocalConfig.min_log_level;
    Hh_logger.Level.set_categories local_config.ServerLocalConfig.log_categories;

    if Sys_utils.is_test_mode () then
      EventLogger.init_fake ()
    else if is_worker then
      HackEventLogger.init_worker
        ~root
        ~hhconfig_version
        ~init_id
        ~custom_columns:(ServerArgs.custom_telemetry_data options)
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags local_config)
        ~time:(Unix.gettimeofday ())
        ~profile_owner
        ~profile_desc
    else
      HackEventLogger.init
        ~root
        ~hhconfig_version
        ~init_id
        ~custom_columns:(ServerArgs.custom_telemetry_data options)
        ~informant_managed
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags local_config)
        ~time:(Unix.gettimeofday ())
        ~max_workers:num_workers
        ~profile_owner
        ~profile_desc
  in
  logging_init init_id ~is_worker:false;
  HackEventLogger.init_start
    ~experiments_config_meta:
      local_config.ServerLocalConfig.experiments_config_meta;
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
  let workers =
    let gc_control = ServerConfig.gc_control config in
    ServerWorker.make
      ~longlived_workers:local_config.ServerLocalConfig.longlived_workers
      ~nbr_procs:num_workers
      gc_control
      handle
      ~logging_init:worker_logging_init
  in
  let genv = ServerEnvBuild.make_genv options config local_config workers in
  (genv, ServerEnvBuild.make_env genv.config ~init_id ~deps_mode)

let run_once options config local_config =
  let (genv, env) =
    setup_server
      options
      config
      local_config
      ~informant_managed:false
      ~monitor_pid:None
  in
  if not (ServerArgs.check_mode genv.options) then (
    Hh_logger.log "ServerMain run_once only supported in check mode.";
    Exit.exit Exit_status.Input_error
  );

  (* The type-checking happens here *)
  let env = program_init genv env in
  (* All of saving state happens here *)
  let (env, save_state_results) =
    match
      ( ServerArgs.save_filename genv.options,
        ServerArgs.save_with_spec genv.options )
    with
    | (None, None) -> (env, None)
    | (Some filename, None) -> (env, ServerInit.save_state genv env filename)
    | (None, Some (spec : ServerArgs.save_state_spec_info)) ->
      (env, ServerInit.save_state genv env spec.ServerArgs.filename)
    | (Some _, Some _) ->
      failwith "Saved state file name is specified in two different ways!"
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
  Printexc.record_backtrace true;
  let (config, local_config) =
    ServerConfig.(load ~silent:false filename options)
  in
  let (genv, env) =
    setup_server
      options
      config
      local_config
      ~informant_managed
      ~monitor_pid:(Some monitor_pid)
  in
  if ServerArgs.check_mode genv.options then (
    Hh_logger.log "Invalid program args - can't run daemon in check mode.";
    Exit.exit Exit_status.Input_error
  );
  HackEventLogger.with_id ~stage:`Init env.init_env.init_id @@ fun () ->
  let env = MainInit.go genv options (fun () -> program_init genv env) in
  CgroupProfiler.print_summary_memory_table ~event:`Init;
  serve genv env in_fds

let daemon_main
    ( informant_managed,
      state,
      options,
      monitor_pid,
      priority_in_fd,
      force_dormant_start_only_in_fd )
    (default_ic, _default_oc) =
  (* Avoid leaking this fd further *)
  let () = Unix.set_close_on_exec priority_in_fd in
  let () = Unix.set_close_on_exec force_dormant_start_only_in_fd in
  let default_in_fd = Daemon.descr_of_in_channel default_ic in

  (* Restore the root directory and other global states from monitor *)
  ServerGlobalState.restore state 0;

  (* Restore hhi files every time the server restarts
    in case the tmp folder changes *)
  ignore (Hhi.get_hhi_root ());

  ServerUtils.with_exit_on_exception @@ fun () ->
  daemon_main_exn
    ~informant_managed
    options
    monitor_pid
    (default_in_fd, priority_in_fd, force_dormant_start_only_in_fd)

let entry = Daemon.register_entry_point "ServerMain.daemon_main" daemon_main
