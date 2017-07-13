(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module MinimalSyntax = Full_fidelity_minimal_syntax
module MinimalToken = Full_fidelity_minimal_token
module TokenKind = Full_fidelity_token_kind
module SyntaxKind = Full_fidelity_syntax_kind

module Container = struct
  (* Set of mutually exclusive contexts. *)
  type t =
  | ClassBody
  | TypeSpecifier
  | TopLevel
  | LambdaBodyExpression
  | CompoundStatement
  | BinaryExpression
  | NoContainer
end
open Container

type predecessor =
  | IfWithoutElse
  | TryWithoutFinally
  | OpenBrace
  | Statement
  | NoPredecessor

type context = {
  closest_parent_container: Container.t;
  predecessor: predecessor;
  inside_switch_body: bool;
  inside_loop_body: bool;
  inside_async_function: bool;
}

let initial_context = {
  closest_parent_container = NoContainer;
  predecessor = NoPredecessor;
  inside_switch_body = false;
  inside_loop_body = false;
  inside_async_function = false;
}

let is_function_async (function_object:MinimalSyntax.t) : bool =
  let open MinimalSyntax in
  let open MinimalToken in
  let open TokenKind in
  match function_object.syntax with
  | FunctionDeclaration {
      function_declaration_header = { syntax = FunctionDeclarationHeader {
        function_async = { syntax = Token {
          kind = Async; _
        }; _ }; _
      }; _ }; _
    }
  | LambdaExpression {
      lambda_async = { syntax = Token {
        kind = Async; _ }; _
      }; _
    } -> true
  | _ -> false

let validate_predecessor (predecessor:MinimalSyntax.t) : predecessor =
  let open MinimalSyntax in
  match syntax predecessor with
  | IfStatement { if_else_clause = {
      syntax = Missing; _
    }; _ } -> IfWithoutElse
  | TryStatement { try_finally_clause = {
      syntax = Missing; _
    }; _ } -> TryWithoutFinally
  | MethodishDeclaration _ -> Statement
  | Token { MinimalToken.kind = TokenKind.LeftBrace; _ } -> OpenBrace
  | _ -> NoPredecessor

let build_context_from_path (full_path:MinimalSyntax.t list) (predecessor:predecessor) : context =
  let open MinimalSyntax in
  let rec aux path acc = match path with
    | [] -> acc
    | { syntax = SimpleTypeSpecifier _; _} :: _ ->
      { acc with closest_parent_container = TypeSpecifier }
    | { syntax = Script _; _} :: t ->
      aux t { acc with closest_parent_container = TopLevel }
    | { syntax = ClassishBody _; _} :: t ->
      aux t { acc with closest_parent_container = ClassBody }
    | { syntax = ForStatement _; _} :: t
    | { syntax = ForeachStatement _; _} :: t
    | { syntax = WhileStatement _; _} :: t
    | { syntax = DoStatement _; _} :: t ->
      aux t { acc with inside_loop_body = true }
    | { syntax = SwitchSection _; _} :: t ->
      aux t { acc with inside_switch_body = true }
    | { syntax = FunctionDeclaration _; _}  as h :: t ->
      aux t { acc with inside_async_function = is_function_async h }
    | { syntax = LambdaExpression _; _} as h :: t ->
      (* If we see a lambda, almost all context is reset, so each field should get consideration
      on if its context flows into the lambda *)
      let acc = {
        closest_parent_container = LambdaBodyExpression;
        predecessor = predecessor;
        inside_switch_body = false;
        inside_loop_body = false;
        inside_async_function = is_function_async h;
      }
      in
      aux t acc
    | { syntax = MinimalSyntax.CompoundStatement _; _} :: t ->
      aux t { acc with closest_parent_container = Container.CompoundStatement }
    | _ :: t -> aux t acc
  in
  aux full_path { initial_context with predecessor }

(**
 * Here's an example of a (simplified) syntax tree:
 *
 * `-- Script
 *    +-- Header
 *    |   `-- ...
 *    `-- Declarations
 *        `-- FunctionDeclaration
 *            +-- Header
 *            |   +-- ...
 *            |   `-- ...
 *            `-- Body
 *                +-- MISSING
 *                `-- QualifiedName
 *
 * This function takes the root of a syntax tree and a position n which
 * corresponds to the nth character of the original file. The parentage is
 * the descent through the syntax tree to the leaf node at position n.
 *
 * The predecessor of a node is the preceding sibling of the node according to
 * the order defined by the children function in full_fidelity_syntax.ml. If a
 * node is the first child of its parent node, then its predecessor is its
 * parent's predecessor.
 *
 * TODO: Add a unit test to enforce children order
 *
 * This predecessor corresponds to the chunk of text immediately before the leaf
 * node at the position we are in. This is useful for tasks such as determining
 * if we are inside a class body or inside the class header or if we are directly
 * following an if statement.
 *
 * The width function returns the span of the current node. At each level, we
 * see if we are inside the leftmost node's width. If not, we discard the left-
 * most node and continue looking at the other nodes in the level. If we *are*
 * inside the leftmost node, then we move to the next level, which is all the
 * children of the current node. Once we have no more children, we have reached
 * a leaf node.
 *
 * In this example, if position is inside the QualifiedName at the bottom,
 * we would expect the function to return:
 * Parentage: [QualifiedName, Body, FunctionDeclaration, Declarations, Script]
 * Predecessor: Some Header
 *
 * TODO: Add a unit test that reflects this example
 **)
let get_parentage_and_predecessor (root:MinimalSyntax.t) (position:int)
 : MinimalSyntax.t list * MinimalSyntax.t option =
  let rec get_next_child ~current_level ~position ~parentage ~predecessor =
    match current_level with
    | [] -> (parentage, predecessor)
    | h :: t ->
      let width = MinimalSyntax.full_width h in
      if position < width then
        get_next_child ~current_level:(MinimalSyntax.children h) ~position
          ~parentage:(h :: parentage) ~predecessor
      else
        let prev = if MinimalSyntax.kind h = SyntaxKind.Missing
          then predecessor
          else (Some h)
        in
        get_next_child ~current_level:t ~position:(position - width) ~parentage
          ~predecessor:prev
  in
  get_next_child ~current_level:[root] ~position ~parentage:[] ~predecessor:None

let make_context (tree:MinimalSyntax.t) (offset:int) : context =
  let (parents, autocomplete_predecessor) =
    get_parentage_and_predecessor tree offset in
  let path = List.rev parents in
  let predecessor = match autocomplete_predecessor with
    | Some p -> validate_predecessor p
    | None -> NoPredecessor
  in
  build_context_from_path path predecessor
