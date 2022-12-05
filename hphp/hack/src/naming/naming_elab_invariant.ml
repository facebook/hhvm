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

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    (* We have to visit the `stmt` since
       1) we need the top level position
       2) we elaborate invariant calls (expressions) to statements *)
    method! on_stmt env (pos, stmt_) =
      let (stmt_, err) =
        match stmt_ with
        | Aast.(
            Expr
              ( _,
                expr_pos,
                Call
                  ( (_, fn_expr_pos, Id (fn_name_pos, fn_name)),
                    targs,
                    exprs,
                    unpacked_element ) ))
          when String.equal fn_name SN.AutoimportedFunctions.invariant ->
          (match exprs with
          | []
          | [_] ->
            let err =
              Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
            in
            let expr = ((), fn_expr_pos, Err.invalid_expr_ fn_expr_pos) in
            (Aast.Expr expr, err)
          | (pk, (_, cond_pos, cond)) :: exprs ->
            let param_err =
              match pk with
              | Ast_defs.Pnormal -> self#zero
              | Ast_defs.Pinout pk_p ->
                Err.nast_check
                @@ Nast_check_error.Inout_in_transformed_pseudofunction
                     { pos = Pos.merge pk_p fn_expr_pos; fn_name = "invariant" }
            in
            let id_expr =
              Aast.Id (fn_name_pos, SN.AutoimportedFunctions.invariant_violation)
            in
            let fn_expr = ((), fn_expr_pos, id_expr) in
            let (exprs, exprs_err) =
              super#on_list (self#on_snd super#on_expr) env exprs
            in
            let err = self#plus exprs_err param_err in
            let violation =
              ((), expr_pos, Aast.Call (fn_expr, targs, exprs, unpacked_element))
            in
            (match cond with
            | Aast.False ->
              (* a false <condition> means unconditional invariant_violation *)
              (Aast.Expr violation, err)
            | _ ->
              let (b1, b2) =
                ([(expr_pos, Aast.Expr violation)], [(Pos.none, Aast.Noop)])
              in
              let cond =
                ((), cond_pos, Aast.Unop (Ast_defs.Unot, ((), cond_pos, cond)))
              in
              (Aast.If (cond, b1, b2), err)))
        | _ -> super#on_stmt_ env stmt_
      in
      ((pos, stmt_), err)

    method private on_snd f env (fst, snd) =
      let (snd, err) = f env snd in
      ((fst, snd), err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
