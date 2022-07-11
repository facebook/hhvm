(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Refactor_sd_types
module A = Aast
module T = Tast
module Env = Refactor_sd_env
module Utils = Aast_names_utils

let failwithpos pos msg =
  raise @@ Refactor_sd_exn (Format.asprintf "%a: %s" Pos.pp pos msg)

let redirect (env : env) (entity_ : entity_) : env * entity_ =
  let var = Env.fresh_var () in
  let env = Env.add_constraint env (Subset (entity_, var)) in
  (env, var)

let rec expr_ (upcasted_id : string) (env : env) ((_ty, pos, e) : T.expr) :
    env * entity =
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.True
  | A.False ->
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.Binop (Ast_defs.Eq None, e1, ((_ty_rhs, _, _) as e2)) ->
    let (env, entity_rhs) = expr_ upcasted_id env e2 in
    let (_, pos, lval) = e1 in
    let env =
      match lval with
      | A.Lvar (_, lid) -> Env.set_local env lid entity_rhs
      | _ -> failwithpos pos ("Unsupported lvalue: " ^ Utils.expr_name lval)
    in
    (env, None)
  | A.Upcast (e, _) ->
    let (env, entity) = expr_ upcasted_id env e in
    let env =
      match entity with
      | Some entity -> Env.add_constraint env (Upcast entity)
      | None -> env
    in
    (env, None)
  | Aast.FunctionPointer (Aast.FP_id (_, id), _) ->
    if String.equal upcasted_id id then
      let entity_ = Literal pos in
      let env = Env.add_constraint env (Introduction pos) in
      (* Handle copy-on-write by creating a variable indirection *)
      let (env, var) = redirect env entity_ in
      (env, Some var)
    else
      (env, None)
  | A.Await e -> expr_ upcasted_id env e
  | A.As (e, _ty, _) -> expr_ upcasted_id env e
  | A.Is (e, _ty) ->
    (* `is` expressions always evaluate to bools, so we discard the entity. *)
    let (env, _) = expr_ upcasted_id env e in
    (env, None)
  | _ -> failwithpos pos ("Unsupported expression: " ^ Utils.expr_name e)

let expr (upcasted_id : string) (env : env) (e : T.expr) : env =
  expr_ upcasted_id env e |> fst

let stmt (upcasted_id : string) (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Expr e -> expr upcasted_id env e
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> failwithpos pos ("Unsupported statement: " ^ Utils.stmt_name stmt)

let block (env : env) (upcasted_id : string) : T.block -> env =
  List.fold ~init:env ~f:(stmt upcasted_id)

let init_params _tast_env (params : T.fun_param list) :
    constraint_ list * entity LMap.t =
  let add_param (constraints, lmap) = function
    | _ -> (constraints, lmap)
  in
  List.fold ~f:add_param ~init:([], LMap.empty) params

let callable function_id tast_env params body : constraint_ list =
  let (param_constraints, param_env) = init_params tast_env params in
  let env = Env.init tast_env param_constraints param_env in
  let env = block env function_id body.A.fb_ast in
  env.constraints

let program
    (function_id : string) (ctx : Provider_context.t) (tast : Tast.program) :
    constraint_ list SMap.t =
  let def (def : T.def) : (string * constraint_ list) list =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_params; _ } = fd.A.fd_fun in
      [(id, callable function_id tast_env f_params f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method A.{ m_body; m_name = (_, method_name); m_params; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable function_id tast_env m_params m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
