(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 * Positioned syntax tree
 *
 * A positioned syntax tree stores the original source text,
 * the offset of the leading trivia, the width of the leading trivia,
 * node proper, and trailing trivia. From all this information we can
 * rapidly compute the absolute position of any portion of the node,
 * or the text.
 *)

module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_minimal_syntax)
module SourceText = Full_fidelity_source_text
module Token = Full_fidelity_positioned_token

module SyntaxWithPositionedToken =
  Full_fidelity_syntax.WithToken(Token)

module PositionedSyntaxValue = struct
  type t = {
    source_text: SourceText.t;
    offset: int; (* Beginning of first trivia *)
    leading_width: int;
    width: int; (* Width of node, not counting trivia *)
    trailing_width: int;
  }

  let make source_text offset leading_width width trailing_width =
    { source_text; offset; leading_width; width; trailing_width }

  let source_text value =
    value.source_text

  let start_offset value =
    value.offset

  let leading_width value =
    value.leading_width

  let width value =
    value.width

  let trailing_width value =
    value.trailing_width

  let to_json value =
    let open Hh_json in
    JSON_Object
      [ "offset", int_ value.offset
      ; "leading_width", int_ value.leading_width
      ; "width", int_ value.width
      ; "trailing_width", int_ value.trailing_width
      ]
end


module PositionedWithValue =
  SyntaxWithPositionedToken.WithSyntaxValue(PositionedSyntaxValue)

open Hh_core
include PositionedWithValue

module PositionedValueBuilder = struct
  let value_from_token token =
    let source_text = Token.source_text token in
    let offset = Token.leading_start_offset token in
    let leading_width = Token.leading_width token in
    let width = Token.width token in
    let trailing_width = Token.trailing_width token in
    PositionedSyntaxValue.make
      source_text offset leading_width width trailing_width

  let value_from_outer_children first last =
    let first_value = value first in
    let last_value = value last in
    let source_text = PositionedSyntaxValue.source_text first_value in
    let first_offset = PositionedSyntaxValue.start_offset first_value in
    let first_leading_width = PositionedSyntaxValue.leading_width first_value in
    let trailing_width =
      PositionedSyntaxValue.trailing_width last_value in
    let last_offset = PositionedSyntaxValue.start_offset last_value in
    let last_leading_width = PositionedSyntaxValue.leading_width last_value in
    let last_width = PositionedSyntaxValue.width last_value in
    let width = (last_offset + last_leading_width + last_width) -
      (first_offset + first_leading_width) in
    PositionedSyntaxValue.make
      source_text first_offset first_leading_width width trailing_width

  let width n =
    PositionedSyntaxValue.width (value n)

  let value_from_children source_text offset kind nodes =
    (**
     * We need to determine the offset, leading, middle and trailing widths of
     * the node to be constructed based on its children.  If the children are
     * all of zero width -- including the case where there are no children at
     * all -- then we make a zero-width value at the given offset.
     * Otherwise, we can determine the associated value from the first and last
     * children that have width.
     *)
    let have_width = List.filter ~f:(fun x -> (width x) > 0) nodes in
    match have_width with
    | [] -> PositionedSyntaxValue.make source_text offset 0 0 0
    | first :: _ -> value_from_outer_children first (List.last_exn have_width)

  let value_from_syntax syntax =
    (* We need to find the first and last nodes that have width. If there are
      no such nodes then we can simply use the first and last nodes, period,
      since they will have an offset and source text we can use. *)
    let f (first, first_not_zero, last_not_zero, last) node =
      if first = None then
        if (width node) > 0 then
          (Some node, Some node, Some node, Some node)
        else
          (Some node, None, None, Some node)
      else if (width node) > 0 then
        if first_not_zero = None then
          (first, Some node, Some node, Some node)
        else
          (first, first_not_zero, Some node, Some node)
      else
        (first, first_not_zero, last_not_zero, Some node) in
    let (f, fnz, lnz, l) =
      fold_over_children f (None, None, None, None) syntax in
    match (f, fnz, lnz, l) with
    | (_, Some first_not_zero, Some last_not_zero, _) ->
      value_from_outer_children first_not_zero last_not_zero
    | (Some first, None, None, Some last) ->
      value_from_outer_children first last
    | _ -> failwith
      "how did we get a node with no children in value_from_syntax?"
end

include PositionedWithValue.WithValueBuilder(PositionedValueBuilder)

module Validated =
  Full_fidelity_validated_syntax.Make(Token)(PositionedSyntaxValue)

let source_text node =
  PositionedSyntaxValue.source_text (value node)

let leading_width node =
  PositionedSyntaxValue.leading_width (value node)

let width node =
  PositionedSyntaxValue.width (value node)

let trailing_width node =
  PositionedSyntaxValue.trailing_width (value node)

let full_width node =
  (leading_width node) + (width node) + (trailing_width node)

let leading_start_offset node =
  PositionedSyntaxValue.start_offset (value node)

