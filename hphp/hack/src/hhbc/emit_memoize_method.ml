(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence
open Emit_memoize_helpers
open Hhbc_ast
module R = Hhbc_string_utils.Reified
module T = Aast

(* Precomputed information required for generation of memoized methods *)
type memoize_info = {
  (* Prefix on method and property names *)
  memoize_class_prefix: string;
  (* Number of instance methods with <<__Memoize>> *)
  memoize_instance_method_count: int;
  (* True if the enclosing class is a trait *)
  memoize_is_trait: bool;
  (* Enclosing class ID *)
  memoize_class_id: Hhbc_id.Class.t;
}

let is_memoize ast_method =
  Emit_attribute.ast_any_is_memoize ast_method.T.m_user_attributes

let make_info ast_class class_id ast_methods =
  let check_method ast_method =
    if is_memoize ast_method then (
      let pos = fst ast_method.T.m_name in
      Emit_memoize_helpers.check_memoize_possible
        pos
        ~params:ast_method.T.m_params
        ~is_method:true;
      if ast_class.T.c_kind = Ast_defs.Cinterface then
        Emit_fatal.raise_fatal_runtime
          pos
          "<<__Memoize>> cannot be used in interfaces"
      else if ast_method.T.m_abstract then
        Emit_fatal.raise_fatal_parse
          pos
          ( "Abstract method "
          ^ Hhbc_id.Class.to_raw_string class_id
          ^ "::"
          ^ snd ast_method.T.m_name
          ^ " cannot be memoized" )
    )
  in
  List.iter ast_methods check_method;
  let is_trait = ast_class.T.c_kind = Ast_defs.Ctrait in
  let class_prefix =
    if is_trait then
      "$" ^ String.lowercase (Hhbc_id.Class.to_raw_string class_id)
    else
      ""
  in
  let instance_count =
    List.count ast_methods ~f:(fun ast_method ->
        is_memoize ast_method && not ast_method.T.m_static)
  in
  {
    memoize_is_trait = is_trait;
    memoize_class_prefix = class_prefix;
    memoize_instance_method_count = instance_count;
    memoize_class_id = class_id;
  }

let call_cls_method info fcall_args method_id with_lsb =
  let method_id = Hhbc_id.Method.add_suffix method_id memoize_suffix in
  if info.memoize_is_trait || with_lsb then
    instr_fcallclsmethodsd fcall_args SpecialClsRef.Self method_id
  else
    instr_fcallclsmethodd fcall_args method_id info.memoize_class_id

let make_memoize_instance_method_no_params_code
    scope deprecation_info method_id is_async =
  let renamed_name = Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let notfound = Label.next_regular () in
  let suspended_get = Label.next_regular () in
  let eager_set = Label.next_regular () in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  let fcall_args =
    if is_async then
      make_fcall_args ~async_eager_label:eager_set 0
    else
      make_fcall_args 0
  in
  gather
    [
      deprecation_body;
      instr_checkthis;
      ( if is_async then
        gather
          [
            instr_memoget_eager notfound suspended_get None;
            instr_retc;
            instr_label suspended_get;
            instr_retc_suspended;
          ]
      else
        gather [instr_memoget notfound None; instr_retc] );
      instr_label notfound;
      instr_this;
      instr_nulluninit;
      instr_nulluninit;
      instr_fcallobjmethodd_nullthrows fcall_args renamed_name;
      instr_memoset None;
      ( if is_async then
        gather
          [
            instr_retc_suspended;
            instr_label eager_set;
            instr_memoset_eager None;
            instr_retc;
          ]
      else
        gather [instr_retc] );
    ]

(* md is the already-renamed memoize method that must be wrapped *)
let make_memoize_instance_method_with_params_code
    ~pos
    env
    scope
    deprecation_info
    method_id
    params
    ast_params
    is_async
    is_reified =
  let renamed_name = Hhbc_id.Method.add_suffix method_id memoize_suffix in
  let param_count = List.length params in
  let notfound = Label.next_regular () in
  let suspended_get = Label.next_regular () in
  let eager_set = Label.next_regular () in
  (* The local that contains the reified generics is the first non parameter
   * local, so the first local is parameter count + 1 when there are reified =
   * generics *)
  let add_reified =
    if is_reified then
      1
    else
      0
  in
  let first_local = Local.Unnamed (param_count + add_reified) in
  let (begin_label, default_value_setters) =
    (* Default value setters belong in the
     * wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  let fcall_args =
    let flags = { default_fcall_flags with has_generics = is_reified } in
    if is_async then
      make_fcall_args ~flags ~async_eager_label:eager_set param_count
    else
      make_fcall_args ~flags param_count
  in
  let (reified_get, reified_memokeym) =
    if not is_reified then
      (empty, empty)
    else
      ( instr_cgetl (Local.Named R.reified_generics_local_name),
        gather
        @@ getmemokeyl
             param_count
             (param_count + add_reified)
             R.reified_generics_local_name )
  in
  let param_count = param_count + add_reified in
  gather
    [
      begin_label;
      Emit_body.emit_method_prolog
        ~env
        ~pos
        ~params
        ~ast_params
        ~tparams:[]
        ~should_emit_init_this:false;
      deprecation_body;
      instr_checkthis;
      param_code_sets params param_count;
      reified_memokeym;
      ( if is_async then
        gather
          [
            instr_memoget_eager
              notfound
              suspended_get
              (Some (first_local, param_count));
            instr_retc;
            instr_label suspended_get;
            instr_retc_suspended;
          ]
      else
        gather
          [instr_memoget notfound (Some (first_local, param_count)); instr_retc]
      );
      instr_label notfound;
      instr_this;
      instr_nulluninit;
      instr_nulluninit;
      param_code_gets params;
      reified_get;
      instr_fcallobjmethodd_nullthrows fcall_args renamed_name;
      instr_memoset (Some (first_local, param_count));
      ( if is_async then
        gather
          [
            instr_retc_suspended;
            instr_label eager_set;
            instr_memoset_eager (Some (first_local, param_count));
            instr_retc;
          ]
      else
        gather [instr_retc] );
      default_value_setters;
    ]

let make_memoize_static_method_no_params_code
    info scope deprecation_info method_id with_lsb is_async =
  let notfound = Label.next_regular () in
  let suspended_get = Label.next_regular () in
  let eager_set = Label.next_regular () in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  let fcall_args =
    if is_async then
      make_fcall_args ~async_eager_label:eager_set 0
    else
      make_fcall_args 0
  in
  gather
    [
      deprecation_body;
      ( if is_async then
        gather
          [
            instr_memoget_eager notfound suspended_get None;
            instr_retc;
            instr_label suspended_get;
            instr_retc_suspended;
          ]
      else
        gather [instr_memoget notfound None; instr_retc] );
      instr_label notfound;
      instr_nulluninit;
      instr_nulluninit;
      instr_nulluninit;
      call_cls_method info fcall_args method_id with_lsb;
      instr_memoset None;
      ( if is_async then
        gather
          [
            instr_retc_suspended;
            instr_label eager_set;
            instr_memoset_eager None;
            instr_retc;
          ]
      else
        gather [instr_retc] );
    ]

let make_memoize_static_method_with_params_code
    ~pos
    env
    info
    scope
    deprecation_info
    method_id
    with_lsb
    params
    ast_params
    is_async
    is_reified =
  let param_count = List.length params in
  let notfound = Label.next_regular () in
  let suspended_get = Label.next_regular () in
  let eager_set = Label.next_regular () in
  (* The local that contains the reified generics is the first non parameter
   * local, so the first local is parameter count + 1 when there are reified =
   * generics *)
  let add_reified =
    if is_reified then
      1
    else
      0
  in
  let first_local = Local.Unnamed (param_count + add_reified) in
  let deprecation_body =
    Emit_body.emit_deprecation_warning scope deprecation_info
  in
  let (begin_label, default_value_setters) =
    (* Default value setters belong in the
     * wrapper method not in the original method *)
    Emit_param.emit_param_default_value_setter env pos params
  in
  let fcall_args =
    let flags = { default_fcall_flags with has_generics = is_reified } in
    if is_async then
      make_fcall_args ~flags ~async_eager_label:eager_set param_count
    else
      make_fcall_args ~flags param_count
  in
  let (reified_get, reified_memokeym) =
    if not is_reified then
      (empty, empty)
    else
      ( instr_cgetl (Local.Named R.reified_generics_local_name),
        gather
        @@ getmemokeyl
             param_count
             (param_count + add_reified)
             R.reified_generics_local_name )
  in
  let param_count = param_count + add_reified in
  gather
    [
      begin_label;
      Emit_body.emit_method_prolog
        ~env
        ~pos
        ~params
        ~ast_params
        ~tparams:[]
        ~should_emit_init_this:false;
      deprecation_body;
      param_code_sets params param_count;
      reified_memokeym;
      ( if is_async then
        gather
          [
            instr_memoget_eager
              notfound
              suspended_get
              (Some (first_local, param_count));
            instr_retc;
            instr_label suspended_get;
            instr_retc_suspended;
          ]
      else
        gather
          [instr_memoget notfound (Some (first_local, param_count)); instr_retc]
      );
      instr_label notfound;
      instr_nulluninit;
      instr_nulluninit;
      instr_nulluninit;
      param_code_gets params;
      reified_get;
      call_cls_method info fcall_args method_id with_lsb;
      instr_memoset (Some (first_local, param_count));
      ( if is_async then
        gather
          [
            instr_retc_suspended;
            instr_label eager_set;
            instr_memoset_eager (Some (first_local, param_count));
            instr_retc;
          ]
      else
        gather [instr_retc] );
      default_value_setters;
    ]

let make_memoize_static_method_code
    ~pos
    env
    info
    scope
    deprecation_info
    method_id
    with_lsb
    params
    ast_params
    is_async
    is_reified =
  if List.is_empty params && not is_reified then
    make_memoize_static_method_no_params_code
      info
      scope
      deprecation_info
      method_id
      with_lsb
      is_async
  else
    make_memoize_static_method_with_params_code
      ~pos
      env
      info
      scope
      deprecation_info
      method_id
      with_lsb
      params
      ast_params
      is_async
      is_reified

let make_memoize_instance_method_code
    ~pos
    env
    scope
    deprecation_info
    method_id
    params
    ast_params
    is_async
    is_reified =
  if List.is_empty params && not is_reified then
    make_memoize_instance_method_no_params_code
      scope
      deprecation_info
      method_id
      is_async
  else
    make_memoize_instance_method_with_params_code
      ~pos
      env
      scope
      deprecation_info
      method_id
      params
      ast_params
      is_async
      is_reified

(* Construct the wrapper function *)
let make_wrapper env return_type params instrs with_lsb is_reified =
  Emit_body.make_body
    instrs
    ( if is_reified then
      [R.reified_generics_local_name]
    else
      []
    (* decl_vars *) )
    true (* is_memoize_wrapper *)
    with_lsb (* is_memoize_wrapper_lsb *)
    [] (* upper_bounds *)
    [] (* shadowed_tparams *)
    params
    (Some return_type)
    None (* doc *)
    (Some env)

let emit
    ~pos
    env
    info
    return_type_info
    scope
    deprecation_info
    params
    ast_params
    is_static
    method_id
    with_lsb
    is_async
    is_reified =
  let instrs =
    Emit_pos.emit_pos_then pos
    @@
    if is_static then
      make_memoize_static_method_code
        ~pos
        env
        info
        scope
        deprecation_info
        method_id
        with_lsb
        params
        ast_params
        is_async
        is_reified
    else
      make_memoize_instance_method_code
        ~pos
        env
        scope
        deprecation_info
        method_id
        params
        ast_params
        is_async
        is_reified
  in
  make_wrapper env return_type_info params instrs with_lsb is_reified

let emit_memoize_wrapper_body
    env
    memoize_info
    ast_method
    ~namespace
    scope
    deprecation_info
    params
    ret
    is_async
    is_reified =
  let is_static = ast_method.T.m_static in
  let tparams =
    List.map (Ast_scope.Scope.get_tparams scope) ~f:(fun t -> snd t.T.tp_name)
  in
  let return_type_info =
    Emit_body.emit_return_type_info ~scope ~skipawaitable:is_async ret
  in
  let params =
    Emit_param.from_asts
      ~namespace
      ~tparams
      ~generate_defaults:true
      ~scope
      params
  in
  let pos = ast_method.T.m_span in
  let (_, original_name) = ast_method.T.m_name in
  let method_id = Hhbc_id.Method.from_ast_name original_name in
  let with_lsb =
    Emit_attribute.ast_any_is_memoize_lsb ast_method.T.m_user_attributes
  in
  emit
    ~pos
    env
    memoize_info
    return_type_info
    scope
    deprecation_info
    params
    ast_method.T.m_params
    is_static
    method_id
    with_lsb
    is_async
    is_reified

let make_memoize_wrapper_method env info ast_class ast_method =
  (* This is cut-and-paste from emit_method above, with special casing for
   * wrappers *)
  let method_visibility = ast_method.T.m_visibility in
  let (_, original_name) = ast_method.T.m_name in
  let ret =
    if original_name = Naming_special_names.Members.__construct then
      None
    else
      T.hint_of_type_hint ast_method.T.m_ret
  in
  let method_id = Hhbc_id.Method.from_ast_name original_name in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method; Ast_scope.ScopeItem.Class ast_class]
  in
  let namespace = ast_class.T.c_namespace in
  let method_is_interceptable =
    Interceptable.is_method_interceptable ast_class method_id
  in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.T.m_user_attributes
  in
  let method_attributes =
    Emit_attribute.add_reified_attribute
      method_attributes
      ast_method.T.m_tparams
  in
  let method_is_async = ast_method.T.m_fun_kind = Ast_defs.FAsync in
  let deprecation_info = Hhas_attribute.deprecation_info method_attributes in
  (* __Memoize is not allowed on lambdas, so we never need to inherit the rx
     level from the declaring scope when we're in a Memoize wrapper *)
  let method_rx_level =
    Rx.rx_level_from_ast ast_method.T.m_user_attributes
    |> Option.value ~default:Rx.NonRx
  in
  let env = Emit_env.with_rx_body (method_rx_level <> Rx.NonRx) env in
  let is_reified =
    List.exists
      ~f:(fun t -> t.T.tp_reified = T.Reified || t.T.tp_reified = T.SoftReified)
      ast_method.T.m_tparams
  in
  let method_body =
    emit_memoize_wrapper_body
      env
      info
      ast_method
      ~namespace
      scope
      deprecation_info
      ast_method.T.m_params
      ret
      method_is_async
      is_reified
  in
  Hhas_method.make
    method_attributes
    method_visibility
    ast_method.T.m_static
    ast_method.T.m_final
    ast_method.T.m_abstract
    false (*method_no_injection*)
    method_id
    method_body
    (Hhas_pos.pos_to_span ast_method.T.m_span)
    method_is_async
    false (*method_is_generator*)
    false (*method_is_pair_generator*)
    false (*method_is_closure_body*)
    method_is_interceptable
    false (*method_is_memoize_impl*)
    method_rx_level
    (* method_rx_disabled *)
    false

let emit_wrapper_methods env info ast_class ast_methods =
  (* Wrapper methods may not have iterators *)
  Iterator.reset_iterator ();
  let hhas_methods =
    List.fold_left ast_methods ~init:[] ~f:(fun acc ast_method ->
        if Emit_attribute.ast_any_is_memoize ast_method.T.m_user_attributes then
          let scope =
            Ast_scope.ScopeItem.Method ast_method :: Emit_env.get_scope env
          in
          let env = Emit_env.with_scope scope env in
          let hhas_method =
            make_memoize_wrapper_method env info ast_class ast_method
          in
          hhas_method :: acc
        else
          acc)
  in
  hhas_methods
