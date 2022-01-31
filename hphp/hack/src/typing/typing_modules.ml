(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let can_access ~(current : Ast_defs.id option) ~(target : Ast_defs.id option) =
  match (current, target) with
  | (None, None)
  | (Some _, None) ->
    `Yes
  | (None, Some m) -> `Outside m
  | (Some (_, m_current), Some (_, m_target))
    when String.equal m_current m_target ->
    `Yes
  | (Some current, Some target) -> `Disjoint (current, target)
