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
open Core
open Emit_memoize_helpers

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
      ~params: ast_method.Ast.m_params;
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

let get_self info =
  if info.memoize_is_trait
  then instr_self
  else gather [
    instr_string (Hhbc_id.Class.to_raw_string info.memoize_class_id);
    instr_clsrefgetc;
  ]

let get_cls_method info param_count method_id =
  let method_id =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  if info.memoize_is_trait
  then gather [
    instr_string (Hhbc_id.Method.to_raw_string method_id);
    instr_self;
    instr_fpushclsmethod ~forward:true param_count;
  ]
  else
    instr_fpushclsmethodd param_count method_id info.memoize_class_id

let make_memoize_instance_method_no_params_code ~non_null_return info method_id =
  let renamed_name =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  let needs_guard =
    info.memoize_instance_method_count = 1
    && not non_null_return in
  let ssmc =
    if needs_guard
    then guarded_shared_single_memoize_cache info.memoize_class_prefix
    else shared_single_memoize_cache info.memoize_class_prefix in
  let ssmcg = guarded_shared_single_memoize_cache_guard info.memoize_class_prefix in
  gather [
    instr_checkthis;
    optional needs_guard [
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_0;
    ];
    instr_baseh;
    instr_querym_cget_pt 0 ssmc;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    optional needs_guard [
      instr_label label_0;
      instr_null;
      instr_maybememotype;
      instr_jmpz label_2;
      instr_baseh;
      instr_querym_cget_pt 0 ssmcg;
      instr_jmpz label_2;
      instr_null;
      instr_retc;
      instr_label label_2;
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_3;
    ];
    instr_this;
    instr_fpushobjmethodd_nullthrows 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_baseh;
    instr_setm_pt 0 ssmc;
    optional needs_guard [
      instr_jmp label_4;
      instr_label label_3;
      instr_this;
      instr_fpushobjmethodd_nullthrows 0 renamed_name;
      instr_fcall 0;
      instr_unboxr;
      instr_label label_4;
      instr_null;
      instr_maybememotype;
      instr_jmpz label_5;
      instr_true;
      instr_baseh;
      instr_setm_pt 0 ssmcg;
      instr_popc;
      instr_label label_5;
    ];
    instr_retc ]

