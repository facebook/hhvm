(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*
 * FUTURE IMPROVEMENTS:
 * - Order suggestions by how likely they are to be what the programmer
 *   wishes to do, not just what is valid
 *)

module MinToken = Full_fidelity_minimal_token
module MinimalSyntax = Full_fidelity_minimal_syntax
module TokenKind = Full_fidelity_token_kind
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree

open FfpAutocompleteContextParser
open FfpAutocompleteContextParser.Container
open FfpAutocompleteContextParser.Predecessor
open FfpAutocompleteContextParser.ContextPredicates
open Core

(* Each keyword completion object has a list of keywords and a function that
   takes a context and returns whether or not the list of keywords is valid
   in that context. *)
type keyword_completion = {
  keywords: string list;
  is_valid_in_context: context -> bool;
}

let abstract_keyword = {
  keywords = ["abstract"];
  is_valid_in_context = begin fun context ->
    (* Abstract class *)
    is_top_level_statement_valid context
    || (* Abstract method *)
    is_class_body_declaration_valid context ||
    is_trait_body_declaration_valid context
  end;
}

let final_keyword = {
  keywords = ["final"];
  is_valid_in_context = begin fun context ->
    (* Final class *)
    (context.predecessor = TopLevelDeclaration ||
    context.predecessor = KeywordAbstract)
    ||
    is_top_level_statement_valid context
    || (* Final method *)
    is_class_body_declaration_valid context
    ||
    is_trait_body_declaration_valid context
    || (* Final after other modifiers *)
    context.closest_parent_container = ClassBody &&
    (context.predecessor = KeywordStatic ||
    context.predecessor = VisibilityModifier)
  end;
}

let implements_keyword = {
  keywords = ["implements"];
  is_valid_in_context = begin fun context ->
    (* Class implements interface *)
    (context.closest_parent_container = ClassHeader ||
    context.closest_parent_container = ClassBody) &&
    (context.predecessor = ClassName ||
    context.predecessor = ExtendsList)
    || (* "require implements" inside a trait *)
    context.closest_parent_container = TraitBody &&
    context.predecessor = KeywordRequire
  end;
}

let extends_keyword = {
  keywords = ["extends"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = InterfaceHeader ||
    context.closest_parent_container = InterfaceBody ||
    context.closest_parent_container = ClassHeader ||
    context.closest_parent_container = ClassBody) &&
    context.predecessor = ClassName
    || (* Inside trait/interface body *)
    (context.closest_parent_container = TraitBody ||
    context.closest_parent_container = InterfaceBody) &&
    context.predecessor = KeywordRequire
  end;
}

let visibility_modifiers = {
  keywords = ["public"; "protected"; "private"];
  is_valid_in_context = begin fun context ->
    is_class_body_declaration_valid context
    ||
    is_trait_body_declaration_valid context
    ||
    context.closest_parent_container = ClassBody &&
    context.predecessor = KeywordFinal
  end;
}

let interface_visibility_modifiers = {
  keywords = ["public"];
  is_valid_in_context = is_interface_body_declaration_valid
}

let static_keyword = {
  keywords = ["static"];
  is_valid_in_context = begin fun context ->
    is_class_body_declaration_valid context ||
    is_interface_body_declaration_valid context ||
    is_trait_body_declaration_valid context
    ||
    (context.closest_parent_container = ClassBody ||
    context.closest_parent_container = InterfaceBody ||
    context.closest_parent_container = TraitBody) &&
    context.predecessor = VisibilityModifier
  end;
}

let async_keyword = {
  keywords = ["async"];
  is_valid_in_context = begin fun context ->
    (* Async method *)
    is_class_body_declaration_valid context ||
    is_trait_body_declaration_valid context
    || (* Async method after modifiers *)
    (context.closest_parent_container = ClassBody ||
    context.closest_parent_container = TraitBody) &&
    (context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordFinal ||
    context.predecessor = KeywordStatic )
    || (* Async top level function *)
    is_top_level_statement_valid context
    || (* Async lambda *)
    is_expression_valid context
  end;
}

let const_keyword = {
  keywords = ["const"];
  is_valid_in_context = begin fun context ->
    is_class_body_declaration_valid context ||
    is_interface_body_declaration_valid context
  end;
}

let use_keyword = {
  keywords = ["use"];
  is_valid_in_context = begin fun context ->
    (* use <trait> inside body *)
    is_class_body_declaration_valid context
    || (* use <namespace> at top level *)
    is_top_level_statement_valid context
    (* TODO: "use" for closures *)
  end;
}

let function_keyword = {
  keywords = ["function"];
  is_valid_in_context = begin fun context ->
    (* Class Method *)
    (* "function" is not valid without a visibility modifier, but we still suggest it here since a
       user may wish to write the function before adding the modifier. *)
    is_class_body_declaration_valid context ||
    is_interface_body_declaration_valid context ||
    is_trait_body_declaration_valid context
    || (* Class method, after modifiers *)
    (context.closest_parent_container = ClassBody ||
    context.closest_parent_container = InterfaceBody ||
    context.closest_parent_container = TraitBody ||
    context.closest_parent_container = FunctionHeader) &&
    (context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordAsync ||
    context.predecessor = KeywordStatic ||
    context.predecessor = KeywordFinal)
    || (* Top level function *)
    is_top_level_statement_valid context
    || (* Top level async function *)
    context.closest_parent_container = TopLevel &&
    context.predecessor = KeywordAsync
  end;
}

