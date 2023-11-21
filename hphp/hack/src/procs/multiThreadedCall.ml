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

exception Coalesced_failures of WorkerController.worker_failure list

let coalesced_failures_to_string failures =
  let failure_strings =
    List.map failures ~f:WorkerController.failure_to_string
  in
  Printf.sprintf
    "Coalesced_failures[%s]"
    (String.concat ~sep:", " failure_strings)

let () =
  Stdlib.Printexc.register_printer @@ function
  | Coalesced_failures failures -> Some (coalesced_failures_to_string failures)
  | _ -> None

type cancel_reason = {
  user_message: string;
  log_message: string;
  timestamp: float;
}

type interrupt_result =
  | Cancel of cancel_reason
  | Continue

type 'env interrupt_handler = 'env -> 'env * interrupt_result

type 'env interrupt_config = {
  env: 'env;
  handlers: 'env -> (Unix.file_descr * 'env interrupt_handler) list;
}

type worker_id = int

let no_interrupt env = { handlers = (fun _ -> []); env }

(* Integer that increases with every invocation of multi_threaded_call, used to
 * distinguish worker handles that belong to current job vs those that are still
 * processing some other job (in cases when multi_threaded_call is called during
 * an already ongoing multi_threaded_call job. *)
let call_id = ref 0

(* Exceptions from parallel jobs are in general not recoverable - the workers
 * are dead and we don't respawn them. The only reason someone should catch
 * them is to log and exit. Setting on_exception handler allows you to do it
 * before any caller has a chance to catch the exception and attempt to handle
 * it. *)
let nested_exception : Exception.t option ref = ref None

let on_exception_ref = ref (fun e -> nested_exception := Some e)

let multi_threaded_call
    (type job_input job_output acc env)
    workers
    (job : worker_id * acc -> job_input -> job_output)
    (merge : worker_id * job_output -> acc -> acc)
    (neutral : acc)
    (next : job_input Hh_bucket.next)
    ?(on_cancelled : (unit -> job_input list) option)
    (interrupt : env interrupt_config) :
    (acc * env) * (job_input list * cancel_reason) option =
  incr call_id;
  let call_id = !call_id in
  (* Split workers into those that are free, and those that are still doing
   * previous jobs. *)
  let (workers, handles) =
    List.fold workers ~init:([], []) ~f:(fun (workers, handles) worker ->
        (* Note than now some handles have mismatched types. We need to remember
         * to check their get_call_id against this multi_threaded_call call_id
         * before trusting the types. *)
        match WorkerController.get_handle_UNSAFE worker with
        | None -> (worker :: workers, handles)
        | Some handle -> (workers, handle :: handles))
  in
  let is_current h = call_id = WorkerController.get_call_id h in
  (* merge accumulator, leaving environment and interrupt handlers untouched *)
  let merge x (y1, y2, y3) = (merge x y1, y2, y3) in
  (* interrupt handlers are irrelevant after job is done *)
  let unpack_result (acc, env, _handlers) = (acc, env) in
  let handler_fds (_, _, handlers) = List.map handlers ~f:fst in
  let rec add_pending acc =
    match next () with
    | Hh_bucket.Done -> acc
    | Hh_bucket.Job a -> add_pending (a :: acc)
    | Hh_bucket.Wait ->
      (* There's not really a good solution to generically getting the pending
         work items when attempting to cancel a job that's in the Wait state,
         so we depend on those jobs to determine their own state in the
         [on_cancelled] handler. *)
      failwith "cancelling jobs with Wait not supported"
  in
  (* When a job is cancelled, return all the jobs that were not started OR were
   * cancelled in the middle (so you better hope they are idempotent).*)
  let check_cancel handles ready_fds (acc, env, handlers) =
    let (env, decision, handlers) =
      List.fold
        handlers
        ~init:(env, Continue, handlers)
        ~f:(fun (env, prior_decision, handlers) (fd, handler) ->
          let is_fd_for_this_handler =
            List.mem ~equal:Poly.( = ) ready_fds fd
          in
          match prior_decision with
          | Cancel _ -> (env, prior_decision, handlers)
          | Continue when not is_fd_for_this_handler ->
            (env, prior_decision, handlers)
          | Continue ->
            let (env, decision) = handler env in
            (* Re-raise the exception even if handler have caught and ignored it *)
            Option.iter !nested_exception ~f:(fun e -> Exception.reraise e);
            (* running a handler could have changed the handlers, so need to regenerate them based on new environment *)
            let handlers = interrupt.handlers env in
            (env, decision, handlers))
    in
    let res = (acc, env, handlers) in
    match decision with
    | Cancel reason ->
      WorkerController.cancel handles;
      let unfinished =
        match on_cancelled with
        | Some f -> f ()
        | None ->
          let unfinished = List.map handles ~f:WorkerController.get_job in
          add_pending unfinished
      in
      (res, Some (unfinished, reason))
    | Continue -> (res, None)
  in
  let rec dispatch workers handles acc =
    (* 'worker' represents available workers. *)
    (* 'handles' represents pendings jobs. *)
    (* 'acc' are the accumulated results. *)
    match workers with
    | None when not @@ List.exists handles ~f:is_current ->
      (* No more handles at this recursion level *)
      (unpack_result acc, None)
    | None (* No more jobs to start *)
    | Some [] ->
      (* No worker available: wait for some workers to finish. *)
      collect [] handles acc
    | Some (worker :: workers) ->
      (* At least one worker is available... *)
      (match next () with
      | Hh_bucket.Wait -> collect (worker :: workers) handles acc
      | Hh_bucket.Done ->
        (* ... but no more job to be distributed, let's collect results. *)
        dispatch None handles acc
      | Hh_bucket.Job bucket ->
        (* ... send a job to the worker.*)
        let worker_id = WorkerController.worker_id worker in
        let handle =
          WorkerController.call
            ~call_id
            worker
            (fun xl -> job (worker_id, neutral) xl)
            bucket
        in
        dispatch (Some workers) (handle :: handles) acc)
  and collect workers handles acc =
    let { WorkerController.readys; waiters; ready_fds } =
      WorkerController.select handles (handler_fds acc)
    in
    let workers = List.map ~f:WorkerController.get_worker readys @ workers in
    (* Collect the results. *)
    let (acc, failures) =
      (* Fold the results of all the finished workers. Also, coalesce the exit
       * statuses for all the failed workers. *)
      List.fold_left
        ~f:
          begin
            fun (acc, failures) h ->
              try
                let res = WorkerController.get_result h in
                (* Results for handles from other calls are cached by get_result
                 * and will be retrieved later, so we ignore them here *)
                let acc =
                  if is_current h then
                    let worker_id =
                      WorkerController.get_worker h
                      |> WorkerController.worker_id
                    in
                    merge (worker_id, res) acc
                  else
                    acc
                in
                (acc, failures)
              with
              | WorkerController.Worker_failed (_, failure) ->
                (acc, failure :: failures)
          end
        ~init:(acc, [])
        readys
    in
    if not (List.is_empty failures) then
      (* If any single worker failed, we stop fanning out more jobs. *)
      raise (Coalesced_failures failures)
    else
      match check_cancel waiters ready_fds acc with
      | (acc, Some unfinished_and_reason) ->
        (unpack_result acc, Some unfinished_and_reason)
      | (acc, None) ->
        (* And continue.. *)
        dispatch (Some workers) waiters acc
  in
  try
    let () = nested_exception := None in
    dispatch
      (Some workers)
      handles
      (neutral, interrupt.env, interrupt.handlers interrupt.env)
  with
  | exn ->
    let e = Exception.wrap exn in
    !on_exception_ref e;
    Exception.reraise e

let call_with_worker_id workers job merge neutral next =
  let ((res, ()), unfinished_and_reason) =
    multi_threaded_call workers job merge neutral next (no_interrupt ())
  in
  assert (Option.is_none unfinished_and_reason);
  res

let call workers job merge neutral next =
  let job (_id, a) b = job a b in
  let merge (_id, a) b = merge a b in
  let ((res, ()), unfinished_and_reason) =
    multi_threaded_call workers job merge neutral next (no_interrupt ())
  in
  assert (Option.is_none unfinished_and_reason);
  res

let call_with_interrupt workers job merge neutral next ?on_cancelled interrupt =
  SharedMem.set_allow_removes false;

  (* Interrupting of nested jobs is not implemented *)
  assert (
    List.for_all workers ~f:(fun x ->
        Option.is_none @@ WorkerController.get_handle_UNSAFE x));
  let job (_id, a) b = job a b in
  let merge (_id, a) b = merge a b in
  let ((res, interrupt_env), unfinished_and_reason) =
    multi_threaded_call workers job merge neutral next ?on_cancelled interrupt
  in
  SharedMem.set_allow_removes true;
  (res, interrupt_env, unfinished_and_reason)

let on_exception f = on_exception_ref := f
