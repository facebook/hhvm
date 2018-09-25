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

let add_reified_attribute attrs params =
  let reified_tparams =
    List.filter_map params
      ~f:(function (_, _, _, false) -> None | (_, (_, s), _, true) -> Some s) in
  if List.length reified_tparams = 0 then attrs else
  (Hhas_attribute.make "__Reified" @@
    List.concat_mapi reified_tparams
      (fun index p -> [TV.Int (Int64.of_int index); TV.String p])) :: attrs

let add_reified_parent_attribute attrs = function
  | ((_, Ast.Happly (_, hl))) :: _ ->
    if List.exists hl ~f:(function (_, Ast.Hreified _) -> true | _ -> false)
    then (Hhas_attribute.make "__HasReifiedParent" []) :: attrs else attrs
  | _ -> attrs
