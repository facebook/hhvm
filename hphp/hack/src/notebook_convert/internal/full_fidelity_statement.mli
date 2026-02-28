(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Note: this utility function is hard to move into the parser
   because the parser is codegenned+functorized.
*)
val is_statement : Full_fidelity_positioned_syntax.syntax -> bool
