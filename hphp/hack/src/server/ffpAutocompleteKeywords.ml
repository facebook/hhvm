 (**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* lambda_analysis.ml *)

module PosSyntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
open Full_fidelity_syntax_kind
open String_utils
open Core

(* Set of mutually exclusive contexts. *)
type container =
  | ClassBody
  | ClassMethod
  | CompoundStatement
  | LambdaBody
  | TypeSpecifier
  | TopLevel
  | TopLevelFunction
  | BinaryExpression
  | NoContainer

type predecessor =
  | IfWithoutElse
  | TryWithoutFinally
  | OpenBrace
  | Statement
  | NoPredecessor

type context = {
  closest_parent_container: container;
  predecessor: predecessor;
  inside_switch_body: bool;
  inside_loop_body: bool;
}

(* TODO: Do more comprehensive testing using actual Hack files *)

let class_leading_modifiers = ["abstract"; "final"; "abstract final"]
let class_leading_modifiers_context context =
  context.closest_parent_container = TopLevel

let class_trailing_modifiers = ["implements"; "extends"]
let class_trailing_modifiers_context context =
  context.closest_parent_container = ClassBody &&
  not (context.predecessor = OpenBrace)

let visibility_modifiers = ["public"; "protected"; "private"]
let visibility_modifiers_context context =
  context.closest_parent_container = ClassBody &&
  (context.predecessor = OpenBrace || context.predecessor = Statement)

let method_modifiers = ["abstract"; "final"; "static"]
let method_modifiers_context context =
  context.closest_parent_container = ClassBody &&
  (context.predecessor = OpenBrace || context.predecessor = Statement)

let class_body_keywords = ["function"; "const"; "use"]
let class_body_context context =
  context.closest_parent_container = ClassBody &&
  (context.predecessor = OpenBrace || context.predecessor = Statement)
(* TODO: function should be suggested after the method modifiers *)

let declaration_keywords = ["enum"; "require"; "class"; "include";
  "require_once"; "include_once"; "function"; "use"; "interface"; "namespace";
  "newtype"; "type"; "trait"]
let declaration_keywords_context context =
  context.closest_parent_container = TopLevel

let type_specifiers = ["bool"; "int"; "float"; "num"; "string"; "arraykey";
  "void"; "resource"; "this"; "classname"; "mixed"; "noreturn"]
let type_specifiers_context context =
  context.closest_parent_container = TypeSpecifier

let loop_body_keywords = ["continue"; "break"]
let loop_body_context context = context.inside_loop_body

let switch_body_keywords = ["case"; "default"; "break"]
let switch_body_context context = context.inside_switch_body

(*
 * TODO: Figure out what exactly a postfix expression is and when one is valid
 * or more importantly, invalid.
 *)
let postfix_expressions = ["clone"; "new"]
let postfix_expressions_context context =
  context.closest_parent_container = BinaryExpression

let general_statements = ["if"; "do"; "while"; "for"; "foreach"; "try";
  "return"; "throw"; "switch"; "yield"; "echo"; "self"]
let general_statements_context context =
  context.closest_parent_container = CompoundStatement ||
  context.closest_parent_container = LambdaBody ||
  context.closest_parent_container = TopLevelFunction

let if_trailing_keywords = ["else"; "elseif"]
let if_trailing_context context =
  context.predecessor = IfWithoutElse

let try_trailing_keywords = ["catch"; "finally"]
let try_trailing_context context =
  context.predecessor = TryWithoutFinally

(* TODO: Implement these keywords *)
let use_body_keywords = ["insteadof";]

let primary_expressions = ["async"; "tuple"; "shape"]

let infix_functions = ["instanceof"]

let scope_resolution_qualifiers = ["self"; "parent"; "static"]

(*
 * Each pair in this list is a list of keywords paired with a function that
 * takes a context and returns whether or not the list of keywords is valid in
 * this context.
 *)
let keyword_matches: (string list * (context -> bool)) list = [
  (class_leading_modifiers, class_leading_modifiers_context);
  (class_body_keywords, class_body_context);
  (class_trailing_modifiers, class_trailing_modifiers_context);
  (method_modifiers, method_modifiers_context);
  (visibility_modifiers, visibility_modifiers_context);
  (declaration_keywords, declaration_keywords_context);
  (type_specifiers, type_specifiers_context);
  (general_statements, general_statements_context);
  (loop_body_keywords, loop_body_context);
  (switch_body_keywords, switch_body_context);
  (postfix_expressions, postfix_expressions_context);
  (if_trailing_keywords, if_trailing_context);
  (try_trailing_keywords, try_trailing_context);
]

let unimplemented_keywords = ["as";]

let initial_context = {
  closest_parent_container = NoContainer;
  predecessor = NoPredecessor;
  inside_switch_body = false;
  inside_loop_body = false;
}

let process_path full_path predecessor =
  let initial_context = { initial_context with predecessor; } in
  let rec aux acc path = match path with
    | [QualifiedNameExpression; Token]
    | [ErrorSyntax; Token] -> acc
    | [SimpleTypeSpecifier; Token] ->
        { acc with closest_parent_container = TypeSpecifier }
    | Script :: SyntaxList :: t ->
        aux { acc with closest_parent_container = TopLevel } t
    | ClassishBody :: SyntaxList :: t ->
        aux { acc with closest_parent_container = ClassBody } t
    | ForStatement :: t
    | ForeachStatement :: t
    | WhileStatement :: t
    | DoStatement :: t -> aux { acc with inside_loop_body = true; } t
    | FunctionDeclaration :: t ->
        aux { acc with closest_parent_container = TopLevelFunction } t
    | SwitchSection :: t ->
        aux { acc with inside_switch_body = true } t
    | SyntaxKind.BinaryExpression :: t ->
        aux { acc with closest_parent_container = BinaryExpression } t
    | LambdaExpression :: t ->
        aux { acc with closest_parent_container = LambdaBody;
                       inside_switch_body = false;
                       inside_loop_body = false } t
    | [] -> acc
    | _ :: t -> aux acc t
  in
  aux initial_context full_path

let get_context_keywords path predecessor =
  let context = process_path path predecessor in
  let keywords = List.filter_map keyword_matches
    ~f:begin fun (keywords, is_valid) ->
    Option.some_if (is_valid context) keywords
  end in
  List.concat keywords

let validate_predecessor predecessor =
  match (PosSyntax.kind predecessor), (PosSyntax.children predecessor) with
  | IfStatement, [_;_;_;_;_;_; if_else_clause]
      when PosSyntax.kind if_else_clause = Missing -> IfWithoutElse
  | TryStatement, [_;_;_; try_finally_clause]
      when PosSyntax.kind try_finally_clause = Missing -> TryWithoutFinally
  | MethodishDeclaration, _ -> Statement
  | Token, _ when (PosSyntax.text predecessor) = "{" -> OpenBrace
  | _, _ -> NoPredecessor

let autocomplete_keyword ~context ~predecessor ~stub =
  let predecessor = match predecessor with
    | Some p -> validate_predecessor p
    | None -> NoPredecessor
  in
  let possibilities = get_context_keywords context predecessor in
  List.filter ~f:(fun x -> string_starts_with x stub) possibilities
