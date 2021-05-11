(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerMonitorUtils

let log s ~tracker =
  Hh_logger.log ("[%s] " ^^ s) (Connection_tracker.log_id tracker)

let server_exists lock_file = not (Lock.check lock_file)

let from_channel_without_buffering ?timeout tic =
  Marshal_tools.from_fd_with_preamble ?timeout (Timeout.descr_of_in_channel tic)

let hh_monitor_config root =
  ServerMonitorUtils.
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
   * See also: ServerMonitor.client_out_of_date
   *)
  try
    while true do
      let _ = Timeout.input_char ic in
      ()
    done
  with
  | End_of_file
  | Sys_error _ ->
    (* Server has exited and hung up on us *)
    ()

let produce_version_payload ~(tracker : Connection_tracker.t) : string =
  Hh_json.JSON_Object
    [
      ("client_version", Hh_json.JSON_String Build_id.build_revision);
      ("tracker_id", Hh_json.JSON_String (Connection_tracker.log_id tracker));
    ]
  |> Hh_json.json_to_string

let get_sockaddr config =
  let sock_name = Socket.get_path config.ServerMonitorUtils.socket_file in
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
      ServerMonitorUtils.connection_error )
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
      ( "Last server exited due to config change. Please re-run client"
      ^^ " to force discovery of the correct version of the client." );
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

let connect_to_monitor ~tracker ~timeout config =
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
  let phase = ref ServerMonitorUtils.Connect_open_socket in
  let finally_close : Timeout.in_channel option ref = ref None in
  Utils.try_finally
    ~f:(fun () ->
      try
        (* Note: Timeout.with_timeout is accomplished internally by an exception;
        hence, our "try/with exn ->" catch-all must be outside with_timeout. *)
        Timeout.with_timeout
          ~timeout
          ~do_:(fun timeout ->
            (* 1. open the socket *)
            phase := ServerMonitorUtils.Connect_open_socket;
            let sockaddr = get_sockaddr config in
            let (ic, oc) = Timeout.open_connection ~timeout sockaddr in
            finally_close := Some ic;
            let tracker =
              Connection_tracker.track tracker ~key:Client_opened_socket
            in

            (* 2. send version, followed by newline *)
            phase := ServerMonitorUtils.Connect_send_version;
            let version_str = produce_version_payload ~tracker in
            let (_ : int) =
              Marshal_tools.to_fd_with_preamble
                (Unix.descr_of_out_channel oc)
                version_str
            in
            phase := ServerMonitorUtils.Connect_send_newline;
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
            phase := ServerMonitorUtils.Connect_receive_connection_ok;
            let cstate : connection_state = from_channel_without_buffering ic in
            let tracker =
              Connection_tracker.track tracker ~key:Client_got_cstate
            in

            (* 4. return Ok *)
            match cstate with
            | Build_id_mismatch_ex mismatch_info
            | Build_id_mismatch_v3 (mismatch_info, _) ->
              wait_on_server_restart ic;
              Error (Build_id_mismatched (Some mismatch_info))
            | _ ->
              finally_close := None;
              Ok (ic, oc, tracker))
          ~on_timeout:(fun _timings ->
            Error
              (Connect_to_monitor_failure
                 {
                   server_exists = server_exists config.lock_file;
                   failure_phase = !phase;
                   failure_reason = Connect_timeout;
                 }))
      with exn ->
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
  connect_to_monitor ~tracker ~timeout:3 config >>= fun (ic, oc, tracker) ->
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
        Ok ServerMonitorUtils.SHUTDOWN_UNVERIFIED
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
        Ok ServerMonitorUtils.SHUTDOWN_VERIFIED
      end

let connect_once ~tracker ~timeout root handoff_options =
  let open Result.Monad_infix in
  let config = hh_monitor_config root in
  let t_start = Unix.gettimeofday () in
  let result =
    let tracker =
      Connection_tracker.(track tracker ~key:Client_start_connect ~time:t_start)
    in
    connect_to_monitor ~tracker ~timeout config >>= fun (ic, oc, tracker) ->
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
      ~on_timeout:(fun _ ->
        Error ServerMonitorUtils.Server_dormant_out_of_retries)
  in
  Result.iter result ~f:(fun _ ->
      log ~tracker "CLIENT_CONNECT_ONCE";
      HackEventLogger.client_connect_once ~t_start;
      ());
  Result.iter_error result ~f:(fun e ->
      let telemetry = ServerMonitorUtils.connection_error_to_telemetry e in
      log
        ~tracker
        "CLIENT_CONNECT_ONCE FAILURE %s"
        (telemetry |> Telemetry.to_string);
      HackEventLogger.client_connect_once_failure ~t_start telemetry;
      ());
  result
