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
  | AfterDoubleColon
  | AfterRightArrow
  | AssignmentExpression
  | ClassBody
  | ClassHeader
  | CompoundStatement
  | ConstantDeclaration
  | FunctionCallArgumentList
  | FunctionHeader
  | IfStatement
  | InterfaceBody
  | InterfaceHeader
  | LambdaBodyExpression
  | TopLevel
  | TraitBody
  | TraitHeader
  | NoContainer
end

module Predecessor = struct
  type t =
  | ClassBodyDeclaration
  | ClassName
  | IfWithoutElse
  | ImplementsList
  | ExtendsList
  | KeywordAbstract
  | KeywordAsync
  | KeywordAwait
  | KeywordCase
  | KeywordClass
  | KeywordConst
  | KeywordEnum
  | KeywordExtends
  | KeywordFinal
  | KeywordFunction
  | KeywordImplements
  | KeywordInclude
  | KeywordInterface
  | KeywordNamespace
  | KeywordNew
  | KeywordNewtype
  | KeywordRequire
  | KeywordReturn
  | KeywordStatic
  | KeywordSwitch
  | KeywordTrait
  | KeywordType
  | KeywordUse
  | Statement
  | TokenColon
  | TokenComma
  | TokenEqual
  | TokenLeftBrace
  | TokenLessThan
  | TokenOpenParen
  | TokenWithoutTrailingTrivia
  | TopLevelDeclaration
  | TryWithoutFinally
  | VisibilityModifier
  | NoPredecessor
end

type context = {
  closest_parent_container: Container.t;
  predecessor: Predecessor.t;
  inside_switch_body: bool;
  inside_loop_body: bool;
  inside_async_function: bool;
  inside_class_body: bool;
}

module ContextPredicates = struct
  open Container
  open Predecessor

  let is_inside_function_call context =
    context.closest_parent_container = FunctionCallArgumentList &&
    (context.predecessor = TokenComma ||
    context.predecessor = TokenOpenParen ||
    context.predecessor = TokenWithoutTrailingTrivia)

  let is_type_valid context =
    (* Function return type *)
    context.closest_parent_container = FunctionHeader &&
    context.predecessor = TokenColon
    || (* Parameter type *)
    context.closest_parent_container = FunctionHeader &&
    (context.predecessor = TokenComma ||
    context.predecessor = TokenOpenParen ||
    context.predecessor = TokenWithoutTrailingTrivia)
    || (* Class property type *)
    context.inside_class_body &&
    context.closest_parent_container = ClassBody &&
    (context.predecessor = VisibilityModifier ||
    context.predecessor = KeywordConst ||
    context.predecessor = KeywordStatic)
    || (* Generic type *)
    context.predecessor = TokenLessThan

  let is_class_body_declaration_valid context =
    context.closest_parent_container = ClassBody &&
    (context.predecessor = TokenLeftBrace ||
    context.predecessor = ClassBodyDeclaration)

  let is_in_return_statement context =
    context.predecessor = KeywordReturn &&
    context.closest_parent_container = CompoundStatement

  let is_at_beginning_of_new_statement context =
    context.closest_parent_container = CompoundStatement &&
    (context.predecessor = Statement ||
    context.predecessor = TokenLeftBrace ||
    context.predecessor = IfWithoutElse ||
    context.predecessor = TryWithoutFinally)
    || (* Cases in a switch body *)
    context.closest_parent_container = CompoundStatement &&
    context.predecessor = TokenColon

  let is_rhs_of_assignment_expression context =
    context.closest_parent_container = AssignmentExpression &&
    context.predecessor = TokenEqual

  let is_in_conditional context =
    context.closest_parent_container = IfStatement &&
    (context.predecessor = TokenOpenParen ||
    context.predecessor = TokenWithoutTrailingTrivia)

  let is_expression_valid context =
    is_rhs_of_assignment_expression context ||
    is_in_conditional context ||
    is_at_beginning_of_new_statement context ||
    is_in_return_statement context ||
    is_inside_function_call context ||
    context.closest_parent_container = LambdaBodyExpression
    (* TODO: or is parameter, or is inside if/switch/while/etc. clause *)

  let is_top_level_statement_valid context =
    context.closest_parent_container = TopLevel &&
    context.predecessor = TopLevelDeclaration
end

