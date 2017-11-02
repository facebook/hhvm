(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(***************************************************
           ("Who watches the Watchmen?")
****************************************************)

open Hh_core

(**
 * Watches a repo and logs state-leave and state-enter
 * events on it using a Watchman subscription.
 *
 * This is useful to ask the question "is this repo mid-update
 * right now?", which isn't provided by the Mercurial or Watchman
 * APIs.
 *
 * Socket can be retrieved by invoking this command with --get-sockname
 *
 * Clients get the status by connecting to the socket and reading
 * newline-terminated messages. The message can be:
 *   'unknown' - watcher doesn't know what state the repo is in
 *   'mid_update' - repo is currently undergoing an update
 *   'settled' - repo is settled (not undergoing an update)
 *
 * After sending 'settled' message, the socket is closed.
 * An 'unknown' or 'mid_updat' message is eventually followed by a
 * 'settled' message when the repo is settled.
 *
 * The Hack Monitor uses the watcher when first starting up to delay
 * initialization while a repo is in the middle of an update. Initialization
 * mid-update is undesirable because the wrong saved state will be loaded
 * (the one corresponding to the revision we are leaving) resulting in
 * a slow recheck after init.
 *
 * Why do we need a socket with a 'settled' notification instead of just
 * having the Hack Server tail the watcher's logs? Consider the following
 * race:
   *
   * Watcher prints 'mid_update' to logs, Watchman has sent State_leave
   * event to subscribers before the Hack Monitor's subscription started,
   * and the Watcher crashes before processing this State_leave.
   *
 *
 * If the Hack Monitor just tailed the watcher logs to see the state of the
 * repo, it would not start a server, would never get the State_leave watchman
 * event on its subscription, and would just wait indefinitely.
 *
 * The watcher must also be used as the source-of-truth for the repo's
 * state during startup instead of the Hack Monitor's own subscription.
 * Consider the following incorrect usage of the Watcher:
   *
   * Hack Monitor starts up, starts a Watchman subscription, checks watcher
   * for repo state and sees 'mid_update', delays starting a server.
   * Waits on its Watchman subscription for a State_leave before starting a
   * server.
   *
 *
 * The Monitor could potentially wait forever in this scenario due to a
 * race condition - Watchman has already sent the State_leave to all
 * subscriptions before the Monitor's subscription was started, but it
 * was not yet processed by the Watcher.
 *
 * The Hack Monitor should instead be waiting for a 'settled' from the
 * Watcher.
 *)

module J = Hh_json_helpers
module Config = WatchmanEventWatcherConfig
module Responses = Config.Responses

type state =
  | Unknown
  (** State_enter heading towards. *)
  | Entering_to of string
  (** State_leave left at. *)
  | Left_at of string

type env = {
  watchman : Watchman.watchman_instance;
  (** Directory we are watching. *)
  root : Path.t;
  state : state;
  socket : Unix.file_descr;
  waiting_clients : Unix.file_descr Queue.t;
}

let ignore_unix_epipe f x =
  try f x with
  | Unix.Unix_error (Unix.EPIPE, _, _) ->
    ()

(**
 * This allows the repo itself to turn on-or-off this Watchman Event
 * Watcher service (and send a fake Settled message instead).
 *
 * This service has its own deploy/release cycle independent of the
 * Hack Server. Adding this toggle to the Hack Server wouldn't retroactively
 * allow us to toggle off this service for older versions of the Hack Server.
 * Putting it here instead allows us to turn off this service regardless of
 * what version of the Hack server is running.
 *)
let is_enabled env =
  let enabled_file = Path.concat env.root ".hh_enable_watchman_event_watcher" in
  Sys.file_exists (Path.to_string enabled_file)

let send_to_fd env v fd =
  let v = if is_enabled env then
    v
  else
    (Hh_logger.log "Service not enabled. Sending fake Settled message instead";
    Responses.Settled)
  in
  let str = Printf.sprintf "%s\n" (Responses.to_string v) in
  let bytes = Unix.single_write fd str 0 (String.length str) in
  (if bytes = (String.length str) then
    ()
  else
    (** We're only writing a few bytes. Not sure what to do here.
     * Retry later when the pipe has been pumped? *)
    Hh_logger.log "Failed to write all bytes");
  match v with
  | Responses.Settled ->
    Unix.close fd
  | Responses.Unknown | Responses.Mid_update ->
    Queue.add fd env.waiting_clients

let watchman_expression_terms = [
  J.strlist ["type"; "f"];
  J.strlist ["name"; "updatestate";];
]

type init_failure =
  | Failure_daemon_already_running
  | Failure_watchman_init

type daemon_init_result =
  | Init_failure of init_failure
  | Init_success

let process_changes changes env =
  let notify_client client =
    (** Notify the client that the repo has settled. *)
      ignore_unix_epipe (send_to_fd env Responses.Settled) client
  in
  let notify_waiting_clients env =
    let clients = Queue.create () in
    let () = Queue.transfer env.waiting_clients clients in
    Queue.fold (fun () client -> notify_client client) () clients
  in
  let open Watchman in
  match changes with
  | Watchman_unavailable ->
    Hh_logger.log "Watchman unavailable. Exiting";
    exit 1
  | Watchman_pushed (Changed_merge_base (mergebase, changes)) ->
    Hh_logger.log "changed mergebase: %s" mergebase;
    let changes = String.concat "\n" (SSet.elements changes) in
    Hh_logger.log "changes: %s" changes;
    let env = { env with state = Left_at mergebase; } in
    let () = notify_waiting_clients env in
    env
  | Watchman_pushed (State_enter (name, json)) ->
    Hh_logger.log "State_enter %s" name;
    let (>>=) = Option.(>>=) in
    let (>>|) = Option.(>>|) in
    ignore (json >>= Watchman_utils.rev_in_state_change >>| fun hg_rev -> begin
      Hh_logger.log "Revision: %s" hg_rev
    end);
    { env with state = Entering_to name; }
  | Watchman_pushed (State_leave (name, json)) ->
    Hh_logger.log "State_leave %s" name;
    let (>>=) = Option.(>>=) in
    let (>>|) = Option.(>>|) in
    ignore (json >>= Watchman_utils.rev_in_state_change >>| fun hg_rev -> begin
      Hh_logger.log "Revision: %s" hg_rev
    end);
    let env = { env with state = Left_at name; } in
    let () = notify_waiting_clients env in
    env
  | Watchman_pushed (Files_changed set) when (SSet.is_empty set)->
    env
  | Watchman_pushed (Files_changed set) ->
    let files = SSet.fold (fun x acc ->
      let exists = Sys.file_exists x in
      acc ^ (Printf.sprintf "%s: %b" x exists) ^ "; ") set "" in
    Hh_logger.log "Changes: %s" files;
    env
  | Watchman_synchronous _ ->
    Hh_logger.log "Watchman unexpectd synchronous response. Exiting";
    exit 1

let check_subscription env =
  let w, changes = Watchman.get_changes env.watchman in
  let env = { env with watchman = w; } in
  let env = process_changes changes env in
  env

let sleep_and_check ?(wait_time=0.3) socket =
  let ready_socket_l, _, _ = Unix.select [socket] [] [] wait_time in
  ready_socket_l <> []

(** Batch accept all new client connections. Return the list of them. *)
let get_new_clients socket =
  let rec get_all_clients_nonblocking socket acc =
    let has_client = sleep_and_check ~wait_time:0.0 socket in
    if not has_client then
      acc
    else
      let acc = try
        let fd, _ = Unix.accept socket in
        fd :: acc
      with
        | Unix.Unix_error _ ->
          acc
      in
      get_all_clients_nonblocking socket acc
  in
  let has_client = sleep_and_check socket in
  if not has_client then
    []
  else
    get_all_clients_nonblocking socket []

let process_client_ env client =
  (** We allow this to throw Unix_error - this lets us ignore broken
   * clients instead of adding them to the waiting_clients queue. *)
  (match env.state with
  | Unknown ->
    send_to_fd env Responses.Unknown client
  | Entering_to _ ->
    send_to_fd env Responses.Mid_update client
  | Left_at _ ->
    send_to_fd env Responses.Settled client);
  env

let process_client env client =
  try process_client_ env client with
  | Unix.Unix_error _ ->
    env

let check_new_connections env =
  let new_clients = get_new_clients env.socket in
  let env = List.fold_left new_clients ~init:env
    ~f:process_client in
  let count = List.length new_clients in
  if count > 0 then HackEventLogger.processed_clients count;
  env

let rec serve env =
  let env = check_subscription env in
  let env = check_new_connections env in
  serve env

let init_watchman root =
  Watchman.init {
    Watchman.subscribe_mode = Some Watchman.All_changes;
    init_timeout = 30;
    sync_directory = "";
    expression_terms = watchman_expression_terms;
    root;
  }

let init root =
  let init_id = Random_id.short_string () in
  HackEventLogger.init_watchman_event_watcher root init_id;
  let lock_file = WatchmanEventWatcherConfig.lock root in
  if not (Lock.grab lock_file) then begin
    Hh_logger.log "Can't grab lock; terminating.\n%!";
    HackEventLogger.lock_stolen lock_file;
    Error Failure_daemon_already_running
  end else
  let watchman = init_watchman root in
  match watchman with
  | None ->
    Hh_logger.log "Error failed to initialize watchman";
    Error Failure_watchman_init
  | Some wenv ->
    let socket = Socket.init_unix_socket (Config.socket_file root) in
    Hh_logger.log "initialized and listening on %s"
      (Config.socket_file root);
    Ok {
      watchman = Watchman.Watchman_alive wenv;
      state = Unknown;
      socket;
      root;
      waiting_clients = Queue.create ();
    }

let to_channel_no_exn oc data =
  try Daemon.to_channel oc ~flush:true data with
  | e ->
    Hh_logger.exc ~prefix:"Warning: writing to channel failed" e

let main root =
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  let result = init root in
  match result with
  | Ok env -> begin
      try serve env with
      | e ->
        let () = Hh_logger.exc
          ~prefix:"WatchmanEventWatcheer uncaught exception. exiting." e in
        raise e
    end
  | Error Failure_daemon_already_running
  | Error Failure_watchman_init ->
    exit 1

let log_file root =
  let log_link = WatchmanEventWatcherConfig.log_link root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  Sys_utils.make_link_of_timestamped log_link


let daemon_main_ root oc =
  match init root with
  | Ok env ->
    to_channel_no_exn oc Init_success;
    serve env
  | Error Failure_watchman_init ->
    to_channel_no_exn oc (Init_failure Failure_watchman_init);
    Hh_logger.log "Watchman init failed. Exiting.";
    HackEventLogger.init_watchman_failed ();
    exit 1;
  | Error Failure_daemon_already_running ->
    Hh_logger.log "Daemon already running. Exiting.";
    exit 1

let daemon_main root (_ic, oc) =
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;
  try daemon_main_ root oc with
  | e ->
    HackEventLogger.uncaught_exception e

(** Typechecker canont infer this type since the input channel
 * is never used so its phantom type is never ineferred. We annotate
 * the type manually to "unit" here to help it out. *)
let daemon_entry : (Path.t, unit, daemon_init_result) Daemon.entry =
  Daemon.register_entry_point
  "Watchman_event_watcher_daemon_main"
  daemon_main

let spawn_daemon root =
  let log_file_path = log_file root in
  let in_fd = Daemon.null_fd () in
  let out_fd = Daemon.fd_of_path log_file_path in
  Printf.eprintf
    "Spawning daemon. Its logs will go to: %s\n%!" log_file_path;
  let {Daemon.channels; _; } =
    Daemon.spawn (in_fd, out_fd, out_fd) daemon_entry root in
  let result = Daemon.from_channel (fst channels) in
  match result with
  | Init_success ->
    ()
  | Init_failure Failure_daemon_already_running ->
    Printf.eprintf "No need to spawn daemon. One already running";
    exit 0
  | Init_failure Failure_watchman_init ->
    Printf.eprintf "Daemon failed to spawn - watchman failure\n%!";
    exit 1
