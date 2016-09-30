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
    ServerArgs.options ->
    string ->
    (unit -> env) ->    (* init function to run while we have init lock *)
    env
end = struct
  (* This code is only executed when the options --check is NOT present *)
  let go options init_id init_fun =
    let root = ServerArgs.root options in
    let t = Unix.gettimeofday () in
    Hh_logger.log "Initializing Server (This might take some time)";
    (* note: we only run periodical tasks on the root, not extras *)
    ServerIdle.init root;
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
      HackSearchService.attach_hooks ();

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

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

let handle_connection_ genv env client =
  let open ServerCommandTypes in
  try
    match ClientProvider.read_connection_type client with
    | Persistent ->
      (match env.persistent_client with
      | Some _ ->
        ClientProvider.send_response_to_client client
          Persistent_client_alredy_exists;
        env
      | None ->
        ClientProvider.send_response_to_client client
          Persistent_client_connected;
        { env with persistent_client =
            Some (ClientProvider.make_persistent client)})
    | Non_persistent ->
      ServerCommand.handle genv env client
  with
  | ClientProvider.Client_went_away | Read_command_timeout ->
    ClientProvider.shutdown_client client;
    env
  | e ->
    HackEventLogger.handle_connection_exception e;
    let msg = Printexc.to_string e in
    EventLogger.master_exception msg;
    Printf.fprintf stderr "Error: %s\n%!" msg;
    Printexc.print_backtrace stderr;
    ClientProvider.shutdown_client client;
    env

let handle_persistent_connection_ genv env client =
   try
     ServerCommand.handle genv env client
   with
   | Sys_error("Broken pipe") | ServerCommandTypes.Read_command_timeout ->
     ClientProvider.shutdown_client client;
     {env with
     persistent_client = None;
     edited_files = Relative_path.Map.empty;
     diag_subscribe = None;}
   | e ->
     let msg = Printexc.to_string e in
     EventLogger.master_exception msg;
     Printf.fprintf stderr "Error: %s\n%!" msg;
     Printexc.print_backtrace stderr;
     ClientProvider.shutdown_client client;
     {env with
     persistent_client = None;
     edited_files = Relative_path.Map.empty;
     diag_subscribe = None;}

let handle_connection genv env client is_persistent =
  ServerIdle.stamp_connection ();
  try match is_persistent with
    | true -> handle_persistent_connection_ genv env client
    | false -> handle_connection_ genv env client
  with
  | Unix.Unix_error (e, _, _) ->
     Printf.fprintf stderr "Unix error: %s\n" (Unix.error_message e);
     Printexc.print_backtrace stderr;
     flush stderr;
     env
  | e ->
     Printf.fprintf stderr "Error: %s\n" (Printexc.to_string e);
     Printexc.print_backtrace stderr;
     flush stderr;
     env

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
let rec recheck_loop acc genv env new_client =
  let open ServerNotifierTypes in
  let t = Unix.gettimeofday () in
  (** When a new client connects, we use the synchronous notifier.
   * This is to get synchronous file system changes when invoking
   * hh_client in terminal.
   *
   * NB: This also uses synchronous notify on establishing a persistent
   * connection. This is harmless, but could maybe be filtered away. *)
  let raw_updates = match new_client with
  | Some _ -> begin
    try Notifier_synchronous_changes (genv.notifier ()) with
    | Watchman.Timeout -> Notifier_unavailable
    end
  | None ->
    genv.notifier_async ()
  in
  let acc, raw_updates = match raw_updates with
  | Notifier_unavailable ->
    { acc with updates_stale = true; }, SSet.empty
  | Notifier_async_changes updates ->
    { acc with updates_stale = true; }, updates
  | Notifier_synchronous_changes updates ->
    { acc with updates_stale = false; }, updates
  in
  let updates = Program.process_updates genv env raw_updates in

  let is_idle = t -. env.last_command_time > 0.5 in

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
    recheck_loop acc genv env new_client
  end

let recheck_loop genv env client =
  let stats, env = recheck_loop empty_recheck_loop_stats genv env client in
  { env with recent_recheck_loop_stats = stats }

let new_serve_iteration_id () =
  Random_id.short_string ()

