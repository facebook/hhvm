(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
 module Syntax = Full_fidelity_editable_positioned_syntax
 module SyntaxTree = Full_fidelity_syntax_tree
    .WithSyntax(Syntax)

open Syntax

let extract_syntax_children_from_script_text text =
  let env = Full_fidelity_parser_env.make () in
  let source_text = SourceText.make Relative_path.default text in
  let syntax_tree = SyntaxTree.make ~env source_text in
  let root = SyntaxTree.root syntax_tree in
  match syntax root with
    | Script { script_declarations; } ->
      children_from_syntax (syntax script_declarations)
    | _ -> failwith "synthesize_function_renaming_assignment_ast: Invalid parent ast"

let synthesize_renaming_function_assignment_ast original renamed =
  let fmt = format_of_string "$bento_renamed_function_%s='%s';\n" in
  let instruction = Printf.sprintf fmt original renamed in
  let children = extract_syntax_children_from_script_text instruction in
  List.find is_expression_statement children

let synthesize_wrapper_function_declaration_ast name =
  let fmt = format_of_string
    "function %s(mixed ...$x){$GLOBALS['bento_renamed_function_%s'](...$x);}\n" in
  let instruction = Printf.sprintf fmt name name in
  let children = extract_syntax_children_from_script_text instruction in
  List.find is_function_declaration children
