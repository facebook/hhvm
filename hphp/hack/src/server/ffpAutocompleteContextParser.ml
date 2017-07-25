(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module PositionedToken = Full_fidelity_positioned_token
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
module TokenKind = Full_fidelity_token_kind

open Core

module Container = struct
  (* Set of mutually exclusive contexts. *)
  type t =
  | BinaryExpression
  | ClassBody
  | CompoundStatement
  | LambdaBodyExpression
  | TopLevel
  | TypeSpecifier
  | NoContainer
end

module Predecessor = struct
  type t =
  | IfWithoutElse
  | OpenBrace
  | Statement
  | TryWithoutFinally
  | NoPredecessor
end

type context = {
  closest_parent_container: Container.t;
  predecessor: Predecessor.t;
  inside_switch_body: bool;
  inside_loop_body: bool;
  inside_async_function: bool;
}

let initial_context = {
  closest_parent_container = Container.NoContainer;
  predecessor = Predecessor.NoPredecessor;
  inside_switch_body = false;
  inside_loop_body = false;
  inside_async_function = false;
}

let is_function_async (function_object:PositionedSyntax.syntax) : bool =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  match function_object with
  | FunctionDeclaration {
      function_declaration_header = { syntax = FunctionDeclarationHeader {
        function_async = async; _
      }; _ }; _
    }
  | LambdaExpression { lambda_async = async; _ } ->
    is_specific_token Async async
  | _ -> false

let validate_predecessor (predecessor:PositionedSyntax.t list) : Predecessor.t =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  let open Predecessor in
  let classify_syntax_as_predecessor node = match syntax node with
    | IfStatement { if_else_clause = {
          syntax = Missing; _
      }; _ } -> Some IfWithoutElse
    | TryStatement { try_finally_clause = {
          syntax = Missing; _
      }; _ } -> Some TryWithoutFinally
    | MethodishDeclaration _ -> Some Statement
    | Token { kind = LeftBrace; _ } -> Some OpenBrace
    | _ -> None
  in
  predecessor
  |> List.find_map ~f:classify_syntax_as_predecessor
  |> Option.value ~default:NoPredecessor

let make_context
  ~(full_path:PositionedSyntax.t list)
  ~(predecessor:PositionedSyntax.t list)
  : context =
  let predecessor = validate_predecessor predecessor in
  let open PositionedSyntax in
  let open Container in
  let check_node node acc = match syntax node with
    | SimpleTypeSpecifier _ ->
      { acc with closest_parent_container = TypeSpecifier }
    | Script _ ->
      { acc with closest_parent_container = TopLevel }
    | ClassishBody _ ->
      { acc with closest_parent_container = ClassBody }
    | ForStatement _
    | ForeachStatement _
    | WhileStatement _
    | DoStatement _ ->
      { acc with inside_loop_body = true }
    | SwitchSection _ ->
      { acc with inside_switch_body = true }
    | FunctionDeclaration _ as func ->
      { acc with inside_async_function = is_function_async func }
    | LambdaExpression _ as lambda ->
      (* If we see a lambda, almost all context is reset, so each field should get consideration
      on if its context flows into the lambda *)
      {
        closest_parent_container = LambdaBodyExpression;
        predecessor = predecessor;
        inside_switch_body = false;
        inside_loop_body = false;
        inside_async_function = is_function_async lambda;
      }
    | PositionedSyntax.CompoundStatement _ ->
      { acc with closest_parent_container = Container.CompoundStatement }
    | _ -> acc
  in
  List.fold_right ~f:check_node ~init:{ initial_context with predecessor } full_path

let get_context_and_stub (syntax_tree:SyntaxTree.t) (offset:int) : context * string =
  let open PositionedSyntax in
  let positioned_tree = from_tree syntax_tree in
  let offset = if offset = width positioned_tree then offset - 1 else offset in
  let autocomplete_node_parentage = parentage positioned_tree offset in
  let previous_offset = match autocomplete_node_parentage with
    | [] -> offset - 1
    | autocomplete_child :: _ -> leading_start_offset autocomplete_child - 1
  in
  let predecessor_parentage = parentage positioned_tree previous_offset in
  match autocomplete_node_parentage with
  | [] -> (* This case occurs iff the completion location is the last character in the file *)
    (make_context ~full_path:predecessor_parentage ~predecessor:predecessor_parentage, "")
  | autocomplete_child :: _ ->
    let token_start_offset = start_offset autocomplete_child in
    let token_end_offset = trailing_start_offset autocomplete_child in
    if offset < token_start_offset then
      (* This case handles when the completion location is in the leading trivia of a node *)
      (make_context ~full_path:predecessor_parentage ~predecessor:predecessor_parentage, "")
    else if offset <= token_end_offset then
      (* This case handles when the completion location is in the main token of a node *)
      (make_context ~full_path:autocomplete_node_parentage ~predecessor:predecessor_parentage,
      text autocomplete_child)
    else (* This case handles when the completion location is in the trailing trivia of a node *)
      (make_context
        ~full_path:autocomplete_node_parentage
        ~predecessor:autocomplete_node_parentage,
        "")