let serve_one_iteration genv env client_provider =
  let recheck_id = new_serve_iteration_id () in
  ServerMonitorUtils.exit_if_parent_dead ();
  let client, has_persistent =
    ClientProvider.sleep_and_check client_provider env.persistent_client in
  let has_parsing_hook = !ServerTypeCheck.hook_after_parsing <> None in
  if not has_persistent && client = None && not has_parsing_hook
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
      ServerIdle.go;
  end;
  let start_t = Unix.gettimeofday () in
  HackEventLogger.with_id ~stage:`Recheck recheck_id @@ fun () ->
  let env = recheck_loop genv env client in
  let stats = env.recent_recheck_loop_stats in
  if stats.rechecked_count > 0 then begin
    HackEventLogger.recheck_end start_t has_parsing_hook
      stats.rechecked_batches
      stats.rechecked_count
      stats.total_rechecked_count;
    Hh_logger.log "Recheck id: %s" recheck_id;
  end;

  Option.iter env.diag_subscribe ~f:begin fun sub ->
    let id = Diagnostic_subscription.get_id sub in
    let errors = Diagnostic_subscription.get_absolute_errors sub in
    if not @@ SMap.is_empty errors then begin
      let res = ServerCommandTypes.DIAGNOSTIC (id, errors) in
      let client = Utils.unsafe_opt env.persistent_client in
      ClientProvider.send_push_message_to_client client res
    end
  end;

  let env = { env with diag_subscribe =
    Option.map env.diag_subscribe ~f:Diagnostic_subscription.mark_as_pushed }
  in

  let env = match client with
  | None -> env
  | Some client -> begin
    try
      let env = handle_connection genv env client false in
      HackEventLogger.handled_connection start_t;
      env
    with
    | e ->
      HackEventLogger.handle_connection_exception e;
      Hh_logger.log "Handling client failed. Ignoring.";
      env
  end in
  if has_persistent then
    let client = Utils.unsafe_opt env.persistent_client in
    HackEventLogger.got_persistent_client_channels start_t;
    (try
      let env = handle_connection genv env client true in
      HackEventLogger.handled_persistent_connection start_t;
      env
    with
    | e ->
      HackEventLogger.handle_persistent_connection_exception e;
      Hh_logger.log "Handling persistent client failed. Ignoring.";
      env)
  else env

let serve genv env in_fd _ =
  let client_provider = ClientProvider.provider_from_file_descriptor in_fd in
  let env = ref env in
  while true do
    env := serve_one_iteration genv !env client_provider;
  done

let program_init genv =
  let env, init_type =
    (* If we are saving, always start from a fresh state -- just in case
     * incremental mode introduces any errors. *)
    if genv.local_config.ServerLocalConfig.use_mini_state &&
      not (ServerArgs.no_load genv.options) &&
      ServerArgs.save_filename genv.options = None then
      match ServerConfig.load_mini_script genv.config with
      | None ->
        let env, _ = ServerInit.init genv in
        env, "fresh"
      | Some load_mini_script ->
        let env, did_load = ServerInit.init ~load_mini_script genv in
        env, if did_load then "mini_load" else "mini_load_fail"
    else
      let env, _ = ServerInit.init genv in
      env, "fresh"
  in
  HackEventLogger.init_end init_type;
  Hh_logger.log "Waiting for daemon(s) to be ready...";
  genv.wait_until_ready ();
  ServerStamp.touch_stamp ();
  HackEventLogger.init_really_end init_type;
  env

let setup_server options handle =
  let init_id = Random_id.short_string () in
  Hh_logger.log "Version: %s" Build_id.build_id_ohai;
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
    lazy_decl;
    lazy_parse;
    load_script_config;
    _
  } as local_config = local_config in
  let saved_state_load_type =
    LoadScriptConfig.saved_state_load_type_to_string load_script_config in
  if Sys_utils.is_test_mode ()
  then EventLogger.init (Daemon.devnull ()) 0.0
  else HackEventLogger.init
    root
    init_id
    (Unix.gettimeofday ())
    lazy_decl
    lazy_parse
    saved_state_load_type;
  let root_s = Path.to_string root in
  if Sys_utils.is_nfs root_s && not enable_on_nfs then begin
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
  let genv, _ = setup_server options handle in
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
let daemon_main_exn options (ic, oc) =
  let in_fd = Daemon.descr_of_in_channel ic in
  let out_fd = Daemon.descr_of_out_channel oc in
  let config, _ = ServerConfig.(load filename options) in
  let handle = SharedMem.init (ServerConfig.sharedmem_config config) in
  SharedMem.connect handle ~is_master:true;

  let genv, init_id = setup_server options handle in
  if ServerArgs.check_mode genv.options then
    (Hh_logger.log "Invalid program args - can't run daemon in check mode.";
    Exit_status.(exit Input_error));
  let env = MainInit.go options init_id (fun () -> program_init genv) in
  serve genv env in_fd out_fd

let daemon_main (state, options) (ic, oc) =
  (* Restore the root directory and other global states from monitor *)
  ServerGlobalState.restore state;
  (* Restore hhi files every time the server restarts
    in case the tmp folder changes *)
  ignore (Hhi.get_hhi_root());

  try daemon_main_exn options (ic, oc)
  with
  | SharedMem.Out_of_shared_memory ->
    ServerInit.print_hash_stats ();
    Printf.eprintf "Error: failed to allocate in the shared heap.\n%!";
    Exit_status.(exit Out_of_shared_memory)
  | SharedMem.Hash_table_full ->
    ServerInit.print_hash_stats ();
    Printf.eprintf "Error: failed to allocate in the shared hashtable.\n%!";
    Exit_status.(exit Hash_table_full)
  | Decl_class.Decl_heap_elems_bug ->
    Exit_status.(exit Decl_heap_elems_bug)


let entry =
  Daemon.register_entry_point "ServerMain.daemon_main" daemon_main
