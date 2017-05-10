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
open Hhbc_ast.MemberOpMode

let memoize_suffix = "$memoize_impl"
let static_memoize_cache = "static$memoize_cache"
let static_memoize_cache_guard = "static$memoize_cache$guard"
let multi_memoize_cache = "$multi$memoize_cache"
let shared_multi_memoize_cache =
  Hhbc_id.Prop.from_raw_string "$shared$multi$memoize_cache"
let single_memoize_cache = "$guarded_single$memoize_cache"
let single_memoize_cache_guard = "$guarded_single$memoize_cache$guard"
let shared_single_memoize_cache =
  Hhbc_id.Prop.from_raw_string
  "$shared$guarded_single$memoize_cache"
let shared_single_memoize_cache_guard =
  Hhbc_id.Prop.from_raw_string
  "$shared$guarded_single$memoize_cache$guard"

let param_code_sets params local =
  let rec aux params local block =
  match params with
  | [] -> block
  | h :: t ->
    let code = gather [
      instr_getmemokeyl (Local.Named (Hhas_param.name h));
      instr_setl (Local.Unnamed local);
      instr_popc ] in
    aux t (local + 1) (gather [block; code]) in
  aux params local empty

let param_code_gets params =
  let rec aux params index block =
    match params with
    | [] -> block
    | h :: t ->
      let block = gather [
        block;
        instr_fpassl index (Local.Named (Hhas_param.name h)) ] in
      aux t (index + 1) block in
  aux params 0 empty

let memoize_function_no_params renamed_name =
  (* TODO: A lot of this codegen doesn't make a lot of sense to me.
  Try to understand it and see if it can be improved. *)
  let local_cache = Local.Unnamed 0 in
  let local_guard = Local.Unnamed 1 in
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  gather [
    instr_entrynop;
    instr_false;
    instr_staticlocinit local_guard static_memoize_cache_guard;
    instr_null;
    instr_staticlocinit local_cache static_memoize_cache;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_0;
    instr_cgetl local_cache;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_2;
    instr_cgetl local_guard;
    instr_jmpz label_2;
    instr_null;
    instr_retc;
    instr_label label_2;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_3;
    instr_fpushfuncd 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_setl local_cache;
    instr_jmp label_4;
    instr_label label_3;
    instr_fpushfuncd 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_label label_4;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_5;
    instr_true;
    instr_setl local_guard;
    instr_popc;
    instr_label label_5;
    instr_retc ]

let memoize_function_with_params params renamed_name =
  let param_count = List.length params in
  let static_local = Local.Unnamed param_count in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed (param_count + 1) in
  gather [
    (* Why do we emit a no-op that cannot be removed here?
    The alleged reason for this no-op is because the runtime handles a branch
    to the first intruction poorly, but we know there will never be any such
    branch in this method. *)
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    instr_dict (Typed_value.Dict []);
    instr_staticlocinit static_local static_memoize_cache;
    param_code_sets params (param_count + 1);
    instr_basel static_local Warn;
    instr_memoget 0 first_local param_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    instr_fpushfuncd param_count renamed_name;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_basel static_local Define;
    instr_memoset 0 first_local param_count;
    instr_retc
  ]

let memoized_function_body params renamed_name =
  if params = [] then
    memoize_function_no_params renamed_name
  else
    memoize_function_with_params params renamed_name

let memoize_function compiled =
  let original_name = Hhas_function.name compiled in
  let renamed_name =
    Hhbc_id.Function.add_suffix original_name memoize_suffix in
  let renamed = Hhas_function.with_name compiled renamed_name in
  let params = Hhas_function.params compiled in
  let body = memoized_function_body params renamed_name in
  let body = instr_seq_to_list body in
  let memoized = Hhas_function.with_body compiled body in
  (renamed, memoized)

let memoize_functions compiled_funcs =
  let mapper compiled =
    if Hhas_attribute.is_memoized (Hhas_function.attributes compiled) then
    let (renamed, memoized) = memoize_function compiled in
      [ renamed; memoized ]
    else
      [ compiled ] in
  Core.List.bind compiled_funcs mapper

let memoize_instance_method_no_params original_name =
  let renamed_name =
    Hhbc_id.Method.add_suffix original_name memoize_suffix in
  (* TODO: A lot of this codegen doesn't make a lot of sense to me.
  Try to understand it and see if it can be improved. *)
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  gather [
    instr_entrynop;
    instr_checkthis;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_0;
    instr_baseh;
    instr_querym_cget_pt 0 shared_single_memoize_cache;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_2;
    instr_baseh;
    instr_querym_cget_pt 0 shared_single_memoize_cache_guard;
    instr_jmpz label_2;
    instr_null;
    instr_retc;
    instr_label label_2;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_3;
    instr_this;
    instr_fpushobjmethodd_nullthrows 0 renamed_name;
    instr_fcall 0;
    instr_unboxr;
    instr_baseh;
    instr_setm_pt 0 shared_single_memoize_cache;
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
    instr_setm_pt 0 shared_single_memoize_cache_guard;
    instr_popc;
    instr_label label_5;
    instr_retc ]

