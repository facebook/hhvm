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

let from_attribute_base attribute_name arguments =
  let attribute_arguments = literals_from_exprs_with_index arguments in
  Hhas_attribute.make attribute_name attribute_arguments

let from_ast : A.user_attribute -> Hhas_attribute.t =
  fun ast_attr ->
  let attribute_name = Litstr.to_string @@ snd ast_attr.Ast.ua_name in
  from_attribute_base attribute_name ast_attr.A.ua_params

let from_asts ast_attributes =
  (* The list of attributes is reversed in the A. *)
  List.map (List.rev ast_attributes) from_ast
