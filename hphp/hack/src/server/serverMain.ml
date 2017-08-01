(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open ServerEnv
open Reordered_argument_collections
open String_utils

(*****************************************************************************)
(* Main initialization *)
(*****************************************************************************)

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
    Hh_logger.log "Server is READY";
    let t' = Unix.gettimeofday () in
    Hh_logger.log "Took %f seconds to initialize." (t' -. t);
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
         Worker.killall ();
         exit (if Errors.is_empty env.errorl then 0 else 1)
      | Some dirname ->
         ServerConvert.go genv env dirname;
         Worker.killall ();
         exit 0

    (* filter and relativize updated file paths *)
    let process_updates genv _env updates =
      let root = Path.to_string @@ ServerArgs.root genv.options in
      (* Because of symlinks, we can have updates from files that aren't in
       * the .hhconfig directory *)
      let updates = SSet.filter updates (fun p -> string_starts_with p root) in
      let updates = Relative_path.(relativize_set Root updates) in
      let to_recheck =
        Relative_path.Set.filter updates begin fun update ->
          ServerEnv.file_filter (Relative_path.suffix update)
        end in
      let genv = if Relative_path.Set.is_empty to_recheck then genv else {
        genv with
        debug_port = Debug_port.write_opt (Debug_event.Disk_files_modified
          (Relative_path.Set.elements to_recheck)) genv.debug_port
      } in
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

let shutdown_persistent_client env client =
  ClientProvider.shutdown_client client;
  ServerFileSync.clear_sync_data env

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

let handle_connection_ genv env client =
  let open ServerCommandTypes in
  try
    match ClientProvider.read_connection_type client with
    | Persistent ->
      let env = match env.persistent_client with
        | Some old_client ->
          ClientProvider.send_push_message_to_client old_client
            NEW_CLIENT_CONNECTED;
          shutdown_persistent_client env old_client
        | None -> env
      in
      ClientProvider.send_response_to_client client Connected;
      { env with persistent_client =
          Some (ClientProvider.make_persistent client)}, None
    | Non_persistent ->
      ServerCommand.handle genv env client
  with
  | ClientProvider.Client_went_away | Read_command_timeout ->
    ClientProvider.shutdown_client client;
    env, None
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
    env, None
  | e ->
    HackEventLogger.handle_connection_exception e;
    let msg = Printexc.to_string e in
    EventLogger.master_exception e;
    Printf.fprintf stderr "Error: %s\n%!" msg;
    Printexc.print_backtrace stderr;
    ClientProvider.shutdown_client client;
    env, None

let handle_persistent_connection_ genv env client =
   try
    let env = { env with ide_idle = false; } in
     ServerCommand.handle genv env client
   with
   (** TODO: Make sure the pipe exception is really about this client. *)
   | Unix.Unix_error (Unix.EPIPE, _, _)
   | Sys_error("Connection reset by peer")
   | Sys_error("Broken pipe")
   | ServerCommandTypes.Read_command_timeout
   | ServerClientProvider.Client_went_away ->
     shutdown_persistent_client env client, None
   | e ->
     let open Marshal_tools in
     let stack = Printexc.get_backtrace () in
     let message = Printexc.to_string e in
     begin try
       ClientProvider.send_push_message_to_client
         client (ServerCommandTypes.FATAL_EXCEPTION { message; stack; })
     with _ -> ()
     end;
     EventLogger.master_exception e;
     Printf.eprintf "Error: %s\n%!" message;
     Printexc.print_backtrace stderr;
     shutdown_persistent_client env client, None

let handle_connection genv env client is_from_existing_persistent_client =
  ServerIdle.stamp_connection ();
  match is_from_existing_persistent_client with
    | true -> handle_persistent_connection_ genv env client
    | false -> handle_connection_ genv env client

let recheck genv old_env check_kind =
  let new_env, to_recheck, total_rechecked =
    ServerTypeCheck.check genv old_env check_kind in
  ServerStamp.touch_stamp_errors (Errors.get_error_list old_env.errorl)
                                 (Errors.get_error_list new_env.errorl);
  new_env, to_recheck, total_rechecked

(* When a rebase occurs, dfind takes a while to give us the full list of
 * updates, and it often comes in batches. To get an accurate measurement
 * of rebase time, we use the heuristic that any changes that come in
 * right after one rechecking round finishes to be part of the same
 * rebase, and we don't log the recheck_end event until the update list
 * is no longer getting populated. *)
let rec recheck_loop ~iteration_flag
acc genv env new_client has_persistent_connection_request =
  let open ServerNotifierTypes in
  let t = Unix.gettimeofday () in
  (** When a new client connects, we use the synchronous notifier.
   * This is to get synchronous file system changes when invoking
   * hh_client in terminal.
   *
   * NB: This also uses synchronous notify on establishing a persistent
   * connection. This is harmless, but could maybe be filtered away. *)
  let env, raw_updates =
    match iteration_flag, new_client, has_persistent_connection_request with
    | Some Force_flush, _, _
    | _, Some _, false -> begin
      env, try Notifier_synchronous_changes (genv.notifier ()) with
      | Watchman.Timeout -> Notifier_unavailable
      end
    | _, None, false when t -. env.last_notifier_check_time > 0.5 ->
      { env with last_notifier_check_time = t; }, genv.notifier_async ()
      (* Do not process any disk changes when there are pending persistent
       * client requests - some of them might be edits, and we don't want to
       * do analysis on mid-edit state of the world *)
    | _, _, true
    | _, None , _->
      env, Notifier_async_changes SSet.empty
  in
  let genv, acc, raw_updates = match raw_updates with
  | Notifier_unavailable ->
    genv, { acc with updates_stale = true; }, SSet.empty
  | Notifier_state_enter (name, _) ->
    let event = (Debug_event.Fresh_vcs_state name) in
    { genv with debug_port = Debug_port.write_opt event genv.debug_port },
    { acc with updates_stale = true; }, SSet.empty
  | Notifier_state_leave _ ->
    genv, { acc with updates_stale = true; }, SSet.empty
  | Notifier_async_changes updates ->
    genv, { acc with updates_stale = true; }, updates
  | Notifier_synchronous_changes updates ->
    genv, { acc with updates_stale = false; }, updates
  in
  let updates = Program.process_updates genv env raw_updates in

  let is_idle = (not has_persistent_connection_request) &&
     (* "average person types [...] between 190 and 200 characters per minute"
      * 60/200 = 0.3 *)
     t -. env.last_command_time > 0.3 in

  let disk_recheck = not (Relative_path.Set.is_empty updates) in
  let ide_recheck =
    (not @@ Relative_path.Set.is_empty env.ide_needs_parsing) && is_idle in
  if (not disk_recheck) && (not ide_recheck) then
    acc, env
  else begin
    HackEventLogger.notifier_returned t (SSet.cardinal raw_updates);
    let disk_needs_parsing =
      Relative_path.Set.union updates env.disk_needs_parsing in

    let env = { env with disk_needs_parsing } in
    let check_kind = if disk_recheck
      then ServerTypeCheck.Full_check
      else ServerTypeCheck.Lazy_check
    in
    let env, rechecked, total_rechecked = recheck genv env check_kind in

    let acc = {
      updates_stale = acc.updates_stale;
      rechecked_batches = acc.rechecked_batches + 1;
      rechecked_count = acc.rechecked_count + rechecked;
      total_rechecked_count = acc.total_rechecked_count + total_rechecked;
    } in
    (* Avoid batching ide rechecks with disk rechecks - there might be
      * other ide edits to process first and we want to give the main loop
      * a chance to process them first. *)
    if ide_recheck then acc, env else
      recheck_loop ~iteration_flag:None
        acc genv env new_client has_persistent_connection_request
  end

let recheck_loop ~iteration_flag
  genv env client has_persistent_connection_request =
    let stats, env = recheck_loop ~iteration_flag
      empty_recheck_loop_stats genv env client
      has_persistent_connection_request in
    { env with recent_recheck_loop_stats = stats }

let new_serve_iteration_id () =
  Random_id.short_string ()

let serve_one_iteration ~iteration_flag genv env client_provider =
  let recheck_id = new_serve_iteration_id () in
  ServerMonitorUtils.exit_if_parent_dead ();
  let client, has_persistent_connection_request =
    ClientProvider.sleep_and_check
      client_provider
      env.persistent_client
      ~ide_idle:env.ide_idle
  in
  (* client here is "None" if we should either handle from our existing  *)
  (* persistent client (i.e. has_persistent_connection_request), or if   *)
  (* there's nothing to handle. It's "Some ..." if we should handle from *)
  (* a new client.                                                       *)
  let has_parsing_hook = !ServerTypeCheck.hook_after_parsing <> None in
  let env = if not has_persistent_connection_request &&
    client = None && not has_parsing_hook
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
  HackEventLogger.with_id ~stage:`Recheck recheck_id @@ fun () ->
  let env = recheck_loop ~iteration_flag genv env client
    has_persistent_connection_request in
  let stats = env.recent_recheck_loop_stats in
  if stats.rechecked_count > 0 then begin
    HackEventLogger.recheck_end start_t has_parsing_hook
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

    let sub, errors = Diagnostic_subscription.pop_errors sub in

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

  let env, next_iteration_flag = match client with
  | None -> env, None
  | Some client -> begin
    try
      (* client here is the new client (not the existing persistent client) *)
      (* whose request we're going to handle.                               *)
      let env, next_iteration_flag = handle_connection genv env client false in
      HackEventLogger.handled_connection start_t;
      env, next_iteration_flag
    with
    | e ->
      HackEventLogger.handle_connection_exception e;
      Hh_logger.log "Handling client failed. Ignoring.";
      env, None
  end in
  if has_persistent_connection_request then
    let client = Utils.unsafe_opt env.persistent_client in
    (* client here is the existing persistent client *)
    (* whose request we're going to handle.          *)
    HackEventLogger.got_persistent_client_channels start_t;
    (try
      let env, next_iteration_flag_2 = handle_connection genv env client true in
      HackEventLogger.handled_persistent_connection start_t;
      env, Option.first_some next_iteration_flag next_iteration_flag_2
    with
    | e ->
      HackEventLogger.handle_persistent_connection_exception e;
      Hh_logger.log "Handling persistent client failed. Ignoring.";
      env, next_iteration_flag)
  else env, next_iteration_flag

let serve genv env in_fd _ =
  let client_provider = ClientProvider.provider_from_file_descriptor in_fd in
  let env = ref env in
  let flag = ref None in
  while true do
    let new_env, next_iteration_flag = serve_one_iteration
      ~iteration_flag:(!flag)
      genv !env client_provider in
    env := new_env;
    flag := next_iteration_flag;
  done

let program_init genv =
  let env, init_type, state_distance =
    (* If we are saving, always start from a fresh state -- just in case
     * incremental mode introduces any errors. *)
    if genv.local_config.ServerLocalConfig.use_mini_state &&
      not (ServerArgs.no_load genv.options) &&
      ServerArgs.save_filename genv.options = None then
      let load_mini_approach = match
        (ServerConfig.load_mini_script genv.config),
        (ServerArgs.with_mini_state genv.options) with
        | None, None ->
          None
        | Some load_mini_script, None ->
          Some (ServerInit.Load_mini_script load_mini_script)
        | None, Some target ->
          Some (ServerInit.Precomputed target)
        | Some _, Some target ->
          Hh_logger.log "Warning - Both a mini script in the server config %s"
            "and a mini state target in server args are configured";
          Hh_logger.log "Ignoring the script and using precomputed target";
          Some (ServerInit.Precomputed target)
      in
      match load_mini_approach with
      | None ->
        let env, _ = ServerInit.init genv in
        env, "fresh", None
      | Some load_mini_approach ->
        let env, init_result = ServerInit.init ~load_mini_approach genv in
        match init_result with
          | ServerInit.Mini_load distance -> env, "mini_load", distance
          | ServerInit.Mini_load_failed err -> env, err, None
    else
      let env, _ = ServerInit.init genv in
      env, "fresh", None
  in
  let timeout = genv.local_config.ServerLocalConfig.load_mini_script_timeout in
  EventLogger.set_init_type init_type;
  HackEventLogger.init_end ~state_distance init_type timeout;
  Hh_logger.log "Waiting for daemon(s) to be ready...";
  genv.wait_until_ready ();
  ServerStamp.touch_stamp ();
  HackEventLogger.init_really_end ~state_distance init_type;
  env

let setup_server ~informant_managed options handle =
  let init_id = Random_id.short_string () in
  Hh_logger.log "Version: %s" Build_id.build_id_ohai;
  Hh_logger.log "Hostname: %s" (Unix.gethostname ());
  let root = ServerArgs.root options in
  (* The OCaml default is 500, but we care about minimizing the memory
   * overhead *)
  let gc_control = Gc.get () in
  Gc.set {gc_control with Gc.max_overhead = 200};
  let config, local_config = ServerConfig.(load filename options) in
  let {ServerLocalConfig.
    cpu_priority;
    io_priority;
    enable_on_nfs;
    lazy_parse;
    lazy_init;
    search_chunk_size;
    load_script_config;
    _
  } as local_config = local_config in
  let saved_state_load_type =
    LoadScriptConfig.saved_state_load_type_to_string load_script_config in
  let use_sql =
    LoadScriptConfig.use_sql load_script_config in
  if Sys_utils.is_test_mode ()
  then EventLogger.init EventLogger.Event_logger_fake 0.0
  else HackEventLogger.init
    root
    init_id
    informant_managed
    (Unix.gettimeofday ())
    lazy_parse
    lazy_init
    saved_state_load_type
    use_sql
    search_chunk_size;
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
  PidLog.log ~reason:"main" (Unix.getpid());
  ServerEnvBuild.make_genv options config local_config handle, init_id

let run_once options handle =
  let genv, _ = setup_server ~informant_managed:false options handle in
  if not (ServerArgs.check_mode genv.options) then
    (Hh_logger.log "ServerMain run_once only supported in check mode.";
    Exit_status.(exit Input_error));
  let env = program_init genv in
  Option.iter (ServerArgs.save_filename genv.options)
    (ServerInit.save_state env);
  Hh_logger.log "Running in check mode";
  Program.run_once_and_exit genv env

(*
 * The server monitor will pass client connections to this process
 * via ic.
 *)
let daemon_main_exn ~informant_managed options (ic, oc) =
  Printexc.record_backtrace true;
  let in_fd = Daemon.descr_of_in_channel ic in
  let out_fd = Daemon.descr_of_out_channel oc in
  let config, _ = ServerConfig.(load filename options) in
  let handle = SharedMem.init (ServerConfig.sharedmem_config config) in
  SharedMem.connect handle ~is_master:true;

  let genv, init_id = setup_server ~informant_managed options handle in
  if ServerArgs.check_mode genv.options then
    (Hh_logger.log "Invalid program args - can't run daemon in check mode.";
    Exit_status.(exit Input_error));
  let env = MainInit.go genv options init_id (fun () -> program_init genv) in
  serve genv env in_fd out_fd

let daemon_main (informant_managed, state, options) (ic, oc) =
  (* Restore the root directory and other global states from monitor *)
  ServerGlobalState.restore state;
  (* Restore hhi files every time the server restarts
    in case the tmp folder changes *)
  ignore (Hhi.get_hhi_root());

  ServerUtils.with_exit_on_exception @@ fun () ->
  daemon_main_exn ~informant_managed options (ic, oc)


let entry =
  Daemon.register_entry_point "ServerMain.daemon_main" daemon_main
