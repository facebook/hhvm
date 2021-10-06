(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type mode =
  | FlagTargets
  | DumpConstraints
  | SolveConstraints [@deriving eq]

type options = { mode: mode }

type entity_ = Literal of Pos.t

type entity = entity_ option

type constraint_ =
  | Exists of entity_
  | Has_static_key of entity_ * Tast.expr_ * Typing_defs.locl_ty
  | Has_dynamic_key of entity_

type env = { constraints: constraint_ list }
