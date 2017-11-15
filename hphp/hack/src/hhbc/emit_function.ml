(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence
module A = Ast
open Hh_core

let find_first_redeclaration pick_name_span l =
  let rec aux seen l =
    match l with
    | [] -> None
    | x :: xs ->
      match pick_name_span x with
      | Some (name, span) ->
        begin match SMap.get name seen with
        | None -> aux (SMap.add name span seen) xs
        | Some original -> Some (name, original, span)
        end
      | None -> aux seen xs in
  aux SMap.empty l

(* Given a function definition, emit code, and in the case of <<__Memoize>>,
 * a wrapper function
 *)
let emit_function : A.fun_ * bool -> Hhas_function.t list =
  fun (ast_fun, is_top) ->
  let namespace = ast_fun.A.f_namespace in
  let original_id, _ =
    Hhbc_id.Function.elaborate_id namespace ast_fun.A.f_name in
  let function_is_async =
    ast_fun.Ast.f_fun_kind = Ast_defs.FAsync
    || ast_fun.Ast.f_fun_kind = Ast_defs.FAsyncGenerator in
  let function_attributes =
    Emit_attribute.from_asts namespace ast_fun.Ast.f_user_attributes in
  let is_memoize = Hhas_attribute.is_memoized function_attributes in
  let deprecation_info = Hhas_attribute.deprecation_info function_attributes in
  let inout_param_locations = List.filter_mapi ast_fun.Ast.f_params
      ~f:(fun i p -> if p.Ast.param_callconv <> Some Ast.Pinout
                     then None else Some (string_of_int i)) in
  let has_inout_args = List.length inout_param_locations <> 0 in
  let renamed_id =
    if is_memoize
    then Hhbc_id.Function.add_suffix
      original_id Emit_memoize_helpers.memoize_suffix
    else if has_inout_args
    then Hhbc_id.Function.add_suffix
      original_id (Emit_inout_helpers.inout_suffix inout_param_locations)
    else original_id in
  let scope = [Ast_scope.ScopeItem.Function ast_fun] in
  let function_body, function_is_generator, function_is_pair_generator =
    Emit_body.emit_body
      ~pos: ast_fun.A.f_span
      ~scope
      ~is_closure_body:false
      ~is_memoize
      ~is_async:function_is_async
      ~deprecation_info:(if is_memoize then None else deprecation_info)
      ~skipawaitable:(ast_fun.Ast.f_fun_kind = Ast_defs.FAsync)
      ~is_return_by_ref:ast_fun.Ast.f_ret_by_ref
      ~default_dropthrough:None
      ~return_value:instr_null
      ~namespace
      ~doc_comment:ast_fun.Ast.f_doc_comment
      ast_fun.Ast.f_params
      ast_fun.Ast.f_ret
      [Ast.Stmt (Ast.Block ast_fun.Ast.f_body)] in
  let normal_function =
    Hhas_function.make
      function_attributes
      renamed_id
      function_body
      (Hhas_pos.pos_to_span ast_fun.Ast.f_span)
      function_is_async
      function_is_generator
      function_is_pair_generator
      is_top
      false (*no_injection*)
      false (*inout_wrapper*)
  in
  if is_memoize
  then [normal_function;
    Emit_memoize_function.emit_wrapper_function
      ~original_id
      ~renamed_id
      ~is_method:false
      ~deprecation_info
      ast_fun]
  else if has_inout_args
  then [Emit_inout_function.emit_wrapper_function
          ~original_id
          ~renamed_id
          ~verify_ret:(ast_fun.Ast.f_ret <> None)
          ast_fun;
        normal_function]
  else [normal_function]

let emit_functions_from_program ast =
  List.concat_map (List.sort (fun (t1, _) (t2, _) -> compare t1 t2) ast)
  (fun (is_top, d) ->
    match d with Ast.Fun fd -> emit_function (fd, is_top) | _ -> [])
