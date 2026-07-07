(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Rejects function type hints with a named variadic parameter (e.g.
   `(function(named int...): void)`) unless the `tco_named_variadic_type`
   typechecker option is enabled. *)
val pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
