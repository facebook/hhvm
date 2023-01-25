(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let on_expr (env, expr, err_acc) =
  match expr with
  | (_, _, Aast.Import _) ->
    Error (env, Naming_phase_error.invalid_expr expr, err_acc)
  | _ -> Ok (env, expr, err_acc)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr = Some on_expr })
