(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Emit_expression

module TV = Typed_value

let from_attribute_base namespace attribute_id arguments =
  try
    let attribute_arguments =
      Ast_constant_folder.literals_from_exprs namespace arguments
    in
    let fq_id =
      let id = snd attribute_id in
      if String.is_prefix ~prefix:"__" id
      then id (* don't do anything to builtin attributes *)
      else
        let fq_id, _ = Hhbc_id.Class.elaborate_id namespace attribute_id in
        Php_escaping.escape (Hhbc_id.Class.to_raw_string fq_id)
    in
    Hhas_attribute.make fq_id attribute_arguments
  with Ast_constant_folder.UserDefinedConstant ->
    Emit_fatal.raise_fatal_parse (fst attribute_id)
      "User-defined constants are not allowed in user attribute expressions"

let from_ast namespace ast_attr =
  from_attribute_base namespace ast_attr.A.ua_name ast_attr.A.ua_params

let from_asts namespace ast_attributes =
  List.map ast_attributes (from_ast namespace)

let ast_is_memoize ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoize ||
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoizeLSB

let ast_is_memoize_lsb ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoizeLSB

let ast_is_deprecated ast_attr =
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaDeprecated

let ast_any_is_memoize ast_attrs =
  List.exists ast_attrs ast_is_memoize

let ast_any_is_memoize_lsb ast_attrs =
  List.exists ast_attrs ast_is_memoize_lsb

let ast_any_is_deprecated ast_attrs =
  List.exists ast_attrs ast_is_deprecated

(* Adds an __Reified attribute for functions and classes with reified type
 * parameters. The arguments to __Reified are number of type parameters
 * followed by the indicies of these reified type parameters
 *)
let add_reified_attribute attrs params =
  let reified_indices =
    List.filter_mapi params
      ~f:(fun i t -> if t.A.tp_reified then Some i else None) in
  if List.is_empty reified_indices then attrs else
  let data = List.length params :: reified_indices in
  (Hhas_attribute.make "__Reified" @@
    List.map data (fun i -> TV.Int (Int64.of_int i))) :: attrs

let add_reified_parent_attribute attrs = function
  | ((_, Ast.Happly (_, hl))) :: _ ->
    if List.exists hl ~f:(function (_, Ast.Hreified _) -> true | _ -> false)
    then (Hhas_attribute.make "__HasReifiedParent" []) :: attrs else attrs
  | _ -> attrs
