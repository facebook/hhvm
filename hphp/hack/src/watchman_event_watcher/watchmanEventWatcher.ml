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

module Args = struct

  type t = {
    root : Path.t;
  }

  let usage = Printf.sprintf
    "Usage: %s [REPO DIRECTORY]\n"
    Sys.argv.(0)

  let parse () =
    let root = ref None in
    let () = Arg.parse [] (fun s -> root := (Some s)) usage in
    match !root with
    | None ->
      Printf.eprintf "%s" usage;
      exit 1
    | Some root ->
      { root = Path.make root; }

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

let () =
  let args = Args.parse () in
  HackEventLogger.init_watchman_event_watcher args.Args.root;
  let watchman = Watchman.init {
    Watchman.subscribe_mode = Some Watchman.All_changes;
    init_timeout = 30;
    sync_directory = "";
    expression_terms = watchman_expression_terms;
    root = args.Args.root;
  } in
  match watchman with
  | None ->
    Hh_logger.log "Error failed to initialize watchman"
  | Some wenv ->
    check_subscription (Watchman.Watchman_alive wenv)
