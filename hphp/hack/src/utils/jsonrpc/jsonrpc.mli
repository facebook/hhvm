(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type writer = Hh_json.json -> unit

type timestamped_json = {
  json: Hh_json.json;
  timestamp: float;
}

type t

(** must call Daemon.entry_point at start of your main *)
val make_t : unit -> t

(** says whether there's already an item on the queue, or stdin is readable meaning that there's something pending on it *)
val has_message : t -> bool

(** similar to [has_message], but can be used to power a 'select' syscall
which will fire when a message is available. *)
val await_until_message :
  t -> [> `Already_has_message | `Wait_for_data_here of Unix.file_descr ]

(** says whether the things we've already enqueued from stdin contain a message that matches the predicate *)
val find_already_queued_message :
  f:(timestamped_json -> bool) -> t -> timestamped_json option

(** This awaits until a message is found which satisfies the predicate,
and returns it as [Some].
If the message was already in the queue at the time this function was called,
then the returned promise will be already-completed.
If [cancellation_token] is fired before a message is found, then
the returned promise will get resolved with [None].
If there's a fault with the incoming file-descriptor like EOF, the returned promise
also gets resolved to [None]. *)
val await_until_found :
  t ->
  predicate:(timestamped_json -> bool) ->
  cancellation_token:unit Lwt.t ->
  timestamped_json option Lwt.t

val get_message :
  t ->
  [> `Message of timestamped_json
  | `Fatal_exception of Marshal_tools.remote_exception_data
  | `Recoverable_exception of Marshal_tools.remote_exception_data
  ]
  Lwt.t

val get_next_request_id : unit -> int
