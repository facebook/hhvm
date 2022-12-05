(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This elaboration pass will:
  - pessimise return types; and
  - add `SupportDyn` attributes to classes; and
  - add upper bounds to type parameters.

  It is intended for use when the `--everything-sdt` typechecker option is set
*)

val top_down_pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t

val bottom_up_pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t
