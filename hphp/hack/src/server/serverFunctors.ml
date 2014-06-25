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
  val recheck: genv -> env -> (SSet.t * SSet.t * bool) -> string list ref -> program_ret
  val infer: (string * int * int) -> out_channel -> unit
  val suggest: string list -> out_channel -> unit
  val parse_options: unit -> ServerArgs.options
  val name: string
  val get_errors: ServerEnv.env -> Errors.t
end

module HackProgram : SERVER_PROGRAM = struct
  let init genv env root =
    (* Completely the wrong way to read hhconfig, but this will be reverted
     * shortly. Ultimately we should build a different options record for Hack
     * and really parse the hhconfig into that, but this functorization should
     * be straightened up a bit first. *)
    let hhconfig = cat_no_fail ((Path.string_of_path root) ^ "/.hhconfig") in
    let enable_hhi_embedding =
      try
        let regex = Str.regexp_string "enable_hhi_embedding" in
        (* This is such a stupid interface. *)
        ignore (Str.search_forward regex hhconfig 0);
        true
      with Not_found -> false in
    let next_files =
      if enable_hhi_embedding then begin
        let next_files_hhi =
          match Hhi.get_hhi_root () with
          | Some hhi_root -> Find.make_next_files_php hhi_root
          | None -> print_endline "Could not locate hhi files"; exit 1 in
        let next_files_root = Find.make_next_files_php root in
        fun () ->
          match next_files_hhi () with
          | [] -> next_files_root ()
          | x -> x
      end else Find.make_next_files_php root in
    match ServerArgs.convert genv.options with
    | None ->
        let env = ServerInit.init genv env next_files in
        if ServerArgs.check_mode genv.options
        then (Exit (if env.errorl = [] then 0 else 1) : program_ret)
        else Continue env
    | Some dirname ->
        ServerConvert.go genv env root next_files dirname;
        Exit 0

  let recheck genv env updates report =
    let diff_php, _, hhconfig_changed = updates in
    if hhconfig_changed then begin
      Printf.printf "hhconfig changed, exiting server to pick up changes\n";
      Die
    end else if not (SSet.is_empty diff_php) then
      let failed_parsing = SSet.union diff_php env.failed_parsing in
      let check_env = { env with failed_parsing = failed_parsing } in
      Continue (ServerTypeCheck.check genv check_env);
    else if !report <> [] then begin
      (* We have a report that the state is inconsistent, at the same
      * time, dfind is telling us that nothing changed between the moment
      * where we produced the report and now. Basically: we have a bug!
      *)
      Printf.printf "SERVER PANIC!!!!!!!!!!!!!\n";
      Printf.printf "*************************************************\n";
      Printf.printf "CRASH REPORT:\n";
      Printf.printf "*************************************************\n";
      List.iter (Printf.printf "%s\n") !report;
      Printf.printf "*************************************************\n";
      Die
    end else Continue env

  let infer = ServerInferType.go

  let suggest _files oc =
    output_string oc "Unimplemented\n";
    flush oc

  let parse_options = ServerArgs.parse_options

  let name = "hh_server"

  let get_errors env = env.errorl
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
(* Code doing the dispatch *)
(*****************************************************************************)

module HandleMessage (Program : SERVER_PROGRAM) : sig

  val respond: genv -> env -> client:client -> msg:ServerMsg.command -> unit

