(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Instruction_sequence
open Emit_type_hint
open Emit_expression

let ast_is_interface ast_class =
  ast_class.A.c_kind = Ast.Cinterface

let default_constructor ast_class =
  let method_attributes = [] in
  let method_name = "86ctor" in
  let method_body = instr_seq_to_list (gather [
    instr_null;
    instr_retc;
  ]) in
  let method_is_abstract = ast_is_interface ast_class in
  let method_is_final = false in
  let method_is_private = false in
  let method_is_protected = false in
  let method_is_public = true in
  let method_is_static = false in
  let method_params = [] in
  let method_return_type = None in
  let method_decl_vars = [] in
  let method_num_iters = 0 in
  let method_num_cls_ref_slots = 0 in
  let method_is_async = false in
  let method_is_generator = false in
  let method_is_pair_generator = false in
  let method_is_closure_body = false in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    method_name
    method_params
    method_return_type
    method_body
    method_decl_vars
    method_num_iters
    method_num_cls_ref_slots
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_extends ~is_enum _tparams extends =
  if is_enum then Some ("HH\\BuiltinEnum") else
  match extends with
  | [] -> None
  | h :: _ -> Some (hint_to_class h)

let from_implements _tparams implements =
  List.map implements hint_to_class

let from_constant (_hint, name, const_init) =
  (* The type hint is omitted. *)
  match const_init with
  | None -> None (* Abstract constants are omitted *)
  | Some init ->
    let constant_name = Litstr.to_string @@ snd name in
    let constant_value = Constant_folder.literal_from_expr init in
    Some (Hhas_constant.make constant_name constant_value)

let from_constants ast_constants =
  List.filter_map ast_constants from_constant

let from_type_constant ast_type_constant =
  match ast_type_constant.A.tconst_type with
  | None -> None (* Abstract type constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let type_constant_name = Litstr.to_string @@
      snd ast_type_constant.A.tconst_name in
    Some (Hhas_type_constant.make type_constant_name)

let from_type_constants ast_type_constants =
  List.filter_map ast_type_constants from_type_constant

let ast_methods ast_class_body =
  let mapper elt =
    match elt with
    | A.Method m -> Some m
    | _ -> None in
  List.filter_map ast_class_body mapper

let from_class_elt_classvars elt =
  match elt with
  | A.ClassVars (kind_list, type_hint, cvl) ->
    List.map cvl (Emit_property.from_ast kind_list type_hint)
  | _ -> []

let from_class_elt_constants elt =
  match elt with
  | A.Const(hint_opt, l) ->
    List.filter_map l (fun (id, e) -> from_constant (hint_opt, id, Some e))
  | _ -> []

let from_class_elt_typeconsts elt =
  match elt with
  | A.TypeConst tc -> from_type_constant tc
  | _ -> None

let from_enum_type opt =
  match opt with
  | Some e ->
    let type_info_user_type = Some (Emit_type_hint.fmt_hint e.A.e_base) in
    let type_info_type_constraint =
      Hhas_type_constraint.make
        None
        [Hhas_type_constraint.HHType; Hhas_type_constraint.ExtendedHint]
    in
    Some (Hhas_type_info.make type_info_user_type type_info_type_constraint)
  | _ -> None

let from_ast : A.class_ -> Hhas_class.t =
  fun ast_class ->
  let class_attributes =
    Emit_attribute.from_asts ast_class.Ast.c_user_attributes in
  let class_name = Litstr.to_string @@ snd ast_class.Ast.c_name in
  let class_is_trait = ast_class.A.c_kind = Ast.Ctrait in
  let class_uses =
    List.filter_map
      ast_class.A.c_body
      (fun x ->
        match x with
        | A.ClassUse (_, (A.Happly ((_, name), _))) -> Some name
        | _ -> None) in
  let class_enum_type =
    if ast_class.A.c_kind = Ast.Cenum
    then from_enum_type ast_class.A.c_enum
    else None
  in

  let class_is_interface = ast_is_interface ast_class in
  let class_is_abstract = ast_class.A.c_kind = Ast.Cabstract in
  let class_is_final =
    ast_class.A.c_final || class_is_trait || (class_enum_type <> None) in
  let tparams = Emit_body.tparams_to_strings ast_class.A.c_tparams in
  let class_base =
    if class_is_interface then None
    else from_extends
          ~is_enum:(class_enum_type <> None)
          tparams
          ast_class.A.c_extends
  in
  let implements =
    if class_is_interface then ast_class.A.c_extends
    else ast_class.A.c_implements in
  let class_implements = from_implements tparams implements in
  let class_body = ast_class.A.c_body in
  (* TODO: communicate this without looking at the name *)
  let is_closure_class =
    String_utils.string_starts_with (snd ast_class.A.c_name) "Closure$" in
  let has_constructor_or_invoke = List.exists class_body
    (fun elt -> match elt with
                | A.Method { A.m_name; _} ->
                  snd m_name = SN.Members.__construct ||
                  snd m_name = "__invoke" && is_closure_class
                | _ -> false) in
  let additional_methods =
    if has_constructor_or_invoke
    then []
    else [default_constructor ast_class] in
  let class_methods =
    Emit_method.from_asts ast_class (ast_methods class_body) in
  let class_methods = class_methods @ additional_methods in
  let class_properties = List.concat_map class_body from_class_elt_classvars in
  let class_constants = List.concat_map class_body from_class_elt_constants in
  let class_type_constants =
    List.filter_map class_body from_class_elt_typeconsts in
  (* TODO: uses, xhp attr uses, xhp category *)
  Hhas_class.make
    class_attributes
    class_base
    class_implements
    class_name
    class_is_final
    class_is_abstract
    class_is_interface
    class_is_trait
    class_uses
    class_enum_type
    class_methods
    class_properties
    class_constants
    class_type_constants

let from_asts ast_classes =
  List.map ast_classes from_ast
