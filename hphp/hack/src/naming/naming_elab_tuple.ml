(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_expr on_error =
  let handler
        : 'a 'b.
          _ * ('a, 'b) Aast.expr ->
          (_ * ('a, 'b) Aast.expr, _ * ('a, 'b) Aast.expr) result =
   fun (env, expr) ->
    match expr with
    | (_, pos, Aast.Tuple []) ->
      on_error (Err.naming @@ Naming_error.Too_few_arguments pos);
      Error (env, Err.invalid_expr expr)
    | _ -> Ok (env, expr)
  in
  handler

let pass on_error =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr = Some (on_expr on_error) })
