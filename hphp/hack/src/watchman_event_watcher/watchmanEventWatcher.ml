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

(**
 * Watches a repo and logs state-leave and state-enter
 * events on it using a Watchman subscription.
 *
 * This is useful to ask the question "is this repo mid-update
 * right now?", which isn't provided by the Mercurial or Watchman
 * APIs.
 *)

module J = Hh_json_helpers

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
  }

  let usage = Printf.sprintf
    "Usage: %s [--daemonize] [REPO DIRECTORY]\n"
    Sys.argv.(0)

  let parse () =
    let root = ref None in
    let daemonize = ref false in
    let options = [
      "--daemonize", Arg.Set daemonize, "spawn watcher daemon";
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
      }

  let root args = args.root

end;;


let process_changes changes =
  let open Watchman in
  match changes with
  | Watchman_unavailable ->
    Hh_logger.log "Watchman unavailable. Exiting";
    exit 1
  | Watchman_pushed (State_enter (name, _)) ->
    Hh_logger.log "State_enter %s" name
  | Watchman_pushed (State_leave (name, _)) ->
    Hh_logger.log "State_leave %s" name
  | Watchman_pushed (Files_changed set) ->
    let files = SSet.fold (fun x acc ->
      let exists = Sys.file_exists x in
      acc ^ (Printf.sprintf "%s: %b" x exists) ^ "; ") set "" in
    Hh_logger.log "Changes: %s" files
  | Watchman_synchronous _ ->
    Hh_logger.log "Watchman unexpectd synchronous response. Exiting";
    exit 1

let rec check_subscription w =
  let deadline = Unix.time () +. 1000.0 in
  let w, changes = Watchman.get_changes ~deadline w in
  process_changes changes;
  check_subscription w

let init root =
  let init_id = Random_id.short_string () in
  HackEventLogger.init_watchman_event_watcher root init_id;
  let lock_file = WatchmanEventWatcherConfig.lock root in
  if not (Lock.grab lock_file) then begin
    Hh_logger.log "Can't grab lock; terminating.\n%!";
    HackEventLogger.lock_stolen lock_file;
    Result.Error Failure_daemon_already_running
  end else
  let watchman = Watchman.init {
    Watchman.subscribe_mode = Some Watchman.All_changes;
    init_timeout = 30;
    sync_directory = "";
    expression_terms = watchman_expression_terms;
    root;
  } in
  match watchman with
  | None ->
    Hh_logger.log "Error failed to initialize watchman";
    Result.Error Failure_watchman_init
  | Some wenv ->
    Hh_logger.log "initialized";
    Result.Ok wenv

let to_channel_no_exn oc data =
  try Daemon.to_channel oc ~flush:true data with
  | e ->
    Hh_logger.exc ~prefix:"Warning: writing to channel failed" e

let main args =
  let result = init args.Args.root in
  match result with
  | Result.Ok wenv ->
    check_subscription (Watchman.Watchman_alive wenv)
  | Result.Error Failure_daemon_already_running
  | Result.Error Failure_watchman_init ->
    exit 1

let log_file root =
  let log_link = WatchmanEventWatcherConfig.log_link root in
  (try Sys.rename log_link (log_link ^ ".old") with _ -> ());
  Sys_utils.make_link_of_timestamped log_link

let daemon_main args (_ic, oc) =
  match init args.Args.root with
  | Result.Ok wenv ->
    to_channel_no_exn oc Init_success;
    check_subscription (Watchman.Watchman_alive wenv)
  | Result.Error e ->
   to_channel_no_exn oc (Init_failure e)

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
  let { Daemon.channels; _ } =
    Daemon.spawn (in_fd, out_fd, out_fd) daemon_entry args in
  let result = Daemon.from_channel (fst channels) in
  match result with
  | Init_success ->
    ()
  | Init_failure Failure_daemon_already_running ->
    Printf.eprintf "Daemon failed to spawn - one already running\n%!";
    exit 1
  | Init_failure Failure_watchman_init ->
    Printf.eprintf "Daemon failed to spawn - watchman failure\n%!";
    exit 1

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  if args.Args.daemonize then
    spawn_daemon args
  else
    main args
