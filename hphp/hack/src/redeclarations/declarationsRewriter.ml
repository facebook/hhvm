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
 * Use the given map reference to count the number of appearances of the
 * tracked entities, and modify it (as a side effect) when needed.
 *)
let rename_top_level_declarations map root =
  match syntax root with
  | FunctionDeclaration _ ->
    let function_name = extract_name_from_function_declaration root in
    let redeclaration_no_opt = FunctionsDictionary.find_opt function_name !map in
    let redeclaration_no = Core_kernel.Option.value ~default:1 redeclaration_no_opt in
    let new_name = String.concat "_"
      ["bento_function"
      ; function_name
      ; string_of_int redeclaration_no] in
    let map = FunctionsDictionary.add function_name (redeclaration_no + 1) map;
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
