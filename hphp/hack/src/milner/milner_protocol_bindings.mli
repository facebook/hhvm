(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  definitions: string list;
  expressions: (string * Milner_syntax.expr) list;
}

(** [name] must be a fresh valid XHP identifier, for example [milner-node_42].
    [child] must be a closed string expression. The payload hint is the shared
    type of this placeholder group. *)
val xhp : name:string -> value_hint:string -> child:Milner_syntax.expr -> t

val expression_tree : value_hint:string -> t
