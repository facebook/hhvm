(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_expr :
      'a 'b.
      _ * ('a, 'b) Aast.expr * Err.t list ->
      ( _ * ('a, 'b) Aast.expr * Err.t list,
        _ * ('a, 'b) Aast.expr * Err.t list )
      result =
 fun (env, expr, err_acc) ->
  match expr with
  | (_, pos, Aast.Tuple []) ->
    let err = Err.naming @@ Naming_error.Too_few_arguments pos in
    Error (env, Err.invalid_expr expr, err :: err_acc)
  | _ -> Ok (env, expr, err_acc)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr = Some on_expr })
