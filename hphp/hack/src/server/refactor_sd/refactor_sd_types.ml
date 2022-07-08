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

type constraint_ =
  | Introduction of Pos.t
  | Upcast of entity_
  | Subset of entity_ * entity_

type refactor_sd_result =
  | Exists_Upcast
  | No_Upcast

type lenv = entity LMap.t KMap.t

type env = {
  constraints: constraint_ list;
  lenv: lenv;
  tast_env: Tast_env.t;
}