end = struct

  let incorrect_hash oc =
    ServerMsg.response_to_channel oc ServerMsg.SERVER_OUT_OF_DATE;
    EventLogger.out_of_date ();
    Printf.printf     "Status: Error\n";
    Printf.printf     "%s is out of date. Exiting.\n" Program.name;
    exit 4

  let status_log env =
    if List.length (Program.get_errors env) = 0
    then Printf.printf "Status: OK\n"
    else Printf.printf "Status: Error\n";
    flush stdout

  let print_status genv env client_root oc =
    let server_root = ServerArgs.root genv.options in
    if not (Path.equal server_root client_root)
    then begin
      let msg = ServerMsg.DIRECTORY_MISMATCH {ServerMsg.server=server_root; ServerMsg.client=client_root} in
      ServerMsg.response_to_channel oc msg;
      Printf.printf "Status: Error\n";
      Printf.printf "server_dir=%s, client_dir=%s\n"
        (Path.string_of_path server_root)
        (Path.string_of_path client_root);
      Printf.printf "%s is not listening to the same directory. Exiting.\n" Program.name;
      exit 5
    end;
    flush stdout;
    (* TODO: check status.directory *)
    status_log env;
    let errors = Program.get_errors env in
    EventLogger.check_response errors;
    ServerError.send_errorl errors oc

  let die_nicely oc =
    ServerMsg.response_to_channel oc ServerMsg.SERVER_DYING;
    EventLogger.killed ();
    Printf.printf "Status: Error\n";
    Printf.printf "Sent KILL command by client. Dying.\n";
    (match !ServerDfind.dfind_pid with
    | Some pid -> Unix.kill pid Sys.sigterm;
    | None -> failwith "Dfind died before we could kill it"
    );
    die ()

  let respond genv env ~client ~msg =
    let ic, oc = client in
    match msg with
    | ServerMsg.ERROR_OUT_OF_DATE -> incorrect_hash oc
    | ServerMsg.PRINT_TYPES fn -> ServerPrintTypes.go fn genv env ic oc
    | ServerMsg.INFER_TYPE (fn, line, char) ->
        Program.infer (fn, line, char) oc
    | ServerMsg.SUGGEST (files) -> Program.suggest files oc
    | ServerMsg.STATUS client_root -> print_status genv env client_root oc
    | ServerMsg.LIST_FILES    -> ServerEnv.list_files env oc
    | ServerMsg.AUTOCOMPLETE content ->
        ServerAutoComplete.auto_complete env content oc
    | ServerMsg.IDENTIFY_FUNCTION (content, line, char) ->
        ServerIdentifyFunction.go content line char oc
    | ServerMsg.OUTLINE content ->
        ServerFileOutline.go content oc
    | ServerMsg.METHOD_JUMP (class_, find_children) ->
        ServerMethodJumps.go class_ find_children env genv oc
    | ServerMsg.SAVE_STATE filename ->
        let dump = ServerSign.dump_state env genv in
        let status = ServerSign.save dump filename in
        output_string oc (status^"\n");
        flush oc
    | ServerMsg.SHOW name ->
        output_string oc "starting\n";
        output_string oc "class:\n";
        SharedMem.invalidate_caches();
        let qual_name = if name.[0] = '\\' then name
          else ("\\"^name) in
        let nenv = env.nenv in
        (match SMap.get qual_name nenv.Naming.iclasses with
        | Some (p, _) -> output_string oc ((Pos.string p)^"\n")
        | None -> output_string oc "Missing from nenv\n");
        let class_ = Typing_env.Classes.get qual_name in
        (match class_ with
        | None ->
            output_string oc "Missing from Typing_env\n"
        | Some c ->
            let class_str = Typing_print.class_ c in
            output_string oc (class_str^"\n")
        );
        output_string oc "function:\n";
        (match SMap.get qual_name nenv.Naming.ifuns with
        | Some (p, _) -> output_string oc ((Pos.string p)^"\n")
        | None -> output_string oc "Missing from nenv\n");
        let fun_ = Typing_env.Funs.get qual_name in
        (match fun_ with
        | None ->
            output_string oc "Missing from Typing_env\n"
        | Some f ->
            let fun_str = Typing_print.fun_ f in
            output_string oc (fun_str^"\n")
        );
        flush oc
    | ServerMsg.KILL -> die_nicely oc
    | ServerMsg.PING -> ServerMsg.response_to_channel oc ServerMsg.PONG
    | ServerMsg.BUILD build_opts ->
      let build_hook = BuildMain.go build_opts genv env oc in
      ServerTypeCheck.hook_after_parsing := begin fun genv env ->
        (* subtle: an exception there (such as writing on a closed pipe)
         * will not be catched by handle_connection() because
         * we have already returned from handle_connection(), hence
         * this additional try.
         *)
        (try
           build_hook genv env;
           close_out oc;
        with exn ->
          Printf.printf "Exn in build_hook: %s" (Printexc.to_string exn);
        );
        ServerTypeCheck.hook_after_parsing := (fun _ _ -> ())
      end
    | ServerMsg.FIND_REFS find_refs_action ->
        ServerFindRefs.go find_refs_action genv env oc
    | ServerMsg.REFACTOR refactor_action ->
        ServerRefactor.go refactor_action genv env oc
    | ServerMsg.PROLOG ->
      let path = PrologMain.go genv env oc in
      (* the rational for this PROLOG_READY: prefix is that the string
       * passed below corresponds to a command that will ultimately be
       * exec'ed by hh_client, and because we also (ab)use 'oc' to
       * communicate possible error message to the client, it's
       * safer to at least add a prefix.
       *)
      output_string oc ("PROLOG_READY:" ^path);
      flush oc
    | ServerMsg.SEARCH query ->
        ServerSearch.go query oc
end

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

module ServerMain (Program : SERVER_PROGRAM) : sig
  val start : unit -> unit
end = struct
  module MainInit = MainInit (Program)
  module HandleMessage = HandleMessage (Program)

  let sleep_and_check socket =
    let ready_socket_l, _, _ = Unix.select [socket] [] [] (1.0) in
    ready_socket_l <> []

  let handle_connection_ genv env socket =
    let cli, _ = Unix.accept socket in
    let ic = Unix.in_channel_of_descr cli in
    let oc = Unix.out_channel_of_descr cli in
    let client = ic, oc in
    let msg = ServerMsg.cmd_from_channel ic in
    let finished, _, _ = Unix.select [cli] [] [] 0.0 in
    if finished <> [] then () else begin
      ServerPeriodical.stamp_connection();
      match msg with
      | ServerMsg.BUILD _ ->
        (* The build step is special. It closes the socket itself. *)
        HandleMessage.respond genv !env ~client ~msg
      | _ ->
        HandleMessage.respond genv !env ~client ~msg;
        (try Unix.close cli with e ->
          Printf.fprintf stderr "Error: %s\n" (Printexc.to_string e);
          flush stderr);
    end

  let handle_connection genv env socket =
    try handle_connection_ genv env socket
    with
    | Unix.Unix_error (e, _, _) ->
        Printf.printf "Unix error: %s\n" (Unix.error_message e);
        flush stdout
    | e ->
        Printf.printf "Error: %s\n" (Printexc.to_string e);
        flush stdout

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
      if has_client then handle_connection genv env socket;
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
      let file = get_log_file (ServerArgs.root options)in
      (try Sys.remove file with _ -> ());
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