let initial_context = {
  closest_parent_container = Container.NoContainer;
  predecessor = Predecessor.NoPredecessor;
  inside_switch_body = false;
  inside_loop_body = false;
  inside_async_function = false;
  inside_class_body = false;
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
  | MethodishDeclaration { methodish_function_decl_header = { syntax =
      FunctionDeclarationHeader { function_async = async; _ }; _
    }; _ }
  | AnonymousFunction { anonymous_async_keyword = async; _ }
  | LambdaExpression { lambda_async = async; _ } ->
    is_specific_token Async async
  | _ -> false

let validate_predecessor (predecessor:PositionedSyntax.t list) : Predecessor.t =
  let open PositionedSyntax in
  let open PositionedToken in
  let open TokenKind in
  let open Predecessor in
  let classify_syntax_as_predecessor node = match syntax node with
    | AliasDeclaration { alias_semicolon = { syntax = Token _; _ }; _ }
    | EnumDeclaration { enum_right_brace = { syntax = Token _; _ }; _ }
    | FunctionDeclaration { function_body = { syntax =
        CompoundStatement { compound_right_brace = { syntax =
          Token _; _
        }; _ }; _
      }; _ }
    | InclusionDirective { inclusion_semicolon = { syntax = Token _; _ }; _ }
    | MarkupSection _
    | NamespaceBody { namespace_right_brace = { syntax = Token _; _ }; _ }
    | NamespaceEmptyBody { namespace_semicolon = { syntax = Token _; _ }; _ }
    | NamespaceUseDeclaration { namespace_use_semicolon = { syntax = Token _; _ }; _ }
    | ClassishBody { classish_body_right_brace = { syntax = Token _; _ }; _ } ->
        Some TopLevelDeclaration
    | ClassishDeclaration {
        classish_implements_list = { syntax = SyntaxList _; _ }; _
      } -> Some ImplementsList
    | ClassishDeclaration {
        classish_extends_list = { syntax = SyntaxList _; _ };
        classish_implements_keyword = { syntax = Missing; _ };
        classish_implements_list = { syntax = Missing; _ };
        _
      } -> Some ExtendsList
    | ClassishDeclaration {
        classish_name = { syntax = Token _; _ };
        classish_type_parameters = { syntax = Missing; _ };
        classish_extends_keyword = { syntax = Missing; _ };
        classish_extends_list = { syntax = Missing; _ };
        classish_implements_keyword = { syntax = Missing; _ };
        classish_implements_list = { syntax = Missing; _ };
        _
      } -> Some ClassName
    | IfStatement { if_else_clause = {
        syntax = Missing; _
      }; _ } -> Some IfWithoutElse
    | TryStatement { try_finally_clause = {
        syntax = Missing; _
      }; _ } -> Some TryWithoutFinally
    | CaseLabel _
    | IfStatement _
    | EchoStatement _
    | WhileStatement _
    | DoStatement _
    | ForStatement _
    | ForeachStatement _
    | TryStatement _
    | SwitchStatement _
    | ReturnStatement _
    | ThrowStatement _
    | BreakStatement _
    | ContinueStatement _
    | ExpressionStatement _ -> Some Statement
    | ConstDeclaration _
    | PropertyDeclaration _
    | MethodishDeclaration _
    | TypeConstDeclaration _  -> Some ClassBodyDeclaration
    | Token { kind = Abstract; _ } -> Some KeywordAbstract
    | Token { kind = Async; _ } -> Some KeywordAsync
    | Token { kind = Await; _ } -> Some KeywordAwait
    | Token { kind = Case; _ } -> Some KeywordCase
    | Token { kind = Class; _ } -> Some KeywordClass
    | Token { kind = Colon; _ } -> Some TokenColon
    | Token { kind = Comma; _ } -> Some TokenComma
    | Token { kind = Const; _ } -> Some KeywordConst
    | Token { kind = Enum; _ } -> Some KeywordEnum
    | Token { kind = Equal; _ } -> Some TokenEqual
    | Token { kind = Extends; _ } -> Some KeywordExtends
    | Token { kind = Final; _ } -> Some KeywordFinal
    | Token { kind = Function; _ } -> Some KeywordFunction
    | Token { kind = Implements; _ } -> Some KeywordImplements
    | Token { kind = Include; _ }
    | Token { kind = Include_once; _ } -> Some KeywordInclude
    | Token { kind = Interface; _ } -> Some KeywordInterface
    | Token { kind = LeftBrace; _ } -> Some TokenLeftBrace
    | Token { kind = LeftParen; _ } -> Some TokenOpenParen
    | Token { kind = LessThan; _ } -> Some TokenLessThan
    | Token { kind = Namespace; _ } -> Some KeywordNamespace
    | Token { kind = New; _ } -> Some KeywordNew
    | Token { kind = Newtype; _ } -> Some KeywordNewtype
    | Token { kind = Public; _ }
    | Token { kind = Private; _ }
    | Token { kind = Protected; _ } -> Some VisibilityModifier
    | Token { kind = Require; _ } -> Some KeywordRequire
    | Token { kind = Return; _ } -> Some KeywordReturn
    | Token { kind = Static; _ } -> Some KeywordStatic
    | Token { kind = Switch; _ } -> Some KeywordSwitch
    | Token { kind = Trait; _ } -> Some KeywordTrait
    | Token { kind = Type; _ } -> Some KeywordType
    | Token { kind = Use; _ } -> Some KeywordUse
    | Token { trailing_width = 0; _ } -> Some TokenWithoutTrailingTrivia
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
  let open PositionedToken in
  let open TokenKind in
  let check_node node acc = match syntax node with
    | Script _ ->
      { acc with closest_parent_container = TopLevel }
    | ClassishDeclaration { classish_keyword = {
        syntax = Token { kind = Interface; _ }; _
      }; _ } ->
      { acc with closest_parent_container = InterfaceHeader }
    | ClassishDeclaration { classish_keyword = {
        syntax = Token { kind = Trait; _ }; _
      }; _ } ->
      { acc with closest_parent_container = TraitHeader }
    | ClassishDeclaration _ ->
      { acc with closest_parent_container = ClassHeader }
    | ClassishBody _ when acc.closest_parent_container = InterfaceHeader ->
      { acc with closest_parent_container = InterfaceBody }
    | ClassishBody _ when acc.closest_parent_container = TraitHeader ->
      { acc with closest_parent_container = TraitBody }
    | ClassishBody _ when acc.closest_parent_container = ClassHeader ->
      { acc with closest_parent_container = ClassBody;
        inside_class_body = true }
    | ConstDeclaration _ ->
      { acc with closest_parent_container = ConstantDeclaration }
    | ForStatement _
    | ForeachStatement _
    | WhileStatement _
    | DoStatement _ ->
      { acc with inside_loop_body = true }
    | SwitchSection _ ->
      { acc with closest_parent_container = Container.CompoundStatement;
        inside_switch_body = true }
    | MethodishDeclaration _
    | FunctionDeclaration _ as func ->
      { acc with inside_async_function = is_function_async func }
    | FunctionDeclarationHeader _ ->
      { acc with closest_parent_container = FunctionHeader }
    | FunctionCallExpression _ ->
      { acc with closest_parent_container = FunctionCallArgumentList }
    | AnonymousFunction _
    | LambdaExpression _ as lambda ->
      (* If we see a lambda, almost all context is reset, so each field should
      get consideration on if its context flows into the lambda *)
      {
        closest_parent_container = LambdaBodyExpression;
        predecessor = predecessor;
        inside_switch_body = false;
        inside_loop_body = false;
        inside_class_body = false;
        inside_async_function = is_function_async lambda;
      }
    | PositionedSyntax.CompoundStatement _ ->
      { acc with closest_parent_container = Container.CompoundStatement }
    | BinaryExpression {
        binary_operator = { syntax = Token { kind = Equal; _ }; _ };
        _
      } -> { acc with closest_parent_container = AssignmentExpression }
    | PositionedSyntax.IfStatement _ ->
      { acc with closest_parent_container = Container.IfStatement }
    | Token { kind = ColonColon; _ } ->
      { acc with closest_parent_container = AfterDoubleColon }
    | Token { kind = MinusGreaterThan; _ } ->
      { acc with closest_parent_container = AfterRightArrow }
    | _ -> acc
  in
  List.fold_right
    ~f:check_node
    ~init:{ initial_context with predecessor }
    full_path

type autocomplete_location_classification =
  | BeforePunctuationToken
  | InLeadingTrivia
  | InToken
  | InTrailingTrivia

let classify_autocomplete_location
  (parents:PositionedSyntax.t list) (offset:int)
  : autocomplete_location_classification =
  let open PositionedSyntax in
  let check_for_specific_token parent =
    let open PositionedToken in
    match syntax parent with
    | Token { kind = TokenKind.EndOfFile; _ } -> InLeadingTrivia
    | Token { kind = TokenKind.RightParen; _ } -> BeforePunctuationToken
    | _ -> InToken
  in
  match parents with
  | [] -> failwith "Empty parentage (this should never happen)"
  | parent :: _ when offset < start_offset parent -> InLeadingTrivia
  | parent :: _ when offset = start_offset parent -> check_for_specific_token parent
  | parent :: _ when offset <= trailing_start_offset parent -> InToken
  | _ -> InTrailingTrivia

let get_context_and_stub (syntax_tree:SyntaxTree.t) (offset:int)
  : context * string =
  let open PositionedSyntax in
  let positioned_tree = from_tree syntax_tree in
  (* If the offset is the same as the width of the whole tree, then the cursor is at the end of
  file, so we move our position to before the last character of the file so that our cursor is
  considered to be in the leading trivia of the end of file character. This guarantees our parentage
  is not empty. *)
  let new_offset =
    if offset >= full_width positioned_tree then full_width positioned_tree - 1
    else offset
  in
  let ancestry = parentage positioned_tree new_offset in
  let location = classify_autocomplete_location ancestry offset in
  let autocomplete_leaf_node = List.hd_exn ancestry in
  let previous_offset = leading_start_offset autocomplete_leaf_node - 1 in
  let predecessor_parentage = parentage positioned_tree previous_offset in
  let node_text = match location with
    | InToken -> text @@ List.hd_exn ancestry
    | BeforePunctuationToken ->
      let token_text = text @@ List.hd_exn predecessor_parentage in
      let identifier_regex = Str.regexp "^\\$?[a-zA-Z0-9_\x7f-\xff]*$" in
      if Str.string_match identifier_regex token_text 0 then token_text else ""
    | _ -> ""
  in
  let (full_path, predecessor) = match location with
    | BeforePunctuationToken
    | InLeadingTrivia -> predecessor_parentage, predecessor_parentage
    | InToken -> ancestry, predecessor_parentage
    | InTrailingTrivia -> ancestry, ancestry
  in
  (make_context ~full_path ~predecessor, node_text)
