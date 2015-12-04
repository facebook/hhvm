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
 * This is a mediator between the typechecker server and the client UI.
 * It ensures that:
   * 1) A client can always open a connection to a socket.
   * 2) A typechecker will be quickly killed even if occupied if the
   *    client's build ID doesn't match.
   * 3) A client UI can readily get feedback even if the typechecker
   *    process is busy.
*)

open ServerUtils
open ServerProcessTools

exception Malformed_build_id
exception Send_fd_failure of int

let fd_to_int (x: Unix.file_descr) : int = Obj.magic x

module Program = struct
  let name = "hh_server"
  (** Seconds since Unix epoch. *)
  let last_request_handoff_timestamp: float ref = ref (Unix.time ())
end

let kill_typechecker typechecker =
  if (typechecker.pid) <> 0 then
    try Unix.kill typechecker.pid Sys.sigusr2 with
    | _ ->
      Hh_logger.log
        "Failed to send Sig_build_id_mismatch signal to typechecker. Trying \
         violently";
      Unix.kill typechecker.pid Sys.sigkill;
  else ()

let rec wait_for_typechecker_exit typechecker start_t =
  if (typechecker.pid) <> 0 then
    match Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED]
        typechecker.pid with
    | 0, _ ->
      Unix.sleep 1;
      wait_for_typechecker_exit typechecker start_t
    | _ ->
      ignore (
        Hh_logger.log_duration
          "Typechecker has exited. Time since sigterm: " start_t)

let setup_handler_for_signals handler signals =
  List.iter begin fun signal ->
    Sys.set_signal signal (Sys.Signal_handle handler)
  end signals

let setup_autokill_typechecker_on_exit typechecker =
  try
    setup_handler_for_signals begin fun _ ->
      Hh_logger.log "Got an exit signal. Killing typechecker and exiting.";
      kill_typechecker typechecker;
      Exit_status.exit Exit_status.Interrupted
    end [Sys.sigint; Sys.sigquit; Sys.sigterm; Sys.sighup];
  with
  | _ ->
    Hh_logger.log "Failed to set signal handler"

