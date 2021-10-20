(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module LMap = Local_id.Map

type mode =
  | FlagTargets
  | DumpConstraints
  | SimplifyConstraints
  | SolveConstraints [@deriving eq]

type options = { mode: mode }

type entity_ = Literal of Pos.t

type entity = entity_ option

type shape_key = SK_string of string [@@deriving eq, ord]

type constraint_ =
  | Exists of entity_
  | Has_static_key of entity_ * shape_key * Typing_defs.locl_ty
  | Has_dynamic_key of entity_

type shape_result =
  | Shape_like_dict of entity_ * (shape_key * Typing_defs.locl_ty) list
  | Dynamically_accessed_dict of entity_

type env = {
  constraints: constraint_ list;
  lenv: entity LMap.t;
  saved_env: Tast.saved_env;
}
