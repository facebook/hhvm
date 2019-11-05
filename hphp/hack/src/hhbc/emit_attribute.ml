(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module TV = Typed_value
module A = Aast

let from_attribute_base namespace (p, attr_name) arguments =
  try
    let attribute_arguments =
      Ast_constant_folder.literals_from_exprs namespace arguments
    in
    let fq_id =
      if String.is_prefix ~prefix:"__" attr_name then
        attr_name
      (* don't do anything to builtin attributes *)
      else
        let id = Hhbc_id.Class.(from_ast_name attr_name |> to_raw_string) in
        Php_escaping.escape id
    in
    Hhas_attribute.make fq_id attribute_arguments
  with Ast_constant_folder.UserDefinedConstant ->
    Emit_fatal.raise_fatal_parse
      p
      "User-defined constants are not allowed in user attribute expressions"

let from_ast namespace ast_attr =
  from_attribute_base namespace ast_attr.A.ua_name ast_attr.A.ua_params

let from_asts namespace ast_attributes =
  List.map ast_attributes ~f:(from_ast namespace)

let ast_is_memoize ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoize
  || snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoizeLSB

let ast_is_memoize_lsb ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoizeLSB

let ast_is_deprecated ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaDeprecated

let ast_any_is_memoize ast_attrs = List.exists ast_attrs ast_is_memoize

let ast_any_is_memoize_lsb ast_attrs = List.exists ast_attrs ast_is_memoize_lsb

let ast_any_is_deprecated ast_attrs = List.exists ast_attrs ast_is_deprecated

(* Adds an __Reified attribute for functions and classes with reified type
 * parameters. The arguments to __Reified are number of type parameters
 * followed by the indicies of these reified type parameters and whether they
 * are soft reified or not
 *)
let add_reified_attribute attrs (params : Tast.tparam list) =
  let is_soft =
    List.exists ~f:(function { A.ua_name = n; _ } -> snd n = "__Soft")
  in
  let is_warn =
    List.exists ~f:(function { A.ua_name = n; _ } -> snd n = "__Warn")
  in
  let reified_data =
    List.filter_mapi params ~f:(fun i t ->
        if t.A.tp_reified = A.Erased then
          None
        else
          Some
            (i, is_soft t.A.tp_user_attributes, is_warn t.A.tp_user_attributes))
  in
  if List.is_empty reified_data then
    attrs
  else
    let int_of_bool b =
      if b then
        1
      else
        0
    in
    let data =
      List.concat_map reified_data ~f:(fun (i, is_soft, is_warn) ->
          [
            TV.Int (Int64.of_int i);
            TV.Int (Int64.of_int @@ int_of_bool is_soft);
            TV.Int (Int64.of_int @@ int_of_bool is_warn);
          ])
    in
    let data = TV.Int (Int64.of_int (List.length params)) :: data in
    Hhas_attribute.make "__Reified" data :: attrs

let add_reified_parent_attribute env attrs extends =
  match extends with
  | (_, A.Happly (_, hl)) :: _ ->
    if Emit_expression.has_non_tparam_generics env hl then
      Hhas_attribute.make "__HasReifiedParent" [] :: attrs
    else
      attrs
  | _ -> attrs
