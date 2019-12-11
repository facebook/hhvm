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

type file_computation =
  | Check of check_file_computation
  | Declare of Relative_path.t
  | Prefetch of Relative_path.t list

type computation_progress = {
  completed: file_computation list;
  remaining: file_computation list;
  deferred: file_computation list;
}

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

type delegate_env = {
  root: string;
  init_id: string;
  mergebase: Hg.hg_rev option;
  num_workers: int;
}
