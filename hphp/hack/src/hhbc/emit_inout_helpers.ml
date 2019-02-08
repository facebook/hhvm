(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel

module H = Hhbc_ast

type wrapper_type = InoutWrapper | RefWrapper

let extract_inout_or_ref_param_locations ~is_sync ~is_closure_or_func params =
  let inout_param_locations = List.filter_mapi params
    ~f:(fun i p -> if p.Ast.param_callconv <> Some Ast.Pinout
                   then None else Some i) in
  if List.length inout_param_locations <> 0 then
    Some InoutWrapper, inout_param_locations
  else
    if not is_sync
    then None, []
    else
    let module O = Hhbc_options in
    let need_wrapper =
      O.create_inout_wrapper_functions !O.compiler_options
      && (Emit_env.is_hh_syntax_enabled ())
      && (O.reffiness_invariance !O.compiler_options || is_closure_or_func)
      && not @@ List.exists params
        ~f:(fun p -> p.Ast.param_is_variadic && p.Ast.param_is_reference) in
    let l =
      if need_wrapper
      then List.filter_mapi params ~f:(fun i p -> Option.some_if p.Ast.param_is_reference i)
      else [] in
    if List.is_empty l then None, []
    else Some RefWrapper, l

let extract_function_inout_or_ref_param_locations fd =
  let is_sync = fd.Ast.f_fun_kind = Ast.FSync in
  extract_inout_or_ref_param_locations
    ~is_closure_or_func:true
    ~is_sync
    fd.Ast.f_params

let extract_method_inout_or_ref_param_locations md ~is_closure_or_func =
  let is_sync = md.Ast.m_fun_kind = Ast.FSync in
  extract_inout_or_ref_param_locations
    ~is_closure_or_func
    ~is_sync
    md.Ast.m_params

let inout_suffix param_location =
  let param_location = List.map ~f:string_of_int param_location in
  "$"
  ^ (String.concat ~sep:";" param_location)
  ^ "$inout"