let leading_end_offset node =
  let w = (leading_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let start_offset node =
  (leading_start_offset node) + (leading_width node)

let end_offset node =
  let w = (width node) - 1 in
  let w = if w < 0 then 0 else w in
  (start_offset node) + w

let trailing_start_offset node =
  (leading_start_offset node) + (leading_width node) + (width node)

let trailing_end_offset node =
  let w = (full_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let leading_start_position node =
  SourceText.offset_to_position (source_text node) (leading_start_offset node)

let leading_end_position node =
  SourceText.offset_to_position (source_text node) (leading_end_offset node)

let start_position node =
  SourceText.offset_to_position (source_text node) (start_offset node)

let end_position node =
  SourceText.offset_to_position (source_text node) (end_offset node)

let trailing_start_position node =
  SourceText.offset_to_position (source_text node) (trailing_start_offset node)

let trailing_end_position node =
  SourceText.offset_to_position (source_text node) (trailing_end_offset node)

let leading_span node =
  ((leading_start_position node), (leading_end_position node))

let span node =
  ((start_position node), (end_position node))

let trailing_span node =
  ((trailing_start_position node), (trailing_end_position node))

let full_span node =
  ((leading_start_position node), (trailing_end_position node))

let full_text node =
  SourceText.sub
    (source_text node) (leading_start_offset node) (full_width node)

let leading_text node =
  SourceText.sub
    (source_text node)
    (leading_start_offset node)
    (leading_width node)

let trailing_text node =
  SourceText.sub
    (source_text node) ((end_offset node) + 1) (trailing_width node)

let text node =
  SourceText.sub (source_text node) (start_offset node) (width node)

let extract_text node =
  Some (text node)

(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage node position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = full_width h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc in
  aux [node] position []

let is_in_body node position =
  let rec aux parents =
    match parents with
    | [] -> false
    | h1 :: t1 ->
      if is_compound_statement h1 then
        match t1 with
        | [] -> false
        | h2 :: _ ->
          is_methodish_declaration h2 || is_function_declaration h2 || aux t1
      else
        aux t1 in
  let parents = parentage node position in
  aux parents

let position file node =
  let source_text = source_text node in
  let start_offset = start_offset node in
  let end_offset = end_offset node in
  Some (SourceText.relative_pos file source_text start_offset end_offset)

module FromMinimal = struct
  module SyntaxKind = Full_fidelity_syntax_kind
  module M = Full_fidelity_minimal_syntax

  exception Multiplicitous_conversion_result of int

  type todo =
  | Build of (M.t * int * todo)
  | Convert of (M.t * todo)
  | Done

  let from_minimal source_text (node : M.t) : t =
    let make_syntax minimal_node positioned_syntax offset =
      let leading_width = M.leading_width minimal_node in
      let width = M.width minimal_node in
      let trailing_width = M.trailing_width minimal_node in
      let value = PositionedSyntaxValue.make
        source_text offset leading_width width trailing_width in
      make positioned_syntax value
    in
    let build minimal_t (results : t list) : syntax * t list =
      match M.kind minimal_t, results with
      | SyntaxKind.SyntaxList, _ ->
        let len =
          match M.syntax minimal_t with
          | M.SyntaxList l -> List.length l
          | _ -> failwith "SyntaxKind out of sync with Syntax (impossible)."
        in
        let rec aux ls n rs =
          match n, rs with
          | 0, _ -> ls, rs
          | _, [] -> failwith "Rebuilding list with insufficient elements."
          | n, (r::rs) -> aux (r :: ls) (n - 1) rs
        in
        let ls, rs = aux [] len results in
        SyntaxList ls, rs
      | SyntaxKind.EndOfFile
      , (  end_of_file_token
        :: results
        ) ->
          EndOfFile
          { end_of_file_token
          }, results
      | SyntaxKind.Script
      , (  script_declarations
        :: results
        ) ->
          Script
          { script_declarations
          }, results
      | SyntaxKind.QualifiedName
      , (  qualified_name_parts
        :: results
        ) ->
          QualifiedName
          { qualified_name_parts
          }, results
      | SyntaxKind.SimpleTypeSpecifier
      , (  simple_type_specifier
        :: results
        ) ->
          SimpleTypeSpecifier
          { simple_type_specifier
          }, results
      | SyntaxKind.LiteralExpression
      , (  literal_expression
        :: results
        ) ->
          LiteralExpression
          { literal_expression
          }, results
      | SyntaxKind.VariableExpression
      , (  variable_expression
        :: results
        ) ->
          VariableExpression
          { variable_expression
          }, results
      | SyntaxKind.PipeVariableExpression
      , (  pipe_variable_expression
        :: results
        ) ->
          PipeVariableExpression
          { pipe_variable_expression
          }, results
      | SyntaxKind.EnumDeclaration
      , (  enum_right_brace
        :: enum_enumerators
        :: enum_left_brace
        :: enum_type
        :: enum_base
        :: enum_colon
        :: enum_name
        :: enum_keyword
        :: enum_attribute_spec
        :: results
        ) ->
          EnumDeclaration
          { enum_attribute_spec
          ; enum_keyword
          ; enum_name
          ; enum_colon
          ; enum_base
          ; enum_type
          ; enum_left_brace
          ; enum_enumerators
          ; enum_right_brace
          }, results
      | SyntaxKind.Enumerator
      , (  enumerator_semicolon
        :: enumerator_value
        :: enumerator_equal
        :: enumerator_name
        :: results
        ) ->
          Enumerator
          { enumerator_name
          ; enumerator_equal
          ; enumerator_value
          ; enumerator_semicolon
          }, results
      | SyntaxKind.AliasDeclaration
      , (  alias_semicolon
        :: alias_type
        :: alias_equal
        :: alias_constraint
        :: alias_generic_parameter
        :: alias_name
        :: alias_keyword
        :: alias_attribute_spec
        :: results
        ) ->
          AliasDeclaration
          { alias_attribute_spec
          ; alias_keyword
          ; alias_name
          ; alias_generic_parameter
          ; alias_constraint
          ; alias_equal
          ; alias_type
          ; alias_semicolon
          }, results
      | SyntaxKind.PropertyDeclaration
      , (  property_semicolon
        :: property_declarators
        :: property_type
        :: property_modifiers
        :: results
        ) ->
          PropertyDeclaration
          { property_modifiers
          ; property_type
          ; property_declarators
          ; property_semicolon
          }, results
      | SyntaxKind.PropertyDeclarator
      , (  property_initializer
        :: property_name
        :: results
        ) ->
          PropertyDeclarator
          { property_name
          ; property_initializer
          }, results
      | SyntaxKind.NamespaceDeclaration
      , (  namespace_body
        :: namespace_name
        :: namespace_keyword
        :: results
        ) ->
          NamespaceDeclaration
          { namespace_keyword
          ; namespace_name
          ; namespace_body
          }, results
      | SyntaxKind.NamespaceBody
      , (  namespace_right_brace
        :: namespace_declarations
        :: namespace_left_brace
        :: results
        ) ->
          NamespaceBody
          { namespace_left_brace
          ; namespace_declarations
          ; namespace_right_brace
          }, results
      | SyntaxKind.NamespaceEmptyBody
      , (  namespace_semicolon
        :: results
        ) ->
          NamespaceEmptyBody
          { namespace_semicolon
          }, results
      | SyntaxKind.NamespaceUseDeclaration
      , (  namespace_use_semicolon
        :: namespace_use_clauses
        :: namespace_use_kind
        :: namespace_use_keyword
        :: results
        ) ->
          NamespaceUseDeclaration
          { namespace_use_keyword
          ; namespace_use_kind
          ; namespace_use_clauses
          ; namespace_use_semicolon
          }, results
      | SyntaxKind.NamespaceGroupUseDeclaration
      , (  namespace_group_use_semicolon
        :: namespace_group_use_right_brace
        :: namespace_group_use_clauses
        :: namespace_group_use_left_brace
        :: namespace_group_use_prefix
        :: namespace_group_use_kind
        :: namespace_group_use_keyword
        :: results
        ) ->
          NamespaceGroupUseDeclaration
          { namespace_group_use_keyword
          ; namespace_group_use_kind
          ; namespace_group_use_prefix
          ; namespace_group_use_left_brace
          ; namespace_group_use_clauses
          ; namespace_group_use_right_brace
          ; namespace_group_use_semicolon
          }, results
      | SyntaxKind.NamespaceUseClause
      , (  namespace_use_alias
        :: namespace_use_as
        :: namespace_use_name
        :: namespace_use_clause_kind
        :: results
        ) ->
          NamespaceUseClause
          { namespace_use_clause_kind
          ; namespace_use_name
          ; namespace_use_as
          ; namespace_use_alias
          }, results
      | SyntaxKind.FunctionDeclaration
      , (  function_body
        :: function_declaration_header
        :: function_attribute_spec
        :: results
        ) ->
          FunctionDeclaration
          { function_attribute_spec
          ; function_declaration_header
          ; function_body
          }, results
      | SyntaxKind.FunctionDeclarationHeader
      , (  function_where_clause
        :: function_type
        :: function_colon
        :: function_right_paren
        :: function_parameter_list
        :: function_left_paren
        :: function_type_parameter_list
        :: function_name
        :: function_ampersand
        :: function_keyword
        :: function_modifiers
        :: results
        ) ->
          FunctionDeclarationHeader
          { function_modifiers
          ; function_keyword
          ; function_ampersand
          ; function_name
          ; function_type_parameter_list
          ; function_left_paren
          ; function_parameter_list
          ; function_right_paren
          ; function_colon
          ; function_type
          ; function_where_clause
          }, results
      | SyntaxKind.WhereClause
      , (  where_clause_constraints
        :: where_clause_keyword
        :: results
        ) ->
          WhereClause
          { where_clause_keyword
          ; where_clause_constraints
          }, results
      | SyntaxKind.WhereConstraint
      , (  where_constraint_right_type
        :: where_constraint_operator
        :: where_constraint_left_type
        :: results
        ) ->
          WhereConstraint
          { where_constraint_left_type
          ; where_constraint_operator
          ; where_constraint_right_type
          }, results
      | SyntaxKind.MethodishDeclaration
      , (  methodish_semicolon
        :: methodish_function_body
        :: methodish_function_decl_header
        :: methodish_attribute
        :: results
        ) ->
          MethodishDeclaration
          { methodish_attribute
          ; methodish_function_decl_header
          ; methodish_function_body
          ; methodish_semicolon
          }, results
      | SyntaxKind.ClassishDeclaration
      , (  classish_body
        :: classish_implements_list
        :: classish_implements_keyword
        :: classish_extends_list
        :: classish_extends_keyword
        :: classish_type_parameters
        :: classish_name
        :: classish_keyword
        :: classish_modifiers
        :: classish_attribute
        :: results
        ) ->
          ClassishDeclaration
          { classish_attribute
          ; classish_modifiers
          ; classish_keyword
          ; classish_name
          ; classish_type_parameters
          ; classish_extends_keyword
          ; classish_extends_list
          ; classish_implements_keyword
          ; classish_implements_list
          ; classish_body
          }, results
      | SyntaxKind.ClassishBody
      , (  classish_body_right_brace
        :: classish_body_elements
        :: classish_body_left_brace
        :: results
        ) ->
          ClassishBody
          { classish_body_left_brace
          ; classish_body_elements
          ; classish_body_right_brace
          }, results
      | SyntaxKind.TraitUsePrecedenceItem
      , (  trait_use_precedence_item_removed_names
        :: trait_use_precedence_item_keyword
        :: trait_use_precedence_item_name
        :: results
        ) ->
          TraitUsePrecedenceItem
          { trait_use_precedence_item_name
          ; trait_use_precedence_item_keyword
          ; trait_use_precedence_item_removed_names
          }, results
      | SyntaxKind.TraitUseAliasItem
      , (  trait_use_alias_item_aliased_name
        :: trait_use_alias_item_modifiers
        :: trait_use_alias_item_keyword
        :: trait_use_alias_item_aliasing_name
        :: results
        ) ->
          TraitUseAliasItem
          { trait_use_alias_item_aliasing_name
          ; trait_use_alias_item_keyword
          ; trait_use_alias_item_modifiers
          ; trait_use_alias_item_aliased_name
          }, results
      | SyntaxKind.TraitUseConflictResolution
      , (  trait_use_conflict_resolution_right_brace
        :: trait_use_conflict_resolution_clauses
        :: trait_use_conflict_resolution_left_brace
        :: trait_use_conflict_resolution_names
        :: trait_use_conflict_resolution_keyword
        :: results
        ) ->
          TraitUseConflictResolution
          { trait_use_conflict_resolution_keyword
          ; trait_use_conflict_resolution_names
          ; trait_use_conflict_resolution_left_brace
          ; trait_use_conflict_resolution_clauses
          ; trait_use_conflict_resolution_right_brace
          }, results
      | SyntaxKind.TraitUse
      , (  trait_use_semicolon
        :: trait_use_names
        :: trait_use_keyword
        :: results
        ) ->
          TraitUse
          { trait_use_keyword
          ; trait_use_names
          ; trait_use_semicolon
          }, results
      | SyntaxKind.RequireClause
      , (  require_semicolon
        :: require_name
        :: require_kind
        :: require_keyword
        :: results
        ) ->
          RequireClause
          { require_keyword
          ; require_kind
          ; require_name
          ; require_semicolon
          }, results
      | SyntaxKind.ConstDeclaration
      , (  const_semicolon
        :: const_declarators
        :: const_type_specifier
        :: const_keyword
        :: const_abstract
        :: results
        ) ->
          ConstDeclaration
          { const_abstract
          ; const_keyword
          ; const_type_specifier
          ; const_declarators
          ; const_semicolon
          }, results
      | SyntaxKind.ConstantDeclarator
      , (  constant_declarator_initializer
        :: constant_declarator_name
        :: results
        ) ->
          ConstantDeclarator
          { constant_declarator_name
          ; constant_declarator_initializer
          }, results
      | SyntaxKind.TypeConstDeclaration
      , (  type_const_semicolon
        :: type_const_type_specifier
        :: type_const_equal
        :: type_const_type_constraint
        :: type_const_type_parameters
        :: type_const_name
        :: type_const_type_keyword
        :: type_const_keyword
        :: type_const_abstract
        :: results
        ) ->
          TypeConstDeclaration
          { type_const_abstract
          ; type_const_keyword
          ; type_const_type_keyword
          ; type_const_name
          ; type_const_type_parameters
          ; type_const_type_constraint
          ; type_const_equal
          ; type_const_type_specifier
          ; type_const_semicolon
          }, results
      | SyntaxKind.DecoratedExpression
      , (  decorated_expression_expression
        :: decorated_expression_decorator
        :: results
        ) ->
          DecoratedExpression
          { decorated_expression_decorator
          ; decorated_expression_expression
          }, results
      | SyntaxKind.ParameterDeclaration
      , (  parameter_default_value
        :: parameter_name
        :: parameter_type
        :: parameter_call_convention
        :: parameter_visibility
        :: parameter_attribute
        :: results
        ) ->
          ParameterDeclaration
          { parameter_attribute
          ; parameter_visibility
          ; parameter_call_convention
          ; parameter_type
          ; parameter_name
          ; parameter_default_value
          }, results
      | SyntaxKind.VariadicParameter
      , (  variadic_parameter_ellipsis
        :: variadic_parameter_type
        :: variadic_parameter_call_convention
        :: results
        ) ->
          VariadicParameter
          { variadic_parameter_call_convention
          ; variadic_parameter_type
          ; variadic_parameter_ellipsis
          }, results
      | SyntaxKind.AttributeSpecification
      , (  attribute_specification_right_double_angle
        :: attribute_specification_attributes
        :: attribute_specification_left_double_angle
        :: results
        ) ->
          AttributeSpecification
          { attribute_specification_left_double_angle
          ; attribute_specification_attributes
          ; attribute_specification_right_double_angle
          }, results
      | SyntaxKind.Attribute
      , (  attribute_right_paren
        :: attribute_values
        :: attribute_left_paren
        :: attribute_name
        :: results
        ) ->
          Attribute
          { attribute_name
          ; attribute_left_paren
          ; attribute_values
          ; attribute_right_paren
          }, results
      | SyntaxKind.InclusionExpression
      , (  inclusion_filename
        :: inclusion_require
        :: results
        ) ->
          InclusionExpression
          { inclusion_require
          ; inclusion_filename
          }, results
      | SyntaxKind.InclusionDirective
      , (  inclusion_semicolon
        :: inclusion_expression
        :: results
        ) ->
          InclusionDirective
          { inclusion_expression
          ; inclusion_semicolon
          }, results
      | SyntaxKind.CompoundStatement
      , (  compound_right_brace
        :: compound_statements
        :: compound_left_brace
        :: results
        ) ->
          CompoundStatement
          { compound_left_brace
          ; compound_statements
          ; compound_right_brace
          }, results
      | SyntaxKind.ExpressionStatement
      , (  expression_statement_semicolon
        :: expression_statement_expression
        :: results
        ) ->
          ExpressionStatement
          { expression_statement_expression
          ; expression_statement_semicolon
          }, results
      | SyntaxKind.MarkupSection
      , (  markup_expression
        :: markup_suffix
        :: markup_text
        :: markup_prefix
        :: results
        ) ->
          MarkupSection
          { markup_prefix
          ; markup_text
          ; markup_suffix
          ; markup_expression
          }, results
      | SyntaxKind.MarkupSuffix
      , (  markup_suffix_name
        :: markup_suffix_less_than_question
        :: results
        ) ->
          MarkupSuffix
          { markup_suffix_less_than_question
          ; markup_suffix_name
          }, results
      | SyntaxKind.UnsetStatement
      , (  unset_semicolon
        :: unset_right_paren
        :: unset_variables
        :: unset_left_paren
        :: unset_keyword
        :: results
        ) ->
          UnsetStatement
          { unset_keyword
          ; unset_left_paren
          ; unset_variables
          ; unset_right_paren
          ; unset_semicolon
          }, results
      | SyntaxKind.UsingStatementBlockScoped
      , (  using_block_body
        :: using_block_right_paren
        :: using_block_expressions
        :: using_block_left_paren
        :: using_block_using_keyword
        :: using_block_await_keyword
        :: results
        ) ->
          UsingStatementBlockScoped
          { using_block_await_keyword
          ; using_block_using_keyword
          ; using_block_left_paren
          ; using_block_expressions
          ; using_block_right_paren
          ; using_block_body
          }, results
      | SyntaxKind.UsingStatementFunctionScoped
      , (  using_function_semicolon
        :: using_function_expression
        :: using_function_using_keyword
        :: using_function_await_keyword
        :: results
        ) ->
          UsingStatementFunctionScoped
          { using_function_await_keyword
          ; using_function_using_keyword
          ; using_function_expression
          ; using_function_semicolon
          }, results
      | SyntaxKind.DeclareDirectiveStatement
      , (  declare_directive_semicolon
        :: declare_directive_right_paren
        :: declare_directive_expression
        :: declare_directive_left_paren
        :: declare_directive_keyword
        :: results
        ) ->
          DeclareDirectiveStatement
          { declare_directive_keyword
          ; declare_directive_left_paren
          ; declare_directive_expression
          ; declare_directive_right_paren
          ; declare_directive_semicolon
          }, results
      | SyntaxKind.DeclareBlockStatement
      , (  declare_block_body
        :: declare_block_right_paren
        :: declare_block_expression
        :: declare_block_left_paren
        :: declare_block_keyword
        :: results
        ) ->
          DeclareBlockStatement
          { declare_block_keyword
          ; declare_block_left_paren
          ; declare_block_expression
          ; declare_block_right_paren
          ; declare_block_body
          }, results
      | SyntaxKind.WhileStatement
      , (  while_body
        :: while_right_paren
        :: while_condition
        :: while_left_paren
        :: while_keyword
        :: results
        ) ->
          WhileStatement
          { while_keyword
          ; while_left_paren
          ; while_condition
          ; while_right_paren
          ; while_body
          }, results
      | SyntaxKind.IfStatement
      , (  if_else_clause
        :: if_elseif_clauses
        :: if_statement
        :: if_right_paren
        :: if_condition
        :: if_left_paren
        :: if_keyword
        :: results
        ) ->
          IfStatement
          { if_keyword
          ; if_left_paren
          ; if_condition
          ; if_right_paren
          ; if_statement
          ; if_elseif_clauses
          ; if_else_clause
          }, results
      | SyntaxKind.ElseifClause
      , (  elseif_statement
        :: elseif_right_paren
        :: elseif_condition
        :: elseif_left_paren
        :: elseif_keyword
        :: results
        ) ->
          ElseifClause
          { elseif_keyword
          ; elseif_left_paren
          ; elseif_condition
          ; elseif_right_paren
          ; elseif_statement
          }, results
      | SyntaxKind.ElseClause
      , (  else_statement
        :: else_keyword
        :: results
        ) ->
          ElseClause
          { else_keyword
          ; else_statement
          }, results
      | SyntaxKind.IfEndIfStatement
      , (  if_endif_semicolon
        :: if_endif_endif_keyword
        :: if_endif_else_colon_clause
        :: if_endif_elseif_colon_clauses
        :: if_endif_statement
        :: if_endif_colon
        :: if_endif_right_paren
        :: if_endif_condition
        :: if_endif_left_paren
        :: if_endif_keyword
        :: results
        ) ->
          IfEndIfStatement
          { if_endif_keyword
          ; if_endif_left_paren
          ; if_endif_condition
          ; if_endif_right_paren
          ; if_endif_colon
          ; if_endif_statement
          ; if_endif_elseif_colon_clauses
          ; if_endif_else_colon_clause
          ; if_endif_endif_keyword
          ; if_endif_semicolon
          }, results
      | SyntaxKind.ElseifColonClause
      , (  elseif_colon_statement
        :: elseif_colon_colon
        :: elseif_colon_right_paren
        :: elseif_colon_condition
        :: elseif_colon_left_paren
        :: elseif_colon_keyword
        :: results
        ) ->
          ElseifColonClause
          { elseif_colon_keyword
          ; elseif_colon_left_paren
          ; elseif_colon_condition
          ; elseif_colon_right_paren
          ; elseif_colon_colon
          ; elseif_colon_statement
          }, results
      | SyntaxKind.ElseColonClause
      , (  else_colon_statement
        :: else_colon_colon
        :: else_colon_keyword
        :: results
        ) ->
          ElseColonClause
          { else_colon_keyword
          ; else_colon_colon
          ; else_colon_statement
          }, results
      | SyntaxKind.TryStatement
      , (  try_finally_clause
        :: try_catch_clauses
        :: try_compound_statement
        :: try_keyword
        :: results
        ) ->
          TryStatement
          { try_keyword
          ; try_compound_statement
          ; try_catch_clauses
          ; try_finally_clause
          }, results
      | SyntaxKind.CatchClause
      , (  catch_body
        :: catch_right_paren
        :: catch_variable
        :: catch_type
        :: catch_left_paren
        :: catch_keyword
        :: results
        ) ->
          CatchClause
          { catch_keyword
          ; catch_left_paren
          ; catch_type
          ; catch_variable
          ; catch_right_paren
          ; catch_body
          }, results
      | SyntaxKind.FinallyClause
      , (  finally_body
        :: finally_keyword
        :: results
        ) ->
          FinallyClause
          { finally_keyword
          ; finally_body
          }, results
      | SyntaxKind.DoStatement
      , (  do_semicolon
        :: do_right_paren
        :: do_condition
        :: do_left_paren
        :: do_while_keyword
        :: do_body
        :: do_keyword
        :: results
        ) ->
          DoStatement
          { do_keyword
          ; do_body
          ; do_while_keyword
          ; do_left_paren
          ; do_condition
          ; do_right_paren
          ; do_semicolon
          }, results
      | SyntaxKind.ForStatement
      , (  for_body
        :: for_right_paren
        :: for_end_of_loop
        :: for_second_semicolon
        :: for_control
        :: for_first_semicolon
        :: for_initializer
        :: for_left_paren
        :: for_keyword
        :: results
        ) ->
          ForStatement
          { for_keyword
          ; for_left_paren
          ; for_initializer
          ; for_first_semicolon
          ; for_control
          ; for_second_semicolon
          ; for_end_of_loop
          ; for_right_paren
          ; for_body
          }, results
      | SyntaxKind.ForeachStatement
      , (  foreach_body
        :: foreach_right_paren
        :: foreach_value
        :: foreach_arrow
        :: foreach_key
        :: foreach_as
        :: foreach_await_keyword
        :: foreach_collection
        :: foreach_left_paren
        :: foreach_keyword
        :: results
        ) ->
          ForeachStatement
          { foreach_keyword
          ; foreach_left_paren
          ; foreach_collection
          ; foreach_await_keyword
          ; foreach_as
          ; foreach_key
          ; foreach_arrow
          ; foreach_value
          ; foreach_right_paren
          ; foreach_body
          }, results
      | SyntaxKind.SwitchStatement
      , (  switch_right_brace
        :: switch_sections
        :: switch_left_brace
        :: switch_right_paren
        :: switch_expression
        :: switch_left_paren
        :: switch_keyword
        :: results
        ) ->
          SwitchStatement
          { switch_keyword
          ; switch_left_paren
          ; switch_expression
          ; switch_right_paren
          ; switch_left_brace
          ; switch_sections
          ; switch_right_brace
          }, results
      | SyntaxKind.SwitchSection
      , (  switch_section_fallthrough
        :: switch_section_statements
        :: switch_section_labels
        :: results
        ) ->
          SwitchSection
          { switch_section_labels
          ; switch_section_statements
          ; switch_section_fallthrough
          }, results
      | SyntaxKind.SwitchFallthrough
      , (  fallthrough_semicolon
        :: fallthrough_keyword
        :: results
        ) ->
          SwitchFallthrough
          { fallthrough_keyword
          ; fallthrough_semicolon
          }, results
      | SyntaxKind.CaseLabel
      , (  case_colon
        :: case_expression
        :: case_keyword
        :: results
        ) ->
          CaseLabel
          { case_keyword
          ; case_expression
          ; case_colon
          }, results
      | SyntaxKind.DefaultLabel
      , (  default_colon
        :: default_keyword
        :: results
        ) ->
          DefaultLabel
          { default_keyword
          ; default_colon
          }, results
      | SyntaxKind.ReturnStatement
      , (  return_semicolon
        :: return_expression
        :: return_keyword
        :: results
        ) ->
          ReturnStatement
          { return_keyword
          ; return_expression
          ; return_semicolon
          }, results
      | SyntaxKind.GotoLabel
      , (  goto_label_colon
        :: goto_label_name
        :: results
        ) ->
          GotoLabel
          { goto_label_name
          ; goto_label_colon
          }, results
      | SyntaxKind.GotoStatement
      , (  goto_statement_semicolon
        :: goto_statement_label_name
        :: goto_statement_keyword
        :: results
        ) ->
          GotoStatement
          { goto_statement_keyword
          ; goto_statement_label_name
          ; goto_statement_semicolon
          }, results
      | SyntaxKind.ThrowStatement
      , (  throw_semicolon
        :: throw_expression
        :: throw_keyword
        :: results
        ) ->
          ThrowStatement
          { throw_keyword
          ; throw_expression
          ; throw_semicolon
          }, results
      | SyntaxKind.BreakStatement
      , (  break_semicolon
        :: break_level
        :: break_keyword
        :: results
        ) ->
          BreakStatement
          { break_keyword
          ; break_level
          ; break_semicolon
          }, results
      | SyntaxKind.ContinueStatement
      , (  continue_semicolon
        :: continue_level
        :: continue_keyword
        :: results
        ) ->
          ContinueStatement
          { continue_keyword
          ; continue_level
          ; continue_semicolon
          }, results
      | SyntaxKind.FunctionStaticStatement
      , (  static_semicolon
        :: static_declarations
        :: static_static_keyword
        :: results
        ) ->
          FunctionStaticStatement
          { static_static_keyword
          ; static_declarations
          ; static_semicolon
          }, results
      | SyntaxKind.StaticDeclarator
      , (  static_initializer
        :: static_name
        :: results
        ) ->
          StaticDeclarator
          { static_name
          ; static_initializer
          }, results
      | SyntaxKind.EchoStatement
      , (  echo_semicolon
        :: echo_expressions
        :: echo_keyword
        :: results
        ) ->
          EchoStatement
          { echo_keyword
          ; echo_expressions
          ; echo_semicolon
          }, results
      | SyntaxKind.GlobalStatement
      , (  global_semicolon
        :: global_variables
        :: global_keyword
        :: results
        ) ->
          GlobalStatement
          { global_keyword
          ; global_variables
          ; global_semicolon
          }, results
      | SyntaxKind.SimpleInitializer
      , (  simple_initializer_value
        :: simple_initializer_equal
        :: results
        ) ->
          SimpleInitializer
          { simple_initializer_equal
          ; simple_initializer_value
          }, results
      | SyntaxKind.AnonymousClass
      , (  anonymous_class_body
        :: anonymous_class_implements_list
        :: anonymous_class_implements_keyword
        :: anonymous_class_extends_list
        :: anonymous_class_extends_keyword
        :: anonymous_class_right_paren
        :: anonymous_class_argument_list
        :: anonymous_class_left_paren
        :: anonymous_class_class_keyword
        :: results
        ) ->
          AnonymousClass
          { anonymous_class_class_keyword
          ; anonymous_class_left_paren
          ; anonymous_class_argument_list
          ; anonymous_class_right_paren
          ; anonymous_class_extends_keyword
          ; anonymous_class_extends_list
          ; anonymous_class_implements_keyword
          ; anonymous_class_implements_list
          ; anonymous_class_body
          }, results
      | SyntaxKind.AnonymousFunction
      , (  anonymous_body
        :: anonymous_use
        :: anonymous_type
        :: anonymous_colon
        :: anonymous_right_paren
        :: anonymous_parameters
        :: anonymous_left_paren
        :: anonymous_function_keyword
        :: anonymous_coroutine_keyword
        :: anonymous_async_keyword
        :: anonymous_static_keyword
        :: results
        ) ->
          AnonymousFunction
          { anonymous_static_keyword
          ; anonymous_async_keyword
          ; anonymous_coroutine_keyword
          ; anonymous_function_keyword
          ; anonymous_left_paren
          ; anonymous_parameters
          ; anonymous_right_paren
          ; anonymous_colon
          ; anonymous_type
          ; anonymous_use
          ; anonymous_body
          }, results
      | SyntaxKind.Php7AnonymousFunction
      , (  php7_anonymous_body
        :: php7_anonymous_type
        :: php7_anonymous_colon
        :: php7_anonymous_use
        :: php7_anonymous_right_paren
        :: php7_anonymous_parameters
        :: php7_anonymous_left_paren
        :: php7_anonymous_function_keyword
        :: php7_anonymous_coroutine_keyword
        :: php7_anonymous_async_keyword
        :: php7_anonymous_static_keyword
        :: results
        ) ->
          Php7AnonymousFunction
          { php7_anonymous_static_keyword
          ; php7_anonymous_async_keyword
          ; php7_anonymous_coroutine_keyword
          ; php7_anonymous_function_keyword
          ; php7_anonymous_left_paren
          ; php7_anonymous_parameters
          ; php7_anonymous_right_paren
          ; php7_anonymous_use
          ; php7_anonymous_colon
          ; php7_anonymous_type
          ; php7_anonymous_body
          }, results
      | SyntaxKind.AnonymousFunctionUseClause
      , (  anonymous_use_right_paren
        :: anonymous_use_variables
        :: anonymous_use_left_paren
        :: anonymous_use_keyword
        :: results
        ) ->
          AnonymousFunctionUseClause
          { anonymous_use_keyword
          ; anonymous_use_left_paren
          ; anonymous_use_variables
          ; anonymous_use_right_paren
          }, results
      | SyntaxKind.LambdaExpression
      , (  lambda_body
        :: lambda_arrow
        :: lambda_signature
        :: lambda_coroutine
        :: lambda_async
        :: results
        ) ->
          LambdaExpression
          { lambda_async
          ; lambda_coroutine
          ; lambda_signature
          ; lambda_arrow
          ; lambda_body
          }, results
      | SyntaxKind.LambdaSignature
      , (  lambda_type
        :: lambda_colon
        :: lambda_right_paren
        :: lambda_parameters
        :: lambda_left_paren
        :: results
        ) ->
          LambdaSignature
          { lambda_left_paren
          ; lambda_parameters
          ; lambda_right_paren
          ; lambda_colon
          ; lambda_type
          }, results
      | SyntaxKind.CastExpression
      , (  cast_operand
        :: cast_right_paren
        :: cast_type
        :: cast_left_paren
        :: results
        ) ->
          CastExpression
          { cast_left_paren
          ; cast_type
          ; cast_right_paren
          ; cast_operand
          }, results
      | SyntaxKind.ScopeResolutionExpression
      , (  scope_resolution_name
        :: scope_resolution_operator
        :: scope_resolution_qualifier
        :: results
        ) ->
          ScopeResolutionExpression
          { scope_resolution_qualifier
          ; scope_resolution_operator
          ; scope_resolution_name
          }, results
      | SyntaxKind.MemberSelectionExpression
      , (  member_name
        :: member_operator
        :: member_object
        :: results
        ) ->
          MemberSelectionExpression
          { member_object
          ; member_operator
          ; member_name
          }, results
      | SyntaxKind.SafeMemberSelectionExpression
      , (  safe_member_name
        :: safe_member_operator
        :: safe_member_object
        :: results
        ) ->
          SafeMemberSelectionExpression
          { safe_member_object
          ; safe_member_operator
          ; safe_member_name
          }, results
      | SyntaxKind.EmbeddedMemberSelectionExpression
      , (  embedded_member_name
        :: embedded_member_operator
        :: embedded_member_object
        :: results
        ) ->
          EmbeddedMemberSelectionExpression
          { embedded_member_object
          ; embedded_member_operator
          ; embedded_member_name
          }, results
      | SyntaxKind.YieldExpression
      , (  yield_operand
        :: yield_keyword
        :: results
        ) ->
          YieldExpression
          { yield_keyword
          ; yield_operand
          }, results
      | SyntaxKind.YieldFromExpression
      , (  yield_from_operand
        :: yield_from_from_keyword
        :: yield_from_yield_keyword
        :: results
        ) ->
          YieldFromExpression
          { yield_from_yield_keyword
          ; yield_from_from_keyword
          ; yield_from_operand
          }, results
      | SyntaxKind.PrefixUnaryExpression
      , (  prefix_unary_operand
        :: prefix_unary_operator
        :: results
        ) ->
          PrefixUnaryExpression
          { prefix_unary_operator
          ; prefix_unary_operand
          }, results
      | SyntaxKind.PostfixUnaryExpression
      , (  postfix_unary_operator
        :: postfix_unary_operand
        :: results
        ) ->
          PostfixUnaryExpression
          { postfix_unary_operand
          ; postfix_unary_operator
          }, results
      | SyntaxKind.BinaryExpression
      , (  binary_right_operand
        :: binary_operator
        :: binary_left_operand
        :: results
        ) ->
          BinaryExpression
          { binary_left_operand
          ; binary_operator
          ; binary_right_operand
          }, results
      | SyntaxKind.InstanceofExpression
      , (  instanceof_right_operand
        :: instanceof_operator
        :: instanceof_left_operand
        :: results
        ) ->
          InstanceofExpression
          { instanceof_left_operand
          ; instanceof_operator
          ; instanceof_right_operand
          }, results
      | SyntaxKind.IsExpression
      , (  is_right_operand
        :: is_operator
        :: is_left_operand
        :: results
        ) ->
          IsExpression
          { is_left_operand
          ; is_operator
          ; is_right_operand
          }, results
      | SyntaxKind.ConditionalExpression
      , (  conditional_alternative
        :: conditional_colon
        :: conditional_consequence
        :: conditional_question
        :: conditional_test
        :: results
        ) ->
          ConditionalExpression
          { conditional_test
          ; conditional_question
          ; conditional_consequence
          ; conditional_colon
          ; conditional_alternative
          }, results
      | SyntaxKind.EvalExpression
      , (  eval_right_paren
        :: eval_argument
        :: eval_left_paren
        :: eval_keyword
        :: results
        ) ->
          EvalExpression
          { eval_keyword
          ; eval_left_paren
          ; eval_argument
          ; eval_right_paren
          }, results
      | SyntaxKind.EmptyExpression
      , (  empty_right_paren
        :: empty_argument
        :: empty_left_paren
        :: empty_keyword
        :: results
        ) ->
          EmptyExpression
          { empty_keyword
          ; empty_left_paren
          ; empty_argument
          ; empty_right_paren
          }, results
      | SyntaxKind.DefineExpression
      , (  define_right_paren
        :: define_argument_list
        :: define_left_paren
        :: define_keyword
        :: results
        ) ->
          DefineExpression
          { define_keyword
          ; define_left_paren
          ; define_argument_list
          ; define_right_paren
          }, results
      | SyntaxKind.HaltCompilerExpression
      , (  halt_compiler_right_paren
        :: halt_compiler_argument_list
        :: halt_compiler_left_paren
        :: halt_compiler_keyword
        :: results
        ) ->
          HaltCompilerExpression
          { halt_compiler_keyword
          ; halt_compiler_left_paren
          ; halt_compiler_argument_list
          ; halt_compiler_right_paren
          }, results
      | SyntaxKind.IssetExpression
      , (  isset_right_paren
        :: isset_argument_list
        :: isset_left_paren
        :: isset_keyword
        :: results
        ) ->
          IssetExpression
          { isset_keyword
          ; isset_left_paren
          ; isset_argument_list
          ; isset_right_paren
          }, results
      | SyntaxKind.FunctionCallExpression
      , (  function_call_right_paren
        :: function_call_argument_list
        :: function_call_left_paren
        :: function_call_receiver
        :: results
        ) ->
          FunctionCallExpression
          { function_call_receiver
          ; function_call_left_paren
          ; function_call_argument_list
          ; function_call_right_paren
          }, results
      | SyntaxKind.FunctionCallWithTypeArgumentsExpression
      , (  function_call_with_type_arguments_right_paren
        :: function_call_with_type_arguments_argument_list
        :: function_call_with_type_arguments_left_paren
        :: function_call_with_type_arguments_type_args
        :: function_call_with_type_arguments_receiver
        :: results
        ) ->
          FunctionCallWithTypeArgumentsExpression
          { function_call_with_type_arguments_receiver
          ; function_call_with_type_arguments_type_args
          ; function_call_with_type_arguments_left_paren
          ; function_call_with_type_arguments_argument_list
          ; function_call_with_type_arguments_right_paren
          }, results
      | SyntaxKind.ParenthesizedExpression
      , (  parenthesized_expression_right_paren
        :: parenthesized_expression_expression
        :: parenthesized_expression_left_paren
        :: results
        ) ->
          ParenthesizedExpression
          { parenthesized_expression_left_paren
          ; parenthesized_expression_expression
          ; parenthesized_expression_right_paren
          }, results
      | SyntaxKind.BracedExpression
      , (  braced_expression_right_brace
        :: braced_expression_expression
        :: braced_expression_left_brace
        :: results
        ) ->
          BracedExpression
          { braced_expression_left_brace
          ; braced_expression_expression
          ; braced_expression_right_brace
          }, results
      | SyntaxKind.EmbeddedBracedExpression
      , (  embedded_braced_expression_right_brace
        :: embedded_braced_expression_expression
        :: embedded_braced_expression_left_brace
        :: results
        ) ->
          EmbeddedBracedExpression
          { embedded_braced_expression_left_brace
          ; embedded_braced_expression_expression
          ; embedded_braced_expression_right_brace
          }, results
      | SyntaxKind.ListExpression
      , (  list_right_paren
        :: list_members
        :: list_left_paren
        :: list_keyword
        :: results
        ) ->
          ListExpression
          { list_keyword
          ; list_left_paren
          ; list_members
          ; list_right_paren
          }, results
      | SyntaxKind.CollectionLiteralExpression
      , (  collection_literal_right_brace
        :: collection_literal_initializers
        :: collection_literal_left_brace
        :: collection_literal_name
        :: results
        ) ->
          CollectionLiteralExpression
          { collection_literal_name
          ; collection_literal_left_brace
          ; collection_literal_initializers
          ; collection_literal_right_brace
          }, results
      | SyntaxKind.ObjectCreationExpression
      , (  object_creation_object
        :: object_creation_new_keyword
        :: results
        ) ->
          ObjectCreationExpression
          { object_creation_new_keyword
          ; object_creation_object
          }, results
      | SyntaxKind.ConstructorCall
      , (  constructor_call_right_paren
        :: constructor_call_argument_list
        :: constructor_call_left_paren
        :: constructor_call_type
        :: results
        ) ->
          ConstructorCall
          { constructor_call_type
          ; constructor_call_left_paren
          ; constructor_call_argument_list
          ; constructor_call_right_paren
          }, results
      | SyntaxKind.ArrayCreationExpression
      , (  array_creation_right_bracket
        :: array_creation_members
        :: array_creation_left_bracket
        :: results
        ) ->
          ArrayCreationExpression
          { array_creation_left_bracket
          ; array_creation_members
          ; array_creation_right_bracket
          }, results
      | SyntaxKind.ArrayIntrinsicExpression
      , (  array_intrinsic_right_paren
        :: array_intrinsic_members
        :: array_intrinsic_left_paren
        :: array_intrinsic_keyword
        :: results
        ) ->
          ArrayIntrinsicExpression
          { array_intrinsic_keyword
          ; array_intrinsic_left_paren
          ; array_intrinsic_members
          ; array_intrinsic_right_paren
          }, results
      | SyntaxKind.DarrayIntrinsicExpression
      , (  darray_intrinsic_right_bracket
        :: darray_intrinsic_members
        :: darray_intrinsic_left_bracket
        :: darray_intrinsic_keyword
        :: results
        ) ->
          DarrayIntrinsicExpression
          { darray_intrinsic_keyword
          ; darray_intrinsic_left_bracket
          ; darray_intrinsic_members
          ; darray_intrinsic_right_bracket
          }, results
      | SyntaxKind.DictionaryIntrinsicExpression
      , (  dictionary_intrinsic_right_bracket
        :: dictionary_intrinsic_members
        :: dictionary_intrinsic_left_bracket
        :: dictionary_intrinsic_keyword
        :: results
        ) ->
          DictionaryIntrinsicExpression
          { dictionary_intrinsic_keyword
          ; dictionary_intrinsic_left_bracket
          ; dictionary_intrinsic_members
          ; dictionary_intrinsic_right_bracket
          }, results
      | SyntaxKind.KeysetIntrinsicExpression
      , (  keyset_intrinsic_right_bracket
        :: keyset_intrinsic_members
        :: keyset_intrinsic_left_bracket
        :: keyset_intrinsic_keyword
        :: results
        ) ->
          KeysetIntrinsicExpression
          { keyset_intrinsic_keyword
          ; keyset_intrinsic_left_bracket
          ; keyset_intrinsic_members
          ; keyset_intrinsic_right_bracket
          }, results
      | SyntaxKind.VarrayIntrinsicExpression
      , (  varray_intrinsic_right_bracket
        :: varray_intrinsic_members
        :: varray_intrinsic_left_bracket
        :: varray_intrinsic_keyword
        :: results
        ) ->
          VarrayIntrinsicExpression
          { varray_intrinsic_keyword
          ; varray_intrinsic_left_bracket
          ; varray_intrinsic_members
          ; varray_intrinsic_right_bracket
          }, results
      | SyntaxKind.VectorIntrinsicExpression
      , (  vector_intrinsic_right_bracket
        :: vector_intrinsic_members
        :: vector_intrinsic_left_bracket
        :: vector_intrinsic_keyword
        :: results
        ) ->
          VectorIntrinsicExpression
          { vector_intrinsic_keyword
          ; vector_intrinsic_left_bracket
          ; vector_intrinsic_members
          ; vector_intrinsic_right_bracket
          }, results
      | SyntaxKind.ElementInitializer
      , (  element_value
        :: element_arrow
        :: element_key
        :: results
        ) ->
          ElementInitializer
          { element_key
          ; element_arrow
          ; element_value
          }, results
      | SyntaxKind.SubscriptExpression
      , (  subscript_right_bracket
        :: subscript_index
        :: subscript_left_bracket
        :: subscript_receiver
        :: results
        ) ->
          SubscriptExpression
          { subscript_receiver
          ; subscript_left_bracket
          ; subscript_index
          ; subscript_right_bracket
          }, results
      | SyntaxKind.EmbeddedSubscriptExpression
      , (  embedded_subscript_right_bracket
        :: embedded_subscript_index
        :: embedded_subscript_left_bracket
        :: embedded_subscript_receiver
        :: results
        ) ->
          EmbeddedSubscriptExpression
          { embedded_subscript_receiver
          ; embedded_subscript_left_bracket
          ; embedded_subscript_index
          ; embedded_subscript_right_bracket
          }, results
      | SyntaxKind.AwaitableCreationExpression
      , (  awaitable_compound_statement
        :: awaitable_coroutine
        :: awaitable_async
        :: results
        ) ->
          AwaitableCreationExpression
          { awaitable_async
          ; awaitable_coroutine
          ; awaitable_compound_statement
          }, results
      | SyntaxKind.XHPChildrenDeclaration
      , (  xhp_children_semicolon
        :: xhp_children_expression
        :: xhp_children_keyword
        :: results
        ) ->
          XHPChildrenDeclaration
          { xhp_children_keyword
          ; xhp_children_expression
          ; xhp_children_semicolon
          }, results
      | SyntaxKind.XHPChildrenParenthesizedList
      , (  xhp_children_list_right_paren
        :: xhp_children_list_xhp_children
        :: xhp_children_list_left_paren
        :: results
        ) ->
          XHPChildrenParenthesizedList
          { xhp_children_list_left_paren
          ; xhp_children_list_xhp_children
          ; xhp_children_list_right_paren
          }, results
      | SyntaxKind.XHPCategoryDeclaration
      , (  xhp_category_semicolon
        :: xhp_category_categories
        :: xhp_category_keyword
        :: results
        ) ->
          XHPCategoryDeclaration
          { xhp_category_keyword
          ; xhp_category_categories
          ; xhp_category_semicolon
          }, results
      | SyntaxKind.XHPEnumType
      , (  xhp_enum_right_brace
        :: xhp_enum_values
        :: xhp_enum_left_brace
        :: xhp_enum_keyword
        :: xhp_enum_optional
        :: results
        ) ->
          XHPEnumType
          { xhp_enum_optional
          ; xhp_enum_keyword
          ; xhp_enum_left_brace
          ; xhp_enum_values
          ; xhp_enum_right_brace
          }, results
      | SyntaxKind.XHPRequired
      , (  xhp_required_keyword
        :: xhp_required_at
        :: results
        ) ->
          XHPRequired
          { xhp_required_at
          ; xhp_required_keyword
          }, results
      | SyntaxKind.XHPClassAttributeDeclaration
      , (  xhp_attribute_semicolon
        :: xhp_attribute_attributes
        :: xhp_attribute_keyword
        :: results
        ) ->
          XHPClassAttributeDeclaration
          { xhp_attribute_keyword
          ; xhp_attribute_attributes
          ; xhp_attribute_semicolon
          }, results
      | SyntaxKind.XHPClassAttribute
      , (  xhp_attribute_decl_required
        :: xhp_attribute_decl_initializer
        :: xhp_attribute_decl_name
        :: xhp_attribute_decl_type
        :: results
        ) ->
          XHPClassAttribute
          { xhp_attribute_decl_type
          ; xhp_attribute_decl_name
          ; xhp_attribute_decl_initializer
          ; xhp_attribute_decl_required
          }, results
      | SyntaxKind.XHPSimpleClassAttribute
      , (  xhp_simple_class_attribute_type
        :: results
        ) ->
          XHPSimpleClassAttribute
          { xhp_simple_class_attribute_type
          }, results
      | SyntaxKind.XHPSimpleAttribute
      , (  xhp_simple_attribute_expression
        :: xhp_simple_attribute_equal
        :: xhp_simple_attribute_name
        :: results
        ) ->
          XHPSimpleAttribute
          { xhp_simple_attribute_name
          ; xhp_simple_attribute_equal
          ; xhp_simple_attribute_expression
          }, results
      | SyntaxKind.XHPSpreadAttribute
      , (  xhp_spread_attribute_right_brace
        :: xhp_spread_attribute_expression
        :: xhp_spread_attribute_spread_operator
        :: xhp_spread_attribute_left_brace
        :: results
        ) ->
          XHPSpreadAttribute
          { xhp_spread_attribute_left_brace
          ; xhp_spread_attribute_spread_operator
          ; xhp_spread_attribute_expression
          ; xhp_spread_attribute_right_brace
          }, results
      | SyntaxKind.XHPOpen
      , (  xhp_open_right_angle
        :: xhp_open_attributes
        :: xhp_open_name
        :: xhp_open_left_angle
        :: results
        ) ->
          XHPOpen
          { xhp_open_left_angle
          ; xhp_open_name
          ; xhp_open_attributes
          ; xhp_open_right_angle
          }, results
      | SyntaxKind.XHPExpression
      , (  xhp_close
        :: xhp_body
        :: xhp_open
        :: results
        ) ->
          XHPExpression
          { xhp_open
          ; xhp_body
          ; xhp_close
          }, results
      | SyntaxKind.XHPClose
      , (  xhp_close_right_angle
        :: xhp_close_name
        :: xhp_close_left_angle
        :: results
        ) ->
          XHPClose
          { xhp_close_left_angle
          ; xhp_close_name
          ; xhp_close_right_angle
          }, results
      | SyntaxKind.TypeConstant
      , (  type_constant_right_type
        :: type_constant_separator
        :: type_constant_left_type
        :: results
        ) ->
          TypeConstant
          { type_constant_left_type
          ; type_constant_separator
          ; type_constant_right_type
          }, results
      | SyntaxKind.VectorTypeSpecifier
      , (  vector_type_right_angle
        :: vector_type_trailing_comma
        :: vector_type_type
        :: vector_type_left_angle
        :: vector_type_keyword
        :: results
        ) ->
          VectorTypeSpecifier
          { vector_type_keyword
          ; vector_type_left_angle
          ; vector_type_type
          ; vector_type_trailing_comma
          ; vector_type_right_angle
          }, results
      | SyntaxKind.KeysetTypeSpecifier
      , (  keyset_type_right_angle
        :: keyset_type_trailing_comma
        :: keyset_type_type
        :: keyset_type_left_angle
        :: keyset_type_keyword
        :: results
        ) ->
          KeysetTypeSpecifier
          { keyset_type_keyword
          ; keyset_type_left_angle
          ; keyset_type_type
          ; keyset_type_trailing_comma
          ; keyset_type_right_angle
          }, results
      | SyntaxKind.TupleTypeExplicitSpecifier
      , (  tuple_type_right_angle
        :: tuple_type_types
        :: tuple_type_left_angle
        :: tuple_type_keyword
        :: results
        ) ->
          TupleTypeExplicitSpecifier
          { tuple_type_keyword
          ; tuple_type_left_angle
          ; tuple_type_types
          ; tuple_type_right_angle
          }, results
      | SyntaxKind.VarrayTypeSpecifier
      , (  varray_right_angle
        :: varray_trailing_comma
        :: varray_type
        :: varray_left_angle
        :: varray_keyword
        :: results
        ) ->
          VarrayTypeSpecifier
          { varray_keyword
          ; varray_left_angle
          ; varray_type
          ; varray_trailing_comma
          ; varray_right_angle
          }, results
      | SyntaxKind.VectorArrayTypeSpecifier
      , (  vector_array_right_angle
        :: vector_array_type
        :: vector_array_left_angle
        :: vector_array_keyword
        :: results
        ) ->
          VectorArrayTypeSpecifier
          { vector_array_keyword
          ; vector_array_left_angle
          ; vector_array_type
          ; vector_array_right_angle
          }, results
      | SyntaxKind.TypeParameter
      , (  type_constraints
        :: type_name
        :: type_variance
        :: results
        ) ->
          TypeParameter
          { type_variance
          ; type_name
          ; type_constraints
          }, results
      | SyntaxKind.TypeConstraint
      , (  constraint_type
        :: constraint_keyword
        :: results
        ) ->
          TypeConstraint
          { constraint_keyword
          ; constraint_type
          }, results
      | SyntaxKind.DarrayTypeSpecifier
      , (  darray_right_angle
        :: darray_trailing_comma
        :: darray_value
        :: darray_comma
        :: darray_key
        :: darray_left_angle
        :: darray_keyword
        :: results
        ) ->
          DarrayTypeSpecifier
          { darray_keyword
          ; darray_left_angle
          ; darray_key
          ; darray_comma
          ; darray_value
          ; darray_trailing_comma
          ; darray_right_angle
          }, results
      | SyntaxKind.MapArrayTypeSpecifier
      , (  map_array_right_angle
        :: map_array_value
        :: map_array_comma
        :: map_array_key
        :: map_array_left_angle
        :: map_array_keyword
        :: results
        ) ->
          MapArrayTypeSpecifier
          { map_array_keyword
          ; map_array_left_angle
          ; map_array_key
          ; map_array_comma
          ; map_array_value
          ; map_array_right_angle
          }, results
      | SyntaxKind.DictionaryTypeSpecifier
      , (  dictionary_type_right_angle
        :: dictionary_type_members
        :: dictionary_type_left_angle
        :: dictionary_type_keyword
        :: results
        ) ->
          DictionaryTypeSpecifier
          { dictionary_type_keyword
          ; dictionary_type_left_angle
          ; dictionary_type_members
          ; dictionary_type_right_angle
          }, results
      | SyntaxKind.ClosureTypeSpecifier
      , (  closure_outer_right_paren
        :: closure_return_type
        :: closure_colon
        :: closure_inner_right_paren
        :: closure_parameter_list
        :: closure_inner_left_paren
        :: closure_function_keyword
        :: closure_coroutine
        :: closure_outer_left_paren
        :: results
        ) ->
          ClosureTypeSpecifier
          { closure_outer_left_paren
          ; closure_coroutine
          ; closure_function_keyword
          ; closure_inner_left_paren
          ; closure_parameter_list
          ; closure_inner_right_paren
          ; closure_colon
          ; closure_return_type
          ; closure_outer_right_paren
          }, results
      | SyntaxKind.ClosureParameterTypeSpecifier
      , (  closure_parameter_type
        :: closure_parameter_call_convention
        :: results
        ) ->
          ClosureParameterTypeSpecifier
          { closure_parameter_call_convention
          ; closure_parameter_type
          }, results
      | SyntaxKind.ClassnameTypeSpecifier
      , (  classname_right_angle
        :: classname_trailing_comma
        :: classname_type
        :: classname_left_angle
        :: classname_keyword
        :: results
        ) ->
          ClassnameTypeSpecifier
          { classname_keyword
          ; classname_left_angle
          ; classname_type
          ; classname_trailing_comma
          ; classname_right_angle
          }, results
      | SyntaxKind.FieldSpecifier
      , (  field_type
        :: field_arrow
        :: field_name
        :: field_question
        :: results
        ) ->
          FieldSpecifier
          { field_question
          ; field_name
          ; field_arrow
          ; field_type
          }, results
      | SyntaxKind.FieldInitializer
      , (  field_initializer_value
        :: field_initializer_arrow
        :: field_initializer_name
        :: results
        ) ->
          FieldInitializer
          { field_initializer_name
          ; field_initializer_arrow
          ; field_initializer_value
          }, results
      | SyntaxKind.ShapeTypeSpecifier
      , (  shape_type_right_paren
        :: shape_type_ellipsis
        :: shape_type_fields
        :: shape_type_left_paren
        :: shape_type_keyword
        :: results
        ) ->
          ShapeTypeSpecifier
          { shape_type_keyword
          ; shape_type_left_paren
          ; shape_type_fields
          ; shape_type_ellipsis
          ; shape_type_right_paren
          }, results
      | SyntaxKind.ShapeExpression
      , (  shape_expression_right_paren
        :: shape_expression_fields
        :: shape_expression_left_paren
        :: shape_expression_keyword
        :: results
        ) ->
          ShapeExpression
          { shape_expression_keyword
          ; shape_expression_left_paren
          ; shape_expression_fields
          ; shape_expression_right_paren
          }, results
      | SyntaxKind.TupleExpression
      , (  tuple_expression_right_paren
        :: tuple_expression_items
        :: tuple_expression_left_paren
        :: tuple_expression_keyword
        :: results
        ) ->
          TupleExpression
          { tuple_expression_keyword
          ; tuple_expression_left_paren
          ; tuple_expression_items
          ; tuple_expression_right_paren
          }, results
      | SyntaxKind.GenericTypeSpecifier
      , (  generic_argument_list
        :: generic_class_type
        :: results
        ) ->
          GenericTypeSpecifier
          { generic_class_type
          ; generic_argument_list
          }, results
      | SyntaxKind.NullableTypeSpecifier
      , (  nullable_type
        :: nullable_question
        :: results
        ) ->
          NullableTypeSpecifier
          { nullable_question
          ; nullable_type
          }, results
      | SyntaxKind.SoftTypeSpecifier
      , (  soft_type
        :: soft_at
        :: results
        ) ->
          SoftTypeSpecifier
          { soft_at
          ; soft_type
          }, results
      | SyntaxKind.TypeArguments
      , (  type_arguments_right_angle
        :: type_arguments_types
        :: type_arguments_left_angle
        :: results
        ) ->
          TypeArguments
          { type_arguments_left_angle
          ; type_arguments_types
          ; type_arguments_right_angle
          }, results
      | SyntaxKind.TypeParameters
      , (  type_parameters_right_angle
        :: type_parameters_parameters
        :: type_parameters_left_angle
        :: results
        ) ->
          TypeParameters
          { type_parameters_left_angle
          ; type_parameters_parameters
          ; type_parameters_right_angle
          }, results
      | SyntaxKind.TupleTypeSpecifier
      , (  tuple_right_paren
        :: tuple_types
        :: tuple_left_paren
        :: results
        ) ->
          TupleTypeSpecifier
          { tuple_left_paren
          ; tuple_types
          ; tuple_right_paren
          }, results
      | SyntaxKind.ErrorSyntax
      , (  error_error
        :: results
        ) ->
          ErrorSyntax
          { error_error
          }, results
      | SyntaxKind.ListItem
      , (  list_separator
        :: list_item
        :: results
        ) ->
          ListItem
          { list_item
          ; list_separator
          }, results

      | _ ->
        failwith @@ Printf.sprintf
          "BUILD: Failed to build %s with %d results."
          (SyntaxKind.to_string @@ M.kind minimal_t)
          (List.length results)
    in

    let rec dispatch (offset : int) (todo : todo) (results : t list) : t =
      match todo with
      | Build (node, node_offset, todo) ->
        let syntax, results = build node results in
        let results = make_syntax node syntax node_offset :: results in
        dispatch offset todo results
      | Convert (n, todo) -> convert offset todo results n
      | Done ->
        (match results with
        | [result] -> result
        | _  -> raise @@ Multiplicitous_conversion_result (List.length results)
        )
    and convert (offset : int) (todo : todo) (results : t list) : M.t -> t = function
    | { M.syntax = M.Token token; _ } as minimal_t ->
      let token = Token.from_minimal source_text token offset in
      let syntax = Token token in
      let node = make_syntax minimal_t syntax offset in
      let offset = offset + M.full_width minimal_t in
      dispatch offset todo (node :: results)
    | { M.syntax = M.Missing; _ } as minimal_t ->
      let node = make_syntax minimal_t Missing offset in
      dispatch offset todo (node :: results)
    | { M.syntax = M.SyntaxList l; _ } as minimal_t ->
      let todo = Build (minimal_t, offset, todo) in
      let todo = List.fold_right ~f:(fun n t -> Convert (n,t)) l ~init:todo in
      dispatch offset todo results
    | { M.syntax = M.EndOfFile x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.end_of_file_token
    | { M.syntax = M.Script x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.script_declarations
    | { M.syntax = M.QualifiedName x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.qualified_name_parts
    | { M.syntax = M.SimpleTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.simple_type_specifier
    | { M.syntax = M.LiteralExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.literal_expression
    | { M.syntax = M.VariableExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.variable_expression
    | { M.syntax = M.PipeVariableExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.pipe_variable_expression
    | { M.syntax = M.EnumDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.enum_right_brace, todo) in
        let todo = Convert (x.enum_enumerators, todo) in
        let todo = Convert (x.enum_left_brace, todo) in
        let todo = Convert (x.enum_type, todo) in
        let todo = Convert (x.enum_base, todo) in
        let todo = Convert (x.enum_colon, todo) in
        let todo = Convert (x.enum_name, todo) in
        let todo = Convert (x.enum_keyword, todo) in
        convert offset todo results x.enum_attribute_spec
    | { M.syntax = M.Enumerator x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.enumerator_semicolon, todo) in
        let todo = Convert (x.enumerator_value, todo) in
        let todo = Convert (x.enumerator_equal, todo) in
        convert offset todo results x.enumerator_name
    | { M.syntax = M.AliasDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.alias_semicolon, todo) in
        let todo = Convert (x.alias_type, todo) in
        let todo = Convert (x.alias_equal, todo) in
        let todo = Convert (x.alias_constraint, todo) in
        let todo = Convert (x.alias_generic_parameter, todo) in
        let todo = Convert (x.alias_name, todo) in
        let todo = Convert (x.alias_keyword, todo) in
        convert offset todo results x.alias_attribute_spec
    | { M.syntax = M.PropertyDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.property_semicolon, todo) in
        let todo = Convert (x.property_declarators, todo) in
        let todo = Convert (x.property_type, todo) in
        convert offset todo results x.property_modifiers
    | { M.syntax = M.PropertyDeclarator x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.property_initializer, todo) in
        convert offset todo results x.property_name
    | { M.syntax = M.NamespaceDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.namespace_body, todo) in
        let todo = Convert (x.namespace_name, todo) in
        convert offset todo results x.namespace_keyword
    | { M.syntax = M.NamespaceBody x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.namespace_right_brace, todo) in
        let todo = Convert (x.namespace_declarations, todo) in
        convert offset todo results x.namespace_left_brace
    | { M.syntax = M.NamespaceEmptyBody x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.namespace_semicolon
    | { M.syntax = M.NamespaceUseDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.namespace_use_semicolon, todo) in
        let todo = Convert (x.namespace_use_clauses, todo) in
        let todo = Convert (x.namespace_use_kind, todo) in
        convert offset todo results x.namespace_use_keyword
    | { M.syntax = M.NamespaceGroupUseDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.namespace_group_use_semicolon, todo) in
        let todo = Convert (x.namespace_group_use_right_brace, todo) in
        let todo = Convert (x.namespace_group_use_clauses, todo) in
        let todo = Convert (x.namespace_group_use_left_brace, todo) in
        let todo = Convert (x.namespace_group_use_prefix, todo) in
        let todo = Convert (x.namespace_group_use_kind, todo) in
        convert offset todo results x.namespace_group_use_keyword
    | { M.syntax = M.NamespaceUseClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.namespace_use_alias, todo) in
        let todo = Convert (x.namespace_use_as, todo) in
        let todo = Convert (x.namespace_use_name, todo) in
        convert offset todo results x.namespace_use_clause_kind
    | { M.syntax = M.FunctionDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.function_body, todo) in
        let todo = Convert (x.function_declaration_header, todo) in
        convert offset todo results x.function_attribute_spec
    | { M.syntax = M.FunctionDeclarationHeader x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.function_where_clause, todo) in
        let todo = Convert (x.function_type, todo) in
        let todo = Convert (x.function_colon, todo) in
        let todo = Convert (x.function_right_paren, todo) in
        let todo = Convert (x.function_parameter_list, todo) in
        let todo = Convert (x.function_left_paren, todo) in
        let todo = Convert (x.function_type_parameter_list, todo) in
        let todo = Convert (x.function_name, todo) in
        let todo = Convert (x.function_ampersand, todo) in
        let todo = Convert (x.function_keyword, todo) in
        convert offset todo results x.function_modifiers
    | { M.syntax = M.WhereClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.where_clause_constraints, todo) in
        convert offset todo results x.where_clause_keyword
    | { M.syntax = M.WhereConstraint x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.where_constraint_right_type, todo) in
        let todo = Convert (x.where_constraint_operator, todo) in
        convert offset todo results x.where_constraint_left_type
    | { M.syntax = M.MethodishDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.methodish_semicolon, todo) in
        let todo = Convert (x.methodish_function_body, todo) in
        let todo = Convert (x.methodish_function_decl_header, todo) in
        convert offset todo results x.methodish_attribute
    | { M.syntax = M.ClassishDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.classish_body, todo) in
        let todo = Convert (x.classish_implements_list, todo) in
        let todo = Convert (x.classish_implements_keyword, todo) in
        let todo = Convert (x.classish_extends_list, todo) in
        let todo = Convert (x.classish_extends_keyword, todo) in
        let todo = Convert (x.classish_type_parameters, todo) in
        let todo = Convert (x.classish_name, todo) in
        let todo = Convert (x.classish_keyword, todo) in
        let todo = Convert (x.classish_modifiers, todo) in
        convert offset todo results x.classish_attribute
    | { M.syntax = M.ClassishBody x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.classish_body_right_brace, todo) in
        let todo = Convert (x.classish_body_elements, todo) in
        convert offset todo results x.classish_body_left_brace
    | { M.syntax = M.TraitUsePrecedenceItem x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.trait_use_precedence_item_removed_names, todo) in
        let todo = Convert (x.trait_use_precedence_item_keyword, todo) in
        convert offset todo results x.trait_use_precedence_item_name
    | { M.syntax = M.TraitUseAliasItem x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.trait_use_alias_item_aliased_name, todo) in
        let todo = Convert (x.trait_use_alias_item_modifiers, todo) in
        let todo = Convert (x.trait_use_alias_item_keyword, todo) in
        convert offset todo results x.trait_use_alias_item_aliasing_name
    | { M.syntax = M.TraitUseConflictResolution x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.trait_use_conflict_resolution_right_brace, todo) in
        let todo = Convert (x.trait_use_conflict_resolution_clauses, todo) in
        let todo = Convert (x.trait_use_conflict_resolution_left_brace, todo) in
        let todo = Convert (x.trait_use_conflict_resolution_names, todo) in
        convert offset todo results x.trait_use_conflict_resolution_keyword
    | { M.syntax = M.TraitUse x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.trait_use_semicolon, todo) in
        let todo = Convert (x.trait_use_names, todo) in
        convert offset todo results x.trait_use_keyword
    | { M.syntax = M.RequireClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.require_semicolon, todo) in
        let todo = Convert (x.require_name, todo) in
        let todo = Convert (x.require_kind, todo) in
        convert offset todo results x.require_keyword
    | { M.syntax = M.ConstDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.const_semicolon, todo) in
        let todo = Convert (x.const_declarators, todo) in
        let todo = Convert (x.const_type_specifier, todo) in
        let todo = Convert (x.const_keyword, todo) in
        convert offset todo results x.const_abstract
    | { M.syntax = M.ConstantDeclarator x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.constant_declarator_initializer, todo) in
        convert offset todo results x.constant_declarator_name
    | { M.syntax = M.TypeConstDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.type_const_semicolon, todo) in
        let todo = Convert (x.type_const_type_specifier, todo) in
        let todo = Convert (x.type_const_equal, todo) in
        let todo = Convert (x.type_const_type_constraint, todo) in
        let todo = Convert (x.type_const_type_parameters, todo) in
        let todo = Convert (x.type_const_name, todo) in
        let todo = Convert (x.type_const_type_keyword, todo) in
        let todo = Convert (x.type_const_keyword, todo) in
        convert offset todo results x.type_const_abstract
    | { M.syntax = M.DecoratedExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.decorated_expression_expression, todo) in
        convert offset todo results x.decorated_expression_decorator
    | { M.syntax = M.ParameterDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.parameter_default_value, todo) in
        let todo = Convert (x.parameter_name, todo) in
        let todo = Convert (x.parameter_type, todo) in
        let todo = Convert (x.parameter_call_convention, todo) in
        let todo = Convert (x.parameter_visibility, todo) in
        convert offset todo results x.parameter_attribute
    | { M.syntax = M.VariadicParameter x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.variadic_parameter_ellipsis, todo) in
        let todo = Convert (x.variadic_parameter_type, todo) in
        convert offset todo results x.variadic_parameter_call_convention
    | { M.syntax = M.AttributeSpecification x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.attribute_specification_right_double_angle, todo) in
        let todo = Convert (x.attribute_specification_attributes, todo) in
        convert offset todo results x.attribute_specification_left_double_angle
    | { M.syntax = M.Attribute x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.attribute_right_paren, todo) in
        let todo = Convert (x.attribute_values, todo) in
        let todo = Convert (x.attribute_left_paren, todo) in
        convert offset todo results x.attribute_name
    | { M.syntax = M.InclusionExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.inclusion_filename, todo) in
        convert offset todo results x.inclusion_require
    | { M.syntax = M.InclusionDirective x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.inclusion_semicolon, todo) in
        convert offset todo results x.inclusion_expression
    | { M.syntax = M.CompoundStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.compound_right_brace, todo) in
        let todo = Convert (x.compound_statements, todo) in
        convert offset todo results x.compound_left_brace
    | { M.syntax = M.ExpressionStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.expression_statement_semicolon, todo) in
        convert offset todo results x.expression_statement_expression
    | { M.syntax = M.MarkupSection x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.markup_expression, todo) in
        let todo = Convert (x.markup_suffix, todo) in
        let todo = Convert (x.markup_text, todo) in
        convert offset todo results x.markup_prefix
    | { M.syntax = M.MarkupSuffix x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.markup_suffix_name, todo) in
        convert offset todo results x.markup_suffix_less_than_question
    | { M.syntax = M.UnsetStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.unset_semicolon, todo) in
        let todo = Convert (x.unset_right_paren, todo) in
        let todo = Convert (x.unset_variables, todo) in
        let todo = Convert (x.unset_left_paren, todo) in
        convert offset todo results x.unset_keyword
    | { M.syntax = M.UsingStatementBlockScoped x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.using_block_body, todo) in
        let todo = Convert (x.using_block_right_paren, todo) in
        let todo = Convert (x.using_block_expressions, todo) in
        let todo = Convert (x.using_block_left_paren, todo) in
        let todo = Convert (x.using_block_using_keyword, todo) in
        convert offset todo results x.using_block_await_keyword
    | { M.syntax = M.UsingStatementFunctionScoped x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.using_function_semicolon, todo) in
        let todo = Convert (x.using_function_expression, todo) in
        let todo = Convert (x.using_function_using_keyword, todo) in
        convert offset todo results x.using_function_await_keyword
    | { M.syntax = M.DeclareDirectiveStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.declare_directive_semicolon, todo) in
        let todo = Convert (x.declare_directive_right_paren, todo) in
        let todo = Convert (x.declare_directive_expression, todo) in
        let todo = Convert (x.declare_directive_left_paren, todo) in
        convert offset todo results x.declare_directive_keyword
    | { M.syntax = M.DeclareBlockStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.declare_block_body, todo) in
        let todo = Convert (x.declare_block_right_paren, todo) in
        let todo = Convert (x.declare_block_expression, todo) in
        let todo = Convert (x.declare_block_left_paren, todo) in
        convert offset todo results x.declare_block_keyword
    | { M.syntax = M.WhileStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.while_body, todo) in
        let todo = Convert (x.while_right_paren, todo) in
        let todo = Convert (x.while_condition, todo) in
        let todo = Convert (x.while_left_paren, todo) in
        convert offset todo results x.while_keyword
    | { M.syntax = M.IfStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.if_else_clause, todo) in
        let todo = Convert (x.if_elseif_clauses, todo) in
        let todo = Convert (x.if_statement, todo) in
        let todo = Convert (x.if_right_paren, todo) in
        let todo = Convert (x.if_condition, todo) in
        let todo = Convert (x.if_left_paren, todo) in
        convert offset todo results x.if_keyword
    | { M.syntax = M.ElseifClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.elseif_statement, todo) in
        let todo = Convert (x.elseif_right_paren, todo) in
        let todo = Convert (x.elseif_condition, todo) in
        let todo = Convert (x.elseif_left_paren, todo) in
        convert offset todo results x.elseif_keyword
    | { M.syntax = M.ElseClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.else_statement, todo) in
        convert offset todo results x.else_keyword
    | { M.syntax = M.IfEndIfStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.if_endif_semicolon, todo) in
        let todo = Convert (x.if_endif_endif_keyword, todo) in
        let todo = Convert (x.if_endif_else_colon_clause, todo) in
        let todo = Convert (x.if_endif_elseif_colon_clauses, todo) in
        let todo = Convert (x.if_endif_statement, todo) in
        let todo = Convert (x.if_endif_colon, todo) in
        let todo = Convert (x.if_endif_right_paren, todo) in
        let todo = Convert (x.if_endif_condition, todo) in
        let todo = Convert (x.if_endif_left_paren, todo) in
        convert offset todo results x.if_endif_keyword
    | { M.syntax = M.ElseifColonClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.elseif_colon_statement, todo) in
        let todo = Convert (x.elseif_colon_colon, todo) in
        let todo = Convert (x.elseif_colon_right_paren, todo) in
        let todo = Convert (x.elseif_colon_condition, todo) in
        let todo = Convert (x.elseif_colon_left_paren, todo) in
        convert offset todo results x.elseif_colon_keyword
    | { M.syntax = M.ElseColonClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.else_colon_statement, todo) in
        let todo = Convert (x.else_colon_colon, todo) in
        convert offset todo results x.else_colon_keyword
    | { M.syntax = M.TryStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.try_finally_clause, todo) in
        let todo = Convert (x.try_catch_clauses, todo) in
        let todo = Convert (x.try_compound_statement, todo) in
        convert offset todo results x.try_keyword
    | { M.syntax = M.CatchClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.catch_body, todo) in
        let todo = Convert (x.catch_right_paren, todo) in
        let todo = Convert (x.catch_variable, todo) in
        let todo = Convert (x.catch_type, todo) in
        let todo = Convert (x.catch_left_paren, todo) in
        convert offset todo results x.catch_keyword
    | { M.syntax = M.FinallyClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.finally_body, todo) in
        convert offset todo results x.finally_keyword
    | { M.syntax = M.DoStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.do_semicolon, todo) in
        let todo = Convert (x.do_right_paren, todo) in
        let todo = Convert (x.do_condition, todo) in
        let todo = Convert (x.do_left_paren, todo) in
        let todo = Convert (x.do_while_keyword, todo) in
        let todo = Convert (x.do_body, todo) in
        convert offset todo results x.do_keyword
    | { M.syntax = M.ForStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.for_body, todo) in
        let todo = Convert (x.for_right_paren, todo) in
        let todo = Convert (x.for_end_of_loop, todo) in
        let todo = Convert (x.for_second_semicolon, todo) in
        let todo = Convert (x.for_control, todo) in
        let todo = Convert (x.for_first_semicolon, todo) in
        let todo = Convert (x.for_initializer, todo) in
        let todo = Convert (x.for_left_paren, todo) in
        convert offset todo results x.for_keyword
    | { M.syntax = M.ForeachStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.foreach_body, todo) in
        let todo = Convert (x.foreach_right_paren, todo) in
        let todo = Convert (x.foreach_value, todo) in
        let todo = Convert (x.foreach_arrow, todo) in
        let todo = Convert (x.foreach_key, todo) in
        let todo = Convert (x.foreach_as, todo) in
        let todo = Convert (x.foreach_await_keyword, todo) in
        let todo = Convert (x.foreach_collection, todo) in
        let todo = Convert (x.foreach_left_paren, todo) in
        convert offset todo results x.foreach_keyword
    | { M.syntax = M.SwitchStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.switch_right_brace, todo) in
        let todo = Convert (x.switch_sections, todo) in
        let todo = Convert (x.switch_left_brace, todo) in
        let todo = Convert (x.switch_right_paren, todo) in
        let todo = Convert (x.switch_expression, todo) in
        let todo = Convert (x.switch_left_paren, todo) in
        convert offset todo results x.switch_keyword
    | { M.syntax = M.SwitchSection x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.switch_section_fallthrough, todo) in
        let todo = Convert (x.switch_section_statements, todo) in
        convert offset todo results x.switch_section_labels
    | { M.syntax = M.SwitchFallthrough x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.fallthrough_semicolon, todo) in
        convert offset todo results x.fallthrough_keyword
    | { M.syntax = M.CaseLabel x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.case_colon, todo) in
        let todo = Convert (x.case_expression, todo) in
        convert offset todo results x.case_keyword
    | { M.syntax = M.DefaultLabel x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.default_colon, todo) in
        convert offset todo results x.default_keyword
    | { M.syntax = M.ReturnStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.return_semicolon, todo) in
        let todo = Convert (x.return_expression, todo) in
        convert offset todo results x.return_keyword
    | { M.syntax = M.GotoLabel x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.goto_label_colon, todo) in
        convert offset todo results x.goto_label_name
    | { M.syntax = M.GotoStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.goto_statement_semicolon, todo) in
        let todo = Convert (x.goto_statement_label_name, todo) in
        convert offset todo results x.goto_statement_keyword
    | { M.syntax = M.ThrowStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.throw_semicolon, todo) in
        let todo = Convert (x.throw_expression, todo) in
        convert offset todo results x.throw_keyword
    | { M.syntax = M.BreakStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.break_semicolon, todo) in
        let todo = Convert (x.break_level, todo) in
        convert offset todo results x.break_keyword
    | { M.syntax = M.ContinueStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.continue_semicolon, todo) in
        let todo = Convert (x.continue_level, todo) in
        convert offset todo results x.continue_keyword
    | { M.syntax = M.FunctionStaticStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.static_semicolon, todo) in
        let todo = Convert (x.static_declarations, todo) in
        convert offset todo results x.static_static_keyword
    | { M.syntax = M.StaticDeclarator x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.static_initializer, todo) in
        convert offset todo results x.static_name
    | { M.syntax = M.EchoStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.echo_semicolon, todo) in
        let todo = Convert (x.echo_expressions, todo) in
        convert offset todo results x.echo_keyword
    | { M.syntax = M.GlobalStatement x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.global_semicolon, todo) in
        let todo = Convert (x.global_variables, todo) in
        convert offset todo results x.global_keyword
    | { M.syntax = M.SimpleInitializer x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.simple_initializer_value, todo) in
        convert offset todo results x.simple_initializer_equal
    | { M.syntax = M.AnonymousClass x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.anonymous_class_body, todo) in
        let todo = Convert (x.anonymous_class_implements_list, todo) in
        let todo = Convert (x.anonymous_class_implements_keyword, todo) in
        let todo = Convert (x.anonymous_class_extends_list, todo) in
        let todo = Convert (x.anonymous_class_extends_keyword, todo) in
        let todo = Convert (x.anonymous_class_right_paren, todo) in
        let todo = Convert (x.anonymous_class_argument_list, todo) in
        let todo = Convert (x.anonymous_class_left_paren, todo) in
        convert offset todo results x.anonymous_class_class_keyword
    | { M.syntax = M.AnonymousFunction x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.anonymous_body, todo) in
        let todo = Convert (x.anonymous_use, todo) in
        let todo = Convert (x.anonymous_type, todo) in
        let todo = Convert (x.anonymous_colon, todo) in
        let todo = Convert (x.anonymous_right_paren, todo) in
        let todo = Convert (x.anonymous_parameters, todo) in
        let todo = Convert (x.anonymous_left_paren, todo) in
        let todo = Convert (x.anonymous_function_keyword, todo) in
        let todo = Convert (x.anonymous_coroutine_keyword, todo) in
        let todo = Convert (x.anonymous_async_keyword, todo) in
        convert offset todo results x.anonymous_static_keyword
    | { M.syntax = M.Php7AnonymousFunction x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.php7_anonymous_body, todo) in
        let todo = Convert (x.php7_anonymous_type, todo) in
        let todo = Convert (x.php7_anonymous_colon, todo) in
        let todo = Convert (x.php7_anonymous_use, todo) in
        let todo = Convert (x.php7_anonymous_right_paren, todo) in
        let todo = Convert (x.php7_anonymous_parameters, todo) in
        let todo = Convert (x.php7_anonymous_left_paren, todo) in
        let todo = Convert (x.php7_anonymous_function_keyword, todo) in
        let todo = Convert (x.php7_anonymous_coroutine_keyword, todo) in
        let todo = Convert (x.php7_anonymous_async_keyword, todo) in
        convert offset todo results x.php7_anonymous_static_keyword
    | { M.syntax = M.AnonymousFunctionUseClause x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.anonymous_use_right_paren, todo) in
        let todo = Convert (x.anonymous_use_variables, todo) in
        let todo = Convert (x.anonymous_use_left_paren, todo) in
        convert offset todo results x.anonymous_use_keyword
    | { M.syntax = M.LambdaExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.lambda_body, todo) in
        let todo = Convert (x.lambda_arrow, todo) in
        let todo = Convert (x.lambda_signature, todo) in
        let todo = Convert (x.lambda_coroutine, todo) in
        convert offset todo results x.lambda_async
    | { M.syntax = M.LambdaSignature x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.lambda_type, todo) in
        let todo = Convert (x.lambda_colon, todo) in
        let todo = Convert (x.lambda_right_paren, todo) in
        let todo = Convert (x.lambda_parameters, todo) in
        convert offset todo results x.lambda_left_paren
    | { M.syntax = M.CastExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.cast_operand, todo) in
        let todo = Convert (x.cast_right_paren, todo) in
        let todo = Convert (x.cast_type, todo) in
        convert offset todo results x.cast_left_paren
    | { M.syntax = M.ScopeResolutionExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.scope_resolution_name, todo) in
        let todo = Convert (x.scope_resolution_operator, todo) in
        convert offset todo results x.scope_resolution_qualifier
    | { M.syntax = M.MemberSelectionExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.member_name, todo) in
        let todo = Convert (x.member_operator, todo) in
        convert offset todo results x.member_object
    | { M.syntax = M.SafeMemberSelectionExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.safe_member_name, todo) in
        let todo = Convert (x.safe_member_operator, todo) in
        convert offset todo results x.safe_member_object
    | { M.syntax = M.EmbeddedMemberSelectionExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.embedded_member_name, todo) in
        let todo = Convert (x.embedded_member_operator, todo) in
        convert offset todo results x.embedded_member_object
    | { M.syntax = M.YieldExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.yield_operand, todo) in
        convert offset todo results x.yield_keyword
    | { M.syntax = M.YieldFromExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.yield_from_operand, todo) in
        let todo = Convert (x.yield_from_from_keyword, todo) in
        convert offset todo results x.yield_from_yield_keyword
    | { M.syntax = M.PrefixUnaryExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.prefix_unary_operand, todo) in
        convert offset todo results x.prefix_unary_operator
    | { M.syntax = M.PostfixUnaryExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.postfix_unary_operator, todo) in
        convert offset todo results x.postfix_unary_operand
    | { M.syntax = M.BinaryExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.binary_right_operand, todo) in
        let todo = Convert (x.binary_operator, todo) in
        convert offset todo results x.binary_left_operand
    | { M.syntax = M.InstanceofExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.instanceof_right_operand, todo) in
        let todo = Convert (x.instanceof_operator, todo) in
        convert offset todo results x.instanceof_left_operand
    | { M.syntax = M.IsExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.is_right_operand, todo) in
        let todo = Convert (x.is_operator, todo) in
        convert offset todo results x.is_left_operand
    | { M.syntax = M.ConditionalExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.conditional_alternative, todo) in
        let todo = Convert (x.conditional_colon, todo) in
        let todo = Convert (x.conditional_consequence, todo) in
        let todo = Convert (x.conditional_question, todo) in
        convert offset todo results x.conditional_test
    | { M.syntax = M.EvalExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.eval_right_paren, todo) in
        let todo = Convert (x.eval_argument, todo) in
        let todo = Convert (x.eval_left_paren, todo) in
        convert offset todo results x.eval_keyword
    | { M.syntax = M.EmptyExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.empty_right_paren, todo) in
        let todo = Convert (x.empty_argument, todo) in
        let todo = Convert (x.empty_left_paren, todo) in
        convert offset todo results x.empty_keyword
    | { M.syntax = M.DefineExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.define_right_paren, todo) in
        let todo = Convert (x.define_argument_list, todo) in
        let todo = Convert (x.define_left_paren, todo) in
        convert offset todo results x.define_keyword
    | { M.syntax = M.HaltCompilerExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.halt_compiler_right_paren, todo) in
        let todo = Convert (x.halt_compiler_argument_list, todo) in
        let todo = Convert (x.halt_compiler_left_paren, todo) in
        convert offset todo results x.halt_compiler_keyword
    | { M.syntax = M.IssetExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.isset_right_paren, todo) in
        let todo = Convert (x.isset_argument_list, todo) in
        let todo = Convert (x.isset_left_paren, todo) in
        convert offset todo results x.isset_keyword
    | { M.syntax = M.FunctionCallExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.function_call_right_paren, todo) in
        let todo = Convert (x.function_call_argument_list, todo) in
        let todo = Convert (x.function_call_left_paren, todo) in
        convert offset todo results x.function_call_receiver
    | { M.syntax = M.FunctionCallWithTypeArgumentsExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.function_call_with_type_arguments_right_paren, todo) in
        let todo = Convert (x.function_call_with_type_arguments_argument_list, todo) in
        let todo = Convert (x.function_call_with_type_arguments_left_paren, todo) in
        let todo = Convert (x.function_call_with_type_arguments_type_args, todo) in
        convert offset todo results x.function_call_with_type_arguments_receiver
    | { M.syntax = M.ParenthesizedExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.parenthesized_expression_right_paren, todo) in
        let todo = Convert (x.parenthesized_expression_expression, todo) in
        convert offset todo results x.parenthesized_expression_left_paren
    | { M.syntax = M.BracedExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.braced_expression_right_brace, todo) in
        let todo = Convert (x.braced_expression_expression, todo) in
        convert offset todo results x.braced_expression_left_brace
    | { M.syntax = M.EmbeddedBracedExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.embedded_braced_expression_right_brace, todo) in
        let todo = Convert (x.embedded_braced_expression_expression, todo) in
        convert offset todo results x.embedded_braced_expression_left_brace
    | { M.syntax = M.ListExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.list_right_paren, todo) in
        let todo = Convert (x.list_members, todo) in
        let todo = Convert (x.list_left_paren, todo) in
        convert offset todo results x.list_keyword
    | { M.syntax = M.CollectionLiteralExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.collection_literal_right_brace, todo) in
        let todo = Convert (x.collection_literal_initializers, todo) in
        let todo = Convert (x.collection_literal_left_brace, todo) in
        convert offset todo results x.collection_literal_name
    | { M.syntax = M.ObjectCreationExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.object_creation_object, todo) in
        convert offset todo results x.object_creation_new_keyword
    | { M.syntax = M.ConstructorCall x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.constructor_call_right_paren, todo) in
        let todo = Convert (x.constructor_call_argument_list, todo) in
        let todo = Convert (x.constructor_call_left_paren, todo) in
        convert offset todo results x.constructor_call_type
    | { M.syntax = M.ArrayCreationExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.array_creation_right_bracket, todo) in
        let todo = Convert (x.array_creation_members, todo) in
        convert offset todo results x.array_creation_left_bracket
    | { M.syntax = M.ArrayIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.array_intrinsic_right_paren, todo) in
        let todo = Convert (x.array_intrinsic_members, todo) in
        let todo = Convert (x.array_intrinsic_left_paren, todo) in
        convert offset todo results x.array_intrinsic_keyword
    | { M.syntax = M.DarrayIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.darray_intrinsic_right_bracket, todo) in
        let todo = Convert (x.darray_intrinsic_members, todo) in
        let todo = Convert (x.darray_intrinsic_left_bracket, todo) in
        convert offset todo results x.darray_intrinsic_keyword
    | { M.syntax = M.DictionaryIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.dictionary_intrinsic_right_bracket, todo) in
        let todo = Convert (x.dictionary_intrinsic_members, todo) in
        let todo = Convert (x.dictionary_intrinsic_left_bracket, todo) in
        convert offset todo results x.dictionary_intrinsic_keyword
    | { M.syntax = M.KeysetIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.keyset_intrinsic_right_bracket, todo) in
        let todo = Convert (x.keyset_intrinsic_members, todo) in
        let todo = Convert (x.keyset_intrinsic_left_bracket, todo) in
        convert offset todo results x.keyset_intrinsic_keyword
    | { M.syntax = M.VarrayIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.varray_intrinsic_right_bracket, todo) in
        let todo = Convert (x.varray_intrinsic_members, todo) in
        let todo = Convert (x.varray_intrinsic_left_bracket, todo) in
        convert offset todo results x.varray_intrinsic_keyword
    | { M.syntax = M.VectorIntrinsicExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.vector_intrinsic_right_bracket, todo) in
        let todo = Convert (x.vector_intrinsic_members, todo) in
        let todo = Convert (x.vector_intrinsic_left_bracket, todo) in
        convert offset todo results x.vector_intrinsic_keyword
    | { M.syntax = M.ElementInitializer x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.element_value, todo) in
        let todo = Convert (x.element_arrow, todo) in
        convert offset todo results x.element_key
    | { M.syntax = M.SubscriptExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.subscript_right_bracket, todo) in
        let todo = Convert (x.subscript_index, todo) in
        let todo = Convert (x.subscript_left_bracket, todo) in
        convert offset todo results x.subscript_receiver
    | { M.syntax = M.EmbeddedSubscriptExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.embedded_subscript_right_bracket, todo) in
        let todo = Convert (x.embedded_subscript_index, todo) in
        let todo = Convert (x.embedded_subscript_left_bracket, todo) in
        convert offset todo results x.embedded_subscript_receiver
    | { M.syntax = M.AwaitableCreationExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.awaitable_compound_statement, todo) in
        let todo = Convert (x.awaitable_coroutine, todo) in
        convert offset todo results x.awaitable_async
    | { M.syntax = M.XHPChildrenDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_children_semicolon, todo) in
        let todo = Convert (x.xhp_children_expression, todo) in
        convert offset todo results x.xhp_children_keyword
    | { M.syntax = M.XHPChildrenParenthesizedList x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_children_list_right_paren, todo) in
        let todo = Convert (x.xhp_children_list_xhp_children, todo) in
        convert offset todo results x.xhp_children_list_left_paren
    | { M.syntax = M.XHPCategoryDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_category_semicolon, todo) in
        let todo = Convert (x.xhp_category_categories, todo) in
        convert offset todo results x.xhp_category_keyword
    | { M.syntax = M.XHPEnumType x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_enum_right_brace, todo) in
        let todo = Convert (x.xhp_enum_values, todo) in
        let todo = Convert (x.xhp_enum_left_brace, todo) in
        let todo = Convert (x.xhp_enum_keyword, todo) in
        convert offset todo results x.xhp_enum_optional
    | { M.syntax = M.XHPRequired x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_required_keyword, todo) in
        convert offset todo results x.xhp_required_at
    | { M.syntax = M.XHPClassAttributeDeclaration x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_attribute_semicolon, todo) in
        let todo = Convert (x.xhp_attribute_attributes, todo) in
        convert offset todo results x.xhp_attribute_keyword
    | { M.syntax = M.XHPClassAttribute x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_attribute_decl_required, todo) in
        let todo = Convert (x.xhp_attribute_decl_initializer, todo) in
        let todo = Convert (x.xhp_attribute_decl_name, todo) in
        convert offset todo results x.xhp_attribute_decl_type
    | { M.syntax = M.XHPSimpleClassAttribute x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.xhp_simple_class_attribute_type
    | { M.syntax = M.XHPSimpleAttribute x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_simple_attribute_expression, todo) in
        let todo = Convert (x.xhp_simple_attribute_equal, todo) in
        convert offset todo results x.xhp_simple_attribute_name
    | { M.syntax = M.XHPSpreadAttribute x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_spread_attribute_right_brace, todo) in
        let todo = Convert (x.xhp_spread_attribute_expression, todo) in
        let todo = Convert (x.xhp_spread_attribute_spread_operator, todo) in
        convert offset todo results x.xhp_spread_attribute_left_brace
    | { M.syntax = M.XHPOpen x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_open_right_angle, todo) in
        let todo = Convert (x.xhp_open_attributes, todo) in
        let todo = Convert (x.xhp_open_name, todo) in
        convert offset todo results x.xhp_open_left_angle
    | { M.syntax = M.XHPExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_close, todo) in
        let todo = Convert (x.xhp_body, todo) in
        convert offset todo results x.xhp_open
    | { M.syntax = M.XHPClose x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.xhp_close_right_angle, todo) in
        let todo = Convert (x.xhp_close_name, todo) in
        convert offset todo results x.xhp_close_left_angle
    | { M.syntax = M.TypeConstant x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.type_constant_right_type, todo) in
        let todo = Convert (x.type_constant_separator, todo) in
        convert offset todo results x.type_constant_left_type
    | { M.syntax = M.VectorTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.vector_type_right_angle, todo) in
        let todo = Convert (x.vector_type_trailing_comma, todo) in
        let todo = Convert (x.vector_type_type, todo) in
        let todo = Convert (x.vector_type_left_angle, todo) in
        convert offset todo results x.vector_type_keyword
    | { M.syntax = M.KeysetTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.keyset_type_right_angle, todo) in
        let todo = Convert (x.keyset_type_trailing_comma, todo) in
        let todo = Convert (x.keyset_type_type, todo) in
        let todo = Convert (x.keyset_type_left_angle, todo) in
        convert offset todo results x.keyset_type_keyword
    | { M.syntax = M.TupleTypeExplicitSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.tuple_type_right_angle, todo) in
        let todo = Convert (x.tuple_type_types, todo) in
        let todo = Convert (x.tuple_type_left_angle, todo) in
        convert offset todo results x.tuple_type_keyword
    | { M.syntax = M.VarrayTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.varray_right_angle, todo) in
        let todo = Convert (x.varray_trailing_comma, todo) in
        let todo = Convert (x.varray_type, todo) in
        let todo = Convert (x.varray_left_angle, todo) in
        convert offset todo results x.varray_keyword
    | { M.syntax = M.VectorArrayTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.vector_array_right_angle, todo) in
        let todo = Convert (x.vector_array_type, todo) in
        let todo = Convert (x.vector_array_left_angle, todo) in
        convert offset todo results x.vector_array_keyword
    | { M.syntax = M.TypeParameter x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.type_constraints, todo) in
        let todo = Convert (x.type_name, todo) in
        convert offset todo results x.type_variance
    | { M.syntax = M.TypeConstraint x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.constraint_type, todo) in
        convert offset todo results x.constraint_keyword
    | { M.syntax = M.DarrayTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.darray_right_angle, todo) in
        let todo = Convert (x.darray_trailing_comma, todo) in
        let todo = Convert (x.darray_value, todo) in
        let todo = Convert (x.darray_comma, todo) in
        let todo = Convert (x.darray_key, todo) in
        let todo = Convert (x.darray_left_angle, todo) in
        convert offset todo results x.darray_keyword
    | { M.syntax = M.MapArrayTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.map_array_right_angle, todo) in
        let todo = Convert (x.map_array_value, todo) in
        let todo = Convert (x.map_array_comma, todo) in
        let todo = Convert (x.map_array_key, todo) in
        let todo = Convert (x.map_array_left_angle, todo) in
        convert offset todo results x.map_array_keyword
    | { M.syntax = M.DictionaryTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.dictionary_type_right_angle, todo) in
        let todo = Convert (x.dictionary_type_members, todo) in
        let todo = Convert (x.dictionary_type_left_angle, todo) in
        convert offset todo results x.dictionary_type_keyword
    | { M.syntax = M.ClosureTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.closure_outer_right_paren, todo) in
        let todo = Convert (x.closure_return_type, todo) in
        let todo = Convert (x.closure_colon, todo) in
        let todo = Convert (x.closure_inner_right_paren, todo) in
        let todo = Convert (x.closure_parameter_list, todo) in
        let todo = Convert (x.closure_inner_left_paren, todo) in
        let todo = Convert (x.closure_function_keyword, todo) in
        let todo = Convert (x.closure_coroutine, todo) in
        convert offset todo results x.closure_outer_left_paren
    | { M.syntax = M.ClosureParameterTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.closure_parameter_type, todo) in
        convert offset todo results x.closure_parameter_call_convention
    | { M.syntax = M.ClassnameTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.classname_right_angle, todo) in
        let todo = Convert (x.classname_trailing_comma, todo) in
        let todo = Convert (x.classname_type, todo) in
        let todo = Convert (x.classname_left_angle, todo) in
        convert offset todo results x.classname_keyword
    | { M.syntax = M.FieldSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.field_type, todo) in
        let todo = Convert (x.field_arrow, todo) in
        let todo = Convert (x.field_name, todo) in
        convert offset todo results x.field_question
    | { M.syntax = M.FieldInitializer x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.field_initializer_value, todo) in
        let todo = Convert (x.field_initializer_arrow, todo) in
        convert offset todo results x.field_initializer_name
    | { M.syntax = M.ShapeTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.shape_type_right_paren, todo) in
        let todo = Convert (x.shape_type_ellipsis, todo) in
        let todo = Convert (x.shape_type_fields, todo) in
        let todo = Convert (x.shape_type_left_paren, todo) in
        convert offset todo results x.shape_type_keyword
    | { M.syntax = M.ShapeExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.shape_expression_right_paren, todo) in
        let todo = Convert (x.shape_expression_fields, todo) in
        let todo = Convert (x.shape_expression_left_paren, todo) in
        convert offset todo results x.shape_expression_keyword
    | { M.syntax = M.TupleExpression x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.tuple_expression_right_paren, todo) in
        let todo = Convert (x.tuple_expression_items, todo) in
        let todo = Convert (x.tuple_expression_left_paren, todo) in
        convert offset todo results x.tuple_expression_keyword
    | { M.syntax = M.GenericTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.generic_argument_list, todo) in
        convert offset todo results x.generic_class_type
    | { M.syntax = M.NullableTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.nullable_type, todo) in
        convert offset todo results x.nullable_question
    | { M.syntax = M.SoftTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.soft_type, todo) in
        convert offset todo results x.soft_at
    | { M.syntax = M.TypeArguments x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.type_arguments_right_angle, todo) in
        let todo = Convert (x.type_arguments_types, todo) in
        convert offset todo results x.type_arguments_left_angle
    | { M.syntax = M.TypeParameters x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.type_parameters_right_angle, todo) in
        let todo = Convert (x.type_parameters_parameters, todo) in
        convert offset todo results x.type_parameters_left_angle
    | { M.syntax = M.TupleTypeSpecifier x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.tuple_right_paren, todo) in
        let todo = Convert (x.tuple_types, todo) in
        convert offset todo results x.tuple_left_paren
    | { M.syntax = M.ErrorSyntax x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results x.error_error
    | { M.syntax = M.ListItem x
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (x.list_separator, todo) in
        convert offset todo results x.list_item
    in
    convert 0 Done [] node
end

let from_minimal = FromMinimal.from_minimal

let from_tree tree =
  from_minimal (SyntaxTree.text tree) (SyntaxTree.root tree)

