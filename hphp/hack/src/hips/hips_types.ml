(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

type 'entity inter_constraint =
  | Arg of A.id_ * int * 'entity
  | Ret of A.id_ * 'entity

type ('intra_constraint, 'entity) constraint_ =
  | Intra of 'intra_constraint
  | Inter of 'entity inter_constraint
