(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let elab_value = function
  | (annot, pos, Aast.Id _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), Some err)
  | expr -> (expr, None)

let elab_key = function
  | (_, _, Aast.(Lvar _ | Lplaceholder _)) as expr -> (expr, None)
  | (annot, pos, _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), Some err)

let on_as_expr on_error as_expr ~ctx =
  match as_expr with
  | Aast.As_v e ->
    let (e, err_opt) = elab_value e in
    Option.iter ~f:on_error err_opt;
    (ctx, Ok (Aast.As_v e))
  | Aast.Await_as_v (pos, e) ->
    let (e, err_opt) = elab_value e in
    Option.iter ~f:on_error err_opt;
    (ctx, Ok (Aast.Await_as_v (pos, e)))
  | Aast.As_kv (ke, ve) ->
    let (ke, key_err_opt) = elab_key ke in
    let (ve, val_err_opt) = elab_value ve in
    Option.iter ~f:on_error key_err_opt;
    Option.iter ~f:on_error val_err_opt;
    (ctx, Ok (Aast.As_kv (ke, ve)))
  | Aast.Await_as_kv (pos, ke, ve) ->
    let (ke, key_err_opt) = elab_key ke in
    let (ve, val_err_opt) = elab_value ve in
    Option.iter ~f:on_error key_err_opt;
    Option.iter ~f:on_error val_err_opt;
    (ctx, Ok (Aast.Await_as_kv (pos, ke, ve)))

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_as_expr = Some (fun elem ~ctx -> on_as_expr on_error elem ~ctx);
      }
