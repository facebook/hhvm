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

  val init : state -> state

  val next :
    files_to_process ->
    state ->
    (files_in_progress * files_to_process * state) option

  val merge : state -> (Errors.t * computation_progress) * state

  val on_cancelled : state -> file_computation list * state

  val process : state -> state
end
