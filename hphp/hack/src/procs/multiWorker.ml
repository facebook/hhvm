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

(* Hide the worker type from our users *)
type worker = WorkerController.worker

type 'a interrupt_config = 'a MultiThreadedCall.interrupt_config

let single_threaded_call_with_worker_id job merge neutral next =
  let x = ref (next ()) in
  let acc = ref neutral in
  (* This is a just a sanity check that the job is serializable and so
   * that the same code will work both in single threaded and parallel
   * mode.
   *)
  let _ = Marshal.to_string job [Marshal.Closures] in
  while not (Hh_bucket.is_done !x) do
    match !x with
    | Hh_bucket.Wait ->
      (* May waiting for remote worker to finish *)
      x := next ()
    | Hh_bucket.Job l ->
      let res = job (0, neutral) l in
      acc := merge (0, res) !acc;
      x := next ()
    | Hh_bucket.Done -> ()
  done;
  !acc

let single_threaded_call job merge neutral next =
  let job (_worker_id, a) b = job a b in
  let merge (_worker_id, a) b = merge a b in
  single_threaded_call_with_worker_id job merge neutral next

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
end = struct
  let call workers ~job ~merge ~neutral ~next =
    match workers with
    | None ->
      Caller.return (single_threaded_call_with_worker_id job merge neutral next)
    | Some workers -> Caller.multi_threaded_call workers job merge neutral next
end

module Call = CallFunctor (struct
  type 'a result = 'a

  let return x = x

  let multi_threaded_call = MultiThreadedCall.call_with_worker_id
end)

let call_with_worker_id = Call.call

let call workers ~(job : 'acc -> 'input -> 'output) ~merge ~neutral ~next =
  let job (_worker_id, a) b = job a b in
  let merge (_worker_id, a) b = merge a b in
  Call.call workers ~job ~merge ~neutral ~next

module type WorkItems_sig = sig
  type t

  type workitem

  val of_workitem : workitem -> t

  val pop : t -> workitem option * t

  val push : workitem -> t -> t
end

let call_stateless :
    (module WorkItems_sig with type workitem = 'input and type t = 'inputs) ->
    worker list ->
    job:('acc -> 'input -> 'input * 'output) ->
    merge:('output -> 'acc -> 'acc) ->
    neutral:'acc ->
    inputs:'inputs ->
    'acc =
  fun (type input inputs)
      (module WorkItems : WorkItems_sig
        with type workitem = input
         and type t = inputs)
      workers
      ~job
      ~merge
      ~neutral
      ~inputs ->
   let inputs_ref = ref inputs in
   let next () =
     let (input, inputs) = WorkItems.pop !inputs_ref in
     inputs_ref := inputs;
     match input with
     | None -> Bucket.Done
     | Some input -> Bucket.Job input
   in
   let merge (input, output) acc =
     inputs_ref := WorkItems.push input !inputs_ref;
     merge output acc
   in
   call (Some workers) ~job ~merge ~neutral ~next

(* If we ever want this in MultiWorkerLwt then move this into CallFunctor *)
let call_with_interrupt
    ?on_cancelled workers ~job ~merge ~neutral ~next ~interrupt =
  match workers with
  | Some workers when not (List.is_empty workers) ->
    Hh_logger.log
      "MultiThreadedCall.call_with_interrupt called with %d workers"
      (List.length workers);
    MultiThreadedCall.call_with_interrupt
      ?on_cancelled
      workers
      job
      merge
      neutral
      next
      interrupt
  | _ ->
    Hh_logger.log "single_threaded_call called with zero workers";
    ( single_threaded_call job merge neutral next,
      interrupt.MultiThreadedCall.env,
      None )

let next ?progress_fn ?max_size workers =
  Hh_bucket.make
    ~num_workers:
      (match workers with
      | Some w -> List.length w
      | None -> 1)
    ?progress_fn
    ?max_size

let make = WorkerController.make

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

let wrapper = { f = call }
