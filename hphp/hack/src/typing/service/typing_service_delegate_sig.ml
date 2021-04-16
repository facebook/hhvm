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

  val create :
    artifact_store_config:ArtifactStore.config ->
    job_runner:(module JobRunner_sig.S) ->
    max_batch_size:int ->
    min_batch_size:int ->
    initial_payload_size:int ->
    raise_on_failure:bool ->
    state

  val start : delegate_env -> state -> state

  val stop : state -> state

  val next :
    files_to_process ->
    file_computation Hash_set.Poly.t ->
    state ->
    state * delegate_next_result option

  val merge : state -> Errors.t -> computation_progress -> state

  val on_cancelled : state -> file_computation list * state

  val process : delegate_job_sig -> typing_result * computation_progress

  val steal : state -> int -> file_computation list * state

  val add_telemetry : state -> Telemetry.t -> Telemetry.t

  (* Get delegate progress message *)
  val get_progress : state -> string option
end
