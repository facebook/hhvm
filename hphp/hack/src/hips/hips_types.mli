(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

(*
 * Interprocedural constraints
 * Arg(f, 1, p) for f(_, p, _)
 * Ret(f, p) for return f(p)
 *)
type 'entity inter_constraint =
  | Arg of A.id_ * int * 'entity
  | Ret of A.id_ * 'entity

(*
 * Constraints are either inter- or intraprocedural
 * For example:
 * Intra Has_static_key(f0, 'a', int)
 * Inter Arg(f, 1, p)
 *)
type ('intra_constraint, 'entity) constraint_ =
  | Intra of 'intra_constraint
  | Inter of 'entity inter_constraint
