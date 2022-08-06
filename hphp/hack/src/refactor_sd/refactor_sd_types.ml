(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module LMap = Local_id.Map
module KMap = Typing_continuations.Map

exception Refactor_sd_exn of string

type analysis_mode =
  | FlagTargets
  | DumpConstraints
  | SimplifyConstraints
  | SolveConstraints [@deriving eq]

type refactor_mode =
  | Class
  | Function
  | Method

type options = {
  analysis_mode: analysis_mode;
  refactor_mode: refactor_mode;
}

type element_info = { element_name: string }

type entity_ =
  | Literal of Pos.t
  | Variable of int
[@@deriving eq, ord]

type entity = entity_ option

type constraint_ =
  | Introduction of Pos.t
  | Upcast of entity_ * Pos.t
  | Subset of entity_ * entity_
  | Called of Pos.t

type refactor_sd_result =
  | Exists_Upcast of Pos.t
  | No_Upcast

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
