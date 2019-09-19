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

let emit_file_attributes fa =
  let namespace = fa.T.fa_namespace in
  Emit_attribute.from_asts namespace fa.T.fa_user_attributes

let emit_file_attributes_from_program (ast : Tast.def list) =
  let aux acc node =
    match node with
    | T.FileAttributes fa -> acc @ emit_file_attributes fa
    | _ -> acc
  in
  List.fold ast ~init:[] ~f:aux
