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
 * This module contains the type describing the structure of a syntax tree.
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
module SourceText = Full_fidelity_source_text
module PositionedToken = Full_fidelity_positioned_token

module SyntaxWithPositionedToken =
  Full_fidelity_syntax.WithToken(PositionedToken)

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
end

open Core
include SyntaxWithPositionedToken.WithSyntaxValue(PositionedSyntaxValue)
module Validated =
  Full_fidelity_validated_syntax.Make(PositionedToken)(PositionedSyntaxValue)

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
      | SyntaxKind.QualifiedNameExpression
      , (  qualified_name_expression
        :: results
        ) ->
          QualifiedNameExpression
          { qualified_name_expression
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
        :: function_coroutine
        :: function_async
        :: results
        ) ->
          FunctionDeclarationHeader
          { function_async
          ; function_coroutine
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
        :: methodish_modifiers
        :: methodish_attribute
        :: results
        ) ->
          MethodishDeclaration
          { methodish_attribute
          ; methodish_modifiers
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
        :: trait_use_alias_item_visibility
        :: trait_use_alias_item_keyword
        :: trait_use_alias_item_aliasing_name
        :: results
        ) ->
          TraitUseAliasItem
          { trait_use_alias_item_aliasing_name
          ; trait_use_alias_item_keyword
          ; trait_use_alias_item_visibility
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
        :: parameter_visibility
        :: parameter_attribute
        :: results
        ) ->
          ParameterDeclaration
          { parameter_attribute
          ; parameter_visibility
          ; parameter_type
          ; parameter_name
          ; parameter_default_value
          }, results
      | SyntaxKind.VariadicParameter
      , (  variadic_parameter_ellipsis
        :: results
        ) ->
          VariadicParameter
          { variadic_parameter_ellipsis
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
      , (  object_creation_right_paren
        :: object_creation_argument_list
        :: object_creation_left_paren
        :: object_creation_type
        :: object_creation_new_keyword
        :: results
        ) ->
          ObjectCreationExpression
          { object_creation_new_keyword
          ; object_creation_type
          ; object_creation_left_paren
          ; object_creation_argument_list
          ; object_creation_right_paren
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
        :: results
        ) ->
          XHPEnumType
          { xhp_enum_keyword
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
      | SyntaxKind.XHPAttribute
      , (  xhp_attribute_expression
        :: xhp_attribute_equal
        :: xhp_attribute_name
        :: results
        ) ->
          XHPAttribute
          { xhp_attribute_name
          ; xhp_attribute_equal
          ; xhp_attribute_expression
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
        :: closure_parameter_types
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
          ; closure_parameter_types
          ; closure_inner_right_paren
          ; closure_colon
          ; closure_return_type
          ; closure_outer_right_paren
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
      let token = PositionedToken.from_minimal source_text token offset in
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
    | { M.syntax = M.EndOfFile
        { M.end_of_file_token
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results end_of_file_token
    | { M.syntax = M.Script
        { M.script_declarations
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results script_declarations
    | { M.syntax = M.SimpleTypeSpecifier
        { M.simple_type_specifier
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results simple_type_specifier
    | { M.syntax = M.LiteralExpression
        { M.literal_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results literal_expression
    | { M.syntax = M.VariableExpression
        { M.variable_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results variable_expression
    | { M.syntax = M.QualifiedNameExpression
        { M.qualified_name_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results qualified_name_expression
    | { M.syntax = M.PipeVariableExpression
        { M.pipe_variable_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results pipe_variable_expression
    | { M.syntax = M.EnumDeclaration
        { M.enum_attribute_spec
        ; M.enum_keyword
        ; M.enum_name
        ; M.enum_colon
        ; M.enum_base
        ; M.enum_type
        ; M.enum_left_brace
        ; M.enum_enumerators
        ; M.enum_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (enum_right_brace, todo) in
        let todo = Convert (enum_enumerators, todo) in
        let todo = Convert (enum_left_brace, todo) in
        let todo = Convert (enum_type, todo) in
        let todo = Convert (enum_base, todo) in
        let todo = Convert (enum_colon, todo) in
        let todo = Convert (enum_name, todo) in
        let todo = Convert (enum_keyword, todo) in
        convert offset todo results enum_attribute_spec
    | { M.syntax = M.Enumerator
        { M.enumerator_name
        ; M.enumerator_equal
        ; M.enumerator_value
        ; M.enumerator_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (enumerator_semicolon, todo) in
        let todo = Convert (enumerator_value, todo) in
        let todo = Convert (enumerator_equal, todo) in
        convert offset todo results enumerator_name
    | { M.syntax = M.AliasDeclaration
        { M.alias_attribute_spec
        ; M.alias_keyword
        ; M.alias_name
        ; M.alias_generic_parameter
        ; M.alias_constraint
        ; M.alias_equal
        ; M.alias_type
        ; M.alias_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (alias_semicolon, todo) in
        let todo = Convert (alias_type, todo) in
        let todo = Convert (alias_equal, todo) in
        let todo = Convert (alias_constraint, todo) in
        let todo = Convert (alias_generic_parameter, todo) in
        let todo = Convert (alias_name, todo) in
        let todo = Convert (alias_keyword, todo) in
        convert offset todo results alias_attribute_spec
    | { M.syntax = M.PropertyDeclaration
        { M.property_modifiers
        ; M.property_type
        ; M.property_declarators
        ; M.property_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (property_semicolon, todo) in
        let todo = Convert (property_declarators, todo) in
        let todo = Convert (property_type, todo) in
        convert offset todo results property_modifiers
    | { M.syntax = M.PropertyDeclarator
        { M.property_name
        ; M.property_initializer
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (property_initializer, todo) in
        convert offset todo results property_name
    | { M.syntax = M.NamespaceDeclaration
        { M.namespace_keyword
        ; M.namespace_name
        ; M.namespace_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (namespace_body, todo) in
        let todo = Convert (namespace_name, todo) in
        convert offset todo results namespace_keyword
    | { M.syntax = M.NamespaceBody
        { M.namespace_left_brace
        ; M.namespace_declarations
        ; M.namespace_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (namespace_right_brace, todo) in
        let todo = Convert (namespace_declarations, todo) in
        convert offset todo results namespace_left_brace
    | { M.syntax = M.NamespaceEmptyBody
        { M.namespace_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results namespace_semicolon
    | { M.syntax = M.NamespaceUseDeclaration
        { M.namespace_use_keyword
        ; M.namespace_use_kind
        ; M.namespace_use_clauses
        ; M.namespace_use_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (namespace_use_semicolon, todo) in
        let todo = Convert (namespace_use_clauses, todo) in
        let todo = Convert (namespace_use_kind, todo) in
        convert offset todo results namespace_use_keyword
    | { M.syntax = M.NamespaceGroupUseDeclaration
        { M.namespace_group_use_keyword
        ; M.namespace_group_use_kind
        ; M.namespace_group_use_prefix
        ; M.namespace_group_use_left_brace
        ; M.namespace_group_use_clauses
        ; M.namespace_group_use_right_brace
        ; M.namespace_group_use_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (namespace_group_use_semicolon, todo) in
        let todo = Convert (namespace_group_use_right_brace, todo) in
        let todo = Convert (namespace_group_use_clauses, todo) in
        let todo = Convert (namespace_group_use_left_brace, todo) in
        let todo = Convert (namespace_group_use_prefix, todo) in
        let todo = Convert (namespace_group_use_kind, todo) in
        convert offset todo results namespace_group_use_keyword
    | { M.syntax = M.NamespaceUseClause
        { M.namespace_use_clause_kind
        ; M.namespace_use_name
        ; M.namespace_use_as
        ; M.namespace_use_alias
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (namespace_use_alias, todo) in
        let todo = Convert (namespace_use_as, todo) in
        let todo = Convert (namespace_use_name, todo) in
        convert offset todo results namespace_use_clause_kind
    | { M.syntax = M.FunctionDeclaration
        { M.function_attribute_spec
        ; M.function_declaration_header
        ; M.function_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (function_body, todo) in
        let todo = Convert (function_declaration_header, todo) in
        convert offset todo results function_attribute_spec
    | { M.syntax = M.FunctionDeclarationHeader
        { M.function_async
        ; M.function_coroutine
        ; M.function_keyword
        ; M.function_ampersand
        ; M.function_name
        ; M.function_type_parameter_list
        ; M.function_left_paren
        ; M.function_parameter_list
        ; M.function_right_paren
        ; M.function_colon
        ; M.function_type
        ; M.function_where_clause
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (function_where_clause, todo) in
        let todo = Convert (function_type, todo) in
        let todo = Convert (function_colon, todo) in
        let todo = Convert (function_right_paren, todo) in
        let todo = Convert (function_parameter_list, todo) in
        let todo = Convert (function_left_paren, todo) in
        let todo = Convert (function_type_parameter_list, todo) in
        let todo = Convert (function_name, todo) in
        let todo = Convert (function_ampersand, todo) in
        let todo = Convert (function_keyword, todo) in
        let todo = Convert (function_coroutine, todo) in
        convert offset todo results function_async
    | { M.syntax = M.WhereClause
        { M.where_clause_keyword
        ; M.where_clause_constraints
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (where_clause_constraints, todo) in
        convert offset todo results where_clause_keyword
    | { M.syntax = M.WhereConstraint
        { M.where_constraint_left_type
        ; M.where_constraint_operator
        ; M.where_constraint_right_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (where_constraint_right_type, todo) in
        let todo = Convert (where_constraint_operator, todo) in
        convert offset todo results where_constraint_left_type
    | { M.syntax = M.MethodishDeclaration
        { M.methodish_attribute
        ; M.methodish_modifiers
        ; M.methodish_function_decl_header
        ; M.methodish_function_body
        ; M.methodish_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (methodish_semicolon, todo) in
        let todo = Convert (methodish_function_body, todo) in
        let todo = Convert (methodish_function_decl_header, todo) in
        let todo = Convert (methodish_modifiers, todo) in
        convert offset todo results methodish_attribute
    | { M.syntax = M.ClassishDeclaration
        { M.classish_attribute
        ; M.classish_modifiers
        ; M.classish_keyword
        ; M.classish_name
        ; M.classish_type_parameters
        ; M.classish_extends_keyword
        ; M.classish_extends_list
        ; M.classish_implements_keyword
        ; M.classish_implements_list
        ; M.classish_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (classish_body, todo) in
        let todo = Convert (classish_implements_list, todo) in
        let todo = Convert (classish_implements_keyword, todo) in
        let todo = Convert (classish_extends_list, todo) in
        let todo = Convert (classish_extends_keyword, todo) in
        let todo = Convert (classish_type_parameters, todo) in
        let todo = Convert (classish_name, todo) in
        let todo = Convert (classish_keyword, todo) in
        let todo = Convert (classish_modifiers, todo) in
        convert offset todo results classish_attribute
    | { M.syntax = M.ClassishBody
        { M.classish_body_left_brace
        ; M.classish_body_elements
        ; M.classish_body_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (classish_body_right_brace, todo) in
        let todo = Convert (classish_body_elements, todo) in
        convert offset todo results classish_body_left_brace
    | { M.syntax = M.TraitUsePrecedenceItem
        { M.trait_use_precedence_item_name
        ; M.trait_use_precedence_item_keyword
        ; M.trait_use_precedence_item_removed_names
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (trait_use_precedence_item_removed_names, todo) in
        let todo = Convert (trait_use_precedence_item_keyword, todo) in
        convert offset todo results trait_use_precedence_item_name
    | { M.syntax = M.TraitUseAliasItem
        { M.trait_use_alias_item_aliasing_name
        ; M.trait_use_alias_item_keyword
        ; M.trait_use_alias_item_visibility
        ; M.trait_use_alias_item_aliased_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (trait_use_alias_item_aliased_name, todo) in
        let todo = Convert (trait_use_alias_item_visibility, todo) in
        let todo = Convert (trait_use_alias_item_keyword, todo) in
        convert offset todo results trait_use_alias_item_aliasing_name
    | { M.syntax = M.TraitUseConflictResolution
        { M.trait_use_conflict_resolution_keyword
        ; M.trait_use_conflict_resolution_names
        ; M.trait_use_conflict_resolution_left_brace
        ; M.trait_use_conflict_resolution_clauses
        ; M.trait_use_conflict_resolution_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (trait_use_conflict_resolution_right_brace, todo) in
        let todo = Convert (trait_use_conflict_resolution_clauses, todo) in
        let todo = Convert (trait_use_conflict_resolution_left_brace, todo) in
        let todo = Convert (trait_use_conflict_resolution_names, todo) in
        convert offset todo results trait_use_conflict_resolution_keyword
    | { M.syntax = M.TraitUse
        { M.trait_use_keyword
        ; M.trait_use_names
        ; M.trait_use_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (trait_use_semicolon, todo) in
        let todo = Convert (trait_use_names, todo) in
        convert offset todo results trait_use_keyword
    | { M.syntax = M.RequireClause
        { M.require_keyword
        ; M.require_kind
        ; M.require_name
        ; M.require_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (require_semicolon, todo) in
        let todo = Convert (require_name, todo) in
        let todo = Convert (require_kind, todo) in
        convert offset todo results require_keyword
    | { M.syntax = M.ConstDeclaration
        { M.const_abstract
        ; M.const_keyword
        ; M.const_type_specifier
        ; M.const_declarators
        ; M.const_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (const_semicolon, todo) in
        let todo = Convert (const_declarators, todo) in
        let todo = Convert (const_type_specifier, todo) in
        let todo = Convert (const_keyword, todo) in
        convert offset todo results const_abstract
    | { M.syntax = M.ConstantDeclarator
        { M.constant_declarator_name
        ; M.constant_declarator_initializer
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (constant_declarator_initializer, todo) in
        convert offset todo results constant_declarator_name
    | { M.syntax = M.TypeConstDeclaration
        { M.type_const_abstract
        ; M.type_const_keyword
        ; M.type_const_type_keyword
        ; M.type_const_name
        ; M.type_const_type_constraint
        ; M.type_const_equal
        ; M.type_const_type_specifier
        ; M.type_const_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (type_const_semicolon, todo) in
        let todo = Convert (type_const_type_specifier, todo) in
        let todo = Convert (type_const_equal, todo) in
        let todo = Convert (type_const_type_constraint, todo) in
        let todo = Convert (type_const_name, todo) in
        let todo = Convert (type_const_type_keyword, todo) in
        let todo = Convert (type_const_keyword, todo) in
        convert offset todo results type_const_abstract
    | { M.syntax = M.DecoratedExpression
        { M.decorated_expression_decorator
        ; M.decorated_expression_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (decorated_expression_expression, todo) in
        convert offset todo results decorated_expression_decorator
    | { M.syntax = M.ParameterDeclaration
        { M.parameter_attribute
        ; M.parameter_visibility
        ; M.parameter_type
        ; M.parameter_name
        ; M.parameter_default_value
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (parameter_default_value, todo) in
        let todo = Convert (parameter_name, todo) in
        let todo = Convert (parameter_type, todo) in
        let todo = Convert (parameter_visibility, todo) in
        convert offset todo results parameter_attribute
    | { M.syntax = M.VariadicParameter
        { M.variadic_parameter_ellipsis
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results variadic_parameter_ellipsis
    | { M.syntax = M.AttributeSpecification
        { M.attribute_specification_left_double_angle
        ; M.attribute_specification_attributes
        ; M.attribute_specification_right_double_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (attribute_specification_right_double_angle, todo) in
        let todo = Convert (attribute_specification_attributes, todo) in
        convert offset todo results attribute_specification_left_double_angle
    | { M.syntax = M.Attribute
        { M.attribute_name
        ; M.attribute_left_paren
        ; M.attribute_values
        ; M.attribute_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (attribute_right_paren, todo) in
        let todo = Convert (attribute_values, todo) in
        let todo = Convert (attribute_left_paren, todo) in
        convert offset todo results attribute_name
    | { M.syntax = M.InclusionExpression
        { M.inclusion_require
        ; M.inclusion_filename
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (inclusion_filename, todo) in
        convert offset todo results inclusion_require
    | { M.syntax = M.InclusionDirective
        { M.inclusion_expression
        ; M.inclusion_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (inclusion_semicolon, todo) in
        convert offset todo results inclusion_expression
    | { M.syntax = M.CompoundStatement
        { M.compound_left_brace
        ; M.compound_statements
        ; M.compound_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (compound_right_brace, todo) in
        let todo = Convert (compound_statements, todo) in
        convert offset todo results compound_left_brace
    | { M.syntax = M.ExpressionStatement
        { M.expression_statement_expression
        ; M.expression_statement_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (expression_statement_semicolon, todo) in
        convert offset todo results expression_statement_expression
    | { M.syntax = M.MarkupSection
        { M.markup_prefix
        ; M.markup_text
        ; M.markup_suffix
        ; M.markup_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (markup_expression, todo) in
        let todo = Convert (markup_suffix, todo) in
        let todo = Convert (markup_text, todo) in
        convert offset todo results markup_prefix
    | { M.syntax = M.MarkupSuffix
        { M.markup_suffix_less_than_question
        ; M.markup_suffix_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (markup_suffix_name, todo) in
        convert offset todo results markup_suffix_less_than_question
    | { M.syntax = M.UnsetStatement
        { M.unset_keyword
        ; M.unset_left_paren
        ; M.unset_variables
        ; M.unset_right_paren
        ; M.unset_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (unset_semicolon, todo) in
        let todo = Convert (unset_right_paren, todo) in
        let todo = Convert (unset_variables, todo) in
        let todo = Convert (unset_left_paren, todo) in
        convert offset todo results unset_keyword
    | { M.syntax = M.WhileStatement
        { M.while_keyword
        ; M.while_left_paren
        ; M.while_condition
        ; M.while_right_paren
        ; M.while_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (while_body, todo) in
        let todo = Convert (while_right_paren, todo) in
        let todo = Convert (while_condition, todo) in
        let todo = Convert (while_left_paren, todo) in
        convert offset todo results while_keyword
    | { M.syntax = M.IfStatement
        { M.if_keyword
        ; M.if_left_paren
        ; M.if_condition
        ; M.if_right_paren
        ; M.if_statement
        ; M.if_elseif_clauses
        ; M.if_else_clause
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (if_else_clause, todo) in
        let todo = Convert (if_elseif_clauses, todo) in
        let todo = Convert (if_statement, todo) in
        let todo = Convert (if_right_paren, todo) in
        let todo = Convert (if_condition, todo) in
        let todo = Convert (if_left_paren, todo) in
        convert offset todo results if_keyword
    | { M.syntax = M.ElseifClause
        { M.elseif_keyword
        ; M.elseif_left_paren
        ; M.elseif_condition
        ; M.elseif_right_paren
        ; M.elseif_statement
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (elseif_statement, todo) in
        let todo = Convert (elseif_right_paren, todo) in
        let todo = Convert (elseif_condition, todo) in
        let todo = Convert (elseif_left_paren, todo) in
        convert offset todo results elseif_keyword
    | { M.syntax = M.ElseClause
        { M.else_keyword
        ; M.else_statement
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (else_statement, todo) in
        convert offset todo results else_keyword
    | { M.syntax = M.TryStatement
        { M.try_keyword
        ; M.try_compound_statement
        ; M.try_catch_clauses
        ; M.try_finally_clause
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (try_finally_clause, todo) in
        let todo = Convert (try_catch_clauses, todo) in
        let todo = Convert (try_compound_statement, todo) in
        convert offset todo results try_keyword
    | { M.syntax = M.CatchClause
        { M.catch_keyword
        ; M.catch_left_paren
        ; M.catch_type
        ; M.catch_variable
        ; M.catch_right_paren
        ; M.catch_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (catch_body, todo) in
        let todo = Convert (catch_right_paren, todo) in
        let todo = Convert (catch_variable, todo) in
        let todo = Convert (catch_type, todo) in
        let todo = Convert (catch_left_paren, todo) in
        convert offset todo results catch_keyword
    | { M.syntax = M.FinallyClause
        { M.finally_keyword
        ; M.finally_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (finally_body, todo) in
        convert offset todo results finally_keyword
    | { M.syntax = M.DoStatement
        { M.do_keyword
        ; M.do_body
        ; M.do_while_keyword
        ; M.do_left_paren
        ; M.do_condition
        ; M.do_right_paren
        ; M.do_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (do_semicolon, todo) in
        let todo = Convert (do_right_paren, todo) in
        let todo = Convert (do_condition, todo) in
        let todo = Convert (do_left_paren, todo) in
        let todo = Convert (do_while_keyword, todo) in
        let todo = Convert (do_body, todo) in
        convert offset todo results do_keyword
    | { M.syntax = M.ForStatement
        { M.for_keyword
        ; M.for_left_paren
        ; M.for_initializer
        ; M.for_first_semicolon
        ; M.for_control
        ; M.for_second_semicolon
        ; M.for_end_of_loop
        ; M.for_right_paren
        ; M.for_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (for_body, todo) in
        let todo = Convert (for_right_paren, todo) in
        let todo = Convert (for_end_of_loop, todo) in
        let todo = Convert (for_second_semicolon, todo) in
        let todo = Convert (for_control, todo) in
        let todo = Convert (for_first_semicolon, todo) in
        let todo = Convert (for_initializer, todo) in
        let todo = Convert (for_left_paren, todo) in
        convert offset todo results for_keyword
    | { M.syntax = M.ForeachStatement
        { M.foreach_keyword
        ; M.foreach_left_paren
        ; M.foreach_collection
        ; M.foreach_await_keyword
        ; M.foreach_as
        ; M.foreach_key
        ; M.foreach_arrow
        ; M.foreach_value
        ; M.foreach_right_paren
        ; M.foreach_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (foreach_body, todo) in
        let todo = Convert (foreach_right_paren, todo) in
        let todo = Convert (foreach_value, todo) in
        let todo = Convert (foreach_arrow, todo) in
        let todo = Convert (foreach_key, todo) in
        let todo = Convert (foreach_as, todo) in
        let todo = Convert (foreach_await_keyword, todo) in
        let todo = Convert (foreach_collection, todo) in
        let todo = Convert (foreach_left_paren, todo) in
        convert offset todo results foreach_keyword
    | { M.syntax = M.SwitchStatement
        { M.switch_keyword
        ; M.switch_left_paren
        ; M.switch_expression
        ; M.switch_right_paren
        ; M.switch_left_brace
        ; M.switch_sections
        ; M.switch_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (switch_right_brace, todo) in
        let todo = Convert (switch_sections, todo) in
        let todo = Convert (switch_left_brace, todo) in
        let todo = Convert (switch_right_paren, todo) in
        let todo = Convert (switch_expression, todo) in
        let todo = Convert (switch_left_paren, todo) in
        convert offset todo results switch_keyword
    | { M.syntax = M.SwitchSection
        { M.switch_section_labels
        ; M.switch_section_statements
        ; M.switch_section_fallthrough
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (switch_section_fallthrough, todo) in
        let todo = Convert (switch_section_statements, todo) in
        convert offset todo results switch_section_labels
    | { M.syntax = M.SwitchFallthrough
        { M.fallthrough_keyword
        ; M.fallthrough_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (fallthrough_semicolon, todo) in
        convert offset todo results fallthrough_keyword
    | { M.syntax = M.CaseLabel
        { M.case_keyword
        ; M.case_expression
        ; M.case_colon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (case_colon, todo) in
        let todo = Convert (case_expression, todo) in
        convert offset todo results case_keyword
    | { M.syntax = M.DefaultLabel
        { M.default_keyword
        ; M.default_colon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (default_colon, todo) in
        convert offset todo results default_keyword
    | { M.syntax = M.ReturnStatement
        { M.return_keyword
        ; M.return_expression
        ; M.return_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (return_semicolon, todo) in
        let todo = Convert (return_expression, todo) in
        convert offset todo results return_keyword
    | { M.syntax = M.GotoLabel
        { M.goto_label_name
        ; M.goto_label_colon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (goto_label_colon, todo) in
        convert offset todo results goto_label_name
    | { M.syntax = M.GotoStatement
        { M.goto_statement_keyword
        ; M.goto_statement_label_name
        ; M.goto_statement_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (goto_statement_semicolon, todo) in
        let todo = Convert (goto_statement_label_name, todo) in
        convert offset todo results goto_statement_keyword
    | { M.syntax = M.ThrowStatement
        { M.throw_keyword
        ; M.throw_expression
        ; M.throw_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (throw_semicolon, todo) in
        let todo = Convert (throw_expression, todo) in
        convert offset todo results throw_keyword
    | { M.syntax = M.BreakStatement
        { M.break_keyword
        ; M.break_level
        ; M.break_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (break_semicolon, todo) in
        let todo = Convert (break_level, todo) in
        convert offset todo results break_keyword
    | { M.syntax = M.ContinueStatement
        { M.continue_keyword
        ; M.continue_level
        ; M.continue_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (continue_semicolon, todo) in
        let todo = Convert (continue_level, todo) in
        convert offset todo results continue_keyword
    | { M.syntax = M.FunctionStaticStatement
        { M.static_static_keyword
        ; M.static_declarations
        ; M.static_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (static_semicolon, todo) in
        let todo = Convert (static_declarations, todo) in
        convert offset todo results static_static_keyword
    | { M.syntax = M.StaticDeclarator
        { M.static_name
        ; M.static_initializer
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (static_initializer, todo) in
        convert offset todo results static_name
    | { M.syntax = M.EchoStatement
        { M.echo_keyword
        ; M.echo_expressions
        ; M.echo_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (echo_semicolon, todo) in
        let todo = Convert (echo_expressions, todo) in
        convert offset todo results echo_keyword
    | { M.syntax = M.GlobalStatement
        { M.global_keyword
        ; M.global_variables
        ; M.global_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (global_semicolon, todo) in
        let todo = Convert (global_variables, todo) in
        convert offset todo results global_keyword
    | { M.syntax = M.SimpleInitializer
        { M.simple_initializer_equal
        ; M.simple_initializer_value
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (simple_initializer_value, todo) in
        convert offset todo results simple_initializer_equal
    | { M.syntax = M.AnonymousFunction
        { M.anonymous_static_keyword
        ; M.anonymous_async_keyword
        ; M.anonymous_coroutine_keyword
        ; M.anonymous_function_keyword
        ; M.anonymous_left_paren
        ; M.anonymous_parameters
        ; M.anonymous_right_paren
        ; M.anonymous_colon
        ; M.anonymous_type
        ; M.anonymous_use
        ; M.anonymous_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (anonymous_body, todo) in
        let todo = Convert (anonymous_use, todo) in
        let todo = Convert (anonymous_type, todo) in
        let todo = Convert (anonymous_colon, todo) in
        let todo = Convert (anonymous_right_paren, todo) in
        let todo = Convert (anonymous_parameters, todo) in
        let todo = Convert (anonymous_left_paren, todo) in
        let todo = Convert (anonymous_function_keyword, todo) in
        let todo = Convert (anonymous_coroutine_keyword, todo) in
        let todo = Convert (anonymous_async_keyword, todo) in
        convert offset todo results anonymous_static_keyword
    | { M.syntax = M.AnonymousFunctionUseClause
        { M.anonymous_use_keyword
        ; M.anonymous_use_left_paren
        ; M.anonymous_use_variables
        ; M.anonymous_use_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (anonymous_use_right_paren, todo) in
        let todo = Convert (anonymous_use_variables, todo) in
        let todo = Convert (anonymous_use_left_paren, todo) in
        convert offset todo results anonymous_use_keyword
    | { M.syntax = M.LambdaExpression
        { M.lambda_async
        ; M.lambda_coroutine
        ; M.lambda_signature
        ; M.lambda_arrow
        ; M.lambda_body
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (lambda_body, todo) in
        let todo = Convert (lambda_arrow, todo) in
        let todo = Convert (lambda_signature, todo) in
        let todo = Convert (lambda_coroutine, todo) in
        convert offset todo results lambda_async
    | { M.syntax = M.LambdaSignature
        { M.lambda_left_paren
        ; M.lambda_parameters
        ; M.lambda_right_paren
        ; M.lambda_colon
        ; M.lambda_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (lambda_type, todo) in
        let todo = Convert (lambda_colon, todo) in
        let todo = Convert (lambda_right_paren, todo) in
        let todo = Convert (lambda_parameters, todo) in
        convert offset todo results lambda_left_paren
    | { M.syntax = M.CastExpression
        { M.cast_left_paren
        ; M.cast_type
        ; M.cast_right_paren
        ; M.cast_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (cast_operand, todo) in
        let todo = Convert (cast_right_paren, todo) in
        let todo = Convert (cast_type, todo) in
        convert offset todo results cast_left_paren
    | { M.syntax = M.ScopeResolutionExpression
        { M.scope_resolution_qualifier
        ; M.scope_resolution_operator
        ; M.scope_resolution_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (scope_resolution_name, todo) in
        let todo = Convert (scope_resolution_operator, todo) in
        convert offset todo results scope_resolution_qualifier
    | { M.syntax = M.MemberSelectionExpression
        { M.member_object
        ; M.member_operator
        ; M.member_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (member_name, todo) in
        let todo = Convert (member_operator, todo) in
        convert offset todo results member_object
    | { M.syntax = M.SafeMemberSelectionExpression
        { M.safe_member_object
        ; M.safe_member_operator
        ; M.safe_member_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (safe_member_name, todo) in
        let todo = Convert (safe_member_operator, todo) in
        convert offset todo results safe_member_object
    | { M.syntax = M.EmbeddedMemberSelectionExpression
        { M.embedded_member_object
        ; M.embedded_member_operator
        ; M.embedded_member_name
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (embedded_member_name, todo) in
        let todo = Convert (embedded_member_operator, todo) in
        convert offset todo results embedded_member_object
    | { M.syntax = M.YieldExpression
        { M.yield_keyword
        ; M.yield_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (yield_operand, todo) in
        convert offset todo results yield_keyword
    | { M.syntax = M.YieldFromExpression
        { M.yield_from_yield_keyword
        ; M.yield_from_from_keyword
        ; M.yield_from_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (yield_from_operand, todo) in
        let todo = Convert (yield_from_from_keyword, todo) in
        convert offset todo results yield_from_yield_keyword
    | { M.syntax = M.PrefixUnaryExpression
        { M.prefix_unary_operator
        ; M.prefix_unary_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (prefix_unary_operand, todo) in
        convert offset todo results prefix_unary_operator
    | { M.syntax = M.PostfixUnaryExpression
        { M.postfix_unary_operand
        ; M.postfix_unary_operator
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (postfix_unary_operator, todo) in
        convert offset todo results postfix_unary_operand
    | { M.syntax = M.BinaryExpression
        { M.binary_left_operand
        ; M.binary_operator
        ; M.binary_right_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (binary_right_operand, todo) in
        let todo = Convert (binary_operator, todo) in
        convert offset todo results binary_left_operand
    | { M.syntax = M.InstanceofExpression
        { M.instanceof_left_operand
        ; M.instanceof_operator
        ; M.instanceof_right_operand
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (instanceof_right_operand, todo) in
        let todo = Convert (instanceof_operator, todo) in
        convert offset todo results instanceof_left_operand
    | { M.syntax = M.ConditionalExpression
        { M.conditional_test
        ; M.conditional_question
        ; M.conditional_consequence
        ; M.conditional_colon
        ; M.conditional_alternative
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (conditional_alternative, todo) in
        let todo = Convert (conditional_colon, todo) in
        let todo = Convert (conditional_consequence, todo) in
        let todo = Convert (conditional_question, todo) in
        convert offset todo results conditional_test
    | { M.syntax = M.EvalExpression
        { M.eval_keyword
        ; M.eval_left_paren
        ; M.eval_argument
        ; M.eval_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (eval_right_paren, todo) in
        let todo = Convert (eval_argument, todo) in
        let todo = Convert (eval_left_paren, todo) in
        convert offset todo results eval_keyword
    | { M.syntax = M.EmptyExpression
        { M.empty_keyword
        ; M.empty_left_paren
        ; M.empty_argument
        ; M.empty_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (empty_right_paren, todo) in
        let todo = Convert (empty_argument, todo) in
        let todo = Convert (empty_left_paren, todo) in
        convert offset todo results empty_keyword
    | { M.syntax = M.DefineExpression
        { M.define_keyword
        ; M.define_left_paren
        ; M.define_argument_list
        ; M.define_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (define_right_paren, todo) in
        let todo = Convert (define_argument_list, todo) in
        let todo = Convert (define_left_paren, todo) in
        convert offset todo results define_keyword
    | { M.syntax = M.IssetExpression
        { M.isset_keyword
        ; M.isset_left_paren
        ; M.isset_argument_list
        ; M.isset_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (isset_right_paren, todo) in
        let todo = Convert (isset_argument_list, todo) in
        let todo = Convert (isset_left_paren, todo) in
        convert offset todo results isset_keyword
    | { M.syntax = M.FunctionCallExpression
        { M.function_call_receiver
        ; M.function_call_left_paren
        ; M.function_call_argument_list
        ; M.function_call_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (function_call_right_paren, todo) in
        let todo = Convert (function_call_argument_list, todo) in
        let todo = Convert (function_call_left_paren, todo) in
        convert offset todo results function_call_receiver
    | { M.syntax = M.ParenthesizedExpression
        { M.parenthesized_expression_left_paren
        ; M.parenthesized_expression_expression
        ; M.parenthesized_expression_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (parenthesized_expression_right_paren, todo) in
        let todo = Convert (parenthesized_expression_expression, todo) in
        convert offset todo results parenthesized_expression_left_paren
    | { M.syntax = M.BracedExpression
        { M.braced_expression_left_brace
        ; M.braced_expression_expression
        ; M.braced_expression_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (braced_expression_right_brace, todo) in
        let todo = Convert (braced_expression_expression, todo) in
        convert offset todo results braced_expression_left_brace
    | { M.syntax = M.EmbeddedBracedExpression
        { M.embedded_braced_expression_left_brace
        ; M.embedded_braced_expression_expression
        ; M.embedded_braced_expression_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (embedded_braced_expression_right_brace, todo) in
        let todo = Convert (embedded_braced_expression_expression, todo) in
        convert offset todo results embedded_braced_expression_left_brace
    | { M.syntax = M.ListExpression
        { M.list_keyword
        ; M.list_left_paren
        ; M.list_members
        ; M.list_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (list_right_paren, todo) in
        let todo = Convert (list_members, todo) in
        let todo = Convert (list_left_paren, todo) in
        convert offset todo results list_keyword
    | { M.syntax = M.CollectionLiteralExpression
        { M.collection_literal_name
        ; M.collection_literal_left_brace
        ; M.collection_literal_initializers
        ; M.collection_literal_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (collection_literal_right_brace, todo) in
        let todo = Convert (collection_literal_initializers, todo) in
        let todo = Convert (collection_literal_left_brace, todo) in
        convert offset todo results collection_literal_name
    | { M.syntax = M.ObjectCreationExpression
        { M.object_creation_new_keyword
        ; M.object_creation_type
        ; M.object_creation_left_paren
        ; M.object_creation_argument_list
        ; M.object_creation_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (object_creation_right_paren, todo) in
        let todo = Convert (object_creation_argument_list, todo) in
        let todo = Convert (object_creation_left_paren, todo) in
        let todo = Convert (object_creation_type, todo) in
        convert offset todo results object_creation_new_keyword
    | { M.syntax = M.ArrayCreationExpression
        { M.array_creation_left_bracket
        ; M.array_creation_members
        ; M.array_creation_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (array_creation_right_bracket, todo) in
        let todo = Convert (array_creation_members, todo) in
        convert offset todo results array_creation_left_bracket
    | { M.syntax = M.ArrayIntrinsicExpression
        { M.array_intrinsic_keyword
        ; M.array_intrinsic_left_paren
        ; M.array_intrinsic_members
        ; M.array_intrinsic_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (array_intrinsic_right_paren, todo) in
        let todo = Convert (array_intrinsic_members, todo) in
        let todo = Convert (array_intrinsic_left_paren, todo) in
        convert offset todo results array_intrinsic_keyword
    | { M.syntax = M.DarrayIntrinsicExpression
        { M.darray_intrinsic_keyword
        ; M.darray_intrinsic_left_bracket
        ; M.darray_intrinsic_members
        ; M.darray_intrinsic_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (darray_intrinsic_right_bracket, todo) in
        let todo = Convert (darray_intrinsic_members, todo) in
        let todo = Convert (darray_intrinsic_left_bracket, todo) in
        convert offset todo results darray_intrinsic_keyword
    | { M.syntax = M.DictionaryIntrinsicExpression
        { M.dictionary_intrinsic_keyword
        ; M.dictionary_intrinsic_left_bracket
        ; M.dictionary_intrinsic_members
        ; M.dictionary_intrinsic_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (dictionary_intrinsic_right_bracket, todo) in
        let todo = Convert (dictionary_intrinsic_members, todo) in
        let todo = Convert (dictionary_intrinsic_left_bracket, todo) in
        convert offset todo results dictionary_intrinsic_keyword
    | { M.syntax = M.KeysetIntrinsicExpression
        { M.keyset_intrinsic_keyword
        ; M.keyset_intrinsic_left_bracket
        ; M.keyset_intrinsic_members
        ; M.keyset_intrinsic_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (keyset_intrinsic_right_bracket, todo) in
        let todo = Convert (keyset_intrinsic_members, todo) in
        let todo = Convert (keyset_intrinsic_left_bracket, todo) in
        convert offset todo results keyset_intrinsic_keyword
    | { M.syntax = M.VarrayIntrinsicExpression
        { M.varray_intrinsic_keyword
        ; M.varray_intrinsic_left_bracket
        ; M.varray_intrinsic_members
        ; M.varray_intrinsic_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (varray_intrinsic_right_bracket, todo) in
        let todo = Convert (varray_intrinsic_members, todo) in
        let todo = Convert (varray_intrinsic_left_bracket, todo) in
        convert offset todo results varray_intrinsic_keyword
    | { M.syntax = M.VectorIntrinsicExpression
        { M.vector_intrinsic_keyword
        ; M.vector_intrinsic_left_bracket
        ; M.vector_intrinsic_members
        ; M.vector_intrinsic_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (vector_intrinsic_right_bracket, todo) in
        let todo = Convert (vector_intrinsic_members, todo) in
        let todo = Convert (vector_intrinsic_left_bracket, todo) in
        convert offset todo results vector_intrinsic_keyword
    | { M.syntax = M.ElementInitializer
        { M.element_key
        ; M.element_arrow
        ; M.element_value
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (element_value, todo) in
        let todo = Convert (element_arrow, todo) in
        convert offset todo results element_key
    | { M.syntax = M.SubscriptExpression
        { M.subscript_receiver
        ; M.subscript_left_bracket
        ; M.subscript_index
        ; M.subscript_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (subscript_right_bracket, todo) in
        let todo = Convert (subscript_index, todo) in
        let todo = Convert (subscript_left_bracket, todo) in
        convert offset todo results subscript_receiver
    | { M.syntax = M.EmbeddedSubscriptExpression
        { M.embedded_subscript_receiver
        ; M.embedded_subscript_left_bracket
        ; M.embedded_subscript_index
        ; M.embedded_subscript_right_bracket
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (embedded_subscript_right_bracket, todo) in
        let todo = Convert (embedded_subscript_index, todo) in
        let todo = Convert (embedded_subscript_left_bracket, todo) in
        convert offset todo results embedded_subscript_receiver
    | { M.syntax = M.AwaitableCreationExpression
        { M.awaitable_async
        ; M.awaitable_coroutine
        ; M.awaitable_compound_statement
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (awaitable_compound_statement, todo) in
        let todo = Convert (awaitable_coroutine, todo) in
        convert offset todo results awaitable_async
    | { M.syntax = M.XHPChildrenDeclaration
        { M.xhp_children_keyword
        ; M.xhp_children_expression
        ; M.xhp_children_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_children_semicolon, todo) in
        let todo = Convert (xhp_children_expression, todo) in
        convert offset todo results xhp_children_keyword
    | { M.syntax = M.XHPChildrenParenthesizedList
        { M.xhp_children_list_left_paren
        ; M.xhp_children_list_xhp_children
        ; M.xhp_children_list_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_children_list_right_paren, todo) in
        let todo = Convert (xhp_children_list_xhp_children, todo) in
        convert offset todo results xhp_children_list_left_paren
    | { M.syntax = M.XHPCategoryDeclaration
        { M.xhp_category_keyword
        ; M.xhp_category_categories
        ; M.xhp_category_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_category_semicolon, todo) in
        let todo = Convert (xhp_category_categories, todo) in
        convert offset todo results xhp_category_keyword
    | { M.syntax = M.XHPEnumType
        { M.xhp_enum_keyword
        ; M.xhp_enum_left_brace
        ; M.xhp_enum_values
        ; M.xhp_enum_right_brace
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_enum_right_brace, todo) in
        let todo = Convert (xhp_enum_values, todo) in
        let todo = Convert (xhp_enum_left_brace, todo) in
        convert offset todo results xhp_enum_keyword
    | { M.syntax = M.XHPRequired
        { M.xhp_required_at
        ; M.xhp_required_keyword
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_required_keyword, todo) in
        convert offset todo results xhp_required_at
    | { M.syntax = M.XHPClassAttributeDeclaration
        { M.xhp_attribute_keyword
        ; M.xhp_attribute_attributes
        ; M.xhp_attribute_semicolon
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_attribute_semicolon, todo) in
        let todo = Convert (xhp_attribute_attributes, todo) in
        convert offset todo results xhp_attribute_keyword
    | { M.syntax = M.XHPClassAttribute
        { M.xhp_attribute_decl_type
        ; M.xhp_attribute_decl_name
        ; M.xhp_attribute_decl_initializer
        ; M.xhp_attribute_decl_required
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_attribute_decl_required, todo) in
        let todo = Convert (xhp_attribute_decl_initializer, todo) in
        let todo = Convert (xhp_attribute_decl_name, todo) in
        convert offset todo results xhp_attribute_decl_type
    | { M.syntax = M.XHPSimpleClassAttribute
        { M.xhp_simple_class_attribute_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results xhp_simple_class_attribute_type
    | { M.syntax = M.XHPAttribute
        { M.xhp_attribute_name
        ; M.xhp_attribute_equal
        ; M.xhp_attribute_expression
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_attribute_expression, todo) in
        let todo = Convert (xhp_attribute_equal, todo) in
        convert offset todo results xhp_attribute_name
    | { M.syntax = M.XHPOpen
        { M.xhp_open_left_angle
        ; M.xhp_open_name
        ; M.xhp_open_attributes
        ; M.xhp_open_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_open_right_angle, todo) in
        let todo = Convert (xhp_open_attributes, todo) in
        let todo = Convert (xhp_open_name, todo) in
        convert offset todo results xhp_open_left_angle
    | { M.syntax = M.XHPExpression
        { M.xhp_open
        ; M.xhp_body
        ; M.xhp_close
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_close, todo) in
        let todo = Convert (xhp_body, todo) in
        convert offset todo results xhp_open
    | { M.syntax = M.XHPClose
        { M.xhp_close_left_angle
        ; M.xhp_close_name
        ; M.xhp_close_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (xhp_close_right_angle, todo) in
        let todo = Convert (xhp_close_name, todo) in
        convert offset todo results xhp_close_left_angle
    | { M.syntax = M.TypeConstant
        { M.type_constant_left_type
        ; M.type_constant_separator
        ; M.type_constant_right_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (type_constant_right_type, todo) in
        let todo = Convert (type_constant_separator, todo) in
        convert offset todo results type_constant_left_type
    | { M.syntax = M.VectorTypeSpecifier
        { M.vector_type_keyword
        ; M.vector_type_left_angle
        ; M.vector_type_type
        ; M.vector_type_trailing_comma
        ; M.vector_type_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (vector_type_right_angle, todo) in
        let todo = Convert (vector_type_trailing_comma, todo) in
        let todo = Convert (vector_type_type, todo) in
        let todo = Convert (vector_type_left_angle, todo) in
        convert offset todo results vector_type_keyword
    | { M.syntax = M.KeysetTypeSpecifier
        { M.keyset_type_keyword
        ; M.keyset_type_left_angle
        ; M.keyset_type_type
        ; M.keyset_type_trailing_comma
        ; M.keyset_type_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (keyset_type_right_angle, todo) in
        let todo = Convert (keyset_type_trailing_comma, todo) in
        let todo = Convert (keyset_type_type, todo) in
        let todo = Convert (keyset_type_left_angle, todo) in
        convert offset todo results keyset_type_keyword
    | { M.syntax = M.TupleTypeExplicitSpecifier
        { M.tuple_type_keyword
        ; M.tuple_type_left_angle
        ; M.tuple_type_types
        ; M.tuple_type_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (tuple_type_right_angle, todo) in
        let todo = Convert (tuple_type_types, todo) in
        let todo = Convert (tuple_type_left_angle, todo) in
        convert offset todo results tuple_type_keyword
    | { M.syntax = M.VarrayTypeSpecifier
        { M.varray_keyword
        ; M.varray_left_angle
        ; M.varray_type
        ; M.varray_trailing_comma
        ; M.varray_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (varray_right_angle, todo) in
        let todo = Convert (varray_trailing_comma, todo) in
        let todo = Convert (varray_type, todo) in
        let todo = Convert (varray_left_angle, todo) in
        convert offset todo results varray_keyword
    | { M.syntax = M.VectorArrayTypeSpecifier
        { M.vector_array_keyword
        ; M.vector_array_left_angle
        ; M.vector_array_type
        ; M.vector_array_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (vector_array_right_angle, todo) in
        let todo = Convert (vector_array_type, todo) in
        let todo = Convert (vector_array_left_angle, todo) in
        convert offset todo results vector_array_keyword
    | { M.syntax = M.TypeParameter
        { M.type_variance
        ; M.type_name
        ; M.type_constraints
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (type_constraints, todo) in
        let todo = Convert (type_name, todo) in
        convert offset todo results type_variance
    | { M.syntax = M.TypeConstraint
        { M.constraint_keyword
        ; M.constraint_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (constraint_type, todo) in
        convert offset todo results constraint_keyword
    | { M.syntax = M.DarrayTypeSpecifier
        { M.darray_keyword
        ; M.darray_left_angle
        ; M.darray_key
        ; M.darray_comma
        ; M.darray_value
        ; M.darray_trailing_comma
        ; M.darray_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (darray_right_angle, todo) in
        let todo = Convert (darray_trailing_comma, todo) in
        let todo = Convert (darray_value, todo) in
        let todo = Convert (darray_comma, todo) in
        let todo = Convert (darray_key, todo) in
        let todo = Convert (darray_left_angle, todo) in
        convert offset todo results darray_keyword
    | { M.syntax = M.MapArrayTypeSpecifier
        { M.map_array_keyword
        ; M.map_array_left_angle
        ; M.map_array_key
        ; M.map_array_comma
        ; M.map_array_value
        ; M.map_array_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (map_array_right_angle, todo) in
        let todo = Convert (map_array_value, todo) in
        let todo = Convert (map_array_comma, todo) in
        let todo = Convert (map_array_key, todo) in
        let todo = Convert (map_array_left_angle, todo) in
        convert offset todo results map_array_keyword
    | { M.syntax = M.DictionaryTypeSpecifier
        { M.dictionary_type_keyword
        ; M.dictionary_type_left_angle
        ; M.dictionary_type_members
        ; M.dictionary_type_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (dictionary_type_right_angle, todo) in
        let todo = Convert (dictionary_type_members, todo) in
        let todo = Convert (dictionary_type_left_angle, todo) in
        convert offset todo results dictionary_type_keyword
    | { M.syntax = M.ClosureTypeSpecifier
        { M.closure_outer_left_paren
        ; M.closure_coroutine
        ; M.closure_function_keyword
        ; M.closure_inner_left_paren
        ; M.closure_parameter_types
        ; M.closure_inner_right_paren
        ; M.closure_colon
        ; M.closure_return_type
        ; M.closure_outer_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (closure_outer_right_paren, todo) in
        let todo = Convert (closure_return_type, todo) in
        let todo = Convert (closure_colon, todo) in
        let todo = Convert (closure_inner_right_paren, todo) in
        let todo = Convert (closure_parameter_types, todo) in
        let todo = Convert (closure_inner_left_paren, todo) in
        let todo = Convert (closure_function_keyword, todo) in
        let todo = Convert (closure_coroutine, todo) in
        convert offset todo results closure_outer_left_paren
    | { M.syntax = M.ClassnameTypeSpecifier
        { M.classname_keyword
        ; M.classname_left_angle
        ; M.classname_type
        ; M.classname_trailing_comma
        ; M.classname_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (classname_right_angle, todo) in
        let todo = Convert (classname_trailing_comma, todo) in
        let todo = Convert (classname_type, todo) in
        let todo = Convert (classname_left_angle, todo) in
        convert offset todo results classname_keyword
    | { M.syntax = M.FieldSpecifier
        { M.field_question
        ; M.field_name
        ; M.field_arrow
        ; M.field_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (field_type, todo) in
        let todo = Convert (field_arrow, todo) in
        let todo = Convert (field_name, todo) in
        convert offset todo results field_question
    | { M.syntax = M.FieldInitializer
        { M.field_initializer_name
        ; M.field_initializer_arrow
        ; M.field_initializer_value
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (field_initializer_value, todo) in
        let todo = Convert (field_initializer_arrow, todo) in
        convert offset todo results field_initializer_name
    | { M.syntax = M.ShapeTypeSpecifier
        { M.shape_type_keyword
        ; M.shape_type_left_paren
        ; M.shape_type_fields
        ; M.shape_type_ellipsis
        ; M.shape_type_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (shape_type_right_paren, todo) in
        let todo = Convert (shape_type_ellipsis, todo) in
        let todo = Convert (shape_type_fields, todo) in
        let todo = Convert (shape_type_left_paren, todo) in
        convert offset todo results shape_type_keyword
    | { M.syntax = M.ShapeExpression
        { M.shape_expression_keyword
        ; M.shape_expression_left_paren
        ; M.shape_expression_fields
        ; M.shape_expression_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (shape_expression_right_paren, todo) in
        let todo = Convert (shape_expression_fields, todo) in
        let todo = Convert (shape_expression_left_paren, todo) in
        convert offset todo results shape_expression_keyword
    | { M.syntax = M.TupleExpression
        { M.tuple_expression_keyword
        ; M.tuple_expression_left_paren
        ; M.tuple_expression_items
        ; M.tuple_expression_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (tuple_expression_right_paren, todo) in
        let todo = Convert (tuple_expression_items, todo) in
        let todo = Convert (tuple_expression_left_paren, todo) in
        convert offset todo results tuple_expression_keyword
    | { M.syntax = M.GenericTypeSpecifier
        { M.generic_class_type
        ; M.generic_argument_list
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (generic_argument_list, todo) in
        convert offset todo results generic_class_type
    | { M.syntax = M.NullableTypeSpecifier
        { M.nullable_question
        ; M.nullable_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (nullable_type, todo) in
        convert offset todo results nullable_question
    | { M.syntax = M.SoftTypeSpecifier
        { M.soft_at
        ; M.soft_type
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (soft_type, todo) in
        convert offset todo results soft_at
    | { M.syntax = M.TypeArguments
        { M.type_arguments_left_angle
        ; M.type_arguments_types
        ; M.type_arguments_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (type_arguments_right_angle, todo) in
        let todo = Convert (type_arguments_types, todo) in
        convert offset todo results type_arguments_left_angle
    | { M.syntax = M.TypeParameters
        { M.type_parameters_left_angle
        ; M.type_parameters_parameters
        ; M.type_parameters_right_angle
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (type_parameters_right_angle, todo) in
        let todo = Convert (type_parameters_parameters, todo) in
        convert offset todo results type_parameters_left_angle
    | { M.syntax = M.TupleTypeSpecifier
        { M.tuple_left_paren
        ; M.tuple_types
        ; M.tuple_right_paren
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (tuple_right_paren, todo) in
        let todo = Convert (tuple_types, todo) in
        convert offset todo results tuple_left_paren
    | { M.syntax = M.ErrorSyntax
        { M.error_error
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        convert offset todo results error_error
    | { M.syntax = M.ListItem
        { M.list_item
        ; M.list_separator
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        let todo = Convert (list_separator, todo) in
        convert offset todo results list_item
    in
    convert 0 Done [] node
end

let from_minimal = FromMinimal.from_minimal

let from_tree tree =
  from_minimal (SyntaxTree.text tree) (SyntaxTree.root tree)

