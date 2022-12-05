(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This combined elaboration and validation pass will replace empty tuples with
   the `invalid_expr_` and raise an error
*)
include Naming_phase_sigs.Elabidation
