(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let on_expr :
      'a 'b.
      _ * ('a, 'b) Aast.expr ->
      (_ * ('a, 'b) Aast.expr, _ * ('a, 'b) Aast.expr) result =
 fun (env, expr) ->
  match expr with
  | (_, _, Aast.Import _) -> Error (env, Naming_phase_error.invalid_expr expr)
  | _ -> Ok (env, expr)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr = Some on_expr })
