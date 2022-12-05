(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This combined elaboration and validation pass will remove the type arguments
   to `Habstr(_,_)` and raise an error

  It is intended for use when the `--higher-kinded-types` typechecker option
  is _not_ set,
*)
val pass : (Naming_phase_env.t, Naming_phase_error.t list) Naming_phase_pass.t
