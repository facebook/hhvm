(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'a job_result = 'a * Relative_path.t list

type seconds_since_epoch = float

type process_file_results = {
  errors: Errors.t;
  deferred_decls: Deferred_decl.deferment list;
}

val should_enable_deferring :
  GlobalOptions.t -> Typing_service_types.check_file_computation -> bool

val process_file :
  Relative_path.Set.t ->
  Provider_context.t ->
  Errors.t ->
  Typing_service_types.check_file_computation ->
  process_file_results

type result = {
  errors: Errors.t;
  delegate_state: Typing_service_delegate.state;
  telemetry: Telemetry.t;
  adhoc_profiling: Adhoc_profiler.CallTree.t;
  diagnostic_pusher: Diagnostic_pusher.t option * seconds_since_epoch option;
}

val go :
  ?profiling:CgroupProfiler.Profiling.t ->
  Provider_context.t ->
  MultiWorker.worker list option ->
  Typing_service_delegate.state ->
  Telemetry.t ->
  Relative_path.Set.t ->
  Relative_path.t list ->
  memory_cap:int option ->
  longlived_workers:bool ->
  remote_execution:ReEnv.t option ->
  check_info:Typing_service_types.check_info ->
  result

(** The last element returned, a list of paths, are the files which have not been
    processed fully or at all due to interrupts. *)
val go_with_interrupt :
  ?diagnostic_pusher:Diagnostic_pusher.t ->
  Provider_context.t ->
  MultiWorker.worker list option ->
  Typing_service_delegate.state ->
  Telemetry.t ->
  Relative_path.Set.t ->
  Relative_path.t list ->
  interrupt:'env MultiWorker.interrupt_config ->
  memory_cap:int option ->
  longlived_workers:bool ->
  remote_execution:ReEnv.t option ->
  check_info:Typing_service_types.check_info ->
  profiling:CgroupProfiler.Profiling.t ->
  ('env * result) job_result

module TestMocking : sig
  val set_is_cancelled : Relative_path.t -> unit
end
