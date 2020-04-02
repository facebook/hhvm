(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module T = Aast
module SN = Naming_special_names

let emit_file_attributes fa =
  let namespace = fa.T.fa_namespace in
  Emit_attribute.from_asts namespace fa.T.fa_user_attributes

let emit_file_attributes_from_program (ast : Tast.def list) =
  let contains_toplevel_code =
    List.exists ast ~f:(function
        | T.Stmt (_, T.Markup _) -> false
        | T.Stmt _ -> true
        | _ -> false)
  in
  let aux acc node =
    match node with
    | T.FileAttributes fa -> acc @ emit_file_attributes fa
    | _ -> acc
  in
  let attributes = List.fold ast ~init:[] ~f:aux in
  if contains_toplevel_code then
    Hhas_attribute.make SN.UserAttributes.uaHasTopLevelCode [] :: attributes
  else
    attributes
