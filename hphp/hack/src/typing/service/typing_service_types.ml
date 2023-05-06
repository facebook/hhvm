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
  | Prefetch of Relative_path.t list
[@@deriving show]

type remote_computation_payload = {
  nonce: string;
  payload: workitem BigList.t;
  changed_files: Relative_path.t list option; [@opaque]
  dirty_files: (Relative_path.t * string option) list; [@opaque]
}
[@@deriving show]

(** This type is used for both input and output of typechecker jobs.
INPUT: [remaining] is the list of files that this job is expected to process, and [completed], [deferred] are empty.
OUTPUT: all the files that were processed by the job are placed in [completed] or [deferred];
if the job had to stop early, then [remaining] are the leftover files that the job failed to process. *)
type typing_progress = {
  remaining: workitem list;
  completed: workitem list;
  deferred: workitem list;
}
[@@deriving show]

(** This type is used for both input and output of typechecker jobs.
It is also used to accumulate the results of all typechecker jobs.
JOB-INPUT: all the fields are empty
JOB-OUTPUT: process_files will merge what it discovered into the typing_result output by each job.
ACCUMULATE: we start with all fields empty, and then merge in the output of each job as it's done. *)
type typing_result = {
  errors: Errors.t;
  dep_edges: Typing_deps.dep_edges;
  profiling_info: Telemetry.t;
      (** Instrumentation about how the workers behaved, e.g. how many decls were
      computed or how much cpu-time it took. This info is merged by adding together the sub-fields,
      so as to aggregate information from multiple workers. *)
}

let make_typing_result () =
  {
    errors = Errors.empty;
    dep_edges = Typing_deps.dep_edges_make ();
    profiling_info = Telemetry.create ();
  }

let accumulate_job_output
    (produced_by_job : typing_result) (accumulated_so_far : typing_result) :
    typing_result =
  {
    errors = Errors.merge produced_by_job.errors accumulated_so_far.errors;
    dep_edges =
      Typing_deps.merge_dep_edges
        produced_by_job.dep_edges
        accumulated_so_far.dep_edges;
    profiling_info =
      Telemetry.add
        produced_by_job.profiling_info
        accumulated_so_far.profiling_info;
  }

type delegate_job_sig = unit -> typing_result * typing_progress

type simple_delegate_job_sig = unit -> typing_result * typing_progress

type progress_kind =
  | Progress
  | DelegateProgress of delegate_job_sig
  | SimpleDelegateProgress of simple_delegate_job_sig

type job_progress = {
  kind: progress_kind;
  progress: typing_progress;
}

type check_info = {
  init_id: string;
  check_reason: string;
  log_errors: bool;
  recheck_id: string option;
  use_max_typechecker_worker_memory_for_decl_deferral: bool;
  per_file_profiling: HackEventLogger.PerFileProfilingConfig.t;
  memtrace_dir: string option;
}

type workitems_to_process = workitem BigList.t

type workitems_in_progress = workitem list

type delegate_next_result = {
  current_bucket: workitem list;
  remaining_jobs: workitem BigList.t;
  job: delegate_job_sig;
}

(**
  This module type exposes an API within hh_server running on the users' host
  that a component that distributes the work to other hosts can call.
  By analogy with MultiWorker, this component may be referred to as
  the controller: it dispatches batches of files to process (e.g., declare
  or type check) to workers.

  There are specific hh_server modules that know how to:
    - snapshot the naming table
    - get the list of files that changed since the merge base
    - import dependency graph edges

  The controller needs to be able to do these things, but it doesn't need to
  know how they are done and which server modules are responsible.

  This is why this module exists: to present a small API surface to
  the controller, providing only the functionality it needs from the server.

  Finally, the existence of this module makes it easy to mock its internals
  when testing the logic of the controller, instead of having to mock
  the individual modules that are responsible for the various operations, such
  as importing dependency graph edges.
 *)
module type LocalServerApi = sig
  (* Called by the controller to update clients with its
     current phase of execution *)
  val send_progress : string -> unit

  (* The state filename contains the state that should be updated.
     This function is called by the controller after it receives a response
     from a worker that contains such state.
     It may be called many times during execution.
  *)
  val update_state : state_filename:string -> check_id:string option -> unit

  val upload_naming_table : nonce:string -> unit

  (* Tells the server to save the naming table state to a given
     destination path.
  *)
  val snapshot_naming_table_base : destination_path:string -> unit Future.t

  (* Tells the server to save just the portion of the naming table that
     changed since the loaded naming table base. If there were no base, then
     the snapshot should be the entire naming table.
  *)
  val snapshot_naming_table_diff : destination_path:string -> unit

  (* Begins getting dirty files given a mergebase.
    *)
  val begin_get_changed_files : mergebase:string option -> string list Future.t

  (* Packages the files changed since the mergebase into a single file.
    *)
  val write_changed_files : string list -> destination_path:string -> unit

  (* Gather the filepaths changed since the mergebase and load their content *)
  val load_changed_files : string list -> (Relative_path.t * string option) list
end

type delegate_env = {
  init_id: string;
  mergebase: Hg.hg_rev option;
  num_workers: int;
  recheck_id: string;
  nonce: Int64.t;
  tenant: string;
  root: string;
  tcopt: TypecheckerOptions.t;
  (* This module exposes to the controller the limited set of operations that
     it needs, without exposing the underlying types or implementation details.
     It is also helpful in simplifying the isolation of the controller
     for unit testing. *)
  server: (module LocalServerApi);
  (* Represents the version of hh_server that the remote hosts should install,
     if it's not the default that they would be otherwise using. This field
     is only useful in development and should not be set in the normal course
     of business during type checking user's code. *)
  version_specifier: string option;
  (* The minimum log level workers should be logging at *)
  worker_min_log_level: Hh_logger.Level.t;
  naming_table_manifold_path: string option;
  (* Function that returns a future of result of the manifold path and changed_files list.
     This largely exists to allow unit tests to run without making saved state calls to watchman.
  *)
  saved_state_data_loader:
    (unit -> (string option * Relative_path.t list, string) result Future.t)
    option;
  load_hack_64_distc_saved_state: bool;
      (* Send a hack/64_distc Manifold path to remote workers *)
}
