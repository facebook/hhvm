(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

let on_stmt on_error stmt ~ctx =
  let res =
    match stmt with
    | ( pos,
        Aast.(
          Expr
            ( annot,
              expr_pos,
              Call
                ({
                   func = (fn_annot, fn_expr_pos, Id (fn_name_pos, fn_name));
                   args;
                   _;
                 } as call_expr) )) )
      when String.equal fn_name SN.AutoimportedFunctions.invariant ->
      (match args with
      | [] ->
        let err = Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos in
        let expr = (annot, fn_expr_pos, Err.invalid_expr_ None) in
        Error ((pos, Aast.Expr expr), err)
      | [(_, expr)] ->
        let err = Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos in
        let expr = Err.invalid_expr expr in
        Error ((pos, Aast.Expr expr), err)
      | (pk, (cond_annot, cond_pos, cond)) :: exprs ->
        let err_opt =
          match pk with
          | Ast_defs.Pnormal -> None
          | Ast_defs.Pinout pk_p ->
            Some
              (Err.nast_check
              @@ Nast_check_error.Inout_in_transformed_pseudofunction
                   { pos = Pos.merge pk_p fn_expr_pos; fn_name = "invariant" })
        in
        let id_expr =
          Aast.Id (fn_name_pos, SN.AutoimportedFunctions.invariant_violation)
        in
        let fn_expr = (fn_annot, fn_expr_pos, id_expr) in
        let violation =
          ( annot,
            expr_pos,
            Aast.(Call { call_expr with func = fn_expr; args = exprs }) )
        in
        (match cond with
        | Aast.False ->
          (* a false <condition> means unconditional invariant_violation *)
          Ok ((pos, Aast.Expr violation), err_opt)
        | _ ->
          let (b1, b2) =
            ([(expr_pos, Aast.Expr violation)], [(Pos.none, Aast.Noop)])
          in
          let cond =
            ( cond_annot,
              cond_pos,
              Aast.Unop (Ast_defs.Unot, (cond_annot, cond_pos, cond)) )
          in
          Ok ((pos, Aast.If (cond, b1, b2)), err_opt)))
    | _ -> Ok (stmt, None)
  in
  match res with
  | Ok (stmt, err_opt) ->
    Option.iter ~f:on_error err_opt;
    (ctx, Ok stmt)
  | Error (stmt, err) ->
    on_error err;
    (ctx, Error stmt)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_stmt = Some (fun elem ~ctx -> on_stmt on_error elem ~ctx);
      }