(* md is the already-renamed memoize method that must be wrapped *)
let make_memoize_instance_method_with_params_code ~pos
  env info method_id params index =
  let renamed_name =
    Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  (* All memoized methods in the same class share a cache. We distinguish the
  methods from each other by adding a unique integer indexing the method itself
  to the set of indices for the cache.
  The total number of unnamed locals is one for the optional index, and
  one for each formal parameter.
  *)
  let index_block, local_count =
    if info.memoize_instance_method_count > 1
    then
      gather [
        instr_int index;
        instr_setl first_local;
        instr_popc ],
      param_count + 1
    else
      empty,
      param_count
  in
  let begin_label, default_value_setters =
    (* Default value setters belong in the wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env params
  in
  (* The index of the first local that represents a formal is the number of
  parameters, plus one for the optional index. This is equal to the count
  of locals, so we'll just use that. *)
  let first_parameter_local = local_count in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params ~needs_local_this:false;
    instr_checkthis;
    index_block;
    param_code_sets params first_parameter_local;
    instr_baseh;
    instr_dim_warn_pt (shared_multi_memoize_cache info.memoize_class_prefix);
    instr_memoget 0 first_local local_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_this;
    instr_fpushobjmethodd_nullthrows param_count renamed_name;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_baseh;
    instr_dim_define_pt (shared_multi_memoize_cache info.memoize_class_prefix);
    instr_memoset 0 first_local local_count;
    instr_retc;
    default_value_setters ]

let make_memoize_static_method_no_params_code ~non_null_return info method_id =
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  let original_name_lc = String.lowercase_ascii
    (Hhbc_id.Method.to_raw_string method_id) in
  let needs_guard = not non_null_return in
  let smc =
    if needs_guard
    then original_name_lc ^ guarded_single_memoize_cache info.memoize_class_prefix
    else original_name_lc ^ single_memoize_cache info.memoize_class_prefix
  in
  gather [
    optional needs_guard [
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_0;
    ];
    instr_string smc;
    get_self info;
    instr_cgets;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    optional needs_guard [
      instr_null;
      instr_maybememotype;
      instr_jmpz label_2;
      instr_string (original_name_lc ^ guarded_single_memoize_cache_guard info.memoize_class_prefix);
      get_self info;
      instr_cgets;
      instr_jmpz label_2;
      instr_null;
      instr_retc;
      instr_label label_2;
      instr_null;
      instr_ismemotype;
      instr_jmpnz label_3;
    ];
    instr_string smc;
    get_self info;
    get_cls_method info 0 method_id;
    instr_fcall 0;
    instr_unboxr;
    instr_sets;
    optional needs_guard [
      instr_jmp label_4;
      instr_label label_3;
      get_cls_method info 0 method_id;
      instr_fcall 0;
      instr_unboxr;
      instr_label label_4;
      instr_null;
      instr_maybememotype;
      instr_jmpz label_5;
      instr_string (original_name_lc ^ guarded_single_memoize_cache_guard info.memoize_class_prefix);
      get_self info;
      instr_true;
      instr_sets;
      instr_popc;
      instr_label label_5;
    ];
    instr_retc ]

let make_memoize_static_method_with_params_code ~pos
  env info method_id params =
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  let original_name_lc = String.lowercase_ascii
    (Hhbc_id.Method.to_raw_string method_id) in
  let begin_label, default_value_setters =
    (* Default value setters belong in the wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env params
  in
  gather [
    begin_label;
    Emit_body.emit_method_prolog ~pos ~params:params ~needs_local_this:false;
    param_code_sets params param_count;
    instr_string (original_name_lc ^ multi_memoize_cache info.memoize_class_prefix);
    get_self info;
    instr_basesc 0;
    instr_memoget 1 first_local param_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_string (original_name_lc ^ multi_memoize_cache info.memoize_class_prefix);
    get_self info;
    get_cls_method info param_count method_id;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_basesc 1;
    instr_memoset 1 first_local param_count;
    instr_retc;
    default_value_setters ]

let make_memoize_static_method_code ~pos ~non_null_return env info method_id params =
  if List.is_empty params then
    make_memoize_static_method_no_params_code ~non_null_return info method_id
  else
    make_memoize_static_method_with_params_code ~pos env info method_id params

let make_memoize_instance_method_code ~pos ~non_null_return env info index method_id params =
  if List.is_empty params && info.memoize_instance_method_count <= 1
  then make_memoize_instance_method_no_params_code ~non_null_return info method_id
  else make_memoize_instance_method_with_params_code ~pos env info method_id params index

(* Construct the wrapper function *)
let make_wrapper return_type params instrs =
  Emit_body.make_body
    instrs
    [] (* decl_vars *)
    true (* is_memoize_wrapper *)
    params
    (Some return_type)
    [] (* static_inits *)
    None (* doc *)

let emit ~pos ~non_null_return env info index return_type_info params is_static method_id =
  let instrs =
    if is_static
    then make_memoize_static_method_code ~pos ~non_null_return env info method_id params
    else make_memoize_instance_method_code ~pos ~non_null_return env info index method_id params
  in
  make_wrapper return_type_info params instrs

let emit_memoize_wrapper_body env memoize_info index ast_method
    ~scope ~namespace params ret =
    let is_static =List.mem ast_method.Ast.m_kind Ast.Static in
    let tparams =
      Core.List.map (Ast_scope.Scope.get_tparams scope) (fun (_, (_, s), _) -> s) in
    let return_type_info =
      Emit_body.emit_return_type_info ~scope ~skipawaitable:false ~namespace ret in
    let non_null_return = cannot_return_null ast_method.Ast.m_fun_kind ast_method.Ast.m_ret in
    let params = Emit_param.from_asts ~namespace ~tparams:tparams ~generate_defaults:true ~scope params in
    let pos = ast_method.Ast.m_span in
    let (_,original_name) = ast_method.Ast.m_name in
    let method_id =
      Hhbc_id.Method.from_ast_name original_name in
    (*let method_id = Hhbc_id.Method.add_suffix method_id Generate_memoized.memoize_suffix in*)
    emit ~pos ~non_null_return env memoize_info index return_type_info params is_static method_id

let make_memoize_wrapper_method env info index ast_class ast_method =
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
  let method_id = Hhbc_id.Method.from_ast_name original_name in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  let namespace = ast_class.Ast.c_namespace in
  let method_body =
    emit_memoize_wrapper_body env info index ast_method
      ~scope ~namespace ast_method.Ast.m_params ret in
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
    false (*method_is_async*)
    false (*method_is_generator*)
    false (*method_is_pair_generator*)
    false (*method_is_closure_body*)

let emit_wrapper_methods env info ast_class ast_methods =
  (* Wrapper methods may not have iterators *)
  Iterator.reset_iterator ();
  let _, hhas_methods =
    List.fold_left ast_methods ~init:(0, []) ~f:(fun (count,acc) ast_method ->
      if Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes
      then
        let hhas_method =
          make_memoize_wrapper_method env info count ast_class ast_method in
        let newcount =
          if Hhas_method.is_static hhas_method then count else count+1 in
        newcount, hhas_method::acc
      else count, acc) in
  hhas_methods

let empty_dict_init = Some (Typed_value.Dict [])
let false_init = Some (Typed_value.Bool false)
let null_init = Some (Typed_value.Null)

let notype = Hhas_type_info.make (Some "") (Hhas_type_constraint.make None [])
let make_instance_properties info ast_methods =
  let is_memoized_instance ast_method =
    Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes &&
    not (List.mem ast_method.Ast.m_kind Ast.Static) in
  let memoized_instance_methods = List.filter ast_methods is_memoized_instance in
  match memoized_instance_methods with
  | [] -> []
  | [ast_method] when List.is_empty ast_method.Ast.m_params ->
    let needs_guard = not (cannot_return_null ast_method.Ast.m_fun_kind ast_method.Ast.m_ret) in
    if needs_guard
    then
      let property2 = Hhas_property.make true false false false false true
        (guarded_shared_single_memoize_cache info.memoize_class_prefix) null_init None notype in
      let property1 = Hhas_property.make true false false false false true
        (guarded_shared_single_memoize_cache_guard info.memoize_class_prefix) false_init None notype in
      [property1; property2]
    else
      let property = Hhas_property.make true false false false false true
        (shared_single_memoize_cache info.memoize_class_prefix) null_init None notype in
      [property]

  | _ ->
    let property = Hhas_property.make true false false false false true
      (shared_multi_memoize_cache info.memoize_class_prefix) empty_dict_init None notype in
    [property]

let make_static_properties info ast_methods =
  let mapper ast_method =
    let is_memoized_static =
      Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes &&
      List.mem ast_method.Ast.m_kind Ast.Static in
    if is_memoized_static then
      let no_params = List.is_empty ast_method.Ast.m_params in
      let original_method_name = snd ast_method.Ast.m_name in
      let prop_name =
        Hhbc_id.Prop.from_ast_name
        (String.lowercase_ascii @@ original_method_name) in
      if no_params then
        let needs_guard =
          not (cannot_return_null ast_method.Ast.m_fun_kind ast_method.Ast.m_ret) in
        if needs_guard then
        let property2 = Hhas_property.make true false false true false true
          (Hhbc_id.Prop.add_suffix prop_name (guarded_single_memoize_cache info.memoize_class_prefix))
          null_init None notype in
        let property1 = Hhas_property.make true false false true false true
          (Hhbc_id.Prop.add_suffix prop_name (guarded_single_memoize_cache_guard info.memoize_class_prefix))
          false_init None notype in
        [property1; property2]
        else
        let property = Hhas_property.make true false false true false true
          (Hhbc_id.Prop.add_suffix prop_name (single_memoize_cache info.memoize_class_prefix))
          null_init None notype in
        [property]
      else
        let property = Hhas_property.make true false false true false true
          (Hhbc_id.Prop.add_suffix prop_name (multi_memoize_cache info.memoize_class_prefix))
          empty_dict_init None notype in
        [property]
    else
      [] in
  List.concat_map ast_methods mapper

let emit_properties info ast_methods =
  make_static_properties info ast_methods @
  make_instance_properties info ast_methods
