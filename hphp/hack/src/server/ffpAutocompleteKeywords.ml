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
 * - Order sensitive suggestions, i.e. after public suggest the word function
 *)

module MinToken = Full_fidelity_minimal_token
module MinimalSyntax = Full_fidelity_minimal_syntax
module TokenKind = Full_fidelity_token_kind
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree

open FfpAutocompleteContextParser
open FfpAutocompleteContextParser.Container
open FfpAutocompleteContextParser.Predecessor
open String_utils
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

(*
 * TODO: Complete these after other modifiers, i.e. "public static asy" should
 * suggest "async" as a completion.
 *)
let method_modifiers = {
  keywords = ["static"; "async"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}

let class_body_keywords = {
  keywords = ["function"; "const"; "use"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = ClassBody &&
    (context.predecessor = OpenBrace ||
    context.predecessor = ClassBodyDeclaration)
  end;
}
(* TODO: function should be suggested after the method modifiers *)

let declaration_keywords = {
  keywords = ["enum"; "require"; "class"; "include";
  "require_once"; "include_once"; "function"; "use"; "interface"; "namespace";
  "newtype"; "type"; "trait"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TopLevel
  end;
}

let type_specifiers = {
  keywords = ["bool"; "int"; "float"; "num"; "string"; "arraykey";
  "void"; "resource"; "this"; "classname"; "mixed"; "noreturn"];
  is_valid_in_context = begin fun context ->
    context.closest_parent_container = TypeSpecifier
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
  keywords = ["if"; "do"; "while"; "for"; "foreach"; "try";
  "return"; "throw"; "switch"; "yield"; "echo"; "async"];
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

(*
 * Each pair in this list is a list of keywords paired with a function that
 * takes a context and returns whether or not the list of keywords is valid in
 * this context.
 *)
let keyword_matches: keyword_completion list = [
  abstract_keyword;
  final_keyword;
  class_body_keywords;
  extends_keyword;
  implements_keyword;
  method_modifiers;
  visibility_modifiers;
  declaration_keywords;
  type_specifiers;
  general_statements;
  loop_body_keywords;
  switch_body_keywords;
  postfix_expressions;
  primary_expressions;
  scope_resolution_qualifiers;
  if_trailing_keywords;
  try_trailing_keywords;
  async_func_body_keywords;
]

let autocomplete_keyword (context:context) (stub:string) : string list =
  let possibilities = List.filter_map keyword_matches
    ~f:begin fun { keywords; is_valid_in_context } ->
    Option.some_if (is_valid_in_context context) keywords
  end in
  possibilities
    |> List.concat
    |> List.filter ~f:(fun x -> string_starts_with x stub)
