(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Evaluate [value] once, then compose bounded expressions and statements that
    preserve its type. Supplied operations must accept and produce [ty]. The enclosing context must provide [defaults]. *)
val compose :
  ty:string ->
  value:Milner_syntax.expr ->
  operations:(Milner_syntax.expr -> Milner_syntax.expr) list ->
  Milner_syntax.expr
