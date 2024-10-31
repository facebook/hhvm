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
 *)

open Hh_prelude

let () = Random.self_init ()

let init_event_logger
    root
    (command : ClientCommand.heavy_command)
    ~init_id
    ~from
    (config : ServerConfig.t)
    (local_config : ServerLocalConfig.t) : unit =
  HackEventLogger.client_init
    ~init_id
    ~from
    ~is_interactive:(ClientArgs.is_interactive command)
    ~custom_columns:(ClientCommand.get_custom_telemetry_data command)
    root;
  HackEventLogger.set_hhconfig_version
    (ServerConfig.version config |> Config_file.version_to_string_opt);
  HackEventLogger.set_rollout_group local_config.ServerLocalConfig.rollout_group;
  HackEventLogger.set_rollout_flags
    (ServerLocalConfigLoad.to_rollout_flags local_config);
  ()

let set_up_signals () =
  (* Ignore SIGPIPE since if it arises from clientConnect then it might indicate server hangup;
     we detect this case already and handle it better than a signal (unhandled signals cause program exit). *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  Sys_utils.set_signal
    Sys.sigint
    (Sys.Signal_handle (fun _ -> raise Exit_status.(Exit_with Interrupted)));
  ()

let set_up_logger ~command_name ~init_id ~root =
  (* We'll chose where Hh_logger.log gets sent *)
  Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
  Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
  Hh_logger.set_id (Printf.sprintf "%s#%s" command_name init_id);
  let client_log_fn = ServerFiles.client_log root in
  try
    (* For irritating reasons T67177821 we might not have permissions
       to write to the file. Pending a fix, let's only set up Hh_logger
       to write to the file if we can indeed safely write to it. *)
    Sys_utils.touch
      (Sys_utils.Touch_existing_or_create_new
         { mkdir_if_new = false; perm_if_new = 0o666 })
      client_log_fn;
    Hh_logger.set_log client_log_fn
  with
  | _ -> ()

let set_up_root root =
  Relative_path.set_path_prefix Relative_path.Root root;
  Server_progress.set_root root;
  ()

let exit_status_of_exn exn =
  let e = Exception.wrap exn in
  let es =
    match exn with
    | Exit_status.Exit_with es -> es
    | _ -> Exit_status.Uncaught_exception e
  in
  (es, e)

let handle_exn_and_exit exn ~command_name =
  let (es, e) = exit_status_of_exn exn in
  (* hide the spinner *)
  ClientSpinner.report ~to_stderr:false ~angery_reaccs_only:false None;
  (* We trust that if someone raised Exit_with then they had the decency to print
     out a user-facing message; we will only print out a user-facing message here
     for uncaught exceptions: lvl=Error gets sent to stderr, but lvl=Info doesn't. *)
  let lvl =
    match exn with
    | Exit_status.Exit_with _ -> Hh_logger.Level.Info
    | _ -> Hh_logger.Level.Error
  in
  Hh_logger.log
    ~lvl
    "CLIENT_BAD_EXIT [%s] %s"
    command_name
    (Exit_status.show_expanded es);
  HackEventLogger.client_bad_exit ~command_name es e;
  Exit.exit es

let exec_command_without_config (command : ClientCommand.light_command) =
  try
    let exit_status =
      match command with
      | ClientCommand.CDecompressZhhdg env -> ClientDecompressZhhdg.main env
    in
    Exit.exit exit_status
  with
  | exn ->
    let (es, _e) = exit_status_of_exn exn in
    Printf.printf
      "[%s] %s\n"
      (ClientCommand.name_camel_case_light command)
      (Exit_status.show_expanded es);
    Exit.exit es

let exec_command_with_config
    (command : ClientCommand.heavy_command) ~init_proc_stack =
  let init_id = Random_id.short_string () in
  let command_name = ClientCommand.name_camel_case_heavy command in

  (* The global variable Relative_path.root must be initialized for a wide variety of things *)
  let root = ClientArgs.root command in
  set_up_root root;
  let from = ClientArgs.from command in

  set_up_logger ~command_name ~init_id ~root;
  Hh_logger.log
    "[hh_client] %s"
    (String.concat ~sep:" " (Array.to_list Sys.argv));

  let (config, local_config) =
    ServerConfig.load
      ~silent:true
      ~from
      ~ai_options:None
      ~cli_config_overrides:
        (ClientArgs.config command |> Option.value ~default:[])
  in
  init_event_logger root command ~init_id ~from config local_config;

  let init_proc_stack =
    Option.some_if
      (String.equal "" from
      || local_config.ServerLocalConfig.log_init_proc_stack_also_on_absent_from
      )
      init_proc_stack
  in
  try
    let exit_status =
      match command with
      | ClientCommand.CCheck check_env ->
        ClientCheck.main check_env config local_config ~init_proc_stack
        (* never returns; does [Exit.exit] itself *)
      | ClientCommand.CStart env ->
        Lwt_utils.run_main (fun () -> ClientStart.main env)
      | ClientCommand.CStop env ->
        Lwt_utils.run_main (fun () -> ClientStop.main env)
      | ClientCommand.CRestart env ->
        Lwt_utils.run_main (fun () -> ClientRestart.main env)
      | ClientCommand.CLsp args ->
        Lwt_utils.run_main (fun () ->
            ClientLsp.main args ~init_id ~config ~local_config ~init_proc_stack)
      | ClientCommand.CRage env ->
        Lwt_utils.run_main (fun () -> ClientRage.main env local_config)
      | ClientCommand.CSavedStateProjectMetadata env ->
        Lwt_utils.run_main (fun () ->
            ClientSavedStateProjectMetadata.main env local_config)
      | ClientCommand.CDownloadSavedState env ->
        Lwt_utils.run_main (fun () ->
            ClientDownloadSavedState.main env local_config)
    in
    Exit.exit exit_status
  with
  | exn -> handle_exn_and_exit exn ~command_name

let () =
  (* no-op, needed at entry-point for Daemon hookup *)
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();
  set_up_signals ();
  let init_proc_stack = Proc.get_proc_stack (Unix.getpid ()) in
  let command =
    ClientArgs.parse_args
      ~from_default:
        (if Proc.is_likely_from_interactive_shell init_proc_stack then
          "[sh]"
        else
          "")
  in
  match command with
  | ClientCommand.Without_config command -> exec_command_without_config command
  | ClientCommand.With_config command ->
    exec_command_with_config command ~init_proc_stack
