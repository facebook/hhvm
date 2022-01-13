(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module LMap = Local_id.Map
module KMap = Typing_continuations.Map

type potential_targets = {
  expressions_to_modify: Pos.t list;
  hints_to_modify: Pos.t list;
}

type mode =
  | FlagTargets
  | DumpConstraints
  | SimplifyConstraints
  | SolveConstraints [@deriving eq]

type options = { mode: mode }

type entity_ =
  | Literal of Pos.t
  | Variable of int
[@@deriving eq, ord]

type entity = entity_ option

type shape_key = SK_string of string [@@deriving eq, ord]

module ShapeKeyMap = Map.Make (struct
  type t = shape_key

  let compare = compare_shape_key
end)

module ResultID = ISet

type shape_keys = ResultID.t * Typing_defs.locl_ty ShapeKeyMap.t

type exists_kind =
  | Allocation
  | Parameter
  | Argument

type constraint_ =
  | Exists of exists_kind * Pos.t
  | Has_static_keys of entity_ * shape_keys
  | Has_dynamic_key of entity_
  | Subset of entity_ * entity_

type shape_result =
  | Shape_like_dict of
      Pos.t * ResultID.t * (shape_key * Typing_defs.locl_ty) list
  | Dynamically_accessed_dict of entity_

type lenv = entity LMap.t KMap.t

type env = {
  constraints: constraint_ list;
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
