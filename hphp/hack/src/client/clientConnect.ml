(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let log ?tracker ?connection_log_id s =
  let id =
    match tracker with
    | Some tracker -> Some (Connection_tracker.log_id tracker)
    | None -> connection_log_id
  in
  match id with
  | Some id -> Hh_logger.log ("[%s] [client-connect] " ^^ s) id
  | None -> Hh_logger.log ("[client-connect] " ^^ s)

type env = {
  root: Path.t;
  from: string;
  local_config: ServerLocalConfig.t;
  autostart: bool;
  force_dormant_start: bool;
  deadline: float option;
  no_load: bool;
  watchman_debug_logging: bool;
  log_inference_constraints: bool;
  remote: bool;
  progress_callback: (string option -> unit) option;
  do_post_handoff_handshake: bool;
  ignore_hh_version: bool;
  save_64bit: string option;
  save_human_readable_64bit_dep_map: string option;
  saved_state_ignore_hhconfig: bool;
  mini_state: string option;
  use_priority_pipe: bool;
  prechecked: bool option;
  config: (string * string) list;
  custom_hhi_path: string option;
  custom_telemetry_data: (string * string) list;
  allow_non_opt_build: bool;
}

type conn = {
  connection_log_id: string;
  t_connected_to_monitor: float;
  t_received_hello: float;
  t_sent_connection_type: float;
  channels: Timeout.in_channel * Out_channel.t;
  server_specific_files: ServerCommandTypes.server_specific_files;
  conn_progress_callback: (string option -> unit) option;
  conn_root: Path.t;
  conn_deadline: float option;
  from: string;
}

let tty_progress_reporter () =
  let angery_reaccs_only =
    Tty.supports_emoji () && ClientMessages.angery_reaccs_only ()
  in
  fun (status : string option) : unit ->
    if Tty.spinner_used () then Tty.print_clear_line stderr;
    match status with
    | None -> ()
    | Some s ->
      Tty.eprintf
        "hh_server is busy: %s %s%!"
        s
        (Tty.spinner ~angery_reaccs_only ())

(** What is the latest progress message written by the server into a file?
We store this in a mutable variable, so we can know whether it's changed and hence whether
to display the warning banner. *)
let latest_server_progress : ServerProgress.t ref =
  ref ServerProgress.{ message = "connecting"; timestamp = 0.; pid = 0 }

(** This reads from the progress file and assigns into mutable variable "latest_status_progress",
and returns whether there was new status. *)
let check_progress () : bool =
  let server_progress = ServerProgress.read () in
  if
    not
      (Float.equal
         server_progress.ServerProgress.timestamp
         !latest_server_progress.ServerProgress.timestamp)
  then begin
    log
      "check_progress: [%s] %s"
      (Utils.timestring server_progress.ServerProgress.timestamp)
      server_progress.ServerProgress.message;
    latest_server_progress := server_progress;
    true
  end else
    false

let show_progress (progress_callback : string option -> unit) : unit =
  let (_any_changes : bool) = check_progress () in
  let message = !latest_server_progress.ServerProgress.message in
  (* We always show progress, even if there were no changes, just so the user
     can see the spinner keep turning around. It looks better that way for things
     like "loading saved-state" which would otherwise look stuck for 30s. *)
  progress_callback (Some ("[" ^ message ^ "]"));
  ()

let check_for_deadline deadline_opt =
  let now = Unix.time () in
  match deadline_opt with
  | Some deadline when Float.(now > deadline) ->
    log
      "check_for_deadline expired: %s > %s"
      (Utils.timestring now)
      (Utils.timestring deadline);
    Printf.eprintf "\nError: hh_client hit timeout, giving up!\n%!";
    raise Exit_status.(Exit_with Out_of_time)
  | _ -> ()

(** Sleeps until the server sends a message. While waiting, prints out spinner
    and progress information using the argument callback. *)
let rec wait_for_server_message
    ~(connection_log_id : string)
    ~(expected_message : 'a ServerCommandTypes.message_type option)
    ~(ic : Timeout.in_channel)
    ~(deadline : float option)
    ~(server_specific_files : ServerCommandTypes.server_specific_files)
    ~(progress_callback : (string option -> unit) option)
    ~(root : Path.t) : _ ServerCommandTypes.message_type Lwt.t =
  check_for_deadline deadline;
  let%lwt (readable, _, _) =
    Lwt_utils.select
      [Timeout.descr_of_in_channel ic]
      []
      [Timeout.descr_of_in_channel ic]
      1.0
  in
  if List.is_empty readable then (
    Option.iter progress_callback ~f:show_progress;
    wait_for_server_message
      ~connection_log_id
      ~expected_message
      ~ic
      ~deadline
      ~server_specific_files
      ~progress_callback
      ~root
  ) else
    (* an inline module to define this exception type, used internally in the function *)
    let module M = struct
      exception Monitor_failed_to_handoff
    end in
    try%lwt
      let fd = Timeout.descr_of_in_channel ic in
      let msg : 'a ServerCommandTypes.message_type =
        Marshal_tools.from_fd_with_preamble fd
      in
      let (is_ping, is_handoff_failed) =
        match msg with
        | ServerCommandTypes.Ping -> (true, false)
        | ServerCommandTypes.Monitor_failed_to_handoff -> (false, true)
        | _ -> (false, false)
      in
      let matches_expected =
        Option.value_map ~default:true ~f:(Poly.( = ) msg) expected_message
      in
      if is_handoff_failed then
        raise M.Monitor_failed_to_handoff
      else if matches_expected && not is_ping then (
        log
          ~connection_log_id
          "wait_for_server_message: got expected %s"
          (ServerCommandTypesUtils.debug_describe_message_type msg);
        Option.iter progress_callback ~f:(fun callback -> callback None);
        Lwt.return msg
      ) else (
        log
          ~connection_log_id
          "wait_for_server_message: didn't want %s"
          (ServerCommandTypesUtils.debug_describe_message_type msg);
        if not is_ping then Option.iter progress_callback ~f:show_progress;
        wait_for_server_message
          ~connection_log_id
          ~expected_message
          ~ic
          ~deadline
          ~server_specific_files
          ~progress_callback
          ~root
      )
    with
    | (End_of_file | Sys_error _ | M.Monitor_failed_to_handoff) as exn ->
      let e = Exception.wrap exn in
      let finale_data =
        Exit.get_finale_data
          server_specific_files.ServerCommandTypes.server_finale_file
      in
      let client_exn = Exception.get_ctor_string e in
      let client_stack =
        e |> Exception.get_backtrace_string |> Exception.clean_stack
      in
      (* log to logfile *)
      log
        ~connection_log_id
        "SERVER_HUNG_UP [%s]\nfinale_data: %s\n%s"
        client_exn
        (Option.value_map
           finale_data
           ~f:Exit.show_finale_data
           ~default:"[none]")
        client_stack;
      (* stderr *)
      let msg =
        match (exn, finale_data) with
        | (M.Monitor_failed_to_handoff, None) -> "Hack server is too busy."
        | (_, None) ->
          "Hack server disconnected suddenly. It might have crashed."
        | (_, Some finale_data) ->
          Printf.sprintf
            "Hack server disconnected suddenly [%s]\n%s"
            (Exit_status.show finale_data.Exit.exit_status)
            (Option.value ~default:"" finale_data.Exit.msg)
      in
      Printf.eprintf "%s\n" msg;
      (* spinner *)
      Option.iter progress_callback ~f:(fun callback -> callback None);
      (* exception, caught by hh_client.ml and logged.
         In most cases we report that find_hh.sh should simply retry the failed command.
         There are only two cases where we say it shouldn't. *)
      let underlying_exit_status =
        Option.map finale_data ~f:(fun d -> d.Exit.exit_status)
      in
      let external_exit_status =
        match underlying_exit_status with
        | Some
            Exit_status.(
              Failed_to_load_should_abort | Server_non_opt_build_mode) ->
          Exit_status.Server_hung_up_should_abort
        | _ -> Exit_status.Server_hung_up_should_retry
      in
      (* log to telemetry *)
      HackEventLogger.server_hung_up
        ~external_exit_status
        ~underlying_exit_status
        ~client_exn
        ~client_stack
        ~server_stack:
          (Option.map
             finale_data
             ~f:(fun { Exit.stack = Utils.Callstack stack; _ } -> stack))
        ~server_msg:(Option.bind finale_data ~f:(fun d -> d.Exit.msg));
      raise (Exit_status.Exit_with external_exit_status)

let wait_for_server_hello
    (connection_log_id : string)
    (ic : Timeout.in_channel)
    (deadline : float option)
    (server_specific_files : ServerCommandTypes.server_specific_files)
    (progress_callback : (string option -> unit) option)
    (root : Path.t) : unit Lwt.t =
  let%lwt (_ : 'a ServerCommandTypes.message_type) =
    wait_for_server_message
      ~connection_log_id
      ~expected_message:(Some ServerCommandTypes.Hello)
      ~ic
      ~deadline
      ~server_specific_files
      ~progress_callback
      ~root
  in
  Lwt.return_unit

let rec connect ?(allow_macos_hack = true) (env : env) (start_time : float) :
    conn Lwt.t =
  check_for_deadline env.deadline;
  let handoff_options =
    {
      MonitorRpc.force_dormant_start = env.force_dormant_start;
      pipe_name =
        ServerController.pipe_type_to_string
          (if env.force_dormant_start then
            ServerController.Force_dormant_start_only
          else if env.use_priority_pipe then
            ServerController.Priority
          else
            ServerController.Default);
    }
  in
  let tracker = Connection_tracker.create () in
  let connection_log_id = Connection_tracker.log_id tracker in
  (* We'll attempt to connect, with timeout up to [env.deadline]. Effectively, the
     unix process list will be where we store our (unbounded) queue of incoming client requests,
     each of them waiting for the monitor's incoming socket to become available; if there's a backlog
     in the monitor->server pipe and the monitor's incoming queue is full, then the monitor's incoming
     socket will become only available after the server has finished a request, and the monitor gets to
     send its next handoff, and take the next item off its incoming queue.
     If the deadline is infinite, I arbitrarily picked 60s as the timeout coupled with "will retry..."
     if it timed out. That's because I distrust infinite timeouts, just in case something got stuck for
     unknown causes, and maybe retrying the connection attempt will get it unstuck? -- a sort of
     "try turning it off then on again". This timeout must be comfortably longer than the monitor's
     own 30s timeout in MonitorMain.hand_off_client_connection_wrapper to handoff to the server;
     if it were shorter, then the monitor's incoming queue would be entirely full of requests that
     were all stale by the time it got to handle them. *)
  let timeout =
    match env.deadline with
    | None -> 60
    | Some deadline ->
      Int.max 1 (int_of_float (deadline -. Unix.gettimeofday ()))
  in
  log
    ~connection_log_id
    "ClientConnect.connect: attempting MonitorConnection.connect_once (%ds)"
    timeout;
  let conn =
    MonitorConnection.connect_once ~tracker ~timeout env.root handoff_options
  in

  let t_connected_to_monitor = Unix.gettimeofday () in
  match conn with
  | Ok (ic, oc, server_specific_files) ->
    log
      ~connection_log_id
      "ClientConnect.connect: successfully connected to monitor.";
    let%lwt () =
      if env.do_post_handoff_handshake then
        wait_for_server_hello
          connection_log_id
          ic
          env.deadline
          server_specific_files
          env.progress_callback
          env.root
      else
        Lwt.return_unit
    in
    let t_received_hello = Unix.gettimeofday () in
    let threshold = 2.0 in
    if
      Sys_utils.is_apple_os ()
      && allow_macos_hack
      && Float.(Unix.gettimeofday () -. t_connected_to_monitor > threshold)
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

        For shorter startup times, MonitorMain.Sent_fds_collector attempts to
        compensate for this issue by having the monitor wait a few seconds after
        handoff before attempting to close its connection fd.

        For longer startup times, a sufficient (if not exactly clean) workaround
        is simply to have the client re-establish a connection.
      *)
      Printf.eprintf
        "Server connection took over %.1f seconds. Refreshing...\n"
        threshold;
      (try Timeout.shutdown_connection ic with
      | _ -> ());
      Timeout.close_in_noerr ic;
      Stdlib.close_out_noerr oc;

      (* allow_macos_hack:false is a defensive measure against infinite connection loops *)
      connect ~allow_macos_hack:false env start_time
    ) else
      Lwt.return
        {
          connection_log_id = Connection_tracker.log_id tracker;
          t_connected_to_monitor;
          t_received_hello;
          t_sent_connection_type = 0.;
          (* placeholder, until we actually send it *)
          channels = (ic, oc);
          server_specific_files;
          conn_progress_callback = env.progress_callback;
          conn_root = env.root;
          conn_deadline = env.deadline;
          from = env.from;
        }
  | Error e ->
    (match e with
    | MonitorUtils.Server_died
    | MonitorUtils.(Connect_to_monitor_failure { server_exists = true; _ }) ->
      log ~tracker "connect: no response yet from server; will retry...";
      Unix.sleepf 0.1;
      connect env start_time
    | MonitorUtils.(Connect_to_monitor_failure { server_exists = false; _ }) ->
      log ~tracker "connect: autostart=%b" env.autostart;
      if env.autostart then (
        let {
          root;
          from;
          local_config = _;
          autostart = _;
          force_dormant_start = _;
          deadline = _;
          no_load;
          watchman_debug_logging;
          log_inference_constraints;
          remote = _;
          progress_callback = _;
          do_post_handoff_handshake = _;
          ignore_hh_version;
          save_64bit;
          save_human_readable_64bit_dep_map;
          saved_state_ignore_hhconfig;
          use_priority_pipe = _;
          prechecked;
          mini_state;
          config;
          custom_hhi_path;
          custom_telemetry_data;
          allow_non_opt_build;
        } =
          env
        in
        HackEventLogger.client_connect_autostart ();
        ClientStart.(
          start_server
            {
              root;
              from;
              no_load;
              watchman_debug_logging;
              log_inference_constraints;
              silent = false;
              exit_on_failure = false;
              ignore_hh_version;
              save_64bit;
              save_human_readable_64bit_dep_map;
              saved_state_ignore_hhconfig;
              prechecked;
              mini_state;
              config;
              custom_hhi_path;
              custom_telemetry_data;
              allow_non_opt_build;
            });
        connect env start_time
      ) else (
        Printf.eprintf
          ("Error: no hh_server running. Either start hh_server"
          ^^ " yourself or run hh_client without --autostart-server false\n%!");
        raise Exit_status.(Exit_with No_server_running_should_retry)
      )
    | MonitorUtils.Server_dormant_out_of_retries ->
      Printf.eprintf
        ("Ran out of retries while waiting for Mercurial to finish rebase. Starting "
        ^^ "the server in the middle of rebase is strongly not recommended and you should "
        ^^ "first finish the rebase before retrying. If you really "
        ^^ "know what you're doing, maybe try --force-dormant-start true\n%!");
      raise Exit_status.(Exit_with Out_of_retries)
    | MonitorUtils.Server_dormant ->
      Printf.eprintf
        ("Error: No server running and connection limit reached for waiting"
        ^^ " on next server to be started. Please wait patiently. If you really"
        ^^ " know what you're doing, maybe try --force-dormant-start true\n%!");
      raise Exit_status.(Exit_with No_server_running_should_retry)
    | MonitorUtils.Build_id_mismatched mismatch_info_opt ->
      MonitorUtils.(
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
  let%lwt ({ channels = (_, oc); _ } as conn) = connect env start_time in
  HackEventLogger.client_established_connection start_time;
  if env.do_post_handoff_handshake then
    ServerCommandLwt.send_connection_type oc ServerCommandTypes.Non_persistent;
  Lwt.return { conn with t_sent_connection_type = Unix.gettimeofday () }

let rpc :
    type a.
    conn -> desc:string -> a ServerCommandTypes.t -> (a * Telemetry.t) Lwt.t =
 fun {
       connection_log_id;
       t_connected_to_monitor;
       t_received_hello;
       t_sent_connection_type;
       channels = (ic, oc);
       server_specific_files;
       conn_progress_callback = progress_callback;
       conn_root;
       conn_deadline = deadline;
       from;
     }
     ~desc
     cmd ->
  let t_ready_to_send_cmd = Unix.gettimeofday () in
  let metadata = { ServerCommandTypes.from; desc } in
  Marshal.to_channel oc (ServerCommandTypes.Rpc (metadata, cmd)) [];
  Out_channel.flush oc;
  let t_sent_cmd = Unix.gettimeofday () in
  let%lwt res =
    wait_for_server_message
      ~connection_log_id
      ~expected_message:None
      ~ic
      ~deadline
      ~server_specific_files
      ~progress_callback
      ~root:conn_root
  in
  match res with
  | ServerCommandTypes.Response (response, tracker) ->
    let open Connection_tracker in
    let telemetry =
      tracker
      |> track ~key:Client_ready_to_send_cmd ~time:t_ready_to_send_cmd
      |> track ~key:Client_sent_cmd ~time:t_sent_cmd
      |> track ~key:Client_received_response
      (* now we can fill in missing information in tracker, which we couldn't fill in earlier
         because we'd already transferred ownership of the tracker to the monitor... *)
      |> track ~key:Client_connected_to_monitor ~time:t_connected_to_monitor
      |> track ~key:Client_received_hello ~time:t_received_hello
      |> track ~key:Client_sent_connection_type ~time:t_sent_connection_type
      |> get_telemetry
    in
    Lwt.return (response, telemetry)
  | ServerCommandTypes.Push _ -> failwith "unexpected 'push' RPC response"
  | ServerCommandTypes.Hello -> failwith "unexpected 'hello' RPC response"
  | ServerCommandTypes.Ping -> failwith "unexpected 'ping' RPC response"
  | ServerCommandTypes.Monitor_failed_to_handoff ->
    failwith "unexpected 'monitor_failed_to_handoff' RPC response"

let rpc_with_retry
    (conn_f : unit -> conn Lwt.t)
    ~(desc : string)
    (cmd : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t) :
    'a Lwt.t =
  ServerCommandTypes.Done_or_retry.call ~f:(fun () ->
      let%lwt conn = conn_f () in
      let%lwt (result, _telemetry) = rpc conn ~desc cmd in
      Lwt.return result)

let rpc_with_retry_list
    (conn_f : unit -> conn Lwt.t)
    ~(desc : string)
    (cmd : 'a ServerCommandTypes.Done_or_retry.t list ServerCommandTypes.t) :
    'a list Lwt.t =
  let call_here s =
    ServerCommandTypes.Done_or_retry.call ~f:(fun () -> Lwt.return s)
  in
  let%lwt conn = conn_f () in
  let%lwt (job_list, _) = rpc conn ~desc cmd in
  List.map job_list ~f:call_here |> Lwt.all
