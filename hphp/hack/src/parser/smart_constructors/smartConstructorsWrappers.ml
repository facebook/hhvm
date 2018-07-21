(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
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
 * This module contains smart constructors implementation that can be used to
 * build AST.
 
 *)

module type SC_S = SmartConstructors.SmartConstructors_S
module SK = Full_fidelity_syntax_kind

module type SyntaxKind_S = sig
  include SC_S
  type original_sc_r [@@deriving show]
  val extract : r -> original_sc_r
  val is_name : r -> bool
  val is_abstract : r -> bool
  val is_missing : r -> bool
  val is_list : r -> bool
  val is_end_of_file : r -> bool
  val is_script : r -> bool
  val is_qualified_name : r -> bool
  val is_simple_type_specifier : r -> bool
  val is_literal_expression : r -> bool
  val is_prefixed_string_expression : r -> bool
  val is_variable_expression : r -> bool
  val is_pipe_variable_expression : r -> bool
  val is_enum_declaration : r -> bool
  val is_enumerator : r -> bool
  val is_alias_declaration : r -> bool
  val is_property_declaration : r -> bool
  val is_property_declarator : r -> bool
  val is_namespace_declaration : r -> bool
  val is_namespace_body : r -> bool
  val is_namespace_empty_body : r -> bool
  val is_namespace_use_declaration : r -> bool
  val is_namespace_group_use_declaration : r -> bool
  val is_namespace_use_clause : r -> bool
  val is_function_declaration : r -> bool
  val is_function_declaration_header : r -> bool
  val is_where_clause : r -> bool
  val is_where_constraint : r -> bool
  val is_methodish_declaration : r -> bool
  val is_classish_declaration : r -> bool
  val is_classish_body : r -> bool
  val is_trait_use_precedence_item : r -> bool
  val is_trait_use_alias_item : r -> bool
  val is_trait_use_conflict_resolution : r -> bool
  val is_trait_use : r -> bool
  val is_require_clause : r -> bool
  val is_const_declaration : r -> bool
  val is_constant_declarator : r -> bool
  val is_type_const_declaration : r -> bool
  val is_decorated_expression : r -> bool
  val is_parameter_declaration : r -> bool
  val is_variadic_parameter : r -> bool
  val is_attribute_specification : r -> bool
  val is_attribute : r -> bool
  val is_inclusion_expression : r -> bool
  val is_inclusion_directive : r -> bool
  val is_compound_statement : r -> bool
  val is_alternate_loop_statement : r -> bool
  val is_expression_statement : r -> bool
  val is_markup_section : r -> bool
  val is_markup_suffix : r -> bool
  val is_unset_statement : r -> bool
  val is_let_statement : r -> bool
  val is_using_statement_block_scoped : r -> bool
  val is_using_statement_function_scoped : r -> bool
  val is_declare_directive_statement : r -> bool
  val is_declare_block_statement : r -> bool
  val is_while_statement : r -> bool
  val is_if_statement : r -> bool
  val is_elseif_clause : r -> bool
  val is_else_clause : r -> bool
  val is_alternate_if_statement : r -> bool
  val is_alternate_elseif_clause : r -> bool
  val is_alternate_else_clause : r -> bool
  val is_try_statement : r -> bool
  val is_catch_clause : r -> bool
  val is_finally_clause : r -> bool
  val is_do_statement : r -> bool
  val is_for_statement : r -> bool
  val is_foreach_statement : r -> bool
  val is_switch_statement : r -> bool
  val is_alternate_switch_statement : r -> bool
  val is_switch_section : r -> bool
  val is_switch_fallthrough : r -> bool
  val is_case_label : r -> bool
  val is_default_label : r -> bool
  val is_return_statement : r -> bool
  val is_goto_label : r -> bool
  val is_goto_statement : r -> bool
  val is_throw_statement : r -> bool
  val is_break_statement : r -> bool
  val is_continue_statement : r -> bool
  val is_function_static_statement : r -> bool
  val is_static_declarator : r -> bool
  val is_echo_statement : r -> bool
  val is_global_statement : r -> bool
  val is_simple_initializer : r -> bool
  val is_anonymous_class : r -> bool
  val is_anonymous_function : r -> bool
  val is_php7_anonymous_function : r -> bool
  val is_anonymous_function_use_clause : r -> bool
  val is_lambda_expression : r -> bool
  val is_lambda_signature : r -> bool
  val is_cast_expression : r -> bool
  val is_scope_resolution_expression : r -> bool
  val is_member_selection_expression : r -> bool
  val is_safe_member_selection_expression : r -> bool
  val is_embedded_member_selection_expression : r -> bool
  val is_yield_expression : r -> bool
  val is_yield_from_expression : r -> bool
  val is_prefix_unary_expression : r -> bool
  val is_postfix_unary_expression : r -> bool
  val is_binary_expression : r -> bool
  val is_instanceof_expression : r -> bool
  val is_is_expression : r -> bool
  val is_as_expression : r -> bool
  val is_nullable_as_expression : r -> bool
  val is_conditional_expression : r -> bool
  val is_eval_expression : r -> bool
  val is_empty_expression : r -> bool
  val is_define_expression : r -> bool
  val is_halt_compiler_expression : r -> bool
  val is_isset_expression : r -> bool
  val is_function_call_expression : r -> bool
  val is_function_call_with_type_arguments_expression : r -> bool
  val is_parenthesized_expression : r -> bool
  val is_braced_expression : r -> bool
  val is_embedded_braced_expression : r -> bool
  val is_list_expression : r -> bool
  val is_collection_literal_expression : r -> bool
  val is_object_creation_expression : r -> bool
  val is_constructor_call : r -> bool
  val is_array_creation_expression : r -> bool
  val is_array_intrinsic_expression : r -> bool
  val is_darray_intrinsic_expression : r -> bool
  val is_dictionary_intrinsic_expression : r -> bool
  val is_keyset_intrinsic_expression : r -> bool
  val is_varray_intrinsic_expression : r -> bool
  val is_vector_intrinsic_expression : r -> bool
  val is_element_initializer : r -> bool
  val is_subscript_expression : r -> bool
  val is_embedded_subscript_expression : r -> bool
  val is_awaitable_creation_expression : r -> bool
  val is_xhp_children_declaration : r -> bool
  val is_xhp_children_parenthesized_list : r -> bool
  val is_xhp_category_declaration : r -> bool
  val is_xhp_enum_type : r -> bool
  val is_xhp_required : r -> bool
  val is_xhp_class_attribute_declaration : r -> bool
  val is_xhp_class_attribute : r -> bool
  val is_xhp_simple_class_attribute : r -> bool
  val is_xhp_simple_attribute : r -> bool
  val is_xhp_spread_attribute : r -> bool
  val is_xhp_open : r -> bool
  val is_xhp_expression : r -> bool
  val is_xhp_close : r -> bool
  val is_type_constant : r -> bool
  val is_vector_type_specifier : r -> bool
  val is_keyset_type_specifier : r -> bool
  val is_tuple_type_explicit_specifier : r -> bool
  val is_varray_type_specifier : r -> bool
  val is_vector_array_type_specifier : r -> bool
  val is_type_parameter : r -> bool
  val is_type_constraint : r -> bool
  val is_darray_type_specifier : r -> bool
  val is_map_array_type_specifier : r -> bool
  val is_dictionary_type_specifier : r -> bool
  val is_closure_type_specifier : r -> bool
  val is_closure_parameter_type_specifier : r -> bool
  val is_classname_type_specifier : r -> bool
  val is_field_specifier : r -> bool
  val is_field_initializer : r -> bool
  val is_shape_type_specifier : r -> bool
  val is_shape_expression : r -> bool
  val is_tuple_expression : r -> bool
  val is_generic_type_specifier : r -> bool
  val is_nullable_type_specifier : r -> bool
  val is_soft_type_specifier : r -> bool
  val is_reified_type_argument : r -> bool
  val is_type_arguments : r -> bool
  val is_type_parameters : r -> bool
  val is_tuple_type_specifier : r -> bool
  val is_error : r -> bool
  val is_list_item : r -> bool

