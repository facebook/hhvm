(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type family =
  | Atom
  | Callable

(** A bounded composition of callable operations preserving the argument's
    type and value. Calling the result requires the default context. *)
val operation : family -> ty:Milner_generate.Type.t -> Milner_syntax.expr
