(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module SU = Hhbc_string_utils
module SN = Naming_special_names

open Core
open Instruction_sequence

let has_valid_access_modifiers kind_list =
  let count_of_modifiers = List.fold_right kind_list
    ~f:(fun x acc ->
        if x = Ast.Private || x = Ast.Public || x = Ast.Protected
        then acc + 1 else acc)
    ~init:0
  in
  (* Either only one modifier or none *)
  count_of_modifiers <= 1

let from_ast_wrapper : bool -> _ ->
  Ast.class_ -> Ast.method_ -> Hhas_method.t =
  fun privatize make_name ast_class ast_method ->
  if not (has_valid_access_modifiers ast_method.Ast.m_kind) then
    Emit_fatal.raise_fatal_parse Pos.none
      "Multiple access type modifiers are not allowed";
  let method_is_abstract =
    List.mem ast_method.Ast.m_kind Ast.Abstract ||
    ast_class.Ast.c_kind = Ast.Cinterface in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let namespace = ast_class.Ast.c_namespace in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.Ast.m_user_attributes in
  let method_is_private =
    privatize || List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected =
    not privatize && List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public =
    not privatize && (
    List.mem ast_method.Ast.m_kind Ast.Public ||
    (not method_is_private && not method_is_protected)) in
  let is_memoize =
    Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let deprecation_info = Hhas_attribute.deprecation_info method_attributes in
  let (pos, original_name) = ast_method.Ast.m_name in
  let (_, class_name) = ast_class.Ast.c_name in
  let class_name = SU.Xhp.mangle @@ Utils.strip_ns class_name in
  let ret = ast_method.Ast.m_ret in
  let method_id = make_name ast_method.Ast.m_name in
  let method_is_async =
    ast_method.Ast.m_fun_kind = Ast_defs.FAsync
    || ast_method.Ast.m_fun_kind = Ast_defs.FAsyncGenerator in
  if ast_class.Ast.c_kind = Ast.Cinterface && not (List.is_empty ast_method.Ast.m_body)
  then Emit_fatal.raise_fatal_parse pos
    (Printf.sprintf "Interface method %s::%s cannot contain body" class_name original_name);
  if not method_is_static
    && ast_class.Ast.c_final
    && ast_class.Ast.c_kind = Ast.Cabstract
  then Emit_fatal.raise_fatal_parse pos
    ("Class " ^ class_name ^ " contains non-static method " ^ original_name
     ^ " and therefore cannot be declared 'abstract final'");
 (* TODO: use something that can't be faked in user code *)
 let method_is_closure_body =
    original_name = "__invoke"
    && String_utils.string_starts_with class_name "Closure$" in
  if not (method_is_static || method_is_closure_body) then
    List.iter ast_method.Ast.m_params (fun p ->
      let pos, id = p.Ast.param_id in
      if id = SN.SpecialIdents.this then
      Emit_fatal.raise_fatal_parse pos "Cannot re-assign $this");
  (* Restrictions on __construct methods with promoted parameters *)
  if original_name = Naming_special_names.Members.__construct
  && List.exists ast_method.Ast.m_params (fun p -> Option.is_some p.Ast.param_modifier)
  then begin
    List.iter ast_method.Ast.m_params (fun p ->
      if List.length (
        List.filter ast_class.Ast.c_body
        (function
         | Ast.ClassVars (_, _, cvl, _) ->
             List.exists cvl (fun (_,id,_) ->
               snd id = SU.Locals.strip_dollar (snd p.Ast.param_id))
         | _ -> false)) > 1
      then Emit_fatal.raise_fatal_parse pos
        (Printf.sprintf "Cannot redeclare %s::%s" class_name (snd p.Ast.param_id)));
    if ast_class.Ast.c_kind = Ast.Cinterface || ast_class.Ast.c_kind = Ast.Ctrait
    then Emit_fatal.raise_fatal_parse pos
      "Constructor parameter promotion not allowed on traits or interfaces";
    if List.mem ast_method.Ast.m_kind Ast.Abstract
    then Emit_fatal.raise_fatal_parse pos
      "parameter modifiers not allowed on abstract __construct";
    if not (List.is_empty ast_method.Ast.m_tparams)
    then Emit_fatal.raise_fatal_parse pos
      "parameter modifiers not supported with type variable annotation"
  end;
  let default_dropthrough =
    if List.mem ast_method.Ast.m_kind Ast.Abstract
    then Some (Emit_fatal.emit_fatal_runtimeomitframe pos
      ("Cannot call abstract method " ^ class_name
        ^ "::" ^ original_name ^ "()"))
    else None
  in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  let scope =
    if method_is_closure_body
    then Ast_scope.ScopeItem.Lambda :: scope else scope in
  let closure_namespace = SMap.get class_name (Emit_env.get_closure_namespaces ()) in
  let namespace = Option.value closure_namespace ~default:namespace in
  let method_body, method_is_generator, method_is_pair_generator =
    Emit_body.emit_body
      ~pos:ast_method.Ast.m_span
      ~scope:scope
      ~is_closure_body:method_is_closure_body
      ~is_memoize
      ~is_async:method_is_async
      ~deprecation_info
      ~skipawaitable:(ast_method.Ast.m_fun_kind = Ast_defs.FAsync)
      ~is_return_by_ref:ast_method.Ast.m_ret_by_ref
      ~default_dropthrough
      ~return_value:instr_null
      ~namespace
      ~doc_comment:ast_method.Ast.m_doc_comment
      ast_method.Ast.m_params
      ret
      [Ast.Stmt (Ast.Block ast_method.Ast.m_body)]
  in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    false (*method_no_injection*)
    method_id
    method_body
    (Hhas_pos.pos_to_span ast_method.Ast.m_span)
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_ast ast_class ast_method =
  let is_memoize = Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let make_name (_, name) =
    if is_memoize
    then Hhbc_id.Method.from_ast_name (name ^ Emit_memoize_helpers.memoize_suffix)
    else Hhbc_id.Method.from_ast_name name
  in
  from_ast_wrapper is_memoize make_name ast_class ast_method


let from_asts ast_class ast_methods =
  List.map ast_methods (from_ast ast_class)
