(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module FunctionsDictionary = Map.Make(String)
module Syntax = Full_fidelity_editable_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree
   .WithSyntax(Syntax)

module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)

open RedeclarationSyntaxSynthesizer
open SyntaxTreeUtility

open Syntax

(**
 * Create a modified node, based on the given node, such that the redeclared
 * functions (TODO: classes declarations and others) will be renamed.
 *
 * In the given map there should the number of appearances for each declaration.
 * Update it by returning a (new_map, Rewriter.Replace/Keep) tuple.
 *)
let rename_top_level_declarations _ root map =
  match syntax root with
  | FunctionDeclaration _ ->
    let function_name = extract_name_from_function_declaration root in
    let redeclaration_no_opt = FunctionsDictionary.find_opt function_name map in
    let redeclaration_no = Core_kernel.Option.value ~default:1 redeclaration_no_opt in
    let new_name = String.concat "_"
      ["bento_function"
      ; function_name
      ; string_of_int redeclaration_no] in
    let map = FunctionsDictionary.add function_name (redeclaration_no + 1) map in
    let renamed_function_declaration = rename_function_declaration new_name root in
    let assignment =
      synthesize_renaming_function_assignment_ast function_name new_name in
    let list' =
      if redeclaration_no = 1 then
        [ synthesize_wrapper_function_declaration_ast function_name
        ; renamed_function_declaration
        ; assignment;]
      else [renamed_function_declaration; assignment;] in
    let list_syntax = SyntaxList list' in
    let list_value = ValueBuilder.value_from_syntax list_syntax in
    (map, Rewriter.Replace (make list_syntax list_value))
  | _ -> (map, Rewriter.Keep)

(**
 * Reads a block of code from standard input until the line "_CODE_BLOCK_END_"
 * is encountered.
 *)
let read_code_block() =
  let rec read_lines acc =
    let line = input_line stdin in
    if line = "_CODE_BLOCK_END_" then acc
    else read_lines (line :: acc)
  in
  let reversed_lines = "" :: (read_lines []) in
  let lines = List.rev reversed_lines in
  String.concat "\n" lines

(**
 * Handle the given code block, renaming all functions and keeping track of
 * each redeclaration based on the given map.
 *)
let handle_code_block map code_block =
  let env = Full_fidelity_parser_env.make () in
  let source_text = SourceText.make Relative_path.default code_block in
  let syntax_tree = SyntaxTree.make ~env source_text in
  let root = SyntaxTree.root syntax_tree in
  let (map, new_root) = Rewriter.parented_aggregating_rewrite_post
    rename_top_level_declarations root map in
  print_endline (Core_kernel.Option.value ~default:"" (extract_text new_root));
  print_endline "_CODE_BLOCK_END_";
  map

let start () =
  let rec loop map =
    try
      let map = handle_code_block map (read_code_block ()) in
      loop map
      with End_of_file -> ()
    in
  loop FunctionsDictionary.empty
