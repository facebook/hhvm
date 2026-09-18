(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Exercise a memoized callable with equal and distinct keys, then return the
    once-evaluated [value]. The enclosing context must provide [defaults]. *)
val operation : ty:string -> value:Milner_syntax.expr -> Milner_syntax.expr
