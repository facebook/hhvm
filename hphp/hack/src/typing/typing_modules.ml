(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Typing_env
module Cls = Folded_class

let can_access_internal
    ~(env : Typing_env_types.env)
    ~(current : string option)
    ~(target : string option) =
  match (current, target) with
  | (None, None)
  | (Some _, None) ->
    `Yes
  | (None, Some m) -> `Outside m
  | (Some m_current, Some m_target) when String.equal m_current m_target ->
    (match Env.get_self_id env with
    | None -> `Yes
    | Some self ->
      (match Env.get_class env self with
      | Decl_entry.Found cls
        when Ast_defs.is_c_trait (Cls.kind cls)
             && (not (Cls.internal cls))
             && not (Cls.is_module_level_trait cls) ->
        `OutsideViaTrait (Cls.pos cls)
      | Decl_entry.Found _
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        `Yes))
  | (Some current, Some target) -> `Disjoint (current, target)

let is_class_visible (env : Typing_env_types.env) (cls : Cls.t) =
  if Cls.internal cls then
    match
      can_access_internal
        ~env
        ~current:(Env.get_current_module env)
        ~target:(Cls.get_module cls)
    with
    | `Yes -> true
    | _ -> false
  else
    true
