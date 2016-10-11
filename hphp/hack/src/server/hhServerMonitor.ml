(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(**
 * Hack for HipHop: type checker's server monitor code.
 *
 * This runs hh server in 1 of 3 ways:
   1) Runs hh server in-process (check mode)
   2) Runs a ServerMonitor in-process which will pass connections to a
      daemonized hh server (non-detached mode).
   3) Daemonizes a ServerMonitor which will pass connections to a daemonized
      hh_server (detached mode).
*)

open HhServerMonitorConfig
module SP = ServerProcess

let start_server options name log_link daemon_entry =
  let log_fds =
    if ServerArgs.should_detach options then begin
      (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      Hh_logger.log "About to spawn %s daemon. Logs will go to %s\n%!"
        name (if Sys.win32 then log_file else log_link);
      let fd = Daemon.fd_of_path log_file in
      fd, fd
    end else begin
      Hh_logger.log "About to spawn %s daemon. Logs will go here." name;
      Unix.stdout, Unix.stderr
    end
  in
  let start_t = Unix.time () in
  let state = ServerGlobalState.save () in
  let {Daemon.pid; Daemon.channels = (ic, oc)} =
    Daemon.spawn
      ~channel_mode:`socket
      log_fds
      daemon_entry
      (state, options) in
  Hh_logger.log "Just started %s server with pid: %d." name pid;
  let server =
    SP.({
      pid = pid;
      name = name;
      in_fd = Daemon.descr_of_in_channel ic;
      out_fd = Daemon.descr_of_out_channel oc;
      start_t = start_t;
      last_request_handoff = ref (Unix.time());
    }) in
  server

let start_hh_server options  =
  let log_link = ServerFiles.log_link (ServerArgs.root options) in
  start_server options Program.hh_server log_link ServerMain.entry


(** Main method of the server monitor daemon. The daemon is responsible for
 * listening to socket requests from hh_client, checking Build ID, and relaying
 * requests to the typechecker process. *)
let monitor_daemon_main (options: ServerArgs.options) =
  let init_id = Random_id.short_string () in
  if Sys_utils.is_test_mode ()
  then EventLogger.init EventLogger.Event_logger_fake 0.0
  else HackEventLogger.init_monitor (ServerArgs.root options) init_id
      (Unix.gettimeofday ());
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;

  let www_root = (ServerArgs.root options) in

  if not (ServerArgs.check_mode options) then begin
    (** Make sure to lock the lockfile before doing *anything*, especially
     * opening the socket. *)
    let lock_file = ServerFiles.lock_file www_root in
    if not (Lock.grab lock_file) then
      (Hh_logger.log "Monitor daemon already running. Killing";
       Exit_status.exit Exit_status.No_error);
  end;

  ignore @@ Sys_utils.setsid ();
  ignore (Hhi.get_hhi_root());
  Relative_path.set_path_prefix Relative_path.Root www_root;

  let config, local_config  =
   ServerConfig.(load filename options) in
  HackEventLogger.set_lazy_levels
   (local_config.ServerLocalConfig.lazy_decl)
   (local_config.ServerLocalConfig.lazy_parse);

  Parsing_hooks.fuzzy := local_config.ServerLocalConfig.enable_fuzzy_search;
  if ServerArgs.check_mode options then
    let shared_config = ServerConfig.(sharedmem_config config) in
    let handle = SharedMem.init shared_config in
    SharedMem.connect handle ~is_master:true;
    ServerMain.run_once options handle
  else
    let hh_server_monitor_starter = begin fun () ->
      let typechecker = start_hh_server options in
      [typechecker]
    end in
    let waiting_client = ServerArgs.waiting_client options in
    ServerMonitor.start_monitoring ~waiting_client ServerMonitorUtils.({
      socket_file = ServerFiles.socket_file www_root;
      lock_file = ServerFiles.lock_file www_root;
      server_log_file = ServerFiles.log_link www_root;
      monitor_log_file = ServerFiles.monitor_log_link www_root;
      load_script_log_file = ServerFiles.load_log www_root;
    }) hh_server_monitor_starter

let daemon_entry =
  Daemon.register_entry_point
    "monitor_daemon_main"
    (fun (options: ServerArgs.options) (_ic, _oc) ->
       monitor_daemon_main options)

(* Starts a monitor daemon if one doesn't already exist. Otherwise,
 * immediately exits with non-zero exit code. This is because the monitor
 * should never actually be attempted to be started if one is already running
 * (i.e. hh_client should play nice and only start a server monitor if one
 * isn't running by first checking the liveness lock file.) *)
let daemon_starter options =
  let root = ServerArgs.root options in
  let log_link = ServerFiles.monitor_log_link root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  let log_file_path = Sys_utils.make_link_of_timestamped log_link in
  let fd = Daemon.fd_of_path log_file_path in
  let {Daemon.pid; _} =
    Daemon.spawn (fd, fd) daemon_entry options in
  Printf.eprintf "Spawned %s (child pid=%d)\n" Program.hh_server pid;
  Printf.eprintf "Logs will go to %s\n%!"
    (if Sys.win32 then log_file_path else log_link);
  Exit_status.No_error

(** Either starts a monitor daemon (which will spawn a typechecker daemon),
 * or just runs the typechecker if detachment not enabled. *)
let start () =
  (* TODO: Catch all exceptions that make it this high, log them, and exit with
   * the proper code *)
  try
    Daemon.check_entry_point (); (* this call might not return *)
    let options = ServerArgs.parse_options () in
    if ServerArgs.should_detach options
    then Exit_status.exit (daemon_starter options)
    else monitor_daemon_main options
  with
  | SharedMem.Out_of_shared_memory ->
      Exit_status.(exit Out_of_shared_memory)
