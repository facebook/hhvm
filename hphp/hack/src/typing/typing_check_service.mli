(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type ('a, 'b, 'c, 'd) job_result = 'a * 'b * 'c * 'd * Relative_path.t list

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
  remote_execution:bool ->
  check_info:Typing_service_types.check_info ->
  Errors.t * Typing_service_delegate.state * Telemetry.t

val go_with_interrupt :
  Provider_context.t ->
  MultiWorker.worker list option ->
  Typing_service_delegate.state ->
  Telemetry.t ->
  Relative_path.Set.t ->
  Relative_path.t list ->
  interrupt:'a MultiWorker.interrupt_config ->
  memory_cap:int option ->
  longlived_workers:bool ->
  remote_execution:bool ->
  check_info:Typing_service_types.check_info ->
  profiling:CgroupProfiler.Profiling.t ->
  (Errors.t, Typing_service_delegate.state, Telemetry.t, 'a) job_result

module TestMocking : sig
  val set_is_cancelled : Relative_path.t -> unit
end
