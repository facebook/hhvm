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

open Core

(**
 * Watches a repo and logs state-leave and state-enter
 * events on it using a Watchman subscription.
 *
 * This is useful to ask the question "is this repo mid-update
 * right now?", which isn't provided by the Mercurial or Watchman
 * APIs.
 *)

module J = Hh_json_helpers
module Config = WatchmanEventWatcherConfig

module Responses = struct

  exception Invalid_response
  exception Send_failure

  type t =
    | Unknown
    | Mid_update
    | Settled

  let to_string = function
    | Unknown -> "unknown"
    | Mid_update -> "mid_update"
    | Settled -> "settled"

  let of_string s = match s with
    | "unknown" -> Unknown
    | "mid_update" -> Mid_update
    | "settled" -> Settled
    | _ -> raise Invalid_response

  let send_to_fd v fd =
    let str = Printf.sprintf "%s\n" (to_string v) in
    let bytes = Unix.write fd str 0 (String.length str) in
    if bytes = (String.length str) then
      ()
    else
      raise Send_failure

end


type state =
  | Unknown
  (** State_enter heading towards. *)
  | Entering_to of string
  (** State_leave left at. *)
  | Left_at of string

type env = {
  watchman : Watchman.watchman_instance;
  state : state;
  socket : Unix.file_descr;
}

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

module Args = struct

  type t = {
    root : Path.t;
    daemonize : bool;
    get_sockname : bool;
  }

  let usage = Printf.sprintf
    "Usage: %s [--daemonize] [REPO DIRECTORY]\n"
    Sys.argv.(0)

  let parse () =
    let root = ref None in
    let daemonize = ref false in
    let get_sockname = ref false in
    let options = [
      "--daemonize", Arg.Set daemonize, "spawn watcher daemon";
      "--get-sockname", Arg.Set get_sockname, "print socket and exit";
    ] in
    let () = Arg.parse options (fun s -> root := (Some s)) usage in
    match !root with
    | None ->
      Printf.eprintf "%s" usage;
      exit 1
    | Some root ->
      {
        root = Path.make root;
        daemonize = !daemonize;
        get_sockname = !get_sockname;
      }

  let root args = args.root

end;;


let process_changes changes env =
  let open Watchman in
  match changes with
  | Watchman_unavailable ->
    Hh_logger.log "Watchman unavailable. Exiting";
    exit 1
  | Watchman_pushed (State_enter (name, _)) ->
    Hh_logger.log "State_enter %s" name;
    { env with state = Entering_to name; }
  | Watchman_pushed (State_leave (name, _)) ->
    Hh_logger.log "State_leave %s" name;
    { env with state = Left_at name; }
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
  (match env.state with
  | Unknown ->
    Responses.send_to_fd Responses.Unknown client
  | Entering_to _ ->
    Responses.send_to_fd Responses.Mid_update client
  | Left_at _ ->
    Responses.send_to_fd Responses.Settled client);
  Unix.close client;
  env

let process_client env client =
  try process_client_ env client with
  | Unix.Unix_error (Unix.EBADF, _, _) ->
    env
  | Responses.Send_failure ->
    env

let check_new_connections env =
  let new_clients = get_new_clients env.socket in
  let env = List.fold_left new_clients ~init:env
    ~f:process_client in
  HackEventLogger.processed_clients (List.length new_clients);
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
    Result.Error Failure_daemon_already_running
  end else
  let watchman = init_watchman root in
  match watchman with
  | None ->
    Hh_logger.log "Error failed to initialize watchman";
    Result.Error Failure_watchman_init
  | Some wenv ->
    let socket = Socket.init_unix_socket (Config.socket_file root) in
    Hh_logger.log "initialized and listening on %s"
      (Config.socket_file root);
    Result.Ok {
      watchman = Watchman.Watchman_alive wenv;
      state = Unknown;
      socket;
    }

let to_channel_no_exn oc data =
  try Daemon.to_channel oc ~flush:true data with
  | e ->
    Hh_logger.exc ~prefix:"Warning: writing to channel failed" e

let main args =
  let result = init args.Args.root in
  match result with
  | Result.Ok env -> begin
      try serve env with
      | e ->
        let () = Hh_logger.exc
          ~prefix:"WatchmanEventWatcheer uncaught exception. exiting." e in
        raise e
    end
  | Result.Error Failure_daemon_already_running
  | Result.Error Failure_watchman_init ->
    exit 1

let log_file root =
  let log_link = WatchmanEventWatcherConfig.log_link root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  Sys_utils.make_link_of_timestamped log_link


let daemon_main_ args oc =
  match init args.Args.root with
  | Result.Ok env ->
    to_channel_no_exn oc Init_success;
    serve env
  | Result.Error Failure_watchman_init ->
    to_channel_no_exn oc (Init_failure Failure_watchman_init);
    Hh_logger.log "Watchman init failed. Exiting.";
    HackEventLogger.init_watchman_failed ();
    exit 1;
  | Result.Error Failure_daemon_already_running ->
    Hh_logger.log "Daemon already running. Exiting.";
    exit 1

let daemon_main args (_ic, oc) =
  try daemon_main_ args oc with
  | e ->
    HackEventLogger.uncaught_exception e

(** Typechecker canont infer this type since the input channel
 * is never used so its phantom type is never ineferred. We annotate
 * the type manually to "unit" here to help it out. *)
let daemon_entry : (Args.t, unit, daemon_init_result) Daemon.entry =
  Daemon.register_entry_point
  "Watchman_event_watcher_daemon_main"
  daemon_main

let spawn_daemon args =
  let log_file_path = log_file args.Args.root in
  let in_fd = Daemon.null_fd () in
  let out_fd = Daemon.fd_of_path log_file_path in
  Printf.eprintf
    "Spawning daemon. Its logs will go to: %s\n%!" log_file_path;
  let {Daemon.channels; _; } =
    Daemon.spawn (in_fd, out_fd, out_fd) daemon_entry args in
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

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  if args.Args.get_sockname then
    Printf.printf "%s%!" (Config.socket_file args.Args.root)
  else if args.Args.daemonize then
    spawn_daemon args
  else
    main args
