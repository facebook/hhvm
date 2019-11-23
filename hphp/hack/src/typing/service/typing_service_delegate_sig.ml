(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_service_types

module type Delegate_sig = sig
  type state

  val create : unit -> state

  val start : delegate_env -> state -> state

  val stop : state -> state

  val next :
    files_to_process ->
    state ->
    state * (files_in_progress * files_to_process * delegate_job_sig) option

  val merge : state -> state

  val on_cancelled : state -> file_computation list * state

  val process : delegate_job_sig -> Errors.t * computation_progress
end
