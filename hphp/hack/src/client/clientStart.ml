(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_hhserver () =
  let exe_name =
    if Sys.win32 then
      "hh_server.exe"
    else
      "hh_server"
  in
  let server_next_to_client =
    Path.(to_string @@ concat (dirname executable_name) exe_name)
  in
  if Sys.file_exists server_next_to_client then
    server_next_to_client
  else
    exe_name

type env = {
  root: Path.t;
  from: string;
  no_load: bool;
  watchman_debug_logging: bool;
  log_inference_constraints: bool;
  profile_log: bool;
  silent: bool;
  exit_on_failure: bool;
  ai_mode: string option;
  debug_port: Unix.file_descr option;
  ignore_hh_version: bool;
  save_64bit: string option;
  saved_state_ignore_hhconfig: bool;
  dynamic_view: bool;
  prechecked: bool option;
  mini_state: string option;
  config: (string * string) list;
  custom_telemetry_data: (string * string) list;
  allow_non_opt_build: bool;
}

(* Sometimes systemd-run is available but we can't use it. For example, the
 * systemd might not have a proper working user session, so we might not be
 * able to run commands via systemd-run as a user process *)
let can_run_systemd () =
  if not Sys.unix then
    false
  else
    (* if we're on Unix, verify systemd-run is in the path *)
    let systemd_binary =
      try
        Unix.open_process_in "which systemd-run 2> /dev/null"
        |> In_channel.input_line
      with _ -> None
    in
    if is_none systemd_binary then
      false
    else
      (* Use `timeout` in case it hangs mysteriously.
       * `--quiet` only suppresses stdout. *)
      let ic =
        Unix.open_process_in
          "timeout 1 systemd-run --scope --quiet --user -- true 2> /dev/null"
      in
      (* If all goes right, `systemd-run` will return immediately with exit code 0
       * and run `true` asynchronously as a service. If it goes wrong, it will exit
       * with a non-zero exit code *)
      match Unix.close_process_in ic with
      | Unix.WEXITED 0 -> true
      | _ -> false

let start_server (env : env) =
  let {
    root;
    from;
    no_load;
    watchman_debug_logging;
    log_inference_constraints;
    profile_log;
    silent;
    exit_on_failure;
    ai_mode;
    debug_port;
    ignore_hh_version;
    save_64bit;
    saved_state_ignore_hhconfig;
    dynamic_view;
    prechecked;
    mini_state;
    config;
    custom_telemetry_data;
    allow_non_opt_build;
  } =
    env
  in
  (* Create a pipe for synchronization with the server: we will wait
     until the server finishes its initialisation phase. *)
  let (in_fd, out_fd) = Unix.pipe () in
  Unix.set_close_on_exec in_fd;
  let ic = Unix.in_channel_of_descr in_fd in
  let serialize_key_value_options
      (option : string) (keyvalues : (string * string) list) =
    keyvalues
    |> List.map ~f:(fun (key, value) ->
           [| option; Printf.sprintf "%s=%s" key value |])
    |> Array.concat
  in
  let ai_options =
    match ai_mode with
    | Some ai -> [| "--ai"; ai |]
    | None -> [||]
  in
  let hh_server = get_hhserver () in
  let hh_server_args =
    Array.concat
      [
        [| hh_server; "-d"; Path.to_string root |];
        ( if String.equal from "" then
          [||]
        else
          [| "--from"; from |] );
        (match mini_state with
        | None -> [||]
        | Some state -> [| "--with-mini-state"; state |]);
        ( if no_load then
          [| "--no-load" |]
        else
          [||] );
        ( if watchman_debug_logging then
          [| "--watchman-debug-logging" |]
        else
          [||] );
        ( if log_inference_constraints then
          [| "--log-inference-constraints" |]
        else
          [||] );
        ( if profile_log then
          [| "--profile-log" |]
        else
          [||] );
        ai_options;
        (* If the client starts up a server monitor process, the output of that
         * bootup is passed to this FD - so this FD needs to be threaded
         * through the server monitor process then to the typechecker process.
         *
         * Note: Yes, the FD is available in the monitor process as well, but
         * it doesn't, and shouldn't, use it. *)
        [| "--waiting-client"; string_of_int (Handle.get_handle out_fd) |];
        ( if ignore_hh_version then
          [| "--ignore-hh-version" |]
        else
          [||] );
        ( if saved_state_ignore_hhconfig then
          [| "--saved-state-ignore-hhconfig" |]
        else
          [||] );
        (match save_64bit with
        | None -> [||]
        | Some dest -> [| "--save-64bit"; dest |]);
        ( if dynamic_view then
          [| "--dynamic-view" |]
        else
          [||] );
        (match prechecked with
        | Some true -> [| "--prechecked" |]
        | _ -> [||]);
        (match prechecked with
        | Some false -> [| "--no-prechecked" |]
        | _ -> [||]);
        serialize_key_value_options "--config" config;
        serialize_key_value_options
          "--custom-telemetry-data"
          custom_telemetry_data;
        ( if allow_non_opt_build then
          [| "--allow-non-opt-build" |]
        else
          [||] );
        (match debug_port with
        | None -> [||]
        | Some fd ->
          [| "--debug-client"; string_of_int @@ Handle.get_handle fd |]);
      ]
  in
  let (stdin, stdout, stderr) =
    if silent then
      let nfd = Unix.openfile Sys_utils.null_path [Unix.O_RDWR] 0 in
      (nfd, nfd, nfd)
    else
      Unix.(stdin, stdout, stderr)
  in
  try
    let (exe, args) =
      if can_run_systemd () then
        (* launch command
         * systemd-run    (creates a transient cgroup)
         *    --scope     (allows synchronous execution of hh_server)
         *    --user      (specifies this to be a user instance)
         *    --quiet     (suppresses output to stdout)
         *    --slice=hack.slice   (puts created units under hack.slice)
         *    hh_server <hh_server args>
         *)
        let systemd_exe = "systemd-run" in
        let systemd_args =
          [|
            systemd_exe; "--scope"; "--user"; "--quiet"; "--slice=hack.slice";
          |]
        in
        (systemd_exe, Array.concat [systemd_args; hh_server_args])
      else
        (hh_server, hh_server_args)
    in
    if not silent then
      Printf.eprintf
        "Server launched with the following command:\n\t%s\n%!"
        (String.concat
           ~sep:" "
           (Array.to_list (Array.map ~f:Filename.quote args)));
    let server_pid = Unix.create_process exe args stdin stdout stderr in
    Unix.close out_fd;

    match Sys_utils.waitpid_non_intr [] server_pid with
    | (_, Unix.WEXITED 0) ->
      assert (String.equal (Stdlib.input_line ic) ServerMonitorUtils.ready);
      Stdlib.close_in ic
    | (_, Unix.WEXITED i) ->
      if not silent then
        Printf.eprintf
          "Starting hh_server failed. Exited with status code: %d!\n"
          i;
      if exit_on_failure then exit 77
    | _ ->
      if not silent then Printf.eprintf "Could not start hh_server!\n";
      if exit_on_failure then exit 77
  with _ ->
    if not silent then Printf.eprintf "Could not start hh_server!\n";
    if exit_on_failure then exit 77

let should_start env =
  let root_s = Path.to_string env.root in
  let handoff_options =
    MonitorRpc.
      {
        force_dormant_start = false;
        pipe_name = HhServerMonitorConfig.(pipe_type_to_string Default);
      }
  in
  let tracker = Connection_tracker.create () in
  Hh_logger.log
    "[%s] ClientStart.should_start"
    (Connection_tracker.log_id tracker);
  match
    MonitorConnection.connect_once ~tracker ~timeout:3 env.root handoff_options
  with
  | Ok _conn -> false
  | Error
      ServerMonitorUtils.(
        Connect_to_monitor_failure { server_exists = false; _ })
  | Error (ServerMonitorUtils.Build_id_mismatched _)
  | Error ServerMonitorUtils.Server_died ->
    true
  | Error ServerMonitorUtils.Server_dormant
  | Error ServerMonitorUtils.Server_dormant_out_of_retries ->
    Printf.eprintf "Server already exists but is dormant";
    false
  | Error
      ServerMonitorUtils.(
        Connect_to_monitor_failure { server_exists = true; _ }) ->
    Printf.eprintf "Replacing unresponsive server for %s\n%!" root_s;
    ClientStop.kill_server env.root env.from;
    true

let main (env : env) : Exit_status.t Lwt.t =
  HackEventLogger.set_from env.from;
  HackEventLogger.client_start ();

  (* TODO(ljw): There are some race conditions here. First scenario: two      *)
  (* processes simultaneously do 'hh start' while the server isn't running.   *)
  (* Both their calls to should_start will see the lockfile absent and        *)
  (* immediately get back 'Server_missing'. So both of them launch hh_server. *)
  (* One hh_server process will be slightly faster and will create a lockfile *)
  (* and shortly start listening on the socket. The other will see that the   *)
  (* lockfile already exists and so terminate with error code 0 immediately   *)
  (* without sending the "ready" message. And the second start_server call    *)
  (* will fail its assert that a "ready" message should come.                 *)
  (*                                                                          *)
  (* Second scenario: one process does 'hh start', and creates the lockfile,  *)
  (* and after a short delay will start listening on its socket. Another      *)
  (* process does 'hh start', sees the lockfile is present, tries to connect, *)
  (* but we're still in above short delay and so it gets ECONNREFUSED         *)
  (* immediately, which (given the presence of a lockfile) is returned as     *)
  (* Monitor_not_ready. The should_start routine deems this an unresponsive   *)
  (* server and so kills it and launches a new server again. The first        *)
  (* process might or might not report failure, depending on how far it got   *)
  (* with its 'hh start' -- i.e. whether or not it yet observed the "ready".  *)
  (*                                                                          *)
  (* Third and most typical scenario: an LSP client like Nuclide is running,  *)
  (* and the user does 'hh restart' which kills the server and then calls     *)
  (* start_server. As soon as LSP sees the server killed, it too immediately  *)
  (* calls ClientStart.main. Maybe it will see the lockfile absent, and so    *)
  (* proceed according to the first race scenario above. Maybe it will see    *)
  (* the lockfile present and proceed according to the second race scenario   *)
  (* above. In both cases the LSP client will see problem reports.            *)
  (*                                                                          *)
  (* The root problem is that should_start assumes an invariant that "if      *)
  (* hh_server monitor is busy then it has failed and should be shut down."   *)
  (* This invariant isn't true. Note that the reason we call should_start,    *)
  (* rather than just using the default ClientConnect.autorestart which calls *)
  (* into ClientStart.start_server, is because we do like its ability to kill *)
  (* an unresponsive server. So the should_start function should simply give  *)
  (* a grace period in case of a temporarily busy server.                     *)
  (*                                                                          *)
  (* I believe there's a second similar problem inside ClientConnect.connect  *)
  (* when its env.autorestart is true. During the short delay window it might *)
  (* immediately get Server_missing similar to the first scenario, and so     *)
  (* ClientStart.start_server, which will fail in the same way. Or it might   *)
  (* immediately get ECONNREFUSED during the delay window, so it will retry   *)
  (* connection attempt immediately, and it might burn through all of its     *)
  (* available retries instantly. That's because ECONNREFUSED is immediate.   *)
  if should_start env then (
    start_server env;
    Lwt.return Exit_status.No_error
  ) else (
    if not env.silent then
      Printf.eprintf
        "Error: Server already exists for %s\nUse hh_client restart if you want to kill it and start a new one\n%!"
        (Path.to_string env.root);
    Lwt.return Exit_status.Server_already_exists
  )
