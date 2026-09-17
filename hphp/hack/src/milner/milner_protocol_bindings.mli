(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  definitions: string list;
  operations: (Milner_syntax.expr -> Milner_syntax.expr) list;
}

(** [name] must be a fresh valid XHP identifier, for example [milner-node_42].
    [child] must be a closed string expression. Operations preserve the supplied
    value's type and identity and require the XHP protocol fixture. *)
val xhp : name:string -> value_hint:string -> child:Milner_syntax.expr -> t

(** Construct and visit a tree with a bounded number of lifts and splices. The
    resulting expression has type [mixed] and preserves the supplied value.
    Requires the expression-tree fixture and [defaults] capabilities. *)
val expression_tree :
  value_hint:string -> Milner_syntax.expr -> Milner_syntax.expr
