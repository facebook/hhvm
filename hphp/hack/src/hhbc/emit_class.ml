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

module SU = Hhbc_string_utils

let ast_is_interface ast_class =
  ast_class.A.c_kind = Ast.Cinterface

let make_86method ~name ~params ~is_static ~is_private ~is_abstract body =
  let method_attributes = [] in
  let params, instrs =
    Label_rewriter.relabel_function params body in
  let method_is_final = false in
  let method_is_private = is_private in
  let method_is_protected = false in
  let method_is_public = not is_private in
  let method_return_type = None in
  let method_decl_vars = [] in
  let method_num_iters = 0 in
  let method_num_cls_ref_slots = 0 in
  let method_is_async = false in
  let method_is_generator = false in
  let method_is_pair_generator = false in
  let method_is_closure_body = false in
  let method_is_memoize_wrapper = false in
  let method_body = Hhas_body.make
    instrs
    method_decl_vars
    method_num_iters
    method_num_cls_ref_slots
    method_is_memoize_wrapper
    params
    method_return_type in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    is_static
    method_is_final
    is_abstract
    (Hhbc_id.Method.from_ast_name name)
    method_body
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_extends ~namespace ~is_enum _tparams extends =
  if is_enum
  then Some (Hhbc_id.Class.from_raw_string "HH\\BuiltinEnum") else
  match extends with
  | [] -> None
  | h :: _ -> Some (hint_to_class ~namespace h)

let from_implements ~namespace implements =
  List.map implements (hint_to_class ~namespace)

let from_constant ns (_hint, name, const_init) =
  (* The type hint is omitted. *)
  match const_init with
  | None -> None (* Abstract constants are omitted *)
  | Some init ->
    let constant_name = Litstr.to_string @@ snd name in
    let constant_value, initializer_instrs =
      match Ast_constant_folder.expr_to_opt_typed_value ns init with
      | Some v ->
        v, None
      | None ->
        Typed_value.Uninit, Some (Emit_expression.from_expr init) in
    Some (Hhas_constant.make constant_name constant_value initializer_instrs)

let from_constants ns ast_constants =
  List.filter_map ast_constants (from_constant ns)

let from_type_constant ast_type_constant =
  match ast_type_constant.A.tconst_type with
  | None -> None (* Abstract type constants are omitted *)
  | Some init ->
    (* TODO: Deal with the constraint *)
    let type_constant_name = Litstr.to_string @@
      snd ast_type_constant.A.tconst_name
    in
    let type_constant_initializer =
      Emit_type_constant.hint_to_type_constant init
    in
    Some (Hhas_type_constant.make type_constant_name type_constant_initializer)

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

let from_class_elt_constants ns elt =
  match elt with
  | A.Const(hint_opt, l) ->
    List.filter_map l (fun (id, e) -> from_constant ns (hint_opt, id, Some e))
  | _ -> []

let from_class_elt_typeconsts elt =
  match elt with
  | A.TypeConst tc -> from_type_constant tc
  | _ -> None

