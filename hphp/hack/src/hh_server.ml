(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(**
 * Hack for HipHop: type checker daemon code.
 *
 * See README for high level overview.
 *
 * Interesting files/directory:
 * - hh_server.ml:       contains mostly the ugly inotify code.
 *
 * Parser code:
 * The parser is written using lex & yacc. It only supports a subset of PHP by
 * design.
 * - parsing/lexer.mll:  the lexer (lex)
 * - parsing/parser.mly: the parser (yacc)
 *
 * Naming:
 * Naming consists in "solving" all the names (making sure every
 * class/variable/etc. are bound).
 * - naming/nast.ml:
 *   Named abstract syntax tree (the datastructure).
 * - naming/naming.ml:
 *   Naming operations (takes an ast and returns a nast).
 * - naming/nastInitCheck.ml:
 *   Checks that all the members in a class are always properly initialized.
 *
 * Typing:
 * - typing/typing_defs.ml:
 *   The datastructures required for typing.
 * - typing/typing_env.ml:
 *   All the operations on the typing environment (e.g. unifying types).
 * - typing/typing_reasong.ml:
 *   Documents why something has a given type (witness system).
 * - typing/typing.ml:
 *   Where everything happens, in two phases:
 *   1. type declarations: we build the environment where we know the type of
 *      everything. (see make_env).
 *   2. for every function and method definition, we check that their
 *      implementation matches their signature (assuming that all other
 *      signatures are correct).
 *)
open Utils
open ServerEnv

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type failed_parsing = SSet.t
type files = SSet.t
type client = in_channel * out_channel

(*****************************************************************************)
(* Main initialization *)
(*****************************************************************************)

module MainInit: sig
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
  let main_hh_server_init genv root=
    grab_lock root;
    init_message();
    ()

  let ready_message() =
    Printf.printf "Server is READY\n";
    flush stdout;
    ()

  let go genv env root =
    let is_flow = ServerArgs.is_flow genv.options in
    let is_check_mode = ServerArgs.check_mode genv.options in
    if not is_check_mode then main_hh_server_init genv root;
    if not is_check_mode then grab_init_lock root;
    ServerPeriodical.init root;
    if not is_check_mode then ServerDfind.dfind_init root;
    if is_flow 
    then 
      let env = Flow.init genv env root in
      if is_check_mode
      then die()
      else release_init_lock root;
      ready_message();
      env
    else 
      let next_files = Find.make_next_files_php root in
      match ServerArgs.convert genv.options with
      | None ->
          let env = ServerInit.init genv env next_files in
          if is_check_mode
          then die()
          else release_init_lock root;
          ready_message();
          env
      | Some dirname ->
          ServerConvert.go genv env root next_files dirname;
          exit 0

end

(*****************************************************************************)
(* Code doing the dispatch *)
(*****************************************************************************)

module HandleMessage: sig

  val respond: genv -> env -> client:client -> msg:ServerMsg.command -> unit

end = struct

  let incorrect_hash oc =
    ServerMsg.response_to_channel oc ServerMsg.SERVER_OUT_OF_DATE;
    EventLogger.out_of_date ();
    Printf.printf     "Status: Error\n";
    Printf.printf     "hh_server is out of date. Exiting.\n";
    exit 4

  let status_log env =
    if List.length env.errorl = 0
    then Printf.printf "Status: OK\n"
    else Printf.printf "Status: Error\n";
    flush stdout

  let print_status genv env client_root ic oc =
    let server_root = ServerArgs.root genv.options in
    if not (Path.equal server_root client_root)
    then begin
      let msg = ServerMsg.DIRECTORY_MISMATCH {ServerMsg.server=server_root; ServerMsg.client=client_root} in
      ServerMsg.response_to_channel oc msg;
      Printf.printf "Status: Error\n";
      Printf.printf "server_dir=%s, client_dir=%s\n"
        (Path.string_of_path server_root)
        (Path.string_of_path client_root);
      Printf.printf "hh_server is not listening to the same directory. Exiting.\n";
      exit 5
    end;
    flush stdout;
    (* TODO: check status.directory *)
    status_log env;
    EventLogger.check_response env.errorl;
    ServerError.send_errorl env.errorl oc

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
        ServerInferType.go (fn, line, char) oc
    | ServerMsg.STATUS client_root -> print_status genv env client_root ic oc
    | ServerMsg.SKIP          -> assert false
    | ServerMsg.LIST_FILES    -> ServerEnv.list_files env oc
    | ServerMsg.AUTOCOMPLETE content ->
        ServerAutoComplete.auto_complete env content oc
    | ServerMsg.IDENTIFY_FUNCTION (content, line, char) ->
        ServerIdentifyFunction.go content line char oc
    | ServerMsg.OUTLINE content ->
        ServerFileOutline.go content oc
    | ServerMsg.SAVE_STATE filename ->
        let dump = ServerSign.dump_state env genv in
        let status = ServerSign.save dump filename in
        output_string oc (status^"\n");
        flush oc
    | ServerMsg.SHOW classname ->
        output_string oc "starting\n";
        output_string oc "class:\n";
        SharedMem.invalidate_caches();
        let nenv = env.nenv in
        (match SMap.get classname nenv.Naming.iclasses with
        | Some (p, _) -> output_string oc ((Pos.string p)^"\n")
        | None -> output_string oc "Missing from nenv\n");
        let class_ = Typing_env.Classes.get classname in
        (match class_ with
        | None ->
            output_string oc "Missing from Typing_env\n"
        | Some c ->
            let class_str = Typing_print.class_ c in
            output_string oc (class_str^"\n")
        );
        output_string oc "function:\n";
        (match SMap.get classname nenv.Naming.ifuns with
        | Some (p, _) -> output_string oc ((Pos.string p)^"\n")
        | None -> output_string oc "Missing from nenv\n");
        let fun_ = Typing_env.Funs.get classname in
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
        build_hook genv env;
        close_out oc;
        ServerTypeCheck.hook_after_parsing := (fun _ _ -> ())
      end
    | ServerMsg.FIND_REFS find_refs_action ->
        ServerFindRefs.go find_refs_action genv env oc
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
end

(*****************************************************************************)
(* The main loop *)
(*****************************************************************************)

let sleep_and_check socket =
  let ready_socket_l, _, _ = Unix.select [socket] [] [] (1.0) in
  ready_socket_l <> []

let recheck genv env updates report =
  let diff_php, diff_js = updates in
  if not (SSet.is_empty diff_php) 
  then
    let failed_parsing = SSet.union diff_php !env.failed_parsing in
    let check_env = { !env with failed_parsing = failed_parsing } in
    env := ServerTypeCheck.check genv check_env;
  else 
    if !report <> []
      then begin
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
        die()
      end

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
    | ServerMsg.SKIP ->
      Printf.printf "THIS IS BAD: Skipping errors!\n"; flush stdout;
      env := { !env with errorl = [];
               failed_parsing = SSet.empty;
               failed_check = SSet.empty};
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
  let root = ServerArgs.root options in
  EventLogger.init root;
  PidLog.init root;
  PidLog.log ~reason:(Some "main") (Unix.getpid());
  let genv = ServerEnvBuild.make_genv ~multicore:true options in
  let env = ServerEnvBuild.make_env options in
  let env = MainInit.go genv env root in
  let socket = Socket.init_unix_socket root in
  EventLogger.init_done ();
  env.skip := false;
  let env = ref env in
  let report = ref [] in
  while true do
    if not (Lock.check root "lock") then begin
      Printf.printf "Lost hh_server lock; reacquiring.\n";
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
    if ServerArgs.is_flow options  
    then Flow.recheck genv env updates report 
    else recheck genv env updates report;
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

    Printf.printf "Spawned hh_server (child pid=%d)\n" pid;
    Printf.printf "Logs will go to %s\n" (get_log_file (ServerArgs.root options));
    flush stdout;
    ()
  end

let () =
  let options = ServerArgs.parse_options() in
  if ServerArgs.should_detach options
  then daemonize options
  else main options
