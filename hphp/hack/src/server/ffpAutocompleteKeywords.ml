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
    (context.closest_parent_container = TopLevel ||
    context.closest_parent_container = ClassHeader) &&
    context.predecessor = MarkupSection
    || (* Abstract method *)
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}

let final_keyword = {
  keywords = ["final"];
  is_valid_in_context = begin fun context ->
    (* Final class *)
    (context.closest_parent_container = TopLevel ||
    context.closest_parent_container = ClassHeader) &&
    (context.predecessor = MarkupSection ||
    context.predecessor = KeywordAbstract)
    || (* Final method *)
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}

let implements_keyword = {
  keywords = ["implements"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = ClassHeader ||
    context.closest_parent_container = ClassBody)
    &&
    (context.predecessor = ClassName ||
     context.predecessor = ExtendsList)
  end;
}

let extends_keyword = {
  keywords = ["extends"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = ClassHeader ||
    context.closest_parent_container = ClassBody)
    &&
    context.predecessor = ClassName
  end;
}

let visibility_modifiers = {
  keywords = ["public"; "protected"; "private"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
    || (* After seeing the word final, the parser thinks the next thing should
      be a type specifier *)
    context.closest_parent_container = TypeSpecifier &&
    context.predecessor = KeywordFinal
  end;
}

let static_keyword = {
  keywords = ["static"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
    ||
    context.closest_parent_container = TypeSpecifier &&
    context.predecessor = VisibilityModifier
  end;
}

let async_keyword = {
  keywords = ["async"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
    ||
    context.closest_parent_container = TypeSpecifier &&
    (context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordStatic)
  end;
}

let const_keyword = {
  keywords = ["const"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}

let use_keyword = {
  keywords = ["use"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}

let function_keyword = {
  keywords = ["function"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = ClassBody ||
    context.closest_parent_container = FunctionHeader) &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration ||
    context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordAsync ||
    context.predecessor = KeywordStatic)
    ||
    context.closest_parent_container = TopLevel
  end;
}

let class_keyword = {
  keywords = ["class"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TopLevel
    ||
    context.closest_parent_container = ClassHeader &&
    (context.predecessor = KeywordAbstract ||
    context.predecessor = KeywordFinal)
  end;
}

let interface_keyword = {
  keywords = ["interface"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TopLevel
  end;
}

let declaration_keywords = {
  keywords = ["enum"; "require"; "include"; "require_once"; "include_once";
    "use"; "namespace"; "newtype"; "type"; "trait"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TopLevel
  end;
}

let void_keyword = {
  keywords = ["void"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = TypeSpecifier ||
    context.closest_parent_container = FunctionHeader) &&
    (context.predecessor = TokenColon ||
    context.predecessor = TokenLessThan)
  end;
}

let noreturn_keyword = {
  keywords = ["noreturn"];
  is_valid_in_context = begin fun context ->
    (context.closest_parent_container = TypeSpecifier ||
    context.closest_parent_container = FunctionHeader) &&
    context.predecessor = TokenColon
  end;
}

let primitive_types = {
  keywords = ["array"; "?array"; "arraykey"; "?arraykey"; "bool"; "?bool"; "classname";
  "?classname"; "darray"; "?darray"; "float"; "?float"; "int"; "?int"; "mixed"; "num"; "?num";
  "string"; "?string"; "resource"; "?resource"; "varray"; "?varray"];
  is_valid_in_context = begin fun context ->
    (* Function return type *)
    (context.closest_parent_container = TypeSpecifier ||
    context.closest_parent_container = FunctionHeader) &&
    context.predecessor = TokenColon
    || (* Parameter type *)
    context.closest_parent_container = TypeSpecifier &&
    (context.predecessor = TokenComma ||
    context.predecessor = OpenParenthesis)
    || (* Class property type *)
    context.inside_class_body &&
    context.closest_parent_container = TypeSpecifier &&
    (context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordConst ||
    context.predecessor = KeywordStatic)
    || (* Generic type *)
    context.closest_parent_container = TypeSpecifier &&
    context.predecessor = TokenLessThan
  end;
}

let this_type_keyword = {
  keywords = ["this"; "?this"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TypeSpecifier &&
    context.predecessor = TokenColon &&
    context.inside_class_body
  end;
}

let loop_body_keywords = {
  keywords = ["continue"; "break"];
  is_valid_in_context = begin fun context ->
    context.inside_loop_body
  end;
}

let switch_body_keywords = {
  keywords = ["case"; "default"; "break"];
  is_valid_in_context = begin fun context ->
    context.inside_switch_body
  end;
}

(*
 * TODO: Ideally, await will always be allowed inside a function body. Typing
 * await in a non-async function should either automatically make the function
 * async or suggest this change.
 *)
let async_func_body_keywords = {
  keywords = ["await"];
  is_valid_in_context = begin fun context ->
    context.inside_async_function &&
    context.closest_parent_container = CompoundStatement
  end;
}

(*
 * TODO: Figure out what exactly a postfix expression is and when one is valid
 * or more importantly, invalid.
 *)
let postfix_expressions = {
  keywords = ["clone"; "new"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = CompoundStatement ||
    context.closest_parent_container = LambdaBodyExpression
  end;
}

let general_statements = {
  keywords = ["if"; "do"; "while"; "for"; "foreach"; "try"; "return"; "throw";
    "switch"; "yield"; "echo"; "async"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = CompoundStatement
  end;
}

let if_trailing_keywords = {
  keywords = ["else"; "elseif"];
  is_valid_in_context = begin fun context ->
    context.predecessor = IfWithoutElse
  end;
}

let try_trailing_keywords = {
  keywords = ["catch"; "finally"];
  is_valid_in_context = begin fun context ->
    context.predecessor = TryWithoutFinally
  end;
}

(*
 * According to the spec, vacuous expressions (a function with no side
 * effects is called then has its result discarded) are allowed.
 * TODO: Should we only complete expressions when it makes sense to do so?
 * i.e. Only suggest these keywords in a return statement, as an argument to a
 * function, or on the RHS of an assignment expression.
 *)
let primary_expressions = {
  keywords = ["tuple"; "shape"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = CompoundStatement ||
    context.closest_parent_container = LambdaBodyExpression
  end;
}

let scope_resolution_qualifiers = {
  keywords = ["self"; "parent"; "static"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = CompoundStatement ||
    context.closest_parent_container = LambdaBodyExpression
  end;
}

(*
 * An improperly formatted use body causes the parser to throw an error so we
 * cannot complete these at the moment.
 *)
(*let use_body_keywords = {
  keywords = ["insteadof"; "as"];
  is_valid_in_context = fun _ -> true
}*)

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
  if_trailing_keywords;
  implements_keyword;
  interface_keyword;
  loop_body_keywords;
  noreturn_keyword;
  postfix_expressions;
  primary_expressions;
  primitive_types;
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
