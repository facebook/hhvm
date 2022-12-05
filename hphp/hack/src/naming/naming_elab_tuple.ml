(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_expr (env, expr, err_acc) =
  let res =
    match expr with
    | (annot, pos, Aast.Tuple []) ->
      let err = Err.naming @@ Naming_error.Too_few_arguments pos in
      Error ((annot, pos, Err.invalid_expr_ pos), err)
    | _ -> Ok expr
  in
  match res with
  | Ok expr -> Naming_phase_pass.Cont.next (env, expr, err_acc)
  | Error (expr, err) ->
    Naming_phase_pass.Cont.finish (env, expr, Err.Free_monoid.plus err_acc err)

let pass = Naming_phase_pass.(top_down { identity with on_expr = Some on_expr })
