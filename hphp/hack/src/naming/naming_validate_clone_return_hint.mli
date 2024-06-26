(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Validate that return type of __clone is void *)
val pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
