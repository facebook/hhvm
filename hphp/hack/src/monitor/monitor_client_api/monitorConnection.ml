(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open MonitorUtils

let log s ~tracker =
  Hh_logger.log ("[%s] " ^^ s) (Connection_tracker.log_id tracker)

let server_exists lock_file = not (Lock.check lock_file)

let from_channel_without_buffering ?timeout tic =
  Marshal_tools.from_fd_with_preamble ?timeout (Timeout.descr_of_in_channel tic)

let hh_monitor_config root =
  MonitorUtils.
    {
      lock_file = ServerFiles.lock_file root;
      socket_file = ServerFiles.socket_file root;
      server_log_file = ServerFiles.log_link root;
      monitor_log_file = ServerFiles.monitor_log_link root;
    }

let wait_on_server_restart ic =
  (* The server is out of date and is going to exit. Subsequent calls
   * to connect on the Unix Domain Socket might succeed, connecting to
   * the server that is about to die, and eventually we will be hung
   * up on while trying to read from our end.
   *
   * To avoid that fate, when we know the server is about to exit, we
   * wait for the connection to be closed, signaling that the server
   * has exited and the OS has cleaned up after it, then we try again.
   *
   * See also: MonitorMain.client_out_of_date
   *)
  try
    while true do
      let (_ : char) = Timeout.input_char ic in
      ()
    done
  with
  | End_of_file
  | Sys_error _ ->
    (* Server has exited and hung up on us *)
    ()

let get_sockaddr config =
  let sock_name = Socket.get_path config.MonitorUtils.socket_file in
  if Sys.win32 then (
    let ic = In_channel.create ~binary:true sock_name in
    let port = Option.value_exn (In_channel.input_binary_int ic) in
    In_channel.close ic;
    Unix.(ADDR_INET (inet_addr_loopback, port))
  ) else
    Unix.ADDR_UNIX sock_name

(* Consume sequence of Prehandoff messages. *)
let rec consume_prehandoff_messages
    ~(timeout : Timeout.t) (ic : Timeout.in_channel) (oc : Stdlib.out_channel) :
    ( Timeout.in_channel
      * Stdlib.out_channel
      * ServerCommandTypes.server_specific_files,
      MonitorUtils.connection_error )
    result =
  let module PH = Prehandoff in
  let m : PH.msg = from_channel_without_buffering ~timeout ic in
  match m with
  | PH.Sentinel server_specific_files -> Ok (ic, oc, server_specific_files)
  | PH.Server_dormant_connections_limit_reached ->
    Printf.eprintf
    @@ "Connections limit on dormant server reached."
    ^^ " Be patient waiting for a server to be started.";
    Error Server_dormant
  | PH.Server_not_alive_dormant _ ->
    Printf.eprintf
      "Waiting for a server to be started...%s\n%!"
      ClientMessages.waiting_for_server_to_be_started_doc;
    consume_prehandoff_messages ~timeout ic oc
  | PH.Server_died_config_change ->
    Printf.eprintf
      ("Last server exited due to config change. Please re-run client"
      ^^ " to force discovery of the correct version of the client.");
    Error Server_died
  | PH.Server_died { PH.status; PH.was_oom } ->
    (match (was_oom, status) with
    | (true, _) -> Printf.eprintf "Last server killed by OOM Manager.\n%!"
    | (false, Unix.WEXITED exit_code) ->
      Printf.eprintf "Last server exited with code: %d.\n%!" exit_code
    | (false, Unix.WSIGNALED signal) ->
      Printf.eprintf "Last server killed by signal: %d.\n%!" signal
    | (false, Unix.WSTOPPED signal) ->
      Printf.eprintf "Last server stopped by signal: %d.\n%!" signal);

    (* Monitor will exit now that it has provided a client with a reason
     * for the last server dying. Wait for the Monitor to exit. *)
    wait_on_server_restart ic;
    Error Server_died

let read_and_log_process_information ~timeout =
  let process = Process.exec Exec_command.Pgrep ["hh_client"; "-a"] in
  match Process.read_and_wait_pid process ~timeout with
  | Ok r ->
    (* redirecting the output of pgrep from stdout to stderr with no formatting, so users see relevant PID and paths.
       Manually pulling out first 5 elements with string handling rather than piping the input into `head`.
       The reason being that the use of Process.exec with Exec_command is to avoid the risk of shelling out
       failing and throwing some exception that gets caught higher up the chain. Instead, allow pgrep to be a single chance to fail.
       Any success can be handled in application code with postprocessing.

       Currently we filter out Scuba-related entries from the process list. They should not contribute to notable hh interaction issues, and create noisy logs.
    *)
    let exclude_scuba_entry =
      List.filter ~f:(fun x -> String_utils.substring_index "scuba" x = -1)
    in
    let process_list =
      r.Process_types.stdout
      |> String.split_on_chars ~on:['\n']
      |> exclude_scuba_entry
    in
    let matching_processes =
      process_list |> (fun x -> List.take x 5) |> String.concat ~sep:"\n"
    in
    Hh_logger.error
      "[monitorConnection] found %d processes when hh took more than 3 seconds to connect to monitor"
      (List.length process_list);
    Printf.eprintf "%s\n%!" matching_processes;
    HackEventLogger.client_connect_to_monitor_slow_log ()
  | Error e ->
    let failure_msg = Process.failure_msg e in
    Printf.eprintf "pgrep failed with reason: %s\n%!" failure_msg;
    let telemetry =
      match e with
      | Process_types.Abnormal_exit { stderr; stdout; status } ->
        let status_msg =
          match status with
          | Unix.WEXITED code -> Printf.sprintf "exited: code = %d" code
          | Unix.WSIGNALED code -> Printf.sprintf "signaled: code = %d" code
          | Unix.WSTOPPED code -> Printf.sprintf "stopped: code = %d" code
        in
        Telemetry.create ()
        |> Telemetry.string_ ~key:"stderr" ~value:stderr
        |> Telemetry.string_ ~key:"stdout" ~value:stdout
        |> Telemetry.string_ ~key:"status" ~value:status_msg
      | Process_types.Timed_out { stderr; stdout } ->
        Telemetry.create ()
        |> Telemetry.string_ ~key:"stderr" ~value:stderr
        |> Telemetry.string_ ~key:"stdout" ~value:stdout
      | Process_types.Overflow_stdin -> Telemetry.create ()
    in
    let desc = "pgrep_failed_for_monitor_connect" in
    Hh_logger.log
      "INVARIANT VIOLATION BUG: [%s] [%s]"
      desc
      (Telemetry.to_string telemetry);
    HackEventLogger.invariant_violation_bug desc ~telemetry;
    ()

let connect_to_monitor
    ?(log_on_slow_connect = false)
    ~tracker
    ~timeout
    ~terminate_monitor_on_version_mismatch
    config =
  (* There are some pathological scenarios concerned with high volumes of requests.
     1. There's a finite unix pipe between monitor and server, used for handoff. When
     that pipe gets full (~30 requests), the monitor will freeze for 4s before closing
     the client request.
     2. In ClientConnect.connect we allow the monitor only 1s to respond before we
     abandon our connection attempt and try again, repeated up to --timeout times (by
     default infinite). If the monitor takes >1s to work through its queue, then every
     item placed there will be expired before the monitor gets to handle it, and we'll
     never recover.
     3. The monitor's finite incoming queue will become full too, disallowing clients from
     even connecting.
     4. Aside from those concerns, just practically, the commonest mode of failure I've observed
     is that the monitor actually just gets stuck -- sometimes stuck on a "read" call to
     read requests parameters where it should make progress or get an EOF but just sits there
     indefinitely, sometimes stuck on a "select" call to see if there's a request on the queue
     where I know there are outstanding requests but again it doesn't see them. I have no
     explanation for these phemona. *)
  let open Connection_tracker in
  let phase = ref MonitorUtils.Connect_open_socket in
  let finally_close : Timeout.in_channel option ref = ref None in
  let warn_of_busy_server_timer : Timer.t option ref = ref None in
  Utils.try_finally
    ~f:(fun () ->
      try
        (* Note: Timeout.with_timeout is accomplished internally by an exception;
           hence, our "try/with exn ->" catch-all must be outside with_timeout. *)
        Timeout.with_timeout
          ~timeout
          ~do_:(fun timeout ->
            (* 1. open the socket *)
            phase := MonitorUtils.Connect_open_socket;
            let monitor_daemon_is_running =
              match
                Process.read_and_wait_pid
                  (Process.exec
                     Exec_command.Pgrep
                     ["-af"; "monitor_daemon_main"])
                  ~timeout:2
              with
              | Ok _ -> true
              | Error _ -> false
            in
            if log_on_slow_connect && monitor_daemon_is_running then
              (* Setting up a timer to fire a notice to the user when hh seems to be slower.
                 Since connect_to_monitor allows a 60 second timeout by default, it's possible that hh appears to be hanging from a user's perspective.
                 We can show people after some time (3 seconds arbitrarily) what processes might be slowing the response. *)
              let callback () =
                Printf.eprintf
                  "The Hack server seems busy and overloaded. The following processes are making requests to hh_server (limited to first 5 shown): \n%!";
                read_and_log_process_information ~timeout:2
              in
              warn_of_busy_server_timer :=
                Some (Timer.set_timer ~interval:3.0 ~callback)
            else
              ();
            let sockaddr = get_sockaddr config in
            let (ic, oc) = Timeout.open_connection ~timeout sockaddr in
            finally_close := Some ic;
            let tracker =
              Connection_tracker.track tracker ~key:Client_opened_socket
            in

            (* 2. send version, followed by newline *)
            phase := MonitorUtils.Connect_send_version;
            let version_str =
              MonitorUtils.VersionPayload.serialize
                ~tracker
                ~terminate_monitor_on_version_mismatch
            in
            let (_ : int) =
              Marshal_tools.to_fd_with_preamble
                (Unix.descr_of_out_channel oc)
                version_str
            in
            phase := MonitorUtils.Connect_send_newline;
            let (_ : int) =
              Unix.write
                (Unix.descr_of_out_channel oc)
                (Bytes.of_string "\n")
                0
                1
            in
            let tracker =
              Connection_tracker.track tracker ~key:Client_sent_version
            in

            (* 3. wait for Connection_ok *)
            (* timeout/exn -> Server_missing_exn/Monitor_connection_misc_exception *)
            phase := MonitorUtils.Connect_receive_connection_ok;
            let cstate : connection_state = from_channel_without_buffering ic in
            let tracker =
              Connection_tracker.track tracker ~key:Client_got_cstate
            in
            let _ =
              match !warn_of_busy_server_timer with
              | None -> ()
              | Some timer -> Timer.cancel_timer timer
            in

            (* 4. return Ok *)
            match cstate with
            | Connection_ok
            | Connection_ok_v2 _ ->
              finally_close := None;
              Ok (ic, oc, tracker)
            | Build_id_mismatch ->
              wait_on_server_restart ic;
              Error (Build_id_mismatched_monitor_will_terminate None)
            | Build_id_mismatch_ex mismatch_info ->
              wait_on_server_restart ic;
              Error
                (Build_id_mismatched_monitor_will_terminate (Some mismatch_info))
            | Build_id_mismatch_v3 (mismatch_info, mismatch_payload) ->
              let monitor_will_terminate =
                match
                  MonitorUtils.MismatchPayload.deserialize mismatch_payload
                with
                | Error msg ->
                  Hh_logger.log
                    "Unparseable response from monitor - %s\n%s"
                    msg
                    mismatch_payload;
                  true
                | Ok { MonitorUtils.MismatchPayload.monitor_will_terminate } ->
                  monitor_will_terminate
              in
              if monitor_will_terminate then begin
                wait_on_server_restart ic;
                Error
                  (Build_id_mismatched_monitor_will_terminate
                     (Some mismatch_info))
              end else
                Error (Build_id_mismatched_client_must_terminate mismatch_info))
          ~on_timeout:(fun _timings ->
            Error
              (Connect_to_monitor_failure
                 {
                   server_exists = server_exists config.lock_file;
                   failure_phase = !phase;
                   failure_reason = Connect_timeout;
                 }))
      with
      | Unix.Unix_error _ as exn ->
        let e = Exception.wrap exn in
        Error
          (Connect_to_monitor_failure
             {
               server_exists = server_exists config.lock_file;
               failure_phase = !phase;
               failure_reason = Connect_exception e;
             }))
    ~finally:(fun () ->
      match !finally_close with
      | None -> ()
      | Some ic ->
        Timeout.shutdown_connection ic;
        Timeout.close_in_noerr ic)

let connect_and_shut_down ~tracker root =
  let open Result.Monad_infix in
  let config = hh_monitor_config root in
  connect_to_monitor
    ~tracker
    ~timeout:3
    ~terminate_monitor_on_version_mismatch:true
    config
  >>= fun (ic, oc, tracker) ->
  log "send_shutdown" ~tracker;
  let (_ : int) =
    Marshal_tools.to_fd_with_preamble
      (Unix.descr_of_out_channel oc)
      (MonitorRpc.SHUT_DOWN tracker)
  in
  Timeout.with_timeout
    ~timeout:3
    ~on_timeout:(fun _timings ->
      if server_exists config.lock_file then
        Ok MonitorUtils.SHUTDOWN_UNVERIFIED
      else
        Error
          (Connect_to_monitor_failure
             {
               server_exists = false;
               failure_phase = Connect_send_shutdown;
               failure_reason = Connect_timeout;
             }))
    ~do_:
      begin
        fun _ ->
          wait_on_server_restart ic;
          Ok MonitorUtils.SHUTDOWN_VERIFIED
      end

let connect_once
    ~tracker
    ~timeout
    ~terminate_monitor_on_version_mismatch
    root
    handoff_options =
  let open Result.Monad_infix in
  let config = hh_monitor_config root in
  let t_start = Unix.gettimeofday () in
  let result =
    let tracker =
      Connection_tracker.(track tracker ~key:Client_start_connect ~time:t_start)
    in
    connect_to_monitor
      ~tracker
      ~timeout
      ~terminate_monitor_on_version_mismatch
      config
    >>= fun (ic, oc, tracker) ->
    let tracker =
      Connection_tracker.(track tracker ~key:Client_ready_to_send_handoff)
    in
    let (_ : int) =
      Marshal_tools.to_fd_with_preamble
        (Unix.descr_of_out_channel oc)
        (MonitorRpc.HANDOFF_TO_SERVER (tracker, handoff_options))
    in
    let elapsed_t = int_of_float (Unix.gettimeofday () -. t_start) in
    let timeout = max (timeout - elapsed_t) 1 in
    Timeout.with_timeout
      ~timeout
      ~do_:(fun timeout -> consume_prehandoff_messages ~timeout ic oc)
      ~on_timeout:(fun _ -> Error MonitorUtils.Server_dormant_out_of_retries)
  in
  (* oops too heavy *)
  Result.iter result ~f:(fun _ ->
      log ~tracker "CLIENT_CONNECT_ONCE";
      HackEventLogger.client_connect_once ~t_start;
      ());
  Result.iter_error result ~f:(fun e ->
      let (reason, telemetry) = MonitorUtils.connection_error_to_telemetry e in
      log ~tracker "CLIENT_CONNECT_ONCE %s" reason;
      HackEventLogger.client_connect_once_failure ~t_start reason telemetry;
      ());
  result
