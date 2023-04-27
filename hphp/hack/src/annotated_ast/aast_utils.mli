(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Conservatively determines if an expression can be captured. This is useful
    for deciding when to paranthesise an expression.

    For example,

        `identity(1 + 2) * 3` -> `1 + 2 * 3`

    would be wrong because binary operations can be captured, but

        `identity($foo->bar) * 3` -> `$foo->bar * 3`

    is not wrong and cannot be wrong in any context because `$foo->bar` cannot
    be captured.
*)
val can_be_captured : ('a, 'b) Aast_defs.expr_ -> bool
