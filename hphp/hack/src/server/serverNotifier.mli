(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type changes =
  | Unavailable
      (** e.g. because DFind is not available, or watchman subscription is down *)
  | SyncChanges of SSet.t
      (** contains all changes up to the point that the notifier was invoked *)
  | AsyncChanges of SSet.t
      (** contains whatever changes have been pushed up to this moment *)
  | StateEnter of string * Hh_json.json option
  | StateLeave of string * Hh_json.json option

type t

(** This takes a filter, and returns all files under root that match *)
type indexer = (string -> bool) -> unit -> string list

val init :
  ServerArgs.options -> ServerLocalConfig.t -> num_workers:int -> t * indexer

val init_null : unit -> t

val init_mock :
  get_changes_async:(unit -> changes) -> get_changes_sync:(unit -> changes) -> t

val wait_until_ready : t -> unit

(** This might return AsyncChanges the ones that we happen to have received by now,
or SyncChanges, depending on the underlying notifier's state *)
val get_changes_async : t -> changes * Watchman.clock option

(** This will always return SyncChanges, all changes up to the point this was invoked. *)
val get_changes_sync : t -> changes * Watchman.clock option

val async_reader_opt : t -> Buffered_line_reader.t option
