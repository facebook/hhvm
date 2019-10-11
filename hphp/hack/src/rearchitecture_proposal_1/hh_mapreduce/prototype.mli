(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Opaque type; underneath it's the same as a regular Unix.file_descr *)
type file_descr

(** Constructs a prototype_file_descr, suitable for rpc_read and rpc_write APIs *)
val file_descr : Unix.file_descr -> file_descr

(** This is the main entrypoint when user invokes "hh_mapreduce prototype <args> " *)
val run : unit -> unit

(** Errors that might occur during RPC communication with prototype *)
type rpc_error =
  | Absent of Exception.t  (** Prototype's socket isn't open to start with *)
  | Disconnected of Exception.t
      (** Prototype socket got closed while trying to read/write to it *)
  | Malformed of string * Utils.callstack
      (** The packet we received wasn't a valid format, maybe because the other party is a wrong version *)
  | Panic of Marshal_tools.remote_exception_data
      (** Worker had an unhandled exception *)

(** Turns an rpc_error into a detailed string suitable for debugging, maybe including stack trace *)
val rpc_error_to_verbose_string : rpc_error -> string

(** Synchronously send over the orchestrator<->worker fd. *)
val rpc_write : file_descr -> 'a -> (unit, rpc_error) result

(** Synchronously receive over the orchestrator<->worker fd. *)
val rpc_read : file_descr -> ('a, rpc_error) result

(** Synchronously requests gently shutdown, then closes the fd. *)
val rpc_close_no_err : file_descr -> unit

(** Orchestrator uses this to synchronously request a new worker. It sends a
message message to the prototype requesting it to fork a new process which will
then invoke the appropriate Dispatch.kind run_worker method. *)
val rpc_request_new_worker :
  string -> Dispatch.kind -> (file_descr, rpc_error) result
