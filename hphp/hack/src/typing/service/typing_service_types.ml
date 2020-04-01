(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type check_file_computation = {
  path: Relative_path.t;
  deferred_count: int;
}
[@@deriving show]

type file_computation =
  | Check of check_file_computation
  | Declare of Relative_path.t
  | Prefetch of Relative_path.t list
[@@deriving show]

type computation_progress = {
  completed: file_computation list;
  remaining: file_computation list;
  deferred: file_computation list;
}
[@@deriving show]

type delegate_job_sig = unit -> Errors.t * computation_progress

type progress_kind =
  | Progress
  | DelegateProgress of delegate_job_sig

type job_progress = {
  kind: progress_kind;
  progress: computation_progress;
}

type check_info = {
  init_id: string;
  recheck_id: string option;
  profile_log: bool;
  profile_type_check_twice: bool;
  profile_type_check_duration_threshold: float;
}

type files_to_process = file_computation list

type files_in_progress = file_computation list

type delegate_next_result = {
  current_bucket: file_computation list;
  remaining_jobs: file_computation list;
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

  (* Called by the controller to update clients with its progress on
    type checking, indicating how far along it is. `unit` represents
    the items that are being processed, e.g., 'workers' or 'files' *)
  val send_percentage_progress :
    operation:string -> done_count:int -> total_count:int -> unit:string -> unit

  (* The state filename contains the state that should be updated.
    This function is called by the controller after it receives a response
    from a worker that contains such state.
    It may be called many times during execution.
    *)
  val update_state : state_filename:string -> unit

  (* Tells the server to save the naming table state to a given
    destination path.
    *)
  val snapshot_naming_table_base : destination_path:string -> unit

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
end

type delegate_env = {
  defer_class_declaration_threshold: int;
  init_id: string;
  mergebase: Hg.hg_rev option;
  num_workers: int;
  recheck_id: string;
  root: string;
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
  (* The mode to be used by remote job runner *)
  remote_mode: JobRunner.remote_mode;
  (* Optional transport channel used by remote type checking. None means default. *)
  transport_channel: string option;
}
