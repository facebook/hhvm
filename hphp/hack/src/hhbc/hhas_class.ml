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
open Hhbc_from_nast

(* Notes:
* require-implements is not represented in HHAS.
* require-extends is not represented in HHAS.
* TODO: c_uses ?
* TODO: c_xhp_attr_uses ?
* TODO: c_xhp_category ?
*)

type t = {
  class_attributes   : Hhas_attribute.t list;
  class_base         : Hhas_type_info.t option;
  class_implements   : Hhas_type_info.t list;
  class_name         : Litstr.id;
  class_is_final     : bool;
  class_is_abstract  : bool;
  class_is_interface : bool;
  class_is_trait     : bool;
  class_is_enum      : bool;
  class_methods      : Hhas_method.t list;
  class_properties   : Hhas_property.t list;
  class_constants    : Hhas_constant.t list;
  class_type_constants : Hhas_type_constant.t list;
}

let make
  class_attributes
  class_base
  class_implements
  class_name
  class_is_final
  class_is_abstract
  class_is_interface
  class_is_trait
  class_is_enum
  class_methods
  class_properties
  class_constants
  class_type_constants =
  {
    class_attributes;
    class_base;
    class_implements;
    class_name;
    class_is_final;
    class_is_abstract;
    class_is_interface;
    class_is_trait;
    class_is_enum;
    class_methods;
    class_properties;
    class_constants;
    class_type_constants
  }

let attributes hhas_class = hhas_class.class_attributes
let base hhas_class = hhas_class.class_base
let implements hhas_class = hhas_class.class_implements
let name hhas_class = hhas_class.class_name
let is_final hhas_class = hhas_class.class_is_final
let is_abstract hhas_class = hhas_class.class_is_abstract
let is_interface hhas_class = hhas_class.class_is_interface
let is_trait hhas_class = hhas_class.class_is_trait
let is_enum hhas_class = hhas_class.class_is_enum
let methods hhas_class = hhas_class.class_methods
let properties hhas_class = hhas_class.class_properties
let constants hhas_class = hhas_class.class_constants
let type_constants hhas_class = hhas_class.class_type_constants

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

let from_extends tparams extends =
  (* TODO: This prints out "extends <"\\Mammal" "\\Mammal" hh_type >"
  instead of "extends Mammal" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  match extends with
  | [] -> None
  | h :: _ -> Some (hint_to_type_info ~always_extended:false tparams h)

let from_implements tparams implements =
  (* TODO: This prints out "implements <"\\IFoo" "\\IFoo" hh_type >"
  instead of "implements IFoo" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  hints_to_type_infos ~always_extended:false tparams implements

let from_class_var cv_kind_list class_var =
  let (_, (_, cv_name), _) = class_var in
  (* TODO: xhp, type, initializer *)
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let property_name = Litstr.to_string @@ cv_name in
  let property_is_private = List.mem cv_kind_list A.Private in
  let property_is_protected = List.mem cv_kind_list A.Protected in
  let property_is_public = List.mem cv_kind_list A.Public in
  let property_is_static = List.mem cv_kind_list A.Static in
  Hhas_property.make
    property_is_private
    property_is_protected
    property_is_public
    property_is_static
    property_name

let from_constant (_hint, name, const_init) =
  (* The type hint is omitted. *)
  match const_init with
  | None -> None (* Abstract constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let constant_name = Litstr.to_string @@ snd name in
    Some (Hhas_constant.make constant_name)

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

let from_class_elt_method ast_class elt =
  match elt with
  | A.Method m -> from_method ast_class m
  | _ -> None

let from_class_elt_classvars elt =
  match elt with
  | A.ClassVars (kind_list, _, cvl) ->
    List.map cvl (from_class_var kind_list)
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

let from_ast : A.class_ -> t =
  fun ast_class ->
  let class_attributes = from_attributes ast_class.A.c_user_attributes in
  let class_name = Litstr.to_string @@ snd ast_class.A.c_name in
  let class_is_trait = ast_class.A.c_kind = Ast.Ctrait in
  let class_is_enum = ast_class.A.c_kind = Ast.Cenum in
  let class_is_interface = ast_is_interface ast_class in
  let class_is_abstract = ast_class.A.c_kind = Ast.Cabstract in
  let class_is_final =
    ast_class.A.c_final || class_is_trait || class_is_enum in
  let tparams = tparams_to_strings ast_class.A.c_tparams in
  let class_base =
    if class_is_interface then None
    else from_extends tparams ast_class.A.c_extends in
  let implements =
    if class_is_interface then ast_class.A.c_extends
    else ast_class.A.c_implements in
  let class_implements = from_implements tparams implements in
  let class_body = ast_class.A.c_body in
  let has_constructor = List.exists class_body
    (fun elt -> match elt with
                | A.Method { A.m_name; _} -> snd m_name = SN.Members.__construct
                | _ -> false) in
  let class_methods =
    if has_constructor then [] else [default_constructor ast_class] in
  let class_methods =
    List.filter_map class_body (from_class_elt_method ast_class)
    @ class_methods in
  let class_properties = List.concat_map class_body from_class_elt_classvars in
  let class_constants = List.concat_map class_body from_class_elt_constants in
  let class_type_constants =
    List.filter_map class_body from_class_elt_typeconsts in
  (* TODO: uses, xhp attr uses, xhp category *)
  make
    class_attributes
    class_base
    class_implements
    class_name
    class_is_final
    class_is_abstract
    class_is_interface
    class_is_trait
    class_is_enum
    class_methods
    class_properties
    class_constants
    class_type_constants

let from_asts ast_classes =
  List.map ast_classes from_ast
