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

let wait_on_server_restart ic =
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

let send_version oc =
  Marshal_tools.to_fd_with_preamble
    (Unix.descr_of_out_channel oc)
    Build_id.build_revision
  |> ignore;

  (* For backwards-compatibility, newline has always followed the version *)
  let (_ : int) =
    Unix.write (Unix.descr_of_out_channel oc) (Bytes.of_string "\n") 0 1
  in
  ()

let send_server_handoff_rpc ~tracker handoff_options oc =
  Marshal_tools.to_fd_with_preamble
    (Unix.descr_of_out_channel oc)
    (MonitorRpc.HANDOFF_TO_SERVER (tracker, handoff_options))
  |> ignore

let send_shutdown_rpc ~tracker oc =
  log "send_shutdown" ~tracker;
  Marshal_tools.to_fd_with_preamble
    (Unix.descr_of_out_channel oc)
    (MonitorRpc.SHUT_DOWN tracker)
  |> ignore

let send_server_progress_rpc ~tracker oc =
  log "send_server_process_rpc" ~tracker;
  let (_ : int) =
    Marshal_tools.to_fd_with_preamble
      (Unix.descr_of_out_channel oc)
      (MonitorRpc.SERVER_PROGRESS tracker)
  in
  ()

let read_server_progress ~tracker ic : string * string option =
  log "read_server_progress" ~tracker;
  from_channel_without_buffering ic

let establish_connection ~timeout config =
  let sock_name = Socket.get_path config.socket_file in
  let sockaddr =
    if Sys.win32 then (
      let ic = In_channel.create ~binary:true sock_name in
      let port = Option.value_exn (In_channel.input_binary_int ic) in
      In_channel.close ic;
      Unix.(ADDR_INET (inet_addr_loopback, port))
    ) else
      Unix.ADDR_UNIX sock_name
  in
  try Ok (Timeout.open_connection ~timeout sockaddr) with
  | (Unix.Unix_error (Unix.ECONNREFUSED, _, _) as e)
  | (Unix.Unix_error (Unix.ENOENT, _, _) as e) ->
    let e = Exception.wrap e in
    if not (server_exists config.lock_file) then
      Error (Server_missing_exn e)
    else
      Error (Monitor_socket_not_ready e)

let get_cstate
    ~(tracker : Connection_tracker.t)
    (config : ServerMonitorUtils.monitor_config)
    ((ic, oc) : Timeout.in_channel * Out_channel.t) :
    ( Timeout.in_channel
      * Out_channel.t
      * ServerMonitorUtils.connection_state
      * Connection_tracker.t,
      ServerMonitorUtils.connection_error )
    result =
  try
    send_version oc;
    let tracker = Connection_tracker.(track tracker ~key:Client_sent_version) in
    let cstate : connection_state = from_channel_without_buffering ic in
    let tracker = Connection_tracker.(track tracker ~key:Client_got_cstate) in
    Ok (ic, oc, cstate, tracker)
  with e ->
    let e = Exception.wrap e in
    log
      "error getting cstate; closing connection. %s"
      ~tracker
      (Exception.to_string e);
    Timeout.shutdown_connection ic;
    Timeout.close_in_noerr ic;
    if not (server_exists config.lock_file) then
      Error (Server_missing_exn e)
    else
      Error (Monitor_connection_failure e)

let verify_cstate ~tracker ic cstate =
  match cstate with
  | Connection_ok
  | Connection_ok_v2 _ ->
    Ok ()
  | Build_id_mismatch_ex mismatch_info
  | Build_id_mismatch_v3 (mismatch_info, _) ->
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
    log "verify_cstate: waiting on server restart" ~tracker;
    wait_on_server_restart ic;
    log "verify_cstate: closing ic" ~tracker;
    Timeout.close_in_noerr ic;
    Error (Build_id_mismatched (Some mismatch_info))
  | Build_id_mismatch ->
    (* The server no longer ever sends this message, as of July 2017 *)
    failwith "Ancient version of server sent old Build_id_mismatch"

(* Consume sequence of Prehandoff messages. *)
let rec consume_prehandoff_messages
    ~(timeout : Timeout.t) (ic : Timeout.in_channel) (oc : Stdlib.out_channel) :
    ( Timeout.in_channel * Stdlib.out_channel * string,
      ServerMonitorUtils.connection_error )
    result =
  let module PH = Prehandoff in
  let m : PH.msg = from_channel_without_buffering ~timeout ic in
  match m with
  | PH.Sentinel finale_file -> Ok (ic, oc, finale_file)
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

