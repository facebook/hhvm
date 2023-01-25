(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let on_expr_ (env, expr_, errs) =
  match expr_ with
  | Aast.Invalid _ -> Error (env, expr_, errs)
  | _ -> Ok (env, expr_, errs)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr_ = Some on_expr_ })