end

module SyntaxKind(SC : SC_S)
  : (SyntaxKind_S
    with module Token = SC.Token
    and type original_sc_r = SC.r
    and type t = SC.t
  ) = struct
  module Token = SC.Token
  type original_sc_r = SC.r [@@deriving show]
  type t = SC.t [@@deriving show]
  type r = SK.t * SC.r [@@deriving show]

  let extract (_, r) = r
  let kind_of (kind, _) = kind
  let compose : SK.t -> t * SC.r -> t * r = fun kind (state, res) ->
    state, (kind, res)
  let initial_state = SC.initial_state

  let make_token token state = compose (SK.Token (SC.Token.kind token)) (SC.make_token token state)
  let make_missing p state = compose SK.Missing (SC.make_missing p state)
  let make_list p items state =
    let kind = if items <> [] then SK.SyntaxList else SK.Missing in
    compose kind (SC.make_list p (Core_list.map ~f:snd items) state)
  let make_end_of_file arg0 state = compose SK.EndOfFile (SC.make_end_of_file (snd arg0) state)
  let make_script arg0 state = compose SK.Script (SC.make_script (snd arg0) state)
  let make_qualified_name arg0 state = compose SK.QualifiedName (SC.make_qualified_name (snd arg0) state)
  let make_simple_type_specifier arg0 state = compose SK.SimpleTypeSpecifier (SC.make_simple_type_specifier (snd arg0) state)
  let make_literal_expression arg0 state = compose SK.LiteralExpression (SC.make_literal_expression (snd arg0) state)
  let make_prefixed_string_expression arg0 arg1 state = compose SK.PrefixedStringExpression (SC.make_prefixed_string_expression (snd arg0) (snd arg1) state)
  let make_variable_expression arg0 state = compose SK.VariableExpression (SC.make_variable_expression (snd arg0) state)
  let make_pipe_variable_expression arg0 state = compose SK.PipeVariableExpression (SC.make_pipe_variable_expression (snd arg0) state)
  let make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = compose SK.EnumDeclaration (SC.make_enum_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) state)
  let make_enumerator arg0 arg1 arg2 arg3 state = compose SK.Enumerator (SC.make_enumerator (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state = compose SK.AliasDeclaration (SC.make_alias_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) state)
  let make_property_declaration arg0 arg1 arg2 arg3 arg4 state = compose SK.PropertyDeclaration (SC.make_property_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_property_declarator arg0 arg1 state = compose SK.PropertyDeclarator (SC.make_property_declarator (snd arg0) (snd arg1) state)
  let make_namespace_declaration arg0 arg1 arg2 state = compose SK.NamespaceDeclaration (SC.make_namespace_declaration (snd arg0) (snd arg1) (snd arg2) state)
  let make_namespace_body arg0 arg1 arg2 state = compose SK.NamespaceBody (SC.make_namespace_body (snd arg0) (snd arg1) (snd arg2) state)
  let make_namespace_empty_body arg0 state = compose SK.NamespaceEmptyBody (SC.make_namespace_empty_body (snd arg0) state)
  let make_namespace_use_declaration arg0 arg1 arg2 arg3 state = compose SK.NamespaceUseDeclaration (SC.make_namespace_use_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = compose SK.NamespaceGroupUseDeclaration (SC.make_namespace_group_use_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) state)
  let make_namespace_use_clause arg0 arg1 arg2 arg3 state = compose SK.NamespaceUseClause (SC.make_namespace_use_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_function_declaration arg0 arg1 arg2 state = compose SK.FunctionDeclaration (SC.make_function_declaration (snd arg0) (snd arg1) (snd arg2) state)
  let make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 state = compose SK.FunctionDeclarationHeader (SC.make_function_declaration_header (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) (snd arg10) state)
  let make_where_clause arg0 arg1 state = compose SK.WhereClause (SC.make_where_clause (snd arg0) (snd arg1) state)
  let make_where_constraint arg0 arg1 arg2 state = compose SK.WhereConstraint (SC.make_where_constraint (snd arg0) (snd arg1) (snd arg2) state)
  let make_methodish_declaration arg0 arg1 arg2 arg3 state = compose SK.MethodishDeclaration (SC.make_methodish_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = compose SK.ClassishDeclaration (SC.make_classish_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) state)
  let make_classish_body arg0 arg1 arg2 state = compose SK.ClassishBody (SC.make_classish_body (snd arg0) (snd arg1) (snd arg2) state)
  let make_trait_use_precedence_item arg0 arg1 arg2 state = compose SK.TraitUsePrecedenceItem (SC.make_trait_use_precedence_item (snd arg0) (snd arg1) (snd arg2) state)
  let make_trait_use_alias_item arg0 arg1 arg2 arg3 state = compose SK.TraitUseAliasItem (SC.make_trait_use_alias_item (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4 state = compose SK.TraitUseConflictResolution (SC.make_trait_use_conflict_resolution (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_trait_use arg0 arg1 arg2 state = compose SK.TraitUse (SC.make_trait_use (snd arg0) (snd arg1) (snd arg2) state)
  let make_require_clause arg0 arg1 arg2 arg3 state = compose SK.RequireClause (SC.make_require_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.ConstDeclaration (SC.make_const_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_constant_declarator arg0 arg1 state = compose SK.ConstantDeclarator (SC.make_constant_declarator (snd arg0) (snd arg1) state)
  let make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = compose SK.TypeConstDeclaration (SC.make_type_const_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) state)
  let make_decorated_expression arg0 arg1 state = compose SK.DecoratedExpression (SC.make_decorated_expression (snd arg0) (snd arg1) state)
  let make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.ParameterDeclaration (SC.make_parameter_declaration (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_variadic_parameter arg0 arg1 arg2 state = compose SK.VariadicParameter (SC.make_variadic_parameter (snd arg0) (snd arg1) (snd arg2) state)
  let make_attribute_specification arg0 arg1 arg2 state = compose SK.AttributeSpecification (SC.make_attribute_specification (snd arg0) (snd arg1) (snd arg2) state)
  let make_attribute arg0 arg1 arg2 arg3 state = compose SK.Attribute (SC.make_attribute (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_inclusion_expression arg0 arg1 state = compose SK.InclusionExpression (SC.make_inclusion_expression (snd arg0) (snd arg1) state)
  let make_inclusion_directive arg0 arg1 state = compose SK.InclusionDirective (SC.make_inclusion_directive (snd arg0) (snd arg1) state)
  let make_compound_statement arg0 arg1 arg2 state = compose SK.CompoundStatement (SC.make_compound_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_alternate_loop_statement arg0 arg1 arg2 arg3 state = compose SK.AlternateLoopStatement (SC.make_alternate_loop_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_expression_statement arg0 arg1 state = compose SK.ExpressionStatement (SC.make_expression_statement (snd arg0) (snd arg1) state)
  let make_markup_section arg0 arg1 arg2 arg3 state = compose SK.MarkupSection (SC.make_markup_section (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_markup_suffix arg0 arg1 state = compose SK.MarkupSuffix (SC.make_markup_suffix (snd arg0) (snd arg1) state)
  let make_unset_statement arg0 arg1 arg2 arg3 arg4 state = compose SK.UnsetStatement (SC.make_unset_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_let_statement arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.LetStatement (SC.make_let_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.UsingStatementBlockScoped (SC.make_using_statement_block_scoped (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_using_statement_function_scoped arg0 arg1 arg2 arg3 state = compose SK.UsingStatementFunctionScoped (SC.make_using_statement_function_scoped (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_declare_directive_statement arg0 arg1 arg2 arg3 arg4 state = compose SK.DeclareDirectiveStatement (SC.make_declare_directive_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_declare_block_statement arg0 arg1 arg2 arg3 arg4 state = compose SK.DeclareBlockStatement (SC.make_declare_block_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_while_statement arg0 arg1 arg2 arg3 arg4 state = compose SK.WhileStatement (SC.make_while_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = compose SK.IfStatement (SC.make_if_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) state)
  let make_elseif_clause arg0 arg1 arg2 arg3 arg4 state = compose SK.ElseifClause (SC.make_elseif_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_else_clause arg0 arg1 state = compose SK.ElseClause (SC.make_else_clause (snd arg0) (snd arg1) state)
  let make_alternate_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = compose SK.AlternateIfStatement (SC.make_alternate_if_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) state)
  let make_alternate_elseif_clause arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.AlternateElseifClause (SC.make_alternate_elseif_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_alternate_else_clause arg0 arg1 arg2 state = compose SK.AlternateElseClause (SC.make_alternate_else_clause (snd arg0) (snd arg1) (snd arg2) state)
  let make_try_statement arg0 arg1 arg2 arg3 state = compose SK.TryStatement (SC.make_try_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.CatchClause (SC.make_catch_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_finally_clause arg0 arg1 state = compose SK.FinallyClause (SC.make_finally_clause (snd arg0) (snd arg1) state)
  let make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = compose SK.DoStatement (SC.make_do_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) state)
  let make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = compose SK.ForStatement (SC.make_for_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) state)
  let make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = compose SK.ForeachStatement (SC.make_foreach_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) state)
  let make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = compose SK.SwitchStatement (SC.make_switch_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) state)
  let make_alternate_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state = compose SK.AlternateSwitchStatement (SC.make_alternate_switch_statement (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) state)
  let make_switch_section arg0 arg1 arg2 state = compose SK.SwitchSection (SC.make_switch_section (snd arg0) (snd arg1) (snd arg2) state)
  let make_switch_fallthrough arg0 arg1 state = compose SK.SwitchFallthrough (SC.make_switch_fallthrough (snd arg0) (snd arg1) state)
  let make_case_label arg0 arg1 arg2 state = compose SK.CaseLabel (SC.make_case_label (snd arg0) (snd arg1) (snd arg2) state)
  let make_default_label arg0 arg1 state = compose SK.DefaultLabel (SC.make_default_label (snd arg0) (snd arg1) state)
  let make_return_statement arg0 arg1 arg2 state = compose SK.ReturnStatement (SC.make_return_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_goto_label arg0 arg1 state = compose SK.GotoLabel (SC.make_goto_label (snd arg0) (snd arg1) state)
  let make_goto_statement arg0 arg1 arg2 state = compose SK.GotoStatement (SC.make_goto_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_throw_statement arg0 arg1 arg2 state = compose SK.ThrowStatement (SC.make_throw_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_break_statement arg0 arg1 arg2 state = compose SK.BreakStatement (SC.make_break_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_continue_statement arg0 arg1 arg2 state = compose SK.ContinueStatement (SC.make_continue_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_function_static_statement arg0 arg1 arg2 state = compose SK.FunctionStaticStatement (SC.make_function_static_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_static_declarator arg0 arg1 state = compose SK.StaticDeclarator (SC.make_static_declarator (snd arg0) (snd arg1) state)
  let make_echo_statement arg0 arg1 arg2 state = compose SK.EchoStatement (SC.make_echo_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_global_statement arg0 arg1 arg2 state = compose SK.GlobalStatement (SC.make_global_statement (snd arg0) (snd arg1) (snd arg2) state)
  let make_simple_initializer arg0 arg1 state = compose SK.SimpleInitializer (SC.make_simple_initializer (snd arg0) (snd arg1) state)
  let make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = compose SK.AnonymousClass (SC.make_anonymous_class (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) state)
  let make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state = compose SK.AnonymousFunction (SC.make_anonymous_function (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) (snd arg10) (snd arg11) (snd arg12) state)
  let make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state = compose SK.Php7AnonymousFunction (SC.make_php7_anonymous_function (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) (snd arg9) (snd arg10) (snd arg11) (snd arg12) state)
  let make_anonymous_function_use_clause arg0 arg1 arg2 arg3 state = compose SK.AnonymousFunctionUseClause (SC.make_anonymous_function_use_clause (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_lambda_expression arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.LambdaExpression (SC.make_lambda_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_lambda_signature arg0 arg1 arg2 arg3 arg4 state = compose SK.LambdaSignature (SC.make_lambda_signature (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_cast_expression arg0 arg1 arg2 arg3 state = compose SK.CastExpression (SC.make_cast_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_scope_resolution_expression arg0 arg1 arg2 state = compose SK.ScopeResolutionExpression (SC.make_scope_resolution_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_member_selection_expression arg0 arg1 arg2 state = compose SK.MemberSelectionExpression (SC.make_member_selection_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_safe_member_selection_expression arg0 arg1 arg2 state = compose SK.SafeMemberSelectionExpression (SC.make_safe_member_selection_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_embedded_member_selection_expression arg0 arg1 arg2 state = compose SK.EmbeddedMemberSelectionExpression (SC.make_embedded_member_selection_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_yield_expression arg0 arg1 state = compose SK.YieldExpression (SC.make_yield_expression (snd arg0) (snd arg1) state)
  let make_yield_from_expression arg0 arg1 arg2 state = compose SK.YieldFromExpression (SC.make_yield_from_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_prefix_unary_expression arg0 arg1 state = compose SK.PrefixUnaryExpression (SC.make_prefix_unary_expression (snd arg0) (snd arg1) state)
  let make_postfix_unary_expression arg0 arg1 state = compose SK.PostfixUnaryExpression (SC.make_postfix_unary_expression (snd arg0) (snd arg1) state)
  let make_binary_expression arg0 arg1 arg2 state = compose SK.BinaryExpression (SC.make_binary_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_instanceof_expression arg0 arg1 arg2 state = compose SK.InstanceofExpression (SC.make_instanceof_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_is_expression arg0 arg1 arg2 state = compose SK.IsExpression (SC.make_is_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_as_expression arg0 arg1 arg2 state = compose SK.AsExpression (SC.make_as_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_nullable_as_expression arg0 arg1 arg2 state = compose SK.NullableAsExpression (SC.make_nullable_as_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_conditional_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.ConditionalExpression (SC.make_conditional_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_eval_expression arg0 arg1 arg2 arg3 state = compose SK.EvalExpression (SC.make_eval_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_empty_expression arg0 arg1 arg2 arg3 state = compose SK.EmptyExpression (SC.make_empty_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_define_expression arg0 arg1 arg2 arg3 state = compose SK.DefineExpression (SC.make_define_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_halt_compiler_expression arg0 arg1 arg2 arg3 state = compose SK.HaltCompilerExpression (SC.make_halt_compiler_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_isset_expression arg0 arg1 arg2 arg3 state = compose SK.IssetExpression (SC.make_isset_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_function_call_expression arg0 arg1 arg2 arg3 state = compose SK.FunctionCallExpression (SC.make_function_call_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.FunctionCallWithTypeArgumentsExpression (SC.make_function_call_with_type_arguments_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_parenthesized_expression arg0 arg1 arg2 state = compose SK.ParenthesizedExpression (SC.make_parenthesized_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_braced_expression arg0 arg1 arg2 state = compose SK.BracedExpression (SC.make_braced_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_embedded_braced_expression arg0 arg1 arg2 state = compose SK.EmbeddedBracedExpression (SC.make_embedded_braced_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_list_expression arg0 arg1 arg2 arg3 state = compose SK.ListExpression (SC.make_list_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_collection_literal_expression arg0 arg1 arg2 arg3 state = compose SK.CollectionLiteralExpression (SC.make_collection_literal_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_object_creation_expression arg0 arg1 state = compose SK.ObjectCreationExpression (SC.make_object_creation_expression (snd arg0) (snd arg1) state)
  let make_constructor_call arg0 arg1 arg2 arg3 state = compose SK.ConstructorCall (SC.make_constructor_call (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_array_creation_expression arg0 arg1 arg2 state = compose SK.ArrayCreationExpression (SC.make_array_creation_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_array_intrinsic_expression arg0 arg1 arg2 arg3 state = compose SK.ArrayIntrinsicExpression (SC.make_array_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_darray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.DarrayIntrinsicExpression (SC.make_darray_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.DictionaryIntrinsicExpression (SC.make_dictionary_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.KeysetIntrinsicExpression (SC.make_keyset_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_varray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.VarrayIntrinsicExpression (SC.make_varray_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_vector_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = compose SK.VectorIntrinsicExpression (SC.make_vector_intrinsic_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_element_initializer arg0 arg1 arg2 state = compose SK.ElementInitializer (SC.make_element_initializer (snd arg0) (snd arg1) (snd arg2) state)
  let make_subscript_expression arg0 arg1 arg2 arg3 state = compose SK.SubscriptExpression (SC.make_subscript_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_embedded_subscript_expression arg0 arg1 arg2 arg3 state = compose SK.EmbeddedSubscriptExpression (SC.make_embedded_subscript_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_awaitable_creation_expression arg0 arg1 arg2 arg3 state = compose SK.AwaitableCreationExpression (SC.make_awaitable_creation_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_xhp_children_declaration arg0 arg1 arg2 state = compose SK.XHPChildrenDeclaration (SC.make_xhp_children_declaration (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_children_parenthesized_list arg0 arg1 arg2 state = compose SK.XHPChildrenParenthesizedList (SC.make_xhp_children_parenthesized_list (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_category_declaration arg0 arg1 arg2 state = compose SK.XHPCategoryDeclaration (SC.make_xhp_category_declaration (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_enum_type arg0 arg1 arg2 arg3 arg4 state = compose SK.XHPEnumType (SC.make_xhp_enum_type (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_xhp_required arg0 arg1 state = compose SK.XHPRequired (SC.make_xhp_required (snd arg0) (snd arg1) state)
  let make_xhp_class_attribute_declaration arg0 arg1 arg2 state = compose SK.XHPClassAttributeDeclaration (SC.make_xhp_class_attribute_declaration (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_class_attribute arg0 arg1 arg2 arg3 state = compose SK.XHPClassAttribute (SC.make_xhp_class_attribute (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_xhp_simple_class_attribute arg0 state = compose SK.XHPSimpleClassAttribute (SC.make_xhp_simple_class_attribute (snd arg0) state)
  let make_xhp_simple_attribute arg0 arg1 arg2 state = compose SK.XHPSimpleAttribute (SC.make_xhp_simple_attribute (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_spread_attribute arg0 arg1 arg2 arg3 state = compose SK.XHPSpreadAttribute (SC.make_xhp_spread_attribute (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_xhp_open arg0 arg1 arg2 arg3 state = compose SK.XHPOpen (SC.make_xhp_open (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_xhp_expression arg0 arg1 arg2 state = compose SK.XHPExpression (SC.make_xhp_expression (snd arg0) (snd arg1) (snd arg2) state)
  let make_xhp_close arg0 arg1 arg2 state = compose SK.XHPClose (SC.make_xhp_close (snd arg0) (snd arg1) (snd arg2) state)
  let make_type_constant arg0 arg1 arg2 state = compose SK.TypeConstant (SC.make_type_constant (snd arg0) (snd arg1) (snd arg2) state)
  let make_vector_type_specifier arg0 arg1 arg2 arg3 arg4 state = compose SK.VectorTypeSpecifier (SC.make_vector_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4 state = compose SK.KeysetTypeSpecifier (SC.make_keyset_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3 state = compose SK.TupleTypeExplicitSpecifier (SC.make_tuple_type_explicit_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_varray_type_specifier arg0 arg1 arg2 arg3 arg4 state = compose SK.VarrayTypeSpecifier (SC.make_varray_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_vector_array_type_specifier arg0 arg1 arg2 arg3 state = compose SK.VectorArrayTypeSpecifier (SC.make_vector_array_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_type_parameter arg0 arg1 arg2 arg3 state = compose SK.TypeParameter (SC.make_type_parameter (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_type_constraint arg0 arg1 state = compose SK.TypeConstraint (SC.make_type_constraint (snd arg0) (snd arg1) state)
  let make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = compose SK.DarrayTypeSpecifier (SC.make_darray_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) state)
  let make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 state = compose SK.MapArrayTypeSpecifier (SC.make_map_array_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) state)
  let make_dictionary_type_specifier arg0 arg1 arg2 arg3 state = compose SK.DictionaryTypeSpecifier (SC.make_dictionary_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = compose SK.ClosureTypeSpecifier (SC.make_closure_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) (snd arg5) (snd arg6) (snd arg7) (snd arg8) state)
  let make_closure_parameter_type_specifier arg0 arg1 state = compose SK.ClosureParameterTypeSpecifier (SC.make_closure_parameter_type_specifier (snd arg0) (snd arg1) state)
  let make_classname_type_specifier arg0 arg1 arg2 arg3 arg4 state = compose SK.ClassnameTypeSpecifier (SC.make_classname_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_field_specifier arg0 arg1 arg2 arg3 state = compose SK.FieldSpecifier (SC.make_field_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_field_initializer arg0 arg1 arg2 state = compose SK.FieldInitializer (SC.make_field_initializer (snd arg0) (snd arg1) (snd arg2) state)
  let make_shape_type_specifier arg0 arg1 arg2 arg3 arg4 state = compose SK.ShapeTypeSpecifier (SC.make_shape_type_specifier (snd arg0) (snd arg1) (snd arg2) (snd arg3) (snd arg4) state)
  let make_shape_expression arg0 arg1 arg2 arg3 state = compose SK.ShapeExpression (SC.make_shape_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_tuple_expression arg0 arg1 arg2 arg3 state = compose SK.TupleExpression (SC.make_tuple_expression (snd arg0) (snd arg1) (snd arg2) (snd arg3) state)
  let make_generic_type_specifier arg0 arg1 state = compose SK.GenericTypeSpecifier (SC.make_generic_type_specifier (snd arg0) (snd arg1) state)
  let make_nullable_type_specifier arg0 arg1 state = compose SK.NullableTypeSpecifier (SC.make_nullable_type_specifier (snd arg0) (snd arg1) state)
  let make_soft_type_specifier arg0 arg1 state = compose SK.SoftTypeSpecifier (SC.make_soft_type_specifier (snd arg0) (snd arg1) state)
  let make_reified_type_argument arg0 arg1 state = compose SK.ReifiedTypeArgument (SC.make_reified_type_argument (snd arg0) (snd arg1) state)
  let make_type_arguments arg0 arg1 arg2 state = compose SK.TypeArguments (SC.make_type_arguments (snd arg0) (snd arg1) (snd arg2) state)
  let make_type_parameters arg0 arg1 arg2 state = compose SK.TypeParameters (SC.make_type_parameters (snd arg0) (snd arg1) (snd arg2) state)
  let make_tuple_type_specifier arg0 arg1 arg2 state = compose SK.TupleTypeSpecifier (SC.make_tuple_type_specifier (snd arg0) (snd arg1) (snd arg2) state)
  let make_error arg0 state = compose SK.ErrorSyntax (SC.make_error (snd arg0) state)
  let make_list_item arg0 arg1 state = compose SK.ListItem (SC.make_list_item (snd arg0) (snd arg1) state)


  let has_kind kind node = kind_of node = kind
  let is_name = has_kind (SK.Token Full_fidelity_token_kind.Name)
  let is_abstract = has_kind (SK.Token Full_fidelity_token_kind.Abstract)
  let is_missing = has_kind SK.Missing
  let is_list = has_kind SK.Missing
  let is_end_of_file                                  = has_kind SK.EndOfFile
  let is_script                                       = has_kind SK.Script
  let is_qualified_name                               = has_kind SK.QualifiedName
  let is_simple_type_specifier                        = has_kind SK.SimpleTypeSpecifier
  let is_literal_expression                           = has_kind SK.LiteralExpression
  let is_prefixed_string_expression                   = has_kind SK.PrefixedStringExpression
  let is_variable_expression                          = has_kind SK.VariableExpression
  let is_pipe_variable_expression                     = has_kind SK.PipeVariableExpression
  let is_enum_declaration                             = has_kind SK.EnumDeclaration
  let is_enumerator                                   = has_kind SK.Enumerator
  let is_alias_declaration                            = has_kind SK.AliasDeclaration
  let is_property_declaration                         = has_kind SK.PropertyDeclaration
  let is_property_declarator                          = has_kind SK.PropertyDeclarator
  let is_namespace_declaration                        = has_kind SK.NamespaceDeclaration
  let is_namespace_body                               = has_kind SK.NamespaceBody
  let is_namespace_empty_body                         = has_kind SK.NamespaceEmptyBody
  let is_namespace_use_declaration                    = has_kind SK.NamespaceUseDeclaration
  let is_namespace_group_use_declaration              = has_kind SK.NamespaceGroupUseDeclaration
  let is_namespace_use_clause                         = has_kind SK.NamespaceUseClause
  let is_function_declaration                         = has_kind SK.FunctionDeclaration
  let is_function_declaration_header                  = has_kind SK.FunctionDeclarationHeader
  let is_where_clause                                 = has_kind SK.WhereClause
  let is_where_constraint                             = has_kind SK.WhereConstraint
  let is_methodish_declaration                        = has_kind SK.MethodishDeclaration
  let is_classish_declaration                         = has_kind SK.ClassishDeclaration
  let is_classish_body                                = has_kind SK.ClassishBody
  let is_trait_use_precedence_item                    = has_kind SK.TraitUsePrecedenceItem
  let is_trait_use_alias_item                         = has_kind SK.TraitUseAliasItem
  let is_trait_use_conflict_resolution                = has_kind SK.TraitUseConflictResolution
  let is_trait_use                                    = has_kind SK.TraitUse
  let is_require_clause                               = has_kind SK.RequireClause
  let is_const_declaration                            = has_kind SK.ConstDeclaration
  let is_constant_declarator                          = has_kind SK.ConstantDeclarator
  let is_type_const_declaration                       = has_kind SK.TypeConstDeclaration
  let is_decorated_expression                         = has_kind SK.DecoratedExpression
  let is_parameter_declaration                        = has_kind SK.ParameterDeclaration
  let is_variadic_parameter                           = has_kind SK.VariadicParameter
  let is_attribute_specification                      = has_kind SK.AttributeSpecification
  let is_attribute                                    = has_kind SK.Attribute
  let is_inclusion_expression                         = has_kind SK.InclusionExpression
  let is_inclusion_directive                          = has_kind SK.InclusionDirective
  let is_compound_statement                           = has_kind SK.CompoundStatement
  let is_alternate_loop_statement                     = has_kind SK.AlternateLoopStatement
  let is_expression_statement                         = has_kind SK.ExpressionStatement
  let is_markup_section                               = has_kind SK.MarkupSection
  let is_markup_suffix                                = has_kind SK.MarkupSuffix
  let is_unset_statement                              = has_kind SK.UnsetStatement
  let is_let_statement                                = has_kind SK.LetStatement
  let is_using_statement_block_scoped                 = has_kind SK.UsingStatementBlockScoped
  let is_using_statement_function_scoped              = has_kind SK.UsingStatementFunctionScoped
  let is_declare_directive_statement                  = has_kind SK.DeclareDirectiveStatement
  let is_declare_block_statement                      = has_kind SK.DeclareBlockStatement
  let is_while_statement                              = has_kind SK.WhileStatement
  let is_if_statement                                 = has_kind SK.IfStatement
  let is_elseif_clause                                = has_kind SK.ElseifClause
  let is_else_clause                                  = has_kind SK.ElseClause
  let is_alternate_if_statement                       = has_kind SK.AlternateIfStatement
  let is_alternate_elseif_clause                      = has_kind SK.AlternateElseifClause
  let is_alternate_else_clause                        = has_kind SK.AlternateElseClause
  let is_try_statement                                = has_kind SK.TryStatement
  let is_catch_clause                                 = has_kind SK.CatchClause
  let is_finally_clause                               = has_kind SK.FinallyClause
  let is_do_statement                                 = has_kind SK.DoStatement
  let is_for_statement                                = has_kind SK.ForStatement
  let is_foreach_statement                            = has_kind SK.ForeachStatement
  let is_switch_statement                             = has_kind SK.SwitchStatement
  let is_alternate_switch_statement                   = has_kind SK.AlternateSwitchStatement
  let is_switch_section                               = has_kind SK.SwitchSection
  let is_switch_fallthrough                           = has_kind SK.SwitchFallthrough
  let is_case_label                                   = has_kind SK.CaseLabel
  let is_default_label                                = has_kind SK.DefaultLabel
  let is_return_statement                             = has_kind SK.ReturnStatement
  let is_goto_label                                   = has_kind SK.GotoLabel
  let is_goto_statement                               = has_kind SK.GotoStatement
  let is_throw_statement                              = has_kind SK.ThrowStatement
  let is_break_statement                              = has_kind SK.BreakStatement
  let is_continue_statement                           = has_kind SK.ContinueStatement
  let is_function_static_statement                    = has_kind SK.FunctionStaticStatement
  let is_static_declarator                            = has_kind SK.StaticDeclarator
  let is_echo_statement                               = has_kind SK.EchoStatement
  let is_global_statement                             = has_kind SK.GlobalStatement
  let is_simple_initializer                           = has_kind SK.SimpleInitializer
  let is_anonymous_class                              = has_kind SK.AnonymousClass
  let is_anonymous_function                           = has_kind SK.AnonymousFunction
  let is_php7_anonymous_function                      = has_kind SK.Php7AnonymousFunction
  let is_anonymous_function_use_clause                = has_kind SK.AnonymousFunctionUseClause
  let is_lambda_expression                            = has_kind SK.LambdaExpression
  let is_lambda_signature                             = has_kind SK.LambdaSignature
  let is_cast_expression                              = has_kind SK.CastExpression
  let is_scope_resolution_expression                  = has_kind SK.ScopeResolutionExpression
  let is_member_selection_expression                  = has_kind SK.MemberSelectionExpression
  let is_safe_member_selection_expression             = has_kind SK.SafeMemberSelectionExpression
  let is_embedded_member_selection_expression         = has_kind SK.EmbeddedMemberSelectionExpression
  let is_yield_expression                             = has_kind SK.YieldExpression
  let is_yield_from_expression                        = has_kind SK.YieldFromExpression
  let is_prefix_unary_expression                      = has_kind SK.PrefixUnaryExpression
  let is_postfix_unary_expression                     = has_kind SK.PostfixUnaryExpression
  let is_binary_expression                            = has_kind SK.BinaryExpression
  let is_instanceof_expression                        = has_kind SK.InstanceofExpression
  let is_is_expression                                = has_kind SK.IsExpression
  let is_as_expression                                = has_kind SK.AsExpression
  let is_nullable_as_expression                       = has_kind SK.NullableAsExpression
  let is_conditional_expression                       = has_kind SK.ConditionalExpression
  let is_eval_expression                              = has_kind SK.EvalExpression
  let is_empty_expression                             = has_kind SK.EmptyExpression
  let is_define_expression                            = has_kind SK.DefineExpression
  let is_halt_compiler_expression                     = has_kind SK.HaltCompilerExpression
  let is_isset_expression                             = has_kind SK.IssetExpression
  let is_function_call_expression                     = has_kind SK.FunctionCallExpression
  let is_function_call_with_type_arguments_expression = has_kind SK.FunctionCallWithTypeArgumentsExpression
  let is_parenthesized_expression                     = has_kind SK.ParenthesizedExpression
  let is_braced_expression                            = has_kind SK.BracedExpression
  let is_embedded_braced_expression                   = has_kind SK.EmbeddedBracedExpression
  let is_list_expression                              = has_kind SK.ListExpression
  let is_collection_literal_expression                = has_kind SK.CollectionLiteralExpression
  let is_object_creation_expression                   = has_kind SK.ObjectCreationExpression
  let is_constructor_call                             = has_kind SK.ConstructorCall
  let is_array_creation_expression                    = has_kind SK.ArrayCreationExpression
  let is_array_intrinsic_expression                   = has_kind SK.ArrayIntrinsicExpression
  let is_darray_intrinsic_expression                  = has_kind SK.DarrayIntrinsicExpression
  let is_dictionary_intrinsic_expression              = has_kind SK.DictionaryIntrinsicExpression
  let is_keyset_intrinsic_expression                  = has_kind SK.KeysetIntrinsicExpression
  let is_varray_intrinsic_expression                  = has_kind SK.VarrayIntrinsicExpression
  let is_vector_intrinsic_expression                  = has_kind SK.VectorIntrinsicExpression
  let is_element_initializer                          = has_kind SK.ElementInitializer
  let is_subscript_expression                         = has_kind SK.SubscriptExpression
  let is_embedded_subscript_expression                = has_kind SK.EmbeddedSubscriptExpression
  let is_awaitable_creation_expression                = has_kind SK.AwaitableCreationExpression
  let is_xhp_children_declaration                     = has_kind SK.XHPChildrenDeclaration
  let is_xhp_children_parenthesized_list              = has_kind SK.XHPChildrenParenthesizedList
  let is_xhp_category_declaration                     = has_kind SK.XHPCategoryDeclaration
  let is_xhp_enum_type                                = has_kind SK.XHPEnumType
  let is_xhp_required                                 = has_kind SK.XHPRequired
  let is_xhp_class_attribute_declaration              = has_kind SK.XHPClassAttributeDeclaration
  let is_xhp_class_attribute                          = has_kind SK.XHPClassAttribute
  let is_xhp_simple_class_attribute                   = has_kind SK.XHPSimpleClassAttribute
  let is_xhp_simple_attribute                         = has_kind SK.XHPSimpleAttribute
  let is_xhp_spread_attribute                         = has_kind SK.XHPSpreadAttribute
  let is_xhp_open                                     = has_kind SK.XHPOpen
  let is_xhp_expression                               = has_kind SK.XHPExpression
  let is_xhp_close                                    = has_kind SK.XHPClose
  let is_type_constant                                = has_kind SK.TypeConstant
  let is_vector_type_specifier                        = has_kind SK.VectorTypeSpecifier
  let is_keyset_type_specifier                        = has_kind SK.KeysetTypeSpecifier
  let is_tuple_type_explicit_specifier                = has_kind SK.TupleTypeExplicitSpecifier
  let is_varray_type_specifier                        = has_kind SK.VarrayTypeSpecifier
  let is_vector_array_type_specifier                  = has_kind SK.VectorArrayTypeSpecifier
  let is_type_parameter                               = has_kind SK.TypeParameter
  let is_type_constraint                              = has_kind SK.TypeConstraint
  let is_darray_type_specifier                        = has_kind SK.DarrayTypeSpecifier
  let is_map_array_type_specifier                     = has_kind SK.MapArrayTypeSpecifier
  let is_dictionary_type_specifier                    = has_kind SK.DictionaryTypeSpecifier
  let is_closure_type_specifier                       = has_kind SK.ClosureTypeSpecifier
  let is_closure_parameter_type_specifier             = has_kind SK.ClosureParameterTypeSpecifier
  let is_classname_type_specifier                     = has_kind SK.ClassnameTypeSpecifier
  let is_field_specifier                              = has_kind SK.FieldSpecifier
  let is_field_initializer                            = has_kind SK.FieldInitializer
  let is_shape_type_specifier                         = has_kind SK.ShapeTypeSpecifier
  let is_shape_expression                             = has_kind SK.ShapeExpression
  let is_tuple_expression                             = has_kind SK.TupleExpression
  let is_generic_type_specifier                       = has_kind SK.GenericTypeSpecifier
  let is_nullable_type_specifier                      = has_kind SK.NullableTypeSpecifier
  let is_soft_type_specifier                          = has_kind SK.SoftTypeSpecifier
  let is_reified_type_argument                        = has_kind SK.ReifiedTypeArgument
  let is_type_arguments                               = has_kind SK.TypeArguments
  let is_type_parameters                              = has_kind SK.TypeParameters
  let is_tuple_type_specifier                         = has_kind SK.TupleTypeSpecifier
  let is_error                                        = has_kind SK.ErrorSyntax
  let is_list_item                                    = has_kind SK.ListItem


end (* SyntaxKind *)
