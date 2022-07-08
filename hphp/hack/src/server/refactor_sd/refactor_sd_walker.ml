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

let stmt (env : env) ((pos, stmt) : T.stmt) : env =
  match stmt with
  | A.Noop -> env
  | _ -> failwithpos pos ("Unsupported statement: " ^ Utils.stmt_name stmt)

let block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let init_params _tast_env (params : T.fun_param list) :
    constraint_ list * entity LMap.t =
  let add_param (constraints, lmap) = function
    | _ -> (constraints, lmap)
  in
  List.fold ~f:add_param ~init:([], LMap.empty) params

let callable tast_env params body _function_id : constraint_ list =
  let (param_constraints, param_env) = init_params tast_env params in
  let env = Env.init tast_env param_constraints param_env in
  let env = block env body.A.fb_ast in
  env.constraints

let program
    (ctx : Provider_context.t) (tast : Tast.program) (function_id : string) :
    constraint_ list SMap.t =
  let def (def : T.def) : (string * constraint_ list) list =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_params; _ } = fd.A.fd_fun in
      [(id, callable tast_env f_params f_body function_id)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method A.{ m_body; m_name = (_, method_name); m_params; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable tast_env m_params m_body function_id)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
