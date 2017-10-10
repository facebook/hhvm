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

module Program = HhServerMonitorConfig.Program
module SP = ServerProcess
module SM = ServerMonitor.Make_monitor
  (HhServerMonitorConfig.HhServerConfig) (HhMonitorInformant);;

let make_tmp_dir () =
  let tmpdir = Path.make (Tmp.temp_dir GlobalConfig.tmp_dir "files") in
  Relative_path.set_path_prefix Relative_path.Tmp tmpdir


(** Main method of the server monitor daemon. The daemon is responsible for
 * listening to socket requests from hh_client, checking Build ID, and relaying
 * requests to the typechecker process. *)
let monitor_daemon_main (options: ServerArgs.options) =
  let init_id = Random_id.short_string () in
  if Sys_utils.is_test_mode ()
  then EventLogger.init EventLogger.Event_logger_fake 0.0
  else HackEventLogger.init_monitor (ServerArgs.root options) init_id
      (Unix.gettimeofday ());
  Utils.profile := ServerArgs.profile_log options;
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
  ignore (make_tmp_dir());
  ignore (Hhi.get_hhi_root());
  Relative_path.set_path_prefix Relative_path.Root www_root;

  let config, local_config  =
    ServerConfig.(load filename options) in
  if local_config.ServerLocalConfig.incremental_init then
    HackEventLogger.set_incremental_init ();
  HackEventLogger.set_search_chunk_size
    local_config.ServerLocalConfig.search_chunk_size;

  HackSearchService.fuzzy := local_config.ServerLocalConfig.enable_fuzzy_search;
  if ServerArgs.save_filename options <> None
    && ServerArgs.use_gen_deps options then
    let shared_config = ServerConfig.(sharedmem_config config) in
    let handle = SharedMem.init shared_config in
    SharedMem.connect handle ~is_master:true;
    ServerMain.save_state options handle
  else if ServerArgs.check_mode options then
    let shared_config = ServerConfig.(sharedmem_config config) in
    let handle = SharedMem.init shared_config in
    SharedMem.connect handle ~is_master:true;
    ServerMain.run_once options handle
  else
    let waiting_client = ServerArgs.waiting_client options in
    let informant_options = {
      HhMonitorInformant.root = ServerArgs.root options;
      state_prefetcher = ServerConfig.(state_prefetcher_script config)
        |> State_prefetcher.of_script_opt;
      allow_subscriptions = local_config.ServerLocalConfig.watchman_subscribe;
      use_dummy = local_config.ServerLocalConfig.use_dummy_informant;
      min_distance_restart =
        local_config.ServerLocalConfig.informant_min_distance_restart;
      use_xdb = local_config.ServerLocalConfig.informant_use_xdb;
    } in
    let max_purgatory_clients =
      local_config.ServerLocalConfig.max_purgatory_clients in
    SM.start_monitoring ~waiting_client
      ~max_purgatory_clients
      options informant_options
      ServerMonitorUtils.({
        socket_file = ServerFiles.socket_file www_root;
        lock_file = ServerFiles.lock_file www_root;
        server_log_file = ServerFiles.log_link www_root;
        monitor_log_file = ServerFiles.monitor_log_link www_root;
        load_script_log_file = ServerFiles.load_log www_root;
      })

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
let start_daemon options =
  let root = ServerArgs.root options in
  let log_link = ServerFiles.monitor_log_link root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  let log_file_path = Sys_utils.make_link_of_timestamped log_link in
  let in_fd = Daemon.null_fd () in
  let out_fd = Daemon.fd_of_path log_file_path in
  let {Daemon.pid; _} =
    Daemon.spawn (in_fd, out_fd, out_fd) daemon_entry options in
  Printf.eprintf "Spawned %s (child pid=%d)\n" Program.hh_server pid;
  Printf.eprintf "Logs will go to %s\n%!" log_file_path;
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
    then Exit_status.exit (start_daemon options)
    else monitor_daemon_main options
  with
  | SharedMem.Out_of_shared_memory ->
      Exit_status.(exit Out_of_shared_memory)
