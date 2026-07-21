(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Rejects variadic named parameters on function definitions (e.g.
   `function f(named int ...$xs): void {}`) unless the
   `tco_variadic_named_parameters` typechecker option is enabled. *)
val pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
