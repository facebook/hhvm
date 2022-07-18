(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Typing_defs
module LMap = Local_id.Map
module KMap = Typing_continuations.Map

exception Shape_analysis_exn of string

type potential_targets = {
  expressions_to_modify: Pos.t list;
  hints_to_modify: Pos.t list;
}

type mode =
  | FlagTargets
  | DumpConstraints
  | SimplifyConstraints
  | Codemod
  | SolveConstraints [@deriving eq]

type options = {
  mode: mode;
  verbosity: int;
}

type entity_ =
  | Literal of Pos.t
  | Variable of int
[@@deriving eq, ord]

type entity = entity_ option

type shape_keys = T.locl_phase T.shape_field_type T.TShapeMap.t

type marker_kind =
  | Allocation
  | Parameter
  | Argument
[@@deriving show { with_path = false }]

type constraint_ =
  | Marks of marker_kind * Pos.t
  | Has_static_key of entity_ * T.TShapeMap.key * T.locl_ty
  | Has_dynamic_key of entity_
  | Subsets of entity_ * entity_
  | Joins of {
      left: entity_;
      right: entity_;
      join: entity_;
    }

type shape_result =
  | Shape_like_dict of Pos.t * marker_kind * shape_keys
  | Dynamically_accessed_dict of entity_

type lenv = entity LMap.t KMap.t

type decorated_constraint = {
  hack_pos: Pos.t;
  origin: int;
  constraint_: constraint_;
}

type env = {
  constraints: decorated_constraint list;
  lenv: lenv;
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

module EntitySet = Set.Make (struct
  type t = entity_

  let compare = compare_entity_
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
