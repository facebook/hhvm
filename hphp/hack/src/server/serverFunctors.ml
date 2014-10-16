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

module type SERVER_PROGRAM = sig
  val preinit : ServerArgs.options -> unit
  val init : genv -> env -> Path.path -> env
  val run_once_and_exit : genv -> env -> Path.path -> unit
  val recheck: genv -> env -> (SSet.t * SSet.t) -> env
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

module MainInit : sig
  val go: Path.path -> (unit -> env) -> env
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

  let ready_message() =
    Printf.printf "Server is READY\n";
    flush stdout;
    ()

  (* This code is only executed when the options --check is NOT present *)
  let go root init_fun =
    grab_lock root;
    init_message();
    grab_init_lock root;
    ServerPeriodical.init root;
    ServerDfind.dfind_init root;
    let env = init_fun () in
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
  let sleep_and_check socket =
    let ready_socket_l, _, _ = Unix.select [socket] [] [] (1.0) in
    ready_socket_l <> []

  let serve genv env socket root =
    let env = ref env in
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
      env := Program.recheck genv !env updates;
      if has_client then Program.handle_connection genv !env socket;
    done

  let create_program_init genv env root = fun () ->
    match ServerArgs.load_save_opt genv.options with
    | None -> Program.init genv env root
    | Some (ServerArgs.Save fn) ->
        let chan = open_out_no_fail fn in
        let env = Program.init genv env root in
        Marshal.to_channel chan env [];
        close_out_no_fail fn chan;
        (* We cannot save the shared memory to `chan` because the OCaml runtime
         * does not expose the underlying file descriptor to C code; so we use
         * a separate ".sharedmem" file.  *)
        SharedMem.save (fn^".sharedmem");
        env
    | Some (ServerArgs.Load fn) ->
        let chan = open_in_no_fail fn in
        let env = Marshal.from_channel chan in
        close_in_no_fail fn chan;
        SharedMem.load (fn^".sharedmem");
        env

  (* The main entry point of the daemon
  * the only trick to understand here, is that env.modified is the set
  * of files that changed, it is only set back to SSet.empty when the
  * type-checker succeeded. So to know if there is some work to be done,
  * we look if env.modified changed.
  *)
  let main options =
    Program.preinit options;
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
    let program_init = create_program_init genv env root in
    let is_check_mode = ServerArgs.check_mode genv.options in
    if is_check_mode
    then
      let env = program_init () in
      Program.run_once_and_exit genv env root
    else
      let env = MainInit.go root program_init in
      let socket = Socket.init_unix_socket root in
      EventLogger.init_done ();
      serve genv env socket root

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
    end else begin
      (* let original parent exit *)

      Printf.printf "Spawned %s (child pid=%d)\n" (Program.name) pid;
      Printf.printf "Logs will go to %s\n" (get_log_file (ServerArgs.root options));
      flush stdout;
      raise Exit
    end

  let start () =
    let options = Program.parse_options() in
    try
      if ServerArgs.should_detach options
      then daemonize options;
      main options
    with Exit ->
      ()
end
