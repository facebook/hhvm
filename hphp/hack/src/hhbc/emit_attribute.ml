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
open Hhbc_from_nast

let ast_attribute_name: A.user_attribute -> Litstr.id =
  fun ast_attr ->
  Litstr.to_string @@ snd ast_attr.Ast.ua_name

let from_attribute_base attribute_name arguments =
  let attribute_arguments = literals_from_exprs_with_index arguments in
  Hhas_attribute.make attribute_name attribute_arguments

let from_ast : A.user_attribute -> Hhas_attribute.t =
  fun ast_attr ->
  let attribute_name = ast_attribute_name ast_attr in
  from_attribute_base attribute_name ast_attr.A.ua_params

let from_asts ast_attributes =
  (* The list of attributes is reversed in the A. *)
  List.map (List.rev ast_attributes) from_ast

let ast_is_memoize : A.user_attribute -> bool =
  fun ast_attr ->
  "__Memoize" = (ast_attribute_name ast_attr)

let ast_any_is_memoize : A.user_attribute list -> bool =
  fun ast_attrs ->
  List.exists ast_attrs ast_is_memoize
