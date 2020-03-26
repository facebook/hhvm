(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module SMUtils = ServerMonitorUtils

exception Server_hung_up of ServerCommandTypes.finale_data option

type env = {
  root: Path.t;
  from: string;
  autostart: bool;
  force_dormant_start: bool;
  deadline: float option;
  no_load: bool;
  watchman_debug_logging: bool;
  log_inference_constraints: bool;
  profile_log: bool;
  remote: bool;
  ai_mode: string option;
  progress_callback: string option -> unit;
  do_post_handoff_handshake: bool;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  use_priority_pipe: bool;
  prechecked: bool option;
  config: (string * string) list;
  allow_non_opt_build: bool;
}

type conn = {
  channels: Timeout.in_channel * Out_channel.t;
  server_finale_file: string;
  conn_progress_callback: string option -> unit;
  conn_root: Path.t;
  conn_deadline: float option;
}

let get_finale_data (server_finale_file : string) :
    ServerCommandTypes.finale_data option =
  try
    let ic = Stdlib.open_in_bin server_finale_file in
    let contents : ServerCommandTypes.finale_data = Marshal.from_channel ic in
    Stdlib.close_in ic;
    Some contents
  with _ -> None

let tty_progress_reporter () =
  let angery_reaccs_only =
    Tty.supports_emoji () && ClientMessages.angery_reaccs_only ()
  in
  fun (status : string option) ->
    ( if Tty.spinner_used () then Tty.print_clear_line stderr;
      match status with
      | None -> ()
      | Some s ->
        Tty.eprintf
          "hh_server is busy: %s %s%!"
          s
          (Tty.spinner ~angery_reaccs_only ())
      : unit )

let null_progress_reporter (_status : string option) : unit = ()

(* what is the server doing? or None if nothing *)
let progress : string option ref = ref None

(* if the server has something not going right, what? *)
let progress_warning : string option ref = ref None

let default_progress_message = "processing"

let check_progress (root : Path.t) : unit =
  match ServerUtils.server_progress ~timeout:3 root with
  | Ok (msg, warning) ->
    progress := msg;
    progress_warning := warning
  | _ -> ()

let delta_t : float = 3.0

let print_wait_msg (progress_callback : string option -> unit) (root : Path.t) :
    unit =
  let had_warning = Option.is_some !progress_warning in
  check_progress root;
  if not had_warning then
    Option.iter !progress_warning ~f:(Printf.eprintf "%s\n%!");
  let progress = Option.value !progress ~default:default_progress_message in
  let final_suffix =
    if Option.is_some !progress_warning then
      " - this can take a long time, see warning above]"
    else
      "]"
  in
  progress_callback (Some ("[" ^ progress ^ final_suffix))

let check_for_deadline deadline_opt =
  let timed_out =
    match deadline_opt with
    | Some d -> Unix.time () > d
    | None -> false
  in
  if timed_out then (
    Printf.eprintf "\nError: hh_client hit timeout, giving up!\n%!";
    raise Exit_status.(Exit_with Out_of_time)
  )

(* Sleeps until the server sends a message. While waiting, prints out spinner
 * and progress information using the argument callback. *)
let rec wait_for_server_message
    ~(expected_message : 'a ServerCommandTypes.message_type option)
    ~(ic : Timeout.in_channel)
    ~(deadline : float option)
    ~(server_finale_file : string)
    ~(progress_callback : string option -> unit)
    ~(root : Path.t) : 'a ServerCommandTypes.message_type Lwt.t =
  check_for_deadline deadline;
  let%lwt (readable, _, _) =
    Lwt_utils.select
      [Timeout.descr_of_in_channel ic]
      []
      [Timeout.descr_of_in_channel ic]
      1.0
  in
  if readable = [] then (
    print_wait_msg progress_callback root;
    wait_for_server_message
      ~expected_message
      ~ic
      ~deadline
      ~server_finale_file
      ~progress_callback
      ~root
  ) else
    try%lwt
      let fd = Timeout.descr_of_in_channel ic in
      let msg : 'a ServerCommandTypes.message_type =
        Marshal_tools.from_fd_with_preamble fd
      in
      let is_not_ping = msg <> ServerCommandTypes.Ping in
      let matches_expected =
        Option.value_map ~default:true ~f:(( = ) msg) expected_message
      in
      if is_not_ping && matches_expected then (
        progress_callback None;
        Lwt.return msg
      ) else (
        if is_not_ping then print_wait_msg progress_callback root;
        wait_for_server_message
          ~expected_message
          ~ic
          ~deadline
          ~server_finale_file
          ~progress_callback
          ~root
      )
    with
    | End_of_file
    | Sys_error _ ->
      let finale_data = get_finale_data server_finale_file in
      progress_callback None;
      raise (Server_hung_up finale_data)

let wait_for_server_hello
    (ic : Timeout.in_channel)
    (deadline : float option)
    (server_finale_file : string)
    (progress_callback : string option -> unit)
    (root : Path.t) : unit Lwt.t =
  let%lwt (_ : 'a ServerCommandTypes.message_type) =
    wait_for_server_message
      ~expected_message:(Some ServerCommandTypes.Hello)
      ~ic
      ~deadline
      ~server_finale_file
      ~progress_callback
      ~root
  in
  Lwt.return_unit

let with_server_hung_up (f : unit -> 'a Lwt.t) : 'a Lwt.t =
  try%lwt f () with
  | Server_hung_up (Some finale_data) ->
    ServerCommandTypes.(
      Printf.eprintf
        "Hack server disconnected suddenly [%s]\n   %s\n"
        (Exit_status.to_string finale_data.exit_status)
        finale_data.msg;
      if finale_data.exit_status = Exit_status.Failed_to_load_should_abort then
        raise Exit_status.(Exit_with Server_hung_up_should_abort)
      else
        raise Exit_status.(Exit_with Server_hung_up_should_retry))
  | Server_hung_up None ->
    Printf.eprintf
      ( "Hack server disconnected suddenly. Most likely a new one"
      ^^ " is being initialized with a better saved state after a large rebase/update.\n"
      );
    raise Exit_status.(Exit_with Server_hung_up_should_retry)

let rec connect
    ?(first_attempt = false)
    ?(allow_macos_hack = true)
    (env : env)
    (start_time : float) : conn Lwt.t =
  check_for_deadline env.deadline;
  let connect_once_start_t = Unix.time () in
  let handoff_options =
    {
      MonitorRpc.force_dormant_start = env.force_dormant_start;
      pipe_name =
        HhServerMonitorConfig.pipe_type_to_string
          ( if env.force_dormant_start then
            HhServerMonitorConfig.Force_dormant_start_only
          else if env.use_priority_pipe then
            HhServerMonitorConfig.Priority
          else
            HhServerMonitorConfig.Default );
    }
  in
  let conn =
    ServerUtils.connect_to_monitor ~timeout:1 env.root handoff_options
  in
  HackEventLogger.client_connect_once connect_once_start_t;
  match conn with
  | Ok (ic, oc, server_finale_file) ->
    let start = Unix.gettimeofday () in
    let%lwt () =
      if env.do_post_handoff_handshake then
        with_server_hung_up @@ fun () ->
        wait_for_server_hello
          ic
          env.deadline
          server_finale_file
          env.progress_callback
          env.root
      else
        Lwt.return_unit
    in
    let threshold = 2.0 in
    if
      Sys_utils.is_apple_os ()
      && allow_macos_hack
      && Unix.gettimeofday () -. start > threshold
    then (
      (*
        HACK: on MacOS, re-establish the connection if it took a long time
        during the initial attempt.

        The MacOS implementation of the server monitor does not appear to make a
        graceful handoff of the client connection to the server main process.
        If, after the handoff, the monitor closes its connection fd before the
        server main attempts any reads, the server main's connection will go
        stale for reads (i.e., reading will generate an EOF). This is the case
        if the server needs to run a long typecheck phase before communication
        with the client, e.g. for cold starts.

        For shorter startup times, ServerMonitor.Sent_fds_collector attempts to
        compensate for this issue by having the monitor wait a few seconds after
        handoff before attempting to close its connection fd.

        For longer startup times, a sufficient (if not exactly clean) workaround
        is simply to have the client re-establish a connection.
      *)
      Printf.eprintf
        "Server connection took over %.1f seconds. Refreshing...\n"
        threshold;
      (try Timeout.shutdown_connection ic with _ -> ());
      Timeout.close_in_noerr ic;
      Stdlib.close_out_noerr oc;

      (* allow_macos_hack:false is a defensive measure against infinite connection loops *)
      connect ~allow_macos_hack:false env start_time
    ) else
      Lwt.return
        {
          channels = (ic, oc);
          server_finale_file;
          conn_progress_callback = env.progress_callback;
          conn_root = env.root;
          conn_deadline = env.deadline;
        }
  | Error e ->
    if first_attempt then
      Printf.eprintf
        "For more detailed logs, try `tail -f $(hh_client --monitor-logname) $(hh_client --logname)`\n";
    (match e with
    | SMUtils.Server_died
    | SMUtils.Monitor_connection_failure ->
      Unix.sleepf 0.1;
      connect env start_time
    | SMUtils.Server_missing ->
      if env.autostart then (
        ClientStart.start_server
          {
            ClientStart.root = env.root;
            from = env.from;
            no_load = env.no_load;
            watchman_debug_logging = env.watchman_debug_logging;
            log_inference_constraints = env.log_inference_constraints;
            profile_log = env.profile_log;
            silent = false;
            exit_on_failure = false;
            ai_mode = env.ai_mode;
            debug_port = None;
            ignore_hh_version = env.ignore_hh_version;
            saved_state_ignore_hhconfig = env.saved_state_ignore_hhconfig;
            dynamic_view = false;
            prechecked = env.prechecked;
            config = env.config;
            allow_non_opt_build = env.allow_non_opt_build;
          };
        connect env start_time
      ) else (
        Printf.eprintf
          ( "Error: no hh_server running. Either start hh_server"
          ^^ " yourself or run hh_client without --autostart-server false\n%!"
          );
        raise Exit_status.(Exit_with No_server_running_should_retry)
      )
    | SMUtils.Server_dormant_out_of_retries ->
      Printf.eprintf
        ( "Ran out of retries while waiting for Mercurial to finish rebase. Starting "
        ^^ "the server in the middle of rebase is strongly not recommended and you should "
        ^^ "first finish the rebase before retrying. If you really "
        ^^ "know what you're doing, maybe try --force-dormant-start\n%!" );
      raise Exit_status.(Exit_with Out_of_retries)
    | SMUtils.Server_dormant ->
      Printf.eprintf
        ( "Error: No server running and connection limit reached for waiting"
        ^^ " on next server to be started. Please wait patiently. If you really"
        ^^ " know what you're doing, maybe try --force-dormant-start\n%!" );
      raise Exit_status.(Exit_with No_server_running_should_retry)
    | SMUtils.Monitor_socket_not_ready ->
      HackEventLogger.client_connect_once_busy start_time;
      Unix.sleepf 0.1;
      connect env start_time
    | SMUtils.Monitor_establish_connection_timeout ->
      (* This should only happen if the Monitor is being DDOSed or has
       * wedged itself. To ameliorate inadvertent self DDOSing by hh_clients,
       * we don't auto-retry a connection when the Monitor is busy .*)
      HackEventLogger.client_connect_once_busy start_time;
      Printf.eprintf "\nError: Monitor is busy. Giving up!\n%!";
      raise Exit_status.(Exit_with Monitor_connection_failure)
    | SMUtils.Build_id_mismatched mismatch_info_opt ->
      ServerMonitorUtils.(
        Printf.eprintf
          "hh_server's version doesn't match the client's, so it will exit.\n";
        begin
          match mismatch_info_opt with
          | None -> ()
          | Some mismatch_info ->
            let secs =
              int_of_float
                (Unix.gettimeofday () -. mismatch_info.existing_launch_time)
            in
            let time =
              if secs > 86400 then
                Printf.sprintf "%n days" (secs / 86400)
              else if secs > 3600 then
                Printf.sprintf "%n hours" (secs / 3600)
              else if secs > 60 then
                Printf.sprintf "%n minutes" (secs / 60)
              else
                Printf.sprintf "%n seconds" secs
            in
            Printf.eprintf
              "  hh_server '%s' was launched %s ago;\n  hh_client '%s' launched now.\n%!"
              (String.concat ~sep:" " mismatch_info.existing_argv)
              time
              (String.concat ~sep:" " (Array.to_list Sys.argv));
            ()
        end;
        if env.autostart then (
          (* The new server is definitely not running yet, adjust the
           * start time and deadline to absorb the server startup time.
           *)
          let now = Unix.time () in
          let deadline =
            Option.map ~f:(fun d -> d +. (now -. start_time)) env.deadline
          in
          Printf.eprintf "Going to launch a new one.\n%!";
          connect { env with deadline } now
        ) else
          raise Exit_status.(Exit_with Exit_status.Build_id_mismatch)))

let connect (env : env) : conn Lwt.t =
  let start_time = Unix.time () in
  try%lwt
    let%lwt ({ channels = (_, oc); _ } as conn) =
      connect ~first_attempt:true env start_time
    in
    HackEventLogger.client_established_connection start_time;
    if env.do_post_handoff_handshake then
      ServerCommandLwt.send_connection_type oc ServerCommandTypes.Non_persistent;
    Lwt.return conn
  with e ->
    (* we'll log this exception, then re-raise the exception, but using the *)
    (* original backtrace of "e" rather than generating a new backtrace.    *)
    let backtrace = Caml.Printexc.get_raw_backtrace () in
    HackEventLogger.client_establish_connection_exception e;
    Caml.Printexc.raise_with_backtrace e backtrace

let rpc : type a. conn -> a ServerCommandTypes.t -> a Lwt.t =
 fun {
       channels = (ic, oc);
       server_finale_file;
       conn_progress_callback = progress_callback;
       conn_root;
       conn_deadline = deadline;
     }
     cmd ->
  Marshal.to_channel oc (ServerCommandTypes.Rpc cmd) [];
  Out_channel.flush oc;
  with_server_hung_up @@ fun () ->
  let%lwt res =
    wait_for_server_message
      ~expected_message:None
      ~ic
      ~deadline
      ~server_finale_file
      ~progress_callback
      ~root:conn_root
  in
  match res with
  | ServerCommandTypes.Response (response, _) -> Lwt.return response
  | ServerCommandTypes.Push _ -> failwith "unexpected 'push' RPC response"
  | ServerCommandTypes.Hello -> failwith "unexpected 'hello' RPC response"
  | ServerCommandTypes.Ping -> failwith "unexpected 'ping' RPC response"

let rpc_with_retry
    (conn_f : unit -> conn Lwt.t)
    (cmd : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t) :
    'a Lwt.t =
  ServerCommandTypes.Done_or_retry.call ~f:(fun () ->
      let%lwt conn = conn_f () in
      let%lwt result = rpc conn cmd in
      Lwt.return result)
