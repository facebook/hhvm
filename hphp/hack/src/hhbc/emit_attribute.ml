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

let ast_attribute_name: A.user_attribute -> Litstr.id =
  fun ast_attr ->
  Litstr.to_string @@ snd ast_attr.Ast.ua_name

let from_attribute_base namespace attribute_name arguments =
  try
    let attribute_arguments =
      Ast_constant_folder.literals_from_exprs_with_index namespace arguments
    in
    Hhas_attribute.make attribute_name attribute_arguments
  with Ast_constant_folder.UserDefinedConstant ->
    Emit_fatal.raise_fatal_parse Pos.none
      "User-defined constants are not allowed in user attribute expressions"

let from_ast : Namespace_env.env -> A.user_attribute -> Hhas_attribute.t =
  fun namespace ast_attr ->
  let attribute_name = ast_attribute_name ast_attr in
  from_attribute_base namespace attribute_name ast_attr.A.ua_params

let from_asts namespace ast_attributes =
  List.map ast_attributes (from_ast namespace)

let ast_is_memoize : A.user_attribute -> bool =
  fun ast_attr ->
  "__Memoize" = (ast_attribute_name ast_attr)

let ast_any_is_memoize : A.user_attribute list -> bool =
  fun ast_attrs ->
  List.exists ast_attrs ast_is_memoize
