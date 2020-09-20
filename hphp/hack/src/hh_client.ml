(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Hack for HipHop: type checker's client code.
 *
 * This code gets called in various different ways:
 * - from emacs, where the output is asynchronous
 * - from vim, where vim is blocked until this code exits
 * - from arc diff, our linter
 * - from arc land, our commit hook
 * - from check trunk, our irc bot which checks the state of trunk
 * - manually, from the command line
 *
 * Usage: hh_client [OPTION]... [WWW DIRECTORY] [FILE]...
 *
 *)

open Hh_prelude

let () = Random.self_init ()

let () =
  (* no-op, needed at entry-point for Daemon hookup *)
  Daemon.check_entry_point ();

  (* Ignore SIGPIPE since we might get a server hangup and don't care (can
   * detect and handle better than a signal). Ignore SIGUSR1 since we sometimes
   * use that for the server to tell us when it's done initializing, but if we
   * aren't explicitly listening we don't care. *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  Sys_utils.set_signal
    Sys.sigint
    (Sys.Signal_handle (fun _ -> raise Exit_status.(Exit_with Interrupted)));
  let init_id = Random_id.short_string () in
  let command = ClientArgs.parse_args ~init_id in
  let command_name =
    match command with
    | ClientCommand.CCheck _ -> "Check"
    | ClientCommand.CStart _ -> "Start"
    | ClientCommand.CStop _ -> "Stop"
    | ClientCommand.CRestart _ -> "Restart"
    | ClientCommand.CLsp _ -> "Lsp"
    | ClientCommand.CDebug _ -> "Debug"
    | ClientCommand.CDownloadSavedState _ -> "DownloadSavedState"
    | ClientCommand.CRage _ -> "Rage"
  in

  (* Set up logging. *)
  let root = ClientArgs.root command in
  HackEventLogger.client_init
    ~init_id
    ~custom_columns:(ClientCommand.get_custom_telemetry_data command)
    (Option.value root ~default:Path.dummy_path);
  Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
  Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
  Hh_logger.set_id (Printf.sprintf "%s#%s" command_name init_id);
  begin
    match root with
    | None -> ()
    | Some root ->
      let client_log_fn = ServerFiles.client_log root in
      (try
         (* For irritating reasons T67177821 we might not have permissions
         to write to the file. Pending a fix, let's only set up Hh_logger
         to write to the file if we can indeed safely write to it. *)
         Sys_utils.touch
           (Sys_utils.Touch_existing_or_create_new
              { mkdir_if_new = false; perm_if_new = 0o666 })
           client_log_fn;
         Hh_logger.set_log client_log_fn
       with _ -> ())
  end;
  Hh_logger.log
    "[hh_client] %s"
    (String.concat ~sep:" " (Array.to_list Sys.argv));

  try
    let exit_status =
      match command with
      | ClientCommand.CCheck check_env ->
        Lwt_main.run (ClientCheck.main check_env)
      | ClientCommand.CStart env -> Lwt_main.run (ClientStart.main env)
      | ClientCommand.CStop env -> Lwt_main.run (ClientStop.main env)
      | ClientCommand.CRestart env -> Lwt_main.run (ClientRestart.main env)
      | ClientCommand.CLsp env -> Lwt_main.run (ClientLsp.main env)
      | ClientCommand.CDebug env -> Lwt_main.run (ClientDebug.main env)
      | ClientCommand.CRage env -> Lwt_main.run (ClientRage.main env)
      | ClientCommand.CDownloadSavedState env ->
        Lwt_main.run (ClientDownloadSavedState.main env)
    in
    Exit.exit exit_status
  with exn ->
    let e = Exception.wrap exn in
    (* We trust that if someone raised Exit_with then they had the decency to print
    out a user-facing message; we will only print out a user-facing message here
    for uncaught exceptions: lvl=Error gets sent to stderr, but lvl=Info doesn't. *)
    let (es, lvl) =
      match exn with
      | Exit_status.Exit_with es -> (es, Hh_logger.Level.Info)
      | _ -> (Exit_status.Uncaught_exception, Hh_logger.Level.Error)
    in
    Hh_logger.log
      ~lvl
      "hh_client bad exit: %s - %s\n%s"
      (Exit_status.show es)
      (Exception.get_ctor_string e)
      (Exception.get_backtrace_string e |> Exception.clean_stack);
    HackEventLogger.client_bad_exit ~command_name es e;
    Exit.exit es
