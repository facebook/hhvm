/**
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
 */
use crate::syntax::*;

pub trait SyntaxType<C>: SyntaxTypeBase<C>
{
    fn make_end_of_file(ctx: &C, end_of_file_token: Self) -> Self;
    fn make_script(ctx: &C, script_declarations: Self) -> Self;
    fn make_qualified_name(ctx: &C, qualified_name_parts: Self) -> Self;
    fn make_module_name(ctx: &C, module_name_parts: Self) -> Self;
    fn make_simple_type_specifier(ctx: &C, simple_type_specifier: Self) -> Self;
    fn make_literal_expression(ctx: &C, literal_expression: Self) -> Self;
    fn make_prefixed_string_expression(ctx: &C, prefixed_string_name: Self, prefixed_string_str: Self) -> Self;
    fn make_prefixed_code_expression(ctx: &C, prefixed_code_prefix: Self, prefixed_code_left_backtick: Self, prefixed_code_body: Self, prefixed_code_right_backtick: Self) -> Self;
    fn make_variable_expression(ctx: &C, variable_expression: Self) -> Self;
    fn make_pipe_variable_expression(ctx: &C, pipe_variable_expression: Self) -> Self;
    fn make_file_attribute_specification(ctx: &C, file_attribute_specification_left_double_angle: Self, file_attribute_specification_keyword: Self, file_attribute_specification_colon: Self, file_attribute_specification_attributes: Self, file_attribute_specification_right_double_angle: Self) -> Self;
    fn make_enum_declaration(ctx: &C, enum_attribute_spec: Self, enum_modifiers: Self, enum_keyword: Self, enum_name: Self, enum_colon: Self, enum_base: Self, enum_type: Self, enum_left_brace: Self, enum_use_clauses: Self, enum_enumerators: Self, enum_right_brace: Self) -> Self;
    fn make_enum_use(ctx: &C, enum_use_keyword: Self, enum_use_names: Self, enum_use_semicolon: Self) -> Self;
    fn make_enumerator(ctx: &C, enumerator_name: Self, enumerator_equal: Self, enumerator_value: Self, enumerator_semicolon: Self) -> Self;
    fn make_enum_class_declaration(ctx: &C, enum_class_attribute_spec: Self, enum_class_modifiers: Self, enum_class_enum_keyword: Self, enum_class_class_keyword: Self, enum_class_name: Self, enum_class_colon: Self, enum_class_base: Self, enum_class_extends: Self, enum_class_extends_list: Self, enum_class_left_brace: Self, enum_class_elements: Self, enum_class_right_brace: Self) -> Self;
    fn make_enum_class_enumerator(ctx: &C, enum_class_enumerator_modifiers: Self, enum_class_enumerator_type: Self, enum_class_enumerator_name: Self, enum_class_enumerator_initializer: Self, enum_class_enumerator_semicolon: Self) -> Self;
    fn make_alias_declaration(ctx: &C, alias_attribute_spec: Self, alias_modifiers: Self, alias_module_kw_opt: Self, alias_keyword: Self, alias_name: Self, alias_generic_parameter: Self, alias_constraint: Self, alias_equal: Self, alias_type: Self, alias_semicolon: Self) -> Self;
    fn make_context_alias_declaration(ctx: &C, ctx_alias_attribute_spec: Self, ctx_alias_keyword: Self, ctx_alias_name: Self, ctx_alias_generic_parameter: Self, ctx_alias_as_constraint: Self, ctx_alias_equal: Self, ctx_alias_context: Self, ctx_alias_semicolon: Self) -> Self;
    fn make_case_type_declaration(ctx: &C, case_type_attribute_spec: Self, case_type_modifiers: Self, case_type_case_keyword: Self, case_type_type_keyword: Self, case_type_name: Self, case_type_generic_parameter: Self, case_type_as: Self, case_type_bounds: Self, case_type_equal: Self, case_type_variants: Self, case_type_semicolon: Self) -> Self;
    fn make_case_type_variant(ctx: &C, case_type_variant_bar: Self, case_type_variant_type: Self) -> Self;
    fn make_property_declaration(ctx: &C, property_attribute_spec: Self, property_modifiers: Self, property_type: Self, property_declarators: Self, property_semicolon: Self) -> Self;
    fn make_property_declarator(ctx: &C, property_name: Self, property_initializer: Self) -> Self;
    fn make_namespace_declaration(ctx: &C, namespace_header: Self, namespace_body: Self) -> Self;
    fn make_namespace_declaration_header(ctx: &C, namespace_keyword: Self, namespace_name: Self) -> Self;
    fn make_namespace_body(ctx: &C, namespace_left_brace: Self, namespace_declarations: Self, namespace_right_brace: Self) -> Self;
    fn make_namespace_empty_body(ctx: &C, namespace_semicolon: Self) -> Self;
    fn make_namespace_use_declaration(ctx: &C, namespace_use_keyword: Self, namespace_use_kind: Self, namespace_use_clauses: Self, namespace_use_semicolon: Self) -> Self;
    fn make_namespace_group_use_declaration(ctx: &C, namespace_group_use_keyword: Self, namespace_group_use_kind: Self, namespace_group_use_prefix: Self, namespace_group_use_left_brace: Self, namespace_group_use_clauses: Self, namespace_group_use_right_brace: Self, namespace_group_use_semicolon: Self) -> Self;
    fn make_namespace_use_clause(ctx: &C, namespace_use_clause_kind: Self, namespace_use_name: Self, namespace_use_as: Self, namespace_use_alias: Self) -> Self;
    fn make_function_declaration(ctx: &C, function_attribute_spec: Self, function_declaration_header: Self, function_body: Self) -> Self;
    fn make_function_declaration_header(ctx: &C, function_modifiers: Self, function_keyword: Self, function_name: Self, function_type_parameter_list: Self, function_left_paren: Self, function_parameter_list: Self, function_right_paren: Self, function_contexts: Self, function_colon: Self, function_readonly_return: Self, function_type: Self, function_where_clause: Self) -> Self;
    fn make_contexts(ctx: &C, contexts_left_bracket: Self, contexts_types: Self, contexts_right_bracket: Self) -> Self;
    fn make_where_clause(ctx: &C, where_clause_keyword: Self, where_clause_constraints: Self) -> Self;
    fn make_where_constraint(ctx: &C, where_constraint_left_type: Self, where_constraint_operator: Self, where_constraint_right_type: Self) -> Self;
    fn make_methodish_declaration(ctx: &C, methodish_attribute: Self, methodish_function_decl_header: Self, methodish_function_body: Self, methodish_semicolon: Self) -> Self;
    fn make_methodish_trait_resolution(ctx: &C, methodish_trait_attribute: Self, methodish_trait_function_decl_header: Self, methodish_trait_equal: Self, methodish_trait_name: Self, methodish_trait_semicolon: Self) -> Self;
    fn make_classish_declaration(ctx: &C, classish_attribute: Self, classish_modifiers: Self, classish_xhp: Self, classish_keyword: Self, classish_name: Self, classish_type_parameters: Self, classish_extends_keyword: Self, classish_extends_list: Self, classish_implements_keyword: Self, classish_implements_list: Self, classish_where_clause: Self, classish_body: Self) -> Self;
    fn make_classish_body(ctx: &C, classish_body_left_brace: Self, classish_body_elements: Self, classish_body_right_brace: Self) -> Self;
    fn make_trait_use(ctx: &C, trait_use_keyword: Self, trait_use_names: Self, trait_use_semicolon: Self) -> Self;
    fn make_require_clause(ctx: &C, require_keyword: Self, require_kind: Self, require_name: Self, require_semicolon: Self) -> Self;
    fn make_const_declaration(ctx: &C, const_attribute_spec: Self, const_modifiers: Self, const_keyword: Self, const_type_specifier: Self, const_declarators: Self, const_semicolon: Self) -> Self;
    fn make_constant_declarator(ctx: &C, constant_declarator_name: Self, constant_declarator_initializer: Self) -> Self;
    fn make_type_const_declaration(ctx: &C, type_const_attribute_spec: Self, type_const_modifiers: Self, type_const_keyword: Self, type_const_type_keyword: Self, type_const_name: Self, type_const_type_parameters: Self, type_const_type_constraints: Self, type_const_equal: Self, type_const_type_specifier: Self, type_const_semicolon: Self) -> Self;
    fn make_context_const_declaration(ctx: &C, context_const_modifiers: Self, context_const_const_keyword: Self, context_const_ctx_keyword: Self, context_const_name: Self, context_const_type_parameters: Self, context_const_constraint: Self, context_const_equal: Self, context_const_ctx_list: Self, context_const_semicolon: Self) -> Self;
    fn make_decorated_expression(ctx: &C, decorated_expression_decorator: Self, decorated_expression_expression: Self) -> Self;
    fn make_parameter_declaration(ctx: &C, parameter_attribute: Self, parameter_visibility: Self, parameter_call_convention: Self, parameter_readonly: Self, parameter_type: Self, parameter_name: Self, parameter_default_value: Self) -> Self;
    fn make_variadic_parameter(ctx: &C, variadic_parameter_call_convention: Self, variadic_parameter_type: Self, variadic_parameter_ellipsis: Self) -> Self;
    fn make_old_attribute_specification(ctx: &C, old_attribute_specification_left_double_angle: Self, old_attribute_specification_attributes: Self, old_attribute_specification_right_double_angle: Self) -> Self;
    fn make_attribute_specification(ctx: &C, attribute_specification_attributes: Self) -> Self;
    fn make_attribute(ctx: &C, attribute_at: Self, attribute_attribute_name: Self) -> Self;
    fn make_inclusion_expression(ctx: &C, inclusion_require: Self, inclusion_filename: Self) -> Self;
    fn make_inclusion_directive(ctx: &C, inclusion_expression: Self, inclusion_semicolon: Self) -> Self;
    fn make_compound_statement(ctx: &C, compound_left_brace: Self, compound_statements: Self, compound_right_brace: Self) -> Self;
    fn make_expression_statement(ctx: &C, expression_statement_expression: Self, expression_statement_semicolon: Self) -> Self;
    fn make_markup_section(ctx: &C, markup_hashbang: Self, markup_suffix: Self) -> Self;
    fn make_markup_suffix(ctx: &C, markup_suffix_less_than_question: Self, markup_suffix_name: Self) -> Self;
    fn make_unset_statement(ctx: &C, unset_keyword: Self, unset_left_paren: Self, unset_variables: Self, unset_right_paren: Self, unset_semicolon: Self) -> Self;
    fn make_declare_local_statement(ctx: &C, declare_local_keyword: Self, declare_local_variable: Self, declare_local_colon: Self, declare_local_type: Self, declare_local_initializer: Self, declare_local_semicolon: Self) -> Self;
    fn make_using_statement_block_scoped(ctx: &C, using_block_await_keyword: Self, using_block_using_keyword: Self, using_block_left_paren: Self, using_block_expressions: Self, using_block_right_paren: Self, using_block_body: Self) -> Self;
    fn make_using_statement_function_scoped(ctx: &C, using_function_await_keyword: Self, using_function_using_keyword: Self, using_function_expression: Self, using_function_semicolon: Self) -> Self;
    fn make_while_statement(ctx: &C, while_keyword: Self, while_left_paren: Self, while_condition: Self, while_right_paren: Self, while_body: Self) -> Self;
    fn make_if_statement(ctx: &C, if_keyword: Self, if_left_paren: Self, if_condition: Self, if_right_paren: Self, if_statement: Self, if_else_clause: Self) -> Self;
    fn make_else_clause(ctx: &C, else_keyword: Self, else_statement: Self) -> Self;
    fn make_try_statement(ctx: &C, try_keyword: Self, try_compound_statement: Self, try_catch_clauses: Self, try_finally_clause: Self) -> Self;
    fn make_catch_clause(ctx: &C, catch_keyword: Self, catch_left_paren: Self, catch_type: Self, catch_variable: Self, catch_right_paren: Self, catch_body: Self) -> Self;
    fn make_finally_clause(ctx: &C, finally_keyword: Self, finally_body: Self) -> Self;
    fn make_do_statement(ctx: &C, do_keyword: Self, do_body: Self, do_while_keyword: Self, do_left_paren: Self, do_condition: Self, do_right_paren: Self, do_semicolon: Self) -> Self;
    fn make_for_statement(ctx: &C, for_keyword: Self, for_left_paren: Self, for_initializer: Self, for_first_semicolon: Self, for_control: Self, for_second_semicolon: Self, for_end_of_loop: Self, for_right_paren: Self, for_body: Self) -> Self;
    fn make_foreach_statement(ctx: &C, foreach_keyword: Self, foreach_left_paren: Self, foreach_collection: Self, foreach_await_keyword: Self, foreach_as: Self, foreach_key: Self, foreach_arrow: Self, foreach_value: Self, foreach_right_paren: Self, foreach_body: Self) -> Self;
    fn make_switch_statement(ctx: &C, switch_keyword: Self, switch_left_paren: Self, switch_expression: Self, switch_right_paren: Self, switch_left_brace: Self, switch_sections: Self, switch_right_brace: Self) -> Self;
    fn make_switch_section(ctx: &C, switch_section_labels: Self, switch_section_statements: Self, switch_section_fallthrough: Self) -> Self;
    fn make_switch_fallthrough(ctx: &C, fallthrough_keyword: Self, fallthrough_semicolon: Self) -> Self;
    fn make_case_label(ctx: &C, case_keyword: Self, case_expression: Self, case_colon: Self) -> Self;
    fn make_default_label(ctx: &C, default_keyword: Self, default_colon: Self) -> Self;
    fn make_match_statement(ctx: &C, match_statement_keyword: Self, match_statement_left_paren: Self, match_statement_expression: Self, match_statement_right_paren: Self, match_statement_left_brace: Self, match_statement_arms: Self, match_statement_right_brace: Self) -> Self;
    fn make_match_statement_arm(ctx: &C, match_statement_arm_pattern: Self, match_statement_arm_arrow: Self, match_statement_arm_body: Self) -> Self;
    fn make_return_statement(ctx: &C, return_keyword: Self, return_expression: Self, return_semicolon: Self) -> Self;
    fn make_yield_break_statement(ctx: &C, yield_break_keyword: Self, yield_break_break: Self, yield_break_semicolon: Self) -> Self;
    fn make_throw_statement(ctx: &C, throw_keyword: Self, throw_expression: Self, throw_semicolon: Self) -> Self;
    fn make_break_statement(ctx: &C, break_keyword: Self, break_semicolon: Self) -> Self;
    fn make_continue_statement(ctx: &C, continue_keyword: Self, continue_semicolon: Self) -> Self;
    fn make_echo_statement(ctx: &C, echo_keyword: Self, echo_expressions: Self, echo_semicolon: Self) -> Self;
    fn make_concurrent_statement(ctx: &C, concurrent_keyword: Self, concurrent_statement: Self) -> Self;
    fn make_simple_initializer(ctx: &C, simple_initializer_equal: Self, simple_initializer_value: Self) -> Self;
    fn make_anonymous_class(ctx: &C, anonymous_class_class_keyword: Self, anonymous_class_left_paren: Self, anonymous_class_argument_list: Self, anonymous_class_right_paren: Self, anonymous_class_extends_keyword: Self, anonymous_class_extends_list: Self, anonymous_class_implements_keyword: Self, anonymous_class_implements_list: Self, anonymous_class_body: Self) -> Self;
    fn make_anonymous_function(ctx: &C, anonymous_attribute_spec: Self, anonymous_async_keyword: Self, anonymous_function_keyword: Self, anonymous_left_paren: Self, anonymous_parameters: Self, anonymous_right_paren: Self, anonymous_ctx_list: Self, anonymous_colon: Self, anonymous_readonly_return: Self, anonymous_type: Self, anonymous_use: Self, anonymous_body: Self) -> Self;
    fn make_anonymous_function_use_clause(ctx: &C, anonymous_use_keyword: Self, anonymous_use_left_paren: Self, anonymous_use_variables: Self, anonymous_use_right_paren: Self) -> Self;
    fn make_variable_pattern(ctx: &C, variable_pattern_variable: Self) -> Self;
    fn make_constructor_pattern(ctx: &C, constructor_pattern_constructor: Self, constructor_pattern_left_paren: Self, constructor_pattern_members: Self, constructor_pattern_right_paren: Self) -> Self;
    fn make_refinement_pattern(ctx: &C, refinement_pattern_variable: Self, refinement_pattern_colon: Self, refinement_pattern_specifier: Self) -> Self;
    fn make_lambda_expression(ctx: &C, lambda_attribute_spec: Self, lambda_async: Self, lambda_signature: Self, lambda_arrow: Self, lambda_body: Self) -> Self;
    fn make_lambda_signature(ctx: &C, lambda_left_paren: Self, lambda_parameters: Self, lambda_right_paren: Self, lambda_contexts: Self, lambda_colon: Self, lambda_readonly_return: Self, lambda_type: Self) -> Self;
    fn make_cast_expression(ctx: &C, cast_left_paren: Self, cast_type: Self, cast_right_paren: Self, cast_operand: Self) -> Self;
    fn make_scope_resolution_expression(ctx: &C, scope_resolution_qualifier: Self, scope_resolution_operator: Self, scope_resolution_name: Self) -> Self;
    fn make_member_selection_expression(ctx: &C, member_object: Self, member_operator: Self, member_name: Self) -> Self;
    fn make_safe_member_selection_expression(ctx: &C, safe_member_object: Self, safe_member_operator: Self, safe_member_name: Self) -> Self;
    fn make_embedded_member_selection_expression(ctx: &C, embedded_member_object: Self, embedded_member_operator: Self, embedded_member_name: Self) -> Self;
    fn make_yield_expression(ctx: &C, yield_keyword: Self, yield_operand: Self) -> Self;
    fn make_prefix_unary_expression(ctx: &C, prefix_unary_operator: Self, prefix_unary_operand: Self) -> Self;
    fn make_postfix_unary_expression(ctx: &C, postfix_unary_operand: Self, postfix_unary_operator: Self) -> Self;
    fn make_binary_expression(ctx: &C, binary_left_operand: Self, binary_operator: Self, binary_right_operand: Self) -> Self;
    fn make_is_expression(ctx: &C, is_left_operand: Self, is_operator: Self, is_right_operand: Self) -> Self;
    fn make_as_expression(ctx: &C, as_left_operand: Self, as_operator: Self, as_right_operand: Self) -> Self;
    fn make_nullable_as_expression(ctx: &C, nullable_as_left_operand: Self, nullable_as_operator: Self, nullable_as_right_operand: Self) -> Self;
    fn make_upcast_expression(ctx: &C, upcast_left_operand: Self, upcast_operator: Self, upcast_right_operand: Self) -> Self;
    fn make_conditional_expression(ctx: &C, conditional_test: Self, conditional_question: Self, conditional_consequence: Self, conditional_colon: Self, conditional_alternative: Self) -> Self;
    fn make_eval_expression(ctx: &C, eval_keyword: Self, eval_left_paren: Self, eval_argument: Self, eval_right_paren: Self) -> Self;
    fn make_isset_expression(ctx: &C, isset_keyword: Self, isset_left_paren: Self, isset_argument_list: Self, isset_right_paren: Self) -> Self;
    fn make_nameof_expression(ctx: &C, nameof_keyword: Self, nameof_target: Self) -> Self;
    fn make_function_call_expression(ctx: &C, function_call_receiver: Self, function_call_type_args: Self, function_call_left_paren: Self, function_call_argument_list: Self, function_call_right_paren: Self) -> Self;
    fn make_function_pointer_expression(ctx: &C, function_pointer_receiver: Self, function_pointer_type_args: Self) -> Self;
    fn make_parenthesized_expression(ctx: &C, parenthesized_expression_left_paren: Self, parenthesized_expression_expression: Self, parenthesized_expression_right_paren: Self) -> Self;
    fn make_braced_expression(ctx: &C, braced_expression_left_brace: Self, braced_expression_expression: Self, braced_expression_right_brace: Self) -> Self;
    fn make_et_splice_expression(ctx: &C, et_splice_expression_dollar: Self, et_splice_expression_left_brace: Self, et_splice_expression_expression: Self, et_splice_expression_right_brace: Self) -> Self;
    fn make_embedded_braced_expression(ctx: &C, embedded_braced_expression_left_brace: Self, embedded_braced_expression_expression: Self, embedded_braced_expression_right_brace: Self) -> Self;
    fn make_list_expression(ctx: &C, list_keyword: Self, list_left_paren: Self, list_members: Self, list_right_paren: Self) -> Self;
    fn make_collection_literal_expression(ctx: &C, collection_literal_name: Self, collection_literal_left_brace: Self, collection_literal_initializers: Self, collection_literal_right_brace: Self) -> Self;
    fn make_object_creation_expression(ctx: &C, object_creation_new_keyword: Self, object_creation_object: Self) -> Self;
    fn make_constructor_call(ctx: &C, constructor_call_type: Self, constructor_call_left_paren: Self, constructor_call_argument_list: Self, constructor_call_right_paren: Self) -> Self;
    fn make_darray_intrinsic_expression(ctx: &C, darray_intrinsic_keyword: Self, darray_intrinsic_explicit_type: Self, darray_intrinsic_left_bracket: Self, darray_intrinsic_members: Self, darray_intrinsic_right_bracket: Self) -> Self;
    fn make_dictionary_intrinsic_expression(ctx: &C, dictionary_intrinsic_keyword: Self, dictionary_intrinsic_explicit_type: Self, dictionary_intrinsic_left_bracket: Self, dictionary_intrinsic_members: Self, dictionary_intrinsic_right_bracket: Self) -> Self;
    fn make_keyset_intrinsic_expression(ctx: &C, keyset_intrinsic_keyword: Self, keyset_intrinsic_explicit_type: Self, keyset_intrinsic_left_bracket: Self, keyset_intrinsic_members: Self, keyset_intrinsic_right_bracket: Self) -> Self;
    fn make_varray_intrinsic_expression(ctx: &C, varray_intrinsic_keyword: Self, varray_intrinsic_explicit_type: Self, varray_intrinsic_left_bracket: Self, varray_intrinsic_members: Self, varray_intrinsic_right_bracket: Self) -> Self;
    fn make_vector_intrinsic_expression(ctx: &C, vector_intrinsic_keyword: Self, vector_intrinsic_explicit_type: Self, vector_intrinsic_left_bracket: Self, vector_intrinsic_members: Self, vector_intrinsic_right_bracket: Self) -> Self;
    fn make_element_initializer(ctx: &C, element_key: Self, element_arrow: Self, element_value: Self) -> Self;
    fn make_subscript_expression(ctx: &C, subscript_receiver: Self, subscript_left_bracket: Self, subscript_index: Self, subscript_right_bracket: Self) -> Self;
    fn make_embedded_subscript_expression(ctx: &C, embedded_subscript_receiver: Self, embedded_subscript_left_bracket: Self, embedded_subscript_index: Self, embedded_subscript_right_bracket: Self) -> Self;
    fn make_awaitable_creation_expression(ctx: &C, awaitable_attribute_spec: Self, awaitable_async: Self, awaitable_compound_statement: Self) -> Self;
    fn make_xhp_children_declaration(ctx: &C, xhp_children_keyword: Self, xhp_children_expression: Self, xhp_children_semicolon: Self) -> Self;
    fn make_xhp_children_parenthesized_list(ctx: &C, xhp_children_list_left_paren: Self, xhp_children_list_xhp_children: Self, xhp_children_list_right_paren: Self) -> Self;
    fn make_xhp_category_declaration(ctx: &C, xhp_category_keyword: Self, xhp_category_categories: Self, xhp_category_semicolon: Self) -> Self;
    fn make_xhp_enum_type(ctx: &C, xhp_enum_like: Self, xhp_enum_keyword: Self, xhp_enum_left_brace: Self, xhp_enum_values: Self, xhp_enum_right_brace: Self) -> Self;
    fn make_xhp_lateinit(ctx: &C, xhp_lateinit_at: Self, xhp_lateinit_keyword: Self) -> Self;
    fn make_xhp_required(ctx: &C, xhp_required_at: Self, xhp_required_keyword: Self) -> Self;
    fn make_xhp_class_attribute_declaration(ctx: &C, xhp_attribute_keyword: Self, xhp_attribute_attributes: Self, xhp_attribute_semicolon: Self) -> Self;
    fn make_xhp_class_attribute(ctx: &C, xhp_attribute_decl_type: Self, xhp_attribute_decl_name: Self, xhp_attribute_decl_initializer: Self, xhp_attribute_decl_required: Self) -> Self;
    fn make_xhp_simple_class_attribute(ctx: &C, xhp_simple_class_attribute_type: Self) -> Self;
    fn make_xhp_simple_attribute(ctx: &C, xhp_simple_attribute_name: Self, xhp_simple_attribute_equal: Self, xhp_simple_attribute_expression: Self) -> Self;
    fn make_xhp_spread_attribute(ctx: &C, xhp_spread_attribute_left_brace: Self, xhp_spread_attribute_spread_operator: Self, xhp_spread_attribute_expression: Self, xhp_spread_attribute_right_brace: Self) -> Self;
    fn make_xhp_open(ctx: &C, xhp_open_left_angle: Self, xhp_open_name: Self, xhp_open_attributes: Self, xhp_open_right_angle: Self) -> Self;
    fn make_xhp_expression(ctx: &C, xhp_open: Self, xhp_body: Self, xhp_close: Self) -> Self;
    fn make_xhp_close(ctx: &C, xhp_close_left_angle: Self, xhp_close_name: Self, xhp_close_right_angle: Self) -> Self;
    fn make_type_constant(ctx: &C, type_constant_left_type: Self, type_constant_separator: Self, type_constant_right_type: Self) -> Self;
    fn make_vector_type_specifier(ctx: &C, vector_type_keyword: Self, vector_type_left_angle: Self, vector_type_type: Self, vector_type_trailing_comma: Self, vector_type_right_angle: Self) -> Self;
    fn make_keyset_type_specifier(ctx: &C, keyset_type_keyword: Self, keyset_type_left_angle: Self, keyset_type_type: Self, keyset_type_trailing_comma: Self, keyset_type_right_angle: Self) -> Self;
    fn make_tuple_type_explicit_specifier(ctx: &C, tuple_type_keyword: Self, tuple_type_left_angle: Self, tuple_type_types: Self, tuple_type_right_angle: Self) -> Self;
    fn make_varray_type_specifier(ctx: &C, varray_keyword: Self, varray_left_angle: Self, varray_type: Self, varray_trailing_comma: Self, varray_right_angle: Self) -> Self;
    fn make_function_ctx_type_specifier(ctx: &C, function_ctx_type_keyword: Self, function_ctx_type_variable: Self) -> Self;
    fn make_type_parameter(ctx: &C, type_attribute_spec: Self, type_reified: Self, type_variance: Self, type_name: Self, type_param_params: Self, type_constraints: Self) -> Self;
    fn make_type_constraint(ctx: &C, constraint_keyword: Self, constraint_type: Self) -> Self;
    fn make_context_constraint(ctx: &C, ctx_constraint_keyword: Self, ctx_constraint_ctx_list: Self) -> Self;
    fn make_darray_type_specifier(ctx: &C, darray_keyword: Self, darray_left_angle: Self, darray_key: Self, darray_comma: Self, darray_value: Self, darray_trailing_comma: Self, darray_right_angle: Self) -> Self;
    fn make_dictionary_type_specifier(ctx: &C, dictionary_type_keyword: Self, dictionary_type_left_angle: Self, dictionary_type_members: Self, dictionary_type_right_angle: Self) -> Self;
    fn make_closure_type_specifier(ctx: &C, closure_outer_left_paren: Self, closure_readonly_keyword: Self, closure_function_keyword: Self, closure_inner_left_paren: Self, closure_parameter_list: Self, closure_inner_right_paren: Self, closure_contexts: Self, closure_colon: Self, closure_readonly_return: Self, closure_return_type: Self, closure_outer_right_paren: Self) -> Self;
    fn make_closure_parameter_type_specifier(ctx: &C, closure_parameter_call_convention: Self, closure_parameter_readonly: Self, closure_parameter_type: Self) -> Self;
    fn make_type_refinement(ctx: &C, type_refinement_type: Self, type_refinement_keyword: Self, type_refinement_left_brace: Self, type_refinement_members: Self, type_refinement_right_brace: Self) -> Self;
    fn make_type_in_refinement(ctx: &C, type_in_refinement_keyword: Self, type_in_refinement_name: Self, type_in_refinement_type_parameters: Self, type_in_refinement_constraints: Self, type_in_refinement_equal: Self, type_in_refinement_type: Self) -> Self;
    fn make_ctx_in_refinement(ctx: &C, ctx_in_refinement_keyword: Self, ctx_in_refinement_name: Self, ctx_in_refinement_type_parameters: Self, ctx_in_refinement_constraints: Self, ctx_in_refinement_equal: Self, ctx_in_refinement_ctx_list: Self) -> Self;
    fn make_classname_type_specifier(ctx: &C, classname_keyword: Self, classname_left_angle: Self, classname_type: Self, classname_trailing_comma: Self, classname_right_angle: Self) -> Self;
    fn make_field_specifier(ctx: &C, field_question: Self, field_name: Self, field_arrow: Self, field_type: Self) -> Self;
    fn make_field_initializer(ctx: &C, field_initializer_name: Self, field_initializer_arrow: Self, field_initializer_value: Self) -> Self;
    fn make_shape_type_specifier(ctx: &C, shape_type_keyword: Self, shape_type_left_paren: Self, shape_type_fields: Self, shape_type_ellipsis: Self, shape_type_right_paren: Self) -> Self;
    fn make_shape_expression(ctx: &C, shape_expression_keyword: Self, shape_expression_left_paren: Self, shape_expression_fields: Self, shape_expression_right_paren: Self) -> Self;
    fn make_tuple_expression(ctx: &C, tuple_expression_keyword: Self, tuple_expression_left_paren: Self, tuple_expression_items: Self, tuple_expression_right_paren: Self) -> Self;
    fn make_generic_type_specifier(ctx: &C, generic_class_type: Self, generic_argument_list: Self) -> Self;
    fn make_nullable_type_specifier(ctx: &C, nullable_question: Self, nullable_type: Self) -> Self;
    fn make_like_type_specifier(ctx: &C, like_tilde: Self, like_type: Self) -> Self;
    fn make_soft_type_specifier(ctx: &C, soft_at: Self, soft_type: Self) -> Self;
    fn make_attributized_specifier(ctx: &C, attributized_specifier_attribute_spec: Self, attributized_specifier_type: Self) -> Self;
    fn make_reified_type_argument(ctx: &C, reified_type_argument_reified: Self, reified_type_argument_type: Self) -> Self;
    fn make_type_arguments(ctx: &C, type_arguments_left_angle: Self, type_arguments_types: Self, type_arguments_right_angle: Self) -> Self;
    fn make_type_parameters(ctx: &C, type_parameters_left_angle: Self, type_parameters_parameters: Self, type_parameters_right_angle: Self) -> Self;
    fn make_tuple_type_specifier(ctx: &C, tuple_left_paren: Self, tuple_types: Self, tuple_right_paren: Self) -> Self;
    fn make_union_type_specifier(ctx: &C, union_left_paren: Self, union_types: Self, union_right_paren: Self) -> Self;
    fn make_intersection_type_specifier(ctx: &C, intersection_left_paren: Self, intersection_types: Self, intersection_right_paren: Self) -> Self;
    fn make_error(ctx: &C, error_error: Self) -> Self;
    fn make_list_item(ctx: &C, list_item: Self, list_separator: Self) -> Self;
    fn make_enum_class_label_expression(ctx: &C, enum_class_label_qualifier: Self, enum_class_label_hash: Self, enum_class_label_expression: Self) -> Self;
    fn make_module_declaration(ctx: &C, module_declaration_attribute_spec: Self, module_declaration_new_keyword: Self, module_declaration_module_keyword: Self, module_declaration_name: Self, module_declaration_left_brace: Self, module_declaration_exports: Self, module_declaration_imports: Self, module_declaration_right_brace: Self) -> Self;
    fn make_module_exports(ctx: &C, module_exports_exports_keyword: Self, module_exports_left_brace: Self, module_exports_exports: Self, module_exports_right_brace: Self) -> Self;
    fn make_module_imports(ctx: &C, module_imports_imports_keyword: Self, module_imports_left_brace: Self, module_imports_imports: Self, module_imports_right_brace: Self) -> Self;
    fn make_module_membership_declaration(ctx: &C, module_membership_declaration_module_keyword: Self, module_membership_declaration_name: Self, module_membership_declaration_semicolon: Self) -> Self;
    fn make_package_expression(ctx: &C, package_expression_keyword: Self, package_expression_name: Self) -> Self;

}
