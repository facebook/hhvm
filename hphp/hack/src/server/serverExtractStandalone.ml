(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Core_kernel
open Ast

exception FunctionNotFound

let extract_function_body func =
  let fname_pos = match Decl_provider.get_fun func with
    | Some f -> f.ft_pos
    | None -> raise FunctionNotFound in
  let rel_filename = Pos.filename fname_pos in
  let abs_filename = Pos.filename (Pos.to_absolute fname_pos) in
  let file_content = In_channel.read_all abs_filename in
  match Ast_provider.find_fun_in_file rel_filename func with
    | Some ast_function ->
      let pos = ast_function.f_span in
      let include_first_whsp = Pos.merge (Pos.first_char_of_line pos) pos in
      Pos.get_text_from_pos file_content include_first_whsp
    | None -> raise FunctionNotFound

let go function_name =
  try extract_function_body function_name with FunctionNotFound -> "Function not found!"
