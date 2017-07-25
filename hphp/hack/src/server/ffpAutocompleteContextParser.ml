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

module Predecessor = struct
  type t =
  | IfWithoutElse
  | TryWithoutFinally
  | OpenBrace
  | Statement
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

let is_function_async (function_object:PositionedSyntax.t) : bool =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  match function_object.syntax with
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
  let rec aux path acc = match path with
    | [] -> acc
    | { syntax = IfStatement { if_else_clause = {
          syntax = Missing; _
      }; _ }; _ } :: t -> aux t IfWithoutElse
    | { syntax = TryStatement { try_finally_clause = {
          syntax = Missing; _
      }; _ }; _ } :: t -> aux t TryWithoutFinally
    | { syntax = MethodishDeclaration _; _ } :: t -> aux t Statement
    | { syntax = Token { kind = LeftBrace; _ }; _ } :: t -> aux t OpenBrace
    | _ :: t -> aux t acc
  in
  aux predecessor NoPredecessor

let make_context
  ~(full_path:PositionedSyntax.t list)
  ~(predecessor:PositionedSyntax.t list)
  : context =
  let predecessor = validate_predecessor predecessor in
  let open PositionedSyntax in
  let open Container in
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
    | { syntax = PositionedSyntax.CompoundStatement _; _} :: t ->
      aux t { acc with closest_parent_container = Container.CompoundStatement }
    | _ :: t -> aux t acc
  in
  aux full_path { initial_context with predecessor }

let get_context_and_stub (syntax_tree:SyntaxTree.t) (offset:int) : context * string =
  let build_context
    ~(parentage:PositionedSyntax.t list)
    ~(predecessor:PositionedSyntax.t list)
    =
    let parentage = List.rev parentage in
    let predecessor = List.rev predecessor in
    make_context ~full_path:parentage ~predecessor
  in

  let open PositionedSyntax in
  let positioned_tree = from_tree syntax_tree in
  let offset = if offset == width positioned_tree then offset - 1 else offset in
  let autocomplete_node_parentage = parentage positioned_tree offset in
  let previous_offset = match autocomplete_node_parentage with
    | [] -> offset - 1
    | autocomplete_child :: _ -> leading_start_offset autocomplete_child - 1
  in
  let predecessor_parentage = parentage positioned_tree previous_offset in
  match autocomplete_node_parentage with
  | [] -> (* This case occurs iff the completion location is the last character in the file *)
    (build_context ~parentage:predecessor_parentage ~predecessor:predecessor_parentage, "")
  | autocomplete_child :: _ ->
    let token_start_offset = start_offset autocomplete_child in
    let token_end_offset = trailing_start_offset autocomplete_child in
    if offset < token_start_offset then
      (* This case handles when the completion location is in the leading trivia of a node *)
      (build_context ~parentage:predecessor_parentage ~predecessor:predecessor_parentage, "")
    else if offset <= token_end_offset then
      (* This case handles when the completion location is in the main token of a node *)
      (build_context ~parentage:autocomplete_node_parentage ~predecessor:predecessor_parentage,
      text autocomplete_child)
    else (* This case handles when the completion location is in the trailing trivia of a node *)
      (build_context
        ~parentage:autocomplete_node_parentage
        ~predecessor:autocomplete_node_parentage,
        "")
