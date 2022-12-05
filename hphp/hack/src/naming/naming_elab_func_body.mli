(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Removes all statements from the `func_body` of `method_`s and `fun_`s. This
   pass is intended for use with .hhi files *)
include Naming_phase_sigs.Elaboration