let memoize_instance_method_with_params params original_name total_count index =
  let renamed_name =
    Hhbc_id.Method.add_suffix original_name memoize_suffix in
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  (* All memoized methods in the same class share a cache. We distinguish the
  methods from each other by adding a unique integer indexing the method itself
  to the set of indices for the cache. *)
  let index_block =
    if total_count = 1 then
      empty
    else
      gather [
        instr_int index;
        instr_setl first_local;
        instr_popc ] in
  (* The total number of unnamed locals is one for the optional index, and
  one for each formal parameter. *)
  let local_count = if total_count = 1 then param_count else param_count + 1 in
  (* The index of the first local that represents a formal is the number of
  parameters, plus one for the optional index. This is equal to the count
  of locals, so we'll just use that. *)
  let first_parameter_local = local_count in
  gather [
    (* Why do we emit a no-op that cannot be removed here? *)
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    instr_checkthis;
    index_block;
    param_code_sets params first_parameter_local;
    instr_baseh;
    instr_dim_warn_pt shared_multi_memoize_cache;
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
    instr_dim_define_pt shared_multi_memoize_cache;
    instr_memoset 0 first_local local_count;
    instr_retc ]

let memoize_static_method_no_params original_name class_name =
  (* TODO: A lot of this codegen doesn't make a lot of sense to me.
  Try to understand it and see if it can be improved. *)
  let label_0 = Label.Regular 0 in
  let label_1 = Label.Regular 1 in
  let label_2 = Label.Regular 2 in
  let label_3 = Label.Regular 3 in
  let label_4 = Label.Regular 4 in
  let label_5 = Label.Regular 5 in
  let original_name_lc = String.lowercase_ascii
    (Hhbc_id.Method.to_raw_string original_name) in
  gather [
    instr_entrynop;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_0;
    instr_string (original_name_lc ^ single_memoize_cache);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_cgets;
    instr_dup;
    instr_istypec Hhbc_ast.OpNull;
    instr_jmpnz label_1;
    instr_retc;
    instr_label label_1;
    instr_popc;
    instr_label label_0;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_2;
    instr_string (original_name_lc ^ single_memoize_cache_guard);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_cgets;
    instr_jmpz label_2;
    instr_null;
    instr_retc;
    instr_label label_2;
    instr_null;
    instr_ismemotype;
    instr_jmpnz label_3;
    instr_string (original_name_lc ^ single_memoize_cache);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_fpushclsmethodd 0
      (Hhbc_id.Method.add_suffix original_name memoize_suffix)
      class_name;
    instr_fcall 0;
    instr_unboxr;
    instr_sets;
    instr_jmp label_4;
    instr_label label_3;
    instr_fpushclsmethodd 0
      (Hhbc_id.Method.add_suffix original_name memoize_suffix)
      class_name;
    instr_fcall 0;
    instr_unboxr;
    instr_label label_4;
    instr_null;
    instr_maybememotype;
    instr_jmpz label_5;
    instr_string (original_name_lc ^ single_memoize_cache_guard);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_true;
    instr_sets;
    instr_popc;
    instr_label label_5;
    instr_retc ]

let memoize_static_method_with_params params original_name class_name =
  let renamed_name =
    Hhbc_id.Method.add_suffix original_name memoize_suffix in
  let param_count = List.length params in
  let label = Label.Regular 0 in
  let first_local = Local.Unnamed param_count in
  let original_name_lc = String.lowercase_ascii
    (Hhbc_id.Method.to_raw_string original_name) in
  gather [
    instr_entrynop;
    Emit_body.emit_method_prolog params;
    param_code_sets params param_count;
    instr_string (original_name_lc ^ multi_memoize_cache);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_basesc 0;
    instr_memoget 1 first_local param_count;
    instr_isuninit;
    instr_jmpnz label;
    instr_cgetcunop;
    instr_retc;
    instr_label label;
    instr_ugetcunop;
    instr_popu;
    (* TODO: The strings have extra leading slashes unnecessarily *)
    instr_string (original_name_lc ^ multi_memoize_cache);
    instr_string (Hhbc_id.Class.to_raw_string class_name);
    instr_clsrefgetc;
    instr_fpushclsmethodd param_count renamed_name class_name;
    param_code_gets params;
    instr_fcall param_count;
    instr_unboxr;
    instr_basesc 0;
    instr_memoset 1 first_local param_count;
    instr_retc ]

let memoize_static_method_body class_name params original_name =
  if params = [] then
    memoize_static_method_no_params original_name class_name
  else
    memoize_static_method_with_params params original_name class_name

let memoize_instance_method_body total_count index params original_name =
  if params = [] && total_count = 1 then
    memoize_instance_method_no_params original_name
  else
    memoize_instance_method_with_params params original_name total_count index

let memoize_method method_ memoizer =
  let original_name = Hhas_method.name method_ in
  let renamed_name =
    Hhbc_id.Method.add_suffix original_name memoize_suffix in
  let renamed = Hhas_method.with_name method_ renamed_name in
  let renamed = Hhas_method.make_private renamed in
  let params = Hhas_method.params method_ in
  let body = memoizer params original_name in
  let body = rewrite_class_refs body in
  let num_cls_ref_slots = get_num_cls_ref_slots body in
  let body = instr_seq_to_list body in
  let memoized = Hhas_method.with_body method_ body in
  let memoized =
    Hhas_method.with_num_cls_ref_slots memoized num_cls_ref_slots
  in
  (renamed, memoized)

let memoize_instance_method method_ total_count index =
  memoize_method method_ (memoize_instance_method_body total_count index)

let memoize_static_method class_ method_ =
  let class_name = Hhas_class.name class_ in
  memoize_method method_ (memoize_static_method_body class_name)

let is_memoized_instance method_ =
  (not (Hhas_method.is_static method_)) &&
  Hhas_attribute.is_memoized (Hhas_method.attributes method_)

let empty_dict_init = Some (Typed_value.Dict [])
let false_init = Some (Typed_value.Bool false)
let null_init = Some (Typed_value.Null)

let add_instance_properties class_ =
  let folder (count, zero_params) method_ =
    if is_memoized_instance method_ then
      (count + 1, zero_params || (Hhas_method.params method_ = []) )
    else
      (count, zero_params) in
  let methods = Hhas_class.methods class_ in
  let (count, zero_params) =
    Core.List.fold_left methods ~init:(0, false) ~f:folder in
  if count = 1 && zero_params then
    let property = Hhas_property.make true false false false false
      shared_single_memoize_cache empty_dict_init None in
    let class_ = Hhas_class.with_property class_ property in
    let property = Hhas_property.make true false false false false
      shared_single_memoize_cache_guard false_init None in
    Hhas_class.with_property class_ property
  else
    let property = Hhas_property.make true false false false false
      shared_multi_memoize_cache empty_dict_init None in
    Hhas_class.with_property class_ property

let memoize_instance_methods class_ =
  let methods = Hhas_class.methods class_ in
  let memoized_count = Core.List.count methods is_memoized_instance in
  let folder (count, acc) method_ =
    if is_memoized_instance method_ then
      let (renamed, memoized) = memoize_instance_method
        method_ memoized_count count in
      (count + 1, memoized :: renamed :: acc)
    else
      (count, method_ :: acc) in
  let (count, methods) = Core.List.fold_left methods ~init:(0, []) ~f:folder in
  if count = 0 then
    class_
  else
    let class_ = add_instance_properties class_ in
    let methods = List.rev methods in
    Hhas_class.with_methods class_ methods

let is_memoized_static method_ =
  (Hhas_method.is_static method_) &&
  Hhas_attribute.is_memoized (Hhas_method.attributes method_)

let add_static_properties class_ =
  let folder class_ method_ =
    if is_memoized_static method_ then
      let params = Hhas_method.params method_ in
      let original_name =
        Hhbc_id.Prop.from_ast_name
        (String.lowercase_ascii @@ Hhbc_id.Method.to_raw_string
        @@ Hhas_method.name method_) in
      if params = [] then
        let property = Hhas_property.make true false false true false
          (Hhbc_id.Prop.add_suffix original_name single_memoize_cache) null_init None in
        let class_ = Hhas_class.with_property class_ property in
        let property = Hhas_property.make true false false true false
          (Hhbc_id.Prop.add_suffix original_name single_memoize_cache_guard) false_init None in
        Hhas_class.with_property class_ property
      else
        let property = Hhas_property.make true false false true false
          (Hhbc_id.Prop.add_suffix original_name multi_memoize_cache) null_init None in
        Hhas_class.with_property class_ property
    else
      class_ in
  let methods = Hhas_class.methods class_ in
  Core.List.fold_left methods ~init:class_ ~f:folder

let memoize_static_methods class_ =
  let mapper method_ =
    if is_memoized_static method_ then
      let (renamed, memoized) = memoize_static_method class_ method_ in
      [ renamed ; memoized ]
    else
      [ method_ ] in
  let class_ = add_static_properties class_ in
  let methods = Hhas_class.methods class_ in
  let methods = Core.List.bind methods mapper in
  Hhas_class.with_methods class_ methods

let memoize_class class_ =
  let class_ = memoize_instance_methods class_ in
  memoize_static_methods class_

let memoize_classes classes =
  List.map memoize_class classes
