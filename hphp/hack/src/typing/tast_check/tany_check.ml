(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let error_codes = Typing_warning_utils.codes Typing_warning.Tany_found

(** Returns [true] when [ty] contains a [Tany] member (possibly inside a union
    or intersection). *)
let rec has_genuine_tany env ty =
  let (env, ty) = Tast_env.expand_type env ty in
  match Typing_defs_core.get_node ty with
  | Typing_defs_core.Tany _ -> true
  | Typing_defs_core.Tunion [] -> false
  | Typing_defs_core.Tunion tyl
  | Typing_defs_core.Tintersection tyl ->
    List.exists tyl ~f:(has_genuine_tany env)
  | _ -> false

let handler ~as_lint:_ =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (ty, pos, _expr_) =
      let (env, ty) = Tast_env.expand_type env ty in
      if has_genuine_tany env ty then
        Tast_env.add_warning env (pos, Tany_found, ())
  end
