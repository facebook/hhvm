(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Ast_defs
module T = Typing_defs
module LMap = Local_id.Map
module KMap = Typing_continuations.Map
module HT = Hips_types

exception Shape_analysis_exn of string

module CommonSet (S : Set.S) = struct
  let unions_map ~f set =
    S.fold (fun elt acc -> S.union (f elt) acc) set S.empty
end

type potential_targets = {
  expressions_to_modify: Pos.t list;
  hints_to_modify: Pos.t list;
}

type mode =
  | FlagTargets
  | DumpConstraints
  | DumpDerivedConstraints
  | SimplifyConstraints
  | Codemod
  | SolveConstraints
  | CloseConstraints [@deriving eq]

type options = {
  mode: mode;
  verbosity: int;
}

type entity_ =
  | Literal of Pos.t
  | Variable of int
  | Inter of HT.entity
[@@deriving eq, ord]

type entity = entity_ option

type shape_keys = T.locl_phase T.shape_field_type T.TShapeMap.t

type marker_kind =
  | Allocation
  | Parameter
  | Return
  | Debug
[@@deriving ord, show { with_path = false }]

module Codemod = struct
  type kind =
    | Allocation
    | Hint
  [@@deriving show { with_path = false }]
end

type source =
  | Base
  | Derived
[@@deriving ord, show { with_path = false }]

type constraint_ =
  | Marks of marker_kind * Pos.t
  | Has_static_key of source * entity_ * T.TShapeField.t * T.locl_ty
  | Has_optional_key of entity_ * T.TShapeField.t
  | Has_dynamic_key of entity_
  | Subsets of entity_ * entity_
[@@deriving ord]

type inter_constraint_ = entity_ HT.inter_constraint_

type shape_result =
  | Shape_like_dict of Pos.t * marker_kind * shape_keys
  | Dynamically_accessed_dict of entity_

type lenv = entity LMap.t KMap.t

type 'constraint_ decorated = {
  hack_pos: Pos.t;
  origin: int;
  constraint_: 'constraint_;
}

type decorated_constraints =
  constraint_ decorated list * inter_constraint_ decorated list

type env = {
  constraints: constraint_ decorated list;
  inter_constraints: inter_constraint_ decorated list;
  lenv: lenv;
  return: entity;
  tast_env: Tast_env.t;
}

module PointsToSet = Set.Make (struct
  type t = entity_ * entity_

  let compare (a, b) (c, d) =
    match compare_entity_ a c with
    | 0 -> compare_entity_ b d
    | x -> x
end)

module EntityMap = Map.Make (struct
  type t = entity_

  let compare = compare_entity_
end)

module EntitySet = struct
  module S = Set.Make (struct
    type t = entity_

    let compare = compare_entity_
  end)

  include S
  include CommonSet (S)
end

module ConstraintSet = Set.Make (struct
  type t = constraint_

  let compare = compare_constraint_
end)

type log_event =
  | Result of {
      id: string;
      shape_result: shape_result;
    }
  | Failure of {
      id: string;
      error_message: string;
    }

type any_constraint = (constraint_, inter_constraint_) HT.any_constraint_
