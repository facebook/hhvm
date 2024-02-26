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

type seconds_since_epoch = float

type process_file_results = {
  file_errors: Errors.t;
  file_map_reduce_data: Map_reduce.t;
  deferred_decls: Deferred_decl.deferment list;
}

val should_enable_deferring : Typing_service_types.check_file_workitem -> bool

val process_file :
  Provider_context.t ->
  Typing_service_types.check_file_workitem ->
  log_errors:bool ->
  decl_cap_mb:int option ->
  process_file_results

type result = {
  errors: Errors.t;
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
  hh_distc_fanout_threshold:int option
    (* Will use hh_distc at the given threshold if provided, otherwise will not use hh_distc.
     * Can be `None` in dev,testing, and non-fb workflows *) ->
  check_info:Typing_service_types.check_info ->
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
  hh_distc_fanout_threshold:int option
    (* Will use hh_distc at the given threshold if provided, otherwise will not use hh_distc
     *  Can be `None` in dev,testing, and non-fb workflows *) ->
  check_info:Typing_service_types.check_info ->
  ('env * result) job_result

module TestMocking : sig
  val set_is_cancelled : Relative_path.t -> unit
end
