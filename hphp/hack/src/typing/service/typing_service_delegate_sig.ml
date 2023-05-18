(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_service_types

module type Delegate_sig = sig
  type state = Typing_service_delegate_types.state [@@deriving show]

  val default : state

  val make :
    job_runner:(module JobRunner_sig.S) ->
    tenant:string ->
    work_stealing_enabled:bool ->
    state

  val start :
    delegate_env ->
    state ->
    cache_remote_decls:bool ->
    use_shallow_decls_saved_state:bool ->
    state

  val stop : state -> state

  val merge : state -> Errors.t -> TypingProgress.t -> state

  val process : delegate_job_sig -> typing_result * workitem list

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
    workitem BigList.t
    * state
    * remote_computation_payload list
    * delegate_next_result option
    * Telemetry.t

  (* Get delegate progress message *)
  val get_progress : state -> string option

  val controller_started : state -> bool

  val did_run : state -> bool
end
