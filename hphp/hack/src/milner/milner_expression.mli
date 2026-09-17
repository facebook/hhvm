(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type family =
  | Atom
  | Flow
  | Async
  | Callable

(** A function that preserves its argument's type and value, constructed from
    bounded compositions of expressions and statements. Its caller must provide
    the default callable context. *)
val operation : family -> ty:Milner_generate.Type.t -> Milner_syntax.expr
