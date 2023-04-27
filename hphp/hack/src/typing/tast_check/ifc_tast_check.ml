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

let options : options =
  {
    opt_mode = Ifc_types.Mcheck;
    (* TODO: support non default security lattices *)
    opt_security_lattice = FlowSet.empty;
  }

let catch_ifc_internal_errors pos f =
  try
    Timeout.with_timeout
      ~timeout:5
      ~do_:(fun _ -> f ())
      ~on_timeout:(fun _ ->
        Hh_logger.log
          "Timed out running IFC analysis on %s"
          (Relative_path.suffix (Pos.filename pos));
        Errors.add_typing_error
          Typing_error.(
            ifc
            @@ Primary.Ifc.Ifc_internal_error
                 { pos; msg = "Timed out running IFC analysis" }))
  with
  (* Solver exceptions*)
  | IFCError error ->
    Errors.add_typing_error
      Typing_error.(
        ifc
        @@ Primary.Ifc.Ifc_internal_error
             { pos; msg = Ifc_pretty.ifc_error_to_string error })
  (* Failwith exceptions *)
  | Failure s ->
    Errors.add_typing_error
      Typing_error.(
        ifc
        @@ Primary.Ifc.Ifc_internal_error
             { pos; msg = "IFC internal assertion failure: " ^ s })
  | e ->
    Errors.add_typing_error
      Typing_error.(
        ifc
        @@ Primary.Ifc.Ifc_internal_error
             { pos; msg = "Unexpected IFC exception: " ^ Exn.to_string e })

let decl_env : decl_env = { de_class = SMap.empty }

let check_errors_from_callable_result result =
  match result with
  | Some r ->
    let results = Ifc.get_solver_result [r] in
    let simplified_results = SMap.map Ifc.simplify results in
    SMap.iter (Ifc.check_valid_flow options) simplified_results
  | None -> ()

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
       _;
     } :
      Tast.method_) : unit =
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
let handle_fun (ctx : Provider_context.t) (fd : Tast.fun_def) =
  let (_, name) = fd.fd_name in
  let {
    f_annotation = saved_env;
    f_params = params;
    f_body = body;
    f_ret = (return, _);
    f_span = pos;
    _;
  } =
    fd.fd_fun
  in
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

let ifc_enabled_on_file tcopt file =
  let ifc_enabled_paths = TypecheckerOptions.ifc_enabled tcopt in
  let path = "/" ^ Relative_path.suffix file in
  List.exists ifc_enabled_paths ~f:(fun prefix -> String.is_prefix path ~prefix)

let should_run_ifc tcopt file =
  match
    ( ifc_enabled_on_file tcopt file,
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
      if should_run_ifc (Tast_env.get_tcopt env) (Tast_env.get_file env) then
        let ctx : Provider_context.t = Tast_env.get_ctx env in
        let (_, classname) = c.c_name in
        let methods = c.c_methods in
        List.iter ~f:(handle_method classname ctx) methods

    method! at_fun_def env f =
      if should_run_ifc (Tast_env.get_tcopt env) (Tast_env.get_file env) then
        let ctx : Provider_context.t = Tast_env.get_ctx env in
        handle_fun ctx f
  end
