(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Instruction_sequence
open Hh_core
open Emit_memoize_helpers
open Hhbc_ast

(* Precomputed information required for generation of memoized methods *)
type memoize_info = {
  (* Prefix on method and property names *)
  memoize_class_prefix : string;

  (* Number of instance methods with <<__Memoize>> *)
  memoize_instance_method_count : int;

  (* True if the enclosing class is a trait *)
  memoize_is_trait : bool;

  (* Enclosing class ID *)
  memoize_class_id : Hhbc_id.Class.t;
}

let is_memoize ast_method =
  Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes

let make_info ast_class class_id ast_methods =
  let check_method ast_method =
    if is_memoize ast_method
    then
    let pos = fst ast_method.Ast.m_name in
    Emit_memoize_helpers.check_memoize_possible pos
      ~ret_by_ref: ast_method.Ast.m_ret_by_ref
      ~params: ast_method.Ast.m_params
      ~is_method:true;
    if ast_class.Ast.c_kind = Ast.Cinterface
    then Emit_fatal.raise_fatal_runtime pos
      "<<__Memoize>> cannot be used in interfaces"
    else if List.mem ast_method.Ast.m_kind Ast.Abstract
    then Emit_fatal.raise_fatal_parse pos
      ("Abstract method " ^ Hhbc_id.Class.to_raw_string class_id ^ "::" ^
      snd ast_method.Ast.m_name ^ " cannot be memoized")
  in
  List.iter ast_methods check_method;
  let is_trait = ast_class.Ast.c_kind = Ast.Ctrait in
  let class_prefix =
    if is_trait
    then "$" ^ String.lowercase_ascii (Hhbc_id.Class.to_raw_string class_id)
    else "" in
  let instance_count =
    List.count ast_methods (fun ast_method -> is_memoize ast_method &&
        not (List.mem ast_method.Ast.m_kind Ast.Static)) in
  {
    memoize_is_trait = is_trait;
    memoize_class_prefix = class_prefix;
    memoize_instance_method_count = instance_count;
    memoize_class_id = class_id;
  }

let get_cls_method info param_count method_id with_lsb =
  let method_id =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  if info.memoize_is_trait || with_lsb
  then
    instr_fpushclsmethodsd param_count SpecialClsRef.Self method_id
  else
    instr_fpushclsmethodd param_count method_id info.memoize_class_id

let make_memoize_instance_method_no_params_code
      scope deprecation_info method_id =
  let renamed_name =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let label = Label.Regular 0 in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  gather [
      deprecation_body;
      instr_checkthis;
      instr_memoget label None;
      instr_retc;
      instr_label label;
      instr_this;
      instr_fpushobjmethodd_nullthrows 0 renamed_name;
      instr_fcall 0 false 1;
      instr_unboxr;
      instr_memoset None;
      instr_retc;
    ]

(* md is the already-renamed memoize method that must be wrapped *)
let make_memoize_instance_method_with_params_code ~pos
  env scope deprecation_info method_id params =
  let renamed_name =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  let begin_label, default_value_setters =
    (* Default value setters belong in the
     * wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params ~should_emit_init_this:false;
    deprecation_body;
    instr_checkthis;
    param_code_sets params param_count;
    instr_memoget label (Some (first_local, param_count));
    instr_retc;
    instr_label label;
    instr_this;
    instr_fpushobjmethodd_nullthrows param_count renamed_name;
    param_code_gets params;
    instr_fcall param_count false 1;
    instr_unboxr;
    instr_memoset (Some (first_local, param_count));
    instr_retc;
    default_value_setters ]

let make_memoize_static_method_no_params_code
  info scope deprecation_info method_id with_lsb =
  let label = Label.Regular 0 in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  gather [
    deprecation_body;
    instr_memoget label None;
    instr_retc;
    instr_label label;
    get_cls_method info 0 method_id with_lsb;
    instr_fcall 0 false 1;
    instr_unboxr;
    instr_memoset None;
    instr_retc ]

let make_memoize_static_method_with_params_code ~pos
  env info scope deprecation_info method_id with_lsb params =
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  let begin_label, default_value_setters =
    (* Default value setters belong in the
     * wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params:params ~should_emit_init_this:false;
    deprecation_body;
    param_code_sets params param_count;
    instr_memoget label (Some (first_local, param_count));
    instr_retc;
    instr_label label;
    get_cls_method info param_count method_id with_lsb;
    param_code_gets params;
    instr_fcall param_count false 1;
    instr_unboxr;
    instr_memoset (Some (first_local, param_count));
    instr_retc;
    default_value_setters ]

let make_memoize_static_method_code ~pos env info scope deprecation_info method_id with_lsb params =
  if List.is_empty params then
    make_memoize_static_method_no_params_code info scope deprecation_info method_id with_lsb
  else
    make_memoize_static_method_with_params_code ~pos env info scope
                                                deprecation_info method_id with_lsb params

let make_memoize_instance_method_code
      ~pos env scope deprecation_info method_id params =
  if List.is_empty params
  then make_memoize_instance_method_no_params_code
         scope deprecation_info method_id
  else make_memoize_instance_method_with_params_code
         ~pos env scope deprecation_info method_id params

(* Construct the wrapper function *)
let make_wrapper env return_type params instrs with_lsb =
  Emit_body.make_body
    instrs
    [] (* decl_vars *)
    true (* is_memoize_wrapper *)
    with_lsb (* is_memoize_wrapper_lsb *)
    params
    (Some return_type)
    [] (* static_inits *)
    None (* doc *)
    (Some env)

let emit ~pos env info return_type_info scope
         deprecation_info params is_static method_id with_lsb =
  let instrs =
    Emit_pos.emit_pos_then pos @@
    if is_static
    then make_memoize_static_method_code
      ~pos env info scope deprecation_info method_id with_lsb params
    else make_memoize_instance_method_code
      ~pos env scope deprecation_info method_id params
  in
  make_wrapper env return_type_info params instrs with_lsb

let emit_memoize_wrapper_body env memoize_info ast_method
                              ~namespace scope deprecation_info params ret =
    let is_static =List.mem ast_method.Ast.m_kind Ast.Static in
    let tparams =
      Hh_core.List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _, _) -> s) in
    let return_type_info =
      Emit_body.emit_return_type_info ~scope ~skipawaitable:false ~namespace ret in
    let params =
      Emit_param.from_asts ~namespace ~tparams:tparams ~generate_defaults:true ~scope params in
    let pos = ast_method.Ast.m_span in
    let (_,original_name) = ast_method.Ast.m_name in
    let method_id =
      Hhbc_id.Method.from_ast_name original_name in
    let with_lsb =
      Emit_attribute.ast_any_is_memoize_lsb ast_method.Ast.m_user_attributes in
    emit ~pos env memoize_info return_type_info scope deprecation_info
         params is_static method_id with_lsb

