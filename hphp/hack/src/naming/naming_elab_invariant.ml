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

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_stmt (env, stmt, err_acc) =
  let res =
    match stmt with
    | ( pos,
        Aast.(
          Expr
            ( _,
              expr_pos,
              Call
                ( (_, fn_expr_pos, Id (fn_name_pos, fn_name)),
                  targs,
                  exprs,
                  unpacked_element ) )) )
      when String.equal fn_name SN.AutoimportedFunctions.invariant ->
      (match exprs with
      | []
      | [_] ->
        let err = Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos in
        let expr = ((), fn_expr_pos, Err.invalid_expr_ fn_expr_pos) in
        Error ((pos, Aast.Expr expr), err)
      | (pk, (_, cond_pos, cond)) :: exprs ->
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
        let fn_expr = ((), fn_expr_pos, id_expr) in
        let violation =
          ((), expr_pos, Aast.Call (fn_expr, targs, exprs, unpacked_element))
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
            ((), cond_pos, Aast.Unop (Ast_defs.Unot, ((), cond_pos, cond)))
          in
          Ok ((pos, Aast.If (cond, b1, b2)), err_opt)))
    | _ -> Ok (stmt, None)
  in
  match res with
  | Ok (stmt, err_opt) ->
    let err =
      Option.value_map
        err_opt
        ~default:err_acc
        ~f:(Err.Free_monoid.plus err_acc)
    in
    Naming_phase_pass.Cont.next (env, stmt, err)
  | Error (stmt, err) ->
    let err = Err.Free_monoid.plus err_acc err in
    Naming_phase_pass.Cont.finish (env, stmt, err)

let pass = Naming_phase_pass.(top_down { identity with on_stmt = Some on_stmt })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
