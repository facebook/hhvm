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
use crate::lexable_token::LexableToken;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;

pub trait SyntaxTypeBase<T, V> {
    fn make_missing(offset: usize) -> Self;
    fn make_token(arg: T) -> Self;
    fn make_list(arg: Box<Vec<Self>>, offset: usize) -> Self where Self : Sized;
}

pub trait SyntaxType<T, V> : SyntaxTypeBase<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    fn make_end_of_file(end_of_file_token: Self) -> Self;
    fn make_script(script_declarations: Self) -> Self;
    fn make_qualified_name(qualified_name_parts: Self) -> Self;
    fn make_simple_type_specifier(simple_type_specifier: Self) -> Self;
    fn make_literal_expression(literal_expression: Self) -> Self;
    fn make_prefixed_string_expression(prefixed_string_name: Self, prefixed_string_str: Self) -> Self;
    fn make_variable_expression(variable_expression: Self) -> Self;
    fn make_pipe_variable_expression(pipe_variable_expression: Self) -> Self;
    fn make_file_attribute_specification(file_attribute_specification_left_double_angle: Self, file_attribute_specification_keyword: Self, file_attribute_specification_colon: Self, file_attribute_specification_attributes: Self, file_attribute_specification_right_double_angle: Self) -> Self;
    fn make_enum_declaration(enum_attribute_spec: Self, enum_keyword: Self, enum_name: Self, enum_colon: Self, enum_base: Self, enum_type: Self, enum_left_brace: Self, enum_enumerators: Self, enum_right_brace: Self) -> Self;
    fn make_enumerator(enumerator_name: Self, enumerator_equal: Self, enumerator_value: Self, enumerator_semicolon: Self) -> Self;
    fn make_record_declaration(record_attribute_spec: Self, record_keyword: Self, record_name: Self, record_left_brace: Self, record_fields: Self, record_right_brace: Self) -> Self;
    fn make_record_field(record_field_name: Self, record_field_colon: Self, record_field_type: Self, record_field_init: Self, record_field_comma: Self) -> Self;
    fn make_alias_declaration(alias_attribute_spec: Self, alias_keyword: Self, alias_name: Self, alias_generic_parameter: Self, alias_constraint: Self, alias_equal: Self, alias_type: Self, alias_semicolon: Self) -> Self;
    fn make_property_declaration(property_attribute_spec: Self, property_modifiers: Self, property_type: Self, property_declarators: Self, property_semicolon: Self) -> Self;
    fn make_property_declarator(property_name: Self, property_initializer: Self) -> Self;
    fn make_namespace_declaration(namespace_keyword: Self, namespace_name: Self, namespace_body: Self) -> Self;
    fn make_namespace_body(namespace_left_brace: Self, namespace_declarations: Self, namespace_right_brace: Self) -> Self;
    fn make_namespace_empty_body(namespace_semicolon: Self) -> Self;
    fn make_namespace_use_declaration(namespace_use_keyword: Self, namespace_use_kind: Self, namespace_use_clauses: Self, namespace_use_semicolon: Self) -> Self;
    fn make_namespace_group_use_declaration(namespace_group_use_keyword: Self, namespace_group_use_kind: Self, namespace_group_use_prefix: Self, namespace_group_use_left_brace: Self, namespace_group_use_clauses: Self, namespace_group_use_right_brace: Self, namespace_group_use_semicolon: Self) -> Self;
    fn make_namespace_use_clause(namespace_use_clause_kind: Self, namespace_use_name: Self, namespace_use_as: Self, namespace_use_alias: Self) -> Self;
    fn make_function_declaration(function_attribute_spec: Self, function_declaration_header: Self, function_body: Self) -> Self;
    fn make_function_declaration_header(function_modifiers: Self, function_keyword: Self, function_name: Self, function_type_parameter_list: Self, function_left_paren: Self, function_parameter_list: Self, function_right_paren: Self, function_colon: Self, function_type: Self, function_where_clause: Self) -> Self;
    fn make_where_clause(where_clause_keyword: Self, where_clause_constraints: Self) -> Self;
    fn make_where_constraint(where_constraint_left_type: Self, where_constraint_operator: Self, where_constraint_right_type: Self) -> Self;
    fn make_methodish_declaration(methodish_attribute: Self, methodish_function_decl_header: Self, methodish_function_body: Self, methodish_semicolon: Self) -> Self;
    fn make_methodish_trait_resolution(methodish_trait_attribute: Self, methodish_trait_function_decl_header: Self, methodish_trait_equal: Self, methodish_trait_name: Self, methodish_trait_semicolon: Self) -> Self;
    fn make_classish_declaration(classish_attribute: Self, classish_modifiers: Self, classish_keyword: Self, classish_name: Self, classish_type_parameters: Self, classish_extends_keyword: Self, classish_extends_list: Self, classish_implements_keyword: Self, classish_implements_list: Self, classish_body: Self) -> Self;
    fn make_classish_body(classish_body_left_brace: Self, classish_body_elements: Self, classish_body_right_brace: Self) -> Self;
    fn make_trait_use_precedence_item(trait_use_precedence_item_name: Self, trait_use_precedence_item_keyword: Self, trait_use_precedence_item_removed_names: Self) -> Self;
    fn make_trait_use_alias_item(trait_use_alias_item_aliasing_name: Self, trait_use_alias_item_keyword: Self, trait_use_alias_item_modifiers: Self, trait_use_alias_item_aliased_name: Self) -> Self;
    fn make_trait_use_conflict_resolution(trait_use_conflict_resolution_keyword: Self, trait_use_conflict_resolution_names: Self, trait_use_conflict_resolution_left_brace: Self, trait_use_conflict_resolution_clauses: Self, trait_use_conflict_resolution_right_brace: Self) -> Self;
    fn make_trait_use(trait_use_keyword: Self, trait_use_names: Self, trait_use_semicolon: Self) -> Self;
    fn make_require_clause(require_keyword: Self, require_kind: Self, require_name: Self, require_semicolon: Self) -> Self;
    fn make_const_declaration(const_visibility: Self, const_abstract: Self, const_keyword: Self, const_type_specifier: Self, const_declarators: Self, const_semicolon: Self) -> Self;
    fn make_constant_declarator(constant_declarator_name: Self, constant_declarator_initializer: Self) -> Self;
    fn make_type_const_declaration(type_const_attribute_spec: Self, type_const_abstract: Self, type_const_keyword: Self, type_const_type_keyword: Self, type_const_name: Self, type_const_type_parameters: Self, type_const_type_constraint: Self, type_const_equal: Self, type_const_type_specifier: Self, type_const_semicolon: Self) -> Self;
    fn make_decorated_expression(decorated_expression_decorator: Self, decorated_expression_expression: Self) -> Self;
    fn make_parameter_declaration(parameter_attribute: Self, parameter_visibility: Self, parameter_call_convention: Self, parameter_type: Self, parameter_name: Self, parameter_default_value: Self) -> Self;
    fn make_variadic_parameter(variadic_parameter_call_convention: Self, variadic_parameter_type: Self, variadic_parameter_ellipsis: Self) -> Self;
    fn make_attribute_specification(attribute_specification_left_double_angle: Self, attribute_specification_attributes: Self, attribute_specification_right_double_angle: Self) -> Self;
    fn make_inclusion_expression(inclusion_require: Self, inclusion_filename: Self) -> Self;
    fn make_inclusion_directive(inclusion_expression: Self, inclusion_semicolon: Self) -> Self;
    fn make_compound_statement(compound_left_brace: Self, compound_statements: Self, compound_right_brace: Self) -> Self;
    fn make_alternate_loop_statement(alternate_loop_opening_colon: Self, alternate_loop_statements: Self, alternate_loop_closing_keyword: Self, alternate_loop_closing_semicolon: Self) -> Self;
    fn make_expression_statement(expression_statement_expression: Self, expression_statement_semicolon: Self) -> Self;
    fn make_markup_section(markup_prefix: Self, markup_text: Self, markup_suffix: Self, markup_expression: Self) -> Self;
    fn make_markup_suffix(markup_suffix_less_than_question: Self, markup_suffix_name: Self) -> Self;
    fn make_unset_statement(unset_keyword: Self, unset_left_paren: Self, unset_variables: Self, unset_right_paren: Self, unset_semicolon: Self) -> Self;
    fn make_let_statement(let_statement_keyword: Self, let_statement_name: Self, let_statement_colon: Self, let_statement_type: Self, let_statement_initializer: Self, let_statement_semicolon: Self) -> Self;
    fn make_using_statement_block_scoped(using_block_await_keyword: Self, using_block_using_keyword: Self, using_block_left_paren: Self, using_block_expressions: Self, using_block_right_paren: Self, using_block_body: Self) -> Self;
    fn make_using_statement_function_scoped(using_function_await_keyword: Self, using_function_using_keyword: Self, using_function_expression: Self, using_function_semicolon: Self) -> Self;
    fn make_declare_directive_statement(declare_directive_keyword: Self, declare_directive_left_paren: Self, declare_directive_expression: Self, declare_directive_right_paren: Self, declare_directive_semicolon: Self) -> Self;
    fn make_declare_block_statement(declare_block_keyword: Self, declare_block_left_paren: Self, declare_block_expression: Self, declare_block_right_paren: Self, declare_block_body: Self) -> Self;
    fn make_while_statement(while_keyword: Self, while_left_paren: Self, while_condition: Self, while_right_paren: Self, while_body: Self) -> Self;
    fn make_if_statement(if_keyword: Self, if_left_paren: Self, if_condition: Self, if_right_paren: Self, if_statement: Self, if_elseif_clauses: Self, if_else_clause: Self) -> Self;
    fn make_elseif_clause(elseif_keyword: Self, elseif_left_paren: Self, elseif_condition: Self, elseif_right_paren: Self, elseif_statement: Self) -> Self;
    fn make_else_clause(else_keyword: Self, else_statement: Self) -> Self;
    fn make_alternate_if_statement(alternate_if_keyword: Self, alternate_if_left_paren: Self, alternate_if_condition: Self, alternate_if_right_paren: Self, alternate_if_colon: Self, alternate_if_statement: Self, alternate_if_elseif_clauses: Self, alternate_if_else_clause: Self, alternate_if_endif_keyword: Self, alternate_if_semicolon: Self) -> Self;
    fn make_alternate_elseif_clause(alternate_elseif_keyword: Self, alternate_elseif_left_paren: Self, alternate_elseif_condition: Self, alternate_elseif_right_paren: Self, alternate_elseif_colon: Self, alternate_elseif_statement: Self) -> Self;
    fn make_alternate_else_clause(alternate_else_keyword: Self, alternate_else_colon: Self, alternate_else_statement: Self) -> Self;
    fn make_try_statement(try_keyword: Self, try_compound_statement: Self, try_catch_clauses: Self, try_finally_clause: Self) -> Self;
    fn make_catch_clause(catch_keyword: Self, catch_left_paren: Self, catch_type: Self, catch_variable: Self, catch_right_paren: Self, catch_body: Self) -> Self;
    fn make_finally_clause(finally_keyword: Self, finally_body: Self) -> Self;
    fn make_do_statement(do_keyword: Self, do_body: Self, do_while_keyword: Self, do_left_paren: Self, do_condition: Self, do_right_paren: Self, do_semicolon: Self) -> Self;
    fn make_for_statement(for_keyword: Self, for_left_paren: Self, for_initializer: Self, for_first_semicolon: Self, for_control: Self, for_second_semicolon: Self, for_end_of_loop: Self, for_right_paren: Self, for_body: Self) -> Self;
    fn make_foreach_statement(foreach_keyword: Self, foreach_left_paren: Self, foreach_collection: Self, foreach_await_keyword: Self, foreach_as: Self, foreach_key: Self, foreach_arrow: Self, foreach_value: Self, foreach_right_paren: Self, foreach_body: Self) -> Self;
    fn make_switch_statement(switch_keyword: Self, switch_left_paren: Self, switch_expression: Self, switch_right_paren: Self, switch_left_brace: Self, switch_sections: Self, switch_right_brace: Self) -> Self;
    fn make_alternate_switch_statement(alternate_switch_keyword: Self, alternate_switch_left_paren: Self, alternate_switch_expression: Self, alternate_switch_right_paren: Self, alternate_switch_opening_colon: Self, alternate_switch_sections: Self, alternate_switch_closing_endswitch: Self, alternate_switch_closing_semicolon: Self) -> Self;
    fn make_switch_section(switch_section_labels: Self, switch_section_statements: Self, switch_section_fallthrough: Self) -> Self;
    fn make_switch_fallthrough(fallthrough_keyword: Self, fallthrough_semicolon: Self) -> Self;
    fn make_case_label(case_keyword: Self, case_expression: Self, case_colon: Self) -> Self;
    fn make_default_label(default_keyword: Self, default_colon: Self) -> Self;
    fn make_return_statement(return_keyword: Self, return_expression: Self, return_semicolon: Self) -> Self;
    fn make_goto_label(goto_label_name: Self, goto_label_colon: Self) -> Self;
    fn make_goto_statement(goto_statement_keyword: Self, goto_statement_label_name: Self, goto_statement_semicolon: Self) -> Self;
    fn make_throw_statement(throw_keyword: Self, throw_expression: Self, throw_semicolon: Self) -> Self;
    fn make_break_statement(break_keyword: Self, break_level: Self, break_semicolon: Self) -> Self;
    fn make_continue_statement(continue_keyword: Self, continue_level: Self, continue_semicolon: Self) -> Self;
    fn make_echo_statement(echo_keyword: Self, echo_expressions: Self, echo_semicolon: Self) -> Self;
    fn make_concurrent_statement(concurrent_keyword: Self, concurrent_statement: Self) -> Self;
    fn make_simple_initializer(simple_initializer_equal: Self, simple_initializer_value: Self) -> Self;
    fn make_anonymous_class(anonymous_class_class_keyword: Self, anonymous_class_left_paren: Self, anonymous_class_argument_list: Self, anonymous_class_right_paren: Self, anonymous_class_extends_keyword: Self, anonymous_class_extends_list: Self, anonymous_class_implements_keyword: Self, anonymous_class_implements_list: Self, anonymous_class_body: Self) -> Self;
    fn make_anonymous_function(anonymous_attribute_spec: Self, anonymous_static_keyword: Self, anonymous_async_keyword: Self, anonymous_coroutine_keyword: Self, anonymous_function_keyword: Self, anonymous_left_paren: Self, anonymous_parameters: Self, anonymous_right_paren: Self, anonymous_colon: Self, anonymous_type: Self, anonymous_use: Self, anonymous_body: Self) -> Self;
    fn make_php7_anonymous_function(php7_anonymous_attribute_spec: Self, php7_anonymous_static_keyword: Self, php7_anonymous_async_keyword: Self, php7_anonymous_coroutine_keyword: Self, php7_anonymous_function_keyword: Self, php7_anonymous_left_paren: Self, php7_anonymous_parameters: Self, php7_anonymous_right_paren: Self, php7_anonymous_use: Self, php7_anonymous_colon: Self, php7_anonymous_type: Self, php7_anonymous_body: Self) -> Self;
    fn make_anonymous_function_use_clause(anonymous_use_keyword: Self, anonymous_use_left_paren: Self, anonymous_use_variables: Self, anonymous_use_right_paren: Self) -> Self;
    fn make_lambda_expression(lambda_attribute_spec: Self, lambda_async: Self, lambda_coroutine: Self, lambda_signature: Self, lambda_arrow: Self, lambda_body: Self) -> Self;
    fn make_lambda_signature(lambda_left_paren: Self, lambda_parameters: Self, lambda_right_paren: Self, lambda_colon: Self, lambda_type: Self) -> Self;
    fn make_cast_expression(cast_left_paren: Self, cast_type: Self, cast_right_paren: Self, cast_operand: Self) -> Self;
    fn make_scope_resolution_expression(scope_resolution_qualifier: Self, scope_resolution_operator: Self, scope_resolution_name: Self) -> Self;
    fn make_member_selection_expression(member_object: Self, member_operator: Self, member_name: Self) -> Self;
    fn make_safe_member_selection_expression(safe_member_object: Self, safe_member_operator: Self, safe_member_name: Self) -> Self;
    fn make_embedded_member_selection_expression(embedded_member_object: Self, embedded_member_operator: Self, embedded_member_name: Self) -> Self;
    fn make_yield_expression(yield_keyword: Self, yield_operand: Self) -> Self;
    fn make_yield_from_expression(yield_from_yield_keyword: Self, yield_from_from_keyword: Self, yield_from_operand: Self) -> Self;
    fn make_prefix_unary_expression(prefix_unary_operator: Self, prefix_unary_operand: Self) -> Self;
    fn make_postfix_unary_expression(postfix_unary_operand: Self, postfix_unary_operator: Self) -> Self;
    fn make_binary_expression(binary_left_operand: Self, binary_operator: Self, binary_right_operand: Self) -> Self;
    fn make_instanceof_expression(instanceof_left_operand: Self, instanceof_operator: Self, instanceof_right_operand: Self) -> Self;
    fn make_is_expression(is_left_operand: Self, is_operator: Self, is_right_operand: Self) -> Self;
    fn make_as_expression(as_left_operand: Self, as_operator: Self, as_right_operand: Self) -> Self;
    fn make_nullable_as_expression(nullable_as_left_operand: Self, nullable_as_operator: Self, nullable_as_right_operand: Self) -> Self;
    fn make_conditional_expression(conditional_test: Self, conditional_question: Self, conditional_consequence: Self, conditional_colon: Self, conditional_alternative: Self) -> Self;
    fn make_eval_expression(eval_keyword: Self, eval_left_paren: Self, eval_argument: Self, eval_right_paren: Self) -> Self;
    fn make_empty_expression(empty_keyword: Self, empty_left_paren: Self, empty_argument: Self, empty_right_paren: Self) -> Self;
    fn make_define_expression(define_keyword: Self, define_left_paren: Self, define_argument_list: Self, define_right_paren: Self) -> Self;
    fn make_halt_compiler_expression(halt_compiler_keyword: Self, halt_compiler_left_paren: Self, halt_compiler_argument_list: Self, halt_compiler_right_paren: Self) -> Self;
    fn make_isset_expression(isset_keyword: Self, isset_left_paren: Self, isset_argument_list: Self, isset_right_paren: Self) -> Self;
    fn make_function_call_expression(function_call_receiver: Self, function_call_type_args: Self, function_call_left_paren: Self, function_call_argument_list: Self, function_call_right_paren: Self) -> Self;
    fn make_parenthesized_expression(parenthesized_expression_left_paren: Self, parenthesized_expression_expression: Self, parenthesized_expression_right_paren: Self) -> Self;
    fn make_braced_expression(braced_expression_left_brace: Self, braced_expression_expression: Self, braced_expression_right_brace: Self) -> Self;
    fn make_embedded_braced_expression(embedded_braced_expression_left_brace: Self, embedded_braced_expression_expression: Self, embedded_braced_expression_right_brace: Self) -> Self;
    fn make_list_expression(list_keyword: Self, list_left_paren: Self, list_members: Self, list_right_paren: Self) -> Self;
    fn make_collection_literal_expression(collection_literal_name: Self, collection_literal_left_brace: Self, collection_literal_initializers: Self, collection_literal_right_brace: Self) -> Self;
    fn make_object_creation_expression(object_creation_new_keyword: Self, object_creation_object: Self) -> Self;
    fn make_constructor_call(constructor_call_type: Self, constructor_call_left_paren: Self, constructor_call_argument_list: Self, constructor_call_right_paren: Self) -> Self;
    fn make_record_creation_expression(record_creation_type: Self, record_creation_left_bracket: Self, record_creation_members: Self, record_creation_right_bracket: Self) -> Self;
    fn make_array_creation_expression(array_creation_left_bracket: Self, array_creation_members: Self, array_creation_right_bracket: Self) -> Self;
    fn make_array_intrinsic_expression(array_intrinsic_keyword: Self, array_intrinsic_left_paren: Self, array_intrinsic_members: Self, array_intrinsic_right_paren: Self) -> Self;
    fn make_darray_intrinsic_expression(darray_intrinsic_keyword: Self, darray_intrinsic_explicit_type: Self, darray_intrinsic_left_bracket: Self, darray_intrinsic_members: Self, darray_intrinsic_right_bracket: Self) -> Self;
    fn make_dictionary_intrinsic_expression(dictionary_intrinsic_keyword: Self, dictionary_intrinsic_explicit_type: Self, dictionary_intrinsic_left_bracket: Self, dictionary_intrinsic_members: Self, dictionary_intrinsic_right_bracket: Self) -> Self;
    fn make_keyset_intrinsic_expression(keyset_intrinsic_keyword: Self, keyset_intrinsic_explicit_type: Self, keyset_intrinsic_left_bracket: Self, keyset_intrinsic_members: Self, keyset_intrinsic_right_bracket: Self) -> Self;
    fn make_varray_intrinsic_expression(varray_intrinsic_keyword: Self, varray_intrinsic_explicit_type: Self, varray_intrinsic_left_bracket: Self, varray_intrinsic_members: Self, varray_intrinsic_right_bracket: Self) -> Self;
    fn make_vector_intrinsic_expression(vector_intrinsic_keyword: Self, vector_intrinsic_explicit_type: Self, vector_intrinsic_left_bracket: Self, vector_intrinsic_members: Self, vector_intrinsic_right_bracket: Self) -> Self;
    fn make_element_initializer(element_key: Self, element_arrow: Self, element_value: Self) -> Self;
    fn make_subscript_expression(subscript_receiver: Self, subscript_left_bracket: Self, subscript_index: Self, subscript_right_bracket: Self) -> Self;
    fn make_embedded_subscript_expression(embedded_subscript_receiver: Self, embedded_subscript_left_bracket: Self, embedded_subscript_index: Self, embedded_subscript_right_bracket: Self) -> Self;
    fn make_awaitable_creation_expression(awaitable_attribute_spec: Self, awaitable_async: Self, awaitable_coroutine: Self, awaitable_compound_statement: Self) -> Self;
    fn make_xhp_children_declaration(xhp_children_keyword: Self, xhp_children_expression: Self, xhp_children_semicolon: Self) -> Self;
    fn make_xhp_children_parenthesized_list(xhp_children_list_left_paren: Self, xhp_children_list_xhp_children: Self, xhp_children_list_right_paren: Self) -> Self;
    fn make_xhp_category_declaration(xhp_category_keyword: Self, xhp_category_categories: Self, xhp_category_semicolon: Self) -> Self;
    fn make_xhp_enum_type(xhp_enum_optional: Self, xhp_enum_keyword: Self, xhp_enum_left_brace: Self, xhp_enum_values: Self, xhp_enum_right_brace: Self) -> Self;
    fn make_xhp_required(xhp_required_at: Self, xhp_required_keyword: Self) -> Self;
    fn make_xhp_class_attribute_declaration(xhp_attribute_keyword: Self, xhp_attribute_attributes: Self, xhp_attribute_semicolon: Self) -> Self;
    fn make_xhp_class_attribute(xhp_attribute_decl_type: Self, xhp_attribute_decl_name: Self, xhp_attribute_decl_initializer: Self, xhp_attribute_decl_required: Self) -> Self;
    fn make_xhp_simple_class_attribute(xhp_simple_class_attribute_type: Self) -> Self;
    fn make_xhp_simple_attribute(xhp_simple_attribute_name: Self, xhp_simple_attribute_equal: Self, xhp_simple_attribute_expression: Self) -> Self;
    fn make_xhp_spread_attribute(xhp_spread_attribute_left_brace: Self, xhp_spread_attribute_spread_operator: Self, xhp_spread_attribute_expression: Self, xhp_spread_attribute_right_brace: Self) -> Self;
    fn make_xhp_open(xhp_open_left_angle: Self, xhp_open_name: Self, xhp_open_attributes: Self, xhp_open_right_angle: Self) -> Self;
    fn make_xhp_expression(xhp_open: Self, xhp_body: Self, xhp_close: Self) -> Self;
    fn make_xhp_close(xhp_close_left_angle: Self, xhp_close_name: Self, xhp_close_right_angle: Self) -> Self;
    fn make_type_constant(type_constant_left_type: Self, type_constant_separator: Self, type_constant_right_type: Self) -> Self;
    fn make_vector_type_specifier(vector_type_keyword: Self, vector_type_left_angle: Self, vector_type_type: Self, vector_type_trailing_comma: Self, vector_type_right_angle: Self) -> Self;
    fn make_keyset_type_specifier(keyset_type_keyword: Self, keyset_type_left_angle: Self, keyset_type_type: Self, keyset_type_trailing_comma: Self, keyset_type_right_angle: Self) -> Self;
    fn make_tuple_type_explicit_specifier(tuple_type_keyword: Self, tuple_type_left_angle: Self, tuple_type_types: Self, tuple_type_right_angle: Self) -> Self;
    fn make_varray_type_specifier(varray_keyword: Self, varray_left_angle: Self, varray_type: Self, varray_trailing_comma: Self, varray_right_angle: Self) -> Self;
    fn make_vector_array_type_specifier(vector_array_keyword: Self, vector_array_left_angle: Self, vector_array_type: Self, vector_array_right_angle: Self) -> Self;
    fn make_type_parameter(type_attribute_spec: Self, type_reified: Self, type_variance: Self, type_name: Self, type_constraints: Self) -> Self;
    fn make_type_constraint(constraint_keyword: Self, constraint_type: Self) -> Self;
    fn make_darray_type_specifier(darray_keyword: Self, darray_left_angle: Self, darray_key: Self, darray_comma: Self, darray_value: Self, darray_trailing_comma: Self, darray_right_angle: Self) -> Self;
    fn make_map_array_type_specifier(map_array_keyword: Self, map_array_left_angle: Self, map_array_key: Self, map_array_comma: Self, map_array_value: Self, map_array_right_angle: Self) -> Self;
    fn make_dictionary_type_specifier(dictionary_type_keyword: Self, dictionary_type_left_angle: Self, dictionary_type_members: Self, dictionary_type_right_angle: Self) -> Self;
    fn make_closure_type_specifier(closure_outer_left_paren: Self, closure_coroutine: Self, closure_function_keyword: Self, closure_inner_left_paren: Self, closure_parameter_list: Self, closure_inner_right_paren: Self, closure_colon: Self, closure_return_type: Self, closure_outer_right_paren: Self) -> Self;
    fn make_closure_parameter_type_specifier(closure_parameter_call_convention: Self, closure_parameter_type: Self) -> Self;
    fn make_classname_type_specifier(classname_keyword: Self, classname_left_angle: Self, classname_type: Self, classname_trailing_comma: Self, classname_right_angle: Self) -> Self;
    fn make_field_specifier(field_question: Self, field_name: Self, field_arrow: Self, field_type: Self) -> Self;
    fn make_field_initializer(field_initializer_name: Self, field_initializer_arrow: Self, field_initializer_value: Self) -> Self;
    fn make_shape_type_specifier(shape_type_keyword: Self, shape_type_left_paren: Self, shape_type_fields: Self, shape_type_ellipsis: Self, shape_type_right_paren: Self) -> Self;
    fn make_shape_expression(shape_expression_keyword: Self, shape_expression_left_paren: Self, shape_expression_fields: Self, shape_expression_right_paren: Self) -> Self;
    fn make_tuple_expression(tuple_expression_keyword: Self, tuple_expression_left_paren: Self, tuple_expression_items: Self, tuple_expression_right_paren: Self) -> Self;
    fn make_generic_type_specifier(generic_class_type: Self, generic_argument_list: Self) -> Self;
    fn make_nullable_type_specifier(nullable_question: Self, nullable_type: Self) -> Self;
    fn make_like_type_specifier(like_tilde: Self, like_type: Self) -> Self;
    fn make_soft_type_specifier(soft_at: Self, soft_type: Self) -> Self;
    fn make_reified_type_argument(reified_type_argument_reified: Self, reified_type_argument_type: Self) -> Self;
    fn make_type_arguments(type_arguments_left_angle: Self, type_arguments_types: Self, type_arguments_right_angle: Self) -> Self;
    fn make_type_parameters(type_parameters_left_angle: Self, type_parameters_parameters: Self, type_parameters_right_angle: Self) -> Self;
    fn make_tuple_type_specifier(tuple_left_paren: Self, tuple_types: Self, tuple_right_paren: Self) -> Self;
    fn make_error(error_error: Self) -> Self;
    fn make_list_item(list_item: Self, list_separator: Self) -> Self;
    fn make_pocket_atom_expression(pocket_atom_glyph: Self, pocket_atom_expression: Self) -> Self;
    fn make_pocket_identifier_expression(pocket_identifier_qualifier: Self, pocket_identifier_pu_operator: Self, pocket_identifier_field: Self, pocket_identifier_operator: Self, pocket_identifier_name: Self) -> Self;
    fn make_pocket_atom_mapping_declaration(pocket_atom_mapping_glyph: Self, pocket_atom_mapping_name: Self, pocket_atom_mapping_left_paren: Self, pocket_atom_mapping_mappings: Self, pocket_atom_mapping_right_paren: Self, pocket_atom_mapping_semicolon: Self) -> Self;
    fn make_pocket_enum_declaration(pocket_enum_modifiers: Self, pocket_enum_enum: Self, pocket_enum_name: Self, pocket_enum_left_brace: Self, pocket_enum_fields: Self, pocket_enum_right_brace: Self) -> Self;
    fn make_pocket_field_type_expr_declaration(pocket_field_type_expr_case: Self, pocket_field_type_expr_type: Self, pocket_field_type_expr_name: Self, pocket_field_type_expr_semicolon: Self) -> Self;
    fn make_pocket_field_type_declaration(pocket_field_type_case: Self, pocket_field_type_type: Self, pocket_field_type_name: Self, pocket_field_type_semicolon: Self) -> Self;
    fn make_pocket_mapping_id_declaration(pocket_mapping_id_name: Self, pocket_mapping_id_initializer: Self) -> Self;
    fn make_pocket_mapping_type_declaration(pocket_mapping_type_keyword: Self, pocket_mapping_type_name: Self, pocket_mapping_type_equal: Self, pocket_mapping_type_type: Self) -> Self;


    fn fold_over_children<'a, U>(
        f: &Fn(&'a Self, U) -> U,
        acc: U,
        syntax: &'a SyntaxVariant<T, V>,
    ) -> U;
    fn kind(&self) -> SyntaxKind;
}
  