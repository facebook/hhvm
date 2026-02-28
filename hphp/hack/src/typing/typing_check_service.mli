(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'a job_result =
  'a * (Relative_path.t list * MultiThreadedCall.cancel_reason) option

(* distc_config specifies if hh_distc is enabled and its thresholds
 * - hh_distc is disabled (eg. config set to None) in dev testing and non-fb workflows
 * - if hh_distc is enabled, jobs with fanout >= fanout_threshold are sent to distc
 *   - if enable_fanout_aware_distc is enabled, then
 *     - if fanout_threshold <= fanout < fanout_full_init_threshold, then
 *       fanout aware distc is performed;
 *     - if fanout_full_init_threshold <= fanout, then full init is performed;
 *   - if enable_fanout_aware_distc is not set, then full init is performed.
 *)
type distc_config_options = {
  enable_fanout_aware_distc: bool;
  fanout_threshold: int;
  fanout_full_init_threshold: int;
}

type distc_config = distc_config_options option

type seconds_since_epoch = float

type process_file_results = {
  file_diagnostics: Diagnostics.t;
  file_map_reduce_data: Map_reduce.t;
  deferred_decls: Deferred_decl.deferment list;
}

val should_enable_deferring : Typing_service_types.check_file_workitem -> bool

val process_file :
  Provider_context.t ->
  Typing_service_types.check_file_workitem ->
  decl_cap_mb:int option ->
  process_file_results

type result = {
  diagnostics: Diagnostics.t;
  warnings_saved_state: Warnings_saved_state.t option;
  telemetry: Telemetry.t;
  time_first_error: seconds_since_epoch option;
}

val go :
  Provider_context.t ->
  MultiWorker.worker list option ->
  Telemetry.t ->
  Relative_path.t list ->
  root:Path.t option ->
  longlived_workers:bool ->
  hh_distc_config:distc_config ->
  check_info:Typing_service_types.check_info ->
  warnings_saved_state:Warnings_saved_state.t option ->
  result

(** The last element returned, a list of paths, are the files which have not been
    processed fully or at all due to interrupts. *)
val go_with_interrupt :
  Provider_context.t ->
  MultiWorker.worker list option ->
  Telemetry.t ->
  Relative_path.t list ->
  root:Path.t option ->
  interrupt:'env MultiWorker.interrupt_config ->
  longlived_workers:bool ->
  hh_distc_config:distc_config ->
  check_info:Typing_service_types.check_info ->
  warnings_saved_state:Warnings_saved_state.t option ->
  ('env * result) job_result

module TestMocking : sig
  val set_is_cancelled : Relative_path.t -> unit
end
