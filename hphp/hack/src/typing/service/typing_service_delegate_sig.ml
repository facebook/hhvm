(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_service_types

module type Delegate_sig = sig
  type state = Typing_service_delegate_types.state [@@deriving show]

  val default : state

  val make :
    artifact_store_config:ArtifactStore.config ->
    job_runner:(module JobRunner_sig.S) ->
    tenant:string ->
    max_batch_size:int ->
    min_batch_size:int ->
    initial_payload_size:int ->
    raise_on_failure:bool ->
    state

  val start :
    delegate_env ->
    state ->
    HulkStrategy.hulk_mode ->
    cache_remote_decls:bool ->
    use_shallow_decls_saved_state:bool ->
    state

  val stop : state -> state

  val next :
    workitems_to_process ->
    workitem Hash_set.Poly.t ->
    state ->
    state * delegate_next_result option

  val merge : state -> Errors.t -> typing_progress -> state

  val on_cancelled : state -> workitem list * state

  val process : delegate_job_sig -> typing_result * typing_progress

  val steal : state -> int -> workitem list * state

  val dispatch :
    state ->
    workitem BigList.t ->
    int ->
    remote_computation_payload list
    * workitem BigList.t
    * state
    * (Telemetry.t * float)

  val collect :
    telemetry:Telemetry.t ->
    state ->
    workitem BigList.t ->
    int ->
    remote_computation_payload list ->
    bool ->
    workitem BigList.t
    * state
    * remote_computation_payload list
    * delegate_next_result option
    * Telemetry.t

  val add_telemetry : state -> Telemetry.t -> Telemetry.t

  (* Get delegate progress message *)
  val get_progress : state -> string option

  val controller_started : state -> bool

  val did_run : state -> bool
end
