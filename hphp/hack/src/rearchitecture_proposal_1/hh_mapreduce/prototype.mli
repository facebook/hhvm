(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val run : unit -> unit
(** This is the main entrypoint when user invokes "hh_mapreduce prototype <args> " *)

(** Errors that might occur during RPC communication with prototype *)
type rpc_error =
  | Disconnected
      (** Either the prototype's socket isn't open to start with, or the prototype socket got closed while trying to read/write to it *)
  | Malformed of string
      (** The packet we received wasn't a valid format, maybe because the other party is a wrong version *)

val rpc_error_to_verbose_string : rpc_error -> string
(** Turns an rpc_error into a detailed string suitable for debugging, maybe including stack trace *)

val rpc_write : Unix.file_descr -> 'a -> (unit, rpc_error) result
(** Synchronously send over the orchestrator<->worker fd. *)

val rpc_read : Unix.file_descr -> ('a, rpc_error) result
(** Synchronously receive over the orchestrator<->worker fd. *)

val rpc_close_no_err : Unix.file_descr -> unit
(** Synchronously requests gently shutdown, then closes the fd. *)

val rpc_request_new_worker :
  string -> Dispatch.kind -> (Unix.file_descr, rpc_error) result
(** Orchestrator uses this to synchronously request a new worker. It sends a message to the
    prototype requesting it to fork a new process which will then invoke the
    appropriate Dispatch.kind run_worker method. *)
