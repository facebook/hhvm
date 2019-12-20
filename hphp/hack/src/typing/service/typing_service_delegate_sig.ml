(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_service_types

module type Delegate_sig = sig
  type state [@@deriving show]

  val create : ?max_batch_size:int -> ?min_batch_size:int -> unit -> state

  val start : delegate_env -> state -> recheck_id:string option -> state

  val stop : state -> state

  val next :
    files_to_process ->
    file_computation Hash_set.Poly.t ->
    state ->
    state * delegate_next_result option

  val merge : state -> Errors.t -> computation_progress -> state

  val on_cancelled : state -> file_computation list * state

  val process : delegate_job_sig -> Errors.t * computation_progress

  val steal : state -> int -> file_computation list * state

  val add_telemetry : state -> Telemetry.t -> Telemetry.t
end
