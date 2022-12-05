(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* invariant is subject to a source-code transform in the HHVM
 * runtime: the arguments to invariant are lazily evaluated only in
 * the case in which the invariant condition does not hold. So:
 *
 *   invariant_violation(<condition>, <format>, <format_args...>)
 *
 * ... is rewritten as:
 *
 *   if (!<condition>) {
 *     invariant_violation(<format>, <format_args...>);
 *   }
 *)
val pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t
