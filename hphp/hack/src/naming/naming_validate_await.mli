(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val validate_fun_def :
  (Naming_phase_error.t -> unit) -> (unit, unit) Aast.fun_def -> unit

val validate_class :
  (Naming_phase_error.t -> unit) -> (unit, unit) Aast.class_ -> unit

val validate_program :
  (Naming_phase_error.t -> unit) -> (unit, unit) Aast.program -> unit

val validate_stmt :
  (Naming_phase_error.t -> unit) -> (unit, unit) Aast.stmt -> unit
