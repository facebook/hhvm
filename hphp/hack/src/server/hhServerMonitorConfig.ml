(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Responsible for starting up a Hack server process.  *)

module SP = ServerProcess

type pipe_type =
  | Default
  | Priority
  | Force_dormant_start_only

let pipe_type_to_string = function
  | Default -> "default"
  | Priority -> "priority"
  | Force_dormant_start_only -> "force_dormant_start_only"

let start_server_daemon ~informant_managed options log_link daemon_entry =
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
       with _ -> ());
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      Hh_logger.log
        "About to spawn typechecker daemon. Logs will go to %s\n%!"
        ( if Sys.win32 then
          log_file
        else
          log_link );
      let fd = Daemon.fd_of_path log_file in
      (in_fd, fd, fd)
    ) else (
      Hh_logger.log "About to spawn typechecker daemon. Logs will go here.";
      (in_fd, Unix.stdout, Unix.stderr)
    )
  in
  let start_t = Unix.time () in
  let state = ServerGlobalState.save (fun () -> ()) in
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
      ( informant_managed,
        state,
        options,
        monitor_pid,
        child_priority_fd,
        child_force_dormant_start_only_force_fd )
  in
  Unix.close child_priority_fd;
  Unix.close child_force_dormant_start_only_force_fd;
  Hh_logger.log "Just started typechecker server with pid: %d." pid;
  let server =
    SP.
      {
        pid;
        finale_file = ServerFiles.server_finale_file pid;
        in_fd = Daemon.descr_of_in_channel ic;
        out_fds =
          [
            (pipe_type_to_string Default, Daemon.descr_of_out_channel oc);
            (pipe_type_to_string Priority, parent_priority_fd);
            ( pipe_type_to_string Force_dormant_start_only,
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

(*
 * Every server has lazy incremental mode capability, but we want to know
 * whether this particular run actually exercised it.
 *)
let check_log_for_lazy_incremental monitor_config =
  let cmd =
    Printf.sprintf
      "grep -m 1 %s %s | wc -l"
      ServerTypeCheck.(check_kind_to_string Lazy_check)
      monitor_config.ServerMonitorUtils.server_log_file
  in
  try
    match Sys_utils.exec_read cmd with
    | Some "0" -> ()
    | Some "1" -> HackEventLogger.set_lazy_incremental ()
    | Some x -> Hh_logger.log "Unexpected output of command '%s': %s" cmd x
    | None ->
      Hh_logger.log "Unexpected output of command '%s': truncated input" cmd
  with e ->
    Hh_logger.log
      "Exception while running command '%s': %s"
      cmd
      (Printexc.to_string e)

module HhServerConfig = struct
  type server_start_options = ServerArgs.options

  let on_server_exit = check_log_for_lazy_incremental

  let start_server
      ?target_saved_state ~informant_managed ~prior_exit_status options =
    match prior_exit_status with
    | Some c
      when (c = Exit_status.(exit_code Sql_assertion_failure))
           || (c = Exit_status.(exit_code Sql_cantopen))
           || (c = Exit_status.(exit_code Sql_corrupt))
           || c = Exit_status.(exit_code Sql_misuse) ->
      start_hh_server ~informant_managed (ServerArgs.set_no_load options true)
    | _ ->
      let options =
        ServerArgs.set_saved_state_target options target_saved_state
      in
      start_hh_server ~informant_managed options

  let kill_server process =
    try Unix.kill process.ServerProcess.pid Sys.sigusr2
    with _ ->
      Hh_logger.log
        "Failed to send sigusr2 signal to server process. Trying violently";
      (try Unix.kill process.ServerProcess.pid Sys.sigkill
       with e ->
         let stack = Printexc.get_backtrace () in
         Hh_logger.exc
           ~prefix:"Failed to violently kill server process: "
           ~stack
           e)

  let rec wait_for_server_exit process start_t =
    let exit_status =
      Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid
    in
    match exit_status with
    | (0, _) ->
      Unix.sleep 1;
      wait_for_server_exit process start_t
    | _ ->
      ignore
        (Hh_logger.log_duration
           (Printf.sprintf "typechecker has exited. Time since sigterm: ")
           start_t)

  let wait_pid process =
    Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid

  let is_saved_state_precomputed = ServerArgs.is_using_precomputed_saved_state
end
