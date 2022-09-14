(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs

type const_entity = A.id [@@deriving ord, show]

type constant_identifier_entity = {
  ident_pos: A.pos;
  class_name_opt: string option;
  const_name: string;
}
[@@deriving ord, show { with_path = false }]

type param_entity = A.id * int [@@deriving ord, show]

type class_identifier_entity = A.id [@@deriving ord, show]

type entity =
  | Param of param_entity
  | Constant of const_entity
  | ConstantIdentifier of constant_identifier_entity
[@@deriving ord, show { with_path = false }]

type ('a, 'b) any_constraint_ =
  | Intra of 'a
  | Inter of 'b
[@@deriving ord]

type 'a inter_constraint_ =
  | Arg of param_entity * 'a
  | Constant of const_entity
  | ConstantInitial of 'a
  | ConstantIdentifier of constant_identifier_entity
  | Param of param_entity
  | ClassExtends of class_identifier_entity
[@@deriving ord]

module type Intra = sig
  type intra_entity

  type intra_constraint

  type inter_constraint = intra_entity inter_constraint_

  type any_constraint = (intra_constraint, inter_constraint) any_constraint_
  [@@deriving ord]

  val is_same_entity : intra_entity -> intra_entity -> bool

  val embed_entity : entity -> intra_entity

  val max_iteration : int

  val equiv : any_constraint list -> any_constraint list -> bool

  val substitute_inter_intra_backwards :
    inter_constraint -> intra_constraint -> intra_constraint option

  val substitute_inter_intra_forwards :
    inter_constraint -> intra_constraint -> intra_constraint option

  val deduce : intra_constraint list -> intra_constraint list

  val subsets : intra_entity -> intra_entity -> intra_constraint
end

let equal_entity (ent1 : entity) (ent2 : entity) : bool =
  match (ent1, ent2) with
  | (Param ((_, f_id), f_idx), Param ((_, g_id), g_idx)) ->
    String.equal f_id g_id && Int.equal f_idx g_idx
  | (Constant (pos1, id1), Constant (pos2, id2)) ->
    A.equal_pos pos1 pos2 && String.equal id1 id2
  | ( ConstantIdentifier
        {
          ident_pos = pos1;
          class_name_opt = Some class_name1;
          const_name = constant_name1;
        },
      ConstantIdentifier
        {
          ident_pos = pos2;
          class_name_opt = Some class_name2;
          const_name = constant_name2;
        } ) ->
    A.equal_pos pos1 pos2
    && String.equal class_name1 class_name2
    && String.equal constant_name1 constant_name2
  | ( ConstantIdentifier
        { ident_pos = pos1; class_name_opt = None; const_name = constant_name1 },
      ConstantIdentifier
        { ident_pos = pos2; class_name_opt = None; const_name = constant_name2 }
    ) ->
    A.equal_pos pos1 pos2 && String.equal constant_name1 constant_name2
  | _ -> false
