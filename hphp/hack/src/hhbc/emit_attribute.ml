(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hh_core
open Emit_expression

let from_attribute_base namespace attribute_id arguments =
  try
    let attribute_arguments =
      Ast_constant_folder.literals_from_exprs_with_index namespace arguments
    in
    Hhas_attribute.make (snd attribute_id) attribute_arguments
  with Ast_constant_folder.UserDefinedConstant ->
    Emit_fatal.raise_fatal_parse (fst attribute_id)
      "User-defined constants are not allowed in user attribute expressions"

let from_ast : Namespace_env.env -> A.user_attribute -> Hhas_attribute.t =
  fun namespace ast_attr ->
  from_attribute_base namespace ast_attr.A.ua_name ast_attr.A.ua_params

let from_asts namespace ast_attributes =
  List.map ast_attributes (from_ast namespace)

let ast_is_memoize : A.user_attribute -> bool =
  fun ast_attr ->
  snd ast_attr.A.ua_name = Naming_special_names.UserAttributes.uaMemoize

let ast_any_is_memoize : A.user_attribute list -> bool =
  fun ast_attrs ->
  List.exists ast_attrs ast_is_memoize
