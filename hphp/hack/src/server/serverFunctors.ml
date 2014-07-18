(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open ServerEnv

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type failed_parsing = SSet.t
type files = SSet.t
type client = in_channel * out_channel

type program_ret =
  | Die
  | Continue of env
  | Exit of int

module type SERVER_PROGRAM = sig
  val init : genv -> env -> Path.path -> program_ret
  val recheck: genv -> env -> (SSet.t * SSet.t) -> string list ref -> program_ret
  val infer: (ServerMsg.file_input * int * int) -> out_channel -> unit
  val suggest: string list -> out_channel -> unit
  val parse_options: unit -> ServerArgs.options
  val name: string
  val get_errors: ServerEnv.env -> Errors.t
  val handle_connection : genv -> env -> Unix.file_descr -> unit
end

(*****************************************************************************)
(* Main initialization *)
(*****************************************************************************)

module MainInit (Program : SERVER_PROGRAM) : sig
  val go: genv -> env -> Path.path -> env
end = struct

  let other_server_running() =
    Printf.printf "Error: another server is already running?\n";
    exit 1

  let init_message() =
    Printf.printf "Initializing Server (This might take some time)\n";
    flush stdout

  let grab_lock root =
    if not (Lock.grab root "lock")
    then other_server_running()

  let grab_init_lock root =
    ignore(Lock.grab root "init")

  let release_init_lock root =
    ignore(Lock.release root "init")

  (* This code is only executed when the options --check is NOT present *)
  let main_hh_server_init root=
    grab_lock root;
    init_message();
    ()

  let ready_message() =
    Printf.printf "Server is READY\n";
    flush stdout;
    ()

  let go genv env root =
    let is_check_mode = ServerArgs.check_mode genv.options in
    if not is_check_mode then main_hh_server_init root;
    if not is_check_mode then grab_init_lock root;
    ServerPeriodical.init root;
    if not is_check_mode then ServerDfind.dfind_init root;
    match Program.init genv env root with
    | Exit code -> exit code
    | Die -> die()
    | Continue env ->
        release_init_lock root;
        ready_message ();
        env
end

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

module ServerMain (Program : SERVER_PROGRAM) : sig
  val start : unit -> unit
end = struct
  module MainInit = MainInit (Program)

  let sleep_and_check socket =
    let ready_socket_l, _, _ = Unix.select [socket] [] [] (1.0) in
    ready_socket_l <> []

  (* The main entry point of the daemon
  * the only trick to understand here, is that env.modified is the set
  * of files that changed, it is only set back to SSet.empty when the
  * type-checker succeeded. So to know if there is some work to be done,
  * we look if env.modified changed.
  *)
  let main options =
    SharedMem.init();
    (* this is to transform SIGPIPE in an exception. A SIGPIPE can happen when
    * someone C-c the client.
    *)
    Sys.set_signal Sys.sigpipe Sys.Signal_ignore;
    let root = ServerArgs.root options in
    EventLogger.init root;
    PidLog.init root;
    PidLog.log ~reason:(Some "main") (Unix.getpid());
    let genv = ServerEnvBuild.make_genv ~multicore:true options in
    let env = ServerEnvBuild.make_env options in
    let env = MainInit.go genv env root in
    let socket = Socket.init_unix_socket root in
    EventLogger.init_done ();
    let env = ref env in
    let report = ref [] in
    while true do
      if not (Lock.check root "lock") then begin
        Printf.printf "Lost %s lock; reacquiring.\n" Program.name;
        EventLogger.lock_lost root "lock";
        if not (Lock.grab root "lock")
        then
          Printf.printf "Failed to reacquire lock; terminating.\n";
          EventLogger.lock_stolen root "lock";
          die()
      end;
      ServerHealth.check();
      ServerPeriodical.call_before_sleeping();
      let has_client = sleep_and_check socket in
      let updates = ServerDfind.get_updates genv root in
      (match Program.recheck genv !env updates report with
      | Die -> die ()
      | Exit code -> exit code
      | Continue env' -> env := env');
  (*    report := ServerCheckError.check_first_error !env.errorl; *)
      if has_client then Program.handle_connection genv !env socket;
    done

  let get_log_file root =
    let user = Sys.getenv "USER" in
    let tmp_dir = Tmp.get_dir() in
    let root_part = Path.slash_escaped_string_of_path root in
    Printf.sprintf "%s/%s-%s.log" tmp_dir user root_part

  let daemonize options =
    (* detach ourselves from the parent process *)
    let pid = Unix.fork() in
    if pid == 0
    then begin
      ignore(Unix.setsid());
      (* close stdin/stdout/stderr *)
      let fd = Unix.openfile "/dev/null" [Unix.O_RDONLY; Unix.O_CREAT] 0o777 in
      Unix.dup2 fd Unix.stdin;
      Unix.close fd;
      let file = get_log_file (ServerArgs.root options) in
      (try Sys.rename file (file ^ ".old") with _ -> ());
      let fd = Unix.openfile file [Unix.O_WRONLY; Unix.O_CREAT; Unix.O_APPEND] 0o777 in
      Unix.dup2 fd Unix.stdout;
      Unix.dup2 fd Unix.stderr;
      Unix.close fd;

      (* child process is ready *)
      main options
    end else begin
      (* let original parent exit *)

      Printf.printf "Spawned %s (child pid=%d)\n" (Program.name) pid;
      Printf.printf "Logs will go to %s\n" (get_log_file (ServerArgs.root options));
      flush stdout;
      ()
    end

  let start () =
    let options = Program.parse_options() in
    if ServerArgs.should_detach options
    then daemonize options
    else main options
end
