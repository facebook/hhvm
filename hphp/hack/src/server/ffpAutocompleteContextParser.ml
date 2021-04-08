(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module PositionedToken = Full_fidelity_positioned_token
module TokenKind = Full_fidelity_token_kind
open Hh_prelude

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

  let is_after_double_colon = function
    | AfterDoubleColon -> true
    | _ -> false

  let is_after_right_arrow = function
    | AfterRightArrow -> true
    | _ -> false

  let is_assignment_expression = function
    | AssignmentExpression -> true
    | _ -> false

  let is_class_body = function
    | ClassBody -> true
    | _ -> false

  let is_class_header = function
    | ClassHeader -> true
    | _ -> false

  let is_compound_statement = function
    | CompoundStatement -> true
    | _ -> false

  let is_constant_declaration = function
    | ConstantDeclaration -> true
    | _ -> false

  let is_function_call_argument_list = function
    | FunctionCallArgumentList -> true
    | _ -> false

  let is_function_header = function
    | FunctionHeader -> true
    | _ -> false

  let is_if_statement = function
    | IfStatement -> true
    | _ -> false

  let is_interface_body = function
    | InterfaceBody -> true
    | _ -> false

  let is_interface_header = function
    | InterfaceHeader -> true
    | _ -> false

  let is_lambda_body_expression = function
    | LambdaBodyExpression -> true
    | _ -> false

  let is_top_level = function
    | TopLevel -> true
    | _ -> false

  let is_trait_body = function
    | TraitBody -> true
    | _ -> false

  let is_trait_header = function
    | TraitHeader -> true
    | _ -> false

  let is_no_container = function
    | NoContainer -> true
    | _ -> false

  let is_class_trait_body = function
    | ClassBody
    | TraitBody ->
      true
    | _ -> false

  let is_trait_interface_body = function
    | TraitBody
    | InterfaceBody ->
      true
    | _ -> false

  let is_class_header_body = function
    | ClassHeader
    | ClassBody ->
      true
    | _ -> false

  let is_interface_header_body = function
    | InterfaceHeader
    | InterfaceBody ->
      true
    | _ -> false

  let is_compound_statement_or_assignment_expression = function
    | CompoundStatement
    | AssignmentExpression ->
      true
    | _ -> false

  let is_interface_or_class_header_or_body = function
    | InterfaceHeader
    | InterfaceBody
    | ClassHeader
    | ClassBody ->
      true
    | _ -> false

  let is_body = function
    | ClassBody
    | InterfaceBody
    | TraitBody ->
      true
    | _ -> false

  let is_body_or_function_header = function
    | ClassBody
    | InterfaceBody
    | TraitBody
    | FunctionHeader ->
      true
    | _ -> false

  let is_class_body_or_function_header = function
    | ClassBody
    | FunctionHeader ->
      true
    | _ -> false

  let is_compound_statement_or_if = function
    | CompoundStatement
    | IfStatement ->
      true
    | _ -> false
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
    | KeywordElse
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

  let is_if_without_else = function
    | IfWithoutElse -> true
    | _ -> false

  let is_class_name = function
    | ClassName -> true
    | _ -> false

  let is_colon = function
    | TokenColon -> true
    | _ -> false

  let is_less_than = function
    | TokenLessThan -> true
    | _ -> false

  let is_kwelse = function
    | KeywordElse -> true
    | _ -> false

  let is_kwreturn = function
    | KeywordReturn -> true
    | _ -> false

  let is_kwnew = function
    | KeywordNew -> true
    | _ -> false

  let is_kwextends = function
    | KeywordExtends -> true
    | _ -> false

  let is_kwimplements = function
    | KeywordImplements -> true
    | _ -> false

  let is_kwuse = function
    | KeywordUse -> true
    | _ -> false

  let is_kwrequire = function
    | KeywordRequire -> true
    | _ -> false

  let is_kwfinal = function
    | KeywordFinal -> true
    | _ -> false

  let is_kwasync = function
    | KeywordAsync -> true
    | _ -> false

  let is_equal = function
    | TokenEqual -> true
    | _ -> false

  let is_try_without_finally = function
    | TryWithoutFinally -> true
    | _ -> false

  let is_visibility_modifier = function
    | VisibilityModifier -> true
    | _ -> false

  let is_top_level_declaration = function
    | TopLevelDeclaration -> true
    | _ -> false

  let is_colon_or_less_than = function
    | TokenColon
    | TokenLessThan ->
      true
    | _ -> false

  let is_top_level_declaration_or_kwabstract = function
    | TopLevelDeclaration
    | KeywordAbstract ->
      true
    | _ -> false

  let is_open_paren_or_without_trailing_trivia = function
    | TokenOpenParen
    | TokenWithoutTrailingTrivia ->
      true
    | _ -> false

  let is_comma_open_paren_or_without_trailing_trivia = function
    | TokenComma
    | TokenOpenParen
    | TokenWithoutTrailingTrivia ->
      true
    | _ -> false

  let is_visibility_mod_kwconst_or_kwstatic = function
    | VisibilityModifier
    | KeywordConst
    | KeywordStatic ->
      true
    | _ -> false

  let is_visibility_mod_kwfinal_or_kwstatic = function
    | VisibilityModifier
    | KeywordFinal
    | KeywordStatic ->
      true
    | _ -> false

  let is_visibility_mod_kwasync_kwfinal_or_kwstatic = function
    | VisibilityModifier
    | KeywordAsync
    | KeywordFinal
    | KeywordStatic ->
      true
    | _ -> false

  let is_visibility_mod_or_kwstatic = function
    | VisibilityModifier
    | KeywordStatic ->
      true
    | _ -> false

  let is_left_brace_or_class_body_decl = function
    | TokenLeftBrace
    | ClassBodyDeclaration ->
      true
    | _ -> false

  let is_stmt_left_brace_if_wo_else_or_try_wo_finally = function
    | Statement
    | TokenLeftBrace
    | IfWithoutElse
    | TryWithoutFinally ->
      true
    | _ -> false

  let is_class_name_or_extends_list = function
    | ClassName
    | ExtendsList ->
      true
    | _ -> false

  let is_kwabstract_or_kwfinal = function
    | KeywordAbstract
    | KeywordFinal ->
      true
    | _ -> false
end

type context = {
  closest_parent_container: Container.t;
  inside_async_function: bool;
  inside_class_body: bool;
  inside_loop_body: bool;
  inside_static_method: bool;
  inside_switch_body: bool;
  predecessor: Predecessor.t;
}

module ContextPredicates = struct
  open Container
  open Predecessor

  let is_inside_function_call context =
    is_function_call_argument_list context.closest_parent_container
    && is_comma_open_paren_or_without_trailing_trivia context.predecessor

  let is_type_valid context =
    (* Function return type *)
    is_function_header context.closest_parent_container
    && is_colon context.predecessor
    (* Parameter type *)
    || is_function_header context.closest_parent_container
       && is_comma_open_paren_or_without_trailing_trivia context.predecessor
    (* Class property type *)
    || is_class_trait_body context.closest_parent_container
       && is_visibility_mod_kwconst_or_kwstatic context.predecessor
    || (* Generic type *)
    is_less_than context.predecessor

  let is_class_body_declaration_valid context =
    is_class_body context.closest_parent_container
    && is_left_brace_or_class_body_decl context.predecessor

  let is_trait_body_declaration_valid context =
    is_trait_body context.closest_parent_container
    && is_left_brace_or_class_body_decl context.predecessor

  let is_interface_body_declaration_valid context =
    is_interface_body context.closest_parent_container
    && is_left_brace_or_class_body_decl context.predecessor

  let is_in_return_statement context =
    is_kwreturn context.predecessor
    && is_compound_statement context.closest_parent_container

  let is_at_beginning_of_new_statement context =
    is_compound_statement context.closest_parent_container
    && is_stmt_left_brace_if_wo_else_or_try_wo_finally context.predecessor
    || (* Cases in a switch body *)
    is_compound_statement context.closest_parent_container
    && is_colon context.predecessor

  let is_rhs_of_assignment_expression context =
    is_assignment_expression context.closest_parent_container
    && is_equal context.predecessor

  let is_in_conditional context =
    is_if_statement context.closest_parent_container
    && is_open_paren_or_without_trailing_trivia context.predecessor

  let is_expression_valid context =
    is_rhs_of_assignment_expression context
    || is_in_conditional context
    || is_at_beginning_of_new_statement context
    || is_in_return_statement context
    || is_inside_function_call context
    || is_lambda_body_expression context.closest_parent_container

  (* TODO: or is parameter, or is inside if/switch/while/etc. clause *)

  let is_top_level_statement_valid context =
    is_top_level context.closest_parent_container
    && is_top_level_declaration context.predecessor
end

let initial_context =
  {
    closest_parent_container = Container.NoContainer;
    inside_async_function = false;
    inside_class_body = false;
    inside_loop_body = false;
    inside_static_method = false;
    inside_switch_body = false;
    predecessor = Predecessor.NoPredecessor;
  }

let validate_predecessor (predecessor : PositionedSyntax.t list) : Predecessor.t
    =
  PositionedSyntax.(
    PositionedToken.(
      TokenKind.(
        Predecessor.(
          let classify_syntax_as_predecessor node =
            match syntax node with
            | AliasDeclaration { alias_semicolon = { syntax = Token _; _ }; _ }
            | EnumDeclaration { enum_right_brace = { syntax = Token _; _ }; _ }
            | FunctionDeclaration
                {
                  function_body =
                    {
                      syntax =
                        CompoundStatement
                          { compound_right_brace = { syntax = Token _; _ }; _ };
                      _;
                    };
                  _;
                }
            | InclusionDirective
                { inclusion_semicolon = { syntax = Token _; _ }; _ }
            | MarkupSection _
            | NamespaceBody
                { namespace_right_brace = { syntax = Token _; _ }; _ }
            | NamespaceEmptyBody
                { namespace_semicolon = { syntax = Token _; _ }; _ }
            | NamespaceUseDeclaration
                { namespace_use_semicolon = { syntax = Token _; _ }; _ }
            | ClassishBody
                { classish_body_right_brace = { syntax = Token _; _ }; _ } ->
              Some TopLevelDeclaration
            | ClassishDeclaration
                { classish_implements_list = { syntax = SyntaxList _; _ }; _ }
              ->
              Some ImplementsList
            | ClassishDeclaration
                {
                  classish_extends_list = { syntax = SyntaxList _; _ };
                  classish_implements_keyword = { syntax = Missing; _ };
                  classish_implements_list = { syntax = Missing; _ };
                  _;
                } ->
              Some ExtendsList
            | ClassishDeclaration
                {
                  classish_name = { syntax = Token _; _ };
                  classish_type_parameters = { syntax = Missing; _ };
                  classish_extends_keyword = { syntax = Missing; _ };
                  classish_extends_list = { syntax = Missing; _ };
                  classish_implements_keyword = { syntax = Missing; _ };
                  classish_implements_list = { syntax = Missing; _ };
                  _;
                } ->
              Some ClassName
            | IfStatement { if_else_clause = { syntax = Missing; _ }; _ } ->
              Some IfWithoutElse
            | TryStatement { try_finally_clause = { syntax = Missing; _ }; _ }
              ->
              Some TryWithoutFinally
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
            | ExpressionStatement _ ->
              Some Statement
            | TraitUse _
            | RequireClause _
            | ConstDeclaration _
            | PropertyDeclaration _
            | MethodishDeclaration _
            | TypeConstDeclaration _ ->
              Some ClassBodyDeclaration
            | Token { kind = Abstract; _ } -> Some KeywordAbstract
            | Token { kind = Async; _ } -> Some KeywordAsync
            | Token { kind = Await; _ } -> Some KeywordAwait
            | Token { kind = Case; _ } -> Some KeywordCase
            | Token { kind = Class; _ } -> Some KeywordClass
            | Token { kind = Colon; _ } -> Some TokenColon
            | Token { kind = Comma; _ } -> Some TokenComma
            | Token { kind = Const; _ } -> Some KeywordConst
            | Token { kind = Else; _ } -> Some KeywordElse
            | Token { kind = Enum; _ } -> Some KeywordEnum
            | Token { kind = Equal; _ } -> Some TokenEqual
            | Token { kind = Extends; _ } -> Some KeywordExtends
            | Token { kind = Final; _ } -> Some KeywordFinal
            | Token { kind = Function; _ } -> Some KeywordFunction
            | Token { kind = Implements; _ } -> Some KeywordImplements
            | Token { kind = Include; _ }
            | Token { kind = Include_once; _ } ->
              Some KeywordInclude
            | Token { kind = Interface; _ } -> Some KeywordInterface
            | Token { kind = LeftBrace; _ } -> Some TokenLeftBrace
            | Token { kind = LeftParen; _ } -> Some TokenOpenParen
            | Token { kind = LessThan; _ } -> Some TokenLessThan
            | Token { kind = Namespace; _ } -> Some KeywordNamespace
            | Token { kind = New; _ } -> Some KeywordNew
            | Token { kind = Newtype; _ } -> Some KeywordNewtype
            | Token { kind = Public; _ }
            | Token { kind = Private; _ }
            | Token { kind = Protected; _ } ->
              Some VisibilityModifier
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
          |> Option.value ~default:NoPredecessor))))

let is_method_static (method_object : PositionedSyntax.syntax) : bool =
  PositionedSyntax.(
    PositionedToken.(
      TokenKind.(
        match method_object with
        | MethodishDeclaration
            {
              methodish_function_decl_header =
                { syntax = FunctionDeclarationHeader h; _ };
              _;
            } ->
          List.exists
            (syntax_node_to_list h.function_modifiers)
            ~f:(is_specific_token Static)
        | _ -> false)))

let is_function_async (function_object : PositionedSyntax.syntax) : bool =
  PositionedSyntax.(
    PositionedToken.(
      TokenKind.(
        match function_object with
        | FunctionDeclaration
            {
              function_declaration_header =
                {
                  syntax =
                    FunctionDeclarationHeader { function_modifiers = m; _ };
                  _;
                };
              _;
            }
        | MethodishDeclaration
            {
              methodish_function_decl_header =
                {
                  syntax =
                    FunctionDeclarationHeader { function_modifiers = m; _ };
                  _;
                };
              _;
            } ->
          List.exists (syntax_node_to_list m) ~f:is_async
        | AnonymousFunction { anonymous_async_keyword = async; _ }
        | LambdaExpression { lambda_async = async; _ } ->
          is_specific_token Async async
        | _ -> false)))

let make_context
    ~(full_path : PositionedSyntax.t list)
    ~(predecessor : PositionedSyntax.t list) : context =
  let predecessor = validate_predecessor predecessor in
  PositionedSyntax.(
    Container.(
      PositionedToken.(
        TokenKind.(
          let check_node node acc =
            match syntax node with
            | Script _ -> { acc with closest_parent_container = TopLevel }
            | ClassishDeclaration
                {
                  classish_keyword =
                    { syntax = Token { kind = Interface; _ }; _ };
                  _;
                } ->
              { acc with closest_parent_container = InterfaceHeader }
            | ClassishDeclaration
                {
                  classish_keyword = { syntax = Token { kind = Trait; _ }; _ };
                  _;
                } ->
              { acc with closest_parent_container = TraitHeader }
            | ClassishDeclaration _ ->
              { acc with closest_parent_container = ClassHeader }
            | ClassishBody _
              when is_interface_header acc.closest_parent_container ->
              { acc with closest_parent_container = InterfaceBody }
            | ClassishBody _ when is_trait_header acc.closest_parent_container
              ->
              { acc with closest_parent_container = TraitBody }
            | ClassishBody _ when is_class_header acc.closest_parent_container
              ->
              {
                acc with
                closest_parent_container = ClassBody;
                inside_class_body = true;
              }
            | ConstDeclaration _ ->
              { acc with closest_parent_container = ConstantDeclaration }
            | ForStatement _
            | ForeachStatement _
            | WhileStatement _
            | DoStatement _ ->
              { acc with inside_loop_body = true }
            | SwitchSection _ ->
              {
                acc with
                closest_parent_container = Container.CompoundStatement;
                inside_switch_body = true;
              }
            | (MethodishDeclaration _ | FunctionDeclaration _) as func ->
              {
                acc with
                inside_async_function = is_function_async func;
                inside_static_method = is_method_static func;
              }
            | FunctionDeclarationHeader _ ->
              { acc with closest_parent_container = FunctionHeader }
            | FunctionCallExpression _ ->
              { acc with closest_parent_container = FunctionCallArgumentList }
            | (AnonymousFunction _ | LambdaExpression _) as lambda ->
              (* If we see a lambda, almost all context is reset, so each field should
      get consideration on if its context flows into the lambda *)
              {
                closest_parent_container = LambdaBodyExpression;
                predecessor;
                inside_switch_body = false;
                inside_loop_body = false;
                inside_class_body = false;
                inside_static_method = is_method_static lambda;
                inside_async_function = is_function_async lambda;
              }
            | PositionedSyntax.CompoundStatement _ ->
              {
                acc with
                closest_parent_container = Container.CompoundStatement;
              }
            | BinaryExpression
                {
                  binary_operator = { syntax = Token { kind = Equal; _ }; _ };
                  _;
                } ->
              { acc with closest_parent_container = AssignmentExpression }
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
            full_path))))

type autocomplete_location_classification =
  | BeforePunctuationToken
  | InLeadingTrivia
  | InToken
  | InTrailingTrivia

let classify_autocomplete_location
    (parents : PositionedSyntax.t list) (off : int) :
    autocomplete_location_classification =
  PositionedSyntax.(
    let check_for_specific_token parent =
      PositionedToken.(
        match syntax parent with
        | Token { kind = TokenKind.EndOfFile; _ } -> InLeadingTrivia
        | Token { kind = TokenKind.RightParen; _ } -> BeforePunctuationToken
        | _ -> InToken)
    in
    match parents with
    | [] -> failwith "Empty parentage (this should never happen)"
    | parent :: _ when off < start_offset parent -> InLeadingTrivia
    | parent :: _ when off = start_offset parent ->
      check_for_specific_token parent
    | parent :: _ when off <= trailing_start_offset parent -> InToken
    | _ -> InTrailingTrivia)

let get_context_and_stub (positioned_tree : PositionedSyntax.t) (off : int) :
    context * string =
  PositionedSyntax.(
    (* If the offset is the same as the width of the whole tree, then the cursor is at the end of
  file, so we move our position to before the last character of the file so that our cursor is
  considered to be in the leading trivia of the end of file character. This guarantees our parentage
  is not empty. *)
    let new_offset =
      if off >= full_width positioned_tree then
        full_width positioned_tree - 1
      else
        off
    in
    let ancestry = parentage positioned_tree new_offset in
    let location = classify_autocomplete_location ancestry off in
    let autocomplete_leaf_node = List.hd_exn ancestry in
    let previous_offset = leading_start_offset autocomplete_leaf_node - 1 in
    let predecessor_parentage = parentage positioned_tree previous_offset in
    let validate_hack_identifier id =
      let identifier_regex = Str.regexp "^\\$?[a-zA-Z0-9_\x7f-\xff]*$" in
      if Str.string_match identifier_regex id 0 then
        id
      else
        ""
    in
    let node_text =
      match location with
      | InToken -> validate_hack_identifier @@ text @@ List.hd_exn ancestry
      | BeforePunctuationToken ->
        validate_hack_identifier @@ text @@ List.hd_exn predecessor_parentage
      | _ -> ""
    in
    let (full_path, predecessor) =
      match location with
      | BeforePunctuationToken
      | InLeadingTrivia ->
        (predecessor_parentage, predecessor_parentage)
      | InToken -> (ancestry, predecessor_parentage)
      | InTrailingTrivia -> (ancestry, ancestry)
    in
    (make_context ~full_path ~predecessor, node_text))
