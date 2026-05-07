(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type settings = {
  root: Path.t;
  watch_spec: FilesToIgnore.watch_spec;
  debug_logging: bool;
  timeout_secs: int;  (** Timeout, in seconds *)
  throttle_time_ms: int;
      (** Value of throttle_time_ms parameter passed to Eden's stream_changes_since API.
          This means that this is the minimum period (in milliseconds) between each time
          Eden will send us a change notification. *)
  report_telemetry: bool;
  state_tracking: bool;
  sync_queries_obey_deferral: bool;
  tracked_states: string list;
      (** List of state names to track for deferring file changes.
          Only used when state_tracking is enabled. *)
}

type changes =
  | FileChanges of {
      files: string list;  (** List is not guaranteed to be deduplicated *)
      translated_at: float;
          (** Unix timestamp, same format as [Unix.gettimeofday] *)
    }
  | CommitTransition of {
      from_commit: string;
      to_commit: string;
      file_changes: string list;
          (** List is not guaranteed to be deduplicated *)
      translated_at: float;
          (** Unix timestamp, same format as [Unix.gettimeofday] *)
    }
  | StateEnter of string
  | StateLeave of string
[@@deriving show]

type edenfs_watcher_error =
  | EdenfsWatcherError of string
  | LostChanges of string
      (** There may have been some changes that Eden has lost track of.
        (e.g., due to Eden restarting).
        The string is just a description of what happened. *)
  | NonEdenWWW  (** The root directory is not inside an EdenFS mount. *)
[@@deriving show]

module type Edenfs_watcher_sig = sig
  type clock

  val pp_clock : Format.formatter -> clock -> unit

  val show_clock : clock -> string

  val equal_clock : clock -> clock -> bool

  type edenfs_watcher_error

  val pp_edenfs_watcher_error : Format.formatter -> edenfs_watcher_error -> unit

  val show_edenfs_watcher_error : edenfs_watcher_error -> string

  type changes

  val pp_changes : Format.formatter -> changes -> unit

  val show_changes : changes -> string

  type instance

  val init : settings -> (instance * clock, edenfs_watcher_error) result

  val get_changes_sync :
    instance ->
    (changes list * clock * Telemetry.t option, edenfs_watcher_error) result

  val get_changes_async :
    instance ->
    (changes list * clock * Telemetry.t option, edenfs_watcher_error) result

  val get_notification_fd :
    instance -> (Caml_unix.file_descr, edenfs_watcher_error) result

  val get_all_files :
    instance -> (string list * Telemetry.t option, edenfs_watcher_error) result

  module Standalone : sig
    val get_changes_since :
      settings ->
      clock ->
      (changes list * clock * Telemetry.t option, edenfs_watcher_error) result
  end

  module Mocking : sig
    val init_returns : (instance * clock, edenfs_watcher_error) result -> unit

    val get_changes_sync_returns :
      (changes list * clock * Telemetry.t option, edenfs_watcher_error) result ->
      unit
  end
end