let make_memoize_wrapper_method env info ast_class ast_method =
  (* This is cut-and-paste from emit_method above, with special casing for
   * wrappers *)
  let method_is_abstract =
    List.mem ast_method.Ast.m_kind Ast.Abstract in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let method_attributes =
    Emit_attribute.from_asts (Emit_env.get_namespace env) ast_method.Ast.m_user_attributes in
  let method_is_private =
    List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected =
    List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public =
    List.mem ast_method.Ast.m_kind Ast.Public ||
    (not method_is_private && not method_is_protected) in
  let (_, original_name) = ast_method.Ast.m_name in
  let ret =
    if original_name = Naming_special_names.Members.__construct
    || original_name = Naming_special_names.Members.__destruct
    then None
    else ast_method.Ast.m_ret in
  let method_is_return_by_ref = ast_method.Ast.m_ret_by_ref in
  let method_id = Hhbc_id.Method.from_ast_name original_name in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  let namespace = ast_class.Ast.c_namespace in
  let method_is_interceptable =
    Interceptable.is_method_interceptable namespace ast_class method_id method_attributes in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.Ast.m_user_attributes in
  let deprecation_info = Hhas_attribute.deprecation_info method_attributes in
  let method_body =
    emit_memoize_wrapper_body env info ast_method
    ~namespace scope deprecation_info ast_method.Ast.m_params ret in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    false (*method_no_injection*)
    false (*method_inout_wrapper*)
    method_id
    method_body
    (Hhas_pos.pos_to_span ast_method.Ast.m_span)
    false (*method_is_async*)
    false (*method_is_generator*)
    false (*method_is_pair_generator*)
    false (*method_is_closure_body*)
    method_is_return_by_ref
    method_is_interceptable
    false (*method_is_memoize_impl*)

let emit_wrapper_methods env info ast_class ast_methods =
  (* Wrapper methods may not have iterators *)
  Iterator.reset_iterator ();
  let _, hhas_methods =
    List.fold_left ast_methods ~init:(0, []) ~f:(fun (count,acc) ast_method ->
      if Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes
      then
        let scope = Ast_scope.ScopeItem.Method ast_method ::
          Emit_env.get_scope env in
        let env = Emit_env.with_scope scope env in
        let hhas_method =
          make_memoize_wrapper_method env info ast_class ast_method
        in
        let newcount =
          if Hhas_method.is_static hhas_method then count else count+1 in
        newcount, hhas_method::acc
      else count, acc) in
  hhas_methods
