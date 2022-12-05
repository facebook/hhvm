(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let on_expr (env, expr, err_acc) =
  let res =
    match expr with
    | (annot, pos, Aast.Tuple []) ->
      let err =
        Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos
      in
      Error ((annot, pos, Naming_phase_error.invalid_expr_ pos), err)
    | _ -> Ok expr
  in
  match res with
  | Ok expr -> Ok (env, expr, err_acc)
  | Error (expr, err) -> Error (env, expr, err :: err_acc)

let pass = Naming_phase_pass.(top_down { identity with on_expr = Some on_expr })
