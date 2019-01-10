(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel

let emit_file_attributes fa =
  let namespace = fa.Ast.fa_namespace in
  Emit_attribute.from_asts namespace fa.Ast.fa_user_attributes

let emit_file_attributes_from_program ast =
  let attrs = List.fold ast ~init:[] ~f:(fun acc ast_node ->
    match ast_node with
    | Ast.FileAttributes attrs -> acc @ emit_file_attributes attrs
    | _ -> acc) in
  attrs
