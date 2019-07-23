(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

open Syntax

let rename_token new_name node =
  match syntax node with
  | Token token ->
    let new_token = Token.synthesize_from token (Token.kind token) new_name in
    synthesize_from node (Token new_token)
  | _ -> failwith "rename_token: not a Token"

let rename_function_declaration_header new_name node =
   let header = get_function_declaration_header (syntax node) in
   let {function_name; _;} = header in
   let new_function_name = rename_token new_name function_name in
   let new_header = { header with function_name = new_function_name} in
   let new_header_syntax = from_function_declaration_header new_header in
   let new_header_value = ValueBuilder.value_from_syntax new_header_syntax in
   make new_header_syntax new_header_value

let rename_function_declaration new_name node =
  let declaration = get_function_declaration (syntax node) in
  let {function_declaration_header; _;} = declaration in
  let new_header = rename_function_declaration_header new_name function_declaration_header in
  let new_declaration = { declaration with function_declaration_header = new_header} in
  let new_declaration_syntax = from_function_declaration new_declaration in
  let new_declaration_value = ValueBuilder.value_from_syntax new_declaration_syntax in
  make new_declaration_syntax new_declaration_value

let extract_name_from_function_declaration node =
  let declaration = get_function_declaration (syntax node) in
  let {function_declaration_header; _;} = declaration in
  let header = get_function_declaration_header (syntax function_declaration_header) in
  let {function_name; _;} = header in
  text function_name
