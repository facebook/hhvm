(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

type param_entity = A.id_ * int [@@deriving eq, ord, show]

type entity = Param of param_entity [@@deriving eq, ord, show]

type ('a, 'b) any_constraint_ =
  | Intra of 'a
  | Inter of 'b

type 'a inter_constraint_ = Arg of param_entity * 'a

module type Intra = sig
  type intra_entity

  type intra_constraint

  type inter_constraint = intra_entity inter_constraint_

  type any_constraint = (intra_constraint, inter_constraint) any_constraint_

  val is_same_entity : entity -> intra_entity -> bool

  val max_iteration : int

  val equiv : any_constraint list -> any_constraint list -> bool

  val substitute_inter_intra :
    inter_constraint -> intra_constraint -> intra_constraint

  val deduce : intra_constraint list -> intra_constraint list
end
