(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let elab_value err_acc = function
  | (annot, pos, Aast.Id _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), err :: err_acc)
  | expr -> (expr, err_acc)

let elab_key err_acc = function
  | (_, _, Aast.(Lvar _ | Lplaceholder _)) as expr -> (expr, err_acc)
  | (annot, pos, _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), err :: err_acc)

let on_as_expr (env, as_expr, err_acc) =
  let (as_expr, err_acc) =
    match as_expr with
    | Aast.As_v e ->
      let (e, err_acc) = elab_value err_acc e in
      (Aast.As_v e, err_acc)
    | Aast.Await_as_v (pos, e) ->
      let (e, err_acc) = elab_value err_acc e in
      (Aast.Await_as_v (pos, e), err_acc)
    | Aast.As_kv (ke, ve) ->
      let (ke, err_acc) = elab_key err_acc ke in
      let (ve, err_acc) = elab_value err_acc ve in
      (Aast.As_kv (ke, ve), err_acc)
    | Aast.Await_as_kv (pos, ke, ve) ->
      let (ke, err_acc) = elab_key err_acc ke in
      let (ve, err_acc) = elab_value err_acc ve in
      (Aast.Await_as_kv (pos, ke, ve), err_acc)
  in
  Ok (env, as_expr, err_acc)

let pass =
  Naming_phase_pass.(bottom_up { identity with on_as_expr = Some on_as_expr })