let consume_prehandoff_messages
    ~(timeout : int) (ic : Timeout.in_channel) (oc : Stdlib.out_channel) :
    ( Timeout.in_channel * Stdlib.out_channel * string,
      ServerMonitorUtils.connection_error )
    result =
  Timeout.with_timeout
    ~timeout
    ~do_:(fun timeout -> consume_prehandoff_messages ~timeout ic oc)
    ~on_timeout:(fun _ ->
      Error ServerMonitorUtils.Server_dormant_out_of_retries)

let connect_to_monitor ~tracker ~timeout config =
  let open Result.Monad_infix in
  Timeout.with_timeout
    ~timeout
    ~on_timeout:(fun timings ->
      (*
      * Monitor should always readily accept connections. In theory, this will
      * only timeout if the Monitor is being very heavily DDOS'd, or the Monitor
      * has wedged itself (a bug).
      *
      * The DDOS occurs when the Monitor's new connections (arriving on
      * the socket) queue grows faster than they are being processed. This can
      * happen in two scenarios:
        * 1) Malicious DDOSer fills up new connection queue (incoming
        *    connections on the socket) quicker than the queue is being
        *    consumed.
        * 2) New client connections to the monitor are being created by the
        *    retry logic in hh_client faster than those cancelled connections
        *    (cancelled due to the timeout above) are being discarded by the
        *    monitor. This could happen from thousands of hh_clients being
        *    used to parallelize a job. This is effectively an inadvertent DDOS.
        *    In detail, suppose the timeout above is set to 1 ssecond and that
        *    1000 thousand hh_client have timed out at the line above. Then these
        *    1000 clients will cancel the connection and retry. But the Monitor's
        *    connection queue still has these dead/canceled connections waiting
        *    to be processed. Suppose it takes the monitor longer than 1
        *    millisecond to handle and discard a dead connection. Then the
        *    1000 retrying hh_clients will again add another 1000 dead
        *    connections during retrying even tho the monitor has discarded
        *    fewer than 1000 dead connections. Thus, no progress will be made
        *    on clearing out dead connections and all new connection attempts
        *    will time out.
        *
        *    We ameliorate this by having the timeout be quite large
        *    (many seconds) and by not auto-retrying connections to the Monitor.
      * *)
      HackEventLogger.client_connect_to_monitor_timeout ();
      let exists_lock_file = server_exists config.lock_file in
      log
        "connect_to_monitor: lockfile=%b timeout=%s"
        ~tracker
        exists_lock_file
        (Timeout.show_timings timings);
      if not exists_lock_file then
        Error (Server_missing_timeout timings)
      else
        Error ServerMonitorUtils.Monitor_establish_connection_timeout)
    ~do_:
      begin
        fun timeout ->
        establish_connection ~timeout config >>= fun (ic, oc) ->
        let tracker =
          Connection_tracker.(track tracker ~key:Client_opened_socket)
        in
        get_cstate ~tracker config (ic, oc)
      end

let connect_and_shut_down ~tracker config =
  let open Result.Monad_infix in
  connect_to_monitor ~tracker ~timeout:3 config
  >>= fun (ic, oc, cstate, tracker) ->
  verify_cstate ~tracker ic cstate >>= fun () ->
  send_shutdown_rpc ~tracker oc;
  Timeout.with_timeout
    ~timeout:3
    ~on_timeout:(fun timings ->
      if not (server_exists config.lock_file) then
        Error (Server_missing_timeout timings)
      else
        Ok ServerMonitorUtils.SHUTDOWN_UNVERIFIED)
    ~do_:
      begin
        fun _ ->
        wait_on_server_restart ic;
        Ok ServerMonitorUtils.SHUTDOWN_VERIFIED
      end