let class_keyword = {
  keywords = ["class"];
  is_valid_in_context = begin fun context ->
    is_top_level_statement_valid context
    ||
    context.closest_parent_container = ClassHeader &&
    (context.predecessor = KeywordAbstract ||
    context.predecessor = KeywordFinal)
  end;
}

let interface_keyword = {
  keywords = ["interface"];
  is_valid_in_context = begin fun context ->
    is_top_level_statement_valid context
  end;
}

let require_constraint_keyword = {
  keywords = ["require"];
  is_valid_in_context = begin fun context ->
    (* Require inside trait body or interface body *)
    (context.closest_parent_container = TraitBody ||
    context.closest_parent_container = InterfaceBody) &&
    (context.predecessor = TokenLeftBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}


let declaration_keywords = {
  keywords = ["enum"; "require"; "include"; "require_once"; "include_once";
    "namespace"; "newtype"; "trait"; "type"];
  is_valid_in_context = begin fun context ->
    is_top_level_statement_valid context
  end;
}

let void_keyword = {
  keywords = ["void"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = FunctionHeader &&
    (context.predecessor = TokenColon ||
    context.predecessor = TokenLessThan)
  end;
}

let noreturn_keyword = {
  keywords = ["noreturn"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = ClassBody ||
    context.closest_parent_container = FunctionHeader) &&
    context.predecessor = TokenColon
  end;
}

let primitive_types = {
  keywords = ["array"; "arraykey"; "bool"; "classname"; "darray"; "float"; "int"; "mixed"; "num";
  "string"; "resource"; "varray"];
  is_valid_in_context = is_type_valid
}

let this_type_keyword = {
  keywords = ["this"; "?this"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = FunctionHeader &&
    context.predecessor = TokenColon &&
    context.inside_class_body
  end;
}

let loop_body_keywords = {
  keywords = ["continue"; "break"];
  is_valid_in_context = begin fun context ->
    context.inside_loop_body &&
    is_at_beginning_of_new_statement context
  end;
}

let switch_body_keywords = {
  keywords = ["case"; "default"; "break"];
  is_valid_in_context = begin fun context ->
    context.inside_switch_body &&
    is_at_beginning_of_new_statement context
  end;
}

let async_func_body_keywords = {
  keywords = ["await"];
  is_valid_in_context = begin fun context ->
    context.inside_async_function &&
    (context.closest_parent_container = CompoundStatement ||
    context.closest_parent_container = AssignmentExpression)
  end;
}

(*
 * TODO: Figure out what exactly a postfix expression is and when one is valid
 * or more importantly, invalid.
 *)
let postfix_expressions = {
  keywords = ["clone"; "new"];
  is_valid_in_context = begin fun context ->
    is_expression_valid context
  end;
}

let general_statements = {
  keywords = ["if"; "do"; "while"; "for"; "foreach"; "try"; "return"; "throw";
    "switch"; "yield"; "echo"];
  is_valid_in_context = begin fun context ->
    is_at_beginning_of_new_statement context
  end;
}

let if_after_else = {
  keywords = ["if"];
  is_valid_in_context = begin fun context ->
    context.predecessor = KeywordElse &&
    (context.closest_parent_container = CompoundStatement ||
    context.closest_parent_container = IfStatement)
  end;
}

let if_trailing_keywords = {
  keywords = ["else"; "else if"];
  is_valid_in_context = begin fun context ->
    context.predecessor = IfWithoutElse &&
    context.closest_parent_container = CompoundStatement
  end;
}

let try_trailing_keywords = {
  keywords = ["catch"; "finally"];
  is_valid_in_context = begin fun context ->
    context.predecessor = TryWithoutFinally &&
    context.closest_parent_container = CompoundStatement
  end;
}

let primary_expressions = {
  keywords = ["tuple"; "shape"];
  is_valid_in_context = begin fun context ->
    is_expression_valid context
  end;
}

let scope_resolution_qualifiers = {
  keywords = ["self"; "parent"; "static"];
  is_valid_in_context = begin fun context ->
    is_expression_valid context
  end;
}

let keyword_matches: keyword_completion list = [
  abstract_keyword;
  async_keyword;
  async_func_body_keywords;
  class_keyword;
  const_keyword;
  declaration_keywords;
  extends_keyword;
  final_keyword;
  function_keyword;
  general_statements;
  if_after_else;
  if_trailing_keywords;
  implements_keyword;
  interface_keyword;
  interface_visibility_modifiers;
  loop_body_keywords;
  noreturn_keyword;
  postfix_expressions;
  primary_expressions;
  primitive_types;
  require_constraint_keyword;
  scope_resolution_qualifiers;
  static_keyword;
  switch_body_keywords;
  this_type_keyword;
  try_trailing_keywords;
  use_keyword;
  visibility_modifiers;
  void_keyword;
]

let autocomplete_keyword (context:context) : string list =
  let check_keyword_match { keywords; is_valid_in_context } =
    Option.some_if (is_valid_in_context context) keywords
  in
  keyword_matches
  |> List.filter_map ~f:check_keyword_match
  |> List.concat
