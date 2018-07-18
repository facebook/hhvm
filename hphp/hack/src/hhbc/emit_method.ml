(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

module SU = Hhbc_string_utils
module SN = Naming_special_names

open Hh_core
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

let rec hint_uses_tparams tparam_names (_, hint)  =
  match hint with
  | Ast.Hsoft h  | Ast.Hoption h ->
    hint_uses_tparams tparam_names h
  | Ast.Hfun (_, ps, _, _, r) ->
    List.exists ps ~f:(hint_uses_tparams tparam_names) ||
    hint_uses_tparams tparam_names r
  | Ast.Htuple ts ->
    List.exists ts ~f:(hint_uses_tparams tparam_names)
  | Ast.Happly ((_, h), ts) ->
    SSet.mem h tparam_names ||
    List.exists ts ~f:(hint_uses_tparams tparam_names)
  | Ast.Hshape { Ast.si_shape_field_list = _l; _ } ->
    (* HHVM currenly does not report errors if constructor type parameter
       is captured in shape field type annotation *)
    (*
    List.exists l ~f:(fun { Ast.sf_hint = h; _ } ->
      hint_uses_tparams tparam_names h)
    *)
    false
  | Ast.Haccess ((_, n), _, _) -> SSet.mem n tparam_names

(* Extracts inout params
 * Or ref params only if the function is a closure
 *)
let extract_inout_or_ref_param_locations is_closure_or_func md  =
  let _, l =
    Emit_inout_helpers.extract_method_inout_or_ref_param_locations
      ~is_closure_or_func md in
  l

let has_kind m k = List.mem m.Ast.m_kind k

let from_ast_wrapper : bool -> _ ->
  Ast.class_ -> Ast.method_ -> Hhas_method.t list =
  fun privatize make_name ast_class ast_method ->
  if not (has_valid_access_modifiers ast_method.Ast.m_kind) then
    Emit_fatal.raise_fatal_parse Pos.none
      "Multiple access type modifiers are not allowed";
  let namespace = ast_class.Ast.c_namespace in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.Ast.m_user_attributes in
  let is_native = Hhas_attribute.has_native method_attributes in
  let is_native_opcode_impl =
    Hhas_attribute.is_native_opcode_impl method_attributes in
  let method_is_abstract =
    ast_class.Ast.c_kind = Ast.Cinterface ||
    has_kind ast_method Ast.Abstract in
  let method_is_final = has_kind ast_method Ast.Final in
  let method_is_static = has_kind ast_method Ast.Static in
  let method_is_private =
    not is_native_opcode_impl &&
    (privatize || has_kind ast_method Ast.Private) in
  let method_is_protected =
    not is_native_opcode_impl &&
    not privatize &&
    has_kind ast_method Ast.Protected in
  let method_is_public =
    is_native_opcode_impl ||
    (not privatize && (
    has_kind ast_method Ast.Public ||
    (not method_is_private && not method_is_protected))) in
  let is_memoize =
    Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let deprecation_info =
    if is_memoize then None else Hhas_attribute.deprecation_info method_attributes
  in
  let is_no_injection = Hhas_attribute.is_no_injection method_attributes in
  let (pos, original_name) = ast_method.Ast.m_name in
  let (_, class_name) = ast_class.Ast.c_name in
  let class_name = SU.Xhp.mangle @@ Utils.strip_ns class_name in
  let ret = ast_method.Ast.m_ret in
  let original_method_id = make_name ast_method.Ast.m_name in
  (* TODO: use something that can't be faked in user code *)
  let method_is_closure_body =
     original_name = "__invoke"
     && String_utils.string_starts_with class_name "Closure$" in
   if not (method_is_static || method_is_closure_body) then
     List.iter ast_method.Ast.m_params (fun p ->
       let pos, id = p.Ast.param_id in
       if id = SN.SpecialIdents.this then
       Emit_fatal.raise_fatal_parse pos "Cannot re-assign $this");
  let inout_param_locations =
    extract_inout_or_ref_param_locations method_is_closure_body ast_method in
  let has_inout_args = List.length inout_param_locations <> 0 in
  let renamed_method_id = if has_inout_args then
    Hhbc_id.Method.from_ast_name @@
      (snd ast_method.Ast.m_name)
      ^ (Emit_inout_helpers.inout_suffix inout_param_locations)
    else original_method_id in
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
  (* Restrictions on __construct methods with promoted parameters *)
  let has_param_promotion = List.exists ast_method.Ast.m_params
    (fun p -> Option.is_some p.Ast.param_modifier)
  in
  if has_param_promotion then
    if original_name = Naming_special_names.Members.__construct
    then begin
      let tparam_names =
        List.fold_left ast_method.Ast.m_tparams
          ~init:SSet.empty
          ~f:(fun acc (_, (_, n), _, _) -> SSet.add n acc) in
      List.iter ast_method.Ast.m_params (fun p ->
        if List.length (
          List.filter ast_class.Ast.c_body
          (function
           | Ast.ClassVars { Ast.cv_names = cvl; _ } ->
               List.exists cvl (fun (_,id,_) ->
                 snd id = SU.Locals.strip_dollar (snd p.Ast.param_id))
           | _ -> false)) > 1
        then Emit_fatal.raise_fatal_parse pos
          (Printf.sprintf "Cannot redeclare %s::%s" class_name (snd p.Ast.param_id));
        (* Disallow method type parameters to be used as type annotations
           on promoted parameters *)
        (* TODO: move to parser errors *)
        if Option.is_some p.Ast.param_modifier
        then match p.Ast.param_hint with
        | Some h when hint_uses_tparams tparam_names h ->
          Emit_fatal.raise_fatal_parse pos
            "parameter modifiers not supported with type variable annotation"
        | _ -> ());

      if ast_class.Ast.c_kind = Ast.Cinterface || ast_class.Ast.c_kind = Ast.Ctrait
      then Emit_fatal.raise_fatal_parse pos
        "Constructor parameter promotion not allowed on traits or interfaces";
      if List.mem ast_method.Ast.m_kind Ast.Abstract
      then Emit_fatal.raise_fatal_parse pos
        "parameter modifiers not allowed on abstract __construct";
      end
    else
      (* not in __construct *)
      Emit_fatal.raise_fatal_parse
        pos "Parameters modifiers not allowed on methods";
  let has_variadic_param = List.exists ast_method.Ast.m_params
    (fun p -> p.Ast.param_is_variadic)
  in
  if has_variadic_param && original_name = Naming_special_names.Members.__call
  then Emit_fatal.raise_fatal_parse pos @@
    Printf.sprintf "Method %s::__call() cannot take a variadic argument" class_name;
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
  let is_return_by_ref = ast_method.Ast.m_ret_by_ref in
  let has_ref_params =
    List.exists ast_method.Ast.m_params ~f:(fun p -> p.Ast.param_is_reference) in
  let method_body, method_is_generator, method_is_pair_generator =
    if is_native_opcode_impl then
      Emit_native_opcode.emit_body
        scope
        namespace
        ast_class.A.c_user_attributes
        ast_method.Ast.m_name
        ast_method.Ast.m_params
        ret,
      false,
      false
    else
      Emit_body.emit_body
        ~pos:ast_method.Ast.m_span
        ~scope:scope
        ~is_closure_body:method_is_closure_body
        ~is_memoize
        ~is_native
        ~is_async:method_is_async
        ~deprecation_info
        ~skipawaitable:(ast_method.Ast.m_fun_kind = Ast_defs.FAsync)
        ~is_return_by_ref
        ~default_dropthrough
        ~return_value:instr_null
        ~namespace
        ~doc_comment:ast_method.Ast.m_doc_comment
        ast_method.Ast.m_params
        ret
        [Ast.Stmt (Pos.none, Ast.Block ast_method.Ast.m_body)]
  in
  let method_id =
    if has_inout_args && (method_is_closure_body || has_ref_params) then
    original_method_id else renamed_method_id in
  let method_is_interceptable =
    Interceptable.is_method_interceptable
      namespace ast_class original_method_id method_attributes in
  let method_span =
    if is_native_opcode_impl then (0, 0)
    else Hhas_pos.pos_to_span ast_method.Ast.m_span in

  let normal_function =
    Hhas_method.make
      method_attributes
      method_is_protected
      method_is_public
      method_is_private
      method_is_static
      method_is_final
      method_is_abstract
      is_no_injection
      false (*method_inout_wrapper*)
      method_id
      method_body
      method_span
      method_is_async
      method_is_generator
      method_is_pair_generator
      method_is_closure_body
      is_return_by_ref
      method_is_interceptable
      is_memoize (*method_is_memoize_impl*)
  in
  let decl_vars = Hhas_body.decl_vars @@ Hhas_method.body normal_function in
  if has_inout_args && not (method_is_abstract && has_ref_params)
  then
    let wrapper =
      Emit_inout_function.emit_wrapper_method
        ~is_closure:method_is_closure_body
        ~decl_vars
        ~original_id:original_method_id
        ~renamed_id:renamed_method_id
        ast_class ast_method in
      if method_is_closure_body || has_ref_params
      then [normal_function; wrapper]
      else [wrapper; normal_function]
  else [normal_function]


let from_ast ast_class ast_method =
  let is_memoize = Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let make_name (_, name) =
    if is_memoize
    then Hhbc_id.Method.from_ast_name (name ^ Emit_memoize_helpers.memoize_suffix)
    else Hhbc_id.Method.from_ast_name name
  in
  from_ast_wrapper is_memoize make_name ast_class ast_method


let from_asts ast_class ast_methods =
  List.concat_map ast_methods ~f:(from_ast ast_class)
