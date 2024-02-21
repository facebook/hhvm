(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * The server monitor is the parent process for a server. It
 * listens to a socket for client connections and passes the connections
 * to the server and serves the following objectives:
 *
   * 1) Readily accepts client connections
   * 2) Confirms a Build ID match (killing itself and the server quickly
   *    on mismatch)
   * 3) Hands the client connection to the daemon server
   * 4) Tracks when the server process crashes or OOMs and echos
   *    its fate to the next client.
 *)

open Hh_prelude.Result.Monad_infix
open Hh_prelude
open ServerProcess
open MonitorUtils

let log s ~tracker =
  Hh_logger.log ("[%s] " ^^ s) (Connection_tracker.log_id tracker)

(** If you try to Unix.close the same FD twice, you get EBADF the second time.
Ocaml doesn't have a linear type system, so it's too hard to be crisp about
when we hand off responsibility for closing the FD. Therefore: we call this
method to ensure an FD is closed, and we just silently gobble up the EBADF
if someone else already closed it. *)
let ensure_fd_closed (fd : Unix.file_descr) : unit =
  try Unix.close fd with
  | Unix.Unix_error (Unix.EBADF, _, _) -> ()

(** This module is to help using Unix "sendmsg" to handoff the client FD
to the server. It's not entirely clear whether it's safe for us in the
monitor to close the FD immediately after calling sendmsg, or whether
we must wait until the server has actually received the FD upon recvmsg.

We got reports from MacOs users that if the monitor closed the FD before
the server had finished recvmsg, then the kernel thinks it was the last open
descriptor for the pipe, and actually closes it; the server subsequently
does recvmsg and reads on the FD and gets an EOF (even though it can write
on the FD and succeed instead of getting EPIPE); the server subsequently
closes its FD and a subsequent "select" by the client blocks forever.

Therefore, instead, the monitor waits to close the FD until after the server has read it.

We detect that the server has read it by having the server write the highest
handoff number it has received to a "server_receipt_to_monitor_file", and the
monitor polls this file to determine which handoff FD can be closed. It might
be that the server receives two FD handoffs in quick succession, and the monitor
only polls the file after the second, so the monitor treats the file as a
"high water mark" and knows that it can close the specified FD plus all earlier ones.

We did this protocol with a file because there aren't alternatives. If we tried
instead to have the server send receipt over the monitor/server pipe, then it
would deadlock if the monitor was also trying to handoff a subsequent FD.
If we tried instead to have the server send receipt over the client/server pipe,
then both monitor and client would be racing to see who receives that receipt first. *)
module Sent_fds_collector = struct
  (** [sequence_number] is a monotonically increasing integer to identify which
  handoff message has been sent from monitor to server. The first sequence
  number is number 1. *)
  let sequence_number : int ref = ref 0

  let get_and_increment_sequence_number () : int =
    sequence_number := !sequence_number + 1;
    !sequence_number

  type handed_off_fd_to_close = {
    tracker: Connection_tracker.t;
    fd: Unix.file_descr;
    m2s_sequence_number: int;
        (** A unique number incremented for each client socket handoff from monitor to server.
            Useful to correlate monitor and server logs. *)
  }

  let handed_off_fds_to_close : handed_off_fd_to_close list ref = ref []

  let cleanup_fd ~tracker ~m2s_sequence_number fd =
    handed_off_fds_to_close :=
      { tracker; fd; m2s_sequence_number } :: !handed_off_fds_to_close

  let collect_fds ~sequence_receipt_high_water_mark =
    (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)

    (* Note: this code is a filter with a side-effect. The side-effect is to
       close FDs that have received their read-receipts. The filter will retain
       the FDs that haven't yet received their read-receipts. *)
    handed_off_fds_to_close :=
      List.filter
        !handed_off_fds_to_close
        ~f:(fun { tracker; m2s_sequence_number; fd } ->
          if m2s_sequence_number > sequence_receipt_high_water_mark then
            true
          else begin
            log
              "closing client socket from handoff #%d upon receipt of #%d"
              ~tracker
              m2s_sequence_number
              sequence_receipt_high_water_mark;
            ensure_fd_closed fd;
            false
          end);
    ()

  (** This must be called in all paths where the server has died, so that the client can
  properly see an EOF on its FD. *)
  let collect_all_fds () =
    collect_fds ~sequence_receipt_high_water_mark:Int.max_value
end

exception Send_fd_failure of int

type env = {
  informant: Informant.t;
  server: ServerProcess.server_process;
  server_start_options: ServerController.server_start_options;
  (* How many times have we tried to relaunch it? *)
  retries: int;
  sql_retries: int;
  watchman_retries: int;
  max_purgatory_clients: int;
  (* Version of this running server, as specified in the config file. *)
  current_version: Config_file.version;
  (* After sending a Server_not_alive_dormant during Prehandoff,
   * clients are put here waiting for a server to come alive, at
   * which point they get pushed through the rest of prehandoff and
   * then sent to the living server.
   *
   * String is the server name it wants to connect to. *)
  purgatory_clients:
    (Connection_tracker.t * MonitorRpc.handoff_options * Unix.file_descr)
    Queue.t;
  (* Whether to ignore hh version mismatches *)
  ignore_hh_version: bool;
}

type t = env * MonitorUtils.monitor_config * Unix.file_descr

type 'a msg_update = ('a, 'a * string) result

(** Use this as the only way to change server.
  It closes all outstanding FDs. That's needed so clients know that they can give up. *)
let set_server (env : env msg_update) new_server : env msg_update =
  env >>= fun env ->
  Sent_fds_collector.collect_all_fds ();
  Ok { env with server = new_server }

let msg_to_channel fd msg =
  (* This FD will be passed to a server process, so avoid using Ocaml's
   * channels which have built-in buffering. Even though we are only writing
   * to the FD here, it seems using Ocaml's channels also causes read
   * buffering to happen here, so the server process doesn't get what was
   * meant for it. *)
  Marshal_tools.to_fd_with_preamble fd msg |> ignore

let setup_handler_for_signals handler signals =
  List.iter signals ~f:(fun signal ->
      Sys_utils.set_signal signal (Sys.Signal_handle handler))

let setup_autokill_server_on_exit process =
  try
    setup_handler_for_signals
      begin
        fun _ ->
          Hh_logger.log "Got an exit signal. Killing server and exiting.";
          ServerController.kill_server ~violently:false process;
          Exit.exit Exit_status.Interrupted
      end
      [Sys.sigint; Sys.sigquit; Sys.sigterm; Sys.sighup]
  with
  | _ -> Hh_logger.log "Failed to set signal handler"

let sleep_and_check socket =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let (ready_socket_l, _, _) = Unix.select [socket] [] [] 1.0 in
  not (List.is_empty ready_socket_l)

let start_server ~informant_managed options exit_status =
  let server_process =
    ServerController.start_server
      ~prior_exit_status:exit_status
      ~informant_managed
      options
  in
  setup_autokill_server_on_exit server_process;
  Alive server_process

let maybe_start_first_server options informant : ServerProcess.server_process =
  if Informant.should_start_first_server informant then (
    Hh_logger.log "Starting first server";
    HackEventLogger.starting_first_server ();
    start_server
      ~informant_managed:(Informant.is_managing informant)
      options
      None
  ) else (
    Hh_logger.log
      ("Not starting first server. "
      ^^ "Starting will be triggered by informant later.");
    Not_yet_started
  )

let kill_server_with_check_and_wait (server : server_process) : unit =
  Sent_fds_collector.collect_all_fds ();
  match server with
  | Alive server ->
    let start_t = Unix.gettimeofday () in
    let rec kill_server_with_check_and_wait_impl () =
      ServerController.kill_server
        ~violently:Float.(Unix.gettimeofday () -. start_t >= 4.0)
        server;
      let success =
        ServerController.wait_for_server_exit
          ~timeout_t:(Unix.gettimeofday () +. 2.0)
          server
      in
      if not success then
        kill_server_with_check_and_wait_impl ()
      else
        let (_ : float) =
          Hh_logger.log_duration
            "typechecker has exited. Time since first signal: "
            start_t
        in
        ()
    in
    kill_server_with_check_and_wait_impl ()
  | _ -> ()

(* Reads current hhconfig contents from disk and returns true if the
 * version specified in there matches our currently running version. *)
let is_config_version_matching env =
  let filename =
    Relative_path.from_root ~suffix:Config_file.file_path_relative_to_repo_root
  in
  let (_hash, config) =
    Config_file.parse_hhconfig (Relative_path.to_absolute filename)
  in
  let new_version =
    Config_file.parse_version (Config_file.Getters.string_opt "version" config)
  in
  0 = Config_file.compare_versions env.current_version new_version

(* Actually starts a new server. *)
let start_new_server (env : env msg_update) exit_status : env msg_update =
  env >>= fun env ->
  let informant_managed = Informant.is_managing env.informant in
  let new_server =
    start_server ~informant_managed env.server_start_options exit_status
  in
  let env = set_server (Ok env) new_server in
  env >>= fun new_env -> Ok { new_env with retries = new_env.retries + 1 }

(* Kill the server (if it's running) and restart it - maybe. Obeying the rules
 * of state transitions. See docs on the ServerProcess.server_process ADT for
 * state transitions. *)
let kill_and_maybe_restart_server (env : env msg_update) exit_status :
    env msg_update =
  env >>= fun env ->
  Hh_logger.log
    "kill_and_maybe_restart_server (env.server=%s)"
    (show_server_process env.server);
  (* We're going to collect all FDs right here and now. This will be done again below,
     in [kill_server_with_check_and_wait], but Informant.reinit might take too long or might throw. *)
  Sent_fds_collector.collect_all_fds ();
  (* Iff we just restart and keep going, we risk
   * Changed_merge_base eventually arriving and restarting the already started server
   * for no reason. Re-issuing merge base query here should bring the Monitor and Server
   * understanding of current revision to be the same *)
  kill_server_with_check_and_wait env.server;

  let version_matches = is_config_version_matching env in
  if ServerController.is_saved_state_precomputed env.server_start_options then begin
    let reason =
      "Not restarting server as requested, server was launched using a "
      ^ "precomputed saved-state. Exiting monitor"
    in
    Hh_logger.log "%s" reason;
    HackEventLogger.refuse_to_restart_server
      ~reason
      ~server_state:(ServerProcess.show_server_process env.server)
      ~version_matches;
    Exit.exit Exit_status.Not_restarting_server_with_precomputed_saved_state
  end;
  match (env.server, version_matches) with
  | (Died_config_changed, _) ->
    (* Now we can start a new instance safely.
     * See diagram on ServerProcess.server_process docs. *)
    start_new_server (Ok env) exit_status
  | (Not_yet_started, false)
  | (Alive _, false)
  | (Died_unexpectedly _, false) ->
    (* Can't start server instance. State goes to Died_config_changed
     * See diagram on ServerProcess.server_process docs. *)
    Hh_logger.log
      "Avoiding starting a new server because version in config no longer matches.";
    set_server (Ok env) Died_config_changed
  | (Not_yet_started, true)
  | (Alive _, true)
  | (Died_unexpectedly _, true) ->
    (* Start new server instance because config matches.
     * See diagram on ServerProcess.server_process docs. *)
    start_new_server (Ok env) exit_status

let read_version_string_then_newline (fd : Unix.file_descr) :
    (MonitorUtils.VersionPayload.serialized, string) result =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let s : string = Marshal_tools.from_fd_with_preamble fd in
  let newline_byte = Bytes.create 1 in
  let _ = Unix.read fd newline_byte 0 1 in
  if String.equal (Bytes.to_string newline_byte) "\n" then
    Ok s
  else
    Error "missing newline after version"

let hand_off_client_connection ~tracker server_fd client_fd =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let m2s_sequence_number =
    Sent_fds_collector.get_and_increment_sequence_number ()
  in
  let msg = MonitorRpc.{ m2s_tracker = tracker; m2s_sequence_number } in
  msg_to_channel server_fd msg;
  log
    "Handed off tracker to server (client socket handoff #%d)"
    ~tracker
    m2s_sequence_number;
  let status = Libancillary.ancil_send_fd server_fd client_fd in
  if status = 0 then begin
    log
      "Handed off client socket to server (handoff #%d)"
      ~tracker
      m2s_sequence_number;
    Sent_fds_collector.cleanup_fd ~tracker ~m2s_sequence_number client_fd
  end else begin
    log
      "Failed to handoff client socket to server (handoff #%d)"
      ~tracker
      m2s_sequence_number;
    raise (Send_fd_failure status)
  end

(** Sends the client connection FD to the server process then closes the FD.
  Backpressure: We have a timeout of 30s here to wait for the server to accept
  the handoff. That timeout will be exceeded if monitor->server pipe has filled
  up from previous requests and the server's current work item is costly. In this
  case we'll give up on the handoff, and hh_client will fail with Server_hung_up_should_retry,
  and find_hh.sh will retry with exponential backoff.
  During the 30s while we're blocked here, if there are lots of other clients trying
  to connect to the monitor and the monitor's incoming queue is full, they'll time
  out trying to open a connection to the monitor. Their response is to back off,
  with exponentially longer timeouts they're willing to wait for the monitor to become
  available. In this way the queue of clients is stored in the unix process list.
  Why did we pick 30s? It's arbitrary. If we decreased then, if there are lots of clients,
  they'll do more work while they needlessly cycle. If we increased up to infinite
  then I worry that a failure for other reasons might look like a hang.
  This 30s must be comfortably shorter than the 60s delay in ClientConnect.connect, since
  if not then by the time we in the monitor timeout we'll find that every single item
  in our incoming queue is already stale! *)
let hand_off_client_connection_wrapper ~tracker server_fd client_fd =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let timeout = 30.0 in
  let to_finally_close = ref (Some client_fd) in
  Utils.try_finally
    ~f:(fun () ->
      let (_, ready_l, _) = Unix.select [] [server_fd] [] timeout in
      if List.is_empty ready_l then
        log
          ~tracker
          "Handoff FD timed out (%.1fs): server's current iteration is taking longer than that, and its incoming pipe is full"
          timeout
      else
        try
          hand_off_client_connection ~tracker server_fd client_fd;
          to_finally_close := None
        with
        | Unix.Unix_error (Unix.EPIPE, _, _) ->
          log
            ~tracker
            "Handoff FD failed: server has closed its end of pipe (EPIPE), maybe due to large rebase or incompatible .hhconfig change or crash"
        | exn ->
          let e = Exception.wrap exn in
          log
            ~tracker
            "Handoff FD failed unexpectedly - %s"
            (Exception.to_string e);
          HackEventLogger.send_fd_failure e)
    ~finally:(fun () ->
      match !to_finally_close with
      | None -> ()
      | Some client_fd ->
        log
          ~tracker
          "Sending Monitor_failed_to_handoff to client, and closing FD";
        msg_to_channel client_fd ServerCommandTypes.Monitor_failed_to_handoff;
        ensure_fd_closed client_fd)

(** Send (possibly empty) sequences of messages before handing off to
      server. *)
let rec client_prehandoff
    ~tracker
    ~is_purgatory_client
    (env : env msg_update)
    handoff_options
    client_fd : env msg_update =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let module PH = Prehandoff in
  env >>= fun env ->
  match env.server with
  | Alive server ->
    let server_fd =
      snd
      @@ List.find_exn server.out_fds ~f:(fun x ->
             String.equal (fst x) handoff_options.MonitorRpc.pipe_name)
    in
    let t_ready = Unix.gettimeofday () in
    let tracker =
      Connection_tracker.(track tracker ~key:Monitor_ready ~time:t_ready)
    in
    (* TODO: Send this to client so it is visible. *)
    log
      "Got %s request for typechecker. Prior request %.1f seconds ago"
      ~tracker
      handoff_options.MonitorRpc.pipe_name
      (t_ready -. !(server.last_request_handoff));
    msg_to_channel client_fd (PH.Sentinel server.server_specific_files);
    let tracker =
      Connection_tracker.(track tracker ~key:Monitor_sent_ack_to_client)
    in
    hand_off_client_connection_wrapper ~tracker server_fd client_fd;
    server.last_request_handoff := Unix.time ();
    Ok env
  | Died_unexpectedly (status, was_oom) ->
    (* Server has died; notify the client *)
    msg_to_channel client_fd (PH.Server_died { PH.status; PH.was_oom });

    (* Next client to connect starts a new server. *)
    Exit.exit Exit_status.No_error
  | Died_config_changed ->
    if not is_purgatory_client then (
      let env = kill_and_maybe_restart_server (Ok env) None in
      env >>= fun env ->
      (* Assert that the restart succeeded, and then push prehandoff through again. *)
      match env.server with
      | Alive _ ->
        (* Server restarted. We want to re-run prehandoff, which will
         * actually do the prehandoff this time. *)
        client_prehandoff
          ~tracker
          ~is_purgatory_client
          (Ok env)
          handoff_options
          client_fd
      | Died_unexpectedly _
      | Died_config_changed
      | Not_yet_started ->
        Hh_logger.log
          ("Unreachable state. Server should be alive after trying a restart"
          ^^ " from Died_config_changed state");
        failwith
          "Failed starting server transitioning off Died_config_changed state"
    ) else (
      msg_to_channel client_fd PH.Server_died_config_change;
      Ok env
    )
  | Not_yet_started ->
    let env : env msg_update =
      if handoff_options.MonitorRpc.force_dormant_start then (
        msg_to_channel
          client_fd
          (PH.Server_not_alive_dormant
             "Warning - starting a server by force-dormant-start option...");
        kill_and_maybe_restart_server (Ok env) None
      ) else (
        msg_to_channel
          client_fd
          (PH.Server_not_alive_dormant
             "Server killed by informant. Waiting for next server...");
        Ok env
      )
    in
    env >>= fun env ->
    if Queue.length env.purgatory_clients >= env.max_purgatory_clients then
      let () =
        msg_to_channel client_fd PH.Server_dormant_connections_limit_reached
      in
      Ok env
    else
      let () =
        Queue.enqueue env.purgatory_clients (tracker, handoff_options, client_fd)
      in
      Ok env

let handle_monitor_rpc (env : env msg_update) client_fd =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  env >>= fun env ->
  let cmd : MonitorRpc.command =
    Marshal_tools.from_fd_with_preamble client_fd
  in
  match cmd with
  | MonitorRpc.HANDOFF_TO_SERVER (tracker, handoff_options) ->
    let tracker =
      Connection_tracker.(track tracker ~key:Monitor_received_handoff)
    in
    client_prehandoff
      ~tracker
      ~is_purgatory_client:false
      (Ok env)
      handoff_options
      client_fd
  | MonitorRpc.SHUT_DOWN tracker ->
    log "Got shutdown RPC. Shutting down." ~tracker;
    kill_server_with_check_and_wait env.server;
    Exit.exit Exit_status.No_error

let ack_and_handoff_client (env : env msg_update) client_fd : env msg_update =
  env >>= fun env ->
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  let start_time = Unix.gettimeofday () in
  read_version_string_then_newline client_fd
  >>= MonitorUtils.VersionPayload.deserialize
  |> Result.map_error ~f:(fun _err -> (env, "Malformed Build ID"))
  >>= fun {
            MonitorUtils.VersionPayload.client_version;
            tracker_id;
            terminate_monitor_on_version_mismatch;
          } ->
  Hh_logger.log
    "[%s] read_version: start_wait=%s, client_version=%s, monitor_version=%s, monitor_ignore_hh_version=%b, terminate_monitor_on_version_mismatch=%b"
    tracker_id
    (start_time |> Utils.timestring)
    client_version
    Build_id.build_revision
    env.ignore_hh_version
    terminate_monitor_on_version_mismatch;
  (* Version is okay if *monitor* was started with --ignore-hh-version, or versions match. *)
  let is_version_ok =
    env.ignore_hh_version || String.equal client_version Build_id.build_revision
  in
  if is_version_ok then begin
    let connection_state = Connection_ok in
    Hh_logger.log
      "[%s] sending %s"
      tracker_id
      (MonitorUtils.show_connection_state connection_state);
    msg_to_channel client_fd connection_state;
    (* following function returns [env msg_update] monad *)
    handle_monitor_rpc (Ok env) client_fd
  end else begin
    let monitor_will_terminate = terminate_monitor_on_version_mismatch in
    let connection_state =
      Build_id_mismatch_v3
        ( MonitorUtils.current_build_info,
          MonitorUtils.MismatchPayload.serialize ~monitor_will_terminate )
    in
    Hh_logger.log
      "[%s] sending %s"
      tracker_id
      (MonitorUtils.show_connection_state connection_state);
    (try msg_to_channel client_fd connection_state with
    | exn -> Hh_logger.log "while sending, exn: %s" (Exn.to_string exn));
    if monitor_will_terminate then begin
      kill_server_with_check_and_wait env.server;
      Exit.exit Exit_status.Build_id_mismatch
    end else begin
      Hh_logger.log "Closing client_fd";
      (try Unix.close client_fd with
      | exn -> Hh_logger.log "while closing, exn: %s" (Exn.to_string exn));
      Ok env
    end
  end

let push_purgatory_clients (env : env msg_update) : env msg_update =
  (* We create a queue and transfer all the purgatory clients to it before
   * processing to avoid repeatedly retrying the same client even after
   * an EBADF. Control flow is easier this way than trying to manage an
   * immutable env in the face of exceptions. *)
  env >>= fun env ->
  let clients = Queue.create () in
  Queue.blit_transfer ~src:env.purgatory_clients ~dst:clients ();
  let env =
    Queue.fold
      ~f:
        begin
          fun env_accumulator (tracker, handoff_options, client_fd) ->
            try
              client_prehandoff
                ~tracker
                ~is_purgatory_client:true
                env_accumulator
                handoff_options
                client_fd
            with
            | Unix.Unix_error (Unix.EPIPE, _, _)
            | Unix.Unix_error (Unix.EBADF, _, _) ->
              log "Purgatory client disconnected. Dropping." ~tracker;
              env_accumulator
          (* @nzthomas TODO, is this an error? *)
        end
      ~init:(Ok env)
      clients
  in
  env

let maybe_push_purgatory_clients (env : env msg_update) : env msg_update =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  env >>= fun env ->
  match (env.server, Queue.length env.purgatory_clients) with
  | (Alive _, 0) -> Ok env
  | (Died_config_changed, _) ->
    (* These clients are waiting for a server to be started. But this Monitor
     * is waiting for a new client to connect (which confirms to us that we
     * are running the correct version of the Monitor). So let them know
     * that they might want to do something. *)
    push_purgatory_clients (Ok env)
  | (Alive _, _) -> push_purgatory_clients (Ok env)
  | (Not_yet_started, _)
  | (Died_unexpectedly _, _) ->
    Ok env

(* Kill command from client is handled by server server, so the monitor
 * needs to check liveness of the server process to know whether
 * to stop itself. *)
let update_status_ (env : env msg_update) monitor_config :
    (env * int option * Informant.server_state, env * string) result =
  let env =
    env >>= fun env ->
    match env.server with
    | Alive process ->
      let (pid, proc_stat) = ServerController.wait_pid process in
      (match (pid, proc_stat) with
      | (0, _) ->
        (* "pid=0" means the pid we waited for (i.e. process) hasn't yet died/stopped *)
        Ok env
      | (_, Unix.WEXITED 0) ->
        (* "pid<>0, WEXITED 0" means the process had a clean exit *)
        set_server (Ok env) (Died_unexpectedly (proc_stat, false))
      | (_, _) ->
        (* this case is any kind of unexpected exit *)
        let is_oom =
          match proc_stat with
          | Unix.WEXITED code
            when (code = Exit_status.(exit_code Out_of_shared_memory))
                 || code = Exit_status.(exit_code Worker_oomed) ->
            true
          | _ -> Sys_utils.check_dmesg_for_oom process.pid "hh_server"
        in
        let (exit_type, exit_code) = Exit_status.unpack proc_stat in
        let time_taken = Unix.time () -. process.start_t in
        Server_progress.write
          "writing crash log (pid: %d, hint: `hh --logname` OR `bunnylol coredumper`)"
          pid;
        let telemetry =
          Telemetry.create ()
          |> Telemetry.int_ ~key:"pid" ~value:pid
          |> Telemetry.string_ ~key:"unix_exit_type" ~value:exit_type
          |> Telemetry.int_ ~key:"unix_exit_code" ~value:exit_code
          |> Telemetry.bool_ ~key:"is_oom" ~value:is_oom
        in
        let (telemetry, exit_status) =
          match
            Exit_status.get_finale_data
              process.server_specific_files
                .ServerCommandTypes.server_finale_file
          with
          | None -> (telemetry, None)
          | Some
              {
                Exit_status.exit_status;
                msg;
                stack = Utils.Callstack stack;
                telemetry = finale_telemetry;
              } ->
            let telemetry =
              telemetry
              |> Telemetry.string_
                   ~key:"exit_status"
                   ~value:(Exit_status.show exit_status)
              |> Telemetry.string_opt ~key:"msg" ~value:msg
              |> Telemetry.string_ ~key:"stack" ~value:stack
              |> Telemetry.object_opt ~key:"data" ~value:finale_telemetry
            in
            (telemetry, Some exit_status)
        in
        Hh_logger.log "TYPECHECKER_EXIT %s" (Telemetry.to_string telemetry);
        HackEventLogger.typechecker_exit
          telemetry
          time_taken
          (monitor_config.server_log_file, monitor_config.monitor_log_file)
          ~exit_type
          ~exit_code
          ~exit_status
          ~is_oom;
        Server_progress.write
          "%s"
          ~disposition:Server_progress.DStopped
          (Option.value_map
             exit_status
             ~f:Exit_status.show
             ~default:"server stopped");
        set_server (Ok env) (Died_unexpectedly (proc_stat, is_oom)))
    | Not_yet_started
    | Died_config_changed
    | Died_unexpectedly _ ->
      Ok env
  in

  env >>= fun env ->
  let (exit_status, server_state) =
    match env.server with
    | Alive _ -> (None, Informant.Server_alive)
    | Died_unexpectedly (Unix.WEXITED c, _) -> (Some c, Informant.Server_dead)
    | Not_yet_started -> (None, Informant.Server_not_yet_started)
    | Died_unexpectedly ((Unix.WSIGNALED _ | Unix.WSTOPPED _), _)
    | Died_config_changed ->
      (None, Informant.Server_dead)
  in
  Ok (env, exit_status, server_state)

let update_status (env : env msg_update) monitor_config : env msg_update =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  update_status_ env monitor_config >>= fun (env, exit_status, server_state) ->
  let informant_report = Informant.report env.informant server_state in
  let is_watchman_fresh_instance =
    match exit_status with
    | Some c when c = Exit_status.(exit_code Watchman_fresh_instance) -> true
    | _ -> false
  in
  let is_watchman_failed =
    match exit_status with
    | Some c when c = Exit_status.(exit_code Watchman_failed) -> true
    | _ -> false
  in
  let is_config_changed =
    match exit_status with
    | Some c when c = Exit_status.(exit_code Hhconfig_changed) -> true
    | _ -> false
  in
  let is_heap_stale =
    match exit_status with
    | Some c
      when (c = Exit_status.(exit_code File_provider_stale))
           || c = Exit_status.(exit_code Decl_not_found) ->
      true
    | _ -> false
  in
  let is_sql_assertion_failure =
    match exit_status with
    | Some c
      when (c = Exit_status.(exit_code Sql_assertion_failure))
           || (c = Exit_status.(exit_code Sql_cantopen))
           || (c = Exit_status.(exit_code Sql_corrupt))
           || c = Exit_status.(exit_code Sql_misuse) ->
      true
    | _ -> false
  in
  let is_worker_error =
    match exit_status with
    | Some c
      when (c = Exit_status.(exit_code Worker_not_found_exception))
           || (c = Exit_status.(exit_code Worker_busy))
           || c = Exit_status.(exit_code Worker_failed_to_send_job) ->
      true
    | _ -> false
  in
  let is_decl_heap_elems_bug =
    match exit_status with
    | Some c when c = Exit_status.(exit_code Decl_heap_elems_bug) -> true
    | _ -> false
  in
  let is_big_rebase =
    match exit_status with
    | Some c when c = Exit_status.(exit_code Big_rebase_detected) -> true
    | _ -> false
  in
  let max_watchman_retries = 3 in
  let max_sql_retries = 3 in
  let (reason, msg_update) =
    match (informant_report, env.server) with
    | (Informant.Move_along, Died_config_changed) ->
      (* No "reason" here -- we're on an "inner-loop non-failure" path *)
      (None, Ok env)
    | (Informant.Restart_server, Died_config_changed) ->
      let reason =
        "Ignoring Informant directed restart - waiting for next client connection to verify server version first"
      in
      (Some reason, Ok env)
    | (Informant.Restart_server, _) ->
      let reason = "Informant directed server restart. Restarting server." in
      (Some reason, kill_and_maybe_restart_server (Ok env) exit_status)
    | (Informant.Move_along, _) ->
      if
        (is_watchman_failed || is_watchman_fresh_instance)
        && env.watchman_retries < max_watchman_retries
      then
        let reason =
          Printf.sprintf
            "Watchman died. Restarting hh_server (attempt: %d)"
            (env.watchman_retries + 1)
        in
        let env = { env with watchman_retries = env.watchman_retries + 1 } in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_decl_heap_elems_bug then
        let reason = "hh_server died due to Decl_heap_elems_bug. Restarting" in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_worker_error then
        let reason = "hh_server died due to worker error. Restarting" in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_config_changed then
        let reason = "hh_server died from hh config change. Restarting" in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_heap_stale then
        let reason =
          "Several large rebases caused shared heap to be stale. Restarting"
        in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_big_rebase then
        let reason = "Server exited because of big rebase. Restarting" in
        (Some reason, set_server (Ok env) Not_yet_started)
      else if is_sql_assertion_failure && env.sql_retries < max_sql_retries then
        let reason =
          Printf.sprintf
            "Sql failed. Restarting hh_server in fresh mode (attempt: %d)"
            (env.sql_retries + 1)
        in
        let env = { env with sql_retries = env.sql_retries + 1 } in
        (Some reason, set_server (Ok env) Not_yet_started)
      else
        (* No reason here, on the inner-loop non-failure path. *)
        (None, Ok env)
  in
  begin
    match reason with
    | None -> ()
    | Some reason ->
      Hh_logger.log "MONITOR_UPDATE_STATUS %s" reason;
      let (new_server, new_error) =
        match msg_update with
        | Ok env -> (env.server, None)
        | Error (env, s) -> (env.server, Some s)
      in
      let telemetry =
        Telemetry.create ()
        |> Telemetry.string_opt
             ~key:"exit_status"
             ~value:(Option.map exit_status ~f:Exit_status.exit_code_to_string)
        |> Telemetry.string_
             ~key:"informant_report"
             ~value:(Informant.show_report informant_report)
        |> Telemetry.string_
             ~key:"server_state"
             ~value:(Informant.show_server_state server_state)
        |> Telemetry.string_
             ~key:"server"
             ~value:(ServerProcess.show_server_process env.server)
        |> Telemetry.string_
             ~key:"new_server"
             ~value:(ServerProcess.show_server_process new_server)
        |> Telemetry.string_opt ~key:"new_error" ~value:new_error
      in
      HackEventLogger.monitor_update_status reason telemetry
  end;
  msg_update

let rec check_and_run_loop
    ?(consecutive_throws = 0)
    (env : env msg_update)
    monitor_config
    (socket : Unix.file_descr) : 'a =
  let (env, consecutive_throws) =
    try (check_and_run_loop_ env monitor_config socket, 0) with
    | Unix.Unix_error (Unix.ECHILD, _, _) as exn ->
      let e = Exception.wrap exn in
      ignore
        (Hh_logger.log
           "check_and_run_loop_ threw with Unix.ECHILD. Exiting. - %s"
           (Exception.get_backtrace_string e));
      Exit.exit Exit_status.No_server_running_should_retry
    | Watchman.Watchman_restarted ->
      Exit.exit Exit_status.Watchman_fresh_instance
    | Exit_status.Exit_with _ as exn ->
      let e = Exception.wrap exn in
      Exception.reraise e
    | exn ->
      let e = Exception.wrap exn in
      if consecutive_throws > 500 then (
        Hh_logger.log "Too many consecutive exceptions.";
        Hh_logger.log
          "Probably an uncaught exception rethrown each retry. Exiting. %s"
          (Exception.to_string e);
        HackEventLogger.monitor_giving_up_exception e;
        Exit.exit (Exit_status.Uncaught_exception e)
      );
      Hh_logger.log
        "check_and_run_loop_ threw with exception: %s"
        (Exception.to_string e);
      (env, consecutive_throws + 1)
  in
  let env =
    match env with
    | Error (e, msg) ->
      Hh_logger.log
        "Encountered error in check_and_run_loop: %s, starting new loop"
        msg;
      Ok e
    | Ok e -> Ok e
  in
  check_and_run_loop ~consecutive_throws env monitor_config socket

and check_and_run_loop_
    (env : env msg_update) monitor_config (socket : Unix.file_descr) :
    env msg_update =
  (* WARNING! Don't use the (slow) HackEventLogger here, in the inner loop non-failure path. *)
  (* That's because HackEventLogger for the monitor is synchronous and takes 50ms/call. *)
  (* But the monitor's non-failure inner loop must handle hundres of clients per second *)
  env >>= fun env ->
  let lock_file = monitor_config.lock_file in
  if not (Lock.grab lock_file) then (
    (* e.g. (conjectured) tmpcleaner deletes the .lock file we have a lock on, and another instance of hh_server
       starts up and acquires its own lock on a lockfile by the same name; our "Lock.grab" will return false because
       we're unable to grab a lock on the file currently under pathname "lock_file".
       Or, maybe users just manually deleted the file. *)
    Hh_logger.log "Lost lock; terminating.\n%!";
    HackEventLogger.lock_stolen lock_file;
    Exit.exit Exit_status.Lock_stolen
  );
  let sequence_receipt_high_water_mark =
    match env.server with
    | ServerProcess.Alive process_data ->
      let pid = process_data.ServerProcess.pid in
      MonitorRpc.read_server_receipt_to_monitor_file
        ~server_receipt_to_monitor_file:
          (ServerFiles.server_receipt_to_monitor_file pid)
      |> Option.value ~default:0
    | _ -> 0
  in
  let env = maybe_push_purgatory_clients (Ok env) in
  (* The first sequence number we send is 1; hence, the default "0" will be a no-op *)
  let () = Sent_fds_collector.collect_fds ~sequence_receipt_high_water_mark in
  let has_client = sleep_and_check socket in
  let env = update_status env monitor_config in
  if not has_client then
    (* Note: this call merely reads from disk; it doesn't go via the slow HackEventLogger. *)
    let () = EventLogger.recheck_disk_files () in
    env
  else
    let (fd, _) =
      try Unix.accept socket with
      | exn ->
        let e = Exception.wrap exn in
        HackEventLogger.accepting_on_socket_exception e;
        Hh_logger.log
          "ACCEPTING_ON_SOCKET_EXCEPTION; closing client FD. %s"
          (Exception.to_string e |> Exception.clean_stack);
        Exception.reraise e
    in
    try ack_and_handoff_client env fd with
    | Exit_status.Exit_with _ as exn ->
      let e = Exception.wrap exn in
      Exception.reraise e
    | Unix.Unix_error (Unix.EINVAL, _, _) as exn ->
      let e = Exception.wrap exn in
      Hh_logger.log
        "Ack_and_handoff failure; closing client FD: %s"
        (Exception.get_ctor_string e);
      ensure_fd_closed fd;
      raise Exit_status.(Exit_with Socket_error)
    | exn ->
      let e = Exception.wrap exn in
      let msg =
        Printf.sprintf
          "Ack_and_handoff failure; closing client FD: %s"
          (Exception.get_ctor_string e)
      in
      Hh_logger.log "%s" msg;
      ensure_fd_closed fd;
      env >>= fun env -> Error (env, msg)

let check_and_run_loop_once (env, monitor_config, socket) =
  let env = check_and_run_loop_ (Ok env) monitor_config socket in
  match env with
  | Ok env -> (env, monitor_config, socket)
  | Error (env, msg) ->
    Hh_logger.log "%s" msg;
    (env, monitor_config, socket)

let start_monitor
    ~current_version
    ~waiting_client
    ~max_purgatory_clients
    server_start_options
    informant_init_env
    monitor_config =
  let socket = Socket.init_unix_socket monitor_config.socket_file in
  (* If the client started the server, it opened an FD before forking, so it
   * can be notified when the monitor socket is ready. The FD number was
   * passed in program args. *)
  Option.iter waiting_client ~f:(fun fd ->
      let oc = Unix.out_channel_of_descr fd in
      try
        Out_channel.output_string oc (MonitorUtils.ready ^ "\n");
        Out_channel.close oc
      with
      | (Sys_error _ | Unix.Unix_error _) as e ->
        Printf.eprintf
          "Caught exception while waking client: %s\n%!"
          (Exn.to_string e));

  (* It is essential that we initiate the Informant before the server if we
   * want to give the opportunity for the Informant to truly take
   * ownership over the lifetime of the server.
   *
   * This is because start_server won't actually start a server if it sees
   * a hg update sentinel file indicating an hg update is in-progress.
   * Starting the informant first ensures that its Watchman watch is started
   * before we check for the hgupdate sentinel file - this is required
   * for the informant to properly observe an update is complete without
   * hitting race conditions. *)
  let informant = Informant.init informant_init_env in
  let server_process =
    maybe_start_first_server server_start_options informant
  in
  let env =
    {
      informant;
      max_purgatory_clients;
      current_version;
      purgatory_clients = Queue.create ();
      server = server_process;
      server_start_options;
      retries = 0;
      sql_retries = 0;
      watchman_retries = 0;
      ignore_hh_version = Informant.should_ignore_hh_version informant_init_env;
    }
  in
  (env, monitor_config, socket)

let start_monitoring
    ~current_version
    ~waiting_client
    ~max_purgatory_clients
    server_start_options
    informant_init_env
    monitor_config =
  let (env, monitor_config, socket) =
    start_monitor
      ~current_version
      ~waiting_client
      ~max_purgatory_clients
      server_start_options
      informant_init_env
      monitor_config
  in
  check_and_run_loop (Ok env) monitor_config socket
