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

let init_event_logger root command ~init_id config local_config : unit =
  HackEventLogger.client_init
    ~init_id
    ~custom_columns:(ClientCommand.get_custom_telemetry_data command)
    ~always_add_sandcastle_info:
      (Option.exists local_config ~f:(fun c ->
           c.ServerLocalConfig.log_events_with_sandcastle_info))
    (Option.value root ~default:Path.dummy_path);
  Option.iter config ~f:(fun config ->
      HackEventLogger.set_hhconfig_version
        (ServerConfig.version config |> Config_file.version_to_string_opt));
  Option.iter local_config ~f:(fun local_config ->
      HackEventLogger.set_rollout_flags
        (ServerLocalConfig.to_rollout_flags local_config));
  ()

let () =
  (* no-op, needed at entry-point for Daemon hookup *)
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();

  (* Ignore SIGPIPE since if it arises from clientConnect then it might indicate server hangup;
     we detect this case already and handle it better than a signal (unhandled signals cause program exit). *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  Sys_utils.set_signal
    Sys.sigint
    (Sys.Signal_handle (fun _ -> raise Exit_status.(Exit_with Interrupted)));
  let init_id = Random_id.short_string () in
  let init_proc_stack = Proc.get_proc_stack (Unix.getpid ()) in
  let from_default =
    if Proc.is_likely_from_interactive_shell init_proc_stack then
      "[sh]"
    else
      ""
  in
  let command = ClientArgs.parse_args ~from_default in
  let command_name =
    match command with
    | ClientCommand.CCheck _ -> "Check"
    | ClientCommand.CStart _ -> "Start"
    | ClientCommand.CStop _ -> "Stop"
    | ClientCommand.CRestart _ -> "Restart"
    | ClientCommand.CLsp _ -> "Lsp"
    | ClientCommand.CSavedStateProjectMetadata _ -> "SavedStateProjectMetadata"
    | ClientCommand.CDownloadSavedState _ -> "DownloadSavedState"
    | ClientCommand.CRage _ -> "Rage"
    | ClientCommand.CDecompressZhhdg _ -> "DecompressZhhdg"
  in

  (* The global variable Relative_path.root must be initialized for a wide variety of things *)
  let root = ClientArgs.root command in
  Option.iter root ~f:(Relative_path.set_path_prefix Relative_path.Root);
  let from = ClientArgs.from command in

  (* We'll chose where Hh_logger.log gets sent *)
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
       with
      | _ -> ())
  end;
  Hh_logger.log
    "[hh_client] %s"
    (String.concat ~sep:" " (Array.to_list Sys.argv));

  (* we also have to patch up HackEventLogger with stuff we learn from root, if available... *)
  let (local_config, config) =
    match root with
    | None -> (None, None)
    | Some root ->
      ServerProgress.set_root root;
      (* The code to load hh.conf (ServerLocalConfig) is a bit weirdly factored.
         It requires a ServerArgs structure, solely to pick out --from and --config options. We
         dont have ServerArgs (we only have client args!) but we do parse --from and --config
         options and will patch them onto a fake ServerArgs. *)
      let fake_server_args =
        ServerArgs.default_options_with_check_mode ~root:(Path.to_string root)
      in
      let fake_server_args = ServerArgs.set_from fake_server_args from in
      let fake_server_args =
        match ClientArgs.config command with
        | None -> fake_server_args
        | Some config -> ServerArgs.set_config fake_server_args config
      in
      let (config, local_config) =
        ServerConfig.load ~silent:true fake_server_args
      in
      (Some local_config, Some config)
  in
  init_event_logger root command ~init_id config local_config;

  try
    let exit_status =
      match command with
      | ClientCommand.CCheck check_env ->
        let local_config = Option.value_exn local_config in
        let init_proc_stack =
          if
            String.is_empty from
            || local_config
                 .ServerLocalConfig.log_init_proc_stack_also_on_absent_from
          then
            Some init_proc_stack
          else
            None
        in
        ClientCheck.main check_env local_config ~init_proc_stack
        (* never returns; does [Exit.exit] itself *)
      | ClientCommand.CStart env ->
        Lwt_utils.run_main (fun () -> ClientStart.main env)
      | ClientCommand.CStop env ->
        Lwt_utils.run_main (fun () -> ClientStop.main env)
      | ClientCommand.CRestart env ->
        Lwt_utils.run_main (fun () -> ClientRestart.main env)
      | ClientCommand.CLsp args ->
        Lwt_utils.run_main (fun () -> ClientLsp.main args ~init_id)
      | ClientCommand.CRage env ->
        Lwt_utils.run_main (fun () ->
            ClientRage.main env (Option.value_exn local_config))
      | ClientCommand.CSavedStateProjectMetadata env ->
        Lwt_utils.run_main (fun () ->
            ClientSavedStateProjectMetadata.main
              env
              (Option.value_exn local_config))
      | ClientCommand.CDownloadSavedState env ->
        Lwt_utils.run_main (fun () ->
            ClientDownloadSavedState.main env (Option.value_exn local_config))
      | ClientCommand.CDecompressZhhdg env -> ClientDecompressZhhdg.main env
    in
    Exit.exit exit_status
  with
  | exn ->
    let e = Exception.wrap exn in
    (* hide the spinner *)
    ClientSpinner.report ~to_stderr:false ~angery_reaccs_only:false None;
    (* We trust that if someone raised Exit_with then they had the decency to print
       out a user-facing message; we will only print out a user-facing message here
       for uncaught exceptions: lvl=Error gets sent to stderr, but lvl=Info doesn't. *)
    let (es, lvl) =
      match exn with
      | Exit_status.Exit_with es -> (es, Hh_logger.Level.Info)
      | _ -> (Exit_status.Uncaught_exception e, Hh_logger.Level.Error)
    in
    Hh_logger.log
      ~lvl
      "CLIENT_BAD_EXIT [%s] %s"
      command_name
      (Exit_status.show_expanded es);
    HackEventLogger.client_bad_exit ~command_name es e;
    Exit.exit es
