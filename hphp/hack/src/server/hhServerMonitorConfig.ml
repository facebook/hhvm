(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(** Responsible for starting up a Hack server process.  *)

module SP = ServerProcess

module Program = struct
  let hh_server = "typechecker"
end

let start_server_daemon ~informant_managed options name log_link daemon_entry =
  let log_fds =
    let in_fd = Daemon.null_fd () in
    if ServerArgs.should_detach options then begin
      (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      Hh_logger.log "About to spawn %s daemon. Logs will go to %s\n%!"
        name (if Sys.win32 then log_file else log_link);
      let fd = Daemon.fd_of_path log_file in
      in_fd, fd, fd
    end else begin
      Hh_logger.log "About to spawn %s daemon. Logs will go here." name;
      in_fd, Unix.stdout, Unix.stderr
    end
  in
  let start_t = Unix.time () in
  let state = ServerGlobalState.save () in
  let {Daemon.pid; Daemon.channels = (ic, oc)} =
    Daemon.spawn
      ~channel_mode:`socket
      log_fds
      daemon_entry
      (informant_managed, state, options) in
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

let start_hh_server ~informant_managed options =
  let log_link = ServerFiles.log_link (ServerArgs.root options) in
  start_server_daemon ~informant_managed options
    Program.hh_server log_link ServerMain.entry


(*
 * Every server has lazy incremental mode capability, but we want to know
 * whether this particular run actually exercised it.
 *)
let check_log_for_lazy_incremental monitor_config =
  let cmd = Printf.sprintf "grep -m 1 %s %s | wc -l"
    ServerTypeCheck.(check_kind_to_string Lazy_check)
    monitor_config.ServerMonitorUtils.server_log_file
  in
  try
    match Sys_utils.exec_read cmd with
    | "0" -> ()
    | "1" -> HackEventLogger.set_lazy_incremental ()
    | x -> Hh_logger.log "Unexpected output of command '%s': %s" cmd x
  with e ->
    Hh_logger.log "Exception while running command '%s': %s"
      cmd (Printexc.to_string e)

module HhServerConfig = struct

  type server_start_options = ServerArgs.options

  let on_server_exit = check_log_for_lazy_incremental

  let start_server ?target_mini_state ~informant_managed ~prior_exit_status options =
    match prior_exit_status with
    | Some c
      when c = Exit_status.(exit_code Sql_assertion_failure) ||
           c = Exit_status.(exit_code Sql_cantopen) ||
           c = Exit_status.(exit_code Sql_corrupt) ||
           c = Exit_status.(exit_code Sql_misuse) ->
      start_hh_server ~informant_managed (ServerArgs.set_no_load options true)
    | _ ->
      let options = ServerArgs.set_mini_state_target options target_mini_state in
      start_hh_server ~informant_managed options

  let kill_server process =
    try Unix.kill process.ServerProcess.pid Sys.sigusr2 with
    | _ -> Hh_logger.log
        "Failed to send sigusr2 signal to server process. Trying \
         violently";
      try Unix.kill process.ServerProcess.pid Sys.sigkill with e ->
        Hh_logger.exc ~prefix: "Failed to violently kill server process: " e

  let rec wait_for_server_exit process start_t =
    let exit_status = Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid in
    match exit_status with
    | 0, _ ->
      Unix.sleep 1;
      wait_for_server_exit process start_t
    | _ ->
      ignore (
        Hh_logger.log_duration (Printf.sprintf
          "%s has exited. Time since sigterm: " process.ServerProcess.name) start_t)

  let wait_pid process =
    Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] process.ServerProcess.pid

end
