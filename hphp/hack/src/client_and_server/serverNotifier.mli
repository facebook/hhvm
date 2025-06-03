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

type clock = ServerNotifierTypes.clock = Watchman of Watchman.clock
[@@deriving show, eq]

val watchman_clock_of_clock : clock -> Watchman.clock

type t

(** This takes a filter, and returns all files under root that match *)
type indexer = (string -> bool) -> unit -> string list

val init :
  ServerArgs.options -> ServerLocalConfig.t -> num_workers:int -> t * indexer

val init_null : unit -> t

val init_mock :
  get_changes_async:(unit -> changes) -> get_changes_sync:(unit -> SSet.t) -> t

val wait_until_ready : t -> unit

(** This might return AsyncChanges the ones that we happen to have received by now,
or SyncChanges, depending on the underlying notifier's state *)
val get_changes_async : t -> changes * clock option

(** Returns all changes up to the point this was invoked, represented as a set
of changed files. This will raise an exception if the file watching service
is unavailable *)
val get_changes_sync : t -> SSet.t * clock option

(** If supported by the backend, returns a file descriptor that can be used to
    be notified about file system changes. The FD being read-ready is a
    necessary condition for a subsequent call to `get_changes_{sync,async}`
    returning any changes. Note that this FD must only be used to
    `poll`/`select` on, but never actually read from. *)
val notification_fd : t -> Caml_unix.file_descr option

(** If supported by the backend, returns a value acting as a necessary condition
    for a subsequent call to `get_changes_{sync,async}` returning any changes. *)
val maybe_changes_available : t -> bool option
