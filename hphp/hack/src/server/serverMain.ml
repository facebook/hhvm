(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ServerEnv
open Reordered_argument_collections
open String_utils
open Option.Monad_infix

(*****************************************************************************)
(* Main initialization *)
(*****************************************************************************)

let exit_on_parent_exit () = Parent.exit_on_parent_exit 10 60

let () = Printexc.record_backtrace true

module MainInit : sig
  val go:
    genv ->
    ServerArgs.options ->
    string ->
    (unit -> env) ->    (* init function to run while we have init lock *)
    env
end = struct
  (* This code is only executed when the options --check is NOT present *)
  let go genv options init_id init_fun =
    let root = ServerArgs.root options in
    let t = Unix.gettimeofday () in
    Hh_logger.log "Initializing Server (This might take some time)";
    (* note: we only run periodical tasks on the root, not extras *)
    ServerIdle.init genv root;
    Hh_logger.log "Init id: %s" init_id;
    let env = HackEventLogger.with_id ~stage:`Init init_id init_fun in
    Hh_logger.log "Server is partially ready";
    let t' = Unix.gettimeofday () in
    Hh_logger.log "Took %f seconds." (t' -. t);

    env
end

module Program =
  struct
    let preinit () =
      (* Warning: Global references inited in this function, should
         be 'restored' in the workers, because they are not 'forked'
         anymore. See `ServerWorker.{save/restore}_state`. *)

      Sys_utils.set_signal Sys.sigusr1
        (Sys.Signal_handle Typing.debug_print_last_pos);
      Sys_utils.set_signal Sys.sigusr2
        (Sys.Signal_handle (fun _ -> (
             Hh_logger.log "Got sigusr2 signal. Going to shut down.";
             Exit_status.exit Exit_status.Server_shutting_down
           )))

    let run_once_and_exit genv env =
      ServerError.print_errorl
        None
        (ServerArgs.json_mode genv.options)
        (List.map (Errors.get_error_list env.errorl) Errors.to_absolute) stdout;
      match ServerArgs.convert genv.options with
      | None ->
         WorkerController.killall ();
         (* as Warnings shouldn't break CI, don't change the exit status except
          * for Errors *)
         let has_errors = if Errors.is_empty env.errorl
           then false
           else
             List.exists ~f:(fun e -> Errors.get_severity e = Errors.Error)
               (Errors.get_error_list env.errorl)
         in
         exit (if has_errors then 1 else 0)
      | Some dirname ->
         ServerConvert.go genv env dirname;
         WorkerController.killall ();
         exit 0

    (* filter and relativize updated file paths *)
    let process_updates genv updates =
      let root = Path.to_string @@ ServerArgs.root genv.options in
      (* Because of symlinks, we can have updates from files that aren't in
       * the .hhconfig directory *)
      let updates = SSet.filter updates (fun p -> string_starts_with p root) in
      let updates = Relative_path.(relativize_set Root updates) in
      let to_recheck =
        Relative_path.Set.filter updates begin fun update ->
          ServerEnv.file_filter (Relative_path.to_absolute update)
        end in
      let config_in_updates =
        Relative_path.Set.mem updates ServerConfig.filename in
      if config_in_updates then begin
        let new_config, _ = ServerConfig.(load filename genv.options) in
        if not (ServerConfig.is_compatible genv.config new_config) then begin
          Hh_logger.log
            "%s changed in an incompatible way; please restart %s.\n"
            (Relative_path.suffix ServerConfig.filename)
            GlobalConfig.program_name;
           (** TODO: Notify the server monitor directly about this. *)
           Exit_status.(exit Hhconfig_changed)
        end;
      end;
      to_recheck
  end

let finalize_init genv init_env =
  ServerUtils.print_hash_stats ();
  Hh_logger.log "Heap size: %d" (SharedMem.heap_size ());
  Hh_logger.log "Server is READY";
  let t' = Unix.gettimeofday () in
  Hh_logger.log "Took %f seconds to initialize." (t' -. init_env.init_start_t);
  HackEventLogger.init_really_end
    ~informant_use_xdb:genv.local_config.ServerLocalConfig.informant_use_xdb
    ~state_distance:init_env.state_distance
    ~approach_name:init_env.approach_name
    ~init_error:init_env.init_error
    init_env.init_type

let shutdown_persistent_client client env  =
  ClientProvider.shutdown_client client;
  let env = { env with
    pending_command_needs_writes = None;
    persistent_client_pending_command_needs_full_check = None;
  } in
  ServerFileSync.clear_sync_data env

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

[@@@warning "-52"] (* we have no alternative but to depend on Sys_error strings *)
let handle_connection_exception env client e stack = match e with
  | ClientProvider.Client_went_away | ServerCommandTypes.Read_command_timeout ->
    ClientProvider.shutdown_client client;
    env
  (** Connection dropped off. Its unforunate that we don't actually know
   * which connection went bad (could be any write to any connection to
   * child processes/daemons), we just assume at this top-level that
   * since its not caught elsewhere, its the connection to the client.
   *
   * TODO: Make sure the pipe exception is really about this client.*)
  | Unix.Unix_error (Unix.EPIPE, _, _)
  | Sys_error("Broken pipe")
  | Sys_error("Connection reset by peer") ->
    Hh_logger.log "Client channel went bad. Shutting down client connection";
    ClientProvider.shutdown_client client;
    env
  | e ->
    HackEventLogger.handle_connection_exception e;
    let msg = Printexc.to_string e in
    EventLogger.master_exception e stack;
    Printf.fprintf stderr "Error: %s\n%!" msg;
    Printexc.print_backtrace stderr;
    ClientProvider.shutdown_client client;
    env
[@@@warning "+52"] (* CARE! scope of suppression should be only handle_connection_exception *)

(* f represents a non-persistent command coming from client. If executing f
 * throws, we need to dispopose of this client (possibly recovering updated
 * environment from Nonfatal_rpc_exception). "return" is a constructor
 * wrapping the return value to make it match return type of f *)
let handle_connection_try return client env f =
  try f () with
  | ServerCommand.Nonfatal_rpc_exception (e, stack, env) ->
    return (handle_connection_exception env client e (Some stack))
  | e ->
    return (handle_connection_exception env client e None)

let handle_connection_ genv env client =
  let open ServerCommandTypes in
  let t = Unix.gettimeofday () in
  handle_connection_try (fun x -> ServerUtils.Done x) client env @@ fun () ->
    match ClientProvider.read_connection_type client with
    | Persistent ->
      let f = fun env ->
        let env = match env.persistent_client with
          | Some old_client ->
            ClientProvider.send_push_message_to_client old_client
              NEW_CLIENT_CONNECTED;
            shutdown_persistent_client old_client env
          | None -> env
        in
        ClientProvider.send_response_to_client client Connected t;
        let env = { env with persistent_client =
            Some (ClientProvider.make_persistent client)} in
        (* If the client connected in the middle of recheck, let them know it's
         * happening. *)
        if env.full_check = Full_check_started then
          ServerBusyStatus.send env
            (ServerCommandTypes.Doing_global_typecheck env.can_interrupt);
        env
      in
      if Option.is_some env.persistent_client
        (* Cleaning up after existing client (in shutdown_persistent_client)
         * will attempt to write to shared memory *)
        then ServerUtils.Needs_writes (env, f, true)
        else ServerUtils.Done (f env)
    | Non_persistent ->
      ServerCommand.handle genv env client

let report_persistent_exception
    ~(e: exn)
    ~(stack: string)
    ~(client: ClientProvider.client)
    ~(is_fatal: bool)
  : unit =
  let open Marshal_tools in
  let message = Printexc.to_string e in
  let push = if is_fatal then ServerCommandTypes.FATAL_EXCEPTION { message; stack; }
  else ServerCommandTypes.NONFATAL_EXCEPTION { message; stack; } in
  begin try ClientProvider.send_push_message_to_client client push with _ -> () end;
  EventLogger.master_exception e (Some stack);
  Printf.eprintf "Error: %s\n%s\n%!" message stack

(* Same as handle_connection_try, but for persistent clients *)
[@@@warning "-52"] (* we have no alternative but to depend on Sys_error strings *)
let handle_persistent_connection_try return client env f =
  try f () with
  (** TODO: Make sure the pipe exception is really about this client. *)
  | Unix.Unix_error (Unix.EPIPE, _, _)
  | Sys_error("Connection reset by peer")
  | Sys_error("Broken pipe")
  | ServerCommandTypes.Read_command_timeout
  | ServerClientProvider.Client_went_away ->
    return env (shutdown_persistent_client client) ~needs_writes:true
  | ServerCommand.Nonfatal_rpc_exception (e, stack, env) ->
    report_persistent_exception ~e ~stack ~client ~is_fatal:false;
    return env (fun env -> env) ~needs_writes:false
  | e ->
    let stack = Printexc.get_backtrace () in
    report_persistent_exception ~e ~stack ~client ~is_fatal:true;
    return env (shutdown_persistent_client client) ~needs_writes:true
[@@@warning "+52"] (* CARE! scope of suppression should be only handle_persistent_connection_try *)

let handle_persistent_connection_ genv env client =
  let return env f ~needs_writes =
    if needs_writes then ServerUtils.Needs_writes (env, f, true)
    else ServerUtils.Done (f env)
  in
  handle_persistent_connection_try return client env
      @@ fun () ->
    let env = { env with ide_idle = false; } in
    ServerCommand.handle genv env client

let handle_connection genv env client client_kind =
  ServerIdle.stamp_connection ();
  (* This "return" is guaranteed to be run as part of main loop, when workers
   * are not busy, so we can ignore whether it needs writes or not - it's always
   * safe for it to write. *)
  let return env f ~needs_writes:_ = f env in
  match client_kind with
    | `Persistent ->
      handle_persistent_connection_ genv env client |>
      ServerUtils.wrap (handle_persistent_connection_try return client)
    | `Non_persistent  ->
      handle_connection_ genv env client |>
      ServerUtils.wrap (handle_connection_try (fun x -> x) client)

let recheck genv old_env check_kind =
  let can_interrupt = check_kind = ServerTypeCheck.Full_check in
  let old_env = { old_env with can_interrupt } in
  let new_env, res = ServerTypeCheck.check genv old_env check_kind in
  let new_env = { new_env with can_interrupt = true } in
  if old_env.init_env.needs_full_init &&
      not new_env.init_env.needs_full_init then
        finalize_init genv new_env.init_env;
  ServerStamp.touch_stamp_errors (Errors.get_error_list old_env.errorl)
                                 (Errors.get_error_list new_env.errorl);
  new_env, res

let query_notifier genv env query_kind t =
  let open ServerNotifierTypes in
  let env, raw_updates = match query_kind with
    | `Sync ->
      env, begin try Notifier_synchronous_changes (genv.notifier ()) with
      | Watchman.Timeout -> Notifier_unavailable
      end
    | `Async ->
      { env with last_notifier_check_time = t; }, genv.notifier_async ()
    | `Skip ->
      env, Notifier_async_changes SSet.empty
  in
  let unpack_updates = function
    | Notifier_unavailable -> true, SSet.empty
    | Notifier_state_enter _ -> true, SSet.empty
    | Notifier_state_leave _ -> true, SSet.empty
    | Notifier_async_changes updates -> true, updates
    | Notifier_synchronous_changes updates -> false, updates
  in

  let updates_stale, raw_updates = unpack_updates raw_updates in

  let rec pump_async_updates acc = match genv.notifier_async_reader () with
    | Some reader when Buffered_line_reader.is_readable reader ->
      let _, raw_updates = unpack_updates (genv.notifier_async ()) in
      pump_async_updates (SSet.union acc raw_updates)
    | _ -> acc
  in

  let raw_updates = pump_async_updates raw_updates in

  let updates = Program.process_updates genv raw_updates in
  if not @@ Relative_path.Set.is_empty updates then
    HackEventLogger.notifier_returned t (SSet.cardinal raw_updates);
  env, updates, updates_stale

(* When a rebase occurs, Watchman/dfind takes a while to give us the full list
 * of updates, and it often comes in batches. To get an accurate measurement
 * of rebase time, we use the heuristic that any changes that come in
 * right after one rechecking round finishes to be part of the same
 * rebase, and we don't log the recheck_end event until the update list
 * is no longer getting populated.
 *
 * The above doesn't apply in presence of interruptions / cancellations -
 * it's possible for client to request current recheck to be stopped.
 *)
let rec recheck_loop acc genv env new_client has_persistent_connection_request =
  let t = Unix.gettimeofday () in
  (** When a new client connects, we use the synchronous notifier.
   * This is to get synchronous file system changes when invoking
   * hh_client in terminal.
   *
   * NB: This also uses synchronous notify on establishing a persistent
   * connection. This is harmless, but could maybe be filtered away. *)
  let query_kind = match new_client, has_persistent_connection_request with
    | Some _, false -> `Sync
    | None, false when t -. env.last_notifier_check_time > 0.5 -> `Async
    (* Do not process any disk changes when there are pending persistent
    * client requests - some of them might be edits, and we don't want to
    * do analysis on mid-edit state of the world *)
    | _ -> `Skip
  in
  let env, updates, updates_stale = query_notifier genv env query_kind t in
  let acc = { acc with updates_stale } in

  let is_idle = (not has_persistent_connection_request) &&
     (* "average person types [...] between 190 and 200 characters per minute"
      * 60/200 = 0.3 *)
     t -. env.last_command_time > 0.3 in

  let env = if Relative_path.Set.is_empty updates then env else { env with
    disk_needs_parsing = Relative_path.Set.union updates env.disk_needs_parsing;
    (* saving any file is our trigger to start full recheck *)
    full_check = Full_check_started;
  } in

  let env = match env.default_client_pending_command_needs_full_check with
    (* We need to auto-restart the recheck to make progress towards handling
     * this command... *)
    | Some (_command, reason, client) when env.full_check = Full_check_needed
    (*... but we don't want to get into a battle with IDE edits stopping
     * rechecks and us restarting them. We're going to heavily favor edits and
     * restart only after a longer period since last edit. Note that we'll still
     * start full recheck immediately after any file save. *)
    && t -. env.last_command_time > 5.0 ->
      let still_there = try
          ClientProvider.ping client;
          true
        with ClientProvider.Client_went_away -> false
      in
      if still_there then begin
        Hh_logger.log "Restarting full check due to %s" reason;
        { env with full_check = Full_check_started }
      end else begin
        ClientProvider.shutdown_client client;
        { env with default_client_pending_command_needs_full_check = None }
      end
    | _ -> env
  in
  (* Same as above, but for persistent clients *)
  let env = match env.persistent_client_pending_command_needs_full_check with
    | Some (_command, reason) when env.full_check = Full_check_needed ->
        Hh_logger.log "Restarting full check due to %s" reason;
        { env with full_check = Full_check_started }
    | _ -> env
  in

  (* We have some new, or previously un-processed updates *)
  let full_check = env.full_check = Full_check_started
    (* Prioritize building search index over full rechecks. *)
    && (Queue.is_empty SearchServiceRunner.SearchServiceRunner.queue
      (* Unless there is something actively waiting for this *)
      || Option.is_some env.default_client_pending_command_needs_full_check)
  in
  let lazy_check =
    (not @@ Relative_path.Set.is_empty env.ide_needs_parsing) && is_idle in
  if (not full_check) && (not lazy_check) then
    acc, env
  else begin
    let check_kind = if lazy_check
      then ServerTypeCheck.Lazy_check
      else ServerTypeCheck.Full_check
    in
    let env, res = recheck genv env check_kind in

    let acc = {
      updates_stale = acc.updates_stale;
      rechecked_batches = acc.rechecked_batches + 1;
      rechecked_count = acc.rechecked_count + res.ServerTypeCheck.reparse_count;
      total_rechecked_count = acc.total_rechecked_count + res.ServerTypeCheck.total_rechecked_count;
    } in
    (* Avoid batching ide rechecks with disk rechecks - there might be
      * other ide edits to process first and we want to give the main loop
      * a chance to process them first.
      * Similarly, if a recheck was interrupted because of arrival of command
      * that needs writes, break the recheck loop to give that command chance
      * to be handled in main loop *)
    if lazy_check || Option.is_some env.pending_command_needs_writes
      then acc, env else
      recheck_loop acc genv env new_client has_persistent_connection_request
  end

let recheck_loop genv env client has_persistent_connection_request =
    let stats, env = recheck_loop empty_recheck_loop_stats genv env client
      has_persistent_connection_request in
    { env with recent_recheck_loop_stats = stats }

let new_serve_iteration_id () =
  Random_id.short_string ()

 (* This is safe to run only in the main loop, when workers are not doing
  * anything. *)
let main_loop_command_handler client_kind client result  =
  match result with
  | ServerUtils.Done env ->  env
  | ServerUtils.Needs_full_recheck (env, f, reason) ->
    begin match client_kind with
    | `Non_persistent ->
      (* We should not accept any new clients until this is cleared *)
      assert (Option.is_none
        env.default_client_pending_command_needs_full_check);
      { env with
        default_client_pending_command_needs_full_check =
          Some (f, reason, client)
      }
    | `Persistent ->
      (* Persistent client will not send any further commands until previous one
       * is handled. *)
      assert (Option.is_none
        env.persistent_client_pending_command_needs_full_check);
      { env with
        persistent_client_pending_command_needs_full_check = Some (f, reason)
      }
    end
  | ServerUtils.Needs_writes (env, f, _) -> f env

let has_pending_disk_changes genv =
  match genv.notifier_async_reader () with
  | Some reader when Buffered_line_reader.is_readable reader -> true
  | _ -> false

let serve_one_iteration genv env client_provider =
  let recheck_id = new_serve_iteration_id () in
  ServerMonitorUtils.exit_if_parent_dead ();
  let client_kind =
    (* If we are already blocked on some client, do not accept more of them.
     * Other clients (that connect through priority pipe, or persistent clients)
     * can still be handled *)
    if Option.is_some env.default_client_pending_command_needs_full_check
    then `Priority else `Any in
  let client, has_persistent_connection_request =
    ClientProvider.sleep_and_check
      client_provider
      env.persistent_client
      ~ide_idle:env.ide_idle
      client_kind
  in
  (* client here is "None" if we should either handle from our existing  *)
  (* persistent client (i.e. has_persistent_connection_request), or if   *)
  (* there's nothing to handle. It's "Some ..." if we should handle from *)
  (* a new client.                                                       *)
  let env = if not has_persistent_connection_request && client = None
  then begin
    let last_stats = env.recent_recheck_loop_stats in
    (* Ugly hack: We want GC_SHAREDMEM_RAN to record the last rechecked
     * count so that we can figure out if the largest reclamations
     * correspond to massive rebases. However, the logging call is done in
     * the SharedMem module, which doesn't know anything about Server stuff.
     * So we wrap the call here. *)
    HackEventLogger.with_rechecked_stats
      last_stats.rechecked_batches
      last_stats.rechecked_count
      last_stats.total_rechecked_count
      (fun () -> SharedMem.collect `aggressive);
    let t = Unix.gettimeofday () in
    if t -. env.last_idle_job_time > 0.5 then begin
      ServerIdle.go ();
      { env with last_idle_job_time = t }
    end else env
  end else env in
  let start_t = Unix.gettimeofday () in
  let stage = if env.init_env.needs_full_init then `Init else `Recheck in
  HackEventLogger.with_id ~stage recheck_id @@ fun () ->
  let env = recheck_loop genv env client
    has_persistent_connection_request in
  let stats = env.recent_recheck_loop_stats in
  if stats.total_rechecked_count > 0 then begin
    HackEventLogger.recheck_end start_t
      stats.rechecked_batches
      stats.rechecked_count
      stats.total_rechecked_count;
    Hh_logger.log "Recheck id: %s" recheck_id;
  end;

  let env = Option.value_map env.diag_subscribe
      ~default:env
      ~f:begin fun sub ->

    let client = Utils.unsafe_opt env.persistent_client in
    (* We possibly just did a lot of work. Check the client again to see
     * that we are still idle before proceeding to send diagnostics *)
    if ClientProvider.client_has_message client then env else
    (* We processed some edits but didn't recheck them yet. *)
    if not @@ Relative_path.Set.is_empty env.ide_needs_parsing then env else
    if has_pending_disk_changes genv then env else

    let sub, errors = Diagnostic_subscription.pop_errors sub env.errorl in

    if not @@ SMap.is_empty errors then begin
      let id = Diagnostic_subscription.get_id sub in
      let res = ServerCommandTypes.DIAGNOSTIC (id, errors) in
      try
        ClientProvider.send_push_message_to_client client res
      with ClientProvider.Client_went_away ->
        (* Leaving cleanup of this condition to handled_connection function *)
        ()
    end;
    { env with diag_subscribe = Some sub }
  end in

  let env = match client with
  | None -> env
  | Some client -> begin
    try
      (* client here is the new client (not the existing persistent client) *)
      (* whose request we're going to handle.                               *)
      let env =
        handle_connection genv env client `Non_persistent |>
          main_loop_command_handler `Non_persistent client
      in
      HackEventLogger.handled_connection start_t;
      env
    with
    | e ->
      HackEventLogger.handle_connection_exception e;
      Hh_logger.log "Handling client failed. Ignoring.";
      env
  end in
  let has_persistent_connection_request =
    (* has_persistent_connection_request means that at the beginning of this
     * iteration of main loop there was a request to read and handle.
     * We'll now try to do it, but it's possible that we have ran a recheck
     * in-between  those two events, and if this recheck was non-blocking, we
     * might have already handled this command there. Proceeding to
     * handle_connection would then block reading a request that is not there
     * anymore, so we need to check and update has_persistent_connection_request
     * again. *)
    Option.value_map env.persistent_client
      ~f:ClientProvider.has_persistent_connection_request
      ~default:false
  in
  let env = if has_persistent_connection_request then
    let client = Utils.unsafe_opt env.persistent_client in
    (* client here is the existing persistent client *)
    (* whose request we're going to handle.          *)
    HackEventLogger.got_persistent_client_channels start_t;
    (try
      let env =
        handle_connection genv env client `Persistent |>
        main_loop_command_handler `Persistent client
      in
      HackEventLogger.handled_persistent_connection start_t;
      env
    with
    | e ->
      HackEventLogger.handle_persistent_connection_exception e;
      Hh_logger.log "Handling persistent client failed. Ignoring.";
      env)
  else env in
  let env = match env.pending_command_needs_writes with
    | Some f ->
      { (f env) with
        pending_command_needs_writes = None
      }
    | None -> env
  in
  let env = match env.persistent_client_pending_command_needs_full_check with
    | Some (f, _reason) when env.full_check = Full_check_done ->
      { (f env) with
        persistent_client_pending_command_needs_full_check = None
      }
    | _ -> env
  in
  let env = match env.default_client_pending_command_needs_full_check with
    | Some (f, _reason, _client) when env.full_check = Full_check_done ->
      { (f env) with
        default_client_pending_command_needs_full_check = None
      }
    | _ -> env
  in
  env

let watchman_interrupt_handler genv env =
  let t = Unix.gettimeofday () in
  let env, updates, updates_stale = query_notifier genv env `Async t in
  (* Async updates can always be stale, so we don't care *)
  ignore updates_stale;
  let size = Relative_path.Set.cardinal updates in
  if size > 0 then begin
    Hh_logger.log
      "Interrupted by Watchman message: %d files changed" size;
    { env with disk_needs_parsing =
        Relative_path.Set.union env.disk_needs_parsing updates },
    MultiThreadedCall.Cancel
  end else
    env, MultiThreadedCall.Continue

let priority_client_interrupt_handler genv client_provider env =
  let t = Unix.gettimeofday () in
  (* For non-persistent clients that don't synchronize file contents, users
   * expect that a query they do immediately after saving a file will reflect
   * this file contents. Async notifications are not always fast enough to
   * quarantee it, so we need an additional sync query before accepting such
   * client *)
  let env, updates, _ = query_notifier genv env `Sync t in

  let size = Relative_path.Set.cardinal updates in
  if size > 0 then begin
    Hh_logger.log
      "Interrupted by Watchman sync query: %d files changed" size;
    { env with disk_needs_parsing =
        Relative_path.Set.union env.disk_needs_parsing updates },
    MultiThreadedCall.Cancel
  end else

  let client, has_persistent_connection_request =
    ClientProvider.sleep_and_check
      client_provider
      env.persistent_client
      ~ide_idle:env.ide_idle
      `Priority
  in
  (* we should only be looking at new priority clients, not existing persistent
   * connection *)
  assert (not has_persistent_connection_request);
  let env = match client with
    (* This is possible because client might have went away during
     * sleep_and_check. *)
    | None -> env
    | Some client -> match handle_connection genv env client `Non_persistent with
      | ServerUtils.Needs_full_recheck _ ->
        failwith "unexpected command needing full recheck in priority channel"
      | ServerUtils.Needs_writes _ ->
        failwith "unexpected command needing writes in priority channel"
      | ServerUtils.Done env -> env
  in
  env, MultiThreadedCall.Continue

let persistent_client_interrupt_handler genv env =
  match env.persistent_client with
  (* Several handlers can become ready simultaneously and one of them can remove
   * the persistent client before we get to it. *)
  | None -> env, MultiThreadedCall.Continue
  | Some client -> match handle_connection genv env client `Persistent with
    | ServerUtils.Needs_full_recheck (env, f, reason) ->
      (* This should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one. *)
      assert (Option.is_none
        env.persistent_client_pending_command_needs_full_check);
      { env with
        persistent_client_pending_command_needs_full_check = Some (f, reason)
      },
      MultiThreadedCall.Continue
    | ServerUtils.Needs_writes (env, f, should_restart_recheck) ->
      let full_check = match env.full_check with
        | Full_check_started when not should_restart_recheck ->
            Full_check_needed
        | x -> x
      in
      (* this should not be possible, because persistent client will not send
       * the next command before receiving results from the previous one *)
      assert (Option.is_none env.pending_command_needs_writes);
      { env with
        pending_command_needs_writes = Some f;
        full_check;
      }, MultiThreadedCall.Cancel
    | ServerUtils.Done env -> env, MultiThreadedCall.Continue

let setup_interrupts env client_provider = { env with
  interrupt_handlers = fun genv env ->
    let {ServerLocalConfig.interrupt_on_watchman; interrupt_on_client; _ }
      = genv.local_config in
    let interrupt_on_watchman = interrupt_on_watchman && env.can_interrupt in
    let interrupt_on_client = interrupt_on_client && env.can_interrupt in
    let handlers = match genv.notifier_async_reader () with
      | Some reader when interrupt_on_watchman ->
        [Buffered_line_reader.get_fd reader, watchman_interrupt_handler genv]
      | _ -> []
    in
    let handlers = match ClientProvider.priority_fd client_provider with
      | Some fd when interrupt_on_client ->
        (fd, priority_client_interrupt_handler genv client_provider)::handlers
      | _ -> handlers
    in
    let handlers =
        match env.persistent_client >>= ClientProvider.get_client_fd with
      | Some fd when interrupt_on_client ->
        (fd, persistent_client_interrupt_handler genv)::handlers
      | _ -> handlers
    in
    handlers
}

let serve genv env in_fds =
  (* During server lifetime dependency table can be not up-to-date. Because of
   * that, we ban access to it be default, forcing the code trying to read it to
   * take it into account, either by explcitely enabling reads (and being fine
   * with stale results), or declaring (in ServerCommand) that it requires full
   * check to be completed before being executed. *)
  let _ : bool = Typing_deps.allow_dependency_table_reads false in
  let () = Errors.set_allow_errors_in_default_path false in
  MultiThreadedCall.on_exception ServerUtils.exit_on_exception;
  let client_provider = ClientProvider.provider_from_file_descriptors in_fds in
  (* This is needed when typecheck_after_init option is disabled. *)
  if not env.init_env.needs_full_init then finalize_init genv env.init_env;
  let env = setup_interrupts env client_provider in
  let env = ref env in
  while true do
    let new_env = serve_one_iteration genv !env client_provider in
    env := new_env
  done

let resolve_init_approach genv =
  if not genv.local_config.ServerLocalConfig.use_mini_state then
    None, "Local_config_mini_state_disabled"
  else if ServerArgs.no_load genv.options then
    None, "Server_args_no_load"
  else if ServerArgs.save_filename genv.options <> None then
    None, "Server_args_saving_state"
  else
    match
      (genv.local_config.ServerLocalConfig.load_state_natively),
      (ServerArgs.with_mini_state genv.options) with
      | false, None ->
        None, "No_native_loading_or_precomputed"
      | true, None ->
        (** Use native loading only if the config specifies a load script,
         * and the local config prefers native. *)
        let use_canary = ServerArgs.load_state_canary genv.options in
        Some (ServerInit.Load_state_natively use_canary), "Load_state_natively"
      | _, Some (ServerArgs.Informant_induced_mini_state_target target) ->
        Some (ServerInit.Load_state_natively_with_target target), "Load_state_natively_with_target"
      | _, Some (ServerArgs.Mini_state_target_info target) ->
        Some (ServerInit.Precomputed target), "Precomputed"

let program_init genv =
  let load_mini_approach, approach_name = resolve_init_approach genv in
  Hh_logger.log "Initing with approach: %s" approach_name;
  let env, init_type, init_error, state_distance =
    match load_mini_approach with
    | None ->
      let env, _ = ServerInit.init genv in
      env, "fresh", None, None
    | Some load_mini_approach ->
      let env, init_result = ServerInit.init ~load_mini_approach genv in
      begin match init_result with
        | ServerInit.Mini_load distance -> env, "mini_load", None, distance
        | ServerInit.Mini_load_failed err -> env, "mini_load_failed", Some err, None
      end
  in
  let env = { env with
    init_env = { env.init_env with
      state_distance;
      approach_name;
      init_error;
      init_type;
    }
  } in
  let timeout = genv.local_config.ServerLocalConfig.load_mini_script_timeout in
  EventLogger.set_init_type init_type;
  HackEventLogger.init_end ~state_distance ~approach_name ~init_error init_type timeout;
  Hh_logger.log "Waiting for daemon(s) to be ready...";
  genv.wait_until_ready ();
  ServerStamp.touch_stamp ();
  let informant_use_xdb = genv.local_config.ServerLocalConfig.informant_use_xdb in
  HackEventLogger.init_lazy_end ~informant_use_xdb ~state_distance ~approach_name
    ~init_error init_type;
  env

let setup_server ~informant_managed ~monitor_pid options handle =
  let init_id = Random_id.short_string () in
  Hh_logger.log "Version: %s" Build_id.build_id_ohai;
  Hh_logger.log "Hostname: %s" (Unix.gethostname ());
  let root = ServerArgs.root options in
  ServerDynamicView.toggle := ServerArgs.dynamic_view options;
  (* The OCaml default is 500, but we care about minimizing the memory
   * overhead *)
  let gc_control = Gc.get () in
  Gc.set {gc_control with Gc.max_overhead = 200};
  let config, local_config = ServerConfig.(load filename options) in
  let {ServerLocalConfig.
    cpu_priority;
    io_priority;
    enable_on_nfs;
    search_chunk_size;
    load_script_config;
    max_workers;
    max_bucket_size;
    load_tiny_state;
    use_full_fidelity_parser;
    interrupt_on_watchman;
    interrupt_on_client;
    _
  } as local_config = local_config in
  List.iter (ServerConfig.ignored_paths config) ~f:FilesToIgnore.ignore_path;
  let saved_state_load_type =
    LoadScriptConfig.saved_state_load_type_to_string load_script_config in
  let use_sql =
    LoadScriptConfig.use_sql load_script_config in
  let devinfra_saved_state_lookup = local_config.ServerLocalConfig.devinfra_saved_state_lookup in
  if Sys_utils.is_test_mode ()
  then EventLogger.init ~exit_on_parent_exit EventLogger.Event_logger_fake 0.0
  else HackEventLogger.init
    ~exit_on_parent_exit
    ~root
    ~init_id
    ~informant_managed
    ~devinfra_saved_state_lookup
    ~time:(Unix.gettimeofday ())
    ~saved_state_load_type
    ~use_sql
    ~search_chunk_size
    ~max_workers
    ~max_bucket_size
    ~load_tiny_state
    ~use_full_fidelity_parser
    ~interrupt_on_watchman
    ~interrupt_on_client;
  let root_s = Path.to_string root in
  let check_mode = ServerArgs.check_mode options in
  if not check_mode && Sys_utils.is_nfs root_s && not enable_on_nfs then begin
    Hh_logger.log "Refusing to run on %s: root is on NFS!" root_s;
    HackEventLogger.nfs_root ();
    Exit_status.(exit Nfs_root);
  end;

  Program.preinit ();
  Sys_utils.set_priorities ~cpu_priority ~io_priority;
  (* this is to transform SIGPIPE in an exception. A SIGPIPE can happen when
   * someone C-c the client.
   *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  PidLog.init (ServerFiles.pids_file root);
  Option.iter monitor_pid ~f:(fun monitor_pid -> PidLog.log ~reason:"monitor" monitor_pid);
  PidLog.log ~reason:"main" (Unix.getpid());
  ServerEnvBuild.make_genv options config local_config handle, init_id


let save_state options handle =
  let genv, _ = setup_server ~informant_managed:false ~monitor_pid:None options handle in
  let env = ServerInit.init_to_save_state genv in
  Option.iter (ServerArgs.save_filename genv.options)
    (ServerInit.save_state genv env);
  Hh_logger.log "Running to save saved state";
  Program.run_once_and_exit genv env


let run_once options handle =
  let genv, _ = setup_server ~informant_managed:false ~monitor_pid:None options handle in
  if not (ServerArgs.check_mode genv.options) then
    (Hh_logger.log "ServerMain run_once only supported in check mode.";
    Exit_status.(exit Input_error));
  let env = program_init genv in
  Option.iter (ServerArgs.save_filename genv.options)
    (ServerInit.save_state genv env);
  Hh_logger.log "Running in check mode";
  Program.run_once_and_exit genv env

(*
 * The server monitor will pass client connections to this process
 * via ic.
 *)
let daemon_main_exn ~informant_managed options monitor_pid in_fds =
  Printexc.record_backtrace true;
  let config, _ = ServerConfig.(load filename options) in
  let handle = SharedMem.init (ServerConfig.sharedmem_config config) in
  SharedMem.connect handle ~is_master:true;

  let genv, init_id = setup_server
      ~informant_managed ~monitor_pid:(Some monitor_pid) options handle in
  if ServerArgs.check_mode genv.options then
    (Hh_logger.log "Invalid program args - can't run daemon in check mode.";
    Exit_status.(exit Input_error));
  let env = MainInit.go genv options init_id (fun () -> program_init genv) in
  serve genv env in_fds

let daemon_main (informant_managed, state, options, monitor_pid, priority_in_fd)
  (default_ic, _) =
  (* Avoid leaking this fd further *)
  let () = Unix.set_close_on_exec priority_in_fd in
  let default_in_fd = Daemon.descr_of_in_channel default_ic in
  (* Restore the root directory and other global states from monitor *)
  ServerGlobalState.restore state;
  (* Restore hhi files every time the server restarts
    in case the tmp folder changes *)
  ignore (Hhi.get_hhi_root());

  ServerUtils.with_exit_on_exception @@ fun () ->
  daemon_main_exn ~informant_managed options monitor_pid
    (default_in_fd, priority_in_fd)


let entry =
  Daemon.register_entry_point "ServerMain.daemon_main" daemon_main
