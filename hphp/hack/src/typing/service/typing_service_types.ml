(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type check_file_workitem = {
  path: Relative_path.t;
  was_already_deferred: bool;
}
[@@deriving show]

type workitem =
  | Check of check_file_workitem
  | Declare of (Relative_path.t * string)
[@@deriving show]

module TypingProgress : sig
  type t

  type progress_outcome = {
    deferred_workitems: workitem list;
    continue: bool;
  }

  val init : workitem list -> t

  val of_completed : workitem list -> t

  val remaining : t -> workitem list

  val completed : t -> workitem list

  val deferred : t -> workitem list

  val progress_through :
    init:'acc -> t -> (workitem -> 'acc -> progress_outcome * 'acc) -> t * 'acc
end = struct
  (** This type is used for both input and output of typechecker jobs.
    INPUT: [remaining] is the list of files that this job is expected to process, and [completed], [deferred] are empty.
    OUTPUT: all the files that were processed by the job are placed in [completed] or [deferred];
    if the job had to stop early, then [remaining] are the leftover files that the job failed to process. *)
  type t = {
    remaining: workitem list;
    completed: workitem list;
    deferred: workitem list;
  }

  type progress_outcome = {
    deferred_workitems: workitem list;
    continue: bool;
  }

  let init remaining = { remaining; completed = []; deferred = [] }

  let of_completed completed = { remaining = []; completed; deferred = [] }

  let remaining t = t.remaining

  let completed t = t.completed

  let deferred t = t.deferred

  let advance
      ({ remaining; completed; deferred } : t)
      (acc : 'acc)
      (f : workitem -> 'acc -> progress_outcome * 'acc) :
      (t * 'acc * bool) option =
    match remaining with
    | [] -> None
    | x :: remaining ->
      let ({ deferred_workitems; continue }, acc) = f x acc in
      let progress =
        {
          remaining;
          completed = x :: completed;
          deferred = deferred_workitems @ deferred;
        }
      in
      Some (progress, acc, continue)

  let progress_through
      ~(init : 'acc)
      (progress : t)
      (f : workitem -> 'acc -> progress_outcome * 'acc) : t * 'acc =
    let rec go (progress : t) (acc : 'acc) =
      match advance progress acc f with
      | None -> (progress, acc)
      | Some (progress, acc, continue) ->
        if continue then
          go progress acc
        else
          (progress, acc)
    in
    go progress init
end

(** This type is used for both input and output of typechecker jobs.
It is also used to accumulate the results of all typechecker jobs.
JOB-INPUT: all the fields are empty
JOB-OUTPUT: process_files will merge what it discovered into the typing_result output by each job.
ACCUMULATE: we start with all fields empty, and then merge in the output of each job as it's done. *)
type typing_result = {
  errors: Errors.t;
  map_reduce_data: Map_reduce.t;
  dep_edges: Typing_deps.dep_edges;
  profiling_info: Telemetry.t;
      (** Instrumentation about how the workers behaved, e.g. how many decls were
      computed or how much cpu-time it took. This info is merged by adding together the sub-fields,
      so as to aggregate information from multiple workers. *)
}

let make_typing_result () =
  {
    errors = Errors.empty;
    map_reduce_data = Map_reduce.empty;
    dep_edges = Typing_deps.dep_edges_make ();
    profiling_info = Telemetry.create ();
  }

let accumulate_job_output
    (produced_by_job : typing_result) (accumulated_so_far : typing_result) :
    typing_result =
  {
    errors = Errors.merge produced_by_job.errors accumulated_so_far.errors;
    map_reduce_data =
      Map_reduce.reduce
        produced_by_job.map_reduce_data
        accumulated_so_far.map_reduce_data;
    dep_edges =
      Typing_deps.merge_dep_edges
        produced_by_job.dep_edges
        accumulated_so_far.dep_edges;
    profiling_info =
      Telemetry.add
        produced_by_job.profiling_info
        accumulated_so_far.profiling_info;
  }

type progress_kind = Progress

type job_progress = {
  kind: progress_kind;
  progress: TypingProgress.t;
}

type check_info = {
  init_id: string;
  check_reason: string;
  log_errors: bool;
  recheck_id: string option;
  per_file_profiling: HackEventLogger.PerFileProfilingConfig.t;
  memtrace_dir: string option;
}

type workitems_to_process = workitem BigList.t

type workitems_in_progress = workitem list
