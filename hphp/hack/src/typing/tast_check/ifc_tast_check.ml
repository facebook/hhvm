(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Ifc_types
open Aast
open Base
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names

let options : options =
  {
    opt_mode = Ifc_types.Mcheck;
    (* TODO: support non default security lattices *)
    opt_security_lattice = FlowSet.empty;
  }

let catch_ifc_internal_errors pos f =
  try f () with
  (* Solver exceptions*)
  | IFCError error ->
    Errors.ifc_internal_error pos (Ifc_pretty.ifc_error_to_string error)
  (* Failwith exceptions *)
  | Failure s ->
    Errors.ifc_internal_error pos ("IFC internal assertion failure: " ^ s)
  | e ->
    Errors.ifc_internal_error
      pos
      ("Unexpected IFC exception: " ^ Exn.to_string e)

let decl_env : decl_env = { de_class = SMap.empty }

let check_errors_from_callable_result result =
  let open Ifc in
  match result with
  | Some r ->
    let results = get_solver_result [r] in
    let simplified_results = SMap.map simplify results in
    SMap.iter (check_valid_flow options) simplified_results
  | None -> ()

let has_attr (name : string) (attrs : Tast.user_attribute list) : bool =
  let matches_name { ua_name = (_, attr_name); _ } =
    String.equal attr_name name
  in
  List.exists ~f:matches_name attrs

(* We only run IFC checks on methods that have policies. Funs and methods without an explicit policy
  are assumed to have the PUBLIC policy by default, but are not checked by IFC. *)
let has_policy attrs = has_attr SN.UserAttributes.uaPolicied attrs

(* Run IFC on a single method, catching exceptions *)
let handle_method
    (class_name : string)
    (ctx : Provider_context.t)
    ({
       m_name = (_, name);
       m_annotation = saved_env;
       m_params = params;
       m_body = body;
       m_ret = (return, _);
       m_span = pos;
       m_static = is_static;
       m_user_attributes = attrs;
       _;
     } :
      Tast.method_) : unit =
  if has_policy attrs then
    catch_ifc_internal_errors pos (fun () ->
        Ifc.analyse_callable
          ~opts:options
          ~class_name
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          ~ctx
          name
          params
          body
          return
        |> check_errors_from_callable_result)

(* Run IFC on a single toplevel function, catching exceptions *)
let handle_fun
    (ctx : Provider_context.t)
    ({
       f_name = (_, name);
       f_annotation = saved_env;
       f_params = params;
       f_body = body;
       f_ret = (return, _);
       f_span = pos;
       f_user_attributes = attrs;
       _;
     } :
      Tast.fun_) =
  if has_policy attrs then
    catch_ifc_internal_errors pos (fun () ->
        Ifc.analyse_callable
          ~opts:options
          ~pos
          ~decl_env
          ~is_static:false
          ~saved_env
          ~ctx
          name
          params
          body
          return
        |> check_errors_from_callable_result)

let should_run_ifc tcopt =
  match
    ( TypecheckerOptions.experimental_feature_enabled
        tcopt
        TypecheckerOptions.experimental_ifc,
      TypecheckerOptions.experimental_feature_enabled
        tcopt
        TypecheckerOptions.experimental_infer_flows )
  with
  | (true, false) -> true
  (* If inferflows is allowed, IFC tast check won't work properly
    since there can be global inference. We're in IFC mode anyways,
    so don't run it here. *)
  | (true, true) -> false
  | _ -> false

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      if should_run_ifc (Tast_env.get_tcopt env) then
        let ctx : Provider_context.t = Tast_env.get_ctx env in
        let (_, classname) = c.c_name in
        let methods = c.c_methods in
        List.iter ~f:(handle_method classname ctx) methods

    method! at_fun_ env f =
      if should_run_ifc (Tast_env.get_tcopt env) then
        let ctx : Provider_context.t = Tast_env.get_ctx env in
        handle_fun ctx f
  end
