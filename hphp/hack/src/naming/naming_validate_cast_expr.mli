(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Ensures that the hint inside a `Cast` expression is valid *)
val pass : (Naming_phase_env.t, Naming_phase_error.t list) Naming_phase_pass.t