let start_hh_server options typechecker_entry log_mode =
  let log_file =
    match log_mode with
    | Daemon.Log_file ->
      let log_link = ServerFiles.log_link (ServerArgs.root options) in
      (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
      let log_file = ServerFiles.make_link_of_timestamped log_link in
      Hh_logger.log "About to spawn typechecker daemon. Logs will go to %s\n%!"
        (if Sys.win32 then log_file else log_link);
      log_file
    | Daemon.Parent_streams ->
      Hh_logger.log "About to spawn typechecker daemon. Logs will go here.";
      ""
  in
  let start_t = Unix.time () in
  let {Daemon.pid; Daemon.channels} =
    Daemon.spawn ~channel_mode:`socket ~log_file ~log_mode typechecker_entry
      options in
  let ic, oc = channels in
  Hh_logger.log "Just started typechecker server with pid: %d." pid;
  let typechecker =
    {
      pid = pid;
      in_fd = Daemon.descr_of_in_channel ic;
      out_fd = Daemon.descr_of_out_channel oc;
      log_file = log_file;
      log_mode = log_mode;
      start_t = start_t;
    } in
  setup_autokill_typechecker_on_exit typechecker;
  typechecker

let sleep_and_check socket =
  let ready_socket_l, _, _ = Unix.select [socket] [] [] (1.0) in
  ready_socket_l <> []

(** Kill command from client is handled by typechecker server, so the monitor
 * needs to check liveness of the typechecker process to know whether
 * to stop itself. *)
let stop_if_typechecker_dead typechecker =
  let pid, proc_stat =
    Unix.waitpid [Unix.WNOHANG; Unix.WUNTRACED] typechecker.pid in
  (if pid <> 0 then
     (check_exit_status proc_stat typechecker;
      Exit_status.(exit Ok))
   else
     ())

let read_build_id_ohai fd =
  let client_build_id: string = Marshal_tools.from_fd_with_preamble fd in
  let newline_byte = String.create 1 in
  let _ = Unix.read fd newline_byte 0 1 in
  if newline_byte <> "\n" then
    (Hh_logger.log "Did not find newline character after build_id ohai";
     raise Malformed_build_id);
  client_build_id

let hand_off_client_connection typechecker client_fd =
  let status = Libancillary.ancil_send_fd typechecker.out_fd client_fd in
  if (status <> 0) then
    (Hh_logger.log "Failed to handoff FD to typechecker.";
     raise (Send_fd_failure status))
  else
    (Unix.close client_fd;
     ())

(** Sends the client connection FD to the typechecker process then closes the
 * FD. *)
let rec hand_off_client_connection_with_retries typechecker retries client_fd =
  let _, ready_l, _ = Unix.select [] [typechecker.out_fd] [] (0.5) in
  if ready_l <> [] then
    try hand_off_client_connection typechecker client_fd
    with
    | e ->
      if retries > 0 then
        (Hh_logger.log "Retrying FD handoff";
         hand_off_client_connection_with_retries
           typechecker (retries - 1) client_fd)
      else
        (Hh_logger.log "No more retries. Ignoring request.";
         HackEventLogger.send_fd_failure e;
         Unix.close client_fd;)
  else if retries > 0 then
    (Hh_logger.log "typechecker socket not yet ready. Retrying.";
     hand_off_client_connection_with_retries
       typechecker (retries - 1) client_fd)
  else
    (Hh_logger.log
       "typechecker socket not yet ready. No more retries. Ignoring request.";)

(** Does not return. *)
let client_out_of_date_ client_channel =
  msg_to_channel client_channel Build_id_mismatch;
  HackEventLogger.out_of_date ()

(** Kills typechecker, sends build ID mismatch message to client, and exits.
 *
 * Does not return. Exits after waiting for typechecker process to exit. So
 * the client can wait for socket closure as indication that both the monitor
 * and typechecker have exited.
*)
let client_out_of_date typechecker client_channel =
  kill_typechecker typechecker;
  let kill_signal_time = Unix.gettimeofday () in
  (** If we detect out of date client, should always kill typechecker and exit
   * monitor, even if messaging to channel or event logger fails. *)
  (try client_out_of_date_ client_channel with
   | e -> Hh_logger.log
       "Handling client_out_of_date threw with: %s" (Printexc.to_string e));
  wait_for_typechecker_exit typechecker kill_signal_time;
  Exit_status.exit Exit_status.Build_id_mismatch

(** Send (possibly empty) sequences of messages before handing off to
 * typechecker. *)
let client_prehandoff typechecker fd =
  msg_to_channel (Unix.out_channel_of_descr fd)
    Prehandoff_sentinel;
  hand_off_client_connection_with_retries typechecker 8 fd;
  HackEventLogger.client_connection_sent ();
  Program.last_request_handoff_timestamp := Unix.time ()

let ack_and_handoff_client typechecker fd =
  (** Output back to the client can be safely done with Ocaml channels, but
   * not input. See also Marshal_tools.ml. *)
  let channel = Unix.out_channel_of_descr fd in
  try
    let client_build_id = read_build_id_ohai fd in
    if client_build_id <> Build_id.build_id_ohai
    then
      client_out_of_date typechecker channel
    else (
      let since_last_request =
        (Unix.time ()) -. (!Program.last_request_handoff_timestamp) in
      (** TODO: Send this to client so it is visible. *)
      Hh_logger.log "Got request. Prior request %.1f seconds ago"
        since_last_request;
      msg_to_channel channel ServerUtils.Connection_ok;
      client_prehandoff typechecker fd
    )
  with
  | Marshal_tools.Malformed_Preamble_Exception ->
    (** TODO: Remove this after 2 Hack deploys. *)
    (Hh_logger.log "
        Marshal tools read malformed preamble, interpreting as version change.
        ";
     client_out_of_date typechecker channel)
  | Malformed_build_id as e ->
    HackEventLogger.malformed_build_id ();
    Hh_logger.log "Malformed Build ID";
    raise e

let check_and_run_loop (_: ServerArgs.options) typechecker
    (lock_file: string) (socket: Unix.file_descr) _ =
  if not (Lock.grab lock_file) then
    (Hh_logger.log "Lost lock; terminating.\n%!";
     HackEventLogger.lock_stolen lock_file;
     Exit_status.(exit Lock_stolen));
  stop_if_typechecker_dead typechecker;
  let has_client = sleep_and_check socket in
  if (not has_client) then
    ()
  else
  try
    let fd, _ = Unix.accept socket in
    try
      HackEventLogger.accepted_client_fd (fd_to_int fd);
      ack_and_handoff_client typechecker fd
    with
    | e ->
      (HackEventLogger.ack_and_handoff_exception e;
       Hh_logger.log
         "Handling client connection failed. Ignoring connection attempt.";
       Unix.close fd)
  with
  | e ->
    (HackEventLogger.accepting_on_socket_exception e;
     Hh_logger.log
       "Accepting on socket failed. Ignoring client connection attempt.")

(** Main method of the server monitor daemon. The daemon is responsible for
 * listening to socket requests from hh_client, checking Build ID, and relaying
 * requests to the typechecker process. *)
let monitor_daemon_main (options: ServerArgs.options) typechecker_entry
    typechecker_log_mode =
  if Sys_utils.is_test_mode ()
  then EventLogger.init (Daemon.devnull ()) 0.0
  else HackEventLogger.init_monitor (ServerArgs.root options)
      (Unix.gettimeofday ());
  if not Sys.win32 then Sys.set_signal Sys.sigpipe Sys.Signal_ignore;
  let www_root = (ServerArgs.root options) in
  ignore @@ Sys_utils.setsid ();
  (** Make sure to lock the lockfile before doing *anything*, especially
   * opening the socket. *)
  let lock_file = ServerFiles.lock_file www_root in
  if not (Lock.grab lock_file) then
    (Hh_logger.log "Monitor daemon already running. Killing";
     Exit_status.exit Exit_status.Ok);
  let socket = Socket.init_unix_socket (ServerFiles.socket_file www_root) in
  let typechecker = start_hh_server options typechecker_entry
      typechecker_log_mode in
  while true do
    try check_and_run_loop
      options typechecker lock_file socket typechecker_entry
    with
    | _ -> ()
  done

(* Starts a monitor daemon if one doesn't already exist. Otherwise,
 * immediately exits with non-zero exit code. This is because the monitor
 * should never actually be attempted to be started if one is already running
 * (i.e. hh_client should play nice and only start a server monitor if one
 * isn't running by first checking the liveness lock file.) *)
let daemon_starter options daemon_entry typechecker_entry =
  let www_root = (ServerArgs.root options) in
  let log_link = ServerFiles.server_monitor_log_link www_root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  let log_file_path = ServerFiles.make_link_of_timestamped log_link in
  let {Daemon.pid; _} = Daemon.spawn ~log_file:log_file_path
      daemon_entry (options, typechecker_entry) in
  Printf.eprintf "Spawned %s (child pid=%d)\n" Program.name pid;
  Printf.eprintf "Logs will go to %s\n%!"
    (if Sys.win32 then log_file_path else log_link);
  Exit_status.Ok

let typechecker_entry =
  Daemon.register_entry_point
    "main"
    (fun options
      (** Must explicitly provide provide phantom types since they will
       * never be inferred because they're immediately stripped away. *)
      ((ic: char Daemon.in_channel), (oc: char Daemon.out_channel)) ->
      let in_fd = Daemon.descr_of_in_channel ic in
      let out_fd = Daemon.descr_of_out_channel oc in
      ServerMain.daemon_main options in_fd out_fd)

let daemon_entry =
  Daemon.register_entry_point
    "monitor_daemon_main"
    (fun (
       (options: ServerArgs.options), typechecker_entry)
       ((_ic: char Daemon.in_channel), (_oc: char Daemon.out_channel)) ->
       monitor_daemon_main options typechecker_entry Daemon.Log_file)

(** Either starts a monitor daemon (which will spawn a typechecker daemon),
 * or just runs the typechecker if detachment not enabled. *)
let start () =
  Daemon.check_entry_point (); (* this call might not return *)
  let options = ServerArgs.parse_options () in
  if ServerArgs.should_detach options
  then
    Exit_status.exit (daemon_starter options daemon_entry typechecker_entry)
  else
    monitor_daemon_main options typechecker_entry Daemon.Parent_streams
