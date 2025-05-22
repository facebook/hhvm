(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Ensures that the return hint and all parameter hints of a polymorphic lambda
    expression are explicitly given *)
val pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
