(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** If a worker process fails, this is raised.
 *
 * Note: When one worker process fails, the remaining in-progress workers are checked
 * for completion/failure, and all their failures (non-zero exit code) are coalesced
 * together into one of these exceptions.
 *
 * No further buckets are distributed to workers.
 *
 * Still-in-progress workers are left to their own accord. *)
exception Coalesced_failures of WorkerController.worker_failure list

val coalesced_failures_to_string :
  WorkerController.worker_failure list -> string

(** If an interrupt handler wants the typecheck to be cancelled, it must
always give a reason. *)
type cancel_reason = {
  user_message: string;
      (** This string followed by "\nPlease re-run hh" will be printed to stdout by clientCheckStatus.ml,
      in the event that the typecheck got cancelled. *)
  log_message: string;
      (** This goes only to logs. The logs will have both [user_message] and [log_message].
      A typical use of log_message is a callstack or exception message. *)
  timestamp: float;
      (** This goes only to logs. We might decide to write the timestamp at which
    the interrupt was generated, or at which it was handled. *)
}

(** An interrupt is set up as a pair [Unix.file_descr * 'env interrupt_handler].
Our interrupts are set up in serverMain.ml...
* The file-descr for our watchman subscription and a handler which processes the watchman event;
* The file-descr for our persistent connection and a handler which processes the RPC;
* The file-descr for our "priority channel" i.e. new hh_client connections and a handler for them.

For instance the watchman handler might determine that a .php file changed on disk, in
which cas it returns [Cancel] and MultiThreadCall stops itself and returns all unfinished
workitems back to its caller; or might determine that no material disk changes
happened in which case it returns [Continue] and MultiThreadedCall will continue. *)
type interrupt_result =
  | Cancel of cancel_reason
  | Continue

type 'env interrupt_handler = 'env -> 'env * interrupt_result

type 'env interrupt_config = {
  env: 'env;
  handlers: 'env -> (Unix.file_descr * 'env interrupt_handler) list;
}

type worker_id = int

val no_interrupt : 'a -> 'a interrupt_config

(** Can raise Coalesced_failures exception. *)
val call :
  WorkerController.worker list ->
  ('c -> 'a -> 'b) ->
  ('b -> 'c -> 'c) ->
  'c ->
  'a Bucket.next ->
  'c

(** Invokes merge with a unique worker id.
    Can raise Coalesced_failures exception. *)
val call_with_worker_id :
  WorkerController.worker list ->
  (worker_id * 'c -> 'a -> 'b) ->
  (worker_id * 'b -> 'c -> 'c) ->
  'c ->
  'a Bucket.next ->
  'c

(** The last element returned, a list of job inputs, are the job inputs which have not been
    processed fully or at all due to interrupts. *)
val call_with_interrupt :
  WorkerController.worker list ->
  ('c -> 'a -> 'b) ->
  ('b -> 'c -> 'c) ->
  'c ->
  'a Bucket.next ->
  ?on_cancelled:
    ((* [on_cancelled] should be specified if your [next] function ever returns
        [Bucket.Wait], and it should return the list of all jobs that haven't
        finished or started yet. *)
     unit ->
    'a list) ->
  'd interrupt_config ->
  'c * 'd * ('a list * cancel_reason) option

val on_exception : (Exception.t -> unit) -> unit
