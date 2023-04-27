(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The informant collects information to tell the monitor when to
    intelligently kill and restart the server daemon.

    For example: An informant may want to watch the repo state and tell
    the monitor to restart the server when a significant change in
    repo revision occurs since a fresh initialization could be faster
    than an incremental type check. *)

type report =
  | Move_along  (** Nothing to see here. *)
  | Restart_server
      (** Kill the server (if one is running) and start a new one. *)
[@@deriving show]

type server_state =
  | Server_not_yet_started
  | Server_alive
  | Server_dead
[@@deriving show]

type t

type options = {
  root: Path.t;
  allow_subscriptions: bool;
      (** Disable the informant - use the dummy instead. *)
  use_dummy: bool;
      (** Don't trigger a server restart if the distance between two
            revisions we are moving between is less than this. *)
  min_distance_restart: int;
  watchman_debug_logging: bool;
      (** Informant should ignore the hh_version when doing version checks. *)
  ignore_hh_version: bool;
      (** Was the server initialized with a precomputed saved-state? *)
  is_saved_state_precomputed: bool;
}

type init_env = options

val init : init_env -> t

(* Same as init, except it preserves internal Revision_map cache.
 * This is used when server decides to restart itself due to rebase - we don't
 * want Informant to then restart the server again. Reinitializing will discard
 * the pending queue of state changes, and issue new query for base revision,
 * in order to "synchronize" base revision understanding between server and
 * monitor. *)
val reinit : t -> unit

val report : t -> server_state -> report

(*
 * Returns true if the informant is actually running and will
 * manage server lifetime.
 *)
val is_managing : t -> bool

val should_start_first_server : t -> bool

val should_ignore_hh_version : init_env -> bool
