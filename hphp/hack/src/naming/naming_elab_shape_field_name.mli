(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* We permit class constants to be used as shape field names. Here we replace
   uses of `self` with the class to which they refer or `unknown` if the shape
   is not defined within the context of a class *)
val top_down_pass : Naming_phase_env.t Naming_phase_pass.t

val bottom_up_pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
