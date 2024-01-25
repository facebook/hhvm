(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Responsible for starting up a Hack server process.  *)

let start_server_daemon
    ~informant_managed
    options
    log_link
    (daemon_entry : (ServerMain.params, _, _) Daemon.entry) =
  let log_fds =
    let in_fd = Daemon.null_fd () in
    if ServerArgs.should_detach options then (
      (try
         let old_log_name i = Printf.sprintf "%s.%d.old" log_link i in
         let max_n_log_files = 20 in
         for i = max_n_log_files - 1 downto 1 do
           if Sys.file_exists (old_log_name i) then
             Sys.rename (old_log_name i) (old_log_name (i + 1))
         done;
         let old = log_link ^ ".old" in
         if Sys.file_exists old then Sys.rename old (old_log_name 1);
         if Sys.file_exists log_link then Sys.rename log_link old
       with
      | _ -> ());
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      Hh_logger.log
        "About to spawn typechecker daemon. Logs will go to %s\n%!"
        (if Sys.win32 then
          log_file
        else
          log_link);
      let fd = Daemon.fd_of_path log_file in
      (in_fd, fd, fd)
    ) else (
      Hh_logger.log "About to spawn typechecker daemon. Logs will go here.";
      (in_fd, Unix.stdout, Unix.stderr)
    )
  in
  let start_t = Unix.time () in
  let state = ServerGlobalState.save ~logging_init:(fun () -> ()) in
  let monitor_pid = Unix.getpid () in
  (* Setting some additional channels between monitor and server *)
  let (parent_priority_fd, child_priority_fd) =
    Unix.socketpair Unix.PF_UNIX Unix.SOCK_STREAM 0
  in
  let () = Unix.set_close_on_exec parent_priority_fd in
  let () = Unix.clear_close_on_exec child_priority_fd in
  let ( parent_force_dormant_start_only_fd,
        child_force_dormant_start_only_force_fd ) =
    Unix.socketpair Unix.PF_UNIX Unix.SOCK_STREAM 0
  in
  let () = Unix.set_close_on_exec parent_force_dormant_start_only_fd in
  let () = Unix.clear_close_on_exec child_force_dormant_start_only_force_fd in
  let { Daemon.pid; Daemon.channels = (ic, oc) } =
    Daemon.spawn
      ~channel_mode:`socket
      log_fds
      daemon_entry
      {
        ServerMain.informant_managed;
        state;
        options;
        monitor_pid;
        priority_in_fd = child_priority_fd;
        force_dormant_start_only_in_fd = child_force_dormant_start_only_force_fd;
      }
  in
  Unix.close child_priority_fd;
  Unix.close child_force_dormant_start_only_force_fd;
  Hh_logger.log "Just started typechecker server with pid: %d." pid;

  (* We'll write an initial progress message to guarantee that the client will
     certainly be able to read the progress file as soon as it learns the progress filename.
     There's a benign race as to whether our message is written first, or whether the server
     started up quickly enough to write its initial message first. It's benign because
     either message will communicate the right intent to the user, and in any case the server
     will always have further progress updates to write. *)
  ServerProgress.write "starting hh_server";

  let server =
    ServerProcess.
      {
        pid;
        server_specific_files =
          {
            ServerCommandTypes.server_finale_file =
              ServerFiles.server_finale_file pid;
          };
        in_fd = Daemon.descr_of_in_channel ic;
        out_fds =
          [
            ( MonitorRpc.pipe_type_to_string MonitorRpc.Default,
              Daemon.descr_of_out_channel oc );
            ( MonitorRpc.pipe_type_to_string MonitorRpc.Priority,
              parent_priority_fd );
            ( MonitorRpc.pipe_type_to_string MonitorRpc.Force_dormant_start_only,
              parent_force_dormant_start_only_fd );
          ];
        start_t;
        last_request_handoff = ref (Unix.time ());
      }
  in
  server

let start_hh_server ~informant_managed options =
  let log_link = ServerFiles.log_link (ServerArgs.root options) in
  start_server_daemon ~informant_managed options log_link ServerMain.entry

type server_start_options = ServerArgs.options

let start_server ~informant_managed ~prior_exit_status options =
  match prior_exit_status with
  | Some c
    when (c = Exit_status.(exit_code Sql_assertion_failure))
         || (c = Exit_status.(exit_code Sql_cantopen))
         || (c = Exit_status.(exit_code Sql_corrupt))
         || c = Exit_status.(exit_code Sql_misuse) ->
    start_hh_server ~informant_managed (ServerArgs.set_no_load options true)
  | _ -> start_hh_server ~informant_managed options

let kill_server ~violently process =
  if not violently then begin
    Hh_logger.log "kill_server: sending SIGUSR2 to %d" process.ServerProcess.pid;
    try Unix.kill process.ServerProcess.pid Sys.sigusr2 with
    | _ -> ()
  end else begin
    Hh_logger.log
      "Failed to send sigusr2 signal to server process. Trying violently";
    try Unix.kill process.ServerProcess.pid Sys.sigkill with
    | exn ->
      let e = Exception.wrap exn in
      Hh_logger.exception_ ~prefix:"Failed to violently kill server process: " e
  end

let wait_for_server_exit ~(timeout_t : float) process =
  let rec wait_for_server_exit_impl () =
    let now_t = Unix.gettimeofday () in
    if now_t > timeout_t then
      false
    else
      let exit_status =
        Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid
      in
      match exit_status with
      | (0, _) ->
        Unix.sleep 1;
        wait_for_server_exit_impl ()
      | _ -> true
  in
  wait_for_server_exit_impl ()

let wait_pid process =
  Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid

let is_saved_state_precomputed = ServerArgs.is_using_precomputed_saved_state
