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

type queue

(** must call Daemon.entry_point at start of your main *)
val make_queue : unit -> queue

val get_read_fd : queue -> Unix.file_descr (* can be used for 'select' *)

(** says whether there's already an item on the queue, or stdin is readable meaning that there's something pending on it *)
val has_message : queue -> bool

(** says whether the things we've already enqueued from stdin contain a message that matches the predicate *)
val find_already_queued_message :
  f:(timestamped_json -> bool) -> queue -> timestamped_json option

val get_message :
  queue ->
  [> `Message of timestamped_json
  | `Fatal_exception of Marshal_tools.remote_exception_data
  | `Recoverable_exception of Marshal_tools.remote_exception_data
  ]
  Lwt.t

val get_next_request_id : unit -> int
