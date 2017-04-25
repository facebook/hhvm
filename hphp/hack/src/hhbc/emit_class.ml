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

let xhp_attribute_declaration_method body =
  {
    A.m_kind = [A.Protected; A.Static];
    A.m_tparams = [];
    A.m_constrs = [];
    A.m_name = Pos.none, "__xhpAttributeDeclaration";
    A.m_params = [];
    A.m_body = body;
    A.m_user_attributes = [];
    A.m_ret = None;
    A.m_ret_by_ref = false;
    A.m_fun_kind = A.FSync;
    A.m_span = Pos.none
  }

let emit_xhp_attribute_array xal =
  let p = Pos.none in
  (* Taken from hphp/parser/hphp.y *)
  let hint_to_num = function
    | "string" -> 1
    | "bool" | "boolean" -> 2
    | "int" | "integer" -> 3
    | "mixed" -> 6
    | "enum" -> 7
    | "real" | "float" | "double" -> 8
    (* Regular class names is type 5 *)
    | _ -> 5
  in
  let get_enum_attributes = function
    | None ->
      failwith "Xhp attribute that's supposed to be an enum but not really"
    | Some (_, es) ->
      let turn_to_kv i e = A.AFkvalue ((p, A.Int (p, string_of_int i)), e) in
      p, A.Array (List.mapi ~f:turn_to_kv es)
  in
  let get_attribute_array_values id enumo =
    let type_ = hint_to_num id in
    let type_ident = (p, A.Int (p, string_of_int type_)) in
    let class_name = match type_ with
      (* regular class names is type 5 *)
      | 5 -> (p, A.String (p, Hhbc_alias.normalize id))
      (* enums are type 7 *)
      | 7 -> get_enum_attributes enumo
      | _ -> (p, A.Null)
    in
    class_name, type_ident
  in
  let inner_array ho expo enumo =
    let e = match expo with None -> (p, A.Null) | Some e -> e in
    let class_name, hint = match ho with
      | None when enumo = None
        -> failwith "Xhp attribute must either have a type or be enum"
      | None -> get_attribute_array_values "enum" enumo
      | Some (_, A.Happly ((_, id), [])) -> get_attribute_array_values id enumo
      | _ -> (p, A.Null), (p, A.String (p, "NYI - Xhp attribute hint"))
    in
    (* TODO: What is index 3? *)
    [A.AFkvalue ((p, A.Int (p, "0")), hint);
     A.AFkvalue ((p, A.Int (p, "1")), class_name);
     A.AFkvalue ((p, A.Int (p, "2")), e);
     A.AFkvalue ((p, A.Int (p, "3")), (p, A.Int (p, "0")))]
  in
  let aux xa =
    let ho = Hhas_xhp_attribute.type_ xa in
    let _, (_, name), expo = Hhas_xhp_attribute.class_var xa in
    let enumo = Hhas_xhp_attribute.maybe_enum xa in
    let k = p, A.String (p, SU.Xhp.clean name) in
    let v = p, A.Array (inner_array ho expo enumo) in
    A.AFkvalue (k, v)
  in
  p, A.Array (List.map ~f:aux xal)

(* AST transformations taken from hphp/parser/hphp.y *)
let emit_xhp_attribute_declaration ast_class xal =
  let p = Pos.none in
  let var_dollar_ = p, A.Lvar (p, "$_") in
  let neg_one = p, A.Int (p, "-1") in
  (* static $_ = -1; *)
  let token1 = A.Static_var [p, A.Binop (A.Eq None, var_dollar_, neg_one)] in
  (* if ($_ === -1) {
   *   $_ = array_merge(parent::__xhpAttributeDeclaration(),
   *                    attributes);
   * }
   *)
  let cond = p, A.Binop (A.EQeqeq, var_dollar_, neg_one) in
  let arg1 =
    p, A.Call (
      (p, A.Class_const ((p, "parent"), (p, "__xhpAttributeDeclaration"))),
      [],
      [])
  in
  let args = [arg1; emit_xhp_attribute_array xal] in
  let array_merge_call = p, A.Call ((p, A.Id (p, "array_merge")), args, []) in
  let true_branch =
    [A.Expr (p, A.Binop (A.Eq None, var_dollar_, array_merge_call))]
  in
  let token2 = A.If (cond, true_branch, []) in
  (* return $_; *)
  let token3 = A.Return (p, Some var_dollar_) in
  let body = [token1; token2; token3] in
  let m = xhp_attribute_declaration_method body in
  Emit_method.from_ast ast_class m

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
      (function
        | A.ClassUse (_, (A.Happly ((_, name), _))) -> Some name
        | _ -> None)
  in
  let class_enum_type =
    if ast_class.A.c_kind = Ast.Cenum
    then from_enum_type ast_class.A.c_enum
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
    if not class_is_xhp || class_xhp_attributes = []
    then []
    else [emit_xhp_attribute_declaration ast_class class_xhp_attributes]
  in
  let additional_methods =
    if has_constructor_or_invoke
    then additional_methods
    else additional_methods @ [default_constructor ast_class]
  in
  let class_methods =
    Emit_method.from_asts ast_class (ast_methods class_body) in
  let class_methods = class_methods @ additional_methods in
  let class_properties = List.concat_map class_body from_class_elt_classvars in
  let class_constants = List.concat_map class_body from_class_elt_constants in
  let class_type_constants =
    List.filter_map class_body from_class_elt_typeconsts in
  (* TODO: xhp attr uses, xhp category *)
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