(** connect_once.
1. OPEN SOCKET. After this point we have a working stdin/stdout to the
process. Implemented in establish_connection.
  | catch EConnRefused/ENoEnt/Timeout 1s when lockfile present ->
    Error Monitor_socket_not_ready.
      This is unexpected! But can happen if you manage to catch the
      monitor in the short timeframe after it has grabbed its lock but
      before it has started listening in on its socket.
      -> "hh_client check/ide" -> retry from step 1, up to 800 times.
         The number 800 is hard-coded in 9 places through the codebase.
      -> "hh_client start" -> print "replacing unresponsive server"
             kill_server; start_server; exit.
  | catch Timeout <retries>s when lockfile present ->
    Error Monitor_establish_connection_timeout
      This is unexpected! after all the monitor is always responsive,
      and indeed start_server waits until responsive before returning.
      But this can happen during a DDOS.
      -> "hh_client check/ide" -> Its retry attempts are passed to the
          monitor connection attempt already. So in this timeout all
          the retries have already been consumed. Just exit.
      -> "hh_client start" -> print "replacing unresponsive server"
             kill_server; start_server; exit.
  | catch EConnRefused/ENoEnt/Timeout when lockfile absent ->
    Error Server_missing.
      -> "hh_client ide" -> raise Exit_with IDE_no_server.
      -> "hh_client check" -> start_server; retry step 1, up to 800x.
      -> "hh_client start" -> start_server; exit.
  | catch other exception -> unhandled.

2. SEND VERSION; READ VERSION; CHECK VERSIONS. After this point we can
safely marshal OCaml types back and forth. Implemented in get_cstate
and verify_cstate.
  | catch any exception when lockfile present ->
    close_connection; Error Monitor_connection_failure.
      This is unexpected!
      -> "hh_client check/ide" -> retry from step 1, up to 800 times.
      -> "hh_client start" -> print "replacing unresponsive server"
             kill_server; start_server; exit.
  | catch any exception when lockfile absent ->
    close_connection; Error Server_missing.
      -> "hh_client ide" -> raise Exit_with IDE_no_server
      -> "hh_client check" -> start_server; retry step 1, up to 800x.
      -> "hh_client start" -> start_server; exit.
  | if version numbers differ ->
    Error Build_mismatch.
      -> "hh_client ide" -> raise Exit_with IDE_no_server.
      -> "hh_client check" -> close_log_tailer; retry from step 1.
      -> "hh_client start" -> start_server; exit.

3. SEND HANDOFF; READ RESPONSE. After this point we have a working
connection to a server who we believe is ready to handle our messages.
Handoff is the stage of the protocol when we're speaking to the monitor
rather than directly to the server process itself. Implemented in
send_server_handoff_rpc and consume_prehandoff_message.
  | response Server_name_not_found ->
    raise Exit_with Server_name_not_found.
  | response Server_not_alive_dormant ->
    print "Waiting for server to start"; retry step 5, unlimited times.
  | response Server_dormant_connections_limit_reached ->
    Error Server_dormant.
      -> "hh_client ide" -> raise Exit_with IDE_no_server.
      -> "hh_client start" -> print "Server already exists but is
        dormant"; exit.
      -> "hh_client check" -> print "No server running, and connection
        limit reached for waiting  on the next server to be started.
        Please wait patiently." raise Exit_with No_server_running.
  | response Server_died ->
    print "Last killed by OOM / signal / stopped by signal / exited";
    wait for server to close; Error Server_died.
      -> "hh_client ide" -> raise Exit_with IDE_no_server.
      -> "hh_client start" -> start_server.
      -> "hh_client check" -> retry from step 1, up to 800 times.
  | catch any exception -> unhandled.

The following two steps aren't implemented inside connect_once but are
typically done by callers after connect_once has succeeded...

4. READ "HELLO" FROM SERVER. After this point we have evidence that the
server is ready to handle our messages. We basically gobble whatever
the server sends until it finally sends a line with just "hello".
Implemented in wait_for_server_hello.
  | read anything other than "hello" -> retry from step 4, up to 800x.
  | catch Timeout 1s -> retry from step 4, up to 800 times.
  | catch exception EndOfFile/Sys_error ->
    raise ServerHungUp.
      -> "hh_client ide/check" -> program exit, code=No_server_running.
      -> clientStart never actually bothers to do step 4.
  | catch other exception -> unhandled.

5. SEND CONNECTION TYPE; READ RESPONSE. After this point we have
evidence that the server is able to handle our connection. The
connection type indicates Persistent vs Non-persistent.
  | response Denied_due_to_existing_persistent_connection.
      -> "hh_client lsp" -> raise Lsp.Error_server_start.
  | catch any exception -> unhandled.
*)
let connect_once ~tracker ~timeout config handoff_options =
  let open Result.Monad_infix in
  let t_start = Unix.gettimeofday () in
  let tracker =
    Connection_tracker.(track tracker ~key:Client_start_connect ~time:t_start)
  in
  connect_to_monitor ~tracker ~timeout config
  >>= fun (ic, oc, cstate, tracker) ->
  verify_cstate ~tracker ic cstate >>= fun () ->
  let tracker =
    Connection_tracker.(track tracker ~key:Client_ready_to_send_handoff)
  in
  send_server_handoff_rpc ~tracker handoff_options oc;
  let elapsed_t = int_of_float (Unix.gettimeofday () -. t_start) in
  let timeout = max (timeout - elapsed_t) 1 in
  consume_prehandoff_messages ~timeout ic oc

let connect_to_monitor_and_get_server_progress ~tracker ~timeout config :
    (string * string option, ServerMonitorUtils.connection_error) result =
  let open Result.Monad_infix in
  connect_to_monitor ~tracker ~timeout config
  >>= fun (ic, oc, cstate, tracker) ->
  verify_cstate ~tracker ic cstate >>= fun () ->
  (* This is similar to connect_once up to this point, where instead of
    * being handed off to server we just get our answer from monitor *)
  send_server_progress_rpc ~tracker oc;
  Ok (read_server_progress ~tracker ic)
