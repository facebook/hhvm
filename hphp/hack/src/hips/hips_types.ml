(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

type const_entity = A.id [@@deriving ord, show]

type identifier_entity = A.id [@@deriving ord, show]

type param_entity = A.id_ * int * Pos.t [@@deriving ord, show]

type entity =
  | Param of param_entity
  | Const of const_entity
  | Identifier of identifier_entity
[@@deriving ord, show { with_path = false }]

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

let equal_entity (ent1 : entity) (ent2 : entity) : bool =
  match (ent1, ent2) with
  | (Param (f_id, f_idx, _), Param (g_id, g_idx, _)) ->
    String.equal f_id g_id && Int.equal f_idx g_idx
  | (Const (pos1, id1), Const (pos2, id2)) ->
    A.equal_pos pos1 pos2 && String.equal id1 id2
  | (Identifier (pos1, name1), Identifier (pos2, name2)) ->
    A.equal_pos pos1 pos2 && String.equal name1 name2
  | _ -> false