let from_enum_type ~namespace opt =
  match opt with
  | Some e ->
    let type_info_user_type =
      Some (Emit_type_hint.fmt_hint ~namespace ~tparams:[] e.A.e_base) in
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
  let class_name, _ =
    Hhbc_id.Class.elaborate_id
    ast_class.Ast.c_namespace ast_class.Ast.c_name in
  let class_is_trait = ast_class.A.c_kind = Ast.Ctrait in
  let class_uses =
    List.filter_map
      ast_class.A.c_body
      (function
        | A.ClassUse (_, (A.Happly ((_, name), _))) -> Some name
        | _ -> None)
  in
  let class_enum_type =
    if ast_class.A.c_kind = Ast.Cenum
    then from_enum_type ast_class.A.c_namespace ast_class.A.c_enum
    else None
  in
  let class_is_xhp = ast_class.A.c_is_xhp in
  let class_xhp_attributes =
    List.filter_map
      ast_class.A.c_body
      (function
        | A.XhpAttr (ho, cv, b, eo) -> Some (Hhas_xhp_attribute.make ho cv b eo)
        | _ -> None)
  in
  let class_xhp_use_attributes =
    List.filter_map
      ast_class.A.c_body
      (function
        | A.XhpAttrUse h -> Some h
        | _ -> None)
  in
  let class_xhp_children =
    List.filter_map
      ast_class.A.c_body
      (function
        | A.XhpChild sl -> Some sl
        | _ -> None)
  in
  let class_xhp_categories =
    List.concat @@
    List.filter_map
      ast_class.A.c_body
      (function
        | A.XhpCategory sl -> Some sl
        | _ -> None)
  in
  let class_is_interface = ast_is_interface ast_class in
  let class_is_abstract = ast_class.A.c_kind = Ast.Cabstract in
  let class_is_final =
    ast_class.A.c_final || class_is_trait || (class_enum_type <> None) in
  let tparams = Emit_body.tparams_to_strings ast_class.A.c_tparams in
  let namespace = ast_class.Ast.c_namespace in
  let class_base =
    if class_is_interface then None
    else from_extends
          ~namespace
          ~is_enum:(class_enum_type <> None)
          tparams
          ast_class.A.c_extends
  in
  let implements =
    if class_is_interface then ast_class.A.c_extends
    else ast_class.A.c_implements in
  let class_implements = from_implements ~namespace implements in
  let class_body = ast_class.A.c_body in
  (* TODO: communicate this without looking at the name *)
  let is_closure_class =
    String_utils.string_starts_with (snd ast_class.A.c_name) "Closure$" in
  let has_constructor_or_invoke = List.exists class_body
    (fun elt -> match elt with
                | A.Method { A.m_name; _} ->
                  snd m_name = SN.Members.__construct ||
                  snd m_name = "__invoke" && is_closure_class
                | _ -> false)
  in
  let additional_methods = [] in
  let additional_methods =
    if not class_is_xhp || class_xhp_categories = []
    then additional_methods
    else additional_methods
      @ [Emit_xhp.from_category_declaration ast_class class_xhp_categories]
  in
  let additional_methods =
    if not class_is_xhp || class_xhp_children = []
    then additional_methods
    else additional_methods
      @ [Emit_xhp.from_children_declaration ast_class class_xhp_children]
  in
  let additional_methods =
    if not class_is_xhp ||
      (class_xhp_attributes = [] && class_xhp_use_attributes = [])
    then additional_methods
    else additional_methods
      @ [Emit_xhp.from_attribute_declaration
          ast_class
          class_xhp_attributes
          class_xhp_use_attributes]
  in
  Label.reset_label ();
  let class_properties = List.concat_map class_body from_class_elt_classvars in
  let class_constants =
    List.concat_map class_body (from_class_elt_constants namespace) in
  let pinit_methods =
    if List.exists class_properties
      (fun p -> Option.is_some (Hhas_property.initializer_instrs p)
                && not (Hhas_property.is_static p))
    then
      let instrs = gather @@ List.filter_map class_properties
        (fun p -> if Hhas_property.is_static p
                  then None else Hhas_property.initializer_instrs p) in
      let instrs = gather [instrs; instr_null; instr_retc] in
      [make_86method
        ~name:"86pinit"
        ~params:[]
        ~is_static:true
        ~is_private:true
        ~is_abstract:false
        instrs]
    else
      [] in
  let sinit_methods =
    if List.exists class_properties
      (fun p -> Option.is_some (Hhas_property.initializer_instrs p)
                && (Hhas_property.is_static p))
    then
      let instrs = gather @@ List.filter_map class_properties
        (fun p -> if not (Hhas_property.is_static p)
                  then None else Hhas_property.initializer_instrs p) in
      let instrs = gather [instrs; instr_null; instr_retc] in
      [make_86method
        ~name:"86sinit"
        ~params:[]
        ~is_static:true
        ~is_private:true
        ~is_abstract:false
        instrs]
    else
      [] in
  let initialized_class_constants = List.filter_map class_constants
      (fun p -> match Hhas_constant.initializer_instrs p with
          | None -> None
          | Some instrs -> Some (Hhas_constant.name p, instrs)) in
  let cinit_methods =
    if List.is_empty initialized_class_constants then []
    else
      let return_label = Label.next_regular () in
      let rec make_cinit_instrs cs =
        match cs with
        | [] ->
          instr_retc
        | (name, instrs) :: cs ->
          if List.is_empty cs
          then
            gather [
              instrs;
              instr_label return_label;
              make_cinit_instrs cs
            ]
          else
            let label = Label.next_regular () in
            gather [
              instr_cgetl (Local.Named "$constName");
              instr_string name;
              instr_eq;
              instr_jmpz label;
              instrs;
              instr_jmp return_label;
              instr_label label;
              make_cinit_instrs cs;
            ] in
      let instrs = make_cinit_instrs initialized_class_constants in
      let params = [Hhas_param.make "$constName" false None None] in
      [make_86method
        ~name:"86cinit"
        ~params
        ~is_static:true
        ~is_private:true
        ~is_abstract:false
        instrs] in
  let ctor_methods =
    if has_constructor_or_invoke then []
    else
      let instrs = gather [instr_null; instr_retc] in
      [make_86method
        ~name:"86ctor"
        ~params:[]
        ~is_static:false
        ~is_private:false
        ~is_abstract:class_is_interface
        instrs] in
  let additional_methods =
    additional_methods @
    ctor_methods @ pinit_methods @ sinit_methods @ cinit_methods in
  let class_methods =
    Emit_method.from_asts ast_class (ast_methods class_body) in
  let class_methods = class_methods @ additional_methods in
  let class_type_constants =
    List.filter_map class_body from_class_elt_typeconsts in
  Hhas_class.make
    class_attributes
    class_base
    class_implements
    class_name
    class_is_final
    class_is_abstract
    class_is_interface
    class_is_trait
    class_is_xhp
    class_uses
    class_enum_type
    class_methods
    class_properties
    class_constants
    class_type_constants

let from_asts ast_classes =
  List.map ast_classes from_ast
