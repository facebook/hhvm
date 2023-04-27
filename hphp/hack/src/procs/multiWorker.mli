(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Hh_prelude

(* The protocol for a next function is to return a list of elements.
 * It will be called repeatedly until it returns an empty list.
 *)

module type CALLER = sig
  type 'a result

  val return : 'a -> 'a result

  val multi_threaded_call :
    WorkerController.worker list ->
    (WorkerController.worker_id * 'c -> 'a -> 'b) ->
    (WorkerController.worker_id * 'b -> 'c -> 'c) ->
    'c ->
    'a Hh_bucket.next ->
    'c result
end

module CallFunctor (Caller : CALLER) : sig
  val call :
    WorkerController.worker list option ->
    job:(WorkerController.worker_id * 'c -> 'a -> 'b) ->
    merge:(WorkerController.worker_id * 'b -> 'c -> 'c) ->
    neutral:'c ->
    next:'a Hh_bucket.next ->
    'c Caller.result
end

type worker

(* List of file descriptors that became ready (and triggered interruption),
 * returns whether current job should be cancelled *)
type 'a interrupt_config = 'a MultiThreadedCall.interrupt_config

val next :
  ?progress_fn:(total:int -> start:int -> length:int -> unit) ->
  ?max_size:int ->
  worker list option ->
  'a list ->
  'a list Hh_bucket.next

(** Can raise MultiThreadedCall.Coalesced_failures unless in single-threaded mode. *)
val call :
  worker list option ->
  job:('acc -> 'input -> 'output) ->
  merge:('output -> 'acc -> 'acc) ->
  neutral:'acc ->
  next:'input Hh_bucket.next ->
  'acc

module type WorkItems_sig = sig
  type t

  type workitem

  val of_workitem : workitem -> t

  val pop : t -> workitem option * t

  val push : workitem -> t -> t
end

(** [job] can return more items to process, e.g. the
  part of the input that it could not process. *)
val call_stateless :
  (module WorkItems_sig with type workitem = 'input and type t = 'inputs) ->
  worker list ->
  job:('acc -> 'input -> 'input * 'output) ->
  merge:('output -> 'acc -> 'acc) ->
  neutral:'acc ->
  inputs:'inputs ->
  'acc

type call_wrapper = {
  f:
    'a 'b 'c.
    worker list option ->
    job:('c -> 'a -> 'b) ->
    merge:('b -> 'c -> 'c) ->
    neutral:'c ->
    next:'a Hh_bucket.next ->
    'c;
}

val wrapper : call_wrapper

(* Can raise MultiThreadedCall.Coalesced_failures unless in single-threaded mode. *)
val call_with_worker_id :
  worker list option ->
  job:(WorkerController.worker_id * 'c -> 'a -> 'b) ->
  merge:(WorkerController.worker_id * 'b -> 'c -> 'c) ->
  neutral:'c ->
  next:'a Hh_bucket.next ->
  'c

(** The last element returned, a list of job inputs, are the job inputs which have not been
    processed fully or at all due to interrupts. *)
val call_with_interrupt :
  ?on_cancelled:
    ((* [on_cancelled] should be specified if your [next] function ever returns
        [Hh_bucket.Wait], and it should return the list of all jobs that haven't
        finished or started yet. *)
     unit ->
    'a list) ->
  worker list option ->
  job:('c -> 'a -> 'b) ->
  merge:('b -> 'c -> 'c) ->
  neutral:'c ->
  next:'a Hh_bucket.next ->
  interrupt:'d interrupt_config ->
  'c * 'd * ('a list * MultiThreadedCall.cancel_reason) option

(* Creates a pool of workers. *)
val make :
  ?call_wrapper:
    (* See docs in WorkerController.worker for call_wrapper. *)
    WorkerController.call_wrapper ->
  longlived_workers:bool ->
  saved_state:'a ->
  entry:'a WorkerController.entry ->
  int ->
  gc_control:Gc.control ->
  heap_handle:SharedMem.handle ->
  worker list
